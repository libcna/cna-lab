// SPDX-License-Identifier: MS-PL
//
// The small marshalling shapes the families share.
//
// Each of these is three lines, and each was about to be written for the eleventh time: eleven
// copies of "allocate four bytes, call, read byte zero" are eleven chances to read the wrong width.

import { allocateStruct, type WasmRouteTable } from "./module.js";

/**
 * Calls a route whose last parameter is a `CNA_Bool*` output.
 *
 * `CNA_Bool` is a `uint8_t`, and the four bytes allocated for it are deliberate: an out-parameter
 * smaller than a word is still written into a word-aligned slot by every compiler here, and
 * reading byte zero of a zeroed four is correct whichever width CNA chooses.
 */
export function outBool(
  routes: WasmRouteTable, route: string, ...args: readonly (number | bigint)[]
): boolean {
  const scope = routes.scope();
  try {
    const out = scope.allocate(4);
    routes.invoke(route, ...args, out);
    return routes.view().getUint8(out) !== 0;
  } finally {
    scope.dispose();
  }
}

/** Calls a route whose last parameter is an `int32_t*` output. */
export function outI32(
  routes: WasmRouteTable, route: string, ...args: readonly (number | bigint)[]
): number {
  const scope = routes.scope();
  try {
    const out = scope.allocate(4);
    routes.invoke(route, ...args, out);
    return routes.view().getInt32(out, true);
  } finally {
    scope.dispose();
  }
}

/** Calls a route whose last parameter is an `int64_t*` output. */
export function outI64(
  routes: WasmRouteTable, route: string, ...args: readonly (number | bigint)[]
): bigint {
  const scope = routes.scope();
  try {
    const out = scope.allocate(8);
    routes.invoke(route, ...args, out);
    return routes.view().getBigInt64(out, true);
  } finally {
    scope.dispose();
  }
}

/** Calls a route whose last parameter is a `float*` output. */
export function outF32(
  routes: WasmRouteTable, route: string, ...args: readonly (number | bigint)[]
): number {
  const scope = routes.scope();
  try {
    const out = scope.allocate(4);
    routes.invoke(route, ...args, out);
    return routes.view().getFloat32(out, true);
  } finally {
    scope.dispose();
  }
}

/**
 * Builds a `CNA_StringView` in module memory and runs `body` with its address.
 *
 * wasm32 lowers a by-value structure argument as a pointer to a caller-owned copy, which is what
 * makes this a pointer rather than two arguments. The copy lives exactly as long as the call.
 */
export function withStringView<T>(
  routes: WasmRouteTable, value: string, body: (pointer: number) => T,
): T {
  const scope = routes.scope();
  try {
    const { pointer, byteLength } = scope.allocateUtf8(value);
    const view = allocateStruct(routes.module, scope, "CNA_StringView", false);
    view.setPointer("data", pointer);
    view.setU64("byte_length", BigInt(byteLength));
    return body(view.pointer);
  } finally {
    scope.dispose();
  }
}
