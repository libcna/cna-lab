// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/SceneModels.hpp"

#include <algorithm>

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"

namespace CNA::Studio
{
    MeshMaterial resolveMeshPartMaterial(const ModelDraw& draw, std::string_view partName,
                                         int materialIndex)
    {
        // The per-part list first, because it is the more specific answer: the single override
        // means "this whole model" and the list means "except these parts" (ED-410).
        for (const auto& [name, material] : draw.partMaterials)
        {
            if (name == partName) { return material; }
        }
        if (draw.materialOverride.has_value()) { return *draw.materialOverride; }

        // glTF says an unnamed material means the *default* material, so a part with none gets a
        // neutral one rather than being skipped.
        const bool named = draw.mesh != nullptr && materialIndex >= 0
                           && static_cast<std::size_t>(materialIndex) < draw.mesh->materials.size();
        return named ? draw.mesh->materials[static_cast<std::size_t>(materialIndex)]
                     : MeshMaterial{};
    }

    SceneDrawOrder orderSceneModelDraws(const SceneModelBatch& batch)
    {
        SceneDrawOrder order;

        // Paired with its depth so the sort has something to sort by, then unpaired: the caller
        // wants indices, and handing back the depths would be handing back a number nothing else
        // in the pass has any use for.
        std::vector<std::pair<float, std::size_t>> blended;

        for (std::size_t index = 0; index < batch.draws.size(); ++index)
        {
            const ModelDraw& draw = batch.draws[index];
            if (draw.mesh == nullptr) { continue; }

            bool hasOpaque = false;
            bool hasBlended = false;
            for (const MeshPart& part : draw.mesh->parts)
            {
                // Masked parts are drawn in the opaque pass: a cut-out is a hard edge, it writes
                // depth, and sorting it would be paying for an ordering it does not need.
                if (resolveMeshPartMaterial(draw, part).alphaMode == MeshAlphaMode::Blend)
                {
                    hasBlended = true;
                }
                else
                {
                    hasOpaque = true;
                }
            }

            if (hasOpaque) { order.opaque.push_back(index); }
            if (!hasBlended) { continue; }

            // View space, from the entity's own origin. An XNA view matrix looks down **-Z**, so
            // the further something is the more negative its view-space z -- the sort below is
            // ascending for that reason, not by oversight. The case that pins it places its panes
            // along the camera's own forward vector rather than along an axis, so it is asserting
            // "further is drawn first" rather than re-stating this sign.
            const StudioVector3 origin =
                transformPosition(draw.world, StudioVector3{0.0f, 0.0f, 0.0f});
            const StudioVector3 eyeSpace = transformPosition(batch.view, origin);
            blended.emplace_back(eyeSpace.z, index);
        }

        std::stable_sort(blended.begin(), blended.end(),
                         [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

        order.blended.reserve(blended.size());
        for (const auto& [depth, index] : blended)
        {
            (void)depth;
            order.blended.push_back(index);
        }

        return order;
    }

    void applyDebugViewToDraw(StudioDebugView debugView, ModelDraw& draw)
    {
        if (debugView == StudioDebugView::None || draw.mesh == nullptr) { return; }

        std::vector<std::pair<std::string, MeshMaterial>> replaced;
        replaced.reserve(draw.mesh->parts.size());
        for (const MeshPart& part : draw.mesh->parts)
        {
            replaced.emplace_back(part.name,
                                  studioDebugMaterial(debugView, resolveMeshPartMaterial(draw, part)));
        }

        // The per-part list carries the answer, and the model-wide override is set to the same
        // view of a default material so that a part the list does not name -- one whose name is
        // empty, which the importer does produce -- is still drawn in the debug view rather than
        // in its own colours beside it.
        draw.partMaterials = std::move(replaced);
        draw.materialOverride = studioDebugMaterial(debugView, MeshMaterial{});
        draw.lighting = studioDebugLighting(debugView, draw.lighting);
    }

    SceneModelBatch buildSceneModelBatch(const SceneDocument& scene, const StudioCamera3D& camera,
                                         const MeshProvider& meshProvider,
                                         const std::vector<Uuid>& selection,
                                         const MaterialProvider& materialProvider,
                                         StudioDebugView debugView)
    {
        SceneModelBatch batch;
        batch.environment = scene.getEnvironment();
        batch.viewProjection = camera.getViewProjectionMatrix();
        batch.view = camera.getViewMatrix();

        // The mirror lives in the projection (StudioCamera3D::getViewProjectionMatrix says why),
        // so it has to be folded in here too or the split would not multiply back to the product.
        batch.projection = multiply(camera.getProjectionMatrix(),
                                    createScale(StudioVector3{1.0f, -1.0f, 1.0f}));

        if (!meshProvider) { return batch; }

        // Collected once for the whole batch, resolved per draw. Collecting is a walk of every
        // entity; resolving is arithmetic over what that walk found, and doing the walk per model
        // would make a scene of a hundred models cost a hundred walks of itself.
        const std::vector<SceneLight> lights = collectSceneLights(scene);

        for (const StudioEntity& entity : scene.getEntities())
        {
            if (!entity.isEnabled()) { continue; }

            const StudioComponent* renderer = entity.findComponent(BuiltinComponentIds::kModelRenderer);
            if (renderer == nullptr) { continue; }

            const Uuid modelId = renderer->getProperty("model").get<PropertyValue::AssetReference>().id;
            if (!modelId.isValid()) { continue; }

            const std::optional<WorldTransform> world = computeWorldTransform(scene, entity.getId());
            if (!world.has_value()) { continue; }

            const MeshData* mesh = meshProvider(modelId);
            if (mesh == nullptr || mesh->isEmpty())
            {
                // Counted rather than dropped silently. "Still importing" and "this entity has no
                // model" look identical on screen, and only one of them is worth waiting for.
                ++batch.pendingMeshes;
                continue;
            }

            ModelDraw draw;
            draw.entityId = entity.getId();
            draw.modelId = modelId;
            draw.mesh = mesh;
            draw.world = toWorldMatrix(*world);
            draw.lighting = computeEffectLighting(lights, world->position);

            // The scene's ambient overrides the lighting's own default. It is a property of the
            // level -- a cave is dark -- so it belongs to the scene rather than to whichever
            // lights happen to be in it.
            draw.lighting.ambientColor =
                StudioVector3{static_cast<float>(scene.getEnvironment().ambientColor.r) / 255.0f,
                              static_cast<float>(scene.getEnvironment().ambientColor.g) / 255.0f,
                              static_cast<float>(scene.getEnvironment().ambientColor.b) / 255.0f};
            // The override, when the entity names one and it can be resolved. An entity pointing at
            // a material that has not loaded draws with its model's own rather than not at all --
            // the same answer `MeshProvider` returning nullptr gets from the mesh side.
            if (materialProvider)
            {
                const Uuid materialId =
                    renderer->getProperty("material").get<PropertyValue::AssetReference>().id;
                if (materialId.isValid()) { draw.materialOverride = materialProvider(materialId); }

                // ED-410: the per-part list, resolved the same way. Each entry is a structure of
                // a part name and a material reference -- which is what `PropertyType::Structure`
                // was finally built for, ED-311 having deliberately left it unbuilt until
                // something real asked.
                const PropertyValue& listValue = renderer->getProperty("materials");
                if (listValue.getType() == PropertyType::List)
                {
                    for (const PropertyValue& item : listValue.get<PropertyValue::ListValue>().items)
                    {
                        if (item.getType() != PropertyType::Structure) { continue; }

                        const auto& structure = item.get<PropertyValue::StructureValue>();
                        const PropertyValue* partName = structure.find("part");
                        const PropertyValue* partMaterial = structure.find("material");
                        if (partName == nullptr || partMaterial == nullptr) { continue; }

                        const std::string name = partName->get<std::string>();
                        const Uuid id = partMaterial->get<PropertyValue::AssetReference>().id;
                        if (name.empty() || !id.isValid()) { continue; }

                        if (const std::optional<MeshMaterial> resolved = materialProvider(id))
                        {
                            draw.partMaterials.emplace_back(name, *resolved);
                        }
                    }
                }
            }

            draw.selected =
                std::find(selection.begin(), selection.end(), entity.getId()) != selection.end();

            // Last, so it sees the materials every other rule has already resolved (STUDIO-11011).
            applyDebugViewToDraw(debugView, draw);

            batch.triangleCount += mesh->getTriangleCount();
            batch.draws.push_back(draw);
        }

        return batch;
    }

    const MeshData* findEntityMesh(const StudioEntity& entity, const MeshProvider& meshProvider)
    {
        if (!meshProvider) { return nullptr; }

        const StudioComponent* renderer = entity.findComponent(BuiltinComponentIds::kModelRenderer);
        if (renderer == nullptr) { return nullptr; }

        const Uuid modelId = renderer->getProperty("model").get<PropertyValue::AssetReference>().id;
        if (!modelId.isValid()) { return nullptr; }

        return meshProvider(modelId);
    }
}
