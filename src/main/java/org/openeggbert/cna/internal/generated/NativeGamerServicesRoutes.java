package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeGamerServicesRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeGamerServicesRoutes {

    private NativeGamerServicesRoutes() {
    }

    /**
     * cna_achievement_collection_contains (gamer_services.h).
     */
    public static native int achievementCollectionContains(long collection, long achievement, boolean[] outContains);

    /**
     * cna_achievement_collection_destroy (gamer_services.h).
     */
    public static native int achievementCollectionDestroy(long collection);

    /**
     * cna_achievement_collection_get_at (gamer_services.h).
     */
    public static native int achievementCollectionGetAt(long collection, int index, long[] outAchievement);

    /**
     * cna_achievement_collection_get_by_key (gamer_services.h).
     */
    public static native int achievementCollectionGetByKey(long collection, byte[] key, long[] outAchievement);

    /**
     * cna_achievement_collection_get_count (gamer_services.h).
     */
    public static native int achievementCollectionGetCount(long collection, int[] outCount);

    /**
     * cna_achievement_collection_get_is_disposed (gamer_services.h).
     */
    public static native int achievementCollectionGetIsDisposed(long collection, boolean[] outIsDisposed);

    /**
     * cna_achievement_collection_index_of (gamer_services.h).
     */
    public static native int achievementCollectionIndexOf(long collection, long achievement, int[] outIndex);

    /**
     * cna_achievement_copy_description (gamer_services.h).
     */
    public static native int achievementCopyDescription(long achievement, byte[] destination, long[] outBytes);

    /**
     * cna_achievement_copy_how_to_earn (gamer_services.h).
     */
    public static native int achievementCopyHowToEarn(long achievement, byte[] destination, long[] outBytes);

    /**
     * cna_achievement_copy_key (gamer_services.h).
     */
    public static native int achievementCopyKey(long achievement, byte[] destination, long[] outBytes);

    /**
     * cna_achievement_copy_name (gamer_services.h).
     */
    public static native int achievementCopyName(long achievement, byte[] destination, long[] outBytes);

    /**
     * cna_achievement_copy_picture (gamer_services.h).
     */
    public static native int achievementCopyPicture(long achievement, byte[] destination, long[] outBytes);

    /**
     * cna_achievement_get_description_size (gamer_services.h).
     */
    public static native int achievementGetDescriptionSize(long achievement, long[] outBytes);

    /**
     * cna_achievement_get_how_to_earn_size (gamer_services.h).
     */
    public static native int achievementGetHowToEarnSize(long achievement, long[] outBytes);

    /**
     * cna_achievement_get_info (gamer_services.h).
     *
     * <p>outInfoIntegral carries CNA_AchievementInfo in this order:
     * <ol start="0">
     *   <li>{@code gamer_score} (int32_t)</li>
     *   <li>{@code display_before_earned} (CNA_Bool)</li>
     *   <li>{@code earned_online} (CNA_Bool)</li>
     *   <li>{@code is_earned} (CNA_Bool)</li>
     *   <li>{@code reserved} (uint8_t)</li>
     *   <li>{@code earned_date_time_ticks} (int64_t)</li>
     * </ol>
     */
    public static native int achievementGetInfo(long achievement, long[] outInfoIntegral);

    /**
     * cna_achievement_get_key_size (gamer_services.h).
     */
    public static native int achievementGetKeySize(long achievement, long[] outBytes);

    /**
     * cna_achievement_get_name_size (gamer_services.h).
     */
    public static native int achievementGetNameSize(long achievement, long[] outBytes);

    /**
     * cna_achievement_get_picture_size (gamer_services.h).
     */
    public static native int achievementGetPictureSize(long achievement, long[] outBytes);

    /**
     * cna_avatar_animation_create (gamer_services.h).
     */
    public static native int avatarAnimationCreate(int preset, long[] outAnimation);

    /**
     * cna_avatar_animation_destroy (gamer_services.h).
     */
    public static native int avatarAnimationDestroy(long animation);

    /**
     * cna_avatar_animation_get_bone_transform_at (gamer_services.h).
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
    public static native int avatarAnimationGetBoneTransformAt(long animation, int index, float[] outTransformFloating);

    /**
     * cna_avatar_animation_get_expression (gamer_services.h).
     *
     * <p>outExpressionIntegral carries CNA_AvatarExpression in this order:
     * <ol start="0">
     *   <li>{@code mouth} (CNA_AvatarMouth)</li>
     *   <li>{@code left_eye} (CNA_AvatarEye)</li>
     *   <li>{@code right_eye} (CNA_AvatarEye)</li>
     *   <li>{@code left_eyebrow} (CNA_AvatarEyebrow)</li>
     *   <li>{@code right_eyebrow} (CNA_AvatarEyebrow)</li>
     * </ol>
     */
    public static native int avatarAnimationGetExpression(long animation, long[] outExpressionIntegral);

    /**
     * cna_avatar_animation_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_AvatarAnimationInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_AvatarAnimationInfo in this order:
     * <ol start="0">
     *   <li>{@code bone_transform_count} (int32_t)</li>
     *   <li>{@code is_disposed} (CNA_Bool)</li>
     *   <li>{@code current_position_ticks} (int64_t)</li>
     *   <li>{@code length_ticks} (int64_t)</li>
     * </ol>
     */
    public static native int avatarAnimationGetInfo(long animation, byte[] outInfoBytes, long[] outInfoIntegral);

    /**
     * cna_avatar_animation_set_current_position (gamer_services.h).
     */
    public static native int avatarAnimationSetCurrentPosition(long animation, long positionTicks);

    /**
     * cna_avatar_animation_update (gamer_services.h).
     */
    public static native int avatarAnimationUpdate(long animation, long elapsedTicks, boolean loop);

    /**
     * cna_avatar_description_copy_description (gamer_services.h).
     */
    public static native int avatarDescriptionCopyDescription(long description, byte[] destination, long[] outBytes);

    /**
     * cna_avatar_description_create (gamer_services.h).
     */
    public static native int avatarDescriptionCreate(byte[] description, long[] outDescription);

    /**
     * cna_avatar_description_create_random (gamer_services.h).
     */
    public static native int avatarDescriptionCreateRandom(long[] outDescription);

    /**
     * cna_avatar_description_create_random_for_body_type (gamer_services.h).
     */
    public static native int avatarDescriptionCreateRandomForBodyType(int bodyType, long[] outDescription);

    /**
     * cna_avatar_description_destroy (gamer_services.h).
     */
    public static native int avatarDescriptionDestroy(long description);

    /**
     * cna_avatar_description_get_from_gamer (gamer_services.h).
     */
    public static native int avatarDescriptionGetFromGamer(long gamer, long[] outDescription);

    /**
     * cna_avatar_description_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_AvatarDescriptionInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     *   <li>{@code reserved[3]} (uint8_t)</li>
     *   <li>{@code reserved[4]} (uint8_t)</li>
     *   <li>{@code reserved[5]} (uint8_t)</li>
     *   <li>{@code reserved[6]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_AvatarDescriptionInfo in this order:
     * <ol start="0">
     *   <li>{@code body_type} (CNA_AvatarBodyType)</li>
     *   <li>{@code description_byte_count} (uint64_t)</li>
     *   <li>{@code is_valid} (CNA_Bool)</li>
     * </ol>
     *
     * <p>outInfoFloating carries CNA_AvatarDescriptionInfo in this order:
     * <ol start="0">
     *   <li>{@code height} (float)</li>
     * </ol>
     */
    public static native int avatarDescriptionGetInfo(long description, byte[] outInfoBytes, long[] outInfoIntegral, float[] outInfoFloating);

    /**
     * cna_avatar_renderer_create (gamer_services.h).
     */
    public static native int avatarRendererCreate(long description, boolean useLoadingEffect, long[] outRenderer);

    /**
     * cna_avatar_renderer_destroy (gamer_services.h).
     */
    public static native int avatarRendererDestroy(long renderer);

    /**
     * cna_avatar_renderer_draw_animation (gamer_services.h).
     */
    public static native int avatarRendererDrawAnimation(long renderer, long animation);

    /**
     * cna_avatar_renderer_draw_bones (gamer_services.h).
     *
     * <p>expressionIntegral carries CNA_AvatarExpression in this order:
     * <ol start="0">
     *   <li>{@code mouth} (CNA_AvatarMouth)</li>
     *   <li>{@code left_eye} (CNA_AvatarEye)</li>
     *   <li>{@code right_eye} (CNA_AvatarEye)</li>
     *   <li>{@code left_eyebrow} (CNA_AvatarEyebrow)</li>
     *   <li>{@code right_eyebrow} (CNA_AvatarEyebrow)</li>
     * </ol>
     */
    public static native int avatarRendererDrawBones(long renderer, float[] bonesFloating, long[] expressionIntegral);

    /**
     * cna_avatar_renderer_get_bind_pose_at (gamer_services.h).
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
    public static native int avatarRendererGetBindPoseAt(long renderer, int index, float[] outTransformFloating);

    /**
     * cna_avatar_renderer_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_AvatarRendererInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_AvatarRendererInfo in this order:
     * <ol start="0">
     *   <li>{@code state} (CNA_AvatarRendererState)</li>
     *   <li>{@code is_disposed} (CNA_Bool)</li>
     * </ol>
     */
    public static native int avatarRendererGetInfo(long renderer, byte[] outInfoBytes, long[] outInfoIntegral);

    /**
     * cna_avatar_renderer_get_lighting (gamer_services.h).
     *
     * <p>outLightColorFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     *
     * <p>outLightDirectionFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     *
     * <p>outAmbientLightColorFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int avatarRendererGetLighting(long renderer, float[] outLightColorFloating, float[] outLightDirectionFloating, float[] outAmbientLightColorFloating);

    /**
     * cna_avatar_renderer_get_parent_bone_at (gamer_services.h).
     */
    public static native int avatarRendererGetParentBoneAt(long renderer, int index, int[] outParentIndex);

    /**
     * cna_avatar_renderer_get_transforms (gamer_services.h).
     *
     * <p>outWorldFloating carries CNA_Matrix in this order:
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
     *
     * <p>outViewFloating carries CNA_Matrix in this order:
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
     *
     * <p>outProjectionFloating carries CNA_Matrix in this order:
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
    public static native int avatarRendererGetTransforms(long renderer, float[] outWorldFloating, float[] outViewFloating, float[] outProjectionFloating);

    /**
     * cna_avatar_renderer_set_lighting (gamer_services.h).
     *
     * <p>lightColorFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     *
     * <p>lightDirectionFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     *
     * <p>ambientLightColorFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int avatarRendererSetLighting(long renderer, float[] lightColorFloating, float[] lightDirectionFloating, float[] ambientLightColorFloating);

    /**
     * cna_avatar_renderer_set_transforms (gamer_services.h).
     *
     * <p>worldFloating carries CNA_Matrix in this order:
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
     *
     * <p>viewFloating carries CNA_Matrix in this order:
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
     *
     * <p>projectionFloating carries CNA_Matrix in this order:
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
    public static native int avatarRendererSetTransforms(long renderer, float[] worldFloating, float[] viewFloating, float[] projectionFloating);

    /**
     * cna_friend_collection_get_is_disposed (gamer_services.h).
     */
    public static native int friendCollectionGetIsDisposed(long collection, boolean[] outIsDisposed);

    /**
     * cna_friend_gamer_copy_presence (gamer_services.h).
     */
    public static native int friendGamerCopyPresence(long gamer, byte[] destination, long[] outBytes);

    /**
     * cna_friend_gamer_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_FriendGamerInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     *   <li>{@code reserved[3]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_FriendGamerInfo in this order:
     * <ol start="0">
     *   <li>{@code friend_request_received_from} (CNA_Bool)</li>
     *   <li>{@code friend_request_sent_to} (CNA_Bool)</li>
     *   <li>{@code has_voice} (CNA_Bool)</li>
     *   <li>{@code invite_accepted} (CNA_Bool)</li>
     *   <li>{@code invite_received_from} (CNA_Bool)</li>
     *   <li>{@code invite_rejected} (CNA_Bool)</li>
     *   <li>{@code invite_sent_to} (CNA_Bool)</li>
     *   <li>{@code is_away} (CNA_Bool)</li>
     *   <li>{@code is_busy} (CNA_Bool)</li>
     *   <li>{@code is_joinable} (CNA_Bool)</li>
     *   <li>{@code is_online} (CNA_Bool)</li>
     *   <li>{@code is_playing} (CNA_Bool)</li>
     * </ol>
     */
    public static native int friendGamerGetInfo(long gamer, byte[] outInfoBytes, long[] outInfoIntegral);

    /**
     * cna_friend_gamer_get_presence_size (gamer_services.h).
     */
    public static native int friendGamerGetPresenceSize(long gamer, long[] outBytes);

    /**
     * cna_gamer_collection_create_enumerator (gamer_services.h).
     */
    public static native int gamerCollectionCreateEnumerator(long collection, long[] outEnumerator);

    /**
     * cna_gamer_collection_destroy (gamer_services.h).
     */
    public static native int gamerCollectionDestroy(long collection);

    /**
     * cna_gamer_collection_get_at (gamer_services.h).
     */
    public static native int gamerCollectionGetAt(long collection, int index, long[] outGamer);

    /**
     * cna_gamer_collection_get_count (gamer_services.h).
     */
    public static native int gamerCollectionGetCount(long collection, int[] outCount);

    /**
     * cna_gamer_copy_display_name (gamer_services.h).
     */
    public static native int gamerCopyDisplayName(long gamer, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_copy_gamertag (gamer_services.h).
     */
    public static native int gamerCopyGamertag(long gamer, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_copy_partner_token (gamer_services.h).
     */
    public static native int gamerCopyPartnerToken(byte[] audienceUri, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_copy_text (gamer_services.h).
     */
    public static native int gamerCopyText(long gamer, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_enumerator_destroy (gamer_services.h).
     */
    public static native int gamerEnumeratorDestroy(long enumerator);

    /**
     * cna_gamer_enumerator_get_current (gamer_services.h).
     */
    public static native int gamerEnumeratorGetCurrent(long enumerator, long[] outGamer);

    /**
     * cna_gamer_enumerator_move_next (gamer_services.h).
     */
    public static native int gamerEnumeratorMoveNext(long enumerator, boolean[] outHasCurrent);

    /**
     * cna_gamer_get_display_name_size (gamer_services.h).
     */
    public static native int gamerGetDisplayNameSize(long gamer, long[] outBytes);

    /**
     * cna_gamer_get_from_gamertag (gamer_services.h).
     */
    public static native int gamerGetFromGamertag(byte[] gamertag, long[] outGamer);

    /**
     * cna_gamer_get_gamertag_size (gamer_services.h).
     */
    public static native int gamerGetGamertagSize(long gamer, long[] outBytes);

    /**
     * cna_gamer_get_is_disposed (gamer_services.h).
     */
    public static native int gamerGetIsDisposed(long gamer, boolean[] outIsDisposed);

    /**
     * cna_gamer_get_partner_token_size (gamer_services.h).
     */
    public static native int gamerGetPartnerTokenSize(byte[] audienceUri, long[] outBytes);

    /**
     * cna_gamer_get_profile (gamer_services.h).
     */
    public static native int gamerGetProfile(long gamer, long[] outProfile);

    /**
     * cna_gamer_get_signed_in_gamer_at (gamer_services.h).
     */
    public static native int gamerGetSignedInGamerAt(int index, long[] outGamer);

    /**
     * cna_gamer_get_signed_in_gamer_at_player_index (gamer_services.h).
     */
    public static native int gamerGetSignedInGamerAtPlayerIndex(int playerIndex, boolean[] outHasGamer, long[] outGamer);

    /**
     * cna_gamer_get_signed_in_gamer_count (gamer_services.h).
     */
    public static native int gamerGetSignedInGamerCount(int[] outCount);

    /**
     * cna_gamer_get_tag (gamer_services.h).
     */
    public static native int gamerGetTag(long gamer, long[] outTag);

    /**
     * cna_gamer_get_text_size (gamer_services.h).
     */
    public static native int gamerGetTextSize(long gamer, long[] outBytes);

    /**
     * cna_gamer_profile_copy_motto (gamer_services.h).
     */
    public static native int gamerProfileCopyMotto(long profile, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_profile_copy_picture (gamer_services.h).
     */
    public static native int gamerProfileCopyPicture(long profile, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_profile_copy_region_name (gamer_services.h).
     */
    public static native int gamerProfileCopyRegionName(long profile, byte[] destination, long[] outBytes);

    /**
     * cna_gamer_profile_destroy (gamer_services.h).
     */
    public static native int gamerProfileDestroy(long profile);

    /**
     * cna_gamer_profile_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_GamerProfileInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_GamerProfileInfo in this order:
     * <ol start="0">
     *   <li>{@code gamer_score} (int32_t)</li>
     *   <li>{@code gamer_zone} (CNA_GamerZone)</li>
     *   <li>{@code titles_played} (int32_t)</li>
     *   <li>{@code total_achievements} (int32_t)</li>
     *   <li>{@code is_disposed} (CNA_Bool)</li>
     * </ol>
     *
     * <p>outInfoFloating carries CNA_GamerProfileInfo in this order:
     * <ol start="0">
     *   <li>{@code reputation} (float)</li>
     * </ol>
     */
    public static native int gamerProfileGetInfo(long profile, byte[] outInfoBytes, long[] outInfoIntegral, float[] outInfoFloating);

    /**
     * cna_gamer_profile_get_motto_size (gamer_services.h).
     */
    public static native int gamerProfileGetMottoSize(long profile, long[] outBytes);

    /**
     * cna_gamer_profile_get_picture_size (gamer_services.h).
     */
    public static native int gamerProfileGetPictureSize(long profile, boolean[] outHasPicture, long[] outBytes);

    /**
     * cna_gamer_profile_get_region_name_size (gamer_services.h).
     */
    public static native int gamerProfileGetRegionNameSize(long profile, long[] outBytes);

    /**
     * cna_gamer_services_dispatcher_get_is_initialized (gamer_services.h).
     */
    public static native int gamerServicesDispatcherGetIsInitialized(boolean[] outIsInitialized);

    /**
     * cna_gamer_services_dispatcher_get_window_handle (gamer_services.h).
     */
    public static native int gamerServicesDispatcherGetWindowHandle(long[] outWindowHandle);

    /**
     * cna_gamer_set_display_name (gamer_services.h).
     */
    public static native int gamerSetDisplayName(long gamer, byte[] displayName);

    /**
     * cna_gamer_set_tag (gamer_services.h).
     */
    public static native int gamerSetTag(long gamer, long tag);

    /**
     * cna_gamer_unsubscribe_ext (gamer_services.h).
     */
    public static native int gamerUnsubscribeExt(long registration);

    /**
     * cna_guide_begin_show_keyboard_input (gamer_services.h).
     */
    public static native int guideBeginShowKeyboardInput(int player, byte[] title, byte[] description, byte[] defaultText, boolean usePasswordMode);

    /**
     * cna_guide_copy_pending_keyboard_input_description_ext (gamer_services.h).
     */
    public static native int guideCopyPendingKeyboardInputDescriptionExt(byte[] destination, long[] outBytes);

    /**
     * cna_guide_copy_pending_keyboard_input_display_text_ext (gamer_services.h).
     */
    public static native int guideCopyPendingKeyboardInputDisplayTextExt(byte[] destination, long[] outBytes);

    /**
     * cna_guide_copy_pending_keyboard_input_title_ext (gamer_services.h).
     */
    public static native int guideCopyPendingKeyboardInputTitleExt(byte[] destination, long[] outBytes);

    /**
     * cna_guide_delay_notifications (gamer_services.h).
     */
    public static native int guideDelayNotifications(long delayTicks);

    /**
     * cna_guide_end_show_keyboard_input (gamer_services.h).
     */
    public static native int guideEndShowKeyboardInput(byte[] destination, long[] outBytes);

    /**
     * cna_guide_end_show_keyboard_input_size (gamer_services.h).
     */
    public static native int guideEndShowKeyboardInputSize(long[] outBytes);

    /**
     * cna_guide_end_show_message_box (gamer_services.h).
     */
    public static native int guideEndShowMessageBox(boolean[] outHasChoice, int[] outButtonIndex);

    /**
     * cna_guide_get_has_pending_keyboard_input_ext (gamer_services.h).
     */
    public static native int guideGetHasPendingKeyboardInputExt(boolean[] outHasPending);

    /**
     * cna_guide_get_has_pending_message_box_ext (gamer_services.h).
     */
    public static native int guideGetHasPendingMessageBoxExt(boolean[] outHasPending);

    /**
     * cna_guide_get_is_screen_saver_enabled (gamer_services.h).
     */
    public static native int guideGetIsScreenSaverEnabled(boolean[] outIsEnabled);

    /**
     * cna_guide_get_is_trial_mode (gamer_services.h).
     */
    public static native int guideGetIsTrialMode(boolean[] outIsTrialMode);

    /**
     * cna_guide_get_is_visible (gamer_services.h).
     */
    public static native int guideGetIsVisible(boolean[] outIsVisible);

    /**
     * cna_guide_get_notification_position (gamer_services.h).
     */
    public static native int guideGetNotificationPosition(int[] outPosition);

    /**
     * cna_guide_get_pending_keyboard_input_description_size_ext (gamer_services.h).
     */
    public static native int guideGetPendingKeyboardInputDescriptionSizeExt(long[] outBytes);

    /**
     * cna_guide_get_pending_keyboard_input_display_text_size_ext (gamer_services.h).
     */
    public static native int guideGetPendingKeyboardInputDisplayTextSizeExt(long[] outBytes);

    /**
     * cna_guide_get_pending_keyboard_input_title_size_ext (gamer_services.h).
     */
    public static native int guideGetPendingKeyboardInputTitleSizeExt(long[] outBytes);

    /**
     * cna_guide_get_pending_message_box_focus_button_ext (gamer_services.h).
     */
    public static native int guideGetPendingMessageBoxFocusButtonExt(int[] outFocusButton);

    /**
     * cna_guide_get_simulate_trial_mode (gamer_services.h).
     */
    public static native int guideGetSimulateTrialMode(boolean[] outSimulate);

    /**
     * cna_guide_render_pending_keyboard_input_ext (gamer_services.h).
     */
    public static native int guideRenderPendingKeyboardInputExt(long device, long spriteBatch, long font, long whitePixel);

    /**
     * cna_guide_render_pending_message_box_ext (gamer_services.h).
     */
    public static native int guideRenderPendingMessageBoxExt(long device, long spriteBatch, long font, long whitePixel);

    /**
     * cna_guide_reset_pending_keyboard_input_ext (gamer_services.h).
     */
    public static native int guideResetPendingKeyboardInputExt();

    /**
     * cna_guide_reset_pending_message_box_ext (gamer_services.h).
     */
    public static native int guideResetPendingMessageBoxExt();

    /**
     * cna_guide_set_is_screen_saver_enabled (gamer_services.h).
     */
    public static native int guideSetIsScreenSaverEnabled(boolean isEnabled);

    /**
     * cna_guide_set_notification_position (gamer_services.h).
     */
    public static native int guideSetNotificationPosition(int position);

    /**
     * cna_guide_set_simulate_trial_mode (gamer_services.h).
     */
    public static native int guideSetSimulateTrialMode(boolean simulate);

    /**
     * cna_guide_show_achievements_ext (gamer_services.h).
     */
    public static native int guideShowAchievementsExt(int player);

    /**
     * cna_guide_show_compose_message (gamer_services.h).
     */
    public static native int guideShowComposeMessage(int player, byte[] text, long[] recipients);

    /**
     * cna_guide_show_friend_request (gamer_services.h).
     */
    public static native int guideShowFriendRequest(int player, long gamer);

    /**
     * cna_guide_show_friends (gamer_services.h).
     */
    public static native int guideShowFriends(int player);

    /**
     * cna_guide_show_game_invite (gamer_services.h).
     */
    public static native int guideShowGameInvite(int player, long[] recipients);

    /**
     * cna_guide_show_game_invite_for_session (gamer_services.h).
     */
    public static native int guideShowGameInviteForSession(byte[] sessionId);

    /**
     * cna_guide_show_gamer_card (gamer_services.h).
     */
    public static native int guideShowGamerCard(int player, long gamer);

    /**
     * cna_guide_show_marketplace (gamer_services.h).
     */
    public static native int guideShowMarketplace(int player);

    /**
     * cna_guide_show_messages (gamer_services.h).
     */
    public static native int guideShowMessages(int player);

    /**
     * cna_guide_show_party (gamer_services.h).
     */
    public static native int guideShowParty(int player);

    /**
     * cna_guide_show_party_sessions (gamer_services.h).
     */
    public static native int guideShowPartySessions(int player);

    /**
     * cna_guide_show_player_review (gamer_services.h).
     */
    public static native int guideShowPlayerReview(int player, long gamer);

    /**
     * cna_guide_show_players (gamer_services.h).
     */
    public static native int guideShowPlayers(int player);

    /**
     * cna_guide_show_sign_in (gamer_services.h).
     */
    public static native int guideShowSignIn(int paneCount, boolean onlineOnly);

    /**
     * cna_guide_simulate_keyboard_input_cancel_ext (gamer_services.h).
     */
    public static native int guideSimulateKeyboardInputCancelExt();

    /**
     * cna_guide_simulate_message_box_click_ext (gamer_services.h).
     */
    public static native int guideSimulateMessageBoxClickExt(int buttonIndex);

    /**
     * cna_guide_was_keyboard_input_canceled_ext (gamer_services.h).
     */
    public static native int guideWasKeyboardInputCanceledExt(boolean[] outWasCanceled);

    /**
     * cna_leaderboard_entry_create_ext (gamer_services.h).
     */
    public static native int leaderboardEntryCreateExt(long gamer, long rating, int ranking, long[] outEntry);

    /**
     * cna_leaderboard_entry_destroy (gamer_services.h).
     */
    public static native int leaderboardEntryDestroy(long entry);

    /**
     * cna_leaderboard_entry_get_columns (gamer_services.h).
     */
    public static native int leaderboardEntryGetColumns(long entry, long[] outColumns);

    /**
     * cna_leaderboard_entry_get_gamer (gamer_services.h).
     */
    public static native int leaderboardEntryGetGamer(long entry, boolean[] outHasGamer, long[] outGamer);

    /**
     * cna_leaderboard_entry_get_info (gamer_services.h).
     *
     * <p>outInfoBytes carries CNA_LeaderboardEntryInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outInfoIntegral carries CNA_LeaderboardEntryInfo in this order:
     * <ol start="0">
     *   <li>{@code ranking} (int32_t)</li>
     *   <li>{@code has_gamer} (CNA_Bool)</li>
     *   <li>{@code rating} (int64_t)</li>
     * </ol>
     */
    public static native int leaderboardEntryGetInfo(long entry, byte[] outInfoBytes, long[] outInfoIntegral);

    /**
     * cna_leaderboard_entry_set_rating (gamer_services.h).
     */
    public static native int leaderboardEntrySetRating(long entry, long rating);

    /**
     * cna_leaderboard_reader_destroy (gamer_services.h).
     */
    public static native int leaderboardReaderDestroy(long reader);

    /**
     * cna_leaderboard_reader_get_entry_at (gamer_services.h).
     */
    public static native int leaderboardReaderGetEntryAt(long reader, int index, long[] outEntry);

    /**
     * cna_leaderboard_reader_get_identity (gamer_services.h).
     *
     * <p>outIdentityBytes carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code key[0]} (char)</li>
     *   <li>{@code key[1]} (char)</li>
     *   <li>{@code key[2]} (char)</li>
     *   <li>{@code key[3]} (char)</li>
     *   <li>{@code key[4]} (char)</li>
     *   <li>{@code key[5]} (char)</li>
     *   <li>{@code key[6]} (char)</li>
     *   <li>{@code key[7]} (char)</li>
     *   <li>{@code key[8]} (char)</li>
     *   <li>{@code key[9]} (char)</li>
     *   <li>{@code key[10]} (char)</li>
     *   <li>{@code key[11]} (char)</li>
     *   <li>{@code key[12]} (char)</li>
     *   <li>{@code key[13]} (char)</li>
     *   <li>{@code key[14]} (char)</li>
     *   <li>{@code key[15]} (char)</li>
     *   <li>{@code key[16]} (char)</li>
     *   <li>{@code key[17]} (char)</li>
     *   <li>{@code key[18]} (char)</li>
     *   <li>{@code key[19]} (char)</li>
     *   <li>{@code key[20]} (char)</li>
     *   <li>{@code key[21]} (char)</li>
     *   <li>{@code key[22]} (char)</li>
     *   <li>{@code key[23]} (char)</li>
     *   <li>{@code key[24]} (char)</li>
     *   <li>{@code key[25]} (char)</li>
     *   <li>{@code key[26]} (char)</li>
     *   <li>{@code key[27]} (char)</li>
     *   <li>{@code key[28]} (char)</li>
     *   <li>{@code key[29]} (char)</li>
     *   <li>{@code key[30]} (char)</li>
     *   <li>{@code key[31]} (char)</li>
     *   <li>{@code key[32]} (char)</li>
     *   <li>{@code key[33]} (char)</li>
     *   <li>{@code key[34]} (char)</li>
     *   <li>{@code key[35]} (char)</li>
     *   <li>{@code key[36]} (char)</li>
     *   <li>{@code key[37]} (char)</li>
     *   <li>{@code key[38]} (char)</li>
     *   <li>{@code key[39]} (char)</li>
     *   <li>{@code key[40]} (char)</li>
     *   <li>{@code key[41]} (char)</li>
     *   <li>{@code key[42]} (char)</li>
     *   <li>{@code key[43]} (char)</li>
     *   <li>{@code key[44]} (char)</li>
     *   <li>{@code key[45]} (char)</li>
     *   <li>{@code key[46]} (char)</li>
     *   <li>{@code key[47]} (char)</li>
     *   <li>{@code key[48]} (char)</li>
     *   <li>{@code key[49]} (char)</li>
     *   <li>{@code key[50]} (char)</li>
     *   <li>{@code key[51]} (char)</li>
     *   <li>{@code key[52]} (char)</li>
     *   <li>{@code key[53]} (char)</li>
     *   <li>{@code key[54]} (char)</li>
     *   <li>{@code key[55]} (char)</li>
     *   <li>{@code key[56]} (char)</li>
     *   <li>{@code key[57]} (char)</li>
     *   <li>{@code key[58]} (char)</li>
     *   <li>{@code key[59]} (char)</li>
     *   <li>{@code key[60]} (char)</li>
     *   <li>{@code key[61]} (char)</li>
     *   <li>{@code key[62]} (char)</li>
     *   <li>{@code key[63]} (char)</li>
     * </ol>
     *
     * <p>outIdentityIntegral carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code game_mode} (int32_t)</li>
     * </ol>
     */
    public static native int leaderboardReaderGetIdentity(long reader, byte[] outIdentityBytes, long[] outIdentityIntegral);

    /**
     * cna_leaderboard_reader_get_info (gamer_services.h).
     *
     * <p>outInfoIntegral carries CNA_LeaderboardReaderInfo in this order:
     * <ol start="0">
     *   <li>{@code page_start} (int32_t)</li>
     *   <li>{@code total_leaderboard_size} (int32_t)</li>
     *   <li>{@code entry_count} (int32_t)</li>
     *   <li>{@code is_disposed} (CNA_Bool)</li>
     *   <li>{@code can_page_down} (CNA_Bool)</li>
     *   <li>{@code can_page_up} (CNA_Bool)</li>
     *   <li>{@code reserved} (uint8_t)</li>
     * </ol>
     */
    public static native int leaderboardReaderGetInfo(long reader, long[] outInfoIntegral);

    /**
     * cna_leaderboard_reader_page_down (gamer_services.h).
     */
    public static native int leaderboardReaderPageDown(long reader);

    /**
     * cna_leaderboard_reader_page_up (gamer_services.h).
     */
    public static native int leaderboardReaderPageUp(long reader);

    /**
     * cna_leaderboard_reader_read (gamer_services.h).
     *
     * <p>identityBytes carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code key[0]} (char)</li>
     *   <li>{@code key[1]} (char)</li>
     *   <li>{@code key[2]} (char)</li>
     *   <li>{@code key[3]} (char)</li>
     *   <li>{@code key[4]} (char)</li>
     *   <li>{@code key[5]} (char)</li>
     *   <li>{@code key[6]} (char)</li>
     *   <li>{@code key[7]} (char)</li>
     *   <li>{@code key[8]} (char)</li>
     *   <li>{@code key[9]} (char)</li>
     *   <li>{@code key[10]} (char)</li>
     *   <li>{@code key[11]} (char)</li>
     *   <li>{@code key[12]} (char)</li>
     *   <li>{@code key[13]} (char)</li>
     *   <li>{@code key[14]} (char)</li>
     *   <li>{@code key[15]} (char)</li>
     *   <li>{@code key[16]} (char)</li>
     *   <li>{@code key[17]} (char)</li>
     *   <li>{@code key[18]} (char)</li>
     *   <li>{@code key[19]} (char)</li>
     *   <li>{@code key[20]} (char)</li>
     *   <li>{@code key[21]} (char)</li>
     *   <li>{@code key[22]} (char)</li>
     *   <li>{@code key[23]} (char)</li>
     *   <li>{@code key[24]} (char)</li>
     *   <li>{@code key[25]} (char)</li>
     *   <li>{@code key[26]} (char)</li>
     *   <li>{@code key[27]} (char)</li>
     *   <li>{@code key[28]} (char)</li>
     *   <li>{@code key[29]} (char)</li>
     *   <li>{@code key[30]} (char)</li>
     *   <li>{@code key[31]} (char)</li>
     *   <li>{@code key[32]} (char)</li>
     *   <li>{@code key[33]} (char)</li>
     *   <li>{@code key[34]} (char)</li>
     *   <li>{@code key[35]} (char)</li>
     *   <li>{@code key[36]} (char)</li>
     *   <li>{@code key[37]} (char)</li>
     *   <li>{@code key[38]} (char)</li>
     *   <li>{@code key[39]} (char)</li>
     *   <li>{@code key[40]} (char)</li>
     *   <li>{@code key[41]} (char)</li>
     *   <li>{@code key[42]} (char)</li>
     *   <li>{@code key[43]} (char)</li>
     *   <li>{@code key[44]} (char)</li>
     *   <li>{@code key[45]} (char)</li>
     *   <li>{@code key[46]} (char)</li>
     *   <li>{@code key[47]} (char)</li>
     *   <li>{@code key[48]} (char)</li>
     *   <li>{@code key[49]} (char)</li>
     *   <li>{@code key[50]} (char)</li>
     *   <li>{@code key[51]} (char)</li>
     *   <li>{@code key[52]} (char)</li>
     *   <li>{@code key[53]} (char)</li>
     *   <li>{@code key[54]} (char)</li>
     *   <li>{@code key[55]} (char)</li>
     *   <li>{@code key[56]} (char)</li>
     *   <li>{@code key[57]} (char)</li>
     *   <li>{@code key[58]} (char)</li>
     *   <li>{@code key[59]} (char)</li>
     *   <li>{@code key[60]} (char)</li>
     *   <li>{@code key[61]} (char)</li>
     *   <li>{@code key[62]} (char)</li>
     *   <li>{@code key[63]} (char)</li>
     * </ol>
     *
     * <p>identityIntegral carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code game_mode} (int32_t)</li>
     * </ol>
     */
    public static native int leaderboardReaderRead(byte[] identityBytes, long[] identityIntegral, int pageStart, int pageSize, long[] outReader);

    /**
     * cna_leaderboard_reader_read_from_gamers (gamer_services.h).
     *
     * <p>identityBytes carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code key[0]} (char)</li>
     *   <li>{@code key[1]} (char)</li>
     *   <li>{@code key[2]} (char)</li>
     *   <li>{@code key[3]} (char)</li>
     *   <li>{@code key[4]} (char)</li>
     *   <li>{@code key[5]} (char)</li>
     *   <li>{@code key[6]} (char)</li>
     *   <li>{@code key[7]} (char)</li>
     *   <li>{@code key[8]} (char)</li>
     *   <li>{@code key[9]} (char)</li>
     *   <li>{@code key[10]} (char)</li>
     *   <li>{@code key[11]} (char)</li>
     *   <li>{@code key[12]} (char)</li>
     *   <li>{@code key[13]} (char)</li>
     *   <li>{@code key[14]} (char)</li>
     *   <li>{@code key[15]} (char)</li>
     *   <li>{@code key[16]} (char)</li>
     *   <li>{@code key[17]} (char)</li>
     *   <li>{@code key[18]} (char)</li>
     *   <li>{@code key[19]} (char)</li>
     *   <li>{@code key[20]} (char)</li>
     *   <li>{@code key[21]} (char)</li>
     *   <li>{@code key[22]} (char)</li>
     *   <li>{@code key[23]} (char)</li>
     *   <li>{@code key[24]} (char)</li>
     *   <li>{@code key[25]} (char)</li>
     *   <li>{@code key[26]} (char)</li>
     *   <li>{@code key[27]} (char)</li>
     *   <li>{@code key[28]} (char)</li>
     *   <li>{@code key[29]} (char)</li>
     *   <li>{@code key[30]} (char)</li>
     *   <li>{@code key[31]} (char)</li>
     *   <li>{@code key[32]} (char)</li>
     *   <li>{@code key[33]} (char)</li>
     *   <li>{@code key[34]} (char)</li>
     *   <li>{@code key[35]} (char)</li>
     *   <li>{@code key[36]} (char)</li>
     *   <li>{@code key[37]} (char)</li>
     *   <li>{@code key[38]} (char)</li>
     *   <li>{@code key[39]} (char)</li>
     *   <li>{@code key[40]} (char)</li>
     *   <li>{@code key[41]} (char)</li>
     *   <li>{@code key[42]} (char)</li>
     *   <li>{@code key[43]} (char)</li>
     *   <li>{@code key[44]} (char)</li>
     *   <li>{@code key[45]} (char)</li>
     *   <li>{@code key[46]} (char)</li>
     *   <li>{@code key[47]} (char)</li>
     *   <li>{@code key[48]} (char)</li>
     *   <li>{@code key[49]} (char)</li>
     *   <li>{@code key[50]} (char)</li>
     *   <li>{@code key[51]} (char)</li>
     *   <li>{@code key[52]} (char)</li>
     *   <li>{@code key[53]} (char)</li>
     *   <li>{@code key[54]} (char)</li>
     *   <li>{@code key[55]} (char)</li>
     *   <li>{@code key[56]} (char)</li>
     *   <li>{@code key[57]} (char)</li>
     *   <li>{@code key[58]} (char)</li>
     *   <li>{@code key[59]} (char)</li>
     *   <li>{@code key[60]} (char)</li>
     *   <li>{@code key[61]} (char)</li>
     *   <li>{@code key[62]} (char)</li>
     *   <li>{@code key[63]} (char)</li>
     * </ol>
     *
     * <p>identityIntegral carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code game_mode} (int32_t)</li>
     * </ol>
     */
    public static native int leaderboardReaderReadFromGamers(byte[] identityBytes, long[] identityIntegral, long[] gamers, long pivotGamer, int pageSize, long[] outReader);

    /**
     * cna_leaderboard_reader_read_from_pivot (gamer_services.h).
     *
     * <p>identityBytes carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code key[0]} (char)</li>
     *   <li>{@code key[1]} (char)</li>
     *   <li>{@code key[2]} (char)</li>
     *   <li>{@code key[3]} (char)</li>
     *   <li>{@code key[4]} (char)</li>
     *   <li>{@code key[5]} (char)</li>
     *   <li>{@code key[6]} (char)</li>
     *   <li>{@code key[7]} (char)</li>
     *   <li>{@code key[8]} (char)</li>
     *   <li>{@code key[9]} (char)</li>
     *   <li>{@code key[10]} (char)</li>
     *   <li>{@code key[11]} (char)</li>
     *   <li>{@code key[12]} (char)</li>
     *   <li>{@code key[13]} (char)</li>
     *   <li>{@code key[14]} (char)</li>
     *   <li>{@code key[15]} (char)</li>
     *   <li>{@code key[16]} (char)</li>
     *   <li>{@code key[17]} (char)</li>
     *   <li>{@code key[18]} (char)</li>
     *   <li>{@code key[19]} (char)</li>
     *   <li>{@code key[20]} (char)</li>
     *   <li>{@code key[21]} (char)</li>
     *   <li>{@code key[22]} (char)</li>
     *   <li>{@code key[23]} (char)</li>
     *   <li>{@code key[24]} (char)</li>
     *   <li>{@code key[25]} (char)</li>
     *   <li>{@code key[26]} (char)</li>
     *   <li>{@code key[27]} (char)</li>
     *   <li>{@code key[28]} (char)</li>
     *   <li>{@code key[29]} (char)</li>
     *   <li>{@code key[30]} (char)</li>
     *   <li>{@code key[31]} (char)</li>
     *   <li>{@code key[32]} (char)</li>
     *   <li>{@code key[33]} (char)</li>
     *   <li>{@code key[34]} (char)</li>
     *   <li>{@code key[35]} (char)</li>
     *   <li>{@code key[36]} (char)</li>
     *   <li>{@code key[37]} (char)</li>
     *   <li>{@code key[38]} (char)</li>
     *   <li>{@code key[39]} (char)</li>
     *   <li>{@code key[40]} (char)</li>
     *   <li>{@code key[41]} (char)</li>
     *   <li>{@code key[42]} (char)</li>
     *   <li>{@code key[43]} (char)</li>
     *   <li>{@code key[44]} (char)</li>
     *   <li>{@code key[45]} (char)</li>
     *   <li>{@code key[46]} (char)</li>
     *   <li>{@code key[47]} (char)</li>
     *   <li>{@code key[48]} (char)</li>
     *   <li>{@code key[49]} (char)</li>
     *   <li>{@code key[50]} (char)</li>
     *   <li>{@code key[51]} (char)</li>
     *   <li>{@code key[52]} (char)</li>
     *   <li>{@code key[53]} (char)</li>
     *   <li>{@code key[54]} (char)</li>
     *   <li>{@code key[55]} (char)</li>
     *   <li>{@code key[56]} (char)</li>
     *   <li>{@code key[57]} (char)</li>
     *   <li>{@code key[58]} (char)</li>
     *   <li>{@code key[59]} (char)</li>
     *   <li>{@code key[60]} (char)</li>
     *   <li>{@code key[61]} (char)</li>
     *   <li>{@code key[62]} (char)</li>
     *   <li>{@code key[63]} (char)</li>
     * </ol>
     *
     * <p>identityIntegral carries CNA_LeaderboardIdentity in this order:
     * <ol start="0">
     *   <li>{@code game_mode} (int32_t)</li>
     * </ol>
     */
    public static native int leaderboardReaderReadFromPivot(byte[] identityBytes, long[] identityIntegral, long pivotGamer, int pageSize, long[] outReader);

    /**
     * cna_property_dictionary_clear (gamer_services.h).
     */
    public static native int propertyDictionaryClear(long dictionary);

    /**
     * cna_property_dictionary_contains_key (gamer_services.h).
     */
    public static native int propertyDictionaryContainsKey(long dictionary, byte[] key, boolean[] outContains);

    /**
     * cna_property_dictionary_copy_key_at (gamer_services.h).
     */
    public static native int propertyDictionaryCopyKeyAt(long dictionary, int index, byte[] destination, long[] outBytes);

    /**
     * cna_property_dictionary_copy_string (gamer_services.h).
     */
    public static native int propertyDictionaryCopyString(long dictionary, byte[] key, byte[] destination, long[] outBytes);

    /**
     * cna_property_dictionary_get_count (gamer_services.h).
     */
    public static native int propertyDictionaryGetCount(long dictionary, int[] outCount);

    /**
     * cna_property_dictionary_get_date_time_ticks (gamer_services.h).
     */
    public static native int propertyDictionaryGetDateTimeTicks(long dictionary, byte[] key, long[] outTicks);

    /**
     * cna_property_dictionary_get_double (gamer_services.h).
     */
    public static native int propertyDictionaryGetDouble(long dictionary, byte[] key, double[] outValue);

    /**
     * cna_property_dictionary_get_int32 (gamer_services.h).
     */
    public static native int propertyDictionaryGetInt32(long dictionary, byte[] key, int[] outValue);

    /**
     * cna_property_dictionary_get_int64 (gamer_services.h).
     */
    public static native int propertyDictionaryGetInt64(long dictionary, byte[] key, long[] outValue);

    /**
     * cna_property_dictionary_get_key_size_at (gamer_services.h).
     */
    public static native int propertyDictionaryGetKeySizeAt(long dictionary, int index, long[] outBytes);

    /**
     * cna_property_dictionary_get_outcome (gamer_services.h).
     */
    public static native int propertyDictionaryGetOutcome(long dictionary, byte[] key, int[] outOutcome);

    /**
     * cna_property_dictionary_get_single (gamer_services.h).
     */
    public static native int propertyDictionaryGetSingle(long dictionary, byte[] key, float[] outValue);

    /**
     * cna_property_dictionary_get_stream_size_ext (gamer_services.h).
     */
    public static native int propertyDictionaryGetStreamSizeExt(long dictionary, byte[] key, boolean[] outHasStream, long[] outBytes);

    /**
     * cna_property_dictionary_get_string_size (gamer_services.h).
     */
    public static native int propertyDictionaryGetStringSize(long dictionary, byte[] key, long[] outBytes);

    /**
     * cna_property_dictionary_get_time_span_ticks (gamer_services.h).
     */
    public static native int propertyDictionaryGetTimeSpanTicks(long dictionary, byte[] key, long[] outTicks);

    /**
     * cna_property_dictionary_remove (gamer_services.h).
     */
    public static native int propertyDictionaryRemove(long dictionary, byte[] key, boolean[] outRemoved);

    /**
     * cna_property_dictionary_set_date_time_ticks (gamer_services.h).
     */
    public static native int propertyDictionarySetDateTimeTicks(long dictionary, byte[] key, long ticks);

    /**
     * cna_property_dictionary_set_double (gamer_services.h).
     */
    public static native int propertyDictionarySetDouble(long dictionary, byte[] key, double value);

    /**
     * cna_property_dictionary_set_int32 (gamer_services.h).
     */
    public static native int propertyDictionarySetInt32(long dictionary, byte[] key, int value);

    /**
     * cna_property_dictionary_set_int64 (gamer_services.h).
     */
    public static native int propertyDictionarySetInt64(long dictionary, byte[] key, long value);

    /**
     * cna_property_dictionary_set_outcome (gamer_services.h).
     */
    public static native int propertyDictionarySetOutcome(long dictionary, byte[] key, int outcome);

    /**
     * cna_property_dictionary_set_single (gamer_services.h).
     */
    public static native int propertyDictionarySetSingle(long dictionary, byte[] key, float value);

    /**
     * cna_property_dictionary_set_string (gamer_services.h).
     */
    public static native int propertyDictionarySetString(long dictionary, byte[] key, byte[] value);

    /**
     * cna_property_dictionary_set_time_span_ticks (gamer_services.h).
     */
    public static native int propertyDictionarySetTimeSpanTicks(long dictionary, byte[] key, long ticks);

    /**
     * cna_property_dictionary_try_get_value_kind_ext (gamer_services.h).
     */
    public static native int propertyDictionaryTryGetValueKindExt(long dictionary, byte[] key, boolean[] outFound, int[] outKind);

    /**
     * cna_signed_in_gamer_award_achievement (gamer_services.h).
     */
    public static native int signedInGamerAwardAchievement(long gamer, byte[] achievementKey);

    /**
     * cna_signed_in_gamer_get_achievements (gamer_services.h).
     */
    public static native int signedInGamerGetAchievements(long gamer, long[] outAchievements);

    /**
     * cna_signed_in_gamer_get_friends (gamer_services.h).
     */
    public static native int signedInGamerGetFriends(long gamer, long[] outFriends);

    /**
     * cna_signed_in_gamer_get_game_defaults (gamer_services.h).
     *
     * <p>outDefaultsBytes carries CNA_GameDefaults in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outDefaultsIntegral carries CNA_GameDefaults in this order:
     * <ol start="0">
     *   <li>{@code game_difficulty} (CNA_GameDifficulty)</li>
     *   <li>{@code controller_sensitivity} (CNA_ControllerSensitivity)</li>
     *   <li>{@code racing_camera_angle} (CNA_RacingCameraAngle)</li>
     *   <li>{@code has_primary_color} (CNA_Bool)</li>
     *   <li>{@code has_secondary_color} (CNA_Bool)</li>
     *   <li>{@code auto_aim} (CNA_Bool)</li>
     *   <li>{@code auto_center} (CNA_Bool)</li>
     *   <li>{@code move_with_right_thumb_stick} (CNA_Bool)</li>
     *   <li>{@code invert_y_axis} (CNA_Bool)</li>
     *   <li>{@code manual_transmission} (CNA_Bool)</li>
     *   <li>{@code accelerate_with_buttons} (CNA_Bool)</li>
     *   <li>{@code brake_with_buttons} (CNA_Bool)</li>
     *   <li>{@code primary_color.r} (uint8_t)</li>
     *   <li>{@code primary_color.g} (uint8_t)</li>
     *   <li>{@code primary_color.b} (uint8_t)</li>
     *   <li>{@code primary_color.a} (uint8_t)</li>
     *   <li>{@code secondary_color.r} (uint8_t)</li>
     *   <li>{@code secondary_color.g} (uint8_t)</li>
     *   <li>{@code secondary_color.b} (uint8_t)</li>
     *   <li>{@code secondary_color.a} (uint8_t)</li>
     * </ol>
     */
    public static native int signedInGamerGetGameDefaults(long gamer, byte[] outDefaultsBytes, long[] outDefaultsIntegral);

    /**
     * cna_signed_in_gamer_get_is_guest (gamer_services.h).
     */
    public static native int signedInGamerGetIsGuest(long gamer, boolean[] outIsGuest);

    /**
     * cna_signed_in_gamer_get_is_signed_in_to_live (gamer_services.h).
     */
    public static native int signedInGamerGetIsSignedInToLive(long gamer, boolean[] outIsSignedInToLive);

    /**
     * cna_signed_in_gamer_get_party_size (gamer_services.h).
     */
    public static native int signedInGamerGetPartySize(long gamer, int[] outPartySize);

    /**
     * cna_signed_in_gamer_get_player_index (gamer_services.h).
     */
    public static native int signedInGamerGetPlayerIndex(long gamer, int[] outPlayerIndex);

    /**
     * cna_signed_in_gamer_get_presence (gamer_services.h).
     *
     * <p>outPresenceIntegral carries CNA_GamerPresence in this order:
     * <ol start="0">
     *   <li>{@code presence_mode} (CNA_GamerPresenceMode)</li>
     *   <li>{@code presence_value} (int32_t)</li>
     * </ol>
     */
    public static native int signedInGamerGetPresence(long gamer, long[] outPresenceIntegral);

    /**
     * cna_signed_in_gamer_get_privileges (gamer_services.h).
     *
     * <p>outPrivilegesBytes carries CNA_GamerPrivileges in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     *   <li>{@code reserved[3]} (uint8_t)</li>
     * </ol>
     *
     * <p>outPrivilegesIntegral carries CNA_GamerPrivileges in this order:
     * <ol start="0">
     *   <li>{@code allow_communication} (CNA_GamerPrivilegeSetting)</li>
     *   <li>{@code allow_profile_viewing} (CNA_GamerPrivilegeSetting)</li>
     *   <li>{@code allow_user_created_content} (CNA_GamerPrivilegeSetting)</li>
     *   <li>{@code allow_online_sessions} (CNA_Bool)</li>
     *   <li>{@code allow_premium_content} (CNA_Bool)</li>
     *   <li>{@code allow_purchase_content} (CNA_Bool)</li>
     *   <li>{@code allow_trade_content} (CNA_Bool)</li>
     * </ol>
     */
    public static native int signedInGamerGetPrivileges(long gamer, byte[] outPrivilegesBytes, long[] outPrivilegesIntegral);

    /**
     * cna_signed_in_gamer_is_friend (gamer_services.h).
     */
    public static native int signedInGamerIsFriend(long gamer, long other, boolean[] outIsFriend);

    /**
     * cna_signed_in_gamer_is_headset (gamer_services.h).
     */
    public static native int signedInGamerIsHeadset(long gamer, long microphoneIndex, boolean[] outIsHeadset);

    /**
     * cna_signed_in_gamer_set_party_size (gamer_services.h).
     */
    public static native int signedInGamerSetPartySize(long gamer, int partySize);

    /**
     * cna_signed_in_gamer_set_presence (gamer_services.h).
     *
     * <p>presenceIntegral carries CNA_GamerPresence in this order:
     * <ol start="0">
     *   <li>{@code presence_mode} (CNA_GamerPresenceMode)</li>
     *   <li>{@code presence_value} (int32_t)</li>
     * </ol>
     */
    public static native int signedInGamerSetPresence(long gamer, long[] presenceIntegral);

    /**
     * cna_signed_in_gamer_set_presence_mode_string_ext (gamer_services.h).
     */
    public static native int signedInGamerSetPresenceModeStringExt(long gamer, byte[] mode);
}
