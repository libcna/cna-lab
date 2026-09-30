/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeGamerServicesRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionContains(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong achievement, jbooleanArray out_contains)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_contains_value = 0;
    CNA_Result call_result = cna.achievement_collection_contains((CNA_AchievementCollectionHandle)collection, (CNA_AchievementHandle)achievement, &out_contains_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_contains_element = (jboolean)out_contains_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_contains, 0, 1, &out_contains_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.achievement_collection_destroy((CNA_AchievementCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jint index, jlongArray out_achievement)
{
    (void)environment;
    (void)declaring_class;
    CNA_AchievementHandle out_achievement_value = 0;
    CNA_Result call_result = cna.achievement_collection_get_at((CNA_AchievementCollectionHandle)collection, (int32_t)index, &out_achievement_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_achievement_element = (jlong)out_achievement_value;
        (*environment)->SetLongArrayRegion(
            environment, out_achievement, 0, 1, &out_achievement_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionGetByKey(
    JNIEnv* environment, jclass declaring_class, jlong collection, jbyteArray key, jlongArray out_achievement)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_AchievementHandle out_achievement_value = 0;
    CNA_Result call_result = cna.achievement_collection_get_by_key((CNA_AchievementCollectionHandle)collection, key_view, &out_achievement_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_achievement_element = (jlong)out_achievement_value;
        (*environment)->SetLongArrayRegion(
            environment, out_achievement, 0, 1, &out_achievement_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.achievement_collection_get_count((CNA_AchievementCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionGetIsDisposed(
    JNIEnv* environment, jclass declaring_class, jlong collection, jbooleanArray out_is_disposed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_disposed_value = 0;
    CNA_Result call_result = cna.achievement_collection_get_is_disposed((CNA_AchievementCollectionHandle)collection, &out_is_disposed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_disposed_element = (jboolean)out_is_disposed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_disposed, 0, 1, &out_is_disposed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCollectionIndexOf(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong achievement, jintArray out_index)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_index_value = 0;
    CNA_Result call_result = cna.achievement_collection_index_of((CNA_AchievementCollectionHandle)collection, (CNA_AchievementHandle)achievement, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_index_element = (jint)out_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCopyDescription(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.achievement_copy_description((CNA_AchievementHandle)achievement, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCopyHowToEarn(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.achievement_copy_how_to_earn((CNA_AchievementHandle)achievement, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCopyKey(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.achievement_copy_key((CNA_AchievementHandle)achievement, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCopyName(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.achievement_copy_name((CNA_AchievementHandle)achievement, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementCopyPicture(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_copy_picture((CNA_AchievementHandle)achievement, destination_values, (uint64_t)destination_size, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetDescriptionSize(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_get_description_size((CNA_AchievementHandle)achievement, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetHowToEarnSize(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_get_how_to_earn_size((CNA_AchievementHandle)achievement, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AchievementInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.achievement_get_info((CNA_AchievementHandle)achievement, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.gamer_score,
            (jlong)out_info_value.display_before_earned,
            (jlong)out_info_value.earned_online,
            (jlong)out_info_value.is_earned,
            (jlong)out_info_value.reserved,
            (jlong)out_info_value.earned_date_time_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetKeySize(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_get_key_size((CNA_AchievementHandle)achievement, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetNameSize(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_get_name_size((CNA_AchievementHandle)achievement, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_achievementGetPictureSize(
    JNIEnv* environment, jclass declaring_class, jlong achievement, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.achievement_get_picture_size((CNA_AchievementHandle)achievement, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationCreate(
    JNIEnv* environment, jclass declaring_class, jint preset, jlongArray out_animation)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvatarAnimationHandle out_animation_value = 0;
    CNA_Result call_result = cna.avatar_animation_create((CNA_AvatarAnimationPreset)preset, &out_animation_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_animation_element = (jlong)out_animation_value;
        (*environment)->SetLongArrayRegion(
            environment, out_animation, 0, 1, &out_animation_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationDestroy(
    JNIEnv* environment, jclass declaring_class, jlong animation)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_animation_destroy((CNA_AvatarAnimationHandle)animation);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationGetBoneTransformAt(
    JNIEnv* environment, jclass declaring_class, jlong animation, jint index, jfloatArray out_transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_transform_value;
    memset(&out_transform_value, 0, sizeof out_transform_value);
    CNA_Result call_result = cna.avatar_animation_get_bone_transform_at((CNA_AvatarAnimationHandle)animation, (int32_t)index, &out_transform_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_transformFloating_values[] = {
            (jfloat)out_transform_value.m11,
            (jfloat)out_transform_value.m12,
            (jfloat)out_transform_value.m13,
            (jfloat)out_transform_value.m14,
            (jfloat)out_transform_value.m21,
            (jfloat)out_transform_value.m22,
            (jfloat)out_transform_value.m23,
            (jfloat)out_transform_value.m24,
            (jfloat)out_transform_value.m31,
            (jfloat)out_transform_value.m32,
            (jfloat)out_transform_value.m33,
            (jfloat)out_transform_value.m34,
            (jfloat)out_transform_value.m41,
            (jfloat)out_transform_value.m42,
            (jfloat)out_transform_value.m43,
            (jfloat)out_transform_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_transformFloating, 0,
            (jsize)(sizeof out_transformFloating_values / sizeof out_transformFloating_values[0]),
            out_transformFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationGetExpression(
    JNIEnv* environment, jclass declaring_class, jlong animation, jlongArray out_expressionIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_expressionIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AvatarExpression out_expression_value;
    memset(&out_expression_value, 0, sizeof out_expression_value);
    out_expression_value.struct_size = (uint32_t)(sizeof out_expression_value);
    out_expression_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.avatar_animation_get_expression((CNA_AvatarAnimationHandle)animation, &out_expression_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_expressionIntegral_values[] = {
            (jlong)out_expression_value.mouth,
            (jlong)out_expression_value.left_eye,
            (jlong)out_expression_value.right_eye,
            (jlong)out_expression_value.left_eyebrow,
            (jlong)out_expression_value.right_eyebrow
        };
        (*environment)->SetLongArrayRegion(environment, out_expressionIntegral, 0,
            (jsize)(sizeof out_expressionIntegral_values / sizeof out_expressionIntegral_values[0]),
            out_expressionIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong animation, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AvatarAnimationInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.avatar_animation_get_info((CNA_AvatarAnimationHandle)animation, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.bone_transform_count,
            (jlong)out_info_value.is_disposed,
            (jlong)out_info_value.current_position_ticks,
            (jlong)out_info_value.length_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationSetCurrentPosition(
    JNIEnv* environment, jclass declaring_class, jlong animation, jlong position_ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_animation_set_current_position((CNA_AvatarAnimationHandle)animation, (int64_t)position_ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarAnimationUpdate(
    JNIEnv* environment, jclass declaring_class, jlong animation, jlong elapsed_ticks, jboolean loop)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_animation_update((CNA_AvatarAnimationHandle)animation, (int64_t)elapsed_ticks, (CNA_Bool)loop);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionCopyDescription(
    JNIEnv* environment, jclass declaring_class, jlong description, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.avatar_description_copy_description((CNA_AvatarDescriptionHandle)description, destination_values, (uint64_t)destination_size, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionCreate(
    JNIEnv* environment, jclass declaring_class, jbyteArray description, jlongArray out_description)
{
    (void)environment;
    (void)declaring_class;
    jsize description_size = (*environment)->GetArrayLength(environment, description);
    jbyte* description_elements = (*environment)->GetByteArrayElements(environment, description, NULL);
    if (description_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* description_values = (uint8_t*)malloc(
        ((size_t)description_size + 1U) * sizeof(*description_values));
    if (description_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, description, description_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < description_size; ++index) {
        description_values[index] = (uint8_t)description_elements[index];
    }
    CNA_AvatarDescriptionHandle out_description_value = 0;
    CNA_Result call_result = cna.avatar_description_create(description_values, (uint64_t)description_size, &out_description_value);
    free(description_values);
    (*environment)->ReleaseByteArrayElements(
        environment, description, description_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_description_element = (jlong)out_description_value;
        (*environment)->SetLongArrayRegion(
            environment, out_description, 0, 1, &out_description_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionCreateRandom(
    JNIEnv* environment, jclass declaring_class, jlongArray out_description)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvatarDescriptionHandle out_description_value = 0;
    CNA_Result call_result = cna.avatar_description_create_random(&out_description_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_description_element = (jlong)out_description_value;
        (*environment)->SetLongArrayRegion(
            environment, out_description, 0, 1, &out_description_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionCreateRandomForBodyType(
    JNIEnv* environment, jclass declaring_class, jint body_type, jlongArray out_description)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvatarDescriptionHandle out_description_value = 0;
    CNA_Result call_result = cna.avatar_description_create_random_for_body_type((CNA_AvatarBodyType)body_type, &out_description_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_description_element = (jlong)out_description_value;
        (*environment)->SetLongArrayRegion(
            environment, out_description, 0, 1, &out_description_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong description)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_description_destroy((CNA_AvatarDescriptionHandle)description);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionGetFromGamer(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_description)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvatarDescriptionHandle out_description_value = 0;
    CNA_Result call_result = cna.avatar_description_get_from_gamer((CNA_GamerHandle)gamer, NULL, NULL, &out_description_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_description_element = (jlong)out_description_value;
        (*environment)->SetLongArrayRegion(
            environment, out_description, 0, 1, &out_description_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarDescriptionGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong description, jbyteArray out_infoBytes, jlongArray out_infoIntegral, jfloatArray out_infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL || out_infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AvatarDescriptionInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.avatar_description_get_info((CNA_AvatarDescriptionHandle)description, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2],
            (jbyte)out_info_value.reserved[3],
            (jbyte)out_info_value.reserved[4],
            (jbyte)out_info_value.reserved[5],
            (jbyte)out_info_value.reserved[6]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.body_type,
            (jlong)out_info_value.description_byte_count,
            (jlong)out_info_value.is_valid
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jfloat out_infoFloating_values[] = {
            (jfloat)out_info_value.height
        };
        (*environment)->SetFloatArrayRegion(environment, out_infoFloating, 0,
            (jsize)(sizeof out_infoFloating_values / sizeof out_infoFloating_values[0]),
            out_infoFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererCreate(
    JNIEnv* environment, jclass declaring_class, jlong description, jboolean use_loading_effect, jlongArray out_renderer)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvatarRendererHandle out_renderer_value = 0;
    CNA_Result call_result = cna.avatar_renderer_create((CNA_AvatarDescriptionHandle)description, (CNA_Bool)use_loading_effect, &out_renderer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_renderer_element = (jlong)out_renderer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_renderer, 0, 1, &out_renderer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererDestroy(
    JNIEnv* environment, jclass declaring_class, jlong renderer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_renderer_destroy((CNA_AvatarRendererHandle)renderer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererDrawAnimation(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jlong animation)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.avatar_renderer_draw_animation((CNA_AvatarRendererHandle)renderer, (CNA_AvatarAnimationHandle)animation);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererDrawBones(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jfloatArray bonesFloating, jlongArray expressionIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (expressionIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize bones_count = (*environment)->GetArrayLength(environment, bonesFloating) / 16;
    if ((*environment)->GetArrayLength(environment, bonesFloating) != bones_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* bones_values = (CNA_Matrix*)calloc(
        (size_t)bones_count + 1U, sizeof(*bones_values));
    if (bones_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize bonesFloating_length = (*environment)->GetArrayLength(environment, bonesFloating);
        jfloat* bonesFloating_values = (jfloat*)malloc(
            ((size_t)bonesFloating_length + 1U) * sizeof(*bonesFloating_values));
        if (bonesFloating_values == NULL) {
            free(bones_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, bonesFloating, 0, bonesFloating_length, bonesFloating_values);
        for (jsize element = 0; element < bones_count; ++element) {
            bones_values[element].m11 = (float)bonesFloating_values[element * 16 + 0];
            bones_values[element].m12 = (float)bonesFloating_values[element * 16 + 1];
            bones_values[element].m13 = (float)bonesFloating_values[element * 16 + 2];
            bones_values[element].m14 = (float)bonesFloating_values[element * 16 + 3];
            bones_values[element].m21 = (float)bonesFloating_values[element * 16 + 4];
            bones_values[element].m22 = (float)bonesFloating_values[element * 16 + 5];
            bones_values[element].m23 = (float)bonesFloating_values[element * 16 + 6];
            bones_values[element].m24 = (float)bonesFloating_values[element * 16 + 7];
            bones_values[element].m31 = (float)bonesFloating_values[element * 16 + 8];
            bones_values[element].m32 = (float)bonesFloating_values[element * 16 + 9];
            bones_values[element].m33 = (float)bonesFloating_values[element * 16 + 10];
            bones_values[element].m34 = (float)bonesFloating_values[element * 16 + 11];
            bones_values[element].m41 = (float)bonesFloating_values[element * 16 + 12];
            bones_values[element].m42 = (float)bonesFloating_values[element * 16 + 13];
            bones_values[element].m43 = (float)bonesFloating_values[element * 16 + 14];
            bones_values[element].m44 = (float)bonesFloating_values[element * 16 + 15];
        }
        free(bonesFloating_values);
    }
    CNA_AvatarExpression expression_value;
    memset(&expression_value, 0, sizeof expression_value);
    expression_value.struct_size = (uint32_t)(sizeof expression_value);
    expression_value.struct_version = UINT32_C(1);
    {
        jlong expressionIntegral_values[5];
        (*environment)->GetLongArrayRegion(environment, expressionIntegral, 0, 5, expressionIntegral_values);
        expression_value.mouth = (CNA_AvatarMouth)expressionIntegral_values[0];
        expression_value.left_eye = (CNA_AvatarEye)expressionIntegral_values[1];
        expression_value.right_eye = (CNA_AvatarEye)expressionIntegral_values[2];
        expression_value.left_eyebrow = (CNA_AvatarEyebrow)expressionIntegral_values[3];
        expression_value.right_eyebrow = (CNA_AvatarEyebrow)expressionIntegral_values[4];
    }
    CNA_Result call_result = cna.avatar_renderer_draw_bones((CNA_AvatarRendererHandle)renderer, bones_values, (uint64_t)bones_count, &expression_value);
    free(bones_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererGetBindPoseAt(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jint index, jfloatArray out_transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_transform_value;
    memset(&out_transform_value, 0, sizeof out_transform_value);
    CNA_Result call_result = cna.avatar_renderer_get_bind_pose_at((CNA_AvatarRendererHandle)renderer, (int32_t)index, &out_transform_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_transformFloating_values[] = {
            (jfloat)out_transform_value.m11,
            (jfloat)out_transform_value.m12,
            (jfloat)out_transform_value.m13,
            (jfloat)out_transform_value.m14,
            (jfloat)out_transform_value.m21,
            (jfloat)out_transform_value.m22,
            (jfloat)out_transform_value.m23,
            (jfloat)out_transform_value.m24,
            (jfloat)out_transform_value.m31,
            (jfloat)out_transform_value.m32,
            (jfloat)out_transform_value.m33,
            (jfloat)out_transform_value.m34,
            (jfloat)out_transform_value.m41,
            (jfloat)out_transform_value.m42,
            (jfloat)out_transform_value.m43,
            (jfloat)out_transform_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_transformFloating, 0,
            (jsize)(sizeof out_transformFloating_values / sizeof out_transformFloating_values[0]),
            out_transformFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AvatarRendererInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.avatar_renderer_get_info((CNA_AvatarRendererHandle)renderer, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.state,
            (jlong)out_info_value.is_disposed
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererGetLighting(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jfloatArray out_light_colorFloating, jfloatArray out_light_directionFloating, jfloatArray out_ambient_light_colorFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_light_colorFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (out_light_directionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (out_ambient_light_colorFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_light_color_value;
    memset(&out_light_color_value, 0, sizeof out_light_color_value);
    CNA_Vector3 out_light_direction_value;
    memset(&out_light_direction_value, 0, sizeof out_light_direction_value);
    CNA_Vector3 out_ambient_light_color_value;
    memset(&out_ambient_light_color_value, 0, sizeof out_ambient_light_color_value);
    CNA_Result call_result = cna.avatar_renderer_get_lighting((CNA_AvatarRendererHandle)renderer, &out_light_color_value, &out_light_direction_value, &out_ambient_light_color_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_light_colorFloating_values[] = {
            (jfloat)out_light_color_value.x,
            (jfloat)out_light_color_value.y,
            (jfloat)out_light_color_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_light_colorFloating, 0,
            (jsize)(sizeof out_light_colorFloating_values / sizeof out_light_colorFloating_values[0]),
            out_light_colorFloating_values);
        jfloat out_light_directionFloating_values[] = {
            (jfloat)out_light_direction_value.x,
            (jfloat)out_light_direction_value.y,
            (jfloat)out_light_direction_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_light_directionFloating, 0,
            (jsize)(sizeof out_light_directionFloating_values / sizeof out_light_directionFloating_values[0]),
            out_light_directionFloating_values);
        jfloat out_ambient_light_colorFloating_values[] = {
            (jfloat)out_ambient_light_color_value.x,
            (jfloat)out_ambient_light_color_value.y,
            (jfloat)out_ambient_light_color_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_ambient_light_colorFloating, 0,
            (jsize)(sizeof out_ambient_light_colorFloating_values / sizeof out_ambient_light_colorFloating_values[0]),
            out_ambient_light_colorFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererGetParentBoneAt(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jint index, jintArray out_parent_index)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_parent_index_value = 0;
    CNA_Result call_result = cna.avatar_renderer_get_parent_bone_at((CNA_AvatarRendererHandle)renderer, (int32_t)index, &out_parent_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_parent_index_element = (jint)out_parent_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_parent_index, 0, 1, &out_parent_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererGetTransforms(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jfloatArray out_worldFloating, jfloatArray out_viewFloating, jfloatArray out_projectionFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_worldFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (out_viewFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (out_projectionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_world_value;
    memset(&out_world_value, 0, sizeof out_world_value);
    CNA_Matrix out_view_value;
    memset(&out_view_value, 0, sizeof out_view_value);
    CNA_Matrix out_projection_value;
    memset(&out_projection_value, 0, sizeof out_projection_value);
    CNA_Result call_result = cna.avatar_renderer_get_transforms((CNA_AvatarRendererHandle)renderer, &out_world_value, &out_view_value, &out_projection_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_worldFloating_values[] = {
            (jfloat)out_world_value.m11,
            (jfloat)out_world_value.m12,
            (jfloat)out_world_value.m13,
            (jfloat)out_world_value.m14,
            (jfloat)out_world_value.m21,
            (jfloat)out_world_value.m22,
            (jfloat)out_world_value.m23,
            (jfloat)out_world_value.m24,
            (jfloat)out_world_value.m31,
            (jfloat)out_world_value.m32,
            (jfloat)out_world_value.m33,
            (jfloat)out_world_value.m34,
            (jfloat)out_world_value.m41,
            (jfloat)out_world_value.m42,
            (jfloat)out_world_value.m43,
            (jfloat)out_world_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_worldFloating, 0,
            (jsize)(sizeof out_worldFloating_values / sizeof out_worldFloating_values[0]),
            out_worldFloating_values);
        jfloat out_viewFloating_values[] = {
            (jfloat)out_view_value.m11,
            (jfloat)out_view_value.m12,
            (jfloat)out_view_value.m13,
            (jfloat)out_view_value.m14,
            (jfloat)out_view_value.m21,
            (jfloat)out_view_value.m22,
            (jfloat)out_view_value.m23,
            (jfloat)out_view_value.m24,
            (jfloat)out_view_value.m31,
            (jfloat)out_view_value.m32,
            (jfloat)out_view_value.m33,
            (jfloat)out_view_value.m34,
            (jfloat)out_view_value.m41,
            (jfloat)out_view_value.m42,
            (jfloat)out_view_value.m43,
            (jfloat)out_view_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_viewFloating, 0,
            (jsize)(sizeof out_viewFloating_values / sizeof out_viewFloating_values[0]),
            out_viewFloating_values);
        jfloat out_projectionFloating_values[] = {
            (jfloat)out_projection_value.m11,
            (jfloat)out_projection_value.m12,
            (jfloat)out_projection_value.m13,
            (jfloat)out_projection_value.m14,
            (jfloat)out_projection_value.m21,
            (jfloat)out_projection_value.m22,
            (jfloat)out_projection_value.m23,
            (jfloat)out_projection_value.m24,
            (jfloat)out_projection_value.m31,
            (jfloat)out_projection_value.m32,
            (jfloat)out_projection_value.m33,
            (jfloat)out_projection_value.m34,
            (jfloat)out_projection_value.m41,
            (jfloat)out_projection_value.m42,
            (jfloat)out_projection_value.m43,
            (jfloat)out_projection_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_projectionFloating, 0,
            (jsize)(sizeof out_projectionFloating_values / sizeof out_projectionFloating_values[0]),
            out_projectionFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererSetLighting(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jfloatArray light_colorFloating, jfloatArray light_directionFloating, jfloatArray ambient_light_colorFloating)
{
    (void)environment;
    (void)declaring_class;
    if (light_colorFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (light_directionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (ambient_light_colorFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 light_color_value;
    memset(&light_color_value, 0, sizeof light_color_value);
    {
        jfloat light_colorFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, light_colorFloating, 0, 3, light_colorFloating_values);
        light_color_value.x = (float)light_colorFloating_values[0];
        light_color_value.y = (float)light_colorFloating_values[1];
        light_color_value.z = (float)light_colorFloating_values[2];
    }
    CNA_Vector3 light_direction_value;
    memset(&light_direction_value, 0, sizeof light_direction_value);
    {
        jfloat light_directionFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, light_directionFloating, 0, 3, light_directionFloating_values);
        light_direction_value.x = (float)light_directionFloating_values[0];
        light_direction_value.y = (float)light_directionFloating_values[1];
        light_direction_value.z = (float)light_directionFloating_values[2];
    }
    CNA_Vector3 ambient_light_color_value;
    memset(&ambient_light_color_value, 0, sizeof ambient_light_color_value);
    {
        jfloat ambient_light_colorFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, ambient_light_colorFloating, 0, 3, ambient_light_colorFloating_values);
        ambient_light_color_value.x = (float)ambient_light_colorFloating_values[0];
        ambient_light_color_value.y = (float)ambient_light_colorFloating_values[1];
        ambient_light_color_value.z = (float)ambient_light_colorFloating_values[2];
    }
    CNA_Result call_result = cna.avatar_renderer_set_lighting((CNA_AvatarRendererHandle)renderer, &light_color_value, &light_direction_value, &ambient_light_color_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_avatarRendererSetTransforms(
    JNIEnv* environment, jclass declaring_class, jlong renderer, jfloatArray worldFloating, jfloatArray viewFloating, jfloatArray projectionFloating)
{
    (void)environment;
    (void)declaring_class;
    if (worldFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (viewFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (projectionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix world_value;
    memset(&world_value, 0, sizeof world_value);
    {
        jfloat worldFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, worldFloating, 0, 16, worldFloating_values);
        world_value.m11 = (float)worldFloating_values[0];
        world_value.m12 = (float)worldFloating_values[1];
        world_value.m13 = (float)worldFloating_values[2];
        world_value.m14 = (float)worldFloating_values[3];
        world_value.m21 = (float)worldFloating_values[4];
        world_value.m22 = (float)worldFloating_values[5];
        world_value.m23 = (float)worldFloating_values[6];
        world_value.m24 = (float)worldFloating_values[7];
        world_value.m31 = (float)worldFloating_values[8];
        world_value.m32 = (float)worldFloating_values[9];
        world_value.m33 = (float)worldFloating_values[10];
        world_value.m34 = (float)worldFloating_values[11];
        world_value.m41 = (float)worldFloating_values[12];
        world_value.m42 = (float)worldFloating_values[13];
        world_value.m43 = (float)worldFloating_values[14];
        world_value.m44 = (float)worldFloating_values[15];
    }
    CNA_Matrix view_value;
    memset(&view_value, 0, sizeof view_value);
    {
        jfloat viewFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, viewFloating, 0, 16, viewFloating_values);
        view_value.m11 = (float)viewFloating_values[0];
        view_value.m12 = (float)viewFloating_values[1];
        view_value.m13 = (float)viewFloating_values[2];
        view_value.m14 = (float)viewFloating_values[3];
        view_value.m21 = (float)viewFloating_values[4];
        view_value.m22 = (float)viewFloating_values[5];
        view_value.m23 = (float)viewFloating_values[6];
        view_value.m24 = (float)viewFloating_values[7];
        view_value.m31 = (float)viewFloating_values[8];
        view_value.m32 = (float)viewFloating_values[9];
        view_value.m33 = (float)viewFloating_values[10];
        view_value.m34 = (float)viewFloating_values[11];
        view_value.m41 = (float)viewFloating_values[12];
        view_value.m42 = (float)viewFloating_values[13];
        view_value.m43 = (float)viewFloating_values[14];
        view_value.m44 = (float)viewFloating_values[15];
    }
    CNA_Matrix projection_value;
    memset(&projection_value, 0, sizeof projection_value);
    {
        jfloat projectionFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, projectionFloating, 0, 16, projectionFloating_values);
        projection_value.m11 = (float)projectionFloating_values[0];
        projection_value.m12 = (float)projectionFloating_values[1];
        projection_value.m13 = (float)projectionFloating_values[2];
        projection_value.m14 = (float)projectionFloating_values[3];
        projection_value.m21 = (float)projectionFloating_values[4];
        projection_value.m22 = (float)projectionFloating_values[5];
        projection_value.m23 = (float)projectionFloating_values[6];
        projection_value.m24 = (float)projectionFloating_values[7];
        projection_value.m31 = (float)projectionFloating_values[8];
        projection_value.m32 = (float)projectionFloating_values[9];
        projection_value.m33 = (float)projectionFloating_values[10];
        projection_value.m34 = (float)projectionFloating_values[11];
        projection_value.m41 = (float)projectionFloating_values[12];
        projection_value.m42 = (float)projectionFloating_values[13];
        projection_value.m43 = (float)projectionFloating_values[14];
        projection_value.m44 = (float)projectionFloating_values[15];
    }
    CNA_Result call_result = cna.avatar_renderer_set_transforms((CNA_AvatarRendererHandle)renderer, &world_value, &view_value, &projection_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_friendCollectionGetIsDisposed(
    JNIEnv* environment, jclass declaring_class, jlong collection, jbooleanArray out_is_disposed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_disposed_value = 0;
    CNA_Result call_result = cna.friend_collection_get_is_disposed((CNA_GamerCollectionHandle)collection, &out_is_disposed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_disposed_element = (jboolean)out_is_disposed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_disposed, 0, 1, &out_is_disposed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_friendGamerCopyPresence(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.friend_gamer_copy_presence((CNA_GamerHandle)gamer, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_friendGamerGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_FriendGamerInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.friend_gamer_get_info((CNA_GamerHandle)gamer, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2],
            (jbyte)out_info_value.reserved[3]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.friend_request_received_from,
            (jlong)out_info_value.friend_request_sent_to,
            (jlong)out_info_value.has_voice,
            (jlong)out_info_value.invite_accepted,
            (jlong)out_info_value.invite_received_from,
            (jlong)out_info_value.invite_rejected,
            (jlong)out_info_value.invite_sent_to,
            (jlong)out_info_value.is_away,
            (jlong)out_info_value.is_busy,
            (jlong)out_info_value.is_joinable,
            (jlong)out_info_value.is_online,
            (jlong)out_info_value.is_playing
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_friendGamerGetPresenceSize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.friend_gamer_get_presence_size((CNA_GamerHandle)gamer, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCollectionCreateEnumerator(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_enumerator)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamerEnumeratorHandle out_enumerator_value = 0;
    CNA_Result call_result = cna.gamer_collection_create_enumerator((CNA_GamerCollectionHandle)collection, &out_enumerator_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_enumerator_element = (jlong)out_enumerator_value;
        (*environment)->SetLongArrayRegion(
            environment, out_enumerator, 0, 1, &out_enumerator_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gamer_collection_destroy((CNA_GamerCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jint index, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.gamer_collection_get_at((CNA_GamerCollectionHandle)collection, (int32_t)index, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.gamer_collection_get_count((CNA_GamerCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCopyDisplayName(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_copy_display_name((CNA_GamerHandle)gamer, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCopyGamertag(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_copy_gamertag((CNA_GamerHandle)gamer, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCopyPartnerToken(
    JNIEnv* environment, jclass declaring_class, jbyteArray audience_uri, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize audience_uri_size = (*environment)->GetArrayLength(environment, audience_uri);
    jbyte* audience_uri_bytes = (*environment)->GetByteArrayElements(environment, audience_uri, NULL);
    if (audience_uri_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView audience_uri_view = {(const char*)audience_uri_bytes, (uint64_t)audience_uri_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, audience_uri, audience_uri_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_copy_partner_token(audience_uri_view, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, audience_uri, audience_uri_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerCopyText(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_copy_text((CNA_GamerHandle)gamer, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerEnumeratorDestroy(
    JNIEnv* environment, jclass declaring_class, jlong enumerator)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gamer_enumerator_destroy((CNA_GamerEnumeratorHandle)enumerator);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerEnumeratorGetCurrent(
    JNIEnv* environment, jclass declaring_class, jlong enumerator, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.gamer_enumerator_get_current((CNA_GamerEnumeratorHandle)enumerator, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerEnumeratorMoveNext(
    JNIEnv* environment, jclass declaring_class, jlong enumerator, jbooleanArray out_has_current)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_current_value = 0;
    CNA_Result call_result = cna.gamer_enumerator_move_next((CNA_GamerEnumeratorHandle)enumerator, &out_has_current_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_current_element = (jboolean)out_has_current_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_current, 0, 1, &out_has_current_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetDisplayNameSize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_get_display_name_size((CNA_GamerHandle)gamer, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetFromGamertag(
    JNIEnv* environment, jclass declaring_class, jbyteArray gamertag, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    jsize gamertag_size = (*environment)->GetArrayLength(environment, gamertag);
    jbyte* gamertag_bytes = (*environment)->GetByteArrayElements(environment, gamertag, NULL);
    if (gamertag_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView gamertag_view = {(const char*)gamertag_bytes, (uint64_t)gamertag_size};
    CNA_GamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.gamer_get_from_gamertag(gamertag_view, &out_gamer_value);
    (*environment)->ReleaseByteArrayElements(environment, gamertag, gamertag_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetGamertagSize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_get_gamertag_size((CNA_GamerHandle)gamer, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetIsDisposed(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_is_disposed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_disposed_value = 0;
    CNA_Result call_result = cna.gamer_get_is_disposed((CNA_GamerHandle)gamer, &out_is_disposed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_disposed_element = (jboolean)out_is_disposed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_disposed, 0, 1, &out_is_disposed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetPartnerTokenSize(
    JNIEnv* environment, jclass declaring_class, jbyteArray audience_uri, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize audience_uri_size = (*environment)->GetArrayLength(environment, audience_uri);
    jbyte* audience_uri_bytes = (*environment)->GetByteArrayElements(environment, audience_uri, NULL);
    if (audience_uri_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView audience_uri_view = {(const char*)audience_uri_bytes, (uint64_t)audience_uri_size};
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_get_partner_token_size(audience_uri_view, &out_bytes_value);
    (*environment)->ReleaseByteArrayElements(environment, audience_uri, audience_uri_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetProfile(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_profile)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamerProfileHandle out_profile_value = 0;
    CNA_Result call_result = cna.gamer_get_profile((CNA_GamerHandle)gamer, &out_profile_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_profile_element = (jlong)out_profile_value;
        (*environment)->SetLongArrayRegion(
            environment, out_profile, 0, 1, &out_profile_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetSignedInGamerAt(
    JNIEnv* environment, jclass declaring_class, jint index, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_SignedInGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.gamer_get_signed_in_gamer_at((int32_t)index, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetSignedInGamerAtPlayerIndex(
    JNIEnv* environment, jclass declaring_class, jint player_index, jbooleanArray out_has_gamer, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_gamer_value = 0;
    CNA_SignedInGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.gamer_get_signed_in_gamer_at_player_index((CNA_PlayerIndex)player_index, &out_has_gamer_value, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_gamer_element = (jboolean)out_has_gamer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_gamer, 0, 1, &out_has_gamer_element);
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetSignedInGamerCount(
    JNIEnv* environment, jclass declaring_class, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.gamer_get_signed_in_gamer_count(&out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetTag(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_tag)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_tag_value = 0;
    CNA_Result call_result = cna.gamer_get_tag((CNA_GamerHandle)gamer, &out_tag_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_tag_element = (jlong)out_tag_value;
        (*environment)->SetLongArrayRegion(
            environment, out_tag, 0, 1, &out_tag_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerGetTextSize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_get_text_size((CNA_GamerHandle)gamer, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileCopyMotto(
    JNIEnv* environment, jclass declaring_class, jlong profile, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_profile_copy_motto((CNA_GamerProfileHandle)profile, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileCopyPicture(
    JNIEnv* environment, jclass declaring_class, jlong profile, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_profile_copy_picture((CNA_GamerProfileHandle)profile, destination_values, (uint64_t)destination_size, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileCopyRegionName(
    JNIEnv* environment, jclass declaring_class, jlong profile, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamer_profile_copy_region_name((CNA_GamerProfileHandle)profile, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileDestroy(
    JNIEnv* environment, jclass declaring_class, jlong profile)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gamer_profile_destroy((CNA_GamerProfileHandle)profile);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong profile, jbyteArray out_infoBytes, jlongArray out_infoIntegral, jfloatArray out_infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL || out_infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GamerProfileInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.gamer_profile_get_info((CNA_GamerProfileHandle)profile, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.gamer_score,
            (jlong)out_info_value.gamer_zone,
            (jlong)out_info_value.titles_played,
            (jlong)out_info_value.total_achievements,
            (jlong)out_info_value.is_disposed
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jfloat out_infoFloating_values[] = {
            (jfloat)out_info_value.reputation
        };
        (*environment)->SetFloatArrayRegion(environment, out_infoFloating, 0,
            (jsize)(sizeof out_infoFloating_values / sizeof out_infoFloating_values[0]),
            out_infoFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileGetMottoSize(
    JNIEnv* environment, jclass declaring_class, jlong profile, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_profile_get_motto_size((CNA_GamerProfileHandle)profile, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileGetPictureSize(
    JNIEnv* environment, jclass declaring_class, jlong profile, jbooleanArray out_has_picture, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_picture_value = 0;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_profile_get_picture_size((CNA_GamerProfileHandle)profile, &out_has_picture_value, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_picture_element = (jboolean)out_has_picture_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_picture, 0, 1, &out_has_picture_element);
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerProfileGetRegionNameSize(
    JNIEnv* environment, jclass declaring_class, jlong profile, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamer_profile_get_region_name_size((CNA_GamerProfileHandle)profile, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerServicesDispatcherGetIsInitialized(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_is_initialized)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_initialized_value = 0;
    CNA_Result call_result = cna.gamer_services_dispatcher_get_is_initialized(&out_is_initialized_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_initialized_element = (jboolean)out_is_initialized_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_initialized, 0, 1, &out_is_initialized_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerServicesDispatcherGetWindowHandle(
    JNIEnv* environment, jclass declaring_class, jlongArray out_window_handle)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_window_handle_value = 0;
    CNA_Result call_result = cna.gamer_services_dispatcher_get_window_handle(&out_window_handle_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_window_handle_element = (jlong)out_window_handle_value;
        (*environment)->SetLongArrayRegion(
            environment, out_window_handle, 0, 1, &out_window_handle_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerSetDisplayName(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray display_name)
{
    (void)environment;
    (void)declaring_class;
    jsize display_name_size = (*environment)->GetArrayLength(environment, display_name);
    jbyte* display_name_bytes = (*environment)->GetByteArrayElements(environment, display_name, NULL);
    if (display_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView display_name_view = {(const char*)display_name_bytes, (uint64_t)display_name_size};
    CNA_Result call_result = cna.gamer_set_display_name((CNA_GamerHandle)gamer, display_name_view);
    (*environment)->ReleaseByteArrayElements(environment, display_name, display_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerSetTag(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong tag)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gamer_set_tag((CNA_GamerHandle)gamer, (uint64_t)tag);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_gamerUnsubscribeExt(
    JNIEnv* environment, jclass declaring_class, jlong registration)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gamer_unsubscribe_ext((CNA_Handle)registration);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideBeginShowKeyboardInput(
    JNIEnv* environment, jclass declaring_class, jint player, jbyteArray title, jbyteArray description, jbyteArray default_text, jboolean use_password_mode)
{
    (void)environment;
    (void)declaring_class;
    jsize title_size = (*environment)->GetArrayLength(environment, title);
    jbyte* title_bytes = (*environment)->GetByteArrayElements(environment, title, NULL);
    if (title_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView title_view = {(const char*)title_bytes, (uint64_t)title_size};
    jsize description_size = (*environment)->GetArrayLength(environment, description);
    jbyte* description_bytes = (*environment)->GetByteArrayElements(environment, description, NULL);
    if (description_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView description_view = {(const char*)description_bytes, (uint64_t)description_size};
    jsize default_text_size = (*environment)->GetArrayLength(environment, default_text);
    jbyte* default_text_bytes = (*environment)->GetByteArrayElements(environment, default_text, NULL);
    if (default_text_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, description, description_bytes, JNI_ABORT);
        (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView default_text_view = {(const char*)default_text_bytes, (uint64_t)default_text_size};
    CNA_Result call_result = cna.guide_begin_show_keyboard_input((CNA_PlayerIndex)player, title_view, description_view, default_text_view, (CNA_Bool)use_password_mode, NULL, NULL);
    (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, description, description_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, default_text, default_text_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideCopyPendingKeyboardInputDescriptionExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.guide_copy_pending_keyboard_input_description_ext((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideCopyPendingKeyboardInputDisplayTextExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.guide_copy_pending_keyboard_input_display_text_ext((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideCopyPendingKeyboardInputTitleExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.guide_copy_pending_keyboard_input_title_ext((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideDelayNotifications(
    JNIEnv* environment, jclass declaring_class, jlong delay_ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_delay_notifications((int64_t)delay_ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideEndShowKeyboardInput(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.guide_end_show_keyboard_input((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideEndShowKeyboardInputSize(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.guide_end_show_keyboard_input_size(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideEndShowMessageBox(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_has_choice, jintArray out_button_index)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_choice_value = 0;
    int32_t out_button_index_value = 0;
    CNA_Result call_result = cna.guide_end_show_message_box(&out_has_choice_value, &out_button_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_choice_element = (jboolean)out_has_choice_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_choice, 0, 1, &out_has_choice_element);
        jint out_button_index_element = (jint)out_button_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_button_index, 0, 1, &out_button_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetHasPendingKeyboardInputExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_has_pending)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_pending_value = 0;
    CNA_Result call_result = cna.guide_get_has_pending_keyboard_input_ext(&out_has_pending_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_pending_element = (jboolean)out_has_pending_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_pending, 0, 1, &out_has_pending_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetHasPendingMessageBoxExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_has_pending)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_pending_value = 0;
    CNA_Result call_result = cna.guide_get_has_pending_message_box_ext(&out_has_pending_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_pending_element = (jboolean)out_has_pending_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_pending, 0, 1, &out_has_pending_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetIsScreenSaverEnabled(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_is_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_enabled_value = 0;
    CNA_Result call_result = cna.guide_get_is_screen_saver_enabled(&out_is_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_enabled_element = (jboolean)out_is_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_enabled, 0, 1, &out_is_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetIsTrialMode(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_is_trial_mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_trial_mode_value = 0;
    CNA_Result call_result = cna.guide_get_is_trial_mode(&out_is_trial_mode_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_trial_mode_element = (jboolean)out_is_trial_mode_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_trial_mode, 0, 1, &out_is_trial_mode_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetIsVisible(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_is_visible)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_visible_value = 0;
    CNA_Result call_result = cna.guide_get_is_visible(&out_is_visible_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_visible_element = (jboolean)out_is_visible_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_visible, 0, 1, &out_is_visible_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetNotificationPosition(
    JNIEnv* environment, jclass declaring_class, jintArray out_position)
{
    (void)environment;
    (void)declaring_class;
    CNA_NotificationPosition out_position_value = 0;
    CNA_Result call_result = cna.guide_get_notification_position(&out_position_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_position_element = (jint)out_position_value;
        (*environment)->SetIntArrayRegion(
            environment, out_position, 0, 1, &out_position_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetPendingKeyboardInputDescriptionSizeExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.guide_get_pending_keyboard_input_description_size_ext(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetPendingKeyboardInputDisplayTextSizeExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.guide_get_pending_keyboard_input_display_text_size_ext(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetPendingKeyboardInputTitleSizeExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.guide_get_pending_keyboard_input_title_size_ext(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetPendingMessageBoxFocusButtonExt(
    JNIEnv* environment, jclass declaring_class, jintArray out_focus_button)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_focus_button_value = 0;
    CNA_Result call_result = cna.guide_get_pending_message_box_focus_button_ext(&out_focus_button_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_focus_button_element = (jint)out_focus_button_value;
        (*environment)->SetIntArrayRegion(
            environment, out_focus_button, 0, 1, &out_focus_button_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideGetSimulateTrialMode(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_simulate)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_simulate_value = 0;
    CNA_Result call_result = cna.guide_get_simulate_trial_mode(&out_simulate_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_simulate_element = (jboolean)out_simulate_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_simulate, 0, 1, &out_simulate_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideRenderPendingKeyboardInputExt(
    JNIEnv* environment, jclass declaring_class, jlong device, jlong sprite_batch, jlong font, jlong white_pixel)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_render_pending_keyboard_input_ext((CNA_Handle)device, (CNA_Handle)sprite_batch, (CNA_Handle)font, (CNA_Handle)white_pixel);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideRenderPendingMessageBoxExt(
    JNIEnv* environment, jclass declaring_class, jlong device, jlong sprite_batch, jlong font, jlong white_pixel)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_render_pending_message_box_ext((CNA_Handle)device, (CNA_Handle)sprite_batch, (CNA_Handle)font, (CNA_Handle)white_pixel);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideResetPendingKeyboardInputExt(
    JNIEnv* environment, jclass declaring_class)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_reset_pending_keyboard_input_ext();
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideResetPendingMessageBoxExt(
    JNIEnv* environment, jclass declaring_class)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_reset_pending_message_box_ext();
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideSetIsScreenSaverEnabled(
    JNIEnv* environment, jclass declaring_class, jboolean is_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_set_is_screen_saver_enabled((CNA_Bool)is_enabled);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideSetNotificationPosition(
    JNIEnv* environment, jclass declaring_class, jint position)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_set_notification_position((CNA_NotificationPosition)position);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideSetSimulateTrialMode(
    JNIEnv* environment, jclass declaring_class, jboolean simulate)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_set_simulate_trial_mode((CNA_Bool)simulate);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowAchievementsExt(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_achievements_ext((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowComposeMessage(
    JNIEnv* environment, jclass declaring_class, jint player, jbyteArray text, jlongArray recipients)
{
    (void)environment;
    (void)declaring_class;
    jsize text_size = (*environment)->GetArrayLength(environment, text);
    jbyte* text_bytes = (*environment)->GetByteArrayElements(environment, text, NULL);
    if (text_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView text_view = {(const char*)text_bytes, (uint64_t)text_size};
    jsize recipients_size = (*environment)->GetArrayLength(environment, recipients);
    jlong* recipients_elements = (*environment)->GetLongArrayElements(environment, recipients, NULL);
    if (recipients_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GamerHandle* recipients_values = (CNA_GamerHandle*)malloc(
        ((size_t)recipients_size + 1U) * sizeof(*recipients_values));
    if (recipients_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, recipients, recipients_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < recipients_size; ++index) {
        recipients_values[index] = (CNA_GamerHandle)recipients_elements[index];
    }
    CNA_Result call_result = cna.guide_show_compose_message((CNA_PlayerIndex)player, text_view, recipients_values, (uint64_t)recipients_size);
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
    free(recipients_values);
    (*environment)->ReleaseLongArrayElements(
        environment, recipients, recipients_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowFriendRequest(
    JNIEnv* environment, jclass declaring_class, jint player, jlong gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_friend_request((CNA_PlayerIndex)player, (CNA_GamerHandle)gamer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowFriends(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_friends((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowGameInvite(
    JNIEnv* environment, jclass declaring_class, jint player, jlongArray recipients)
{
    (void)environment;
    (void)declaring_class;
    jsize recipients_size = (*environment)->GetArrayLength(environment, recipients);
    jlong* recipients_elements = (*environment)->GetLongArrayElements(environment, recipients, NULL);
    if (recipients_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GamerHandle* recipients_values = (CNA_GamerHandle*)malloc(
        ((size_t)recipients_size + 1U) * sizeof(*recipients_values));
    if (recipients_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, recipients, recipients_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < recipients_size; ++index) {
        recipients_values[index] = (CNA_GamerHandle)recipients_elements[index];
    }
    CNA_Result call_result = cna.guide_show_game_invite((CNA_PlayerIndex)player, recipients_values, (uint64_t)recipients_size);
    free(recipients_values);
    (*environment)->ReleaseLongArrayElements(
        environment, recipients, recipients_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowGameInviteForSession(
    JNIEnv* environment, jclass declaring_class, jbyteArray session_id)
{
    (void)environment;
    (void)declaring_class;
    jsize session_id_size = (*environment)->GetArrayLength(environment, session_id);
    jbyte* session_id_bytes = (*environment)->GetByteArrayElements(environment, session_id, NULL);
    if (session_id_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView session_id_view = {(const char*)session_id_bytes, (uint64_t)session_id_size};
    CNA_Result call_result = cna.guide_show_game_invite_for_session(session_id_view);
    (*environment)->ReleaseByteArrayElements(environment, session_id, session_id_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowGamerCard(
    JNIEnv* environment, jclass declaring_class, jint player, jlong gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_gamer_card((CNA_PlayerIndex)player, (CNA_GamerHandle)gamer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowMarketplace(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_marketplace((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowMessages(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_messages((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowParty(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_party((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowPartySessions(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_party_sessions((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowPlayerReview(
    JNIEnv* environment, jclass declaring_class, jint player, jlong gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_player_review((CNA_PlayerIndex)player, (CNA_GamerHandle)gamer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowPlayers(
    JNIEnv* environment, jclass declaring_class, jint player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_players((CNA_PlayerIndex)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideShowSignIn(
    JNIEnv* environment, jclass declaring_class, jint pane_count, jboolean online_only)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_show_sign_in((int32_t)pane_count, (CNA_Bool)online_only);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideSimulateKeyboardInputCancelExt(
    JNIEnv* environment, jclass declaring_class)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_simulate_keyboard_input_cancel_ext();
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideSimulateMessageBoxClickExt(
    JNIEnv* environment, jclass declaring_class, jint button_index)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.guide_simulate_message_box_click_ext((int32_t)button_index);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_guideWasKeyboardInputCanceledExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_was_canceled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_was_canceled_value = 0;
    CNA_Result call_result = cna.guide_was_keyboard_input_canceled_ext(&out_was_canceled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_was_canceled_element = (jboolean)out_was_canceled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_was_canceled, 0, 1, &out_was_canceled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntryCreateExt(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong rating, jint ranking, jlongArray out_entry)
{
    (void)environment;
    (void)declaring_class;
    CNA_LeaderboardEntryHandle out_entry_value = 0;
    CNA_Result call_result = cna.leaderboard_entry_create_ext((CNA_GamerHandle)gamer, (int64_t)rating, (int32_t)ranking, &out_entry_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_entry_element = (jlong)out_entry_value;
        (*environment)->SetLongArrayRegion(
            environment, out_entry, 0, 1, &out_entry_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntryDestroy(
    JNIEnv* environment, jclass declaring_class, jlong entry)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.leaderboard_entry_destroy((CNA_LeaderboardEntryHandle)entry);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntryGetColumns(
    JNIEnv* environment, jclass declaring_class, jlong entry, jlongArray out_columns)
{
    (void)environment;
    (void)declaring_class;
    CNA_PropertyDictionaryHandle out_columns_value = 0;
    CNA_Result call_result = cna.leaderboard_entry_get_columns((CNA_LeaderboardEntryHandle)entry, &out_columns_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_columns_element = (jlong)out_columns_value;
        (*environment)->SetLongArrayRegion(
            environment, out_columns, 0, 1, &out_columns_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntryGetGamer(
    JNIEnv* environment, jclass declaring_class, jlong entry, jbooleanArray out_has_gamer, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_gamer_value = 0;
    CNA_GamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.leaderboard_entry_get_gamer((CNA_LeaderboardEntryHandle)entry, &out_has_gamer_value, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_gamer_element = (jboolean)out_has_gamer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_gamer, 0, 1, &out_has_gamer_element);
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntryGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong entry, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardEntryInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.leaderboard_entry_get_info((CNA_LeaderboardEntryHandle)entry, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.ranking,
            (jlong)out_info_value.has_gamer,
            (jlong)out_info_value.rating
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardEntrySetRating(
    JNIEnv* environment, jclass declaring_class, jlong entry, jlong rating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.leaderboard_entry_set_rating((CNA_LeaderboardEntryHandle)entry, (int64_t)rating);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderDestroy(
    JNIEnv* environment, jclass declaring_class, jlong reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.leaderboard_reader_destroy((CNA_LeaderboardReaderHandle)reader);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderGetEntryAt(
    JNIEnv* environment, jclass declaring_class, jlong reader, jint index, jlongArray out_entry)
{
    (void)environment;
    (void)declaring_class;
    CNA_LeaderboardEntryHandle out_entry_value = 0;
    CNA_Result call_result = cna.leaderboard_reader_get_entry_at((CNA_LeaderboardReaderHandle)reader, (int32_t)index, &out_entry_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_entry_element = (jlong)out_entry_value;
        (*environment)->SetLongArrayRegion(
            environment, out_entry, 0, 1, &out_entry_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderGetIdentity(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray out_identityBytes, jlongArray out_identityIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_identityBytes == NULL || out_identityIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardIdentity out_identity_value;
    memset(&out_identity_value, 0, sizeof out_identity_value);
    out_identity_value.struct_size = (uint32_t)(sizeof out_identity_value);
    out_identity_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.leaderboard_reader_get_identity((CNA_LeaderboardReaderHandle)reader, &out_identity_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_identityBytes_values[] = {
            (jbyte)out_identity_value.key[0],
            (jbyte)out_identity_value.key[1],
            (jbyte)out_identity_value.key[2],
            (jbyte)out_identity_value.key[3],
            (jbyte)out_identity_value.key[4],
            (jbyte)out_identity_value.key[5],
            (jbyte)out_identity_value.key[6],
            (jbyte)out_identity_value.key[7],
            (jbyte)out_identity_value.key[8],
            (jbyte)out_identity_value.key[9],
            (jbyte)out_identity_value.key[10],
            (jbyte)out_identity_value.key[11],
            (jbyte)out_identity_value.key[12],
            (jbyte)out_identity_value.key[13],
            (jbyte)out_identity_value.key[14],
            (jbyte)out_identity_value.key[15],
            (jbyte)out_identity_value.key[16],
            (jbyte)out_identity_value.key[17],
            (jbyte)out_identity_value.key[18],
            (jbyte)out_identity_value.key[19],
            (jbyte)out_identity_value.key[20],
            (jbyte)out_identity_value.key[21],
            (jbyte)out_identity_value.key[22],
            (jbyte)out_identity_value.key[23],
            (jbyte)out_identity_value.key[24],
            (jbyte)out_identity_value.key[25],
            (jbyte)out_identity_value.key[26],
            (jbyte)out_identity_value.key[27],
            (jbyte)out_identity_value.key[28],
            (jbyte)out_identity_value.key[29],
            (jbyte)out_identity_value.key[30],
            (jbyte)out_identity_value.key[31],
            (jbyte)out_identity_value.key[32],
            (jbyte)out_identity_value.key[33],
            (jbyte)out_identity_value.key[34],
            (jbyte)out_identity_value.key[35],
            (jbyte)out_identity_value.key[36],
            (jbyte)out_identity_value.key[37],
            (jbyte)out_identity_value.key[38],
            (jbyte)out_identity_value.key[39],
            (jbyte)out_identity_value.key[40],
            (jbyte)out_identity_value.key[41],
            (jbyte)out_identity_value.key[42],
            (jbyte)out_identity_value.key[43],
            (jbyte)out_identity_value.key[44],
            (jbyte)out_identity_value.key[45],
            (jbyte)out_identity_value.key[46],
            (jbyte)out_identity_value.key[47],
            (jbyte)out_identity_value.key[48],
            (jbyte)out_identity_value.key[49],
            (jbyte)out_identity_value.key[50],
            (jbyte)out_identity_value.key[51],
            (jbyte)out_identity_value.key[52],
            (jbyte)out_identity_value.key[53],
            (jbyte)out_identity_value.key[54],
            (jbyte)out_identity_value.key[55],
            (jbyte)out_identity_value.key[56],
            (jbyte)out_identity_value.key[57],
            (jbyte)out_identity_value.key[58],
            (jbyte)out_identity_value.key[59],
            (jbyte)out_identity_value.key[60],
            (jbyte)out_identity_value.key[61],
            (jbyte)out_identity_value.key[62],
            (jbyte)out_identity_value.key[63]
        };
        (*environment)->SetByteArrayRegion(environment, out_identityBytes, 0,
            (jsize)(sizeof out_identityBytes_values / sizeof out_identityBytes_values[0]),
            out_identityBytes_values);
        jlong out_identityIntegral_values[] = {
            (jlong)out_identity_value.game_mode
        };
        (*environment)->SetLongArrayRegion(environment, out_identityIntegral, 0,
            (jsize)(sizeof out_identityIntegral_values / sizeof out_identityIntegral_values[0]),
            out_identityIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardReaderInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.leaderboard_reader_get_info((CNA_LeaderboardReaderHandle)reader, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.page_start,
            (jlong)out_info_value.total_leaderboard_size,
            (jlong)out_info_value.entry_count,
            (jlong)out_info_value.is_disposed,
            (jlong)out_info_value.can_page_down,
            (jlong)out_info_value.can_page_up,
            (jlong)out_info_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderPageDown(
    JNIEnv* environment, jclass declaring_class, jlong reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.leaderboard_reader_page_down((CNA_LeaderboardReaderHandle)reader);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderPageUp(
    JNIEnv* environment, jclass declaring_class, jlong reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.leaderboard_reader_page_up((CNA_LeaderboardReaderHandle)reader);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderRead(
    JNIEnv* environment, jclass declaring_class, jbyteArray identityBytes, jlongArray identityIntegral, jint page_start, jint page_size, jlongArray out_reader)
{
    (void)environment;
    (void)declaring_class;
    if (identityBytes == NULL || identityIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardIdentity identity_value;
    memset(&identity_value, 0, sizeof identity_value);
    identity_value.struct_size = (uint32_t)(sizeof identity_value);
    identity_value.struct_version = UINT32_C(1);
    {
        jbyte identityBytes_values[64];
        (*environment)->GetByteArrayRegion(environment, identityBytes, 0, 64, identityBytes_values);
        identity_value.key[0] = (char)identityBytes_values[0];
        identity_value.key[1] = (char)identityBytes_values[1];
        identity_value.key[2] = (char)identityBytes_values[2];
        identity_value.key[3] = (char)identityBytes_values[3];
        identity_value.key[4] = (char)identityBytes_values[4];
        identity_value.key[5] = (char)identityBytes_values[5];
        identity_value.key[6] = (char)identityBytes_values[6];
        identity_value.key[7] = (char)identityBytes_values[7];
        identity_value.key[8] = (char)identityBytes_values[8];
        identity_value.key[9] = (char)identityBytes_values[9];
        identity_value.key[10] = (char)identityBytes_values[10];
        identity_value.key[11] = (char)identityBytes_values[11];
        identity_value.key[12] = (char)identityBytes_values[12];
        identity_value.key[13] = (char)identityBytes_values[13];
        identity_value.key[14] = (char)identityBytes_values[14];
        identity_value.key[15] = (char)identityBytes_values[15];
        identity_value.key[16] = (char)identityBytes_values[16];
        identity_value.key[17] = (char)identityBytes_values[17];
        identity_value.key[18] = (char)identityBytes_values[18];
        identity_value.key[19] = (char)identityBytes_values[19];
        identity_value.key[20] = (char)identityBytes_values[20];
        identity_value.key[21] = (char)identityBytes_values[21];
        identity_value.key[22] = (char)identityBytes_values[22];
        identity_value.key[23] = (char)identityBytes_values[23];
        identity_value.key[24] = (char)identityBytes_values[24];
        identity_value.key[25] = (char)identityBytes_values[25];
        identity_value.key[26] = (char)identityBytes_values[26];
        identity_value.key[27] = (char)identityBytes_values[27];
        identity_value.key[28] = (char)identityBytes_values[28];
        identity_value.key[29] = (char)identityBytes_values[29];
        identity_value.key[30] = (char)identityBytes_values[30];
        identity_value.key[31] = (char)identityBytes_values[31];
        identity_value.key[32] = (char)identityBytes_values[32];
        identity_value.key[33] = (char)identityBytes_values[33];
        identity_value.key[34] = (char)identityBytes_values[34];
        identity_value.key[35] = (char)identityBytes_values[35];
        identity_value.key[36] = (char)identityBytes_values[36];
        identity_value.key[37] = (char)identityBytes_values[37];
        identity_value.key[38] = (char)identityBytes_values[38];
        identity_value.key[39] = (char)identityBytes_values[39];
        identity_value.key[40] = (char)identityBytes_values[40];
        identity_value.key[41] = (char)identityBytes_values[41];
        identity_value.key[42] = (char)identityBytes_values[42];
        identity_value.key[43] = (char)identityBytes_values[43];
        identity_value.key[44] = (char)identityBytes_values[44];
        identity_value.key[45] = (char)identityBytes_values[45];
        identity_value.key[46] = (char)identityBytes_values[46];
        identity_value.key[47] = (char)identityBytes_values[47];
        identity_value.key[48] = (char)identityBytes_values[48];
        identity_value.key[49] = (char)identityBytes_values[49];
        identity_value.key[50] = (char)identityBytes_values[50];
        identity_value.key[51] = (char)identityBytes_values[51];
        identity_value.key[52] = (char)identityBytes_values[52];
        identity_value.key[53] = (char)identityBytes_values[53];
        identity_value.key[54] = (char)identityBytes_values[54];
        identity_value.key[55] = (char)identityBytes_values[55];
        identity_value.key[56] = (char)identityBytes_values[56];
        identity_value.key[57] = (char)identityBytes_values[57];
        identity_value.key[58] = (char)identityBytes_values[58];
        identity_value.key[59] = (char)identityBytes_values[59];
        identity_value.key[60] = (char)identityBytes_values[60];
        identity_value.key[61] = (char)identityBytes_values[61];
        identity_value.key[62] = (char)identityBytes_values[62];
        identity_value.key[63] = (char)identityBytes_values[63];
    }
    {
        jlong identityIntegral_values[1];
        (*environment)->GetLongArrayRegion(environment, identityIntegral, 0, 1, identityIntegral_values);
        identity_value.game_mode = (int32_t)identityIntegral_values[0];
    }
    CNA_LeaderboardReaderHandle out_reader_value = 0;
    CNA_Result call_result = cna.leaderboard_reader_read(&identity_value, (int32_t)page_start, (int32_t)page_size, &out_reader_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reader_element = (jlong)out_reader_value;
        (*environment)->SetLongArrayRegion(
            environment, out_reader, 0, 1, &out_reader_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderReadFromGamers(
    JNIEnv* environment, jclass declaring_class, jbyteArray identityBytes, jlongArray identityIntegral, jlongArray gamers, jlong pivot_gamer, jint page_size, jlongArray out_reader)
{
    (void)environment;
    (void)declaring_class;
    if (identityBytes == NULL || identityIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardIdentity identity_value;
    memset(&identity_value, 0, sizeof identity_value);
    identity_value.struct_size = (uint32_t)(sizeof identity_value);
    identity_value.struct_version = UINT32_C(1);
    {
        jbyte identityBytes_values[64];
        (*environment)->GetByteArrayRegion(environment, identityBytes, 0, 64, identityBytes_values);
        identity_value.key[0] = (char)identityBytes_values[0];
        identity_value.key[1] = (char)identityBytes_values[1];
        identity_value.key[2] = (char)identityBytes_values[2];
        identity_value.key[3] = (char)identityBytes_values[3];
        identity_value.key[4] = (char)identityBytes_values[4];
        identity_value.key[5] = (char)identityBytes_values[5];
        identity_value.key[6] = (char)identityBytes_values[6];
        identity_value.key[7] = (char)identityBytes_values[7];
        identity_value.key[8] = (char)identityBytes_values[8];
        identity_value.key[9] = (char)identityBytes_values[9];
        identity_value.key[10] = (char)identityBytes_values[10];
        identity_value.key[11] = (char)identityBytes_values[11];
        identity_value.key[12] = (char)identityBytes_values[12];
        identity_value.key[13] = (char)identityBytes_values[13];
        identity_value.key[14] = (char)identityBytes_values[14];
        identity_value.key[15] = (char)identityBytes_values[15];
        identity_value.key[16] = (char)identityBytes_values[16];
        identity_value.key[17] = (char)identityBytes_values[17];
        identity_value.key[18] = (char)identityBytes_values[18];
        identity_value.key[19] = (char)identityBytes_values[19];
        identity_value.key[20] = (char)identityBytes_values[20];
        identity_value.key[21] = (char)identityBytes_values[21];
        identity_value.key[22] = (char)identityBytes_values[22];
        identity_value.key[23] = (char)identityBytes_values[23];
        identity_value.key[24] = (char)identityBytes_values[24];
        identity_value.key[25] = (char)identityBytes_values[25];
        identity_value.key[26] = (char)identityBytes_values[26];
        identity_value.key[27] = (char)identityBytes_values[27];
        identity_value.key[28] = (char)identityBytes_values[28];
        identity_value.key[29] = (char)identityBytes_values[29];
        identity_value.key[30] = (char)identityBytes_values[30];
        identity_value.key[31] = (char)identityBytes_values[31];
        identity_value.key[32] = (char)identityBytes_values[32];
        identity_value.key[33] = (char)identityBytes_values[33];
        identity_value.key[34] = (char)identityBytes_values[34];
        identity_value.key[35] = (char)identityBytes_values[35];
        identity_value.key[36] = (char)identityBytes_values[36];
        identity_value.key[37] = (char)identityBytes_values[37];
        identity_value.key[38] = (char)identityBytes_values[38];
        identity_value.key[39] = (char)identityBytes_values[39];
        identity_value.key[40] = (char)identityBytes_values[40];
        identity_value.key[41] = (char)identityBytes_values[41];
        identity_value.key[42] = (char)identityBytes_values[42];
        identity_value.key[43] = (char)identityBytes_values[43];
        identity_value.key[44] = (char)identityBytes_values[44];
        identity_value.key[45] = (char)identityBytes_values[45];
        identity_value.key[46] = (char)identityBytes_values[46];
        identity_value.key[47] = (char)identityBytes_values[47];
        identity_value.key[48] = (char)identityBytes_values[48];
        identity_value.key[49] = (char)identityBytes_values[49];
        identity_value.key[50] = (char)identityBytes_values[50];
        identity_value.key[51] = (char)identityBytes_values[51];
        identity_value.key[52] = (char)identityBytes_values[52];
        identity_value.key[53] = (char)identityBytes_values[53];
        identity_value.key[54] = (char)identityBytes_values[54];
        identity_value.key[55] = (char)identityBytes_values[55];
        identity_value.key[56] = (char)identityBytes_values[56];
        identity_value.key[57] = (char)identityBytes_values[57];
        identity_value.key[58] = (char)identityBytes_values[58];
        identity_value.key[59] = (char)identityBytes_values[59];
        identity_value.key[60] = (char)identityBytes_values[60];
        identity_value.key[61] = (char)identityBytes_values[61];
        identity_value.key[62] = (char)identityBytes_values[62];
        identity_value.key[63] = (char)identityBytes_values[63];
    }
    {
        jlong identityIntegral_values[1];
        (*environment)->GetLongArrayRegion(environment, identityIntegral, 0, 1, identityIntegral_values);
        identity_value.game_mode = (int32_t)identityIntegral_values[0];
    }
    jsize gamers_size = (*environment)->GetArrayLength(environment, gamers);
    jlong* gamers_elements = (*environment)->GetLongArrayElements(environment, gamers, NULL);
    if (gamers_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GamerHandle* gamers_values = (CNA_GamerHandle*)malloc(
        ((size_t)gamers_size + 1U) * sizeof(*gamers_values));
    if (gamers_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, gamers, gamers_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < gamers_size; ++index) {
        gamers_values[index] = (CNA_GamerHandle)gamers_elements[index];
    }
    CNA_LeaderboardReaderHandle out_reader_value = 0;
    CNA_Result call_result = cna.leaderboard_reader_read_from_gamers(&identity_value, gamers_values, (uint64_t)gamers_size, (CNA_GamerHandle)pivot_gamer, (int32_t)page_size, &out_reader_value);
    free(gamers_values);
    (*environment)->ReleaseLongArrayElements(
        environment, gamers, gamers_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reader_element = (jlong)out_reader_value;
        (*environment)->SetLongArrayRegion(
            environment, out_reader, 0, 1, &out_reader_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_leaderboardReaderReadFromPivot(
    JNIEnv* environment, jclass declaring_class, jbyteArray identityBytes, jlongArray identityIntegral, jlong pivot_gamer, jint page_size, jlongArray out_reader)
{
    (void)environment;
    (void)declaring_class;
    if (identityBytes == NULL || identityIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_LeaderboardIdentity identity_value;
    memset(&identity_value, 0, sizeof identity_value);
    identity_value.struct_size = (uint32_t)(sizeof identity_value);
    identity_value.struct_version = UINT32_C(1);
    {
        jbyte identityBytes_values[64];
        (*environment)->GetByteArrayRegion(environment, identityBytes, 0, 64, identityBytes_values);
        identity_value.key[0] = (char)identityBytes_values[0];
        identity_value.key[1] = (char)identityBytes_values[1];
        identity_value.key[2] = (char)identityBytes_values[2];
        identity_value.key[3] = (char)identityBytes_values[3];
        identity_value.key[4] = (char)identityBytes_values[4];
        identity_value.key[5] = (char)identityBytes_values[5];
        identity_value.key[6] = (char)identityBytes_values[6];
        identity_value.key[7] = (char)identityBytes_values[7];
        identity_value.key[8] = (char)identityBytes_values[8];
        identity_value.key[9] = (char)identityBytes_values[9];
        identity_value.key[10] = (char)identityBytes_values[10];
        identity_value.key[11] = (char)identityBytes_values[11];
        identity_value.key[12] = (char)identityBytes_values[12];
        identity_value.key[13] = (char)identityBytes_values[13];
        identity_value.key[14] = (char)identityBytes_values[14];
        identity_value.key[15] = (char)identityBytes_values[15];
        identity_value.key[16] = (char)identityBytes_values[16];
        identity_value.key[17] = (char)identityBytes_values[17];
        identity_value.key[18] = (char)identityBytes_values[18];
        identity_value.key[19] = (char)identityBytes_values[19];
        identity_value.key[20] = (char)identityBytes_values[20];
        identity_value.key[21] = (char)identityBytes_values[21];
        identity_value.key[22] = (char)identityBytes_values[22];
        identity_value.key[23] = (char)identityBytes_values[23];
        identity_value.key[24] = (char)identityBytes_values[24];
        identity_value.key[25] = (char)identityBytes_values[25];
        identity_value.key[26] = (char)identityBytes_values[26];
        identity_value.key[27] = (char)identityBytes_values[27];
        identity_value.key[28] = (char)identityBytes_values[28];
        identity_value.key[29] = (char)identityBytes_values[29];
        identity_value.key[30] = (char)identityBytes_values[30];
        identity_value.key[31] = (char)identityBytes_values[31];
        identity_value.key[32] = (char)identityBytes_values[32];
        identity_value.key[33] = (char)identityBytes_values[33];
        identity_value.key[34] = (char)identityBytes_values[34];
        identity_value.key[35] = (char)identityBytes_values[35];
        identity_value.key[36] = (char)identityBytes_values[36];
        identity_value.key[37] = (char)identityBytes_values[37];
        identity_value.key[38] = (char)identityBytes_values[38];
        identity_value.key[39] = (char)identityBytes_values[39];
        identity_value.key[40] = (char)identityBytes_values[40];
        identity_value.key[41] = (char)identityBytes_values[41];
        identity_value.key[42] = (char)identityBytes_values[42];
        identity_value.key[43] = (char)identityBytes_values[43];
        identity_value.key[44] = (char)identityBytes_values[44];
        identity_value.key[45] = (char)identityBytes_values[45];
        identity_value.key[46] = (char)identityBytes_values[46];
        identity_value.key[47] = (char)identityBytes_values[47];
        identity_value.key[48] = (char)identityBytes_values[48];
        identity_value.key[49] = (char)identityBytes_values[49];
        identity_value.key[50] = (char)identityBytes_values[50];
        identity_value.key[51] = (char)identityBytes_values[51];
        identity_value.key[52] = (char)identityBytes_values[52];
        identity_value.key[53] = (char)identityBytes_values[53];
        identity_value.key[54] = (char)identityBytes_values[54];
        identity_value.key[55] = (char)identityBytes_values[55];
        identity_value.key[56] = (char)identityBytes_values[56];
        identity_value.key[57] = (char)identityBytes_values[57];
        identity_value.key[58] = (char)identityBytes_values[58];
        identity_value.key[59] = (char)identityBytes_values[59];
        identity_value.key[60] = (char)identityBytes_values[60];
        identity_value.key[61] = (char)identityBytes_values[61];
        identity_value.key[62] = (char)identityBytes_values[62];
        identity_value.key[63] = (char)identityBytes_values[63];
    }
    {
        jlong identityIntegral_values[1];
        (*environment)->GetLongArrayRegion(environment, identityIntegral, 0, 1, identityIntegral_values);
        identity_value.game_mode = (int32_t)identityIntegral_values[0];
    }
    CNA_LeaderboardReaderHandle out_reader_value = 0;
    CNA_Result call_result = cna.leaderboard_reader_read_from_pivot(&identity_value, (CNA_GamerHandle)pivot_gamer, (int32_t)page_size, &out_reader_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reader_element = (jlong)out_reader_value;
        (*environment)->SetLongArrayRegion(
            environment, out_reader, 0, 1, &out_reader_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryClear(
    JNIEnv* environment, jclass declaring_class, jlong dictionary)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.property_dictionary_clear((CNA_PropertyDictionaryHandle)dictionary);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryContainsKey(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbooleanArray out_contains)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Bool out_contains_value = 0;
    CNA_Result call_result = cna.property_dictionary_contains_key((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_contains_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_contains_element = (jboolean)out_contains_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_contains, 0, 1, &out_contains_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryCopyKeyAt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jint index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.property_dictionary_copy_key_at((CNA_PropertyDictionaryHandle)dictionary, (int32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryCopyString(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.property_dictionary_copy_string((CNA_PropertyDictionaryHandle)dictionary, key_view, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetCount(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_count((CNA_PropertyDictionaryHandle)dictionary, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetDateTimeTicks(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_date_time_ticks((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_ticks_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetDouble(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jdoubleArray out_value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    double out_value_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_double((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_value_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_value_element = (jdouble)out_value_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetInt32(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_int32((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_value_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetInt64(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    int64_t out_value_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_int64((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_value_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_element = (jlong)out_value_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetKeySizeAt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_key_size_at((CNA_PropertyDictionaryHandle)dictionary, (int32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetOutcome(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jintArray out_outcome)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_LeaderboardOutcome out_outcome_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_outcome((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_outcome_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_outcome_element = (jint)out_outcome_value;
        (*environment)->SetIntArrayRegion(
            environment, out_outcome, 0, 1, &out_outcome_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetSingle(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    float out_value_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_single((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_value_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetStreamSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbooleanArray out_has_stream, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Bool out_has_stream_value = 0;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_stream_size_ext((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_has_stream_value, &out_bytes_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_stream_element = (jboolean)out_has_stream_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_stream, 0, 1, &out_has_stream_element);
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetStringSize(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_string_size((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_bytes_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryGetTimeSpanTicks(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.property_dictionary_get_time_span_ticks((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_ticks_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryRemove(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbooleanArray out_removed)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Bool out_removed_value = 0;
    CNA_Result call_result = cna.property_dictionary_remove((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_removed_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_removed_element = (jboolean)out_removed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_removed, 0, 1, &out_removed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetDateTimeTicks(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_date_time_ticks((CNA_PropertyDictionaryHandle)dictionary, key_view, (int64_t)ticks);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetDouble(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jdouble value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_double((CNA_PropertyDictionaryHandle)dictionary, key_view, (double)value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetInt32(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jint value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_int32((CNA_PropertyDictionaryHandle)dictionary, key_view, (int32_t)value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetInt64(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlong value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_int64((CNA_PropertyDictionaryHandle)dictionary, key_view, (int64_t)value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetOutcome(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jint outcome)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_outcome((CNA_PropertyDictionaryHandle)dictionary, key_view, (CNA_LeaderboardOutcome)outcome);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetSingle(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_single((CNA_PropertyDictionaryHandle)dictionary, key_view, (float)value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetString(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbyteArray value)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize value_size = (*environment)->GetArrayLength(environment, value);
    jbyte* value_bytes = (*environment)->GetByteArrayElements(environment, value, NULL);
    if (value_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView value_view = {(const char*)value_bytes, (uint64_t)value_size};
    CNA_Result call_result = cna.property_dictionary_set_string((CNA_PropertyDictionaryHandle)dictionary, key_view, value_view);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, value, value_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionarySetTimeSpanTicks(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Result call_result = cna.property_dictionary_set_time_span_ticks((CNA_PropertyDictionaryHandle)dictionary, key_view, (int64_t)ticks);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_propertyDictionaryTryGetValueKindExt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbooleanArray out_found, jintArray out_kind)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Bool out_found_value = 0;
    CNA_PropertyValueKind out_kind_value = 0;
    CNA_Result call_result = cna.property_dictionary_try_get_value_kind_ext((CNA_PropertyDictionaryHandle)dictionary, key_view, &out_found_value, &out_kind_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_found_element = (jboolean)out_found_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_found, 0, 1, &out_found_element);
        jint out_kind_element = (jint)out_kind_value;
        (*environment)->SetIntArrayRegion(
            environment, out_kind, 0, 1, &out_kind_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerAwardAchievement(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray achievement_key)
{
    (void)environment;
    (void)declaring_class;
    jsize achievement_key_size = (*environment)->GetArrayLength(environment, achievement_key);
    jbyte* achievement_key_bytes = (*environment)->GetByteArrayElements(environment, achievement_key, NULL);
    if (achievement_key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView achievement_key_view = {(const char*)achievement_key_bytes, (uint64_t)achievement_key_size};
    CNA_Result call_result = cna.signed_in_gamer_award_achievement((CNA_SignedInGamerHandle)gamer, achievement_key_view);
    (*environment)->ReleaseByteArrayElements(environment, achievement_key, achievement_key_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetAchievements(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_achievements)
{
    (void)environment;
    (void)declaring_class;
    CNA_AchievementCollectionHandle out_achievements_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_achievements((CNA_SignedInGamerHandle)gamer, &out_achievements_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_achievements_element = (jlong)out_achievements_value;
        (*environment)->SetLongArrayRegion(
            environment, out_achievements, 0, 1, &out_achievements_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetFriends(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_friends)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamerCollectionHandle out_friends_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_friends((CNA_SignedInGamerHandle)gamer, &out_friends_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_friends_element = (jlong)out_friends_value;
        (*environment)->SetLongArrayRegion(
            environment, out_friends, 0, 1, &out_friends_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetGameDefaults(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray out_defaultsBytes, jlongArray out_defaultsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_defaultsBytes == NULL || out_defaultsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GameDefaults out_defaults_value;
    memset(&out_defaults_value, 0, sizeof out_defaults_value);
    out_defaults_value.struct_size = (uint32_t)(sizeof out_defaults_value);
    out_defaults_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.signed_in_gamer_get_game_defaults((CNA_SignedInGamerHandle)gamer, &out_defaults_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_defaultsBytes_values[] = {
            (jbyte)out_defaults_value.reserved[0],
            (jbyte)out_defaults_value.reserved[1],
            (jbyte)out_defaults_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_defaultsBytes, 0,
            (jsize)(sizeof out_defaultsBytes_values / sizeof out_defaultsBytes_values[0]),
            out_defaultsBytes_values);
        jlong out_defaultsIntegral_values[] = {
            (jlong)out_defaults_value.game_difficulty,
            (jlong)out_defaults_value.controller_sensitivity,
            (jlong)out_defaults_value.racing_camera_angle,
            (jlong)out_defaults_value.has_primary_color,
            (jlong)out_defaults_value.has_secondary_color,
            (jlong)out_defaults_value.auto_aim,
            (jlong)out_defaults_value.auto_center,
            (jlong)out_defaults_value.move_with_right_thumb_stick,
            (jlong)out_defaults_value.invert_y_axis,
            (jlong)out_defaults_value.manual_transmission,
            (jlong)out_defaults_value.accelerate_with_buttons,
            (jlong)out_defaults_value.brake_with_buttons,
            (jlong)out_defaults_value.primary_color.r,
            (jlong)out_defaults_value.primary_color.g,
            (jlong)out_defaults_value.primary_color.b,
            (jlong)out_defaults_value.primary_color.a,
            (jlong)out_defaults_value.secondary_color.r,
            (jlong)out_defaults_value.secondary_color.g,
            (jlong)out_defaults_value.secondary_color.b,
            (jlong)out_defaults_value.secondary_color.a
        };
        (*environment)->SetLongArrayRegion(environment, out_defaultsIntegral, 0,
            (jsize)(sizeof out_defaultsIntegral_values / sizeof out_defaultsIntegral_values[0]),
            out_defaultsIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetIsGuest(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_is_guest)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_guest_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_is_guest((CNA_SignedInGamerHandle)gamer, &out_is_guest_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_guest_element = (jboolean)out_is_guest_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_guest, 0, 1, &out_is_guest_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetIsSignedInToLive(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_is_signed_in_to_live)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_signed_in_to_live_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_is_signed_in_to_live((CNA_SignedInGamerHandle)gamer, &out_is_signed_in_to_live_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_signed_in_to_live_element = (jboolean)out_is_signed_in_to_live_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_signed_in_to_live, 0, 1, &out_is_signed_in_to_live_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetPartySize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jintArray out_party_size)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_party_size_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_party_size((CNA_SignedInGamerHandle)gamer, &out_party_size_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_party_size_element = (jint)out_party_size_value;
        (*environment)->SetIntArrayRegion(
            environment, out_party_size, 0, 1, &out_party_size_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetPlayerIndex(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jintArray out_player_index)
{
    (void)environment;
    (void)declaring_class;
    CNA_PlayerIndex out_player_index_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_get_player_index((CNA_SignedInGamerHandle)gamer, &out_player_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_player_index_element = (jint)out_player_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_player_index, 0, 1, &out_player_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetPresence(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_presenceIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_presenceIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GamerPresence out_presence_value;
    memset(&out_presence_value, 0, sizeof out_presence_value);
    out_presence_value.struct_size = (uint32_t)(sizeof out_presence_value);
    out_presence_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.signed_in_gamer_get_presence((CNA_SignedInGamerHandle)gamer, &out_presence_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_presenceIntegral_values[] = {
            (jlong)out_presence_value.presence_mode,
            (jlong)out_presence_value.presence_value
        };
        (*environment)->SetLongArrayRegion(environment, out_presenceIntegral, 0,
            (jsize)(sizeof out_presenceIntegral_values / sizeof out_presenceIntegral_values[0]),
            out_presenceIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerGetPrivileges(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray out_privilegesBytes, jlongArray out_privilegesIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_privilegesBytes == NULL || out_privilegesIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GamerPrivileges out_privileges_value;
    memset(&out_privileges_value, 0, sizeof out_privileges_value);
    out_privileges_value.struct_size = (uint32_t)(sizeof out_privileges_value);
    out_privileges_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.signed_in_gamer_get_privileges((CNA_SignedInGamerHandle)gamer, &out_privileges_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_privilegesBytes_values[] = {
            (jbyte)out_privileges_value.reserved[0],
            (jbyte)out_privileges_value.reserved[1],
            (jbyte)out_privileges_value.reserved[2],
            (jbyte)out_privileges_value.reserved[3]
        };
        (*environment)->SetByteArrayRegion(environment, out_privilegesBytes, 0,
            (jsize)(sizeof out_privilegesBytes_values / sizeof out_privilegesBytes_values[0]),
            out_privilegesBytes_values);
        jlong out_privilegesIntegral_values[] = {
            (jlong)out_privileges_value.allow_communication,
            (jlong)out_privileges_value.allow_profile_viewing,
            (jlong)out_privileges_value.allow_user_created_content,
            (jlong)out_privileges_value.allow_online_sessions,
            (jlong)out_privileges_value.allow_premium_content,
            (jlong)out_privileges_value.allow_purchase_content,
            (jlong)out_privileges_value.allow_trade_content
        };
        (*environment)->SetLongArrayRegion(environment, out_privilegesIntegral, 0,
            (jsize)(sizeof out_privilegesIntegral_values / sizeof out_privilegesIntegral_values[0]),
            out_privilegesIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerIsFriend(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong other, jbooleanArray out_is_friend)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_friend_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_is_friend((CNA_SignedInGamerHandle)gamer, (CNA_GamerHandle)other, &out_is_friend_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_friend_element = (jboolean)out_is_friend_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_friend, 0, 1, &out_is_friend_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerIsHeadset(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong microphone_index, jbooleanArray out_is_headset)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_headset_value = 0;
    CNA_Result call_result = cna.signed_in_gamer_is_headset((CNA_SignedInGamerHandle)gamer, (uint64_t)microphone_index, &out_is_headset_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_headset_element = (jboolean)out_is_headset_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_headset, 0, 1, &out_is_headset_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerSetPartySize(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jint party_size)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.signed_in_gamer_set_party_size((CNA_SignedInGamerHandle)gamer, (int32_t)party_size);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerSetPresence(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray presenceIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (presenceIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GamerPresence presence_value;
    memset(&presence_value, 0, sizeof presence_value);
    presence_value.struct_size = (uint32_t)(sizeof presence_value);
    presence_value.struct_version = UINT32_C(1);
    {
        jlong presenceIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, presenceIntegral, 0, 2, presenceIntegral_values);
        presence_value.presence_mode = (CNA_GamerPresenceMode)presenceIntegral_values[0];
        presence_value.presence_value = (int32_t)presenceIntegral_values[1];
    }
    CNA_Result call_result = cna.signed_in_gamer_set_presence((CNA_SignedInGamerHandle)gamer, &presence_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGamerServicesRoutes_signedInGamerSetPresenceModeStringExt(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray mode)
{
    (void)environment;
    (void)declaring_class;
    jsize mode_size = (*environment)->GetArrayLength(environment, mode);
    jbyte* mode_bytes = (*environment)->GetByteArrayElements(environment, mode, NULL);
    if (mode_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView mode_view = {(const char*)mode_bytes, (uint64_t)mode_size};
    CNA_Result call_result = cna.signed_in_gamer_set_presence_mode_string_ext((CNA_SignedInGamerHandle)gamer, mode_view);
    (*environment)->ReleaseByteArrayElements(environment, mode, mode_bytes, JNI_ABORT);
    return (jint)call_result;
}
