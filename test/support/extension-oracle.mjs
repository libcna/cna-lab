// SPDX-License-Identifier: MS-PL

/**
 * The oracles for CNA's remaining graphics extensions in a browser: a reachability census of every
 * public extension class, and the ASCII quantizer, whose every texel is predictable from
 * `AsciiQuantizer.cpp` alone.
 *
 * What the census claims is marshalling, not semantics: that each class constructs, that every
 * accessor on it can be read, and that every settable one survives a write. The ASCII evidence is
 * the semantics half, asserted texel by texel.
 */

import assert from "node:assert/strict";

/** The public extension classes the page constructs, in its order. */
export const EXTENSION_CLASSES = Object.freeze([
  "PbrEffect", "SkinnedPbrEffect", "CrtEffect", "DepthEffect", "AsciiPostProcessEffect", "DebugDraw",
]);

/**
 * Asserts the census.
 *
 * `requireAll` is what a strong-artifact run requires: every class constructs. An artifact built
 * without `CNA_CNAEXT` refuses them instead, and the ordinary suite records that rather than
 * requiring it -- but either way every refusal must be CNA's own NOT_SUPPORTED, never the binding's.
 */
export function assertExtensionCensus(census, { requireAll = true } = {}) {
  assert.equal(census.censusError ?? null, null, `the census itself failed: ${census.stack ?? ""}`);
  assert.deepEqual(
    census.rows.map((row) => row.name), [...EXTENSION_CLASSES],
    "the census covers every public extension class",
  );
  let read = 0, wrote = 0, roundTripped = 0, refused = 0;
  for (const row of census.rows) {
    if (!row.constructed) {
      assert.ok(!requireAll, `${row.name} would not construct: ${row.error}`);
      assert.equal(
        row.cnaResult, 6,
        `${row.name} must be refused by CNA's own NOT_SUPPORTED rather than by the binding: ${row.error}`,
      );
      refused += 1;
      continue;
    }
    assert.deepEqual(row.failures, [], `${row.name} has members that do not marshal: ${row.failures.join("; ")}`);
    read += row.read;
    wrote += row.wrote;
    roundTripped += row.roundTripped;
  }
  if (requireAll) {
    // And it exercised something: a run that constructed everything and read nothing would
    // satisfy every assertion above.
    assert.ok(read >= EXTENSION_CLASSES.length, `the census read accessors on every class (${read})`);
    assert.ok(wrote >= 2, `and wrote some (${wrote})`);
    assert.ok(roundTripped > 0 && roundTripped <= wrote, `and some survived (${roundTripped} of ${wrote})`);
  }
  return { classes: census.rows.length, read, wrote, roundTripped, refused };
}

/** `AsciiQuantizer.cpp`'s glyph index: a ten-character ramp whose first character is a space. */
export function asciiGlyphIndex(luminance0to255) {
  const index = Math.trunc(luminance0to255 / 255 * 9 + 0.5);
  return Math.min(Math.max(index, 0), 9);
}

/** The same file's luminance and Color-mode background, both integer-truncating as C++ does. */
export function asciiCell([r, g, b]) {
  const luminance = 0.299 * r + 0.587 * g + 0.114 * b;
  return {
    glyphIndex: asciiGlyphIndex(luminance),
    foreground: [r, g, b, 255],
    background: [Math.trunc(r / 4), Math.trunc(g / 4), Math.trunc(b / 4), 255],
  };
}

/**
 * The ASCII quantizer, texel by texel.
 *
 * Three of the four source quadrants are dark enough that their glyph index rounds to zero, and
 * index zero of `" .:-=+*#%@"` is a space with no lit pixels -- so those cells are exactly their
 * background and a single colour covers all sixteen texels. The white quadrant is the only one
 * with two colours in it, and both are named.
 */
export function assertAsciiEvidence(ascii) {
  const N = 8;
  const cells = ascii.source.map(asciiCell);
  // The premise the per-texel expectations rest on, asserted rather than assumed: if CNA ever
  // changed the ramp or the luminance weights, this fails here and says so, instead of failing as
  // a wall of unexplained texel differences below.
  assert.deepEqual(
    cells.map((cell) => cell.glyphIndex), [0, 0, 0, 9],
    "three quadrants quantize to the ramp's space and the white one to its last character",
  );

  // Cell size and grid, before any texel: a size read as one number, or transposed, dies here.
  assert.deepEqual(ascii.colour.cellSize, [4, 4]);
  assert.deepEqual(ascii.colour.grid, [2, 2], "an 8x8 source in 4x4 cells is a 2x2 grid");
  assert.deepEqual(ascii.collapsed.cellSize, [8, 8]);
  assert.deepEqual(ascii.collapsed.grid, [1, 1], "one cell as large as the source is one cell");
  assert.deepEqual(ascii.oblong.cellSize, [2, 4]);
  assert.deepEqual(
    ascii.oblong.grid, [4, 2],
    "a 2-wide 4-tall cell gives four columns and two rows; a transposed pair gives two and four",
  );

  const texel = (frame, x, y) => frame.pixels[y * N + x];
  // The three dark quadrants: every texel is the background and nothing else.
  const quadrantOrigin = [[0, 0], [4, 0], [0, 4], [4, 4]];
  for (const index of [0, 1, 2]) {
    const [originX, originY] = quadrantOrigin[index];
    for (let y = originY; y < originY + 4; y += 1) {
      for (let x = originX; x < originX + 4; x += 1) {
        assert.deepEqual(
          texel(ascii.colour, x, y), cells[index].background,
          `quadrant ${index} texel (${x}, ${y}) is the cell background, which is the average / 4`,
        );
      }
    }
  }
  // The white quadrant: two colours, both of them named, and both actually present.
  const white = cells[3];
  const seen = new Set();
  for (let y = 4; y < 8; y += 1) {
    for (let x = 4; x < 8; x += 1) {
      const value = texel(ascii.colour, x, y);
      const isForeground = value.every((c, i) => c === white.foreground[i]);
      const isBackground = value.every((c, i) => c === white.background[i]);
      assert.ok(
        isForeground || isBackground,
        `the lit quadrant's texel (${x}, ${y}) is ${JSON.stringify(value)}, which is neither its ` +
        `foreground ${JSON.stringify(white.foreground)} nor its background ` +
        `${JSON.stringify(white.background)}`,
      );
      seen.add(isForeground ? "foreground" : "background");
    }
  }
  assert.deepEqual([...seen].sort(), ["background", "foreground"],
    "the '@' glyph paints some of the cell and leaves the rest as background");

  // One cell over the whole source: its average is the average of the four quadrants, and its
  // background is that average over four. This is the assertion a per-cell average that summed
  // the wrong region fails.
  const average = [0, 1, 2].map((channel) =>
    Math.trunc(ascii.source.reduce((sum, q) => sum + q[channel] * 16, 0) / 64));
  const collapsed = asciiCell(average);
  const distinct = new Set(ascii.collapsed.pixels.map((p) => p.join(",")));
  for (const value of distinct) {
    const parsed = value.split(",").map(Number);
    const isForeground = parsed.every((c, i) => c === collapsed.foreground[i]);
    const isBackground = parsed.every((c, i) => c === collapsed.background[i]);
    assert.ok(
      isForeground || isBackground,
      `the collapsed cell contains ${value}; its average is ${average} so its only legal colours ` +
      `are ${collapsed.foreground} and ${collapsed.background}`,
    );
  }

  // BlackWhite mode: the same source, and a palette that no longer carries the scene's colour.
  assert.equal(ascii.mode, 1, "Color is the default quantize mode");
  assert.equal(ascii.blackWhiteMode, 0, "and the mode was changed");
  for (const pixel of ascii.blackWhite.pixels) {
    const [r, g, b] = pixel;
    assert.ok(r === g && g === b, `BlackWhite mode leaves no hue, and this texel is ${pixel}`);
  }
  assert.notDeepEqual(
    ascii.blackWhite.pixels, ascii.colour.pixels,
    "and the mode reached CNA rather than being stored managed",
  );
}
