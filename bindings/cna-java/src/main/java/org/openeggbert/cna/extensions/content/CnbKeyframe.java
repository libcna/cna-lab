package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;

/**
 * One bone's pose at one moment of an animation clip.
 *
 * @param TimeSeconds when in the clip this pose applies
 * @param Translation the bone's position
 * @param Rotation the bone's orientation
 * @param Scale the bone's scale
 */
public record CnbKeyframe(
        double TimeSeconds, Vector3 Translation, Quaternion Rotation, Vector3 Scale) {
}
