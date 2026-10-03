#!/usr/bin/env node

/**
 * Runs the built template in a real browser against a real CNA WebAssembly runtime.
 *
 * A Vite build that succeeds says nothing about whether a game runs, so this serves the built
 * bundle and the `cna_c_api` artifact over HTTP, drives the same `HelloGame` a Node consumer
 * drives, and asserts the frames it actually drew.
 */

import assert from "node:assert/strict";
import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const DIST = path.join(ROOT, "dist");
const artifactDirectory = process.env.CNA_WASM_ARTIFACT_DIR;
const frames = Number.parseInt(process.env.CNA_BROWSER_FRAMES ?? "60", 10);

if (!artifactDirectory) {
  throw new Error("set CNA_WASM_ARTIFACT_DIR to the directory holding cna_c_api.mjs and cna_c_api.wasm");
}
if (!fs.existsSync(path.join(artifactDirectory, "cna_c_api.mjs"))) {
  throw new Error(`no cna_c_api.mjs under ${artifactDirectory}`);
}
if (!fs.existsSync(path.join(DIST, "index.html"))) throw new Error("run npm run build first");
if (!Number.isSafeInteger(frames) || frames <= 0) throw new Error("CNA_BROWSER_FRAMES must be positive");

const TYPES = new Map(Object.entries({
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".map": "application/json; charset=utf-8",
  ".wasm": "application/wasm",
  ".data": "application/octet-stream",
}));

async function importPlaywright() {
  const candidates = [];
  if (process.env.CNA_PLAYWRIGHT_MODULE) candidates.push(process.env.CNA_PLAYWRIGHT_MODULE);
  candidates.push("playwright");
  candidates.push(path.resolve(path.dirname(process.execPath), "../lib/node_modules/playwright/index.mjs"));
  for (const candidate of candidates) {
    try {
      const specifier = path.isAbsolute(candidate) ? pathToFileURL(candidate).href : candidate;
      return await import(specifier);
    } catch {
      continue;
    }
  }
  throw new Error("playwright is not installed; set CNA_PLAYWRIGHT_MODULE to an install of it");
}

const server = http.createServer((request, response) => {
  const url = new URL(request.url ?? "/", "http://localhost");
  let file = null;
  if (url.pathname === "/") file = path.join(DIST, "index.html");
  else if (url.pathname.startsWith("/wasm/")) file = path.join(artifactDirectory, url.pathname.slice("/wasm/".length));
  else file = path.join(DIST, url.pathname.slice(1));
  if (!fs.existsSync(file) || !fs.statSync(file).isFile()) {
    response.writeHead(404).end("not found");
    return;
  }
  response.writeHead(200, {
    "content-type": TYPES.get(path.extname(file)) ?? "application/octet-stream",
    "cache-control": "no-store",
  });
  fs.createReadStream(file).pipe(response);
});
await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
const port = server.address().port;

const playwright = await importPlaywright();
const browser = await playwright.chromium.launch({
  args: ["--use-gl=swiftshader", "--enable-unsafe-swiftshader", "--disable-gpu-sandbox"],
});
try {
  const page = await browser.newPage();
  // CNA writes its own log to stderr, which Emscripten routes to console.error whatever the level.
  const runtimeLog = /^\[(INFO|DEBUG|TRACE|WARN|WARNING|EXPERIMENT)\]\[[A-Z]+\] /;
  const consoleErrors = [];
  page.on("console", (message) => {
    if (message.type() === "error" && !runtimeLog.test(message.text())) consoleErrors.push(message.text());
  });
  page.on("pageerror", (error) => consoleErrors.push(String(error)));
  await page.goto(`http://127.0.0.1:${port}/?wasm=/wasm/cna_c_api.mjs&frames=${frames}`, { waitUntil: "load" });
  await page.waitForFunction(
    () => globalThis.__cnaTemplateResult !== undefined,
    undefined,
    { timeout: 180_000 },
  );
  const result = await page.evaluate(() => globalThis.__cnaTemplateResult);
  assert.equal(result.status, "ok", result.error ?? "");
  assert.equal(result.backend, "wasm");
  assert.match(String(result.abiVersion), /^\d+\.\d+\.\d+$/);
  assert.equal(result.frames, frames);
  assert.ok(result.updates >= frames, `expected at least ${frames} updates, saw ${result.updates}`);
  assert.equal(result.disposed, true);
  assert.deepEqual(consoleErrors, []);
  console.log(
    `CNA_TS_BROWSER_WASM=PASS FRAMES=${result.frames} UPDATES=${result.updates} ` +
    `ABI=${result.abiVersion} DISPOSED=${result.disposed}`,
  );
} finally {
  await browser.close();
  server.close();
}
