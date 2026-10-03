package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector3;

/**
 * One endpoint of a queued debug line.
 *
 * @param Position where the endpoint is, in world space
 * @param Color the colour the line was queued with
 */
public record DebugVertex(Vector3 Position, Color Color) {
}
