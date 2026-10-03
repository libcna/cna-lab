#!/usr/bin/env node

import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

import ts from "typescript";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const CNA_TS_VERSION = "0.1.0";
const TYPESCRIPT_VERSION = "5.9.2";
const VITE_VERSION = "8.2.2";

function parseArgs(values) {
  const result = { language: null, output: null };
  for (let index = 0; index < values.length; index += 1) {
    const value = values[index];
    if (value === "--language") result.language = values[++index];
    else if (value === "--output") result.output = values[++index];
    else throw new Error(`unknown argument: ${value}`);
  }
  if (!new Set(["typescript", "javascript"]).has(result.language)) {
    throw new Error("--language must be typescript or javascript");
  }
  if (!result.output) throw new Error("--output is required");
  return result;
}

function projectName(directory) {
  const value = path.basename(directory).toLowerCase().replace(/[^a-z0-9._-]+/g, "-");
  return value || "cna-game";
}

function ensureEmpty(directory) {
  if (directory === path.parse(directory).root) throw new Error("refusing to generate at filesystem root");
  if (fs.existsSync(directory) && fs.readdirSync(directory).length > 0) {
    throw new Error(`output directory is not empty: ${directory}`);
  }
  fs.mkdirSync(path.join(directory, "src"), { recursive: true });
}

function transpile(source, fileName) {
  const result = ts.transpileModule(source, {
    fileName,
    reportDiagnostics: true,
    compilerOptions: {
      target: ts.ScriptTarget.ES2022,
      module: ts.ModuleKind.ES2022,
      useDefineForClassFields: true,
      verbatimModuleSyntax: true,
    },
  });
  const errors = (result.diagnostics ?? []).filter(
    (diagnostic) => diagnostic.category === ts.DiagnosticCategory.Error,
  );
  if (errors.length > 0) {
    throw new Error(ts.formatDiagnostics(errors, {
      getCanonicalFileName: (value) => value,
      getCurrentDirectory: () => ROOT,
      getNewLine: () => "\n",
    }));
  }
  return result.outputText;
}

function copySource(directory, language) {
  for (const name of ["HelloGame", "main"]) {
    const source = fs.readFileSync(path.join(ROOT, "src", `${name}.ts`), "utf8");
    const extension = language === "typescript" ? "ts" : "js";
    const output = language === "typescript" ? source : transpile(source, `${name}.ts`);
    fs.writeFileSync(path.join(directory, "src", `${name}.${extension}`), output);
  }
}

function writeProject(directory, language) {
  const typescript = language === "typescript";
  const packageJson = {
    name: projectName(directory),
    version: "0.1.0",
    private: true,
    type: "module",
    scripts: {
      dev: "vite",
      build: typescript ? "tsc -p tsconfig.json && vite build" : "vite build",
      preview: "vite preview",
      ...(typescript ? {} : { smoke: "node smoke.mjs" }),
    },
    dependencies: { "cna-ts": CNA_TS_VERSION },
    devDependencies: typescript
      ? { typescript: TYPESCRIPT_VERSION, vite: VITE_VERSION }
      : { vite: VITE_VERSION },
    engines: { node: "^20.19.0 || >=22.12.0" },
  };
  fs.writeFileSync(path.join(directory, "package.json"), `${JSON.stringify(packageJson, null, 2)}\n`);
  let html = fs.readFileSync(path.join(ROOT, "index.html"), "utf8");
  if (!typescript) html = html.replace("/src/main.ts", "/src/main.js");
  fs.writeFileSync(path.join(directory, "index.html"), html);
  fs.copyFileSync(path.join(ROOT, "vite.config.js"), path.join(directory, "vite.config.js"));
  fs.copyFileSync(path.join(ROOT, "LICENSE"), path.join(directory, "LICENSE"));
  copySource(directory, language);

  if (typescript) {
    fs.copyFileSync(path.join(ROOT, "tsconfig.json"), path.join(directory, "tsconfig.json"));
  } else {
    fs.writeFileSync(
      path.join(directory, "smoke.mjs"),
      `import assert from "node:assert/strict";\n` +
        `import { GameTime, TimeSpan, Vector2 } from "cna-ts";\n` +
        `import { HelloGame } from "./src/HelloGame.js";\n` +
        `const game = new HelloGame();\n` +
        `game.SimulateFrame(new GameTime(TimeSpan.Zero, TimeSpan.FromMilliseconds(16)));\n` +
        `const expected = Vector2.Multiply(new Vector2(48, 32), 0.016);\n` +
        `assert.equal(game.UpdateCount, 1);\n` +
        `assert.ok(Vector2.Distance(game.Position, expected) < 1e-6);\n` +
        `assert.equal(game.ComponentInitializeCount, 1);\n` +
        `assert.equal(game.ComponentUpdateCount, 1);\n` +
        `assert.equal(game.ServiceRoundTrip, true);\n` +
        `assert.equal(game.SpacePressed, true);\n` +
        `assert.ok(Vector2.Distance(game.RotatedUnitX, Vector2.UnitY) < 1e-6);\n` +
        `game.Dispose();\n` +
        `console.log("CNA_TS_JAVASCRIPT_MANAGED_SMOKE=PASS");\n`,
    );
  }

  fs.writeFileSync(
    path.join(directory, "README.md"),
    `# ${projectName(directory)}\n\n` +
      `Generated from the canonical CNA-TS ${language === "typescript" ? "TypeScript" : "JavaScript"} template.\n\n` +
      `The Web bundle and managed XNA canary are build-verified. A real CNA browser runtime remains blocked until a packaged CNA C-ABI WebAssembly module is available. Electron, Android, and iOS are planned rather than claimed supported.\n`,
  );
}

const options = parseArgs(process.argv.slice(2));
const output = path.resolve(options.output);
ensureEmpty(output);
writeProject(output, options.language);
console.log(`GENERATED_LANGUAGE=${options.language}`);
console.log(`GENERATED_OUTPUT=${output}`);
