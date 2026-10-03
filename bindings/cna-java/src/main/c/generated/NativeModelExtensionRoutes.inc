/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeModelExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerCopyBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong player, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.animation_player_copy_bone_transforms((CNA_AnimationPlayerHandle)player, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerCopyCurrentClipName(
    JNIEnv* environment, jclass declaring_class, jlong player, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.animation_player_copy_current_clip_name((CNA_AnimationPlayerHandle)player, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerCopySkinTransforms(
    JNIEnv* environment, jclass declaring_class, jlong player, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.animation_player_copy_skin_transforms((CNA_AnimationPlayerHandle)player, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerCopyWorldTransforms(
    JNIEnv* environment, jclass declaring_class, jlong player, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.animation_player_copy_world_transforms((CNA_AnimationPlayerHandle)player, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerCreate(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_player)
{
    (void)environment;
    (void)declaring_class;
    CNA_AnimationPlayerHandle out_player_value = 0;
    CNA_Result call_result = cna.animation_player_create((CNA_SkinningDataHandle)data, &out_player_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_player_element = (jlong)out_player_value;
        (*environment)->SetLongArrayRegion(
            environment, out_player, 0, 1, &out_player_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerDestroy(
    JNIEnv* environment, jclass declaring_class, jlong player)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.animation_player_destroy((CNA_AnimationPlayerHandle)player);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerGetCurrentClipInfo(
    JNIEnv* environment, jclass declaring_class, jlong player, jbooleanArray out_has_clip, jdoubleArray out_duration_seconds, jlongArray out_track_count)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_clip_value = 0;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_Result call_result = cna.animation_player_get_current_clip_info((CNA_AnimationPlayerHandle)player, &out_has_clip_value, &out_duration_seconds_value, &out_track_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_clip_element = (jboolean)out_has_clip_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_clip, 0, 1, &out_has_clip_element);
        jdouble out_duration_seconds_element = (jdouble)out_duration_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_duration_seconds, 0, 1, &out_duration_seconds_element);
        jlong out_track_count_element = (jlong)out_track_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_track_count, 0, 1, &out_track_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerGetCurrentClipNameByteCount(
    JNIEnv* environment, jclass declaring_class, jlong player, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.animation_player_get_current_clip_name_byte_count((CNA_AnimationPlayerHandle)player, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerGetCurrentPosition(
    JNIEnv* environment, jclass declaring_class, jlong player, jdoubleArray out_position_seconds)
{
    (void)environment;
    (void)declaring_class;
    double out_position_seconds_value = 0;
    CNA_Result call_result = cna.animation_player_get_current_position((CNA_AnimationPlayerHandle)player, &out_position_seconds_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_position_seconds_element = (jdouble)out_position_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_position_seconds, 0, 1, &out_position_seconds_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerStartClip(
    JNIEnv* environment, jclass declaring_class, jlong player, jbyteArray clip_name)
{
    (void)environment;
    (void)declaring_class;
    jsize clip_name_size = (*environment)->GetArrayLength(environment, clip_name);
    jbyte* clip_name_bytes = (*environment)->GetByteArrayElements(environment, clip_name, NULL);
    if (clip_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView clip_name_view = {(const char*)clip_name_bytes, (uint64_t)clip_name_size};
    CNA_Result call_result = cna.animation_player_start_clip((CNA_AnimationPlayerHandle)player, clip_name_view);
    (*environment)->ReleaseByteArrayElements(environment, clip_name, clip_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_animationPlayerUpdate(
    JNIEnv* environment, jclass declaring_class, jlong player, jdouble time_seconds, jboolean relative_to_current_time, jboolean loop)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.animation_player_update((CNA_AnimationPlayerHandle)player, (double)time_seconds, (CNA_Bool)relative_to_current_time, (CNA_Bool)loop);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_contentManagerLoadModel(
    JNIEnv* environment, jclass declaring_class, jlong content_manager, jbyteArray asset_name, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    jsize asset_name_size = (*environment)->GetArrayLength(environment, asset_name);
    jbyte* asset_name_bytes = (*environment)->GetByteArrayElements(environment, asset_name, NULL);
    if (asset_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView asset_name_view = {(const char*)asset_name_bytes, (uint64_t)asset_name_size};
    CNA_ModelHandle out_model_value = 0;
    CNA_Result call_result = cna.content_manager_load_model((CNA_Handle)content_manager, asset_name_view, &out_model_value);
    (*environment)->ReleaseByteArrayElements(environment, asset_name, asset_name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_effectCopyTypeName(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.effect_copy_type_name((CNA_EffectHandle)effect, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_effectGetTypeNameByteCount(
    JNIEnv* environment, jclass declaring_class, jlong effect, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.effect_get_type_name_byte_count((CNA_EffectHandle)effect, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_matrixCreateInfinitePerspectiveFieldOfViewExt(
    JNIEnv* environment, jclass declaring_class, jfloat field_of_view, jfloat aspect_ratio, jfloat near_plane_distance, jfloatArray out_matrixFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_matrixFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_matrix_value;
    memset(&out_matrix_value, 0, sizeof out_matrix_value);
    CNA_Result call_result = cna.matrix_create_infinite_perspective_field_of_view_ext((float)field_of_view, (float)aspect_ratio, (float)near_plane_distance, &out_matrix_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_matrixFloating_values[] = {
            (jfloat)out_matrix_value.m11,
            (jfloat)out_matrix_value.m12,
            (jfloat)out_matrix_value.m13,
            (jfloat)out_matrix_value.m14,
            (jfloat)out_matrix_value.m21,
            (jfloat)out_matrix_value.m22,
            (jfloat)out_matrix_value.m23,
            (jfloat)out_matrix_value.m24,
            (jfloat)out_matrix_value.m31,
            (jfloat)out_matrix_value.m32,
            (jfloat)out_matrix_value.m33,
            (jfloat)out_matrix_value.m34,
            (jfloat)out_matrix_value.m41,
            (jfloat)out_matrix_value.m42,
            (jfloat)out_matrix_value.m43,
            (jfloat)out_matrix_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_matrixFloating, 0,
            (jsize)(sizeof out_matrixFloating_values / sizeof out_matrixFloating_values[0]),
            out_matrixFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAddCameraExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray descriptorIntegral, jfloatArray descriptorFloating, jbyteArray descriptorName)
{
    (void)environment;
    (void)declaring_class;
    if (descriptorIntegral == NULL || descriptorFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_ModelCameraDescriptorEXT descriptor_value;
    memset(&descriptor_value, 0, sizeof descriptor_value);
    descriptor_value.camera.struct_size = (uint32_t)(sizeof descriptor_value.camera);
    descriptor_value.camera.struct_version = UINT32_C(1);
    {
        jlong descriptorIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, descriptorIntegral, 0, 4, descriptorIntegral_values);
        descriptor_value.camera.scene_node_index = (int32_t)descriptorIntegral_values[0];
        descriptor_value.camera.is_perspective = (CNA_Bool)descriptorIntegral_values[1];
        descriptor_value.camera.has_infinite_far_plane = (CNA_Bool)descriptorIntegral_values[2];
        descriptor_value.camera.has_authored_aspect_ratio = (CNA_Bool)descriptorIntegral_values[3];
    }
    {
        jfloat descriptorFloating_values[36];
        (*environment)->GetFloatArrayRegion(environment, descriptorFloating, 0, 36, descriptorFloating_values);
        descriptor_value.camera.projection.m11 = (float)descriptorFloating_values[0];
        descriptor_value.camera.projection.m12 = (float)descriptorFloating_values[1];
        descriptor_value.camera.projection.m13 = (float)descriptorFloating_values[2];
        descriptor_value.camera.projection.m14 = (float)descriptorFloating_values[3];
        descriptor_value.camera.projection.m21 = (float)descriptorFloating_values[4];
        descriptor_value.camera.projection.m22 = (float)descriptorFloating_values[5];
        descriptor_value.camera.projection.m23 = (float)descriptorFloating_values[6];
        descriptor_value.camera.projection.m24 = (float)descriptorFloating_values[7];
        descriptor_value.camera.projection.m31 = (float)descriptorFloating_values[8];
        descriptor_value.camera.projection.m32 = (float)descriptorFloating_values[9];
        descriptor_value.camera.projection.m33 = (float)descriptorFloating_values[10];
        descriptor_value.camera.projection.m34 = (float)descriptorFloating_values[11];
        descriptor_value.camera.projection.m41 = (float)descriptorFloating_values[12];
        descriptor_value.camera.projection.m42 = (float)descriptorFloating_values[13];
        descriptor_value.camera.projection.m43 = (float)descriptorFloating_values[14];
        descriptor_value.camera.projection.m44 = (float)descriptorFloating_values[15];
        descriptor_value.camera.world_transform.m11 = (float)descriptorFloating_values[16];
        descriptor_value.camera.world_transform.m12 = (float)descriptorFloating_values[17];
        descriptor_value.camera.world_transform.m13 = (float)descriptorFloating_values[18];
        descriptor_value.camera.world_transform.m14 = (float)descriptorFloating_values[19];
        descriptor_value.camera.world_transform.m21 = (float)descriptorFloating_values[20];
        descriptor_value.camera.world_transform.m22 = (float)descriptorFloating_values[21];
        descriptor_value.camera.world_transform.m23 = (float)descriptorFloating_values[22];
        descriptor_value.camera.world_transform.m24 = (float)descriptorFloating_values[23];
        descriptor_value.camera.world_transform.m31 = (float)descriptorFloating_values[24];
        descriptor_value.camera.world_transform.m32 = (float)descriptorFloating_values[25];
        descriptor_value.camera.world_transform.m33 = (float)descriptorFloating_values[26];
        descriptor_value.camera.world_transform.m34 = (float)descriptorFloating_values[27];
        descriptor_value.camera.world_transform.m41 = (float)descriptorFloating_values[28];
        descriptor_value.camera.world_transform.m42 = (float)descriptorFloating_values[29];
        descriptor_value.camera.world_transform.m43 = (float)descriptorFloating_values[30];
        descriptor_value.camera.world_transform.m44 = (float)descriptorFloating_values[31];
        descriptor_value.camera.aspect_ratio = (float)descriptorFloating_values[32];
        descriptor_value.camera.field_of_view = (float)descriptorFloating_values[33];
        descriptor_value.camera.near_plane_distance = (float)descriptorFloating_values[34];
        descriptor_value.camera.far_plane_distance = (float)descriptorFloating_values[35];
    }
    jsize descriptorName_size = (*environment)->GetArrayLength(environment, descriptorName);
    jbyte* descriptorName_bytes = (*environment)->GetByteArrayElements(environment, descriptorName, NULL);
    if (descriptorName_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    descriptor_value.name.data = (const char*)descriptorName_bytes;
    descriptor_value.name.byte_length = (uint64_t)descriptorName_size;
    CNA_Result call_result = cna.model_add_camera_ext((CNA_ModelHandle)model, &descriptor_value);
    (*environment)->ReleaseByteArrayElements(environment, descriptorName, descriptorName_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAddSkinExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jlong data, jlongArray mesh_indices)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize mesh_indices_size = (*environment)->GetArrayLength(environment, mesh_indices);
    jlong* mesh_indices_elements = (*environment)->GetLongArrayElements(environment, mesh_indices, NULL);
    if (mesh_indices_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t* mesh_indices_values = (uint64_t*)malloc(
        ((size_t)mesh_indices_size + 1U) * sizeof(*mesh_indices_values));
    if (mesh_indices_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, mesh_indices, mesh_indices_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < mesh_indices_size; ++index) {
        mesh_indices_values[index] = (uint64_t)mesh_indices_elements[index];
    }
    CNA_Result call_result = cna.model_add_skin_ext((CNA_ModelHandle)model, name_view, (CNA_SkinningDataHandle)data, mesh_indices_values, (uint64_t)mesh_indices_size);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(mesh_indices_values);
    (*environment)->ReleaseLongArrayElements(
        environment, mesh_indices, mesh_indices_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsCopyClipNameAt(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlong clip_index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_animations_ext_copy_clip_name_at((CNA_ModelAnimationsEXTHandle)animations, (uint64_t)clip_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsCopyTypeName(
    JNIEnv* environment, jclass declaring_class, jlong animations, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_animations_ext_copy_type_name((CNA_ModelAnimationsEXTHandle)animations, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsDestroy(
    JNIEnv* environment, jclass declaring_class, jlong animations)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_animations_ext_destroy((CNA_ModelAnimationsEXTHandle)animations);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsGetClipCount(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_animations_ext_get_clip_count((CNA_ModelAnimationsEXTHandle)animations, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsGetClipInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlong clip_index, jdoubleArray out_duration_seconds, jlongArray out_track_count, jintArray out_target_space)
{
    (void)environment;
    (void)declaring_class;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_ClipTargetSpaceEXT out_target_space_value = 0;
    CNA_Result call_result = cna.model_animations_ext_get_clip_info_at((CNA_ModelAnimationsEXTHandle)animations, (uint64_t)clip_index, &out_duration_seconds_value, &out_track_count_value, &out_target_space_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_duration_seconds_element = (jdouble)out_duration_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_duration_seconds, 0, 1, &out_duration_seconds_element);
        jlong out_track_count_element = (jlong)out_track_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_track_count, 0, 1, &out_track_count_element);
        jint out_target_space_element = (jint)out_target_space_value;
        (*environment)->SetIntArrayRegion(
            environment, out_target_space, 0, 1, &out_target_space_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsGetClipNameByteCountAt(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlong clip_index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_animations_ext_get_clip_name_byte_count_at((CNA_ModelAnimationsEXTHandle)animations, (uint64_t)clip_index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsGetTypeNameByteCount(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_animations_ext_get_type_name_byte_count((CNA_ModelAnimationsEXTHandle)animations, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelAnimationsSetClipTargetSpaceAt(
    JNIEnv* environment, jclass declaring_class, jlong animations, jlong clip_index, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_animations_ext_set_clip_target_space_at((CNA_ModelAnimationsEXTHandle)animations, (uint64_t)clip_index, (CNA_ClipTargetSpaceEXT)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelApplyBindPoseBoneTransformsExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong data, jlongArray out_posed_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_posed_count_value = 0;
    CNA_Result call_result = cna.model_apply_bind_pose_bone_transforms_ext((CNA_ModelHandle)model, (CNA_SkinningDataHandle)data, &out_posed_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_posed_count_element = (jlong)out_posed_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_posed_count, 0, 1, &out_posed_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelApplyClipToBonesExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong animations, jlong clip_index, jdouble time_seconds)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_apply_clip_to_bones_ext((CNA_ModelHandle)model, (CNA_ModelAnimationsEXTHandle)animations, (uint64_t)clip_index, (double)time_seconds);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneAddChild(
    JNIEnv* environment, jclass declaring_class, jlong bone, jlong child)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_bone_add_child((CNA_ModelBoneHandle)bone, (CNA_ModelBoneHandle)child);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_bone_collection_destroy((CNA_ModelBoneCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong index, jlongArray out_bone)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelBoneHandle out_bone_value = 0;
    CNA_Result call_result = cna.model_bone_collection_get_at((CNA_ModelBoneCollectionHandle)collection, (uint64_t)index, &out_bone_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bone_element = (jlong)out_bone_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bone, 0, 1, &out_bone_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_bone_collection_get_count((CNA_ModelBoneCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneCopyName(
    JNIEnv* environment, jclass declaring_class, jlong bone, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_bone_copy_name((CNA_ModelBoneHandle)bone, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneCreate(
    JNIEnv* environment, jclass declaring_class, jint index, jbyteArray name, jlongArray out_bone)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_ModelBoneHandle out_bone_value = 0;
    CNA_Result call_result = cna.model_bone_create((int32_t)index, name_view, &out_bone_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bone_element = (jlong)out_bone_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bone, 0, 1, &out_bone_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneDestroy(
    JNIEnv* environment, jclass declaring_class, jlong bone)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_bone_destroy((CNA_ModelBoneHandle)bone);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneGetChildren(
    JNIEnv* environment, jclass declaring_class, jlong bone, jlongArray out_children)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelBoneCollectionHandle out_children_value = 0;
    CNA_Result call_result = cna.model_bone_get_children((CNA_ModelBoneHandle)bone, &out_children_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_children_element = (jlong)out_children_value;
        (*environment)->SetLongArrayRegion(
            environment, out_children, 0, 1, &out_children_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneGetIndex(
    JNIEnv* environment, jclass declaring_class, jlong bone, jintArray out_index)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_index_value = 0;
    CNA_Result call_result = cna.model_bone_get_index((CNA_ModelBoneHandle)bone, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_index_element = (jint)out_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneGetNameByteCount(
    JNIEnv* environment, jclass declaring_class, jlong bone, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_bone_get_name_byte_count((CNA_ModelBoneHandle)bone, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneGetParent(
    JNIEnv* environment, jclass declaring_class, jlong bone, jbooleanArray out_has_parent, jlongArray out_parent)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_parent_value = 0;
    CNA_ModelBoneHandle out_parent_value = 0;
    CNA_Result call_result = cna.model_bone_get_parent((CNA_ModelBoneHandle)bone, &out_has_parent_value, &out_parent_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_parent_element = (jboolean)out_has_parent_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_parent, 0, 1, &out_has_parent_element);
        jlong out_parent_element = (jlong)out_parent_value;
        (*environment)->SetLongArrayRegion(
            environment, out_parent, 0, 1, &out_parent_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneGetTransform(
    JNIEnv* environment, jclass declaring_class, jlong bone, jfloatArray out_transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_transform_value;
    memset(&out_transform_value, 0, sizeof out_transform_value);
    CNA_Result call_result = cna.model_bone_get_transform((CNA_ModelBoneHandle)bone, &out_transform_value);
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
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelBoneSetTransform(
    JNIEnv* environment, jclass declaring_class, jlong bone, jfloatArray transformFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Matrix transform_value;
    memset(&transform_value, 0, sizeof transform_value);
    {
        jfloat transformFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, transformFloating, 0, 16, transformFloating_values);
        transform_value.m11 = (float)transformFloating_values[0];
        transform_value.m12 = (float)transformFloating_values[1];
        transform_value.m13 = (float)transformFloating_values[2];
        transform_value.m14 = (float)transformFloating_values[3];
        transform_value.m21 = (float)transformFloating_values[4];
        transform_value.m22 = (float)transformFloating_values[5];
        transform_value.m23 = (float)transformFloating_values[6];
        transform_value.m24 = (float)transformFloating_values[7];
        transform_value.m31 = (float)transformFloating_values[8];
        transform_value.m32 = (float)transformFloating_values[9];
        transform_value.m33 = (float)transformFloating_values[10];
        transform_value.m34 = (float)transformFloating_values[11];
        transform_value.m41 = (float)transformFloating_values[12];
        transform_value.m42 = (float)transformFloating_values[13];
        transform_value.m43 = (float)transformFloating_values[14];
        transform_value.m44 = (float)transformFloating_values[15];
    }
    CNA_Result call_result = cna.model_bone_set_transform((CNA_ModelBoneHandle)bone, transform_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelClearCamerasExt(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_clear_cameras_ext((CNA_ModelHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelClearSkinsExt(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_clear_skins_ext((CNA_ModelHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyAbsoluteBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_copy_absolute_bone_transforms((CNA_ModelHandle)model, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_copy_bone_transforms((CNA_ModelHandle)model, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyCameraNameExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_camera_name_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyGltfImportDiagnosticCodeExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_gltf_import_diagnostic_code_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyGltfImportDiagnosticDetailExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlong detail_index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_gltf_import_diagnostic_detail_ext((CNA_ModelHandle)model, (uint64_t)index, (uint64_t)detail_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyGltfImportDiagnosticMessageExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_gltf_import_diagnostic_message_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyGltfImportDiagnosticSubjectExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_gltf_import_diagnostic_subject_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopyMaterialVariantNameExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_material_variant_name_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCopySkinNameExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_copy_skin_name_ext((CNA_ModelHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCreateSkinSkeletonHandleExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_data)
{
    (void)environment;
    (void)declaring_class;
    CNA_SkinningDataHandle out_data_value = 0;
    CNA_Result call_result = cna.model_create_skin_skeleton_handle_ext((CNA_ModelHandle)model, (uint64_t)index, &out_data_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_data_element = (jlong)out_data_value;
        (*environment)->SetLongArrayRegion(
            environment, out_data, 0, 1, &out_data_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelCreateWithParents(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray bones, jlongArray meshes, jlongArray mesh_parents, jlong root_bone_index, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    jsize bones_size = (*environment)->GetArrayLength(environment, bones);
    jlong* bones_elements = (*environment)->GetLongArrayElements(environment, bones, NULL);
    if (bones_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_ModelBoneHandle* bones_values = (CNA_ModelBoneHandle*)malloc(
        ((size_t)bones_size + 1U) * sizeof(*bones_values));
    if (bones_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, bones, bones_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bones_size; ++index) {
        bones_values[index] = (CNA_ModelBoneHandle)bones_elements[index];
    }
    jsize meshes_size = (*environment)->GetArrayLength(environment, meshes);
    jlong* meshes_elements = (*environment)->GetLongArrayElements(environment, meshes, NULL);
    if (meshes_elements == NULL) {
    free(bones_values);
    (*environment)->ReleaseLongArrayElements(
        environment, bones, bones_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_ModelMeshHandle* meshes_values = (CNA_ModelMeshHandle*)malloc(
        ((size_t)meshes_size + 1U) * sizeof(*meshes_values));
    if (meshes_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, meshes, meshes_elements, JNI_ABORT);
    free(bones_values);
    (*environment)->ReleaseLongArrayElements(
        environment, bones, bones_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < meshes_size; ++index) {
        meshes_values[index] = (CNA_ModelMeshHandle)meshes_elements[index];
    }
    jsize mesh_parents_size = (*environment)->GetArrayLength(environment, mesh_parents);
    jlong* mesh_parents_elements = (*environment)->GetLongArrayElements(environment, mesh_parents, NULL);
    if (mesh_parents_elements == NULL) {
    free(meshes_values);
    (*environment)->ReleaseLongArrayElements(
        environment, meshes, meshes_elements, JNI_ABORT);
    free(bones_values);
    (*environment)->ReleaseLongArrayElements(
        environment, bones, bones_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_ModelBoneHandle* mesh_parents_values = (CNA_ModelBoneHandle*)malloc(
        ((size_t)mesh_parents_size + 1U) * sizeof(*mesh_parents_values));
    if (mesh_parents_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, mesh_parents, mesh_parents_elements, JNI_ABORT);
    free(meshes_values);
    (*environment)->ReleaseLongArrayElements(
        environment, meshes, meshes_elements, JNI_ABORT);
    free(bones_values);
    (*environment)->ReleaseLongArrayElements(
        environment, bones, bones_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < mesh_parents_size; ++index) {
        mesh_parents_values[index] = (CNA_ModelBoneHandle)mesh_parents_elements[index];
    }
    CNA_ModelHandle out_model_value = 0;
    CNA_Result call_result = cna.model_create_with_parents((CNA_Handle)graphics_device, bones_values, (uint64_t)bones_size, meshes_values, (uint64_t)meshes_size, mesh_parents_values, (uint64_t)mesh_parents_size, (uint64_t)root_bone_index, &out_model_value);
    free(bones_values);
    (*environment)->ReleaseLongArrayElements(
        environment, bones, bones_elements, JNI_ABORT);
    free(meshes_values);
    (*environment)->ReleaseLongArrayElements(
        environment, meshes, meshes_elements, JNI_ABORT);
    free(mesh_parents_values);
    (*environment)->ReleaseLongArrayElements(
        environment, mesh_parents, mesh_parents_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelDestroy(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_destroy((CNA_ModelHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelDraw(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray worldFloating, jfloatArray viewFloating, jfloatArray projectionFloating)
{
    (void)environment;
    (void)declaring_class;
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
    CNA_Result call_result = cna.model_draw((CNA_ModelHandle)model, world_value, view_value, projection_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelEffectCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_effect_collection_destroy((CNA_ModelEffectCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelEffectCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong index, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.model_effect_collection_get_at((CNA_ModelEffectCollectionHandle)collection, (uint64_t)index, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelEffectCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_effect_collection_get_count((CNA_ModelEffectCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetBoneTransformCount(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_get_bone_transform_count((CNA_ModelHandle)model, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetBones(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_bones)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelBoneCollectionHandle out_bones_value = 0;
    CNA_Result call_result = cna.model_get_bones((CNA_ModelHandle)model, &out_bones_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bones_element = (jlong)out_bones_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bones, 0, 1, &out_bones_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetBoundingSphereExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jbooleanArray out_has_value, jfloatArray out_sphereFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_sphereFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Bool out_has_value_value = 0;
    CNA_BoundingSphere out_sphere_value;
    memset(&out_sphere_value, 0, sizeof out_sphere_value);
    CNA_Result call_result = cna.model_get_bounding_sphere_ext((CNA_ModelHandle)model, &out_has_value_value, &out_sphere_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_value_element = (jboolean)out_has_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_value, 0, 1, &out_has_value_element);
        jfloat out_sphereFloating_values[] = {
            (jfloat)out_sphere_value.center.x,
            (jfloat)out_sphere_value.center.y,
            (jfloat)out_sphere_value.center.z,
            (jfloat)out_sphere_value.radius
        };
        (*environment)->SetFloatArrayRegion(environment, out_sphereFloating, 0,
            (jsize)(sizeof out_sphereFloating_values / sizeof out_sphereFloating_values[0]),
            out_sphereFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetCameraCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_get_camera_count_ext((CNA_ModelHandle)model, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetCameraExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_cameraIntegral, jfloatArray out_cameraFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_cameraIntegral == NULL || out_cameraFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_ModelCameraEXT out_camera_value;
    memset(&out_camera_value, 0, sizeof out_camera_value);
    out_camera_value.struct_size = (uint32_t)(sizeof out_camera_value);
    out_camera_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.model_get_camera_ext((CNA_ModelHandle)model, (uint64_t)index, &out_camera_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_cameraIntegral_values[] = {
            (jlong)out_camera_value.scene_node_index,
            (jlong)out_camera_value.is_perspective,
            (jlong)out_camera_value.has_infinite_far_plane,
            (jlong)out_camera_value.has_authored_aspect_ratio
        };
        (*environment)->SetLongArrayRegion(environment, out_cameraIntegral, 0,
            (jsize)(sizeof out_cameraIntegral_values / sizeof out_cameraIntegral_values[0]),
            out_cameraIntegral_values);
        jfloat out_cameraFloating_values[] = {
            (jfloat)out_camera_value.projection.m11,
            (jfloat)out_camera_value.projection.m12,
            (jfloat)out_camera_value.projection.m13,
            (jfloat)out_camera_value.projection.m14,
            (jfloat)out_camera_value.projection.m21,
            (jfloat)out_camera_value.projection.m22,
            (jfloat)out_camera_value.projection.m23,
            (jfloat)out_camera_value.projection.m24,
            (jfloat)out_camera_value.projection.m31,
            (jfloat)out_camera_value.projection.m32,
            (jfloat)out_camera_value.projection.m33,
            (jfloat)out_camera_value.projection.m34,
            (jfloat)out_camera_value.projection.m41,
            (jfloat)out_camera_value.projection.m42,
            (jfloat)out_camera_value.projection.m43,
            (jfloat)out_camera_value.projection.m44,
            (jfloat)out_camera_value.world_transform.m11,
            (jfloat)out_camera_value.world_transform.m12,
            (jfloat)out_camera_value.world_transform.m13,
            (jfloat)out_camera_value.world_transform.m14,
            (jfloat)out_camera_value.world_transform.m21,
            (jfloat)out_camera_value.world_transform.m22,
            (jfloat)out_camera_value.world_transform.m23,
            (jfloat)out_camera_value.world_transform.m24,
            (jfloat)out_camera_value.world_transform.m31,
            (jfloat)out_camera_value.world_transform.m32,
            (jfloat)out_camera_value.world_transform.m33,
            (jfloat)out_camera_value.world_transform.m34,
            (jfloat)out_camera_value.world_transform.m41,
            (jfloat)out_camera_value.world_transform.m42,
            (jfloat)out_camera_value.world_transform.m43,
            (jfloat)out_camera_value.world_transform.m44,
            (jfloat)out_camera_value.aspect_ratio,
            (jfloat)out_camera_value.field_of_view,
            (jfloat)out_camera_value.near_plane_distance,
            (jfloat)out_camera_value.far_plane_distance
        };
        (*environment)->SetFloatArrayRegion(environment, out_cameraFloating, 0,
            (jsize)(sizeof out_cameraFloating_values / sizeof out_cameraFloating_values[0]),
            out_cameraFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetCameraNameByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_camera_name_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetContentTagDictionaryExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jbooleanArray out_has_tag, jlongArray out_dictionary)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_tag_value = 0;
    CNA_ObjectDictionaryHandle out_dictionary_value = 0;
    CNA_Result call_result = cna.model_get_content_tag_dictionary_ext((CNA_ModelHandle)model, &out_has_tag_value, &out_dictionary_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_tag_element = (jboolean)out_has_tag_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_tag, 0, 1, &out_has_tag_element);
        jlong out_dictionary_element = (jlong)out_dictionary_value;
        (*environment)->SetLongArrayRegion(
            environment, out_dictionary, 0, 1, &out_dictionary_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportDiagnosticCodeByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_gltf_import_diagnostic_code_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportDiagnosticDetailByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlong detail_index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_gltf_import_diagnostic_detail_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, (uint64_t)detail_index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportDiagnosticExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_diagnosticIntegral, jdoubleArray out_diagnosticDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (out_diagnosticIntegral == NULL || out_diagnosticDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GltfImportDiagnosticEXT out_diagnostic_value;
    memset(&out_diagnostic_value, 0, sizeof out_diagnostic_value);
    out_diagnostic_value.struct_size = (uint32_t)(sizeof out_diagnostic_value);
    out_diagnostic_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.model_get_gltf_import_diagnostic_ext((CNA_ModelHandle)model, (uint64_t)index, &out_diagnostic_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_diagnosticIntegral_values[] = {
            (jlong)out_diagnostic_value.severity,
            (jlong)out_diagnostic_value.kind,
            (jlong)out_diagnostic_value.count,
            (jlong)out_diagnostic_value.detail_count
        };
        (*environment)->SetLongArrayRegion(environment, out_diagnosticIntegral, 0,
            (jsize)(sizeof out_diagnosticIntegral_values / sizeof out_diagnosticIntegral_values[0]),
            out_diagnosticIntegral_values);
        jdouble out_diagnosticDoubles_values[] = {
            (jdouble)out_diagnostic_value.worst_magnitude
        };
        (*environment)->SetDoubleArrayRegion(environment, out_diagnosticDoubles, 0,
            (jsize)(sizeof out_diagnosticDoubles_values / sizeof out_diagnosticDoubles_values[0]),
            out_diagnosticDoubles_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportDiagnosticMessageByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_gltf_import_diagnostic_message_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportDiagnosticSubjectByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_gltf_import_diagnostic_subject_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetGltfImportReportExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_reportIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_reportIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GltfImportReportEXT out_report_value;
    memset(&out_report_value, 0, sizeof out_report_value);
    out_report_value.struct_size = (uint32_t)(sizeof out_report_value);
    out_report_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.model_get_gltf_import_report_ext((CNA_ModelHandle)model, &out_report_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reportIntegral_values[] = {
            (jlong)out_report_value.node_count,
            (jlong)out_report_value.mesh_instance_count,
            (jlong)out_report_value.distinct_mesh_count,
            (jlong)out_report_value.shared_mesh_count,
            (jlong)out_report_value.max_node_depth,
            (jlong)out_report_value.camera_node_count,
            (jlong)out_report_value.light_node_count,
            (jlong)out_report_value.imported_light_count,
            (jlong)out_report_value.primitive_count,
            (jlong)out_report_value.skin_count,
            (jlong)out_report_value.animation_count,
            (jlong)out_report_value.clip_count,
            (jlong)out_report_value.diagnostic_count,
            (jlong)out_report_value.warning_count,
            (jlong)out_report_value.dropped_feature_count,
            (jlong)out_report_value.approximation_count,
            (jlong)out_report_value.anything_lost
        };
        (*environment)->SetLongArrayRegion(environment, out_reportIntegral, 0,
            (jsize)(sizeof out_reportIntegral_values / sizeof out_reportIntegral_values[0]),
            out_reportIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetMaterialVariantCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_get_material_variant_count_ext((CNA_ModelHandle)model, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetMaterialVariantExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.model_get_material_variant_ext((CNA_ModelHandle)model, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetMaterialVariantNameByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_material_variant_name_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetMeshes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_meshes)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshCollectionHandle out_meshes_value = 0;
    CNA_Result call_result = cna.model_get_meshes((CNA_ModelHandle)model, &out_meshes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_meshes_element = (jlong)out_meshes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_meshes, 0, 1, &out_meshes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetRoot(
    JNIEnv* environment, jclass declaring_class, jlong model, jbooleanArray out_has_root, jlongArray out_root)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_root_value = 0;
    CNA_ModelBoneHandle out_root_value = 0;
    CNA_Result call_result = cna.model_get_root((CNA_ModelHandle)model, &out_has_root_value, &out_root_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_root_element = (jboolean)out_has_root_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_root, 0, 1, &out_has_root_element);
        jlong out_root_element = (jlong)out_root_value;
        (*environment)->SetLongArrayRegion(
            environment, out_root, 0, 1, &out_root_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetSkinCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_get_skin_count_ext((CNA_ModelHandle)model, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetSkinExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbooleanArray out_has_data, jlongArray out_mesh_count)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_data_value = 0;
    uint64_t out_mesh_count_value = 0;
    CNA_Result call_result = cna.model_get_skin_ext((CNA_ModelHandle)model, (uint64_t)index, &out_has_data_value, &out_mesh_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_data_element = (jboolean)out_has_data_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_data, 0, 1, &out_has_data_element);
        jlong out_mesh_count_element = (jlong)out_mesh_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_mesh_count, 0, 1, &out_mesh_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetSkinMeshIndexExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlong mesh_index, jlongArray out_model_mesh_index)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_model_mesh_index_value = 0;
    CNA_Result call_result = cna.model_get_skin_mesh_index_ext((CNA_ModelHandle)model, (uint64_t)index, (uint64_t)mesh_index, &out_model_mesh_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_mesh_index_element = (jlong)out_model_mesh_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model_mesh_index, 0, 1, &out_model_mesh_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelGetSkinNameByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_get_skin_name_byte_count_ext((CNA_ModelHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_collection_destroy((CNA_ModelMeshCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong index, jlongArray out_mesh)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshHandle out_mesh_value = 0;
    CNA_Result call_result = cna.model_mesh_collection_get_at((CNA_ModelMeshCollectionHandle)collection, (uint64_t)index, &out_mesh_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_mesh_element = (jlong)out_mesh_value;
        (*environment)->SetLongArrayRegion(
            environment, out_mesh, 0, 1, &out_mesh_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_mesh_collection_get_count((CNA_ModelMeshCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshCopyName(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.model_mesh_copy_name((CNA_ModelMeshHandle)mesh, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshCreateNamed(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jbyteArray name, jlongArray parts, jlongArray out_mesh)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize parts_size = (*environment)->GetArrayLength(environment, parts);
    jlong* parts_elements = (*environment)->GetLongArrayElements(environment, parts, NULL);
    if (parts_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_ModelMeshPartHandle* parts_values = (CNA_ModelMeshPartHandle*)malloc(
        ((size_t)parts_size + 1U) * sizeof(*parts_values));
    if (parts_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, parts, parts_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < parts_size; ++index) {
        parts_values[index] = (CNA_ModelMeshPartHandle)parts_elements[index];
    }
    CNA_ModelMeshHandle out_mesh_value = 0;
    CNA_Result call_result = cna.model_mesh_create_named((CNA_Handle)graphics_device, name_view, parts_values, (uint64_t)parts_size, &out_mesh_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(parts_values);
    (*environment)->ReleaseLongArrayElements(
        environment, parts, parts_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_mesh_element = (jlong)out_mesh_value;
        (*environment)->SetLongArrayRegion(
            environment, out_mesh, 0, 1, &out_mesh_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshDestroy(
    JNIEnv* environment, jclass declaring_class, jlong mesh)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_destroy((CNA_ModelMeshHandle)mesh);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshGetBoundingSphere(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_BoundingSphere out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.model_mesh_get_bounding_sphere((CNA_ModelMeshHandle)mesh, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.center.x,
            (jfloat)out_value_value.center.y,
            (jfloat)out_value_value.center.z,
            (jfloat)out_value_value.radius
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshGetEffects(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jlongArray out_effects)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelEffectCollectionHandle out_effects_value = 0;
    CNA_Result call_result = cna.model_mesh_get_effects((CNA_ModelMeshHandle)mesh, &out_effects_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effects_element = (jlong)out_effects_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effects, 0, 1, &out_effects_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshGetMeshParts(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jlongArray out_parts)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshPartCollectionHandle out_parts_value = 0;
    CNA_Result call_result = cna.model_mesh_get_mesh_parts((CNA_ModelMeshHandle)mesh, &out_parts_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_parts_element = (jlong)out_parts_value;
        (*environment)->SetLongArrayRegion(
            environment, out_parts, 0, 1, &out_parts_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshGetNameByteCount(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.model_mesh_get_name_byte_count((CNA_ModelMeshHandle)mesh, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshGetParentBone(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jbooleanArray out_has_parent, jlongArray out_parent)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_parent_value = 0;
    CNA_ModelBoneHandle out_parent_value = 0;
    CNA_Result call_result = cna.model_mesh_get_parent_bone((CNA_ModelMeshHandle)mesh, &out_has_parent_value, &out_parent_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_parent_element = (jboolean)out_has_parent_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_parent, 0, 1, &out_has_parent_element);
        jlong out_parent_element = (jlong)out_parent_value;
        (*environment)->SetLongArrayRegion(
            environment, out_parent, 0, 1, &out_parent_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_part_collection_destroy((CNA_ModelMeshPartCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartCollectionGetAt(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlong index, jlongArray out_part)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshPartHandle out_part_value = 0;
    CNA_Result call_result = cna.model_mesh_part_collection_get_at((CNA_ModelMeshPartCollectionHandle)collection, (uint64_t)index, &out_part_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_part_element = (jlong)out_part_value;
        (*environment)->SetLongArrayRegion(
            environment, out_part, 0, 1, &out_part_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.model_mesh_part_collection_get_count((CNA_ModelMeshPartCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartCreate(
    JNIEnv* environment, jclass declaring_class, jlong vertex_buffer, jlong index_buffer, jint num_vertices, jint primitive_count, jint start_index, jint vertex_offset, jlongArray out_part)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshPartHandle out_part_value = 0;
    CNA_Result call_result = cna.model_mesh_part_create((CNA_VertexBufferHandle)vertex_buffer, (CNA_IndexBufferHandle)index_buffer, (int32_t)num_vertices, (int32_t)primitive_count, (int32_t)start_index, (int32_t)vertex_offset, &out_part_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_part_element = (jlong)out_part_value;
        (*environment)->SetLongArrayRegion(
            environment, out_part, 0, 1, &out_part_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartDestroy(
    JNIEnv* environment, jclass declaring_class, jlong part)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_part_destroy((CNA_ModelMeshPartHandle)part);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetEffect(
    JNIEnv* environment, jclass declaring_class, jlong part, jbooleanArray out_has_effect, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_effect_value = 0;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_effect((CNA_ModelMeshPartHandle)part, &out_has_effect_value, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_effect_element = (jboolean)out_has_effect_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_effect, 0, 1, &out_has_effect_element);
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetIndexBuffer(
    JNIEnv* environment, jclass declaring_class, jlong part, jbooleanArray out_has_buffer, jlongArray out_buffer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_buffer_value = 0;
    CNA_IndexBufferHandle out_buffer_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_index_buffer((CNA_ModelMeshPartHandle)part, &out_has_buffer_value, &out_buffer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_buffer_element = (jboolean)out_has_buffer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_buffer, 0, 1, &out_has_buffer_element);
        jlong out_buffer_element = (jlong)out_buffer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_buffer, 0, 1, &out_buffer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetMorphTargetDataExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jbooleanArray out_has_data, jlongArray out_data)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_data_value = 0;
    CNA_MorphTargetDataEXTHandle out_data_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_morph_target_data_ext((CNA_ModelMeshPartHandle)part, &out_has_data_value, &out_data_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_data_element = (jboolean)out_has_data_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_data, 0, 1, &out_has_data_element);
        jlong out_data_element = (jlong)out_data_value;
        (*environment)->SetLongArrayRegion(
            environment, out_data, 0, 1, &out_data_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetNumVertices(
    JNIEnv* environment, jclass declaring_class, jlong part, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_num_vertices((CNA_ModelMeshPartHandle)part, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetPrimitiveCount(
    JNIEnv* environment, jclass declaring_class, jlong part, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_primitive_count((CNA_ModelMeshPartHandle)part, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetPrimitiveTypeExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_PrimitiveType out_value_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_primitive_type_ext((CNA_ModelMeshPartHandle)part, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetSamplerStateExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jint slot, jlongArray out_stateIntegral, jfloatArray out_stateFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_stateIntegral == NULL || out_stateFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SamplerState out_state_value;
    memset(&out_state_value, 0, sizeof out_state_value);
    out_state_value.struct_size = (uint32_t)(sizeof out_state_value);
    out_state_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.model_mesh_part_get_sampler_state_ext((CNA_ModelMeshPartHandle)part, (CNA_PbrTextureSlot)slot, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_stateIntegral_values[] = {
            (jlong)out_state_value.address_u,
            (jlong)out_state_value.address_v,
            (jlong)out_state_value.address_w,
            (jlong)out_state_value.filter,
            (jlong)out_state_value.max_anisotropy,
            (jlong)out_state_value.max_mip_level,
            (jlong)out_state_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_stateIntegral, 0,
            (jsize)(sizeof out_stateIntegral_values / sizeof out_stateIntegral_values[0]),
            out_stateIntegral_values);
        jfloat out_stateFloating_values[] = {
            (jfloat)out_state_value.mip_map_level_of_detail_bias
        };
        (*environment)->SetFloatArrayRegion(environment, out_stateFloating, 0,
            (jsize)(sizeof out_stateFloating_values / sizeof out_stateFloating_values[0]),
            out_stateFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetStartIndex(
    JNIEnv* environment, jclass declaring_class, jlong part, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_start_index((CNA_ModelMeshPartHandle)part, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetVertexBuffer(
    JNIEnv* environment, jclass declaring_class, jlong part, jbooleanArray out_has_buffer, jlongArray out_buffer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_buffer_value = 0;
    CNA_VertexBufferHandle out_buffer_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_vertex_buffer((CNA_ModelMeshPartHandle)part, &out_has_buffer_value, &out_buffer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_buffer_element = (jboolean)out_has_buffer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_buffer, 0, 1, &out_has_buffer_element);
        jlong out_buffer_element = (jlong)out_buffer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_buffer, 0, 1, &out_buffer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartGetVertexOffset(
    JNIEnv* environment, jclass declaring_class, jlong part, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.model_mesh_part_get_vertex_offset((CNA_ModelMeshPartHandle)part, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartSetEffect(
    JNIEnv* environment, jclass declaring_class, jlong part, jlong effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_part_set_effect((CNA_ModelMeshPartHandle)part, (CNA_EffectHandle)effect);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartSetMorphTargetDataExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jlong data)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_part_set_morph_target_data_ext((CNA_ModelMeshPartHandle)part, (CNA_MorphTargetDataEXTHandle)data);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartSetMorphWeightsExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jfloatArray weights)
{
    (void)environment;
    (void)declaring_class;
    jsize weights_size = (*environment)->GetArrayLength(environment, weights);
    jfloat* weights_elements = (*environment)->GetFloatArrayElements(environment, weights, NULL);
    if (weights_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* weights_values = (float*)malloc(
        ((size_t)weights_size + 1U) * sizeof(*weights_values));
    if (weights_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < weights_size; ++index) {
        weights_values[index] = (float)weights_elements[index];
    }
    CNA_Result call_result = cna.model_mesh_part_set_morph_weights_ext((CNA_ModelMeshPartHandle)part, weights_values, (uint64_t)weights_size);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartSetPrimitiveTypeExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_part_set_primitive_type_ext((CNA_ModelMeshPartHandle)part, (CNA_PrimitiveType)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshPartSetSamplerStateExt(
    JNIEnv* environment, jclass declaring_class, jlong part, jint slot, jlongArray stateIntegral, jfloatArray stateFloating)
{
    (void)environment;
    (void)declaring_class;
    if (stateIntegral == NULL || stateFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SamplerState state_value;
    memset(&state_value, 0, sizeof state_value);
    state_value.struct_size = (uint32_t)(sizeof state_value);
    state_value.struct_version = UINT32_C(1);
    {
        jlong stateIntegral_values[7];
        (*environment)->GetLongArrayRegion(environment, stateIntegral, 0, 7, stateIntegral_values);
        state_value.address_u = (CNA_TextureAddressMode)stateIntegral_values[0];
        state_value.address_v = (CNA_TextureAddressMode)stateIntegral_values[1];
        state_value.address_w = (CNA_TextureAddressMode)stateIntegral_values[2];
        state_value.filter = (CNA_TextureFilter)stateIntegral_values[3];
        state_value.max_anisotropy = (int32_t)stateIntegral_values[4];
        state_value.max_mip_level = (int32_t)stateIntegral_values[5];
        state_value.reserved = (uint32_t)stateIntegral_values[6];
    }
    {
        jfloat stateFloating_values[1];
        (*environment)->GetFloatArrayRegion(environment, stateFloating, 0, 1, stateFloating_values);
        state_value.mip_map_level_of_detail_bias = (float)stateFloating_values[0];
    }
    CNA_Result call_result = cna.model_mesh_part_set_sampler_state_ext((CNA_ModelMeshPartHandle)part, (CNA_PbrTextureSlot)slot, &state_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshSetBoundingSphere(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_BoundingSphere value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[4];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 4, valueFloating_values);
        value_value.center.x = (float)valueFloating_values[0];
        value_value.center.y = (float)valueFloating_values[1];
        value_value.center.z = (float)valueFloating_values[2];
        value_value.radius = (float)valueFloating_values[3];
    }
    CNA_Result call_result = cna.model_mesh_set_bounding_sphere((CNA_ModelMeshHandle)mesh, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelMeshSetParentBone(
    JNIEnv* environment, jclass declaring_class, jlong mesh, jlong parent)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_mesh_set_parent_bone((CNA_ModelMeshHandle)mesh, (CNA_ModelBoneHandle)parent);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelSetBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray sourceFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize source_count = (*environment)->GetArrayLength(environment, sourceFloating) / 16;
    if ((*environment)->GetArrayLength(environment, sourceFloating) != source_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* source_values = (CNA_Matrix*)calloc(
        (size_t)source_count + 1U, sizeof(*source_values));
    if (source_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize sourceFloating_length = (*environment)->GetArrayLength(environment, sourceFloating);
        jfloat* sourceFloating_values = (jfloat*)malloc(
            ((size_t)sourceFloating_length + 1U) * sizeof(*sourceFloating_values));
        if (sourceFloating_values == NULL) {
            free(source_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, sourceFloating, 0, sourceFloating_length, sourceFloating_values);
        for (jsize element = 0; element < source_count; ++element) {
            source_values[element].m11 = (float)sourceFloating_values[element * 16 + 0];
            source_values[element].m12 = (float)sourceFloating_values[element * 16 + 1];
            source_values[element].m13 = (float)sourceFloating_values[element * 16 + 2];
            source_values[element].m14 = (float)sourceFloating_values[element * 16 + 3];
            source_values[element].m21 = (float)sourceFloating_values[element * 16 + 4];
            source_values[element].m22 = (float)sourceFloating_values[element * 16 + 5];
            source_values[element].m23 = (float)sourceFloating_values[element * 16 + 6];
            source_values[element].m24 = (float)sourceFloating_values[element * 16 + 7];
            source_values[element].m31 = (float)sourceFloating_values[element * 16 + 8];
            source_values[element].m32 = (float)sourceFloating_values[element * 16 + 9];
            source_values[element].m33 = (float)sourceFloating_values[element * 16 + 10];
            source_values[element].m34 = (float)sourceFloating_values[element * 16 + 11];
            source_values[element].m41 = (float)sourceFloating_values[element * 16 + 12];
            source_values[element].m42 = (float)sourceFloating_values[element * 16 + 13];
            source_values[element].m43 = (float)sourceFloating_values[element * 16 + 14];
            source_values[element].m44 = (float)sourceFloating_values[element * 16 + 15];
        }
        free(sourceFloating_values);
    }
    CNA_Result call_result = cna.model_set_bone_transforms((CNA_ModelHandle)model, source_values, (uint64_t)source_count);
    free(source_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelSetGltfImportReportExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray reportIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (reportIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GltfImportReportEXT report_value;
    memset(&report_value, 0, sizeof report_value);
    report_value.struct_size = (uint32_t)(sizeof report_value);
    report_value.struct_version = UINT32_C(1);
    {
        jlong reportIntegral_values[17];
        (*environment)->GetLongArrayRegion(environment, reportIntegral, 0, 17, reportIntegral_values);
        report_value.node_count = (uint64_t)reportIntegral_values[0];
        report_value.mesh_instance_count = (uint64_t)reportIntegral_values[1];
        report_value.distinct_mesh_count = (uint64_t)reportIntegral_values[2];
        report_value.shared_mesh_count = (uint64_t)reportIntegral_values[3];
        report_value.max_node_depth = (uint64_t)reportIntegral_values[4];
        report_value.camera_node_count = (uint64_t)reportIntegral_values[5];
        report_value.light_node_count = (uint64_t)reportIntegral_values[6];
        report_value.imported_light_count = (uint64_t)reportIntegral_values[7];
        report_value.primitive_count = (uint64_t)reportIntegral_values[8];
        report_value.skin_count = (uint64_t)reportIntegral_values[9];
        report_value.animation_count = (uint64_t)reportIntegral_values[10];
        report_value.clip_count = (uint64_t)reportIntegral_values[11];
        report_value.diagnostic_count = (uint64_t)reportIntegral_values[12];
        report_value.warning_count = (uint64_t)reportIntegral_values[13];
        report_value.dropped_feature_count = (uint64_t)reportIntegral_values[14];
        report_value.approximation_count = (uint64_t)reportIntegral_values[15];
        report_value.anything_lost = (CNA_Bool)reportIntegral_values[16];
    }
    CNA_Result call_result = cna.model_set_gltf_import_report_ext((CNA_ModelHandle)model, &report_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_modelSetMaterialVariantExt(
    JNIEnv* environment, jclass declaring_class, jlong model, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.model_set_material_variant_ext((CNA_ModelHandle)model, (int32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataBlend(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray weights, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize weights_size = (*environment)->GetArrayLength(environment, weights);
    jfloat* weights_elements = (*environment)->GetFloatArrayElements(environment, weights, NULL);
    if (weights_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* weights_values = (float*)malloc(
        ((size_t)weights_size + 1U) * sizeof(*weights_values));
    if (weights_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < weights_size; ++index) {
        weights_values[index] = (float)weights_elements[index];
    }
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_blend((CNA_MorphTargetDataEXTHandle)data, weights_values, (uint64_t)weights_size, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyBaseVertexBytes(
    JNIEnv* environment, jclass declaring_class, jlong data, jbyteArray destination, jlongArray out_byte_count)
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
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_base_vertex_bytes((CNA_MorphTargetDataEXTHandle)data, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyNormalDeltas(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong target_index, jfloatArray destinationFloating, jlongArray out_delta_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 3;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 3) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3* destination_values = (CNA_Vector3*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].x = (float)destinationFloating_values[element * 3 + 0];
            destination_values[element].y = (float)destinationFloating_values[element * 3 + 1];
            destination_values[element].z = (float)destinationFloating_values[element * 3 + 2];
        }
        free(destinationFloating_values);
    }
    uint64_t out_delta_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_normal_deltas((CNA_MorphTargetDataEXTHandle)data, (uint64_t)target_index, destination_values, (uint64_t)destination_count, &out_delta_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 3 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 3 + 0] = (jfloat)destination_values[element].x;
                    destinationFloating_out[element * 3 + 1] = (jfloat)destination_values[element].y;
                    destinationFloating_out[element * 3 + 2] = (jfloat)destination_values[element].z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 3), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_delta_count_element = (jlong)out_delta_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_delta_count, 0, 1, &out_delta_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyPositionDeltas(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong target_index, jfloatArray destinationFloating, jlongArray out_delta_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 3;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 3) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3* destination_values = (CNA_Vector3*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].x = (float)destinationFloating_values[element * 3 + 0];
            destination_values[element].y = (float)destinationFloating_values[element * 3 + 1];
            destination_values[element].z = (float)destinationFloating_values[element * 3 + 2];
        }
        free(destinationFloating_values);
    }
    uint64_t out_delta_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_position_deltas((CNA_MorphTargetDataEXTHandle)data, (uint64_t)target_index, destination_values, (uint64_t)destination_count, &out_delta_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 3 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 3 + 0] = (jfloat)destination_values[element].x;
                    destinationFloating_out[element * 3 + 1] = (jfloat)destination_values[element].y;
                    destinationFloating_out[element * 3 + 2] = (jfloat)destination_values[element].z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 3), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_delta_count_element = (jlong)out_delta_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_delta_count, 0, 1, &out_delta_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyTangentDeltas(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong target_index, jfloatArray destinationFloating, jlongArray out_delta_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 3;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 3) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3* destination_values = (CNA_Vector3*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].x = (float)destinationFloating_values[element * 3 + 0];
            destination_values[element].y = (float)destinationFloating_values[element * 3 + 1];
            destination_values[element].z = (float)destinationFloating_values[element * 3 + 2];
        }
        free(destinationFloating_values);
    }
    uint64_t out_delta_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_tangent_deltas((CNA_MorphTargetDataEXTHandle)data, (uint64_t)target_index, destination_values, (uint64_t)destination_count, &out_delta_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 3 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 3 + 0] = (jfloat)destination_values[element].x;
                    destinationFloating_out[element * 3 + 1] = (jfloat)destination_values[element].y;
                    destinationFloating_out[element * 3 + 2] = (jfloat)destination_values[element].z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 3), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_delta_count_element = (jlong)out_delta_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_delta_count, 0, 1, &out_delta_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyTriangleIndicesExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jintArray destination, jlongArray out_index_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jint* destination_elements = (*environment)->GetIntArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint32_t* destination_values = (uint32_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint32_t)destination_elements[index];
    }
    uint64_t out_index_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_triangle_indices_ext((CNA_MorphTargetDataEXTHandle)data, destination_values, (uint64_t)destination_size, &out_index_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jint)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseIntArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_count_element = (jlong)out_index_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index_count, 0, 1, &out_index_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyWeightKeyframe(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong keyframe_index, jdoubleArray out_time_seconds, jfloatArray weights, jlongArray out_weight_count, jfloatArray in_tangents, jlongArray out_in_tangent_count, jfloatArray out_tangents, jlongArray out_out_tangent_count)
{
    (void)environment;
    (void)declaring_class;
    double out_time_seconds_value = 0;
    jsize weights_size = (*environment)->GetArrayLength(environment, weights);
    jfloat* weights_elements = (*environment)->GetFloatArrayElements(environment, weights, NULL);
    if (weights_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* weights_values = (float*)malloc(
        ((size_t)weights_size + 1U) * sizeof(*weights_values));
    if (weights_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < weights_size; ++index) {
        weights_values[index] = (float)weights_elements[index];
    }
    uint64_t out_weight_count_value = 0;
    jsize in_tangents_size = (*environment)->GetArrayLength(environment, in_tangents);
    jfloat* in_tangents_elements = (*environment)->GetFloatArrayElements(environment, in_tangents, NULL);
    if (in_tangents_elements == NULL) {
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* in_tangents_values = (float*)malloc(
        ((size_t)in_tangents_size + 1U) * sizeof(*in_tangents_values));
    if (in_tangents_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, in_tangents, in_tangents_elements, JNI_ABORT);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < in_tangents_size; ++index) {
        in_tangents_values[index] = (float)in_tangents_elements[index];
    }
    uint64_t out_in_tangent_count_value = 0;
    jsize out_tangents_size = (*environment)->GetArrayLength(environment, out_tangents);
    jfloat* out_tangents_elements = (*environment)->GetFloatArrayElements(environment, out_tangents, NULL);
    if (out_tangents_elements == NULL) {
    free(in_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, in_tangents, in_tangents_elements, JNI_ABORT);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* out_tangents_values = (float*)malloc(
        ((size_t)out_tangents_size + 1U) * sizeof(*out_tangents_values));
    if (out_tangents_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, out_tangents, out_tangents_elements, JNI_ABORT);
    free(in_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, in_tangents, in_tangents_elements, JNI_ABORT);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < out_tangents_size; ++index) {
        out_tangents_values[index] = (float)out_tangents_elements[index];
    }
    uint64_t out_out_tangent_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_weight_keyframe((CNA_MorphTargetDataEXTHandle)data, (uint64_t)keyframe_index, &out_time_seconds_value, weights_values, (uint64_t)weights_size, &out_weight_count_value, in_tangents_values, (uint64_t)in_tangents_size, &out_in_tangent_count_value, out_tangents_values, (uint64_t)out_tangents_size, &out_out_tangent_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < weights_size; ++index) {
            weights_elements[index] = (jfloat)weights_values[index];
        }
    }
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < in_tangents_size; ++index) {
            in_tangents_elements[index] = (jfloat)in_tangents_values[index];
        }
    }
    free(in_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, in_tangents, in_tangents_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < out_tangents_size; ++index) {
            out_tangents_elements[index] = (jfloat)out_tangents_values[index];
        }
    }
    free(out_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, out_tangents, out_tangents_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_time_seconds_element = (jdouble)out_time_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_time_seconds, 0, 1, &out_time_seconds_element);
        jlong out_weight_count_element = (jlong)out_weight_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_weight_count, 0, 1, &out_weight_count_element);
        jlong out_in_tangent_count_element = (jlong)out_in_tangent_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_in_tangent_count, 0, 1, &out_in_tangent_count_element);
        jlong out_out_tangent_count_element = (jlong)out_out_tangent_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_out_tangent_count, 0, 1, &out_out_tangent_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataCopyWeights(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray destination, jlongArray out_weight_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jfloat* destination_elements = (*environment)->GetFloatArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* destination_values = (float*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (float)destination_elements[index];
    }
    uint64_t out_weight_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_copy_weights((CNA_MorphTargetDataEXTHandle)data, destination_values, (uint64_t)destination_size, &out_weight_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jfloat)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_weight_count_element = (jlong)out_weight_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_weight_count, 0, 1, &out_weight_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataDestroy(
    JNIEnv* environment, jclass declaring_class, jlong data)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.morph_target_data_ext_destroy((CNA_MorphTargetDataEXTHandle)data);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataGetBaseVertexByteCount(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_get_base_vertex_byte_count((CNA_MorphTargetDataEXTHandle)data, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataGetRecomputeFlatNormalsExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jbooleanArray out_recompute)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_recompute_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_get_recompute_flat_normals_ext((CNA_MorphTargetDataEXTHandle)data, &out_recompute_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_recompute_element = (jboolean)out_recompute_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_recompute, 0, 1, &out_recompute_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataGetStride(
    JNIEnv* environment, jclass declaring_class, jlong data, jintArray out_stride)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_stride_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_get_stride((CNA_MorphTargetDataEXTHandle)data, &out_stride_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_stride_element = (jint)out_stride_value;
        (*environment)->SetIntArrayRegion(
            environment, out_stride, 0, 1, &out_stride_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataGetTargetCount(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_target_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_target_count_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_get_target_count((CNA_MorphTargetDataEXTHandle)data, &out_target_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_target_count_element = (jlong)out_target_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_target_count, 0, 1, &out_target_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataGetWeightTrackInfo(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_keyframe_count, jbooleanArray out_step_interpolation, jbooleanArray out_cubic_spline)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_keyframe_count_value = 0;
    CNA_Bool out_step_interpolation_value = 0;
    CNA_Bool out_cubic_spline_value = 0;
    CNA_Result call_result = cna.morph_target_data_ext_get_weight_track_info((CNA_MorphTargetDataEXTHandle)data, &out_keyframe_count_value, &out_step_interpolation_value, &out_cubic_spline_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_keyframe_count_element = (jlong)out_keyframe_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keyframe_count, 0, 1, &out_keyframe_count_element);
        jboolean out_step_interpolation_element = (jboolean)out_step_interpolation_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_step_interpolation, 0, 1, &out_step_interpolation_element);
        jboolean out_cubic_spline_element = (jboolean)out_cubic_spline_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_cubic_spline, 0, 1, &out_cubic_spline_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataSetRecomputeFlatNormalsExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jboolean recompute)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.morph_target_data_ext_set_recompute_flat_normals_ext((CNA_MorphTargetDataEXTHandle)data, (CNA_Bool)recompute);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataSetTangentDeltas(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong target_index, jfloatArray deltasFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize deltas_count = (*environment)->GetArrayLength(environment, deltasFloating) / 3;
    if ((*environment)->GetArrayLength(environment, deltasFloating) != deltas_count * 3) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3* deltas_values = (CNA_Vector3*)calloc(
        (size_t)deltas_count + 1U, sizeof(*deltas_values));
    if (deltas_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize deltasFloating_length = (*environment)->GetArrayLength(environment, deltasFloating);
        jfloat* deltasFloating_values = (jfloat*)malloc(
            ((size_t)deltasFloating_length + 1U) * sizeof(*deltasFloating_values));
        if (deltasFloating_values == NULL) {
            free(deltas_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, deltasFloating, 0, deltasFloating_length, deltasFloating_values);
        for (jsize element = 0; element < deltas_count; ++element) {
            deltas_values[element].x = (float)deltasFloating_values[element * 3 + 0];
            deltas_values[element].y = (float)deltasFloating_values[element * 3 + 1];
            deltas_values[element].z = (float)deltasFloating_values[element * 3 + 2];
        }
        free(deltasFloating_values);
    }
    CNA_Result call_result = cna.morph_target_data_ext_set_tangent_deltas((CNA_MorphTargetDataEXTHandle)data, (uint64_t)target_index, deltas_values, (uint64_t)deltas_count);
    free(deltas_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataSetTriangleIndicesExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jintArray indices)
{
    (void)environment;
    (void)declaring_class;
    jsize indices_size = (*environment)->GetArrayLength(environment, indices);
    jint* indices_elements = (*environment)->GetIntArrayElements(environment, indices, NULL);
    if (indices_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint32_t* indices_values = (uint32_t*)malloc(
        ((size_t)indices_size + 1U) * sizeof(*indices_values));
    if (indices_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, indices, indices_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < indices_size; ++index) {
        indices_values[index] = (uint32_t)indices_elements[index];
    }
    CNA_Result call_result = cna.morph_target_data_ext_set_triangle_indices_ext((CNA_MorphTargetDataEXTHandle)data, indices_values, (uint64_t)indices_size);
    free(indices_values);
    (*environment)->ReleaseIntArrayElements(
        environment, indices, indices_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_morphTargetDataSetWeights(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray weights)
{
    (void)environment;
    (void)declaring_class;
    jsize weights_size = (*environment)->GetArrayLength(environment, weights);
    jfloat* weights_elements = (*environment)->GetFloatArrayElements(environment, weights, NULL);
    if (weights_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* weights_values = (float*)malloc(
        ((size_t)weights_size + 1U) * sizeof(*weights_values));
    if (weights_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, weights, weights_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < weights_size; ++index) {
        weights_values[index] = (float)weights_elements[index];
    }
    CNA_Result call_result = cna.morph_target_data_ext_set_weights((CNA_MorphTargetDataEXTHandle)data, weights_values, (uint64_t)weights_size);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelAddPart(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jlong vertex_buffer, jlong index_buffer, jlong part, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.skinned_model_ext_add_part((CNA_SkinnedModelEXTHandle)model, name_view, (CNA_VertexBufferHandle)vertex_buffer, (CNA_IndexBufferHandle)index_buffer, (CNA_ModelMeshPartHandle)part, (CNA_Handle)texture);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelAttachParts(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong other)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinned_model_ext_attach_parts((CNA_SkinnedModelEXTHandle)model, (CNA_SkinnedModelEXTHandle)other);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelComputeBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray clip_name, jdouble position_seconds, jboolean loop, jfloatArray destinationFloating, jlongArray out_bone_count)
{
    (void)environment;
    (void)declaring_class;
    jsize clip_name_size = (*environment)->GetArrayLength(environment, clip_name);
    jbyte* clip_name_bytes = (*environment)->GetByteArrayElements(environment, clip_name, NULL);
    if (clip_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView clip_name_view = {(const char*)clip_name_bytes, (uint64_t)clip_name_size};
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        (*environment)->ReleaseByteArrayElements(environment, clip_name, clip_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, clip_name, clip_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            (*environment)->ReleaseByteArrayElements(environment, clip_name, clip_name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_bone_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_compute_bone_transforms((CNA_SkinnedModelEXTHandle)model, clip_name_view, (double)position_seconds, (CNA_Bool)loop, destination_values, (uint64_t)destination_count, &out_bone_count_value);
    (*environment)->ReleaseByteArrayElements(environment, clip_name, clip_name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bone_count_element = (jlong)out_bone_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bone_count, 0, 1, &out_bone_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyBindPoseLocal(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_bind_pose_local((CNA_SkinnedModelEXTHandle)model, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyClipNameAt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong clip_index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_clip_name_at((CNA_SkinnedModelEXTHandle)model, (uint64_t)clip_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyClipTrack(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jlong track_index, jintArray out_bone_index, jfloatArray destinationFloating, jdoubleArray destinationDoubles, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    int32_t out_bone_index_value = 0;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 10;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 10 ||
        (*environment)->GetArrayLength(environment, destinationDoubles) != destination_count * 1) {
        (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT* destination_values = (CNA_KeyframeEXT*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].translation.x = (float)destinationFloating_values[element * 10 + 0];
            destination_values[element].translation.y = (float)destinationFloating_values[element * 10 + 1];
            destination_values[element].translation.z = (float)destinationFloating_values[element * 10 + 2];
            destination_values[element].rotation.x = (float)destinationFloating_values[element * 10 + 3];
            destination_values[element].rotation.y = (float)destinationFloating_values[element * 10 + 4];
            destination_values[element].rotation.z = (float)destinationFloating_values[element * 10 + 5];
            destination_values[element].rotation.w = (float)destinationFloating_values[element * 10 + 6];
            destination_values[element].scale.x = (float)destinationFloating_values[element * 10 + 7];
            destination_values[element].scale.y = (float)destinationFloating_values[element * 10 + 8];
            destination_values[element].scale.z = (float)destinationFloating_values[element * 10 + 9];
        }
        free(destinationFloating_values);
    }
    {
        jsize destinationDoubles_length = (*environment)->GetArrayLength(environment, destinationDoubles);
        jdouble* destinationDoubles_values = (jdouble*)malloc(
            ((size_t)destinationDoubles_length + 1U) * sizeof(*destinationDoubles_values));
        if (destinationDoubles_values == NULL) {
            free(destination_values);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetDoubleArrayRegion(environment, destinationDoubles, 0, destinationDoubles_length, destinationDoubles_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].time_seconds = (double)destinationDoubles_values[element * 1 + 0];
        }
        free(destinationDoubles_values);
    }
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_clip_track((CNA_SkinnedModelEXTHandle)model, name_view, (uint64_t)track_index, &out_bone_index_value, destination_values, (uint64_t)destination_count, &out_keyframe_count_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 10 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 10 + 0] = (jfloat)destination_values[element].translation.x;
                    destinationFloating_out[element * 10 + 1] = (jfloat)destination_values[element].translation.y;
                    destinationFloating_out[element * 10 + 2] = (jfloat)destination_values[element].translation.z;
                    destinationFloating_out[element * 10 + 3] = (jfloat)destination_values[element].rotation.x;
                    destinationFloating_out[element * 10 + 4] = (jfloat)destination_values[element].rotation.y;
                    destinationFloating_out[element * 10 + 5] = (jfloat)destination_values[element].rotation.z;
                    destinationFloating_out[element * 10 + 6] = (jfloat)destination_values[element].rotation.w;
                    destinationFloating_out[element * 10 + 7] = (jfloat)destination_values[element].scale.x;
                    destinationFloating_out[element * 10 + 8] = (jfloat)destination_values[element].scale.y;
                    destinationFloating_out[element * 10 + 9] = (jfloat)destination_values[element].scale.z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 10), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
        {
            jdouble* destinationDoubles_out = (jdouble*)malloc(
                ((size_t)destination_count * 1 + 1U) * sizeof(*destinationDoubles_out));
            if (destinationDoubles_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationDoubles_out[element * 1 + 0] = (jdouble)destination_values[element].time_seconds;
                }
                (*environment)->SetDoubleArrayRegion(
                    environment, destinationDoubles, 0,
                    (jsize)(destination_count * 1), destinationDoubles_out);
                free(destinationDoubles_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_bone_index_element = (jint)out_bone_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_bone_index, 0, 1, &out_bone_index_element);
        jlong out_keyframe_count_element = (jlong)out_keyframe_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keyframe_count, 0, 1, &out_keyframe_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyInverseBindPoseGlobal(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_inverse_bind_pose_global((CNA_SkinnedModelEXTHandle)model, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyParentBoneIndices(
    JNIEnv* environment, jclass declaring_class, jlong model, jintArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jint* destination_elements = (*environment)->GetIntArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    int32_t* destination_values = (int32_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (int32_t)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_parent_bone_indices((CNA_SkinnedModelEXTHandle)model, destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jint)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseIntArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCopyPartNameAt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part_index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.skinned_model_ext_copy_part_name_at((CNA_SkinnedModelEXTHandle)model, (uint64_t)part_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelCreateDefault(
    JNIEnv* environment, jclass declaring_class, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    CNA_SkinnedModelEXTHandle out_model_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_create_default(&out_model_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelDestroy(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinned_model_ext_destroy((CNA_SkinnedModelEXTHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetBoneCount(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_bone_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bone_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_bone_count((CNA_SkinnedModelEXTHandle)model, &out_bone_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bone_count_element = (jlong)out_bone_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bone_count, 0, 1, &out_bone_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetClipCount(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_clip_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_clip_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_clip_count((CNA_SkinnedModelEXTHandle)model, &out_clip_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_clip_count_element = (jlong)out_clip_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_clip_count, 0, 1, &out_clip_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetClipInfo(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jbooleanArray out_found, jdoubleArray out_duration_seconds, jlongArray out_track_count)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Bool out_found_value = 0;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_clip_info((CNA_SkinnedModelEXTHandle)model, name_view, &out_found_value, &out_duration_seconds_value, &out_track_count_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_found_element = (jboolean)out_found_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_found, 0, 1, &out_found_element);
        jdouble out_duration_seconds_element = (jdouble)out_duration_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_duration_seconds, 0, 1, &out_duration_seconds_element);
        jlong out_track_count_element = (jlong)out_track_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_track_count, 0, 1, &out_track_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetClipNameByteCountAt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong clip_index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_clip_name_byte_count_at((CNA_SkinnedModelEXTHandle)model, (uint64_t)clip_index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetOwnedResourceCounts(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_vertex_buffers, jlongArray out_index_buffers, jlongArray out_parts, jlongArray out_textures)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_vertex_buffers_value = 0;
    uint64_t out_index_buffers_value = 0;
    uint64_t out_parts_value = 0;
    uint64_t out_textures_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_owned_resource_counts((CNA_SkinnedModelEXTHandle)model, &out_vertex_buffers_value, &out_index_buffers_value, &out_parts_value, &out_textures_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_vertex_buffers_element = (jlong)out_vertex_buffers_value;
        (*environment)->SetLongArrayRegion(
            environment, out_vertex_buffers, 0, 1, &out_vertex_buffers_element);
        jlong out_index_buffers_element = (jlong)out_index_buffers_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index_buffers, 0, 1, &out_index_buffers_element);
        jlong out_parts_element = (jlong)out_parts_value;
        (*environment)->SetLongArrayRegion(
            environment, out_parts, 0, 1, &out_parts_element);
        jlong out_textures_element = (jlong)out_textures_value;
        (*environment)->SetLongArrayRegion(
            environment, out_textures, 0, 1, &out_textures_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetPartAt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part_index, jlongArray out_part, jbooleanArray out_has_texture, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_ModelMeshPartHandle out_part_value = 0;
    CNA_Bool out_has_texture_value = 0;
    CNA_Handle out_texture_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_part_at((CNA_SkinnedModelEXTHandle)model, (uint64_t)part_index, &out_part_value, &out_has_texture_value, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_part_element = (jlong)out_part_value;
        (*environment)->SetLongArrayRegion(
            environment, out_part, 0, 1, &out_part_element);
        jboolean out_has_texture_element = (jboolean)out_has_texture_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_texture, 0, 1, &out_has_texture_element);
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetPartCount(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_part_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_part_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_part_count((CNA_SkinnedModelEXTHandle)model, &out_part_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_part_count_element = (jlong)out_part_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_part_count, 0, 1, &out_part_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelGetPartNameByteCountAt(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part_index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.skinned_model_ext_get_part_name_byte_count_at((CNA_SkinnedModelEXTHandle)model, (uint64_t)part_index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelRemoveClip(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.skinned_model_ext_remove_clip((CNA_SkinnedModelEXTHandle)model, name_view);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinnedModelRemovePart(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.skinned_model_ext_remove_part((CNA_SkinnedModelEXTHandle)model, name_view);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopyBindPose(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinning_data_copy_bind_pose((CNA_SkinningDataHandle)data, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopyClipNameAt(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong clip_index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.skinning_data_copy_clip_name_at((CNA_SkinningDataHandle)data, (uint64_t)clip_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopyClipTrack(
    JNIEnv* environment, jclass declaring_class, jlong data, jbyteArray name, jlong track_index, jintArray out_bone_index, jfloatArray destinationFloating, jdoubleArray destinationDoubles, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    int32_t out_bone_index_value = 0;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 10;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 10 ||
        (*environment)->GetArrayLength(environment, destinationDoubles) != destination_count * 1) {
        (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT* destination_values = (CNA_KeyframeEXT*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].translation.x = (float)destinationFloating_values[element * 10 + 0];
            destination_values[element].translation.y = (float)destinationFloating_values[element * 10 + 1];
            destination_values[element].translation.z = (float)destinationFloating_values[element * 10 + 2];
            destination_values[element].rotation.x = (float)destinationFloating_values[element * 10 + 3];
            destination_values[element].rotation.y = (float)destinationFloating_values[element * 10 + 4];
            destination_values[element].rotation.z = (float)destinationFloating_values[element * 10 + 5];
            destination_values[element].rotation.w = (float)destinationFloating_values[element * 10 + 6];
            destination_values[element].scale.x = (float)destinationFloating_values[element * 10 + 7];
            destination_values[element].scale.y = (float)destinationFloating_values[element * 10 + 8];
            destination_values[element].scale.z = (float)destinationFloating_values[element * 10 + 9];
        }
        free(destinationFloating_values);
    }
    {
        jsize destinationDoubles_length = (*environment)->GetArrayLength(environment, destinationDoubles);
        jdouble* destinationDoubles_values = (jdouble*)malloc(
            ((size_t)destinationDoubles_length + 1U) * sizeof(*destinationDoubles_values));
        if (destinationDoubles_values == NULL) {
            free(destination_values);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetDoubleArrayRegion(environment, destinationDoubles, 0, destinationDoubles_length, destinationDoubles_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].time_seconds = (double)destinationDoubles_values[element * 1 + 0];
        }
        free(destinationDoubles_values);
    }
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.skinning_data_copy_clip_track((CNA_SkinningDataHandle)data, name_view, (uint64_t)track_index, &out_bone_index_value, destination_values, (uint64_t)destination_count, &out_keyframe_count_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 10 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 10 + 0] = (jfloat)destination_values[element].translation.x;
                    destinationFloating_out[element * 10 + 1] = (jfloat)destination_values[element].translation.y;
                    destinationFloating_out[element * 10 + 2] = (jfloat)destination_values[element].translation.z;
                    destinationFloating_out[element * 10 + 3] = (jfloat)destination_values[element].rotation.x;
                    destinationFloating_out[element * 10 + 4] = (jfloat)destination_values[element].rotation.y;
                    destinationFloating_out[element * 10 + 5] = (jfloat)destination_values[element].rotation.z;
                    destinationFloating_out[element * 10 + 6] = (jfloat)destination_values[element].rotation.w;
                    destinationFloating_out[element * 10 + 7] = (jfloat)destination_values[element].scale.x;
                    destinationFloating_out[element * 10 + 8] = (jfloat)destination_values[element].scale.y;
                    destinationFloating_out[element * 10 + 9] = (jfloat)destination_values[element].scale.z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 10), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
        {
            jdouble* destinationDoubles_out = (jdouble*)malloc(
                ((size_t)destination_count * 1 + 1U) * sizeof(*destinationDoubles_out));
            if (destinationDoubles_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationDoubles_out[element * 1 + 0] = (jdouble)destination_values[element].time_seconds;
                }
                (*environment)->SetDoubleArrayRegion(
                    environment, destinationDoubles, 0,
                    (jsize)(destination_count * 1), destinationDoubles_out);
                free(destinationDoubles_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_bone_index_element = (jint)out_bone_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_bone_index, 0, 1, &out_bone_index_element);
        jlong out_keyframe_count_element = (jlong)out_keyframe_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keyframe_count, 0, 1, &out_keyframe_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopyInverseBindPose(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinning_data_copy_inverse_bind_pose((CNA_SkinningDataHandle)data, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopySkeletonHierarchy(
    JNIEnv* environment, jclass declaring_class, jlong data, jintArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jint* destination_elements = (*environment)->GetIntArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    int32_t* destination_values = (int32_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (int32_t)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinning_data_copy_skeleton_hierarchy((CNA_SkinningDataHandle)data, destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jint)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseIntArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopySkeletonRootNameExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.skinning_data_copy_skeleton_root_name_ext((CNA_SkinningDataHandle)data, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataCopySkeletonRootPrefix(
    JNIEnv* environment, jclass declaring_class, jlong data, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinning_data_copy_skeleton_root_prefix((CNA_SkinningDataHandle)data, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataDestroy(
    JNIEnv* environment, jclass declaring_class, jlong data)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinning_data_destroy((CNA_SkinningDataHandle)data);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetBoneCount(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_bone_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bone_count_value = 0;
    CNA_Result call_result = cna.skinning_data_get_bone_count((CNA_SkinningDataHandle)data, &out_bone_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bone_count_element = (jlong)out_bone_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bone_count, 0, 1, &out_bone_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetClipCount(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_clip_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_clip_count_value = 0;
    CNA_Result call_result = cna.skinning_data_get_clip_count((CNA_SkinningDataHandle)data, &out_clip_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_clip_count_element = (jlong)out_clip_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_clip_count, 0, 1, &out_clip_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetClipInfo(
    JNIEnv* environment, jclass declaring_class, jlong data, jbyteArray name, jbooleanArray out_found, jdoubleArray out_duration_seconds, jlongArray out_track_count)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Bool out_found_value = 0;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_Result call_result = cna.skinning_data_get_clip_info((CNA_SkinningDataHandle)data, name_view, &out_found_value, &out_duration_seconds_value, &out_track_count_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_found_element = (jboolean)out_found_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_found, 0, 1, &out_found_element);
        jdouble out_duration_seconds_element = (jdouble)out_duration_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_duration_seconds, 0, 1, &out_duration_seconds_element);
        jlong out_track_count_element = (jlong)out_track_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_track_count, 0, 1, &out_track_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetClipNameByteCountAt(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong clip_index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.skinning_data_get_clip_name_byte_count_at((CNA_SkinningDataHandle)data, (uint64_t)clip_index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetClipTargetSpaceExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong clip_index, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_ClipTargetSpaceEXT out_value_value = 0;
    CNA_Result call_result = cna.skinning_data_get_clip_target_space_ext((CNA_SkinningDataHandle)data, (uint64_t)clip_index, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetSkeletonRootNameByteCountExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.skinning_data_get_skeleton_root_name_byte_count_ext((CNA_SkinningDataHandle)data, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataGetSkeletonRootNodeIndexExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.skinning_data_get_skeleton_root_node_index_ext((CNA_SkinningDataHandle)data, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataSetClipTargetSpaceExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jlong clip_index, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinning_data_set_clip_target_space_ext((CNA_SkinningDataHandle)data, (uint64_t)clip_index, (CNA_ClipTargetSpaceEXT)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataSetSkeletonRootNameExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jbyteArray name)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.skinning_data_set_skeleton_root_name_ext((CNA_SkinningDataHandle)data, name_view);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeModelExtensionRoutes_skinningDataSetSkeletonRootNodeIndexExt(
    JNIEnv* environment, jclass declaring_class, jlong data, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinning_data_set_skeleton_root_node_index_ext((CNA_SkinningDataHandle)data, (int32_t)value);
    return (jint)call_result;
}
