// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Viewport/CnaModelPass.hpp"

#include <cstdint>
#include <exception>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <optional>
#include <vector>

#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "CNA/Studio/Scene/SceneModels.hpp"

#include "Microsoft/Xna/Framework/Graphics/AlphaModeEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectFog.hpp"
#include "Microsoft/Xna/Framework/Graphics/IShadowReceiverEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PunctualLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"

#include "CNA/Graphics/DirectionalLightEXT.hpp"
#include "CNA/Graphics/ShadowMap.hpp"
#include "CNA/Graphics/ShadowQuality.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"

namespace Xna = Microsoft::Xna::Framework;
namespace XnaGraphics = Microsoft::Xna::Framework::Graphics;

namespace CNA::Studio
{
    namespace
    {
        /**
         * @brief The cull mode that matches what the importer produces, after the Y mirror.
         *
         * Named rather than written at the call site because the reasoning is two steps and both
         * are easy to get backwards: the importer winds triangles counter-clockwise seen from
         * *outside in world space*, the view-projection mirrors Y, and mirroring reverses apparent
         * winding -- so an outward face reaches the rasteriser clockwise, and culling the
         * counter-clockwise ones keeps exactly the faces that should be seen. If models ever come
         * out hollow, this constant is the first thing to try, and the header says so too.
         */
        constexpr XnaGraphics::CullMode kOutwardFaces = XnaGraphics::CullMode::CullCounterClockwiseFace;

        /**
         * @brief Whether to draw through `PbrEffect`. **False, and the reason is a measurement.**
         *
         * The owner chose PBR with a `BasicEffect` fallback, and the PBR path below is written and
         * complete. It is off because on the one backend this repository can photograph -- EASYGL
         * under Xvfb -- `PbrEffect` draws *nothing at all*: it constructs without throwing, accepts
         * every parameter, reports its draw calls, and puts no pixels on screen, while
         * `BasicEffect` renders the same geometry, matrices and lights correctly. That is CNA gap
         * G-05 in NEXT.md, along with the two things ruled out on the way (backface culling, and
         * an unbound base-colour sampler, which is a real second bug in `FillGpuDrawParams`).
         *
         * So this is not a preference between two working paths -- it is one working path and one
         * that has been verified not to. Flipping this to `true` is how the PBR path is re-tested
         * when CNA's changes; nothing else has to move.
         */
        constexpr bool kPreferPbrEffect = false;

        /**
         * @brief The shadow map's resolution, as CNA spells it (`plan.md` STUDIO-20006).
         *
         * Medium, which is 1024 square with a 1-texel filter. Chosen rather than defaulted: the
         * depth bias CNA ships is documented as tuned for this size, so anything else trades a
         * sharper shadow for one that either stripes its own caster or floats away from it, and
         * neither is a trade an editor should make on a user's behalf without offering the choice.
         * When STUDIO-20006 grows a quality setting, this constant is what it replaces.
         */
        constexpr CNA::Graphics::ShadowQuality kShadowQuality = CNA::Graphics::ShadowQuality::Medium;

        /**
         * @brief Converts one of the editor's matrices to XNA's.
         *
         * Field for field and nothing else, because `StudioMatrix` was written to mirror
         * `Microsoft::Xna::Framework::Matrix` exactly (ED-400). This function existing at all is
         * the price of `cna-studio-core` not being allowed to name a CNA type -- not a conversion
         * of conventions, which is what makes it safe to read past.
         */
        Xna::Matrix toXna(const StudioMatrix& matrix)
        {
            Xna::Matrix result;
            result.M11 = matrix.m11; result.M12 = matrix.m12; result.M13 = matrix.m13; result.M14 = matrix.m14;
            result.M21 = matrix.m21; result.M22 = matrix.m22; result.M23 = matrix.m23; result.M24 = matrix.m24;
            result.M31 = matrix.m31; result.M32 = matrix.m32; result.M33 = matrix.m33; result.M34 = matrix.m34;
            result.M41 = matrix.m41; result.M42 = matrix.m42; result.M43 = matrix.m43; result.M44 = matrix.m44;
            return result;
        }

        Xna::Vector3 toXna(const StudioVector3& vector)
        {
            return Xna::Vector3{vector.x, vector.y, vector.z};
        }
    }

    struct CnaModelPass::Impl
    {
        XnaGraphics::GraphicsDevice* device = nullptr;
        const AssetDatabase* assets = nullptr;

        /**
         * @brief Whichever effect this build got. Exactly one of the two is non-null.
         *
         * Two members rather than a base-class pointer: `IEffectMatrices` and `IEffectLights` are
         * the shared interfaces and they do not cover the material properties, which are where
         * the two effects genuinely differ. A pass that reached them through the interfaces alone
         * would be a PBR pass that could not set metallic.
         */
        std::unique_ptr<XnaGraphics::PbrEffect> pbr;
        std::unique_ptr<XnaGraphics::BasicEffect> basic;
        std::string effectName{"none"};

        /**
         * @brief The scene's shadow map, or null where this build cannot generate one.
         *
         * `plan.md` STUDIO-20006. Constructed on the first pass that needs it rather than in
         * `initialize`: a 2D project never asks for one, and a render target of up to 2048 square
         * is not worth holding for a viewport that will never draw into it.
         */
        std::unique_ptr<CNA::Graphics::ShadowMap> shadows;

        /** @brief True while @ref shadows holds a map generated for the frame being drawn. */
        bool shadowsLive = false;

        /**
         * @brief Set once this device has been found unable to shadow, so it is asked only once.
         *
         * `ShadowMap`'s constructor allocates a render target of up to 2048 square *before*
         * `isSupported` can be asked, so a pass that reconstructed it every frame to re-ask would
         * allocate and free that target sixty times a second on exactly the renderers that can do
         * nothing with it.
         */
        bool shadowsRefused = false;

        /** @brief Draws put into the map by the last @ref renderShadowMap, for the stats. */
        std::size_t shadowCasters = 0;

        /** @brief One upload per model asset, drawn once per entity that names it. */
        struct GpuPart
        {
            std::unique_ptr<XnaGraphics::VertexBuffer> vertices;
            std::unique_ptr<XnaGraphics::IndexBuffer> indices;
            int vertexCount = 0;
            int triangleCount = 0;
            int materialIndex = -1;

            /**
             * @brief The mesh part's name, kept so ED-410's per-part overrides can be matched.
             *
             * Copied at upload rather than looked up in the `MeshData` at draw time: the buffers
             * are cached per asset and the parts that produced them may have been skipped, so the
             * index into `mesh.parts` and the index into `parts` here are not the same number.
             */
            std::string name;
        };

        struct GpuModel
        {
            std::vector<GpuPart> parts;
        };

        std::unordered_map<Uuid, std::unique_ptr<GpuModel>> models;

        /** @brief Textures a material named, resolved by path against the database. */
        std::unordered_map<std::string, std::shared_ptr<XnaGraphics::Texture2D>> textures;
        std::unordered_set<std::string> failedTextures;

        /**
         * @brief One white texel, bound wherever a PBR material names no base-colour texture.
         *
         * Not decoration and not a placeholder for a missing file -- it works around a real gap in
         * CNA (G-05). `PbrEffect::FillGpuDrawParams` sets `textureEnabled = true` unconditionally
         * while binding `texture0` only when a texture was given, so a material with a base-colour
         * *factor* and no map -- which is most hand-authored glTF, including this repository's own
         * example crate -- reaches the shader sampling an unbound texture and renders black.
         * `BasicEffect` has a `TextureEnabled` flag and does not have the problem.
         *
         * White is also the arithmetically correct answer rather than a trick: glTF multiplies the
         * base-colour factor by the base-colour texture, and the identity for that multiply is 1.
         */
        std::shared_ptr<XnaGraphics::Texture2D> whiteTexel;

        /** @brief Uploads @p mesh, or returns what was uploaded before. */
        GpuModel* resolveModel(const Uuid& assetId, const MeshData& mesh, ModelPassStats& stats)
        {
            if (const auto found = models.find(assetId); found != models.end())
            {
                return found->second.get();
            }

            auto model = std::make_unique<GpuModel>();

            for (const MeshPart& part : mesh.parts)
            {
                if (part.vertices.empty() || part.indices.size() < 3) { continue; }

                // The copy `MeshData` was shaped to make cheap: field for field, so this is a
                // conversion of types rather than of meaning (see MeshData.hpp's third decision).
                std::vector<XnaGraphics::VertexPositionNormalTexture> vertices;
                vertices.reserve(part.vertices.size());
                for (const MeshVertex& vertex : part.vertices)
                {
                    XnaGraphics::VertexPositionNormalTexture converted;
                    converted.Position = toXna(vertex.position);
                    converted.Normal = toXna(vertex.normal);
                    converted.TextureCoordinate = Xna::Vector2{vertex.texCoord.x, vertex.texCoord.y};
                    vertices.push_back(converted);
                }

                GpuPart gpuPart;
                gpuPart.vertexCount = static_cast<int>(vertices.size());
                gpuPart.triangleCount = static_cast<int>(part.indices.size() / 3);
                gpuPart.materialIndex = part.materialIndex;
                gpuPart.name = part.name;

                try
                {
                    gpuPart.vertices = std::make_unique<XnaGraphics::VertexBuffer>(
                        *device, XnaGraphics::VertexPositionNormalTexture::getVertexDeclarationStatic(),
                        gpuPart.vertexCount, XnaGraphics::BufferUsage::WriteOnly);
                    gpuPart.vertices->SetData(vertices.data(), gpuPart.vertexCount);

                    // Thirty-two-bit indices throughout rather than sixteen where they would fit.
                    // A cube would fit in sixteen and a scanned prop would not, and a pass that
                    // chose per model would have two paths where the rare one is the one that
                    // breaks. The memory is a few hundred kilobytes on a model worth drawing.
                    gpuPart.indices = std::make_unique<XnaGraphics::IndexBuffer>(
                        *device, XnaGraphics::IndexElementSize::ThirtyTwoBits,
                        static_cast<int>(part.indices.size()), XnaGraphics::BufferUsage::WriteOnly);
                    gpuPart.indices->SetData(part.indices.data(),
                                             static_cast<int>(part.indices.size()));
                }
                catch (const std::exception&)
                {
                    // A buffer that will not allocate is a device problem, not a document one.
                    // Drop the part and keep the rest: half a model drawn is more use than a
                    // viewport that stops, and the counters below show something went wrong.
                    continue;
                }

                ++stats.buffersCreated;
                model->parts.push_back(std::move(gpuPart));
            }

            GpuModel* raw = model.get();
            models.emplace(assetId, std::move(model));
            return raw;
        }

        /** @brief Returns the texture @p relativePath names, resolved through the database. */
        XnaGraphics::Texture2D* resolveTexture(const std::string& relativePath, ModelPassStats& stats)
        {
            if (relativePath.empty() || assets == nullptr) { return nullptr; }

            if (const auto found = textures.find(relativePath); found != textures.end())
            {
                return found->second.get();
            }
            if (failedTextures.count(relativePath) > 0) { return nullptr; }

            // The importer hands over a path relative to the model file and says so: it has no
            // database and must not pretend to. Resolving it is this side's job, and the database
            // is what knows where the project's assets are.
            const AssetRecord* record = assets->findByPath(relativePath);
            if (record == nullptr)
            {
                failedTextures.insert(relativePath);
                ++stats.missingTextures;
                return nullptr;
            }

            try
            {
                auto texture = std::make_shared<XnaGraphics::Texture2D>(
                    assets->resolvePath(record->sourcePath), *device);
                XnaGraphics::Texture2D* raw = texture.get();
                textures.emplace(relativePath, std::move(texture));
                return raw;
            }
            catch (const std::exception&)
            {
                failedTextures.insert(relativePath);
                ++stats.missingTextures;
                return nullptr;
            }
        }

        /**
         * @brief Applies @p environment's fog to whichever effect this build has.
         *
         * Linear between a start and an end distance, which is the whole of what `IEffectFog`
         * offers and therefore the whole of what this editor can promise. Both effects implement
         * that interface, so this is the one place in the pass that needs no branch on which one
         * was constructed.
         */
        void applyFog(const SceneEnvironment& environment)
        {
            XnaGraphics::IEffectFog* fog = pbr != nullptr
                                               ? static_cast<XnaGraphics::IEffectFog*>(pbr.get())
                                               : static_cast<XnaGraphics::IEffectFog*>(basic.get());
            if (fog == nullptr) { return; }

            fog->setFogEnabledProperty(environment.fogEnabled);
            if (!environment.fogEnabled) { return; }

            fog->setFogColorProperty(
                toXna(StudioVector3{static_cast<float>(environment.fogColor.r) / 255.0f,
                                    static_cast<float>(environment.fogColor.g) / 255.0f,
                                    static_cast<float>(environment.fogColor.b) / 255.0f}));
            fog->setFogStartProperty(environment.fogStart);
            fog->setFogEndProperty(environment.fogEnd);
        }

        /**
         * @brief Whichever effect this build has, as the interface the CNA extensions live behind.
         *
         * `BasicEffect` implements `IShadowReceiverEXT` exactly as `PbrEffect` does -- punctual
         * lights and shadow maps both -- so neither is a PBR-only feature and reaching them
         * through the concrete `PbrEffect` would silently switch them off on the path this build
         * actually takes (`kPreferPbrEffect` is false). That mistake was made here once already,
         * for the same reason the withdrawn gaps G-13 and G-14 were filed: reading one type's
         * header and concluding something about the API.
         */
        [[nodiscard]] XnaGraphics::IShadowReceiverEXT* shadowReceiver() const
        {
            if (pbr != nullptr) { return static_cast<XnaGraphics::IShadowReceiverEXT*>(pbr.get()); }
            if (basic != nullptr) { return static_cast<XnaGraphics::IShadowReceiverEXT*>(basic.get()); }
            return nullptr;
        }

        /** @brief Applies @p lighting to whichever effect this build has. */
        void applyLighting(const EffectLighting& lighting)
        {
            XnaGraphics::IEffectLights* lights =
                pbr != nullptr ? static_cast<XnaGraphics::IEffectLights*>(pbr.get())
                               : static_cast<XnaGraphics::IEffectLights*>(basic.get());
            if (lights == nullptr) { return; }

            if (lighting.useDefaultLighting)
            {
                // XNA's own three-point rig. Deliberately not an approximation of it written here:
                // the point of calling the framework's is that a CNA scene and an XNA one with no
                // lights in them look the same.
                lights->EnableDefaultLighting();

                // Except for the one thing the user may have said out loud (`plan.md`
                // STUDIO-20004). `EnableDefaultLighting` sets its own ambient, so a scene somebody
                // had darkened and not yet put a lamp in was drawn at XNA's brightness and the
                // setting did nothing. Applied *after* the call, because that is what it
                // overwrites, and only when the scene states an ambient of its own.
                if (lighting.ambientOverridesDefault)
                {
                    lights->setAmbientLightColorProperty(toXna(lighting.ambientColor));
                }
                return;
            }

            lights->setLightingEnabledProperty(true);
            lights->setAmbientLightColorProperty(toXna(lighting.ambientColor));

            XnaGraphics::DirectionalLight* slots[3] = {&lights->getDirectionalLight0Property(),
                                                       &lights->getDirectionalLight1Property(),
                                                       &lights->getDirectionalLight2Property()};

            for (std::size_t i = 0; i < 3; ++i)
            {
                const bool used = i < lighting.lightCount;
                slots[i]->setEnabledProperty(used);
                if (!used) { continue; }

                slots[i]->setDirectionProperty(toXna(lighting.lights[i].direction));
                slots[i]->setDiffuseColorProperty(toXna(lighting.lights[i].diffuseColor));
                slots[i]->setSpecularColorProperty(toXna(lighting.lights[i].specularColor));
            }

            // And the one real point or spot light, through CNA's own extension (`plan.md`
            // STUDIO-20003). `IShadowReceiverEXT` rather than `PbrEffect`, because both effects
            // implement it -- an earlier draft reached for the concrete type and switched the
            // feature off on the only path this build takes.
            XnaGraphics::IShadowReceiverEXT* receiver = shadowReceiver();
            if (receiver == nullptr) { return; }

            XnaGraphics::PunctualLightEXT punctual;
            if (lighting.hasPunctual)
            {
                punctual.Kind = lighting.punctual.kind == SceneLightKind::Spot
                                    ? XnaGraphics::PunctualLightKindEXT::Spot
                                    : XnaGraphics::PunctualLightKindEXT::Point;
                punctual.Position = toXna(lighting.punctual.position);
                punctual.Direction = toXna(lighting.punctual.direction);
                punctual.DiffuseColor = toXna(lighting.punctual.diffuseColor);
                punctual.Range = lighting.punctual.range;
                punctual.InnerAngle = lighting.punctual.innerAngle;
                punctual.OuterAngle = lighting.punctual.outerAngle;
            }

            // Set even when there is none: `Kind::None` leaves every other field inert, and an
            // effect is reused across draws -- a lamp left attached from the previous model would
            // light one that is nowhere near it.
            receiver->setPunctualLightEXT(punctual);
        }

        /**
         * @brief Points the effect at this frame's shadow map, or takes it away (STUDIO-20006).
         *
         * Per draw, because `receiveShadows` is per draw -- and *both* ways for the same reason
         * the punctual light is set even when there is none: one effect is reused across every
         * model in the batch, so a map left attached from the previous draw would shadow a model
         * whose author switched shadows off.
         */
        void applyShadows(bool receives)
        {
            XnaGraphics::IShadowReceiverEXT* receiver = shadowReceiver();
            if (receiver == nullptr) { return; }

            const bool on = receives && shadowsLive && shadows != nullptr;
            receiver->setShadowsEnabledEXT(on);
            if (!on)
            {
                receiver->setShadowMapEXT(nullptr);
                return;
            }

            receiver->setShadowMapEXT(shadows->getShadowTexture());
            receiver->setLightViewProjectionEXT(shadows->getLightViewProjection());

            // Both taken from the map rather than chosen here. The filter radius is the one the
            // map's own resolution was tuned for, and the bias trades shadow acne against a
            // shadow that detaches from its caster -- numbers with no defensible value that is
            // not the generator's, which is why `ShadowMap` publishes them at all.
            receiver->setShadowFilterRadiusEXT(shadows->getFilterRadius());
            receiver->setShadowDepthBiasEXT(shadows->getDepthBias());
        }

        // `applyMaterial` lived here and resolved a part's material itself -- the per-part
        // list, then the model override, then the part's own. `STUDIO-19004` needed the same
        // answer *before* the draw, to decide which pass a part belongs in, so the rule moved to
        // `resolveMeshPartMaterial` in the CNA-free scene module and this copy went with it.
        // Two copies of it would have been two chances to put a part in one pass and draw it with
        // the other's material.

        /** @brief Studio's alpha mode as CNA spells it. */
        [[nodiscard]] static XnaGraphics::AlphaModeEXT toXnaAlphaMode(MeshAlphaMode mode)
        {
            switch (mode)
            {
                case MeshAlphaMode::Mask:  return XnaGraphics::AlphaModeEXT::Mask;
                case MeshAlphaMode::Blend: return XnaGraphics::AlphaModeEXT::Blend;
                case MeshAlphaMode::Opaque: break;
            }
            return XnaGraphics::AlphaModeEXT::Opaque;
        }

        /** @brief Sets @p material on whichever effect this build has. */
        void applyResolvedMaterial(const MeshMaterial& material, ModelPassStats& stats)
        {
            XnaGraphics::Texture2D* diffuse = resolveTexture(material.diffuseTexturePath, stats);

            if (pbr != nullptr)
            {
                // See `whiteTexel`: PbrEffect has no texture-enabled flag and always samples.
                if (diffuse == nullptr) { diffuse = whiteTexel.get(); }

                pbr->setDiffuseColorProperty(toXna(material.diffuseColor));
                pbr->setEmissiveFactorProperty(toXna(material.emissiveColor));
                pbr->setAlphaProperty(material.alpha);
                pbr->setMetallicFactorProperty(material.metallic);
                pbr->setRoughnessFactorProperty(material.roughness);
                // No texture-enabled flag on this one, unlike BasicEffect: a null texture is
                // how PbrEffect is told there is none.
                pbr->setTextureProperty(diffuse);
                pbr->setNormalMapProperty(resolveTexture(material.normalTexturePath, stats));
                pbr->setMetallicRoughnessMapProperty(
                    resolveTexture(material.metallicRoughnessTexturePath, stats));
                pbr->setEmissiveMapProperty(resolveTexture(material.emissiveTexturePath, stats));
                pbr->setOcclusionMapProperty(resolveTexture(material.occlusionTexturePath, stats));

                // glTF's own alpha coverage (`plan.md` STUDIO-19004). The device's blend state
                // decides whether the *result* is blended; this decides what the shader does with
                // the alpha channel before that, which is the half a blend state cannot express --
                // a masked leaf is cut out, not faded.
                pbr->setAlphaModeEXTProperty(toXnaAlphaMode(material.alphaMode));
                pbr->setAlphaCutoffEXTProperty(material.alphaCutoff);
                return;
            }

            if (basic == nullptr) { return; }

            basic->setDiffuseColorProperty(toXna(material.diffuseColor));
            basic->setEmissiveColorProperty(toXna(material.emissiveColor));
            basic->setSpecularColorProperty(toXna(material.specularColor));
            basic->setSpecularPowerProperty(material.specularPower);
            basic->setAlphaProperty(material.alpha);
            basic->setTextureProperty(diffuse);
            basic->setTextureEnabledProperty(diffuse != nullptr);
        }

        /**
         * @brief Sets a sprite's tint and texture, unlit.
         *
         * Unlit deliberately: a sprite's art already has its lighting painted into it, which is
         * what a 2D game *is*, and running it through the same directional rig as the models would
         * darken every sprite by an amount that depends on where the sun happens to be. The tint
         * goes in as the emissive colour so it survives lighting being off.
         */
        void applySpriteMaterial(const StudioColor& tint, XnaGraphics::Texture2D* texture)
        {
            const StudioVector3 colour{static_cast<float>(tint.r) / 255.0f,
                                       static_cast<float>(tint.g) / 255.0f,
                                       static_cast<float>(tint.b) / 255.0f};
            const float alpha = static_cast<float>(tint.a) / 255.0f;

            if (pbr != nullptr)
            {
                pbr->setLightingEnabledProperty(false);
                pbr->setDiffuseColorProperty(toXna(colour));
                pbr->setEmissiveFactorProperty(toXna(colour));
                pbr->setAlphaProperty(alpha);
                pbr->setTextureProperty(texture);
                pbr->setNormalMapProperty(nullptr);
                pbr->setMetallicRoughnessMapProperty(nullptr);
                pbr->setEmissiveMapProperty(nullptr);
                return;
            }

            if (basic == nullptr) { return; }

            basic->setLightingEnabledProperty(false);
            basic->setDiffuseColorProperty(toXna(colour));
            basic->setEmissiveColorProperty(toXna(colour));
            basic->setSpecularColorProperty(toXna(StudioVector3{0.0f, 0.0f, 0.0f}));
            basic->setAlphaProperty(alpha);
            basic->setTextureProperty(texture);
            basic->setTextureEnabledProperty(true);
        }

        /** @brief Sets the world/view/projection the effect draws @p world with. */
        void applyMatrices(const StudioMatrix& world, const StudioMatrix& view,
                           const StudioMatrix& projection)
        {
            XnaGraphics::IEffectMatrices* matrices =
                pbr != nullptr ? static_cast<XnaGraphics::IEffectMatrices*>(pbr.get())
                               : static_cast<XnaGraphics::IEffectMatrices*>(basic.get());
            if (matrices == nullptr) { return; }

            // The view and the projection go in separately, rather than the product as the
            // projection with an identity view. That shortcut works for BasicEffect and renders
            // PbrEffect's specular against an eye at the origin: it recovers the camera position
            // by *inverting the view matrix*, so an identity view is a camera claiming to be at
            // (0, 0, 0). The batch carries both forms from one camera and a test pins their
            // product, which is what keeps this from becoming two answers to one question.
            matrices->setWorldProperty(toXna(world));
            matrices->setViewProperty(toXna(view));
            matrices->setProjectionProperty(toXna(projection));
        }

        /** @brief Applies the effect's current state to the device. */
        void applyEffect()
        {
            if (pbr != nullptr) { pbr->Apply(); }
            else if (basic != nullptr) { basic->Apply(); }
        }
    };

    CnaModelPass::CnaModelPass() : impl_(std::make_unique<Impl>()) {}
    CnaModelPass::~CnaModelPass() = default;

    void CnaModelPass::initialize(XnaGraphics::GraphicsDevice& device, const AssetDatabase& assets)
    {
        impl_->device = &device;
        impl_->assets = &assets;

        try
        {
            impl_->whiteTexel = std::make_shared<XnaGraphics::Texture2D>(device, 1, 1);
            const Xna::Color white(255, 255, 255, 255);
            impl_->whiteTexel->SetData(&white, 1);
        }
        catch (const std::exception&)
        {
            impl_->whiteTexel.reset();
        }

        // PbrEffect first, because the owner chose it; BasicEffect when it cannot be had. Trying
        // and catching rather than asking is deliberate -- there is no capability flag for "this
        // backend can compile the PBR shader", and a wrong guess either way is a black viewport.
        if (kPreferPbrEffect)
        {
            try
            {
                impl_->pbr = std::make_unique<XnaGraphics::PbrEffect>(device);
                impl_->effectName = "PbrEffect";
                return;
            }
            catch (const std::exception&)
            {
                impl_->pbr.reset();
            }
        }

        try
        {
            impl_->basic = std::make_unique<XnaGraphics::BasicEffect>(device);
            impl_->effectName = "BasicEffect";
        }
        catch (const std::exception&)
        {
            impl_->basic.reset();
            impl_->effectName = "none";
        }
    }

    void CnaModelPass::shutdown()
    {
        impl_->models.clear();
        impl_->textures.clear();
        impl_->failedTextures.clear();
        impl_->whiteTexel.reset();

        // Before the device pointer goes: the map owns a render target created against it, and a
        // `shutdown` that left one behind would outlive the device it was made from.
        impl_->shadows.reset();
        impl_->shadowsLive = false;
        impl_->shadowsRefused = false;
        impl_->shadowCasters = 0;

        impl_->pbr.reset();
        impl_->basic.reset();
        impl_->effectName = "none";
        impl_->device = nullptr;
        impl_->assets = nullptr;
    }

    void CnaModelPass::invalidateModel(const Uuid& assetId)
    {
        if (!assetId.isValid())
        {
            impl_->models.clear();
            // The textures go too. A model reimported because its file changed may name different
            // ones, and a remembered failure must not outlive the file coming back -- the same
            // rule `CnaSceneRenderer::invalidateTexture` follows and for the same reason.
            impl_->textures.clear();
            impl_->failedTextures.clear();
            return;
        }

        impl_->models.erase(assetId);
    }

    bool CnaModelPass::isReady() const
    {
        return impl_->device != nullptr && (impl_->pbr != nullptr || impl_->basic != nullptr);
    }

    const std::string& CnaModelPass::getEffectName() const { return impl_->effectName; }

    ModelPassStats CnaModelPass::renderSprites(
        const SceneSpriteBatch3D& sprites, const SceneModelBatch& batch,
        const std::function<XnaGraphics::Texture2D*(const Uuid&)>& resolveTexture)
    {
        ModelPassStats stats;
        stats.effect = impl_->effectName;

        if (!isReady() || sprites.quads.empty() || !resolveTexture) { return stats; }

        XnaGraphics::GraphicsDevice& device = *impl_->device;

        // Depth *tested* so a model in front of a sprite hides it, depth *writes off* so a
        // sprite's own transparent corners do not punch a hole later sprites cannot draw through.
        device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::DepthRead);
        device.setBlendStateProperty(XnaGraphics::BlendState::AlphaBlend);
        device.getSamplerStatesProperty()[0] = XnaGraphics::SamplerState::PointClamp;

        // Both sides. A sprite is a flat thing with no inside, and half of them would otherwise
        // vanish the moment the camera orbited past their plane -- which is the one thing a user
        // orbiting a 2D scene will certainly do.
        XnaGraphics::RasterizerState rasterizer;
        rasterizer.setCullModeProperty(XnaGraphics::CullMode::None);
        device.setRasterizerStateProperty(rasterizer);

        // One quad at a time, in the order the batch sorted them. Batching by texture would be
        // faster and would also reorder them, and back-to-front order is the whole reason a
        // transparent pass looks right.
        for (const SpriteQuad3D& quad : sprites.quads)
        {
            XnaGraphics::Texture2D* texture = resolveTexture(quad.textureId);
            if (texture == nullptr)
            {
                ++stats.missingTextures;
                continue;
            }

            std::array<XnaGraphics::VertexPositionNormalTexture, 4> vertices{};
            for (std::size_t i = 0; i < 4; ++i)
            {
                vertices[i].Position = toXna(quad.corners[i]);

                // Facing back along -Z, which is where the unrotated camera is. A sprite is lit by
                // nothing in particular -- see the emissive setting below -- so this exists to be
                // a well-defined normal rather than to be shaded by.
                vertices[i].Normal = Xna::Vector3{0.0f, 0.0f, -1.0f};
                vertices[i].TextureCoordinate =
                    Xna::Vector2{quad.texCoords[i].x, quad.texCoords[i].y};
            }

            const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};

            impl_->applyMatrices(StudioMatrix{}, batch.view, batch.projection);

            // Sprites are fogged too. A sprite that stayed crisp in a scene where the models faded
            // would look like it was floating in front of the fog rather than standing in it.
            impl_->applyFog(batch.environment);

            // And not shadowed, for the same reason `applySpriteMaterial` turns lighting off: a
            // sprite is a flat quad facing the camera, and nothing in the document says whether it
            // receives -- `receiveShadows` belongs to `CNA.ModelRenderer`. Said out loud rather
            // than left to the last model's state, which is the effect's and carries over.
            impl_->applyShadows(false);

            impl_->applySpriteMaterial(quad.tint, texture);
            impl_->applyEffect();

            // DrawUserIndexedPrimitives rather than a cached buffer per sprite: the corners move
            // whenever the entity does, so a buffer would be rewritten every frame anyway, and one
            // per entity is a GPU allocation per entity for four vertices.
            device.DrawUserIndexedPrimitives(XnaGraphics::PrimitiveType::TriangleList,
                                             vertices.data(), 0, 4, indices.data(), 0, 2);

            ++stats.spritesDrawn;
            stats.trianglesDrawn += 2;
        }

        device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::None);
        device.setBlendStateProperty(XnaGraphics::BlendState::AlphaBlend);
        return stats;
    }

    std::size_t CnaModelPass::renderShadowMap(const SceneModelBatch& batch)
    {
        const SceneShadowPlan& plan = batch.shadows;

        // First, and whatever happens below. Every early return from here is a frame with no map,
        // and `applyShadows` reads this flag to decide whether to attach one -- so clearing it up
        // front is what stops a scene whose sun was just deleted from keeping yesterday's shadows.
        impl_->shadowsLive = false;
        impl_->shadowCasters = 0;

        if (!isReady() || !plan.enabled || plan.casterBounds.isEmpty()) { return 0; }
        if (impl_->shadowsRefused) { return 0; }

        XnaGraphics::GraphicsDevice& device = *impl_->device;

        // Two capabilities, and they are genuinely different questions: `ShadowMap::isSupported`
        // asks whether this renderer can *generate* a map, and this asks whether its lit shaders
        // can *sample* one. CNA's own shadow example checks both and says why -- on Vulkan the
        // first is true and the second is false, and applying a shadow-sampling effect there does
        // not render an unshadowed picture, it crashes mid-draw. Generating a map nothing can read
        // would also be a full render target's worth of work per frame for no pixels.
        if (!device.SupportsShadowSamplingEXT())
        {
            impl_->shadowsRefused = true;
            impl_->shadows.reset();
            return 0;
        }

        if (impl_->shadows == nullptr)
        {
            try
            {
                impl_->shadows = std::make_unique<CNA::Graphics::ShadowMap>(device, kShadowQuality);
            }
            catch (const std::exception&)
            {
                // A target that will not allocate is a device problem, and the honest response is
                // a scene without shadows rather than a viewport that stops.
                impl_->shadowsRefused = true;
                impl_->shadows.reset();
                return 0;
            }

            if (!impl_->shadows->isSupported())
            {
                impl_->shadowsRefused = true;
                impl_->shadows.reset();
                return 0;
            }
        }

        CNA::Graphics::ShadowMap& map = *impl_->shadows;

        CNA::Graphics::DirectionalLightEXT sun;
        sun.Direction = toXna(plan.direction);

        // The plan's colour already carries the intensity, so the light's own multiplier stays at
        // one rather than applying it twice. Neither reaches the map -- a shadow caster writes
        // distance and nothing else -- but `begin` takes the light whole and a field left wrong
        // because it is currently unread is a field that is wrong when something reads it.
        sun.Color = toXna(plan.color);
        sun.Intensity = 1.0f;
        sun.CastsShadows = true;

        const Xna::BoundingBox bounds{toXna(plan.casterBounds.min), toXna(plan.casterBounds.max)};

        try
        {
            map.begin(sun, bounds);
        }
        catch (const std::exception&)
        {
            return 0;
        }

        std::size_t casters = 0;

        try
        {
            // Both faces, which is what CNA's own example draws its casters with. The scene pass
            // culls counter-clockwise because the *camera's* projection mirrors Y; the light's
            // view-projection is CNA's own and mirrors nothing, so that reasoning does not carry
            // over -- and a caster that is a single unclosed surface, which plenty of imported
            // geometry is, casts nothing at all when either side is culled.
            XnaGraphics::RasterizerState rasterizer;
            rasterizer.setCullModeProperty(XnaGraphics::CullMode::None);
            device.setRasterizerStateProperty(rasterizer);
            device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::Default);
            device.setBlendStateProperty(XnaGraphics::BlendState::Opaque);

            XnaGraphics::ShaderEffect* caster = map.getCasterEffect();

            for (const ModelDraw& draw : batch.draws)
            {
                if (!draw.castsShadow || draw.mesh == nullptr) { continue; }

                // Counted into a statistics object that is thrown away: a buffer uploaded here is
                // the same buffer the shading pass draws from, and reporting it twice would make
                // `buffersCreated` say a model was uploaded two frames running.
                ModelPassStats uploads;
                Impl::GpuModel* model = impl_->resolveModel(draw.modelId, *draw.mesh, uploads);
                if (model == nullptr || model->parts.empty()) { continue; }

                // `applyCaster` re-binds the effect and re-uploads the light matrix, then resets
                // the world matrix to identity -- so the world goes up after it, not before. The
                // uniform is the documented way to place a caster: `getCasterEffect` returns a
                // raw `ShaderEffect` precisely so that an app with its own transforms can use it.
                if (caster != nullptr)
                {
                    map.applyCaster();
                    const Xna::Matrix world = toXna(draw.world);
                    caster->SetUniformMat4("uWorld", &world.M11);
                }

                for (const Impl::GpuPart& part : model->parts)
                {
                    // Every part, whatever its alpha mode. A blended pane's shadow is a question
                    // this editor cannot answer honestly -- the map stores one distance per texel
                    // and has nowhere to put "half blocked" -- and a window that casts a solid
                    // shadow is a smaller lie than a wall that casts none because its glass was
                    // modelled as one part of it.
                    device.SetVertexBuffer(part.vertices.get());
                    device.setIndicesProperty(part.indices.get());
                    device.DrawIndexedPrimitives(XnaGraphics::PrimitiveType::TriangleList, 0, 0,
                                                 part.vertexCount, 0, part.triangleCount);
                }

                ++casters;
            }
        }
        catch (const std::exception&)
        {
            // Closed below whatever happened in here: `begin` throws on a pass that is already
            // open, so a pass left open by an exception is a viewport that never shadows again.
            casters = 0;
        }

        try
        {
            map.end();
        }
        catch (const std::exception&)
        {
            return 0;
        }

        // Left as `render` expects to find it -- it sets its own rasteriser and depth state, but
        // the sprite pass and the wireframe overlay both run against whatever is current, and
        // `None` is what every other pass in this file leaves behind.
        device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::None);

        if (casters == 0) { return 0; }

        impl_->shadowsLive = true;
        impl_->shadowCasters = casters;
        return casters;
    }

    ModelPassStats CnaModelPass::render(const SceneModelBatch& batch)
    {
        ModelPassStats stats;
        stats.effect = impl_->effectName;

        if (!isReady() || batch.draws.empty()) { return stats; }

        XnaGraphics::GraphicsDevice& device = *impl_->device;

        device.getSamplerStatesProperty()[0] = XnaGraphics::SamplerState::LinearWrap;

        XnaGraphics::RasterizerState rasterizer;
        rasterizer.setCullModeProperty(kOutwardFaces);
        device.setRasterizerStateProperty(rasterizer);

        // Which draws go in which pass, and in what order the blended one runs (STUDIO-19004).
        // Worked out by a CNA-free function so the *ordering* -- the part with an answer that can
        // be wrong -- is testable without a device.
        const SceneDrawOrder order = orderSceneModelDraws(batch);

        // Counted once per model however many passes it appears in: a window frame with a pane in
        // it is one model, and a statistic that said two would be a statistic about this loop.
        std::vector<bool> counted(batch.draws.size(), false);

        const auto drawPass = [&](const std::vector<std::size_t>& indices, bool blendedPass) {
            if (indices.empty()) { return; }

            if (blendedPass)
            {
                // Depth *read* rather than Default: a blended surface must still be hidden by the
                // wall in front of it, and must not write a depth that stops the pane behind it
                // from drawing. Writing depth here is how a window comes to occlude the room.
                device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::DepthRead);
                device.setBlendStateProperty(XnaGraphics::BlendState::AlphaBlend);
            }
            else
            {
                // The target this draws into is created with a depth buffer for exactly this
                // state to use; without it a model's own back faces punch through its front.
                device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::Default);
                device.setBlendStateProperty(XnaGraphics::BlendState::Opaque);
            }

            for (const std::size_t index : indices)
            {
                const ModelDraw& draw = batch.draws[index];
                if (draw.mesh == nullptr) { continue; }

                Impl::GpuModel* model = impl_->resolveModel(draw.modelId, *draw.mesh, stats);
                if (model == nullptr || model->parts.empty()) { continue; }

                impl_->applyMatrices(draw.world, batch.view, batch.projection);
                impl_->applyLighting(draw.lighting);
                impl_->applyShadows(draw.receivesShadow);
                impl_->applyFog(batch.environment);

                for (const Impl::GpuPart& part : model->parts)
                {
                    // Resolved through the same CNA-free function the ordering used, rather than
                    // a second copy of the rule here: the two disagreeing would put a part in one
                    // pass and draw it with the other's material.
                    const MeshMaterial resolved =
                        resolveMeshPartMaterial(draw, part.name, part.materialIndex);
                    if ((resolved.alphaMode == MeshAlphaMode::Blend) != blendedPass) { continue; }

                    impl_->applyResolvedMaterial(resolved, stats);
                    impl_->applyEffect();

                    device.SetVertexBuffer(part.vertices.get());
                    device.setIndicesProperty(part.indices.get());
                    device.DrawIndexedPrimitives(XnaGraphics::PrimitiveType::TriangleList, 0, 0,
                                                 part.vertexCount, 0, part.triangleCount);

                    stats.trianglesDrawn += static_cast<std::size_t>(part.triangleCount);
                }

                if (!counted[index])
                {
                    counted[index] = true;
                    ++stats.modelsDrawn;
                }
            }
        };

        drawPass(order.opaque, /*blendedPass=*/false);
        drawPass(order.blended, /*blendedPass=*/true);

        // Recorded by `renderShadowMap`, reported here: the shadow pass runs before this one and
        // returns its own count, but the Diagnostics panel is given one `ModelPassStats` for the
        // frame and a second number arriving separately is a number that can go missing.
        stats.shadowCasters = impl_->shadowCasters;

        // Left as the caller found it. The wireframe drawn over this is a `SpriteBatch` pass, and
        // a SpriteBatch that inherits a depth test compares against depths no sprite ever wrote --
        // which is how an overlay comes to be invisible for reasons nothing in its own code shows.
        device.setDepthStencilStateProperty(XnaGraphics::DepthStencilState::None);

        return stats;
    }
}
