// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Viewport/CnaModelPass.hpp
 * @brief Draws imported models as solid, lit geometry through CNA's public API (plan.md ED-402).
 *
 * The CNA half of ED-402. What to draw was decided in `cna-studio-scene`
 * (`SceneModels.hpp`) where CI can check it with no device; this uploads that decision and issues
 * the draw calls. Four things are decided *here*, because each is about the device rather than
 * about the document.
 *
 * **1. `PbrEffect` where the build has one, `BasicEffect` where it does not.** The owner chose PBR
 * with a fallback, and the fallback is not defensive programming for its own sake: `PbrEffect` is
 * `NOXNA`, a CNA extension, and this editor supports fourteen backends of three support tiers
 * (F-02). The effect is constructed once and the choice reported through `ModelPassStats::effect`
 * so the Diagnostics panel can say which one a build actually got -- "why does it look different
 * on this machine" deserves an answer that is not a screenshot comparison.
 *
 * **2. The render target needs a depth buffer, and the 2D one has none.** `CnaSceneRenderer`'s
 * target is created with the two-argument `RenderTarget2D` constructor, which is documented as
 * "no depth buffer" -- correct for sprites, which sort by draw order, and useless for models,
 * which sort per pixel. Without one a crate renders with its back faces punched through its front.
 * So the 3D view asks for a target *with* depth, and the renderer recreates the target when the
 * requirement changes rather than keeping two.
 *
 * **3. Cull counter-clockwise faces -- which is right *because* of the Y mirror, not despite it.**
 * The importer winds triangles counter-clockwise seen from outside, in world space
 * (`meshWindingMatchesNormals`). The view-projection then mirrors Y, and mirroring one axis
 * reverses apparent winding, so an outward-facing triangle arrives at the rasteriser clockwise --
 * which `CullCounterClockwiseFace`, XNA's own default, keeps. Two reversals that cancel is exactly
 * the kind of reasoning that is worth writing down and then *checking*, so the constant below is
 * one line to change and the screenshot in NEXT.md is what confirms it.
 *
 * **4. Buffers are cached per model asset, never per entity.** Ten crates are one vertex buffer
 * drawn ten times with ten world matrices. Keyed by `Uuid` for the reason D-08 gives everywhere
 * else: an asset that moves keeps its id.
 */

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/Scene/SceneModels.hpp"
#include "CNA/Studio/Scene/SceneShadows.hpp"
#include "CNA/Studio/Scene/SceneSprites3D.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class Texture2D;
}

namespace CNA::Studio
{
    class AssetDatabase;

    /** @brief What one model pass did, for the diagnostics panel and the tests. */
    struct ModelPassStats
    {
        /** @brief Models drawn, counting one per entity rather than per shared asset. */
        std::size_t modelsDrawn = 0;

        /** @brief Triangles submitted, before whatever the device culls. */
        std::size_t trianglesDrawn = 0;

        /** @brief Vertex buffers created this pass, which is non-zero only as models first appear. */
        std::size_t buffersCreated = 0;

        /** @brief Draws whose material named a texture that could not be resolved. */
        std::size_t missingTextures = 0;

        /** @brief Sprite quads drawn after the models, blended back to front. */
        std::size_t spritesDrawn = 0;

        /**
         * @brief Draws rendered into the shadow map, which is zero whenever no map was generated.
         *
         * Reported for the same reason @ref effect is: whether this build shadows at all depends
         * on the renderer, and "why are there no shadows on this machine" should have an answer in
         * the Diagnostics panel rather than in a screenshot comparison.
         */
        std::size_t shadowCasters = 0;

        /**
         * @brief 1 when the sky was drawn this frame, 0 otherwise (`plan.md` STUDIO-20005).
         *
         * Reported for the same reason @ref effect is: whether a build draws a sky at all depends
         * on the renderer, and "why is there no sky on this machine" deserves an answer in the
         * Diagnostics panel rather than in a screenshot comparison.
         */
        std::size_t skiesDrawn = 0;

        /**
         * @brief True when an image-based light reached the effect this frame.
         *
         * Distinct from @ref skiesDrawn because the two can differ, and the difference is the
         * whole of what `STUDIO-20005` has to disclose: on a `BasicEffect` build the sky is drawn
         * and lights nothing.
         */
        bool environmentLit = false;

        /**
         * @brief Which effect the pass is using: "PbrEffect", "BasicEffect", or "none".
         *
         * Reported rather than assumed, because it varies by build and it is the first thing worth
         * knowing when a model looks different on one machine than another.
         */
        std::string effect = "none";
    };

    /**
     * @brief Uploads and draws the models a `SceneModelBatch` names.
     *
     * Owns its GPU resources and releases them on `shutdown`. Not copyable: it holds device
     * objects whose lifetime is the device's.
     */
    class CnaModelPass
    {
    public:
        CnaModelPass();
        ~CnaModelPass();

        CnaModelPass(const CnaModelPass&) = delete;
        CnaModelPass& operator=(const CnaModelPass&) = delete;

        /**
         * @brief Binds the pass to a device and the database its textures are resolved through.
         *
         * Constructs the effect here rather than lazily, so a build that cannot make a `PbrEffect`
         * has said so before the first frame instead of during it.
         */
        void initialize(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device,
                        const AssetDatabase& assets);

        /** @brief Releases every buffer, texture and effect. */
        void shutdown();

        /**
         * @brief Draws @p batch into the *current* render target, which must have depth.
         *
         * The caller owns the target and the clear: this pass runs between the background and the
         * wireframe overlay, and a pass that cleared would erase the one and a pass that set its
         * own target would have nowhere to put the other.
         */
        ModelPassStats render(const SceneModelBatch& batch);

        /**
         * @brief Draws @p sprites as textured quads, after @p batch's models.
         *
         * After, and with **depth writing off while depth testing stays on**. Both halves matter.
         * Testing on is what lets a model stand in front of a sprite behind it. Writing off is what
         * stops a sprite's own transparent corners from punching a hole in the depth buffer that a
         * sprite drawn later cannot draw through -- the classic symptom being a rectangle of
         * background around every sprite that overlaps another.
         *
         * @param resolveTexture How a sprite's `Uuid` becomes a texture. Injected rather than
         *        looked up here, because the renderer beside this one already caches every sprite
         *        texture in the project and a second cache would load them all twice.
         */
        ModelPassStats renderSprites(
            const SceneSpriteBatch3D& sprites, const SceneModelBatch& batch,
            const std::function<Microsoft::Xna::Framework::Graphics::Texture2D*(const Uuid&)>&
                resolveTexture);

        /**
         * @brief Draws the scene's sky, and prepares the light it casts (`plan.md` STUDIO-20005).
         *
         * Called *inside* the target the models are drawn into and *before* them, which is what
         * `CNA::Graphics::Skybox` asks for: it draws a fullscreen triangle with the view's
         * translation stripped, so the sky never moves with the camera and never occludes
         * geometry. The engine layer's fullscreen mechanism carries no depth configuration, which
         * is why "first" rather than "last at the far plane" -- CNA records that deviation itself.
         *
         * **Two products from one panorama, and only the ones something can use.** The cube the
         * sky is drawn from is generated whenever `SceneSkyPlan::draws` is set, because drawing it
         * needs no effect support. The irradiance, prefiltered specular and BRDF table are
         * generated only when the effect can take them -- `setImageBasedLightEXT` is declared on
         * `PbrEffect` and `SkinnedPbrEffect` and nowhere else, and this build draws through
         * `BasicEffect` (`G-05`). Convolving a hemisphere per texel for a light nothing can sample
         * would be a second of CPU work per sky for no pixels.
         *
         * Everything is cached against the environment map's asset id, so a scene redrawn sixty
         * times a second processes its panorama once.
         *
         * @param resolveTexture How the panorama's `Uuid` becomes a texture. Injected for the
         *        reason @ref renderSprites injects one: the renderer beside this already caches
         *        every project texture, and a second cache would load them all twice.
         * @return 1 when a sky was drawn, 0 otherwise.
         */
        std::size_t renderSky(
            const SceneModelBatch& batch, int width, int height,
            const std::function<Microsoft::Xna::Framework::Graphics::Texture2D*(const Uuid&)>&
                resolveTexture);

        /**
         * @brief Renders @p batch's casters into the shadow map, before @ref render (STUDIO-20006).
         *
         * Called *outside* the render target the models are drawn into, because the pass binds and
         * restores its own -- `ShadowMap::end` rebinds the back buffer rather than whatever was
         * there before, so a call made inside the scene's target would leave the rest of the frame
         * drawing into the window. The map it produces is attached to every receiving draw by
         * @ref render, so the two calls are one operation split by what they bind rather than two
         * features.
         *
         * Takes the batch alone and reads `SceneModelBatch::shadows`, rather than taking a plan
         * beside it: a plan and a batch passed separately are two things a caller can get out of
         * step, and the answer to "which of these draws casts" has to be the one the batch was
         * built with.
         *
         * Draws nothing, and releases whatever was there, when the plan is not `enabled`, when the
         * device cannot generate maps (`ShadowMap::isSupported`), or when it cannot *sample* one
         * (`GraphicsDevice::SupportsShadowSamplingEXT`). A stale map left attached would shadow
         * the scene with the arrangement it had two edits ago, which is worse than no shadow.
         *
         * @return How many draws were rendered into the map.
         */
        std::size_t renderShadowMap(const SceneModelBatch& batch);

        /** @brief Drops the GPU buffers for @p assetId, or all of them when it is nil. */
        void invalidateModel(const Uuid& assetId);

        /** @brief True when an effect was constructed and the pass can draw. */
        [[nodiscard]] bool isReady() const;

        /** @brief Which effect was constructed: "PbrEffect", "BasicEffect" or "none". */
        [[nodiscard]] const std::string& getEffectName() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
