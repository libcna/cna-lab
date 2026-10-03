package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeModelExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeModelExtensionRoutes {

    private NativeModelExtensionRoutes() {
    }

    /**
     * cna_animation_player_copy_bone_transforms (models.h).
     */
    public static native int animationPlayerCopyBoneTransforms(long player, float[] destinationFloating, long[] outCount);

    /**
     * cna_animation_player_copy_current_clip_name (models.h).
     */
    public static native int animationPlayerCopyCurrentClipName(long player, byte[] destination, long[] outByteCount);

    /**
     * cna_animation_player_copy_skin_transforms (models.h).
     */
    public static native int animationPlayerCopySkinTransforms(long player, float[] destinationFloating, long[] outCount);

    /**
     * cna_animation_player_copy_world_transforms (models.h).
     */
    public static native int animationPlayerCopyWorldTransforms(long player, float[] destinationFloating, long[] outCount);

    /**
     * cna_animation_player_create (models.h).
     */
    public static native int animationPlayerCreate(long data, long[] outPlayer);

    /**
     * cna_animation_player_destroy (models.h).
     */
    public static native int animationPlayerDestroy(long player);

    /**
     * cna_animation_player_get_current_clip_info (models.h).
     */
    public static native int animationPlayerGetCurrentClipInfo(long player, boolean[] outHasClip, double[] outDurationSeconds, long[] outTrackCount);

    /**
     * cna_animation_player_get_current_clip_name_byte_count (models.h).
     */
    public static native int animationPlayerGetCurrentClipNameByteCount(long player, long[] outByteCount);

    /**
     * cna_animation_player_get_current_position (models.h).
     */
    public static native int animationPlayerGetCurrentPosition(long player, double[] outPositionSeconds);

    /**
     * cna_animation_player_start_clip (models.h).
     */
    public static native int animationPlayerStartClip(long player, byte[] clipName);

    /**
     * cna_animation_player_update (models.h).
     */
    public static native int animationPlayerUpdate(long player, double timeSeconds, boolean relativeToCurrentTime, boolean loop);

    /**
     * cna_content_manager_load_model (models.h).
     */
    public static native int contentManagerLoadModel(long contentManager, byte[] assetName, long[] outModel);

    /**
     * cna_effect_copy_type_name (effects.h).
     */
    public static native int effectCopyTypeName(long effect, byte[] destination, long[] outByteCount);

    /**
     * cna_effect_get_type_name_byte_count (effects.h).
     */
    public static native int effectGetTypeNameByteCount(long effect, long[] outByteCount);

    /**
     * cna_matrix_create_infinite_perspective_field_of_view_ext (models.h).
     *
     * <p>outMatrixFloating carries CNA_Matrix in this order:
     * <ol start="0">
     *   <li>{@code m11} (float)</li>
     *   <li>{@code m12} (float)</li>
     *   <li>{@code m13} (float)</li>
     *   <li>{@code m14} (float)</li>
     *   <li>{@code m21} (float)</li>
     *   <li>{@code m22} (float)</li>
     *   <li>{@code m23} (float)</li>
     *   <li>{@code m24} (float)</li>
     *   <li>{@code m31} (float)</li>
     *   <li>{@code m32} (float)</li>
     *   <li>{@code m33} (float)</li>
     *   <li>{@code m34} (float)</li>
     *   <li>{@code m41} (float)</li>
     *   <li>{@code m42} (float)</li>
     *   <li>{@code m43} (float)</li>
     *   <li>{@code m44} (float)</li>
     * </ol>
     */
    public static native int matrixCreateInfinitePerspectiveFieldOfViewExt(float fieldOfView, float aspectRatio, float nearPlaneDistance, float[] outMatrixFloating);

    /**
     * cna_model_add_camera_ext (models.h).
     *
     * <p>descriptorIntegral carries CNA_ModelCameraDescriptorEXT in this order:
     * <ol start="0">
     *   <li>{@code camera.scene_node_index} (int32_t)</li>
     *   <li>{@code camera.is_perspective} (CNA_Bool)</li>
     *   <li>{@code camera.has_infinite_far_plane} (CNA_Bool)</li>
     *   <li>{@code camera.has_authored_aspect_ratio} (CNA_Bool)</li>
     * </ol>
     *
     * <p>descriptorFloating carries CNA_ModelCameraDescriptorEXT in this order:
     * <ol start="0">
     *   <li>{@code camera.projection.m11} (float)</li>
     *   <li>{@code camera.projection.m12} (float)</li>
     *   <li>{@code camera.projection.m13} (float)</li>
     *   <li>{@code camera.projection.m14} (float)</li>
     *   <li>{@code camera.projection.m21} (float)</li>
     *   <li>{@code camera.projection.m22} (float)</li>
     *   <li>{@code camera.projection.m23} (float)</li>
     *   <li>{@code camera.projection.m24} (float)</li>
     *   <li>{@code camera.projection.m31} (float)</li>
     *   <li>{@code camera.projection.m32} (float)</li>
     *   <li>{@code camera.projection.m33} (float)</li>
     *   <li>{@code camera.projection.m34} (float)</li>
     *   <li>{@code camera.projection.m41} (float)</li>
     *   <li>{@code camera.projection.m42} (float)</li>
     *   <li>{@code camera.projection.m43} (float)</li>
     *   <li>{@code camera.projection.m44} (float)</li>
     *   <li>{@code camera.world_transform.m11} (float)</li>
     *   <li>{@code camera.world_transform.m12} (float)</li>
     *   <li>{@code camera.world_transform.m13} (float)</li>
     *   <li>{@code camera.world_transform.m14} (float)</li>
     *   <li>{@code camera.world_transform.m21} (float)</li>
     *   <li>{@code camera.world_transform.m22} (float)</li>
     *   <li>{@code camera.world_transform.m23} (float)</li>
     *   <li>{@code camera.world_transform.m24} (float)</li>
     *   <li>{@code camera.world_transform.m31} (float)</li>
     *   <li>{@code camera.world_transform.m32} (float)</li>
     *   <li>{@code camera.world_transform.m33} (float)</li>
     *   <li>{@code camera.world_transform.m34} (float)</li>
     *   <li>{@code camera.world_transform.m41} (float)</li>
     *   <li>{@code camera.world_transform.m42} (float)</li>
     *   <li>{@code camera.world_transform.m43} (float)</li>
     *   <li>{@code camera.world_transform.m44} (float)</li>
     *   <li>{@code camera.aspect_ratio} (float)</li>
     *   <li>{@code camera.field_of_view} (float)</li>
     *   <li>{@code camera.near_plane_distance} (float)</li>
     *   <li>{@code camera.far_plane_distance} (float)</li>
     * </ol>
     *
     * <p>descriptorName carries CNA_ModelCameraDescriptorEXT.name as UTF-8 bytes, borrowed for the call.
     */
    public static native int modelAddCameraExt(long model, long[] descriptorIntegral, float[] descriptorFloating, byte[] descriptorName);

    /**
     * cna_model_add_skin_ext (models.h).
     */
    public static native int modelAddSkinExt(long model, byte[] name, long data, long[] meshIndices);

    /**
     * cna_model_animations_ext_copy_clip_name_at (models.h).
     */
    public static native int modelAnimationsCopyClipNameAt(long animations, long clipIndex, byte[] destination, long[] outByteCount);

    /**
     * cna_model_animations_ext_copy_type_name (models.h).
     */
    public static native int modelAnimationsCopyTypeName(long animations, byte[] destination, long[] outByteCount);

    /**
     * cna_model_animations_ext_destroy (models.h).
     */
    public static native int modelAnimationsDestroy(long animations);

    /**
     * cna_model_animations_ext_get_clip_count (models.h).
     */
    public static native int modelAnimationsGetClipCount(long animations, long[] outCount);

    /**
     * cna_model_animations_ext_get_clip_info_at (models.h).
     */
    public static native int modelAnimationsGetClipInfoAt(long animations, long clipIndex, double[] outDurationSeconds, long[] outTrackCount, int[] outTargetSpace);

    /**
     * cna_model_animations_ext_get_clip_name_byte_count_at (models.h).
     */
    public static native int modelAnimationsGetClipNameByteCountAt(long animations, long clipIndex, long[] outByteCount);

    /**
     * cna_model_animations_ext_get_type_name_byte_count (models.h).
     */
    public static native int modelAnimationsGetTypeNameByteCount(long animations, long[] outByteCount);

    /**
     * cna_model_animations_ext_set_clip_target_space_at (models.h).
     */
    public static native int modelAnimationsSetClipTargetSpaceAt(long animations, long clipIndex, int value);

    /**
     * cna_model_apply_bind_pose_bone_transforms_ext (models.h).
     */
    public static native int modelApplyBindPoseBoneTransformsExt(long model, long data, long[] outPosedCount);

    /**
     * cna_model_apply_clip_to_bones_ext (models.h).
     */
    public static native int modelApplyClipToBonesExt(long model, long animations, long clipIndex, double timeSeconds);

    /**
     * cna_model_bone_add_child (models.h).
     */
    public static native int modelBoneAddChild(long bone, long child);

    /**
     * cna_model_bone_collection_destroy (models.h).
     */
    public static native int modelBoneCollectionDestroy(long collection);

    /**
     * cna_model_bone_collection_get_at (models.h).
     */
    public static native int modelBoneCollectionGetAt(long collection, long index, long[] outBone);

    /**
     * cna_model_bone_collection_get_count (models.h).
     */
    public static native int modelBoneCollectionGetCount(long collection, long[] outCount);

    /**
     * cna_model_bone_copy_name (models.h).
     */
    public static native int modelBoneCopyName(long bone, byte[] destination, long[] outByteCount);

    /**
     * cna_model_bone_create (models.h).
     */
    public static native int modelBoneCreate(int index, byte[] name, long[] outBone);

    /**
     * cna_model_bone_destroy (models.h).
     */
    public static native int modelBoneDestroy(long bone);

    /**
     * cna_model_bone_get_children (models.h).
     */
    public static native int modelBoneGetChildren(long bone, long[] outChildren);

    /**
     * cna_model_bone_get_index (models.h).
     */
    public static native int modelBoneGetIndex(long bone, int[] outIndex);

    /**
     * cna_model_bone_get_name_byte_count (models.h).
     */
    public static native int modelBoneGetNameByteCount(long bone, long[] outByteCount);

    /**
     * cna_model_bone_get_parent (models.h).
     */
    public static native int modelBoneGetParent(long bone, boolean[] outHasParent, long[] outParent);

    /**
     * cna_model_bone_get_transform (models.h).
     *
     * <p>outTransformFloating carries CNA_Matrix in this order:
     * <ol start="0">
     *   <li>{@code m11} (float)</li>
     *   <li>{@code m12} (float)</li>
     *   <li>{@code m13} (float)</li>
     *   <li>{@code m14} (float)</li>
     *   <li>{@code m21} (float)</li>
     *   <li>{@code m22} (float)</li>
     *   <li>{@code m23} (float)</li>
     *   <li>{@code m24} (float)</li>
     *   <li>{@code m31} (float)</li>
     *   <li>{@code m32} (float)</li>
     *   <li>{@code m33} (float)</li>
     *   <li>{@code m34} (float)</li>
     *   <li>{@code m41} (float)</li>
     *   <li>{@code m42} (float)</li>
     *   <li>{@code m43} (float)</li>
     *   <li>{@code m44} (float)</li>
     * </ol>
     */
    public static native int modelBoneGetTransform(long bone, float[] outTransformFloating);

    /**
     * cna_model_bone_set_transform (models.h).
     */
    public static native int modelBoneSetTransform(long bone, float[] transformFloating);

    /**
     * cna_model_clear_cameras_ext (models.h).
     */
    public static native int modelClearCamerasExt(long model);

    /**
     * cna_model_clear_skins_ext (models.h).
     */
    public static native int modelClearSkinsExt(long model);

    /**
     * cna_model_copy_absolute_bone_transforms (models.h).
     */
    public static native int modelCopyAbsoluteBoneTransforms(long model, float[] destinationFloating, long[] outCount);

    /**
     * cna_model_copy_bone_transforms (models.h).
     */
    public static native int modelCopyBoneTransforms(long model, float[] destinationFloating, long[] outCount);

    /**
     * cna_model_copy_camera_name_ext (models.h).
     */
    public static native int modelCopyCameraNameExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_gltf_import_diagnostic_code_ext (models.h).
     */
    public static native int modelCopyGltfImportDiagnosticCodeExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_gltf_import_diagnostic_detail_ext (models.h).
     */
    public static native int modelCopyGltfImportDiagnosticDetailExt(long model, long index, long detailIndex, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_gltf_import_diagnostic_message_ext (models.h).
     */
    public static native int modelCopyGltfImportDiagnosticMessageExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_gltf_import_diagnostic_subject_ext (models.h).
     */
    public static native int modelCopyGltfImportDiagnosticSubjectExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_material_variant_name_ext (models.h).
     */
    public static native int modelCopyMaterialVariantNameExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_copy_skin_name_ext (models.h).
     */
    public static native int modelCopySkinNameExt(long model, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_model_create_skin_skeleton_handle_ext (models.h).
     */
    public static native int modelCreateSkinSkeletonHandleExt(long model, long index, long[] outData);

    /**
     * cna_model_create_with_parents (models.h).
     */
    public static native int modelCreateWithParents(long graphicsDevice, long[] bones, long[] meshes, long[] meshParents, long rootBoneIndex, long[] outModel);

    /**
     * cna_model_destroy (models.h).
     */
    public static native int modelDestroy(long model);

    /**
     * cna_model_draw (models.h).
     */
    public static native int modelDraw(long model, float[] worldFloating, float[] viewFloating, float[] projectionFloating);

    /**
     * cna_model_effect_collection_destroy (models.h).
     */
    public static native int modelEffectCollectionDestroy(long collection);

    /**
     * cna_model_effect_collection_get_at (models.h).
     */
    public static native int modelEffectCollectionGetAt(long collection, long index, long[] outEffect);

    /**
     * cna_model_effect_collection_get_count (models.h).
     */
    public static native int modelEffectCollectionGetCount(long collection, long[] outCount);

    /**
     * cna_model_get_bone_transform_count (models.h).
     */
    public static native int modelGetBoneTransformCount(long model, long[] outCount);

    /**
     * cna_model_get_bones (models.h).
     */
    public static native int modelGetBones(long model, long[] outBones);

    /**
     * cna_model_get_bounding_sphere_ext (models.h).
     *
     * <p>outSphereFloating carries CNA_BoundingSphere in this order:
     * <ol start="0">
     *   <li>{@code center.x} (float)</li>
     *   <li>{@code center.y} (float)</li>
     *   <li>{@code center.z} (float)</li>
     *   <li>{@code radius} (float)</li>
     * </ol>
     */
    public static native int modelGetBoundingSphereExt(long model, boolean[] outHasValue, float[] outSphereFloating);

    /**
     * cna_model_get_camera_count_ext (models.h).
     */
    public static native int modelGetCameraCountExt(long model, long[] outCount);

    /**
     * cna_model_get_camera_ext (models.h).
     *
     * <p>outCameraIntegral carries CNA_ModelCameraEXT in this order:
     * <ol start="0">
     *   <li>{@code scene_node_index} (int32_t)</li>
     *   <li>{@code is_perspective} (CNA_Bool)</li>
     *   <li>{@code has_infinite_far_plane} (CNA_Bool)</li>
     *   <li>{@code has_authored_aspect_ratio} (CNA_Bool)</li>
     * </ol>
     *
     * <p>outCameraFloating carries CNA_ModelCameraEXT in this order:
     * <ol start="0">
     *   <li>{@code projection.m11} (float)</li>
     *   <li>{@code projection.m12} (float)</li>
     *   <li>{@code projection.m13} (float)</li>
     *   <li>{@code projection.m14} (float)</li>
     *   <li>{@code projection.m21} (float)</li>
     *   <li>{@code projection.m22} (float)</li>
     *   <li>{@code projection.m23} (float)</li>
     *   <li>{@code projection.m24} (float)</li>
     *   <li>{@code projection.m31} (float)</li>
     *   <li>{@code projection.m32} (float)</li>
     *   <li>{@code projection.m33} (float)</li>
     *   <li>{@code projection.m34} (float)</li>
     *   <li>{@code projection.m41} (float)</li>
     *   <li>{@code projection.m42} (float)</li>
     *   <li>{@code projection.m43} (float)</li>
     *   <li>{@code projection.m44} (float)</li>
     *   <li>{@code world_transform.m11} (float)</li>
     *   <li>{@code world_transform.m12} (float)</li>
     *   <li>{@code world_transform.m13} (float)</li>
     *   <li>{@code world_transform.m14} (float)</li>
     *   <li>{@code world_transform.m21} (float)</li>
     *   <li>{@code world_transform.m22} (float)</li>
     *   <li>{@code world_transform.m23} (float)</li>
     *   <li>{@code world_transform.m24} (float)</li>
     *   <li>{@code world_transform.m31} (float)</li>
     *   <li>{@code world_transform.m32} (float)</li>
     *   <li>{@code world_transform.m33} (float)</li>
     *   <li>{@code world_transform.m34} (float)</li>
     *   <li>{@code world_transform.m41} (float)</li>
     *   <li>{@code world_transform.m42} (float)</li>
     *   <li>{@code world_transform.m43} (float)</li>
     *   <li>{@code world_transform.m44} (float)</li>
     *   <li>{@code aspect_ratio} (float)</li>
     *   <li>{@code field_of_view} (float)</li>
     *   <li>{@code near_plane_distance} (float)</li>
     *   <li>{@code far_plane_distance} (float)</li>
     * </ol>
     */
    public static native int modelGetCameraExt(long model, long index, long[] outCameraIntegral, float[] outCameraFloating);

    /**
     * cna_model_get_camera_name_byte_count_ext (models.h).
     */
    public static native int modelGetCameraNameByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_get_content_tag_dictionary_ext (models.h).
     */
    public static native int modelGetContentTagDictionaryExt(long model, boolean[] outHasTag, long[] outDictionary);

    /**
     * cna_model_get_gltf_import_diagnostic_code_byte_count_ext (models.h).
     */
    public static native int modelGetGltfImportDiagnosticCodeByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_get_gltf_import_diagnostic_detail_byte_count_ext (models.h).
     */
    public static native int modelGetGltfImportDiagnosticDetailByteCountExt(long model, long index, long detailIndex, long[] outByteCount);

    /**
     * cna_model_get_gltf_import_diagnostic_ext (models.h).
     *
     * <p>outDiagnosticIntegral carries CNA_GltfImportDiagnosticEXT in this order:
     * <ol start="0">
     *   <li>{@code severity} (CNA_GltfImportDiagnosticSeverityEXT)</li>
     *   <li>{@code kind} (CNA_GltfImportDiagnosticKindEXT)</li>
     *   <li>{@code count} (uint64_t)</li>
     *   <li>{@code detail_count} (uint64_t)</li>
     * </ol>
     *
     * <p>outDiagnosticDoubles carries CNA_GltfImportDiagnosticEXT in this order:
     * <ol start="0">
     *   <li>{@code worst_magnitude} (double)</li>
     * </ol>
     */
    public static native int modelGetGltfImportDiagnosticExt(long model, long index, long[] outDiagnosticIntegral, double[] outDiagnosticDoubles);

    /**
     * cna_model_get_gltf_import_diagnostic_message_byte_count_ext (models.h).
     */
    public static native int modelGetGltfImportDiagnosticMessageByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_get_gltf_import_diagnostic_subject_byte_count_ext (models.h).
     */
    public static native int modelGetGltfImportDiagnosticSubjectByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_get_gltf_import_report_ext (models.h).
     *
     * <p>outReportIntegral carries CNA_GltfImportReportEXT in this order:
     * <ol start="0">
     *   <li>{@code node_count} (uint64_t)</li>
     *   <li>{@code mesh_instance_count} (uint64_t)</li>
     *   <li>{@code distinct_mesh_count} (uint64_t)</li>
     *   <li>{@code shared_mesh_count} (uint64_t)</li>
     *   <li>{@code max_node_depth} (uint64_t)</li>
     *   <li>{@code camera_node_count} (uint64_t)</li>
     *   <li>{@code light_node_count} (uint64_t)</li>
     *   <li>{@code imported_light_count} (uint64_t)</li>
     *   <li>{@code primitive_count} (uint64_t)</li>
     *   <li>{@code skin_count} (uint64_t)</li>
     *   <li>{@code animation_count} (uint64_t)</li>
     *   <li>{@code clip_count} (uint64_t)</li>
     *   <li>{@code diagnostic_count} (uint64_t)</li>
     *   <li>{@code warning_count} (uint64_t)</li>
     *   <li>{@code dropped_feature_count} (uint64_t)</li>
     *   <li>{@code approximation_count} (uint64_t)</li>
     *   <li>{@code anything_lost} (CNA_Bool)</li>
     * </ol>
     */
    public static native int modelGetGltfImportReportExt(long model, long[] outReportIntegral);

    /**
     * cna_model_get_material_variant_count_ext (models.h).
     */
    public static native int modelGetMaterialVariantCountExt(long model, long[] outCount);

    /**
     * cna_model_get_material_variant_ext (models.h).
     */
    public static native int modelGetMaterialVariantExt(long model, int[] outValue);

    /**
     * cna_model_get_material_variant_name_byte_count_ext (models.h).
     */
    public static native int modelGetMaterialVariantNameByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_get_meshes (models.h).
     */
    public static native int modelGetMeshes(long model, long[] outMeshes);

    /**
     * cna_model_get_root (models.h).
     */
    public static native int modelGetRoot(long model, boolean[] outHasRoot, long[] outRoot);

    /**
     * cna_model_get_skin_count_ext (models.h).
     */
    public static native int modelGetSkinCountExt(long model, long[] outCount);

    /**
     * cna_model_get_skin_ext (models.h).
     */
    public static native int modelGetSkinExt(long model, long index, boolean[] outHasData, long[] outMeshCount);

    /**
     * cna_model_get_skin_mesh_index_ext (models.h).
     */
    public static native int modelGetSkinMeshIndexExt(long model, long index, long meshIndex, long[] outModelMeshIndex);

    /**
     * cna_model_get_skin_name_byte_count_ext (models.h).
     */
    public static native int modelGetSkinNameByteCountExt(long model, long index, long[] outByteCount);

    /**
     * cna_model_mesh_collection_destroy (models.h).
     */
    public static native int modelMeshCollectionDestroy(long collection);

    /**
     * cna_model_mesh_collection_get_at (models.h).
     */
    public static native int modelMeshCollectionGetAt(long collection, long index, long[] outMesh);

    /**
     * cna_model_mesh_collection_get_count (models.h).
     */
    public static native int modelMeshCollectionGetCount(long collection, long[] outCount);

    /**
     * cna_model_mesh_copy_name (models.h).
     */
    public static native int modelMeshCopyName(long mesh, byte[] destination, long[] outByteCount);

    /**
     * cna_model_mesh_create_named (models.h).
     */
    public static native int modelMeshCreateNamed(long graphicsDevice, byte[] name, long[] parts, long[] outMesh);

    /**
     * cna_model_mesh_destroy (models.h).
     */
    public static native int modelMeshDestroy(long mesh);

    /**
     * cna_model_mesh_get_bounding_sphere (models.h).
     *
     * <p>outValueFloating carries CNA_BoundingSphere in this order:
     * <ol start="0">
     *   <li>{@code center.x} (float)</li>
     *   <li>{@code center.y} (float)</li>
     *   <li>{@code center.z} (float)</li>
     *   <li>{@code radius} (float)</li>
     * </ol>
     */
    public static native int modelMeshGetBoundingSphere(long mesh, float[] outValueFloating);

    /**
     * cna_model_mesh_get_effects (models.h).
     */
    public static native int modelMeshGetEffects(long mesh, long[] outEffects);

    /**
     * cna_model_mesh_get_mesh_parts (models.h).
     */
    public static native int modelMeshGetMeshParts(long mesh, long[] outParts);

    /**
     * cna_model_mesh_get_name_byte_count (models.h).
     */
    public static native int modelMeshGetNameByteCount(long mesh, long[] outByteCount);

    /**
     * cna_model_mesh_get_parent_bone (models.h).
     */
    public static native int modelMeshGetParentBone(long mesh, boolean[] outHasParent, long[] outParent);

    /**
     * cna_model_mesh_part_collection_destroy (models.h).
     */
    public static native int modelMeshPartCollectionDestroy(long collection);

    /**
     * cna_model_mesh_part_collection_get_at (models.h).
     */
    public static native int modelMeshPartCollectionGetAt(long collection, long index, long[] outPart);

    /**
     * cna_model_mesh_part_collection_get_count (models.h).
     */
    public static native int modelMeshPartCollectionGetCount(long collection, long[] outCount);

    /**
     * cna_model_mesh_part_create (models.h).
     */
    public static native int modelMeshPartCreate(long vertexBuffer, long indexBuffer, int numVertices, int primitiveCount, int startIndex, int vertexOffset, long[] outPart);

    /**
     * cna_model_mesh_part_destroy (models.h).
     */
    public static native int modelMeshPartDestroy(long part);

    /**
     * cna_model_mesh_part_get_effect (models.h).
     */
    public static native int modelMeshPartGetEffect(long part, boolean[] outHasEffect, long[] outEffect);

    /**
     * cna_model_mesh_part_get_index_buffer (models.h).
     */
    public static native int modelMeshPartGetIndexBuffer(long part, boolean[] outHasBuffer, long[] outBuffer);

    /**
     * cna_model_mesh_part_get_morph_target_data_ext (models.h).
     */
    public static native int modelMeshPartGetMorphTargetDataExt(long part, boolean[] outHasData, long[] outData);

    /**
     * cna_model_mesh_part_get_num_vertices (models.h).
     */
    public static native int modelMeshPartGetNumVertices(long part, int[] outValue);

    /**
     * cna_model_mesh_part_get_primitive_count (models.h).
     */
    public static native int modelMeshPartGetPrimitiveCount(long part, int[] outValue);

    /**
     * cna_model_mesh_part_get_primitive_type_ext (models.h).
     */
    public static native int modelMeshPartGetPrimitiveTypeExt(long part, int[] outValue);

    /**
     * cna_model_mesh_part_get_sampler_state_ext (models.h).
     *
     * <p>outStateIntegral carries CNA_SamplerState in this order:
     * <ol start="0">
     *   <li>{@code address_u} (CNA_TextureAddressMode)</li>
     *   <li>{@code address_v} (CNA_TextureAddressMode)</li>
     *   <li>{@code address_w} (CNA_TextureAddressMode)</li>
     *   <li>{@code filter} (CNA_TextureFilter)</li>
     *   <li>{@code max_anisotropy} (int32_t)</li>
     *   <li>{@code max_mip_level} (int32_t)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     * </ol>
     *
     * <p>outStateFloating carries CNA_SamplerState in this order:
     * <ol start="0">
     *   <li>{@code mip_map_level_of_detail_bias} (float)</li>
     * </ol>
     */
    public static native int modelMeshPartGetSamplerStateExt(long part, int slot, long[] outStateIntegral, float[] outStateFloating);

    /**
     * cna_model_mesh_part_get_start_index (models.h).
     */
    public static native int modelMeshPartGetStartIndex(long part, int[] outValue);

    /**
     * cna_model_mesh_part_get_vertex_buffer (models.h).
     */
    public static native int modelMeshPartGetVertexBuffer(long part, boolean[] outHasBuffer, long[] outBuffer);

    /**
     * cna_model_mesh_part_get_vertex_offset (models.h).
     */
    public static native int modelMeshPartGetVertexOffset(long part, int[] outValue);

    /**
     * cna_model_mesh_part_set_effect (models.h).
     */
    public static native int modelMeshPartSetEffect(long part, long effect);

    /**
     * cna_model_mesh_part_set_morph_target_data_ext (models.h).
     */
    public static native int modelMeshPartSetMorphTargetDataExt(long part, long data);

    /**
     * cna_model_mesh_part_set_morph_weights_ext (models.h).
     */
    public static native int modelMeshPartSetMorphWeightsExt(long part, float[] weights);

    /**
     * cna_model_mesh_part_set_primitive_type_ext (models.h).
     */
    public static native int modelMeshPartSetPrimitiveTypeExt(long part, int value);

    /**
     * cna_model_mesh_part_set_sampler_state_ext (models.h).
     *
     * <p>stateIntegral carries CNA_SamplerState in this order:
     * <ol start="0">
     *   <li>{@code address_u} (CNA_TextureAddressMode)</li>
     *   <li>{@code address_v} (CNA_TextureAddressMode)</li>
     *   <li>{@code address_w} (CNA_TextureAddressMode)</li>
     *   <li>{@code filter} (CNA_TextureFilter)</li>
     *   <li>{@code max_anisotropy} (int32_t)</li>
     *   <li>{@code max_mip_level} (int32_t)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     * </ol>
     *
     * <p>stateFloating carries CNA_SamplerState in this order:
     * <ol start="0">
     *   <li>{@code mip_map_level_of_detail_bias} (float)</li>
     * </ol>
     */
    public static native int modelMeshPartSetSamplerStateExt(long part, int slot, long[] stateIntegral, float[] stateFloating);

    /**
     * cna_model_mesh_set_bounding_sphere (models.h).
     */
    public static native int modelMeshSetBoundingSphere(long mesh, float[] valueFloating);

    /**
     * cna_model_mesh_set_parent_bone (models.h).
     */
    public static native int modelMeshSetParentBone(long mesh, long parent);

    /**
     * cna_model_set_bone_transforms (models.h).
     */
    public static native int modelSetBoneTransforms(long model, float[] sourceFloating);

    /**
     * cna_model_set_gltf_import_report_ext (models.h).
     *
     * <p>reportIntegral carries CNA_GltfImportReportEXT in this order:
     * <ol start="0">
     *   <li>{@code node_count} (uint64_t)</li>
     *   <li>{@code mesh_instance_count} (uint64_t)</li>
     *   <li>{@code distinct_mesh_count} (uint64_t)</li>
     *   <li>{@code shared_mesh_count} (uint64_t)</li>
     *   <li>{@code max_node_depth} (uint64_t)</li>
     *   <li>{@code camera_node_count} (uint64_t)</li>
     *   <li>{@code light_node_count} (uint64_t)</li>
     *   <li>{@code imported_light_count} (uint64_t)</li>
     *   <li>{@code primitive_count} (uint64_t)</li>
     *   <li>{@code skin_count} (uint64_t)</li>
     *   <li>{@code animation_count} (uint64_t)</li>
     *   <li>{@code clip_count} (uint64_t)</li>
     *   <li>{@code diagnostic_count} (uint64_t)</li>
     *   <li>{@code warning_count} (uint64_t)</li>
     *   <li>{@code dropped_feature_count} (uint64_t)</li>
     *   <li>{@code approximation_count} (uint64_t)</li>
     *   <li>{@code anything_lost} (CNA_Bool)</li>
     * </ol>
     */
    public static native int modelSetGltfImportReportExt(long model, long[] reportIntegral);

    /**
     * cna_model_set_material_variant_ext (models.h).
     */
    public static native int modelSetMaterialVariantExt(long model, int value);

    /**
     * cna_morph_target_data_ext_blend (models.h).
     */
    public static native int morphTargetDataBlend(long data, float[] weights, byte[] destination, long[] outByteCount);

    /**
     * cna_morph_target_data_ext_copy_base_vertex_bytes (models.h).
     */
    public static native int morphTargetDataCopyBaseVertexBytes(long data, byte[] destination, long[] outByteCount);

    /**
     * cna_morph_target_data_ext_copy_normal_deltas (models.h).
     */
    public static native int morphTargetDataCopyNormalDeltas(long data, long targetIndex, float[] destinationFloating, long[] outDeltaCount);

    /**
     * cna_morph_target_data_ext_copy_position_deltas (models.h).
     */
    public static native int morphTargetDataCopyPositionDeltas(long data, long targetIndex, float[] destinationFloating, long[] outDeltaCount);

    /**
     * cna_morph_target_data_ext_copy_tangent_deltas (models.h).
     */
    public static native int morphTargetDataCopyTangentDeltas(long data, long targetIndex, float[] destinationFloating, long[] outDeltaCount);

    /**
     * cna_morph_target_data_ext_copy_triangle_indices_ext (models.h).
     */
    public static native int morphTargetDataCopyTriangleIndicesExt(long data, int[] destination, long[] outIndexCount);

    /**
     * cna_morph_target_data_ext_copy_weight_keyframe (models.h).
     */
    public static native int morphTargetDataCopyWeightKeyframe(long data, long keyframeIndex, double[] outTimeSeconds, float[] weights, long[] outWeightCount, float[] inTangents, long[] outInTangentCount, float[] outTangents, long[] outOutTangentCount);

    /**
     * cna_morph_target_data_ext_copy_weights (models.h).
     */
    public static native int morphTargetDataCopyWeights(long data, float[] destination, long[] outWeightCount);

    /**
     * cna_morph_target_data_ext_destroy (models.h).
     */
    public static native int morphTargetDataDestroy(long data);

    /**
     * cna_morph_target_data_ext_get_base_vertex_byte_count (models.h).
     */
    public static native int morphTargetDataGetBaseVertexByteCount(long data, long[] outByteCount);

    /**
     * cna_morph_target_data_ext_get_recompute_flat_normals_ext (models.h).
     */
    public static native int morphTargetDataGetRecomputeFlatNormalsExt(long data, boolean[] outRecompute);

    /**
     * cna_morph_target_data_ext_get_stride (models.h).
     */
    public static native int morphTargetDataGetStride(long data, int[] outStride);

    /**
     * cna_morph_target_data_ext_get_target_count (models.h).
     */
    public static native int morphTargetDataGetTargetCount(long data, long[] outTargetCount);

    /**
     * cna_morph_target_data_ext_get_weight_track_info (models.h).
     */
    public static native int morphTargetDataGetWeightTrackInfo(long data, long[] outKeyframeCount, boolean[] outStepInterpolation, boolean[] outCubicSpline);

    /**
     * cna_morph_target_data_ext_set_recompute_flat_normals_ext (models.h).
     */
    public static native int morphTargetDataSetRecomputeFlatNormalsExt(long data, boolean recompute);

    /**
     * cna_morph_target_data_ext_set_tangent_deltas (models.h).
     */
    public static native int morphTargetDataSetTangentDeltas(long data, long targetIndex, float[] deltasFloating);

    /**
     * cna_morph_target_data_ext_set_triangle_indices_ext (models.h).
     */
    public static native int morphTargetDataSetTriangleIndicesExt(long data, int[] indices);

    /**
     * cna_morph_target_data_ext_set_weights (models.h).
     */
    public static native int morphTargetDataSetWeights(long data, float[] weights);

    /**
     * cna_skinned_model_ext_add_part (models.h).
     */
    public static native int skinnedModelAddPart(long model, byte[] name, long vertexBuffer, long indexBuffer, long part, long texture);

    /**
     * cna_skinned_model_ext_attach_parts (models.h).
     */
    public static native int skinnedModelAttachParts(long model, long other);

    /**
     * cna_skinned_model_ext_compute_bone_transforms (models.h).
     */
    public static native int skinnedModelComputeBoneTransforms(long model, byte[] clipName, double positionSeconds, boolean loop, float[] destinationFloating, long[] outBoneCount);

    /**
     * cna_skinned_model_ext_copy_bind_pose_local (models.h).
     */
    public static native int skinnedModelCopyBindPoseLocal(long model, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinned_model_ext_copy_clip_name_at (models.h).
     */
    public static native int skinnedModelCopyClipNameAt(long model, long clipIndex, byte[] destination, long[] outByteCount);

    /**
     * cna_skinned_model_ext_copy_clip_track (models.h).
     */
    public static native int skinnedModelCopyClipTrack(long model, byte[] name, long trackIndex, int[] outBoneIndex, float[] destinationFloating, double[] destinationDoubles, long[] outKeyframeCount);

    /**
     * cna_skinned_model_ext_copy_inverse_bind_pose_global (models.h).
     */
    public static native int skinnedModelCopyInverseBindPoseGlobal(long model, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinned_model_ext_copy_parent_bone_indices (models.h).
     */
    public static native int skinnedModelCopyParentBoneIndices(long model, int[] destination, long[] outCount);

    /**
     * cna_skinned_model_ext_copy_part_name_at (models.h).
     */
    public static native int skinnedModelCopyPartNameAt(long model, long partIndex, byte[] destination, long[] outByteCount);

    /**
     * cna_skinned_model_ext_create_default (models.h).
     */
    public static native int skinnedModelCreateDefault(long[] outModel);

    /**
     * cna_skinned_model_ext_destroy (models.h).
     */
    public static native int skinnedModelDestroy(long model);

    /**
     * cna_skinned_model_ext_get_bone_count (models.h).
     */
    public static native int skinnedModelGetBoneCount(long model, long[] outBoneCount);

    /**
     * cna_skinned_model_ext_get_clip_count (models.h).
     */
    public static native int skinnedModelGetClipCount(long model, long[] outClipCount);

    /**
     * cna_skinned_model_ext_get_clip_info (models.h).
     */
    public static native int skinnedModelGetClipInfo(long model, byte[] name, boolean[] outFound, double[] outDurationSeconds, long[] outTrackCount);

    /**
     * cna_skinned_model_ext_get_clip_name_byte_count_at (models.h).
     */
    public static native int skinnedModelGetClipNameByteCountAt(long model, long clipIndex, long[] outByteCount);

    /**
     * cna_skinned_model_ext_get_owned_resource_counts (models.h).
     */
    public static native int skinnedModelGetOwnedResourceCounts(long model, long[] outVertexBuffers, long[] outIndexBuffers, long[] outParts, long[] outTextures);

    /**
     * cna_skinned_model_ext_get_part_at (models.h).
     */
    public static native int skinnedModelGetPartAt(long model, long partIndex, long[] outPart, boolean[] outHasTexture, long[] outTexture);

    /**
     * cna_skinned_model_ext_get_part_count (models.h).
     */
    public static native int skinnedModelGetPartCount(long model, long[] outPartCount);

    /**
     * cna_skinned_model_ext_get_part_name_byte_count_at (models.h).
     */
    public static native int skinnedModelGetPartNameByteCountAt(long model, long partIndex, long[] outByteCount);

    /**
     * cna_skinned_model_ext_remove_clip (models.h).
     */
    public static native int skinnedModelRemoveClip(long model, byte[] name);

    /**
     * cna_skinned_model_ext_remove_part (models.h).
     */
    public static native int skinnedModelRemovePart(long model, byte[] name);

    /**
     * cna_skinning_data_copy_bind_pose (models.h).
     */
    public static native int skinningDataCopyBindPose(long data, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinning_data_copy_clip_name_at (models.h).
     */
    public static native int skinningDataCopyClipNameAt(long data, long clipIndex, byte[] destination, long[] outByteCount);

    /**
     * cna_skinning_data_copy_clip_track (models.h).
     */
    public static native int skinningDataCopyClipTrack(long data, byte[] name, long trackIndex, int[] outBoneIndex, float[] destinationFloating, double[] destinationDoubles, long[] outKeyframeCount);

    /**
     * cna_skinning_data_copy_inverse_bind_pose (models.h).
     */
    public static native int skinningDataCopyInverseBindPose(long data, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinning_data_copy_skeleton_hierarchy (models.h).
     */
    public static native int skinningDataCopySkeletonHierarchy(long data, int[] destination, long[] outCount);

    /**
     * cna_skinning_data_copy_skeleton_root_name_ext (models.h).
     */
    public static native int skinningDataCopySkeletonRootNameExt(long data, byte[] destination, long[] outByteCount);

    /**
     * cna_skinning_data_copy_skeleton_root_prefix (models.h).
     */
    public static native int skinningDataCopySkeletonRootPrefix(long data, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinning_data_destroy (models.h).
     */
    public static native int skinningDataDestroy(long data);

    /**
     * cna_skinning_data_get_bone_count (models.h).
     */
    public static native int skinningDataGetBoneCount(long data, long[] outBoneCount);

    /**
     * cna_skinning_data_get_clip_count (models.h).
     */
    public static native int skinningDataGetClipCount(long data, long[] outClipCount);

    /**
     * cna_skinning_data_get_clip_info (models.h).
     */
    public static native int skinningDataGetClipInfo(long data, byte[] name, boolean[] outFound, double[] outDurationSeconds, long[] outTrackCount);

    /**
     * cna_skinning_data_get_clip_name_byte_count_at (models.h).
     */
    public static native int skinningDataGetClipNameByteCountAt(long data, long clipIndex, long[] outByteCount);

    /**
     * cna_skinning_data_get_clip_target_space_ext (models.h).
     */
    public static native int skinningDataGetClipTargetSpaceExt(long data, long clipIndex, int[] outValue);

    /**
     * cna_skinning_data_get_skeleton_root_name_byte_count_ext (models.h).
     */
    public static native int skinningDataGetSkeletonRootNameByteCountExt(long data, long[] outByteCount);

    /**
     * cna_skinning_data_get_skeleton_root_node_index_ext (models.h).
     */
    public static native int skinningDataGetSkeletonRootNodeIndexExt(long data, int[] outValue);

    /**
     * cna_skinning_data_set_clip_target_space_ext (models.h).
     */
    public static native int skinningDataSetClipTargetSpaceExt(long data, long clipIndex, int value);

    /**
     * cna_skinning_data_set_skeleton_root_name_ext (models.h).
     */
    public static native int skinningDataSetSkeletonRootNameExt(long data, byte[] name);

    /**
     * cna_skinning_data_set_skeleton_root_node_index_ext (models.h).
     */
    public static native int skinningDataSetSkeletonRootNodeIndexExt(long data, int value);
}
