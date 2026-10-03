// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/SceneShadows.hpp
 * @brief Which light casts the scene's shadows, and over what (`plan.md` STUDIO-20006).
 *
 * `CNA.ModelRenderer` has carried `castShadows` and `receiveShadows` since Phase 1, both
 * defaulting to true, both editable, and read by nothing. CNA can draw shadows -- `PbrEffect`
 * implements `IShadowReceiverEXT` and `CNA::Graphics::ShadowMap` generates the map -- so what was
 * missing is a pass in Studio's viewport and the two decisions it needs.
 *
 * Both decisions are arithmetic over a batch, so they live here, CNA-free and tested against a
 * document rather than against an image. What is left for the device layer is `begin`, a loop,
 * `end`, and attaching the result -- which is the same division `STUDIO-11010` and `STUDIO-11008`
 * use, and the reason this file exists rather than three more branches inside the pass.
 *
 * ### One light, and it is a directional one
 *
 * `ShadowMap::begin` takes a `DirectionalLightEXT`: the map is an orthographic volume fitted to
 * the scene, which is what a directional light's shadow is and is not what a point light's is (six
 * faces of a cube). A punctual light carries its *own* shadow in `PunctualLightEXT`, so this is
 * deliberately only about the sun.
 *
 * The brightest enabled directional light wins, which is the same rule `computeEffectLighting`
 * uses for the three slots and for the same reason: a distant sun matters more to how a scene
 * looks than a dim fill, and document order is not something a user arranges deliberately.
 */

#include <cstddef>
#include <vector>

#include "CNA/Studio/Core/StudioMath.hpp"
#include "CNA/Studio/Scene/SceneLighting.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"

namespace CNA::Studio
{
    /**
     * @brief Declared rather than included, because `SceneModels.hpp` includes *this*.
     *
     * The plan is a field of @ref SceneModelBatch -- that is what stops the viewport from holding
     * a second copy of it that could disagree with the batch it belongs to -- so a by-value member
     * needs this header complete before that one. `planSceneShadows` takes the batch by reference,
     * which a declaration is enough for, and the definition includes both.
     */
    struct SceneModelBatch;

    /** @brief What a shadow pass should render, or that there is nothing to render. */
    struct SceneShadowPlan
    {
        /**
         * @brief True when the pass is worth running at all.
         *
         * False for a scene with no enabled directional light, or with nothing in it that casts.
         * Both are ordinary states rather than failures -- a 2D scene has neither -- and running
         * the pass anyway would cost a full-screen render to produce a map meaning "nothing
         * occludes", every frame, for a picture that cannot change.
         */
        bool enabled = false;

        /** @brief The direction the shadowing light travels. Unit length when @ref enabled. */
        StudioVector3 direction{0.0f, 1.0f, 0.0f};

        /** @brief Its colour times its intensity, for the light the map is generated from. */
        StudioVector3 color{1.0f, 1.0f, 1.0f};

        /**
         * @brief World bounds of everything that casts, which the projection is fitted to.
         *
         * Fitting matters more than it looks: a volume twice the size it needs is a quarter of the
         * effective resolution. The bounds are the *casters'* rather than the whole scene's, for
         * exactly that reason -- a distant skybox entity would otherwise halve the shadow quality
         * of everything a user can see.
         */
        WorldBounds3D casterBounds = WorldBounds3D::makeEmpty();

        /** @brief How many draws in the batch are casters. Zero means @ref enabled is false. */
        std::size_t casters = 0;

        /** @brief How many draws sample the map. May be zero while @ref enabled is true. */
        std::size_t receivers = 0;
    };

    /**
     * @brief Decides what @p batch's shadow pass should draw, given @p lights.
     *
     * @param lights The scene's lights, as `collectSceneLights` returns them. Only the enabled
     *        directional ones are considered; see this file's header.
     *
     * A batch with casters and no receivers still runs: the map is cheap to generate and the
     * receivers are decided per draw, so a scene where the user has just unticked the last
     * *receiver* should not silently change what the *casters* do. A batch with no casters does
     * not, because there is nothing to put in the map.
     */
    [[nodiscard]] SceneShadowPlan planSceneShadows(const SceneModelBatch& batch,
                                                   const std::vector<SceneLight>& lights);
}
