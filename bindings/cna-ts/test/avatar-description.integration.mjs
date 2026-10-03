// SPDX-License-Identifier: MS-PL
//
// `AvatarDescription`, which needs no gamer service.
//
// The rest of the avatar surface refuses, and rightly: a renderer needs avatar assets and a
// signed-in gamer this host does not have. A *description* needs neither, so it is projected.
//
// `CreateRandom` draws from CNA's avatar catalog. At ABI 0.21 CNA returned XNA-for-Windows' all-zero,
// invalid description and ignored the body type; CNA's gamer-services work replaced that with
// real descriptions, and the body-type overload now keeps its argument. This binding used to drop
// the argument on the grounds that it could not change the answer -- it can now, so it is sent.

import assert from "node:assert/strict";
import path from "node:path";
import test from "node:test";

import { GamerServices, LoadNodeNativeBackend } from "../dist/index.js";
import { CNA_ABI_MAJOR, CNA_ABI_MINOR } from "../dist/internal/abi.js";
import { assertAvatarEvidence } from "./support/non-engine-oracle.mjs";

const library = process.env.CNA_NATIVE_LIBRARY;
if (!library) {
  throw new Error(
    `CNA_NATIVE_LIBRARY must name an existing CNA C ABI ${CNA_ABI_MAJOR}.${CNA_ABI_MINOR}.x shared library`,
  );
}

await LoadNodeNativeBackend({
  CnaLibrary: path.resolve(library),
  BridgeModule: path.resolve(process.env.CNA_NODE_BRIDGE ?? "build/cna_node_bridge.node"),
});

const { AvatarBodyType, AvatarDescription } = GamerServices;

/** CNA's own constant, and the length the canonical constructor requires. */
const DESCRIPTION_BYTES = 1021;

/** The same evidence shape the browser page produces, so both backends face one oracle. */
function avatarEvidence() {
  const random = AvatarDescription.CreateRandom();
  const female = AvatarDescription.CreateRandom(AvatarBodyType.Female);
  const male = AvatarDescription.CreateRandom(AvatarBodyType.Male);
  const rebuilt = new AvatarDescription(random.Description);
  let shortRefused = "ACCEPTED";
  try { new AvatarDescription(new Array(10).fill(0)); }
  catch (error) { shortRefused = error?.constructor?.name ?? "unknown"; }
  let badBodyType = "ACCEPTED";
  try { AvatarDescription.CreateRandom(99); }
  catch (error) { badBodyType = error?.constructor?.name ?? "unknown"; }
  const draws = Array.from({ length: 8 }, () => AvatarDescription.CreateRandom().Description.join(","));
  return {
    length: random.Description.length,
    isValid: random.IsValid,
    bodyType: random.BodyType,
    height: random.Height,
    distinct: new Set(draws).size,
    femaleBodyType: female.BodyType,
    maleBodyType: male.BodyType,
    roundTrip: rebuilt.Description.every((byte, at) => byte === random.Description[at]),
    rebuiltValid: rebuilt.IsValid,
    rebuiltBodyType: rebuilt.BodyType,
    shortRefused,
    badBodyType,
  };
}

test("the Node backend's avatar evidence satisfies the shared oracle", () => {
  assertAvatarEvidence(avatarEvidence());
});

test("CreateRandom draws a valid description from CNA's catalog", () => {
  const first = AvatarDescription.CreateRandom();
  const second = AvatarDescription.CreateRandom();
  for (const description of [first, second]) {
    assert.equal(
      description.Description.length, DESCRIPTION_BYTES,
      "a description is exactly one description's worth of bytes",
    );
    assert.equal(description.IsValid, true, "and it is a usable avatar");
    assert.ok(description.Height > 1 && description.Height < 2.5, `height ${description.Height}`);
  }
});

test("the bodyType overload validates its argument and keeps it", () => {
  // Both halves: a projection that stopped validating, or one that dropped the argument (as this
  // one did while CNA ignored it), fails here.
  for (let attempt = 0; attempt < 8; attempt += 1) {
    assert.equal(AvatarDescription.CreateRandom(AvatarBodyType.Female).BodyType, AvatarBodyType.Female);
    assert.equal(AvatarDescription.CreateRandom(AvatarBodyType.Male).BodyType, AvatarBodyType.Male);
  }
});

test("a description round-trips through its own bytes", () => {
  const original = AvatarDescription.CreateRandom(AvatarBodyType.Female);
  const rebuilt = new AvatarDescription(original.Description);
  assert.deepEqual(
    rebuilt.Description, original.Description,
    "the bytes survive being handed back to the constructor",
  );
  assert.equal(
    rebuilt.BodyType, original.BodyType,
    "and the body type is read out of the bytes by CNA rather than defaulted here -- the " +
    "constructor used to report Male for any input at all, including none",
  );
  assert.equal(rebuilt.IsValid, original.IsValid);
  assert.notEqual(
    rebuilt.Description, original.Description,
    "Description hands back a copy, so a caller cannot edit the description in place",
  );
});

test("Description is a copy each time it is read", () => {
  const description = AvatarDescription.CreateRandom();
  const first = description.Description;
  first[0] = (first[0] + 1) & 0xff;
  assert.notDeepEqual(
    first, description.Description,
    "editing what one read returns must not change what the next read returns",
  );
});

test("a wrong-length description is refused rather than padded", () => {
  for (const length of [0, 1, DESCRIPTION_BYTES - 1, DESCRIPTION_BYTES + 1]) {
    assert.throws(
      () => new AvatarDescription(new Array(length).fill(0)),
      (error) => error != null,
      `a ${length}-byte description must be refused, not accepted and silently resized`,
    );
  }
  assert.doesNotThrow(
    () => new AvatarDescription(new Array(DESCRIPTION_BYTES).fill(0)),
    "and exactly one description's worth of bytes is accepted",
  );
});

test("an all-zero description is well-formed but not valid", () => {
  const zeroed = new AvatarDescription(new Array(DESCRIPTION_BYTES).fill(0));
  assert.equal(zeroed.Description.length, DESCRIPTION_BYTES);
  assert.equal(
    zeroed.IsValid, false,
    "validity is decided by the description's first byte, not by whether there are any bytes -- " +
    "which is what IsValid used to answer",
  );
  assert.equal(
    zeroed.BodyType, AvatarBodyType.Female,
    "and it still reports a body type, which comes from CNA reading the bytes",
  );
});

test("a body type that is not one is refused by name", () => {
  for (const bad of [99, -1, 2]) {
    assert.throws(
      () => AvatarDescription.CreateRandom(bad),
      (error) => error?.constructor?.name === "ArgumentOutOfRangeException",
      `${bad} is not an AvatarBodyType, and XNA refuses it by that name`,
    );
  }
});

test("what still needs a gamer still refuses", () => {
  // Nothing here fabricates a service: reading an avatar off a gamer needs a gamer, and this host
  // has none.
  assert.throws(
    () => AvatarDescription.BeginGetFromGamer(null, null, null),
    (error) => error?.constructor?.name === "GamerServicesNotAvailableException",
  );
});
