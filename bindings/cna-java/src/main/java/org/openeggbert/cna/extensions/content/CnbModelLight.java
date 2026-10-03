package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Vector3;

/**
 * One directional light baked into a compiled model.
 *
 * @param Direction the direction the light travels in
 * @param DiffuseColor its diffuse colour, as linear red, green and blue
 */
public record CnbModelLight(Vector3 Direction, Vector3 DiffuseColor) {
}
