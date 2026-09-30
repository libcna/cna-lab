// SPDX-License-Identifier: MS-PL

/**
 * The oracles, checked against evidence that should fail them.
 *
 * Every browser claim in this package is an oracle applied to evidence a page produced, so an
 * oracle that accepts anything makes every suite that uses it green and meaningless. The mutation
 * harness cannot catch that: it plants a defect, rebuilds `dist`, and compares artifacts, and an
 * oracle lives in `test/support`, which `dist` does not contain — so a mutant in one leaves the
 * build byte-identical and the harness correctly refuses to score it.
 *
 * This is the check that does fit: give each oracle evidence with exactly one thing wrong and
 * require it to say so. Not a mutation of the binding — a mutation of the *evidence*, which is what
 * an oracle is for.
 */

import assert from "node:assert/strict";
import test from "node:test";

import {
  EXTENSION_CLASSES, asciiCell, assertAsciiEvidence, assertExtensionCensus,
} from "./support/extension-oracle.mjs";

/** A census that should pass, which each case below then breaks in one place. */
function census() {
  return {
    rows: EXTENSION_CLASSES.map((name) => ({
      name, constructed: true, failures: [], read: 4, wrote: 1, roundTripped: 1,
    })),
  };
}

/** ASCII evidence that should pass: what `AsciiQuantizer.cpp` predicts for the page's source. */
function ascii() {
  const N = 8;
  const source = [[40, 0, 0], [0, 20, 0], [0, 0, 80], [255, 255, 255]];
  const cells = source.map(asciiCell);
  const frame = (cellSize, grid, texel) => ({
    cellSize, grid,
    pixels: Array.from({ length: N * N }, (_, index) => texel(index % N, Math.trunc(index / N))),
  });
  const colour = frame([4, 4], [2, 2], (x, y) => {
    const quadrant = (y < 4 ? 0 : 2) + (x < 4 ? 0 : 1);
    if (quadrant < 3) return cells[quadrant].background;
    return (x + y) % 2 === 0 ? cells[3].foreground : cells[3].background;
  });
  const average = [0, 1, 2].map((channel) =>
    Math.trunc(source.reduce((sum, q) => sum + q[channel] * 16, 0) / 64));
  const collapsedCell = asciiCell(average);
  return {
    source,
    mode: 1,
    colour,
    collapsed: frame([8, 8], [1, 1], () => collapsedCell.background),
    oblong: frame([2, 4], [4, 2], () => [0, 0, 0, 255]),
    blackWhiteMode: 0,
    blackWhite: frame([4, 4], [2, 2], (x, y) => ((x + y) % 2 === 0 ? [255, 255, 255, 255] : [0, 0, 0, 255])),
  };
}

test("the oracles accept the evidence a working backend produces", () => {
  const totals = assertExtensionCensus(census());
  assert.equal(totals.classes, EXTENSION_CLASSES.length);
  assert.equal(totals.refused, 0);
  assertAsciiEvidence(ascii());
});

test("a default artifact's refusals are accepted when they are CNA's own", () => {
  const refused = census();
  for (const row of refused.rows) Object.assign(row, { constructed: false, cnaResult: 6, error: "no layer" });
  assert.equal(assertExtensionCensus(refused, { requireAll: false }).refused, EXTENSION_CLASSES.length);
});

/**
 * One broken thing per case, and the oracle has to find it.
 *
 * Each entry names what a real binding defect would look like in the evidence.
 */
const CASES = [
  ["a class that did not construct on an artifact that has the layer", () => {
    const broken = census();
    Object.assign(broken.rows[0], { constructed: false, cnaResult: 6, error: "refused" });
    return () => assertExtensionCensus(broken);
  }],
  ["a class refused by the binding rather than by CNA", () => {
    const broken = census();
    Object.assign(broken.rows[2], { constructed: false, cnaResult: 0, error: "binding" });
    return () => assertExtensionCensus(broken, { requireAll: false });
  }],
  ["an accessor that does not marshal", () => {
    const broken = census();
    broken.rows[3].failures = ["Width threw"];
    return () => assertExtensionCensus(broken);
  }],
  ["a census that read nothing", () => {
    const broken = census();
    for (const row of broken.rows) row.read = 0;
    return () => assertExtensionCensus(broken);
  }],
  ["a census that skipped a class", () => {
    const broken = census();
    broken.rows.pop();
    return () => assertExtensionCensus(broken);
  }],
  ["an ASCII grid read with its axes swapped", () => {
    const broken = ascii();
    broken.oblong.grid = [2, 4];
    return () => assertAsciiEvidence(broken);
  }],
  ["an ASCII dark cell that is not its background", () => {
    const broken = ascii();
    broken.colour.pixels[0] = [40, 0, 0, 255];
    return () => assertAsciiEvidence(broken);
  }],
  ["a BlackWhite mode that kept the hue", () => {
    const broken = ascii();
    broken.blackWhite.pixels[0] = [255, 0, 0, 255];
    return () => assertAsciiEvidence(broken);
  }],
];

for (const [name, build] of CASES) {
  test(`the oracles reject ${name}`, () => {
    assert.throws(build(), assert.AssertionError, `${name} was accepted`);
  });
}
