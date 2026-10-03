#!/usr/bin/env node

import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const temporary = fs.mkdtempSync(path.join(os.tmpdir(), "cna-ts-template-"));

function parseTarball(values) {
  for (let index = 0; index < values.length; index += 1) {
    if (values[index] === "--package") return path.resolve(values[index + 1]);
  }
  if (process.env.CNA_TS_TARBALL) return path.resolve(process.env.CNA_TS_TARBALL);
  throw new Error("pass --package /path/to/cna-ts.tgz or set CNA_TS_TARBALL");
}

function run(command, args, cwd) {
  const result = spawnSync(command, args, { cwd, encoding: "utf8" });
  if (result.status !== 0) {
    throw new Error(`${command} ${args.join(" ")} failed\n${result.stdout}${result.stderr}`);
  }
  return result.stdout.trim();
}

function npmCommand() {
  if (process.env.npm_execpath?.endsWith(".js") && fs.existsSync(process.env.npm_execpath)) {
    return { command: process.execPath, prefix: [process.env.npm_execpath] };
  }
  const embedded = path.resolve(path.dirname(process.execPath), "../lib/node_modules/npm/bin/npm-cli.js");
  if (fs.existsSync(embedded)) return { command: process.execPath, prefix: [embedded] };
  return { command: "npm", prefix: [] };
}

function install(directory, tarball) {
  const npm = npmCommand();
  run(
    npm.command,
    [
      ...npm.prefix,
      "install",
      tarball,
      "--no-save",
      "--ignore-scripts",
      "--no-audit",
      "--no-fund",
      "--cache",
      path.join(temporary, "npm-cache"),
    ],
    directory,
  );
}

function sourceFiles(directory) {
  const result = [];
  for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const file = path.join(directory, entry.name);
    if (entry.isDirectory()) result.push(...sourceFiles(file));
    else result.push(file);
  }
  return result;
}

function auditGenerated(directory, language) {
  const files = sourceFiles(directory);
  const text = files.map((file) => fs.readFileSync(file, "utf8")).join("\n");
  for (const forbidden of ["../cna-ts", "../cna-js", "@openeggbert/cna-js", ROOT]) {
    if (text.includes(forbidden)) throw new Error(`generated ${language} project contains ${forbidden}`);
  }
  const packageJson = JSON.parse(fs.readFileSync(path.join(directory, "package.json"), "utf8"));
  if (packageJson.dependencies?.["cna-ts"] !== "0.1.0") {
    throw new Error(`generated ${language} project does not pin cna-ts 0.1.0`);
  }
  if (language === "javascript") {
    if (files.some((file) => file.endsWith(".ts"))) throw new Error("JavaScript project contains TypeScript source");
    if (packageJson.devDependencies?.typescript) throw new Error("JavaScript project depends on TypeScript");
  }
}

const tarball = parseTarball(process.argv.slice(2));
if (!fs.existsSync(tarball)) throw new Error(`missing package tarball: ${tarball}`);

try {
  for (const language of ["typescript", "javascript"]) {
    const directory = path.join(temporary, language);
    run(
      process.execPath,
      [path.join(ROOT, "tools/create-project.mjs"), "--language", language, "--output", directory],
      ROOT,
    );
    auditGenerated(directory, language);
    install(directory, tarball);
    if (language === "typescript") {
      run(process.execPath, [path.join(directory, "node_modules/typescript/bin/tsc"), "-p", "tsconfig.json"], directory);
    }
    run(process.execPath, [path.join(directory, "node_modules/vite/bin/vite.js"), "build"], directory);
    if (!fs.existsSync(path.join(directory, "dist/index.html"))) throw new Error(`${language} bundle is absent`);
    if (language === "javascript") run(process.execPath, ["smoke.mjs"], directory);
  }
  console.log(`PACKED_CNA_TS=${path.basename(tarball)}`);
  console.log("GENERATED_TYPESCRIPT_BUILD=PASS");
  console.log("GENERATED_JAVASCRIPT_BUILD=PASS");
  console.log("GENERATED_JAVASCRIPT_MANAGED_SMOKE=PASS");
  console.log("LEGACY_OR_SIBLING_REFERENCES=0");
} finally {
  fs.rmSync(temporary, { recursive: true, force: true });
}
