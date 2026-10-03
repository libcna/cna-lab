package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;

/**
 * One part's material factors.
 *
 * <p>glTF's metallic-roughness model, which is the vocabulary CNA compiles into. The textures
 * that go with these factors are separate, because a slot can be empty and a factor never is:
 * see {@link CnbModelData#getMaterialTexture}.
 *
 * @param BaseColorFactor the albedo tint, red through alpha
 * @param EmissiveFactor the emissive colour
 * @param SpecularColorFactor the specular tint
 * @param MetallicFactor how metallic the surface is, zero to one
 * @param RoughnessFactor how rough the surface is, zero to one
 * @param Ior the index of refraction
 * @param SpecularFactor the specular strength
 * @param NormalScale how strongly the normal map is applied
 * @param OcclusionStrength how strongly the occlusion map is applied
 * @param AlphaCutoff the threshold {@link CnbAlphaMode#Mask} compares against
 * @param AlphaMode how alpha is treated
 * @param DoubleSided whether back faces are drawn
 */
public record CnbMaterial(
        Vector4 BaseColorFactor,
        Vector3 EmissiveFactor,
        Vector3 SpecularColorFactor,
        float MetallicFactor,
        float RoughnessFactor,
        float Ior,
        float SpecularFactor,
        float NormalScale,
        float OcclusionStrength,
        float AlphaCutoff,
        CnbAlphaMode AlphaMode,
        boolean DoubleSided) {
}
