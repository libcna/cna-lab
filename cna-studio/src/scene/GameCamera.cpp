// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/GameCamera.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"

namespace CNA::Studio
{
    namespace
    {
        /** @brief Returns the scene's primary camera component, or nullptr. */
        const StudioEntity* findPrimaryCamera(const SceneDocument& scene)
        {
            const StudioEntity* firstCamera = nullptr;

            for (const StudioEntity& entity : scene.getEntities())
            {
                // A disabled entity is not in the game at all, so its camera is not either.
                if (!entity.isEnabled()) { continue; }

                const StudioComponent* camera = entity.findComponent(BuiltinComponentIds::kCamera);
                if (camera == nullptr) { continue; }

                if (camera->getProperty("isPrimary").get<bool>(true)) { return &entity; }
                if (firstCamera == nullptr) { firstCamera = &entity; }
            }

            // No camera claims to be primary, but one exists. Using it beats drawing from the
            // origin: a scene whose only camera has the flag cleared is a mistake the user can see
            // immediately when the view is theirs, and cannot see at all when it is not.
            return firstCamera;
        }
    }

    namespace
    {
        /**
         * @brief How far in front of a perspective game camera its orbit pivot is put.
         *
         * Arbitrary and says so: a perspective view is unchanged by where the pivot sits along the
         * view direction, because the eye is `pivot - forward * distance` and both move together.
         * It matters only for an orthographic one, which derives its visible height from the
         * distance -- and that case takes the distance from `orthographicSize` instead.
         */
        constexpr float kGameCameraPivotDistance = 10.0f;

        /** @brief The same constant `SceneTransform` uses, for the same reason it is a constant. */
        constexpr float kDegreesToRadians = 3.14159265358979323846f / 180.0f;

        /** @brief The orbit distance at which @p height world units fill the view. */
        float orthographicHeightToDistance(float height, float fieldOfView)
        {
            // `getOrthographicHeight` is `2 * distance * tan(fov / 2)`, so this is its inverse.
            const float halfAngle = std::tan(fieldOfView * 0.5f);
            if (!(halfAngle > 0.0f) || !(height > 0.0f)) { return kGameCameraPivotDistance; }
            return height / (2.0f * halfAngle);
        }
    }

    GameView computeGameView(const SceneDocument& scene, const StudioVector2& viewportSize)
    {
        const StudioEntity* entity = findPrimaryCamera(scene);
        if (entity == nullptr)
        {
            GameView fallback;
            fallback.camera.setViewportSize(viewportSize);
            return fallback;
        }

        return computeGameViewFor(scene, entity->getId(), viewportSize);
    }

    GameView computeGameViewFor(const SceneDocument& scene, const Uuid& cameraId,
                                const StudioVector2& viewportSize)
    {
        GameView view;
        view.camera.setViewportSize(viewportSize);

        const StudioEntity* entity = scene.findEntity(cameraId);
        if (entity == nullptr) { return view; }

        const StudioComponent* camera = entity->findComponent(BuiltinComponentIds::kCamera);
        if (camera == nullptr) { return view; }

        view.cameraId = entity->getId();
        view.clearColor = camera->getProperty("clearColor").get<StudioColor>(view.clearColor);

        if (const std::optional<WorldTransform> world = computeWorldTransform(scene, entity->getId()))
        {
            view.camera.setCenter(StudioVector2{world->position.x, world->position.y});
        }

        // Height, not width: `orthographicSize` is the visible height in world units, so the zoom
        // is pixels per world unit vertically and the width follows from the aspect. A resize then
        // shows more of the world rather than stretching what was already on screen.
        const float orthographicSize = camera->getProperty("orthographicSize").get<float>(600.0f);
        if (orthographicSize > 0.0f && viewportSize.y > 0.0f)
        {
            view.camera.setZoom(viewportSize.y / orthographicSize);
        }

        // And the 3D half (`plan.md` STUDIO-20007). `projection` has been on the descriptor since
        // Phase 1 and this function read every other property and ignored it, so a scene authored
        // in 3D was previewed through a 2D sprite pass that draws no models at all.
        view.perspective =
            camera->getProperty("projection").get<PropertyValue::EnumValue>().name == "Perspective";

        view.camera3D.setViewportSize(viewportSize);
        view.camera3D.setProjection(view.perspective ? CameraProjection::Perspective
                                                     : CameraProjection::Orthographic);
        view.camera3D.setFieldOfView(camera->getProperty("fieldOfView").get<float>(45.0f)
                                     * kDegreesToRadians);
        view.camera3D.setClipPlanes(camera->getProperty("nearPlane").get<float>(0.1f),
                                    camera->getProperty("farPlane").get<float>(1000.0f));

        if (const std::optional<WorldTransform> world =
                computeWorldTransform(scene, entity->getId()))
        {
            // The entity's own forward axis, +Z rotated by its rotation -- the one convention this
            // codebase has for which way an entity faces, established by `CNA.Light` and written
            // down in `SceneLighting.hpp`. Two answers to that question about the same transform
            // would be one answer too many.
            const StudioVector3 forward =
                normalize(rotate(world->rotation, StudioVector3{0.0f, 0.0f, 1.0f}));

            // `StudioCamera3D` is an orbit: it is positioned by a pivot, a distance and two
            // angles, and the eye is `pivot - forward * distance`. A game camera has no orbit, so
            // the pivot is put in front of it at a fixed distance and the eye lands exactly on the
            // entity. The distance is arbitrary for a perspective view and is *not* for an
            // orthographic one -- there the visible height is derived from it, so it is set from
            // `orthographicSize` instead and the two projections stay consistent with the 2D path.
            const float distance = view.perspective
                ? kGameCameraPivotDistance
                : orthographicHeightToDistance(orthographicSize, view.camera3D.getFieldOfView());

            view.camera3D.setDistance(distance);
            view.camera3D.setPitch(std::asin(std::clamp(forward.y, -1.0f, 1.0f)));
            view.camera3D.setYaw(std::atan2(-forward.x, -forward.z));
            view.camera3D.setPivot(add(world->position, scale(forward, distance)));
        }

        return view;
    }
}
