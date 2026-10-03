/**
 * CNA's extended graphics layer, projected for Java.
 *
 * <p>Nothing in this package is part of Microsoft XNA 4.0. It is CNA's own capability --
 * physically based effects, shader effects, the ASCII, CRT, depth and colour-matrix effects, the
 * debug line renderer and the renderer capability queries -- and it lives here rather than in
 * {@code Microsoft.Xna.Framework} for exactly that reason: the strict packages stay a faithful
 * XNA projection, and a game that only wants XNA never sees any of this. CNA ABI 0.30 retired
 * the engine layer (render pipeline, post-process chain, shadows, clustered lighting, probes,
 * particles, compute and instancing), and this package no longer projects it.
 *
 * <p>The extended layer is an opt-in CNA build option. Its declarations exist in every build so
 * the exported ABI never changes shape, and the routes that need a native extension object
 * answer {@code NOT_SUPPORTED} when the layer is absent. {@link
 * org.openeggbert.cna.extensions.graphics.GraphicsExtension#isAvailable()} reports which build
 * is loaded; the pure value operations here work either way.
 */
package org.openeggbert.cna.extensions.graphics;
