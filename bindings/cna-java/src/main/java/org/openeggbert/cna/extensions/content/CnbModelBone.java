package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;

/**
 * One bone of a compiled model's hierarchy.
 *
 * @param Name the bone's name
 * @param Parent the parent bone's index, or -1 for a root
 * @param Transform the bone's transform, relative to its parent
 */
public record CnbModelBone(String Name, int Parent, Matrix Transform) {
}
