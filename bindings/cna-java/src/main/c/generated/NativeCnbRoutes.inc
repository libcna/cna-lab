/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeCnbRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAnimationClipCopyKeyframes(
    JNIEnv* environment, jclass declaring_class, jlong clip, jlong track, jfloatArray destinationFloating, jdoubleArray destinationDoubles, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 10;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 10 ||
        (*environment)->GetArrayLength(environment, destinationDoubles) != destination_count * 1) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT* destination_values = (CNA_KeyframeEXT*)calloc(
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
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetDoubleArrayRegion(environment, destinationDoubles, 0, destinationDoubles_length, destinationDoubles_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].time_seconds = (double)destinationDoubles_values[element * 1 + 0];
        }
        free(destinationDoubles_values);
    }
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.cnb_animation_clip_copy_keyframes((CNA_CnbAnimationClipHandle)clip, (uint64_t)track, destination_values, (uint64_t)destination_count, &out_keyframe_count_value);
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
        jlong out_keyframe_count_element = (jlong)out_keyframe_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keyframe_count, 0, 1, &out_keyframe_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAnimationClipDestroy(
    JNIEnv* environment, jclass declaring_class, jlong clip)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_animation_clip_destroy((CNA_CnbAnimationClipHandle)clip);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAnimationClipGet(
    JNIEnv* environment, jclass declaring_class, jlong clip, jdoubleArray out_duration_seconds, jlongArray out_track_count, jintArray out_target_space)
{
    (void)environment;
    (void)declaring_class;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_ClipTargetSpaceEXT out_target_space_value = 0;
    CNA_Result call_result = cna.cnb_animation_clip_get((CNA_CnbAnimationClipHandle)clip, &out_duration_seconds_value, &out_track_count_value, &out_target_space_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAnimationClipGetTrack(
    JNIEnv* environment, jclass declaring_class, jlong clip, jlong track, jintArray out_bone_index, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_bone_index_value = 0;
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.cnb_animation_clip_get_track((CNA_CnbAnimationClipHandle)clip, (uint64_t)track, &out_bone_index_value, &out_keyframe_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAssetTypeIdFromName(
    JNIEnv* environment, jclass declaring_class, jbyteArray name, jintArray out_asset_type_id)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    uint32_t out_asset_type_id_value = 0;
    CNA_Result call_result = cna.cnb_asset_type_id_from_name(name_view, &out_asset_type_id_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_asset_type_id_element = (jint)out_asset_type_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_asset_type_id, 0, 1, &out_asset_type_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbAudioFrameBytes(
    JNIEnv* environment, jclass declaring_class, jint format, jint channels, jintArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_audio_frame_bytes((CNA_CnbAudioFormat)format, (uint32_t)channels, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_byte_count_element = (jint)out_byte_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbBuildModelFromCnj(
    JNIEnv* environment, jclass declaring_class, jbyteArray cnj_path, jbyteArray content_root, jlongArray out_result)
{
    (void)environment;
    (void)declaring_class;
    jsize cnj_path_size = (*environment)->GetArrayLength(environment, cnj_path);
    jbyte* cnj_path_bytes = (*environment)->GetByteArrayElements(environment, cnj_path, NULL);
    if (cnj_path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView cnj_path_view = {(const char*)cnj_path_bytes, (uint64_t)cnj_path_size};
    jsize content_root_size = (*environment)->GetArrayLength(environment, content_root);
    jbyte* content_root_bytes = (*environment)->GetByteArrayElements(environment, content_root, NULL);
    if (content_root_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, cnj_path, cnj_path_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_root_view = {(const char*)content_root_bytes, (uint64_t)content_root_size};
    CNA_CnbModelFromCnjHandle out_result_value = 0;
    CNA_Result call_result = cna.cnb_build_model_from_cnj(cnj_path_view, content_root_view, &out_result_value);
    (*environment)->ReleaseByteArrayElements(environment, cnj_path, cnj_path_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_root, content_root_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_result_element = (jlong)out_result_value;
        (*environment)->SetLongArrayRegion(
            environment, out_result, 0, 1, &out_result_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterCopyBytes(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_byte_writer_copy_bytes((CNA_CnbByteWriterHandle)writer, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray out_writer)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbByteWriterHandle out_writer_value = 0;
    CNA_Result call_result = cna.cnb_byte_writer_create(&out_writer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_writer_element = (jlong)out_writer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_writer, 0, 1, &out_writer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterCreateFromBytes(
    JNIEnv* environment, jclass declaring_class, jbyteArray initial, jlongArray out_writer)
{
    (void)environment;
    (void)declaring_class;
    jsize initial_size = (*environment)->GetArrayLength(environment, initial);
    jbyte* initial_elements = (*environment)->GetByteArrayElements(environment, initial, NULL);
    if (initial_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* initial_values = (uint8_t*)malloc(
        ((size_t)initial_size + 1U) * sizeof(*initial_values));
    if (initial_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, initial, initial_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < initial_size; ++index) {
        initial_values[index] = (uint8_t)initial_elements[index];
    }
    CNA_CnbByteWriterHandle out_writer_value = 0;
    CNA_Result call_result = cna.cnb_byte_writer_create_from_bytes(initial_values, (uint64_t)initial_size, &out_writer_value);
    free(initial_values);
    (*environment)->ReleaseByteArrayElements(
        environment, initial, initial_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_writer_element = (jlong)out_writer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_writer, 0, 1, &out_writer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterDestroy(
    JNIEnv* environment, jclass declaring_class, jlong writer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_destroy((CNA_CnbByteWriterHandle)writer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterGetSize(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlongArray out_size)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_size_value = 0;
    CNA_Result call_result = cna.cnb_byte_writer_get_size((CNA_CnbByteWriterHandle)writer, &out_size_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_size_element = (jlong)out_size_value;
        (*environment)->SetLongArrayRegion(
            environment, out_size, 0, 1, &out_size_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterTake(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_byte_writer_take((CNA_CnbByteWriterHandle)writer, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteBytes(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    CNA_Result call_result = cna.cnb_byte_writer_write_bytes((CNA_CnbByteWriterHandle)writer, bytes_values, (uint64_t)bytes_size);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteF32(
    JNIEnv* environment, jclass declaring_class, jlong writer, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_f32((CNA_CnbByteWriterHandle)writer, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteF64(
    JNIEnv* environment, jclass declaring_class, jlong writer, jdouble value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_f64((CNA_CnbByteWriterHandle)writer, (double)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteI32(
    JNIEnv* environment, jclass declaring_class, jlong writer, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_i32((CNA_CnbByteWriterHandle)writer, (int32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteKeyframe(
    JNIEnv* environment, jclass declaring_class, jlong writer, jfloatArray keyframeFloating, jdoubleArray keyframeDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (keyframeFloating == NULL || keyframeDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT keyframe_value;
    memset(&keyframe_value, 0, sizeof keyframe_value);
    {
        jfloat keyframeFloating_values[10];
        (*environment)->GetFloatArrayRegion(environment, keyframeFloating, 0, 10, keyframeFloating_values);
        keyframe_value.translation.x = (float)keyframeFloating_values[0];
        keyframe_value.translation.y = (float)keyframeFloating_values[1];
        keyframe_value.translation.z = (float)keyframeFloating_values[2];
        keyframe_value.rotation.x = (float)keyframeFloating_values[3];
        keyframe_value.rotation.y = (float)keyframeFloating_values[4];
        keyframe_value.rotation.z = (float)keyframeFloating_values[5];
        keyframe_value.rotation.w = (float)keyframeFloating_values[6];
        keyframe_value.scale.x = (float)keyframeFloating_values[7];
        keyframe_value.scale.y = (float)keyframeFloating_values[8];
        keyframe_value.scale.z = (float)keyframeFloating_values[9];
    }
    {
        jdouble keyframeDoubles_values[1];
        (*environment)->GetDoubleArrayRegion(environment, keyframeDoubles, 0, 1, keyframeDoubles_values);
        keyframe_value.time_seconds = (double)keyframeDoubles_values[0];
    }
    CNA_Result call_result = cna.cnb_byte_writer_write_keyframe((CNA_CnbByteWriterHandle)writer, &keyframe_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteString(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray value)
{
    (void)environment;
    (void)declaring_class;
    jsize value_size = (*environment)->GetArrayLength(environment, value);
    jbyte* value_bytes = (*environment)->GetByteArrayElements(environment, value, NULL);
    if (value_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView value_view = {(const char*)value_bytes, (uint64_t)value_size};
    CNA_Result call_result = cna.cnb_byte_writer_write_string((CNA_CnbByteWriterHandle)writer, value_view);
    (*environment)->ReleaseByteArrayElements(environment, value, value_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteU16(
    JNIEnv* environment, jclass declaring_class, jlong writer, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_u16((CNA_CnbByteWriterHandle)writer, (uint16_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteU32(
    JNIEnv* environment, jclass declaring_class, jlong writer, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_u32((CNA_CnbByteWriterHandle)writer, (uint32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteU64(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlong value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_u64((CNA_CnbByteWriterHandle)writer, (uint64_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteU8(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyte value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_u8((CNA_CnbByteWriterHandle)writer, (uint8_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbByteWriterWriteZeros(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlong byte_count)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_byte_writer_write_zeros((CNA_CnbByteWriterHandle)writer, (uint64_t)byte_count);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCheckedAdd(
    JNIEnv* environment, jclass declaring_class, jlong a, jlong b, jlongArray out_sum)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_sum_value = 0;
    CNA_Result call_result = cna.cnb_checked_add((uint64_t)a, (uint64_t)b, &out_sum_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sum_element = (jlong)out_sum_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sum, 0, 1, &out_sum_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCheckedMultiply(
    JNIEnv* environment, jclass declaring_class, jlong a, jlong b, jlongArray out_product)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_product_value = 0;
    CNA_Result call_result = cna.cnb_checked_multiply((uint64_t)a, (uint64_t)b, &out_product_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_product_element = (jlong)out_product_value;
        (*environment)->SetLongArrayRegion(
            environment, out_product, 0, 1, &out_product_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbChunkEntryIsMandatory(
    JNIEnv* environment, jclass declaring_class, jlongArray entryIntegral, jbooleanArray out_mandatory)
{
    (void)environment;
    (void)declaring_class;
    if (entryIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbChunkEntry entry_value;
    memset(&entry_value, 0, sizeof entry_value);
    entry_value.struct_size = (uint32_t)(sizeof entry_value);
    entry_value.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    {
        jlong entryIntegral_values[9];
        (*environment)->GetLongArrayRegion(environment, entryIntegral, 0, 9, entryIntegral_values);
        entry_value.offset = (uint64_t)entryIntegral_values[0];
        entry_value.stored_size = (uint64_t)entryIntegral_values[1];
        entry_value.uncompressed_size = (uint64_t)entryIntegral_values[2];
        entry_value.type = (CNA_CnbChunkId)entryIntegral_values[3];
        entry_value.flags = (uint32_t)entryIntegral_values[4];
        entry_value.checksum = (uint32_t)entryIntegral_values[5];
        entry_value.compression = (CNA_CnbCompression)entryIntegral_values[6];
        entry_value.alignment = (uint32_t)entryIntegral_values[7];
        entry_value.reserved = (uint32_t)entryIntegral_values[8];
    }
    CNA_Bool out_mandatory_value = 0;
    CNA_Result call_result = cna.cnb_chunk_entry_is_mandatory(&entry_value, &out_mandatory_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_mandatory_element = (jboolean)out_mandatory_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_mandatory, 0, 1, &out_mandatory_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultCopyAbsorbedFile(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_cnj_result_copy_absorbed_file((CNA_CnjToCnbResultHandle)result, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultCopyAssetTypeName(
    JNIEnv* environment, jclass declaring_class, jlong result, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_cnj_result_copy_asset_type_name((CNA_CnjToCnbResultHandle)result, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultCopyBytes(
    JNIEnv* environment, jclass declaring_class, jlong result, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_cnj_result_copy_bytes((CNA_CnjToCnbResultHandle)result, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultCopyExternalReference(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_cnj_result_copy_external_reference((CNA_CnjToCnbResultHandle)result, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultDestroy(
    JNIEnv* environment, jclass declaring_class, jlong result)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_cnj_result_destroy((CNA_CnjToCnbResultHandle)result);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetAbsorbedFileCount(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_absorbed_file_count((CNA_CnjToCnbResultHandle)result, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetAbsorbedFileSize(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_absorbed_file_size((CNA_CnjToCnbResultHandle)result, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetAssetTypeId(
    JNIEnv* environment, jclass declaring_class, jlong result, jintArray out_asset_type_id)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_asset_type_id_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_asset_type_id((CNA_CnjToCnbResultHandle)result, &out_asset_type_id_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_asset_type_id_element = (jint)out_asset_type_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_asset_type_id, 0, 1, &out_asset_type_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetAssetTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_asset_type_name_size((CNA_CnjToCnbResultHandle)result, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetExternalReferenceCount(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_external_reference_count((CNA_CnjToCnbResultHandle)result, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCnjResultGetExternalReferenceSize(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_cnj_result_get_external_reference_size((CNA_CnjToCnbResultHandle)result, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCompileCnj(
    JNIEnv* environment, jclass declaring_class, jbyteArray cnj_path, jbyteArray content_root, jbyteArray content_name, jlongArray out_result)
{
    (void)environment;
    (void)declaring_class;
    jsize cnj_path_size = (*environment)->GetArrayLength(environment, cnj_path);
    jbyte* cnj_path_bytes = (*environment)->GetByteArrayElements(environment, cnj_path, NULL);
    if (cnj_path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView cnj_path_view = {(const char*)cnj_path_bytes, (uint64_t)cnj_path_size};
    jsize content_root_size = (*environment)->GetArrayLength(environment, content_root);
    jbyte* content_root_bytes = (*environment)->GetByteArrayElements(environment, content_root, NULL);
    if (content_root_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, cnj_path, cnj_path_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_root_view = {(const char*)content_root_bytes, (uint64_t)content_root_size};
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, content_root, content_root_bytes, JNI_ABORT);
        (*environment)->ReleaseByteArrayElements(environment, cnj_path, cnj_path_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    CNA_CnjToCnbResultHandle out_result_value = 0;
    CNA_Result call_result = cna.cnb_compile_cnj(cnj_path_view, content_root_view, content_name_view, &out_result_value);
    (*environment)->ReleaseByteArrayElements(environment, cnj_path, cnj_path_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_root, content_root_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_result_element = (jlong)out_result_value;
        (*environment)->SetLongArrayRegion(
            environment, out_result, 0, 1, &out_result_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyAssetTypeName(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_asset_type_name((uint32_t)asset_type_id, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyAudioFormatName(
    JNIEnv* environment, jclass declaring_class, jint format, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_audio_format_name((CNA_CnbAudioFormat)format, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyChunkIdString(
    JNIEnv* environment, jclass declaring_class, jint id, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_chunk_id_string((CNA_CnbChunkId)id, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyCompressed(
    JNIEnv* environment, jclass declaring_class, jbyteArray raw, jint codec, jint level, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize raw_size = (*environment)->GetArrayLength(environment, raw);
    jbyte* raw_elements = (*environment)->GetByteArrayElements(environment, raw, NULL);
    if (raw_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* raw_values = (uint8_t*)malloc(
        ((size_t)raw_size + 1U) * sizeof(*raw_values));
    if (raw_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, raw, raw_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < raw_size; ++index) {
        raw_values[index] = (uint8_t)raw_elements[index];
    }
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    free(raw_values);
    (*environment)->ReleaseByteArrayElements(
        environment, raw, raw_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    free(raw_values);
    (*environment)->ReleaseByteArrayElements(
        environment, raw, raw_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_copy_compressed(raw_values, (uint64_t)raw_size, (CNA_CnbCompression)codec, (int32_t)level, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    free(raw_values);
    (*environment)->ReleaseByteArrayElements(
        environment, raw, raw_elements, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyCompressionName(
    JNIEnv* environment, jclass declaring_class, jint codec, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_compression_name((CNA_CnbCompression)codec, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyDecompressed(
    JNIEnv* environment, jclass declaring_class, jbyteArray stored, jint codec, jlong uncompressed_size, jlong max_uncompressed_size, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize stored_size = (*environment)->GetArrayLength(environment, stored);
    jbyte* stored_elements = (*environment)->GetByteArrayElements(environment, stored, NULL);
    if (stored_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* stored_values = (uint8_t*)malloc(
        ((size_t)stored_size + 1U) * sizeof(*stored_values));
    if (stored_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, stored, stored_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < stored_size; ++index) {
        stored_values[index] = (uint8_t)stored_elements[index];
    }
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    free(stored_values);
    (*environment)->ReleaseByteArrayElements(
        environment, stored, stored_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    free(stored_values);
    (*environment)->ReleaseByteArrayElements(
        environment, stored, stored_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_copy_decompressed(stored_values, (uint64_t)stored_size, (CNA_CnbCompression)codec, (uint64_t)uncompressed_size, (uint64_t)max_uncompressed_size, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    free(stored_values);
    (*environment)->ReleaseByteArrayElements(
        environment, stored, stored_elements, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyFormatMagic(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_copy_format_magic(destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyLogicalNameProblem(
    JNIEnv* environment, jclass declaring_class, jbyteArray logical_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize logical_name_size = (*environment)->GetArrayLength(environment, logical_name);
    jbyte* logical_name_bytes = (*environment)->GetByteArrayElements(environment, logical_name, NULL);
    if (logical_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView logical_name_view = {(const char*)logical_name_bytes, (uint64_t)logical_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, logical_name, logical_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_logical_name_problem(logical_name_view, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, logical_name, logical_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCopyTextureFormatName(
    JNIEnv* environment, jclass declaring_class, jint format, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_copy_texture_format_name((CNA_CnbTextureFormat)format, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCrc32c(
    JNIEnv* environment, jclass declaring_class, jbyteArray data, jintArray out_checksum)
{
    (void)environment;
    (void)declaring_class;
    jsize data_size = (*environment)->GetArrayLength(environment, data);
    jbyte* data_elements = (*environment)->GetByteArrayElements(environment, data, NULL);
    if (data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* data_values = (uint8_t*)malloc(
        ((size_t)data_size + 1U) * sizeof(*data_values));
    if (data_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < data_size; ++index) {
        data_values[index] = (uint8_t)data_elements[index];
    }
    uint32_t out_checksum_value = 0;
    CNA_Result call_result = cna.cnb_crc32c(data_values, (uint64_t)data_size, &out_checksum_value);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_checksum_element = (jint)out_checksum_value;
        (*environment)->SetIntArrayRegion(
            environment, out_checksum, 0, 1, &out_checksum_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCrc32cContinue(
    JNIEnv* environment, jclass declaring_class, jint previous, jbyteArray data, jintArray out_checksum)
{
    (void)environment;
    (void)declaring_class;
    jsize data_size = (*environment)->GetArrayLength(environment, data);
    jbyte* data_elements = (*environment)->GetByteArrayElements(environment, data, NULL);
    if (data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* data_values = (uint8_t*)malloc(
        ((size_t)data_size + 1U) * sizeof(*data_values));
    if (data_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < data_size; ++index) {
        data_values[index] = (uint8_t)data_elements[index];
    }
    uint32_t out_checksum_value = 0;
    CNA_Result call_result = cna.cnb_crc32c_continue((uint32_t)previous, data_values, (uint64_t)data_size, &out_checksum_value);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_checksum_element = (jint)out_checksum_value;
        (*environment)->SetIntArrayRegion(
            environment, out_checksum, 0, 1, &out_checksum_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCrc32cPortable(
    JNIEnv* environment, jclass declaring_class, jbyteArray data, jintArray out_checksum)
{
    (void)environment;
    (void)declaring_class;
    jsize data_size = (*environment)->GetArrayLength(environment, data);
    jbyte* data_elements = (*environment)->GetByteArrayElements(environment, data, NULL);
    if (data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* data_values = (uint8_t*)malloc(
        ((size_t)data_size + 1U) * sizeof(*data_values));
    if (data_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < data_size; ++index) {
        data_values[index] = (uint8_t)data_elements[index];
    }
    uint32_t out_checksum_value = 0;
    CNA_Result call_result = cna.cnb_crc32c_portable(data_values, (uint64_t)data_size, &out_checksum_value);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_checksum_element = (jint)out_checksum_value;
        (*environment)->SetIntArrayRegion(
            environment, out_checksum, 0, 1, &out_checksum_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbCrc32cUsesHardware(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_uses_hardware)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_uses_hardware_value = 0;
    CNA_Result call_result = cna.cnb_crc32c_uses_hardware(&out_uses_hardware_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_uses_hardware_element = (jboolean)out_uses_hardware_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_uses_hardware, 0, 1, &out_uses_hardware_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeAnimationClip(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_clip)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbAnimationClipHandle out_clip_value = 0;
    CNA_Result call_result = cna.cnb_decode_animation_clip((CNA_CnbDocumentHandle)document, &out_clip_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_clip_element = (jlong)out_clip_value;
        (*environment)->SetLongArrayRegion(
            environment, out_clip, 0, 1, &out_clip_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeCurve(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_curve)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveHandle out_curve_value = 0;
    CNA_Result call_result = cna.cnb_decode_curve((CNA_CnbDocumentHandle)document, &out_curve_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_curve_element = (jlong)out_curve_value;
        (*environment)->SetLongArrayRegion(
            environment, out_curve, 0, 1, &out_curve_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeDdsAsTextureCube(
    JNIEnv* environment, jclass declaring_class, jbyteArray bytes, jbyteArray origin, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    jsize origin_size = (*environment)->GetArrayLength(environment, origin);
    jbyte* origin_bytes = (*environment)->GetByteArrayElements(environment, origin, NULL);
    if (origin_bytes == NULL) {
        free(bytes_values);
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView origin_view = {(const char*)origin_bytes, (uint64_t)origin_size};
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_decode_dds_as_texture_cube(bytes_values, (uint64_t)bytes_size, origin_view, &out_texture_value);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, origin, origin_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeModel(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbModelDataHandle out_model_value = 0;
    CNA_Result call_result = cna.cnb_decode_model((CNA_CnbDocumentHandle)document, &out_model_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSongDurationMilliseconds(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray out_duration_milliseconds)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_duration_milliseconds_value = 0;
    CNA_Result call_result = cna.cnb_decode_song_duration_milliseconds((CNA_CnbDocumentHandle)document, &out_duration_milliseconds_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_duration_milliseconds_element = (jint)out_duration_milliseconds_value;
        (*environment)->SetIntArrayRegion(
            environment, out_duration_milliseconds, 0, 1, &out_duration_milliseconds_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSongName(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_decode_song_name((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSongNameSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_decode_song_name_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSongStreamReference(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_decode_song_stream_reference((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSongStreamReferenceSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_decode_song_stream_reference_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSoundEffect(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_sound)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbSoundEffectDataHandle out_sound_value = 0;
    CNA_Result call_result = cna.cnb_decode_sound_effect((CNA_CnbDocumentHandle)document, &out_sound_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sound_element = (jlong)out_sound_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sound, 0, 1, &out_sound_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeSpriteFont(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_font)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbSpriteFontDataHandle out_font_value = 0;
    CNA_Result call_result = cna.cnb_decode_sprite_font((CNA_CnbDocumentHandle)document, &out_font_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_font_element = (jlong)out_font_value;
        (*environment)->SetLongArrayRegion(
            environment, out_font, 0, 1, &out_font_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_decode_texture2d((CNA_CnbDocumentHandle)document, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeTexture3d(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_decode_texture3d((CNA_CnbDocumentHandle)document, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeTextureCube(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_decode_texture_cube((CNA_CnbDocumentHandle)document, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeVideo(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_infoIntegral, jfloatArray out_infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL || out_infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbVideoInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_VIDEO_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_decode_video((CNA_CnbDocumentHandle)document, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.duration_milliseconds,
            (jlong)out_info_value.width,
            (jlong)out_info_value.height,
            (jlong)out_info_value.soundtrack_type,
            (jlong)out_info_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jfloat out_infoFloating_values[] = {
            (jfloat)out_info_value.frames_per_second
        };
        (*environment)->SetFloatArrayRegion(environment, out_infoFloating, 0,
            (jsize)(sizeof out_infoFloating_values / sizeof out_infoFloating_values[0]),
            out_infoFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeVideoStreamReference(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_decode_video_stream_reference((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeVideoStreamReferenceSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_decode_video_stream_reference_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDecodeWavAsSoundEffect(
    JNIEnv* environment, jclass declaring_class, jbyteArray bytes, jbyteArray origin, jlongArray out_sound)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    jsize origin_size = (*environment)->GetArrayLength(environment, origin);
    jbyte* origin_bytes = (*environment)->GetByteArrayElements(environment, origin, NULL);
    if (origin_bytes == NULL) {
        free(bytes_values);
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView origin_view = {(const char*)origin_bytes, (uint64_t)origin_size};
    CNA_CnbSoundEffectDataHandle out_sound_value = 0;
    CNA_Result call_result = cna.cnb_decode_wav_as_sound_effect(bytes_values, (uint64_t)bytes_size, origin_view, &out_sound_value);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, origin, origin_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sound_element = (jlong)out_sound_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sound, 0, 1, &out_sound_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentCopyChunkData(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_document_copy_chunk_data((CNA_CnbDocumentHandle)document, (uint64_t)index, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentCopyExternalReferenceName(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_document_copy_external_reference_name((CNA_CnbDocumentHandle)document, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentCopyMetadataAssetTypeName(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_document_copy_metadata_asset_type_name((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentCopyMetadataContentName(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_document_copy_metadata_content_name((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentCopyOrigin(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_document_copy_origin((CNA_CnbDocumentHandle)document, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentDestroy(
    JNIEnv* environment, jclass declaring_class, jlong document)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_document_destroy((CNA_CnbDocumentHandle)document);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentFindAll(
    JNIEnv* environment, jclass declaring_class, jlong document, jint type, jlongArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jlong* destination_elements = (*environment)->GetLongArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t* destination_values = (uint64_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint64_t)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_document_find_all((CNA_CnbDocumentHandle)document, (CNA_CnbChunkId)type, destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jlong)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseLongArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentFindSingle(
    JNIEnv* environment, jclass declaring_class, jlong document, jint type, jbooleanArray out_found, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_found_value = 0;
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_document_find_single((CNA_CnbDocumentHandle)document, (CNA_CnbChunkId)type, &out_found_value, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_found_element = (jboolean)out_found_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_found, 0, 1, &out_found_element);
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetAssetSchemaVersion(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray out_schema_version)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_schema_version_value = 0;
    CNA_Result call_result = cna.cnb_document_get_asset_schema_version((CNA_CnbDocumentHandle)document, &out_schema_version_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_schema_version_element = (jint)out_schema_version_value;
        (*environment)->SetIntArrayRegion(
            environment, out_schema_version, 0, 1, &out_schema_version_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetAssetTypeId(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray out_asset_type_id)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_asset_type_id_value = 0;
    CNA_Result call_result = cna.cnb_document_get_asset_type_id((CNA_CnbDocumentHandle)document, &out_asset_type_id_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_asset_type_id_element = (jint)out_asset_type_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_asset_type_id, 0, 1, &out_asset_type_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetChunk(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jlongArray out_entryIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_entryIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbChunkEntry out_entry_value;
    memset(&out_entry_value, 0, sizeof out_entry_value);
    out_entry_value.struct_size = (uint32_t)(sizeof out_entry_value);
    out_entry_value.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_document_get_chunk((CNA_CnbDocumentHandle)document, (uint64_t)index, &out_entry_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_entryIntegral_values[] = {
            (jlong)out_entry_value.offset,
            (jlong)out_entry_value.stored_size,
            (jlong)out_entry_value.uncompressed_size,
            (jlong)out_entry_value.type,
            (jlong)out_entry_value.flags,
            (jlong)out_entry_value.checksum,
            (jlong)out_entry_value.compression,
            (jlong)out_entry_value.alignment,
            (jlong)out_entry_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_entryIntegral, 0,
            (jsize)(sizeof out_entryIntegral_values / sizeof out_entryIntegral_values[0]),
            out_entryIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetChunkCount(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_chunk_count((CNA_CnbDocumentHandle)document, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetContainerMajor(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray out_major)
{
    (void)environment;
    (void)declaring_class;
    uint16_t out_major_value = 0;
    CNA_Result call_result = cna.cnb_document_get_container_major((CNA_CnbDocumentHandle)document, &out_major_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_major_element = (jint)out_major_value;
        (*environment)->SetIntArrayRegion(
            environment, out_major, 0, 1, &out_major_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetContainerMinor(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray out_minor)
{
    (void)environment;
    (void)declaring_class;
    uint16_t out_minor_value = 0;
    CNA_Result call_result = cna.cnb_document_get_container_minor((CNA_CnbDocumentHandle)document, &out_minor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_minor_element = (jint)out_minor_value;
        (*environment)->SetIntArrayRegion(
            environment, out_minor, 0, 1, &out_minor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetExternalReference(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jbyteArray what_for_diagnostics, jlongArray out_referenceIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_referenceIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize what_for_diagnostics_size = (*environment)->GetArrayLength(environment, what_for_diagnostics);
    jbyte* what_for_diagnostics_bytes = (*environment)->GetByteArrayElements(environment, what_for_diagnostics, NULL);
    if (what_for_diagnostics_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView what_for_diagnostics_view = {(const char*)what_for_diagnostics_bytes, (uint64_t)what_for_diagnostics_size};
    CNA_CnbExternalReference out_reference_value;
    memset(&out_reference_value, 0, sizeof out_reference_value);
    out_reference_value.struct_size = (uint32_t)(sizeof out_reference_value);
    out_reference_value.struct_version = CNA_CNB_EXTERNAL_REFERENCE_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_document_get_external_reference((CNA_CnbDocumentHandle)document, (uint64_t)index, what_for_diagnostics_view, &out_reference_value);
    (*environment)->ReleaseByteArrayElements(environment, what_for_diagnostics, what_for_diagnostics_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_referenceIntegral_values[] = {
            (jlong)out_reference_value.flags,
            (jlong)out_reference_value.expected_asset_type_id
        };
        (*environment)->SetLongArrayRegion(environment, out_referenceIntegral, 0,
            (jsize)(sizeof out_referenceIntegral_values / sizeof out_referenceIntegral_values[0]),
            out_referenceIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetExternalReferenceCount(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_external_reference_count((CNA_CnbDocumentHandle)document, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetExternalReferenceNameSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_external_reference_name_size((CNA_CnbDocumentHandle)document, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetLimits(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_limitsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbReadLimits out_limits_value;
    memset(&out_limits_value, 0, sizeof out_limits_value);
    out_limits_value.struct_size = (uint32_t)(sizeof out_limits_value);
    out_limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_document_get_limits((CNA_CnbDocumentHandle)document, &out_limits_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_limitsIntegral_values[] = {
            (jlong)out_limits_value.max_file_size,
            (jlong)out_limits_value.max_chunk_size,
            (jlong)out_limits_value.max_total_uncompressed_size,
            (jlong)out_limits_value.max_chunk_count,
            (jlong)out_limits_value.max_string_bytes,
            (jlong)out_limits_value.max_array_element_count,
            (jlong)out_limits_value.max_chunk_alignment
        };
        (*environment)->SetLongArrayRegion(environment, out_limitsIntegral, 0,
            (jsize)(sizeof out_limitsIntegral_values / sizeof out_limitsIntegral_values[0]),
            out_limitsIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetMetadata(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray out_metadataBytes, jlongArray out_metadataIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_metadataBytes == NULL || out_metadataIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMetadata out_metadata_value;
    memset(&out_metadata_value, 0, sizeof out_metadata_value);
    out_metadata_value.struct_size = (uint32_t)(sizeof out_metadata_value);
    out_metadata_value.struct_version = CNA_CNB_METADATA_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_document_get_metadata((CNA_CnbDocumentHandle)document, &out_metadata_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_metadataBytes_values[] = {
            (jbyte)out_metadata_value.reserved[0],
            (jbyte)out_metadata_value.reserved[1],
            (jbyte)out_metadata_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_metadataBytes, 0,
            (jsize)(sizeof out_metadataBytes_values / sizeof out_metadataBytes_values[0]),
            out_metadataBytes_values);
        jlong out_metadataIntegral_values[] = {
            (jlong)out_metadata_value.present,
            (jlong)out_metadata_value.flags
        };
        (*environment)->SetLongArrayRegion(environment, out_metadataIntegral, 0,
            (jsize)(sizeof out_metadataIntegral_values / sizeof out_metadataIntegral_values[0]),
            out_metadataIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetMetadataAssetTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_metadata_asset_type_name_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetMetadataContentNameSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_metadata_content_name_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentGetOriginSize(
    JNIEnv* environment, jclass declaring_class, jlong document, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_document_get_origin_size((CNA_CnbDocumentHandle)document, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentOpenChunk(
    JNIEnv* environment, jclass declaring_class, jlong document, jlong index, jlongArray out_reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbReaderHandle out_reader_value = 0;
    CNA_Result call_result = cna.cnb_document_open_chunk((CNA_CnbDocumentHandle)document, (uint64_t)index, &out_reader_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reader_element = (jlong)out_reader_value;
        (*environment)->SetLongArrayRegion(
            environment, out_reader, 0, 1, &out_reader_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentParse(
    JNIEnv* environment, jclass declaring_class, jbyteArray bytes, jbyteArray origin, jlongArray limitsIntegral, jlongArray out_document)
{
    (void)environment;
    (void)declaring_class;
    if (limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    jsize origin_size = (*environment)->GetArrayLength(environment, origin);
    jbyte* origin_bytes = (*environment)->GetByteArrayElements(environment, origin, NULL);
    if (origin_bytes == NULL) {
        free(bytes_values);
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView origin_view = {(const char*)origin_bytes, (uint64_t)origin_size};
    CNA_CnbReadLimits limits_value;
    memset(&limits_value, 0, sizeof limits_value);
    limits_value.struct_size = (uint32_t)(sizeof limits_value);
    limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    {
        jlong limitsIntegral_values[7];
        (*environment)->GetLongArrayRegion(environment, limitsIntegral, 0, 7, limitsIntegral_values);
        limits_value.max_file_size = (uint64_t)limitsIntegral_values[0];
        limits_value.max_chunk_size = (uint64_t)limitsIntegral_values[1];
        limits_value.max_total_uncompressed_size = (uint64_t)limitsIntegral_values[2];
        limits_value.max_chunk_count = (uint32_t)limitsIntegral_values[3];
        limits_value.max_string_bytes = (uint32_t)limitsIntegral_values[4];
        limits_value.max_array_element_count = (uint32_t)limitsIntegral_values[5];
        limits_value.max_chunk_alignment = (uint32_t)limitsIntegral_values[6];
    }
    CNA_CnbDocumentHandle out_document_value = 0;
    CNA_Result call_result = cna.cnb_document_parse(bytes_values, (uint64_t)bytes_size, origin_view, &limits_value, &out_document_value);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, origin, origin_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_document_element = (jlong)out_document_value;
        (*environment)->SetLongArrayRegion(
            environment, out_document, 0, 1, &out_document_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentParseFile(
    JNIEnv* environment, jclass declaring_class, jbyteArray path, jlongArray limitsIntegral, jlongArray out_document)
{
    (void)environment;
    (void)declaring_class;
    if (limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize path_size = (*environment)->GetArrayLength(environment, path);
    jbyte* path_bytes = (*environment)->GetByteArrayElements(environment, path, NULL);
    if (path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView path_view = {(const char*)path_bytes, (uint64_t)path_size};
    CNA_CnbReadLimits limits_value;
    memset(&limits_value, 0, sizeof limits_value);
    limits_value.struct_size = (uint32_t)(sizeof limits_value);
    limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    {
        jlong limitsIntegral_values[7];
        (*environment)->GetLongArrayRegion(environment, limitsIntegral, 0, 7, limitsIntegral_values);
        limits_value.max_file_size = (uint64_t)limitsIntegral_values[0];
        limits_value.max_chunk_size = (uint64_t)limitsIntegral_values[1];
        limits_value.max_total_uncompressed_size = (uint64_t)limitsIntegral_values[2];
        limits_value.max_chunk_count = (uint32_t)limitsIntegral_values[3];
        limits_value.max_string_bytes = (uint32_t)limitsIntegral_values[4];
        limits_value.max_array_element_count = (uint32_t)limitsIntegral_values[5];
        limits_value.max_chunk_alignment = (uint32_t)limitsIntegral_values[6];
    }
    CNA_CnbDocumentHandle out_document_value = 0;
    CNA_Result call_result = cna.cnb_document_parse_file(path_view, &limits_value, &out_document_value);
    (*environment)->ReleaseByteArrayElements(environment, path, path_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_document_element = (jlong)out_document_value;
        (*environment)->SetLongArrayRegion(
            environment, out_document, 0, 1, &out_document_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentReadEmbeddedTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong document, jbyteArray label, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    jsize label_size = (*environment)->GetArrayLength(environment, label);
    jbyte* label_bytes = (*environment)->GetByteArrayElements(environment, label, NULL);
    if (label_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView label_view = {(const char*)label_bytes, (uint64_t)label_size};
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_document_read_embedded_texture2d((CNA_CnbDocumentHandle)document, label_view, &out_texture_value);
    (*environment)->ReleaseByteArrayElements(environment, label, label_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentRequireAsset(
    JNIEnv* environment, jclass declaring_class, jlong document, jint expected_asset_type_id, jint max_schema_version)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_document_require_asset((CNA_CnbDocumentHandle)document, (uint32_t)expected_asset_type_id, (uint32_t)max_schema_version);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentRequireMandatoryChunksUnderstood(
    JNIEnv* environment, jclass declaring_class, jlong document, jintArray known_types)
{
    (void)environment;
    (void)declaring_class;
    jsize known_types_size = (*environment)->GetArrayLength(environment, known_types);
    jint* known_types_elements = (*environment)->GetIntArrayElements(environment, known_types, NULL);
    if (known_types_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_CnbChunkId* known_types_values = (CNA_CnbChunkId*)malloc(
        ((size_t)known_types_size + 1U) * sizeof(*known_types_values));
    if (known_types_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, known_types, known_types_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < known_types_size; ++index) {
        known_types_values[index] = (CNA_CnbChunkId)known_types_elements[index];
    }
    CNA_Result call_result = cna.cnb_document_require_mandatory_chunks_understood((CNA_CnbDocumentHandle)document, known_types_values, (uint64_t)known_types_size);
    free(known_types_values);
    (*environment)->ReleaseIntArrayElements(
        environment, known_types, known_types_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbDocumentRequireSingle(
    JNIEnv* environment, jclass declaring_class, jlong document, jint type, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_document_require_single((CNA_CnbDocumentHandle)document, (CNA_CnbChunkId)type, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeCurve(
    JNIEnv* environment, jclass declaring_class, jlong curve, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_curve((CNA_CurveHandle)curve, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeModel(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_model((CNA_CnbModelDataHandle)model, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeSong(
    JNIEnv* environment, jclass declaring_class, jbyteArray stream_reference, jbyteArray name, jint duration_milliseconds, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize stream_reference_size = (*environment)->GetArrayLength(environment, stream_reference);
    jbyte* stream_reference_bytes = (*environment)->GetByteArrayElements(environment, stream_reference, NULL);
    if (stream_reference_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView stream_reference_view = {(const char*)stream_reference_bytes, (uint64_t)stream_reference_size};
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_song(stream_reference_view, name_view, (uint32_t)duration_milliseconds, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeSoundEffect(
    JNIEnv* environment, jclass declaring_class, jlong sound, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_sound_effect((CNA_CnbSoundEffectDataHandle)sound, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeSpriteFont(
    JNIEnv* environment, jclass declaring_class, jlong font, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_sprite_font((CNA_CnbSpriteFontDataHandle)font, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong texture, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_texture2d((CNA_CnbTextureDataHandle)texture, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeTexture3d(
    JNIEnv* environment, jclass declaring_class, jlong texture, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_texture3d((CNA_CnbTextureDataHandle)texture, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeTextureCube(
    JNIEnv* environment, jclass declaring_class, jlong texture, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_texture_cube((CNA_CnbTextureDataHandle)texture, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbEncodeVideo(
    JNIEnv* environment, jclass declaring_class, jbyteArray stream_reference, jlongArray infoIntegral, jfloatArray infoFloating, jbyteArray content_name, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    if (infoIntegral == NULL || infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize stream_reference_size = (*environment)->GetArrayLength(environment, stream_reference);
    jbyte* stream_reference_bytes = (*environment)->GetByteArrayElements(environment, stream_reference, NULL);
    if (stream_reference_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView stream_reference_view = {(const char*)stream_reference_bytes, (uint64_t)stream_reference_size};
    CNA_CnbVideoInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_VIDEO_INFO_STRUCT_VERSION;
    {
        jlong infoIntegral_values[5];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 5, infoIntegral_values);
        info_value.duration_milliseconds = (uint32_t)infoIntegral_values[0];
        info_value.width = (uint32_t)infoIntegral_values[1];
        info_value.height = (uint32_t)infoIntegral_values[2];
        info_value.soundtrack_type = (CNA_VideoSoundtrackType)infoIntegral_values[3];
        info_value.reserved = (uint32_t)infoIntegral_values[4];
    }
    {
        jfloat infoFloating_values[1];
        (*environment)->GetFloatArrayRegion(environment, infoFloating, 0, 1, infoFloating_values);
        info_value.frames_per_second = (float)infoFloating_values[0];
    }
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_elements = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* destination_values = (uint8_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (uint8_t)destination_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_encode_video(stream_reference_view, &info_value, content_name_view, destination_values, (uint64_t)destination_size, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, stream_reference, stream_reference_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetAssetTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_asset_type_name_size((uint32_t)asset_type_id, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetAudioFormatNameSize(
    JNIEnv* environment, jclass declaring_class, jint format, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_audio_format_name_size((CNA_CnbAudioFormat)format, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetChunkIdStringSize(
    JNIEnv* environment, jclass declaring_class, jint id, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_chunk_id_string_size((CNA_CnbChunkId)id, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetCompressedByteCount(
    JNIEnv* environment, jclass declaring_class, jbyteArray raw, jint codec, jint level, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize raw_size = (*environment)->GetArrayLength(environment, raw);
    jbyte* raw_elements = (*environment)->GetByteArrayElements(environment, raw, NULL);
    if (raw_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* raw_values = (uint8_t*)malloc(
        ((size_t)raw_size + 1U) * sizeof(*raw_values));
    if (raw_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, raw, raw_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < raw_size; ++index) {
        raw_values[index] = (uint8_t)raw_elements[index];
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_compressed_byte_count(raw_values, (uint64_t)raw_size, (CNA_CnbCompression)codec, (int32_t)level, &out_byte_count_value);
    free(raw_values);
    (*environment)->ReleaseByteArrayElements(
        environment, raw, raw_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetCompressionNameSize(
    JNIEnv* environment, jclass declaring_class, jint codec, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_compression_name_size((CNA_CnbCompression)codec, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetLogicalNameProblemSize(
    JNIEnv* environment, jclass declaring_class, jbyteArray logical_name, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize logical_name_size = (*environment)->GetArrayLength(environment, logical_name);
    jbyte* logical_name_bytes = (*environment)->GetByteArrayElements(environment, logical_name, NULL);
    if (logical_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView logical_name_view = {(const char*)logical_name_bytes, (uint64_t)logical_name_size};
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_logical_name_problem_size(logical_name_view, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, logical_name, logical_name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetTextureFormatNameSize(
    JNIEnv* environment, jclass declaring_class, jint format, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_get_texture_format_name_size((CNA_CnbTextureFormat)format, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetTextureFormatUnitBytes(
    JNIEnv* environment, jclass declaring_class, jint format, jintArray out_unit_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_unit_bytes_value = 0;
    CNA_Result call_result = cna.cnb_get_texture_format_unit_bytes((CNA_CnbTextureFormat)format, &out_unit_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_unit_bytes_element = (jint)out_unit_bytes_value;
        (*environment)->SetIntArrayRegion(
            environment, out_unit_bytes, 0, 1, &out_unit_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbGetTextureLevelByteSize(
    JNIEnv* environment, jclass declaring_class, jint format, jint width, jint height, jint depth, jlongArray out_byte_size)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_size_value = 0;
    CNA_Result call_result = cna.cnb_get_texture_level_byte_size((CNA_CnbTextureFormat)format, (uint32_t)width, (uint32_t)height, (uint32_t)depth, &out_byte_size_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_size_element = (jlong)out_byte_size_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_size, 0, 1, &out_byte_size_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbHasMagic(
    JNIEnv* environment, jclass declaring_class, jbyteArray bytes, jbooleanArray out_has_magic)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    CNA_Bool out_has_magic_value = 0;
    CNA_Result call_result = cna.cnb_has_magic(bytes_values, (uint64_t)bytes_size, &out_has_magic_value);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_magic_element = (jboolean)out_has_magic_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_magic, 0, 1, &out_has_magic_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbImportDdsAsTextureCube(
    JNIEnv* environment, jclass declaring_class, jbyteArray dds_path, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    jsize dds_path_size = (*environment)->GetArrayLength(environment, dds_path);
    jbyte* dds_path_bytes = (*environment)->GetByteArrayElements(environment, dds_path, NULL);
    if (dds_path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView dds_path_view = {(const char*)dds_path_bytes, (uint64_t)dds_path_size};
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_import_dds_as_texture_cube(dds_path_view, &out_texture_value);
    (*environment)->ReleaseByteArrayElements(environment, dds_path, dds_path_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbImportImageAsTexture2d(
    JNIEnv* environment, jclass declaring_class, jbyteArray image_path, jbyteArray optionsBytes, jlongArray optionsIntegral, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    if (optionsBytes == NULL || optionsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize image_path_size = (*environment)->GetArrayLength(environment, image_path);
    jbyte* image_path_bytes = (*environment)->GetByteArrayElements(environment, image_path, NULL);
    if (image_path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView image_path_view = {(const char*)image_path_bytes, (uint64_t)image_path_size};
    CNA_CnbImageImportOptions options_value;
    memset(&options_value, 0, sizeof options_value);
    options_value.struct_size = (uint32_t)(sizeof options_value);
    options_value.struct_version = CNA_CNB_IMAGE_IMPORT_OPTIONS_STRUCT_VERSION;
    {
        jbyte optionsBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, optionsBytes, 0, 3, optionsBytes_values);
        options_value.color_key[0] = (uint8_t)optionsBytes_values[0];
        options_value.color_key[1] = (uint8_t)optionsBytes_values[1];
        options_value.color_key[2] = (uint8_t)optionsBytes_values[2];
    }
    {
        jlong optionsIntegral_values[1];
        (*environment)->GetLongArrayRegion(environment, optionsIntegral, 0, 1, optionsIntegral_values);
        options_value.has_color_key = (CNA_Bool)optionsIntegral_values[0];
    }
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_import_image_as_texture2d(image_path_view, &options_value, &out_texture_value);
    (*environment)->ReleaseByteArrayElements(environment, image_path, image_path_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbImportWavAsSoundEffect(
    JNIEnv* environment, jclass declaring_class, jbyteArray wav_path, jlongArray out_sound)
{
    (void)environment;
    (void)declaring_class;
    jsize wav_path_size = (*environment)->GetArrayLength(environment, wav_path);
    jbyte* wav_path_bytes = (*environment)->GetByteArrayElements(environment, wav_path, NULL);
    if (wav_path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView wav_path_view = {(const char*)wav_path_bytes, (uint64_t)wav_path_size};
    CNA_CnbSoundEffectDataHandle out_sound_value = 0;
    CNA_Result call_result = cna.cnb_import_wav_as_sound_effect(wav_path_view, &out_sound_value);
    (*environment)->ReleaseByteArrayElements(environment, wav_path, wav_path_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sound_element = (jlong)out_sound_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sound, 0, 1, &out_sound_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsBlockCompressedTextureFormat(
    JNIEnv* environment, jclass declaring_class, jint format, jbooleanArray out_block_compressed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_block_compressed_value = 0;
    CNA_Result call_result = cna.cnb_is_block_compressed_texture_format((CNA_CnbTextureFormat)format, &out_block_compressed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_block_compressed_element = (jboolean)out_block_compressed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_block_compressed, 0, 1, &out_block_compressed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsCompressionSupported(
    JNIEnv* environment, jclass declaring_class, jint codec, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.cnb_is_compression_supported((CNA_CnbCompression)codec, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsCustomAssetTypeId(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jbooleanArray out_custom)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_custom_value = 0;
    CNA_Result call_result = cna.cnb_is_custom_asset_type_id((uint32_t)asset_type_id, &out_custom_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_custom_element = (jboolean)out_custom_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_custom, 0, 1, &out_custom_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsKnownTextureFormat(
    JNIEnv* environment, jclass declaring_class, jint value, jbooleanArray out_known)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_known_value = 0;
    CNA_Result call_result = cna.cnb_is_known_texture_format((uint32_t)value, &out_known_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_known_element = (jboolean)out_known_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_known, 0, 1, &out_known_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsWellFormedChunkId(
    JNIEnv* environment, jclass declaring_class, jint id, jbooleanArray out_well_formed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_well_formed_value = 0;
    CNA_Result call_result = cna.cnb_is_well_formed_chunk_id((CNA_CnbChunkId)id, &out_well_formed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_well_formed_element = (jboolean)out_well_formed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_well_formed, 0, 1, &out_well_formed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbIsWellFormedUtf8(
    JNIEnv* environment, jclass declaring_class, jbyteArray text, jbooleanArray out_well_formed)
{
    (void)environment;
    (void)declaring_class;
    jsize text_size = (*environment)->GetArrayLength(environment, text);
    jbyte* text_bytes = (*environment)->GetByteArrayElements(environment, text, NULL);
    if (text_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView text_view = {(const char*)text_bytes, (uint64_t)text_size};
    CNA_Bool out_well_formed_value = 0;
    CNA_Result call_result = cna.cnb_is_well_formed_utf8(text_view, &out_well_formed_value);
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_well_formed_element = (jboolean)out_well_formed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_well_formed, 0, 1, &out_well_formed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbLoaderRegistryCopyRegisteredTypeName(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_loader_registry_copy_registered_type_name((uint32_t)asset_type_id, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbLoaderRegistryGetRegisteredTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_loader_registry_get_registered_type_name_size((uint32_t)asset_type_id, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbLoaderRegistryIsRegistered(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jbooleanArray out_registered)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_registered_value = 0;
    CNA_Result call_result = cna.cnb_loader_registry_is_registered((uint32_t)asset_type_id, &out_registered_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_registered_element = (jboolean)out_registered_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_registered, 0, 1, &out_registered_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbLoaderRegistryRegisterBuiltins(
    JNIEnv* environment, jclass declaring_class)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_loader_registry_register_builtins();
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbMakeChunkId(
    JNIEnv* environment, jclass declaring_class, jbyte a, jbyte b, jbyte c, jbyte d, jintArray out_id)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbChunkId out_id_value = 0;
    CNA_Result call_result = cna.cnb_make_chunk_id((uint8_t)a, (uint8_t)b, (uint8_t)c, (uint8_t)d, &out_id_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_id_element = (jint)out_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_id, 0, 1, &out_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddBone(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jint parent, jfloatArray transform, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    if (transform == NULL || (*environment)->GetArrayLength(environment, transform) != (jsize)16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    float* transform_values = NULL;
    jfloat* transform_elements = NULL;
    if (transform != NULL) {
        jsize transform_size = (*environment)->GetArrayLength(environment, transform);
        transform_elements = (*environment)->GetFloatArrayElements(environment, transform, NULL);
        if (transform_elements == NULL) {
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        transform_values = (float*)malloc(
            ((size_t)transform_size + 1U) * sizeof(*transform_values));
        if (transform_values == NULL) {
            (*environment)->ReleaseFloatArrayElements(
                environment, transform, transform_elements, JNI_ABORT);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        for (jsize index = 0; index < transform_size; ++index) {
            transform_values[index] = (float)transform_elements[index];
        }
    }
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_bone((CNA_CnbModelDataHandle)model, name_view, (int32_t)parent, transform_values, &out_index_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (transform_elements != NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, transform, transform_elements, JNI_ABORT);
    }
    free(transform_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddLight(
    JNIEnv* environment, jclass declaring_class, jlong model, jfloatArray lightFloating, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    if (lightFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelLight light_value;
    memset(&light_value, 0, sizeof light_value);
    {
        jfloat lightFloating_values[6];
        (*environment)->GetFloatArrayRegion(environment, lightFloating, 0, 6, lightFloating_values);
        light_value.direction[0] = (float)lightFloating_values[0];
        light_value.direction[1] = (float)lightFloating_values[1];
        light_value.direction[2] = (float)lightFloating_values[2];
        light_value.diffuse_color[0] = (float)lightFloating_values[3];
        light_value.diffuse_color[1] = (float)lightFloating_values[4];
        light_value.diffuse_color[2] = (float)lightFloating_values[5];
    }
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_light((CNA_CnbModelDataHandle)model, &light_value, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddMesh(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray name, jint parent_bone, jintArray part_indices, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize part_indices_size = (*environment)->GetArrayLength(environment, part_indices);
    jint* part_indices_elements = (*environment)->GetIntArrayElements(environment, part_indices, NULL);
    if (part_indices_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint32_t* part_indices_values = (uint32_t*)malloc(
        ((size_t)part_indices_size + 1U) * sizeof(*part_indices_values));
    if (part_indices_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, part_indices, part_indices_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < part_indices_size; ++index) {
        part_indices_values[index] = (uint32_t)part_indices_elements[index];
    }
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_mesh((CNA_CnbModelDataHandle)model, name_view, (int32_t)parent_bone, part_indices_values, (uint64_t)part_indices_size, &out_index_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(part_indices_values);
    (*environment)->ReleaseIntArrayElements(
        environment, part_indices, part_indices_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddMorphTarget(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_morph_target((CNA_CnbModelDataHandle)model, (uint64_t)part, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddMorphWeightKey(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jdouble time_seconds, jfloatArray weights, jfloatArray in_tangents, jfloatArray out_tangents, jlongArray out_index)
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
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_morph_weight_key((CNA_CnbModelDataHandle)model, (uint64_t)part, (double)time_seconds, weights_values, (uint64_t)weights_size, in_tangents_values, (uint64_t)in_tangents_size, out_tangents_values, (uint64_t)out_tangents_size, &out_index_value);
    free(weights_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, weights, weights_elements, JNI_ABORT);
    free(in_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, in_tangents, in_tangents_elements, JNI_ABORT);
    free(out_tangents_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, out_tangents, out_tangents_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelAddPart(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray infoBytes, jlongArray infoIntegral, jbyteArray name, jbyteArray external_effect, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    if (infoBytes == NULL || infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelPartInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    {
        jbyte infoBytes_values[2];
        (*environment)->GetByteArrayRegion(environment, infoBytes, 0, 2, infoBytes_values);
        info_value.reserved[0] = (uint8_t)infoBytes_values[0];
        info_value.reserved[1] = (uint8_t)infoBytes_values[1];
    }
    {
        jlong infoIntegral_values[9];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 9, infoIntegral_values);
        info_value.vertex_stride = (uint32_t)infoIntegral_values[0];
        info_value.vertex_count = (uint32_t)infoIntegral_values[1];
        info_value.index_count = (uint32_t)infoIntegral_values[2];
        info_value.index_element_size = (uint32_t)infoIntegral_values[3];
        info_value.primitive_topology = (uint32_t)infoIntegral_values[4];
        info_value.primitive_count = (uint32_t)infoIntegral_values[5];
        info_value.effect_kind = (CNA_CnbEffectKind)infoIntegral_values[6];
        info_value.vertex_color_enabled = (CNA_Bool)infoIntegral_values[7];
        info_value.unlit = (CNA_Bool)infoIntegral_values[8];
    }
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize external_effect_size = (*environment)->GetArrayLength(environment, external_effect);
    jbyte* external_effect_bytes = (*environment)->GetByteArrayElements(environment, external_effect, NULL);
    if (external_effect_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView external_effect_view = {(const char*)external_effect_bytes, (uint64_t)external_effect_size};
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_model_add_part((CNA_CnbModelDataHandle)model, &info_value, name_view, external_effect_view, &out_index_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, external_effect, external_effect_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelClearMorph(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_clear_morph((CNA_CnbModelDataHandle)model, (uint64_t)part);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelClearSkeleton(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_clear_skeleton((CNA_CnbModelDataHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyAnimationKeyframes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlong track, jfloatArray destinationFloating, jdoubleArray destinationDoubles, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 10;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 10 ||
        (*environment)->GetArrayLength(environment, destinationDoubles) != destination_count * 1) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT* destination_values = (CNA_KeyframeEXT*)calloc(
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
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetDoubleArrayRegion(environment, destinationDoubles, 0, destinationDoubles_length, destinationDoubles_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].time_seconds = (double)destinationDoubles_values[element * 1 + 0];
        }
        free(destinationDoubles_values);
    }
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_animation_keyframes((CNA_CnbModelDataHandle)model, (uint64_t)index, (uint64_t)track, destination_values, (uint64_t)destination_count, &out_keyframe_count_value);
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
        jlong out_keyframe_count_element = (jlong)out_keyframe_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keyframe_count, 0, 1, &out_keyframe_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyAnimationName(
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
    CNA_Result call_result = cna.cnb_model_copy_animation_name((CNA_CnbModelDataHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyBoneName(
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
    CNA_Result call_result = cna.cnb_model_copy_bone_name((CNA_CnbModelDataHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMaterialTexture(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jint slot, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_model_copy_material_texture((CNA_CnbModelDataHandle)model, (uint64_t)part, (CNA_CnbMaterialTextureSlot)slot, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMeshName(
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
    CNA_Result call_result = cna.cnb_model_copy_mesh_name((CNA_CnbModelDataHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMeshPartIndices(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jintArray destination, jlongArray out_index_count)
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
    CNA_Result call_result = cna.cnb_model_copy_mesh_part_indices((CNA_CnbModelDataHandle)model, (uint64_t)index, destination_values, (uint64_t)destination_size, &out_index_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMorphTargetDeltas(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong target, jint stream, jfloatArray destination, jlongArray out_value_count)
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
    uint64_t out_value_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_morph_target_deltas((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)target, (CNA_CnbMorphDeltaStream)stream, destination_values, (uint64_t)destination_size, &out_value_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jfloat)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_count_element = (jlong)out_value_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value_count, 0, 1, &out_value_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMorphWeightKeyValues(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong key, jint stream, jfloatArray destination, jlongArray out_value_count)
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
    uint64_t out_value_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_morph_weight_key_values((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)key, (CNA_CnbMorphKeyStream)stream, destination_values, (uint64_t)destination_size, &out_value_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jfloat)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_count_element = (jlong)out_value_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value_count, 0, 1, &out_value_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyMorphWeights(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jfloatArray destination, jlongArray out_value_count)
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
    uint64_t out_value_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_morph_weights((CNA_CnbModelDataHandle)model, (uint64_t)part, destination_values, (uint64_t)destination_size, &out_value_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jfloat)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_count_element = (jlong)out_value_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value_count, 0, 1, &out_value_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyPartExternalEffect(
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
    CNA_Result call_result = cna.cnb_model_copy_part_external_effect((CNA_CnbModelDataHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyPartIndexBytes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_model_copy_part_index_bytes((CNA_CnbModelDataHandle)model, (uint64_t)index, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyPartName(
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
    CNA_Result call_result = cna.cnb_model_copy_part_name((CNA_CnbModelDataHandle)model, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopyPartVertexBytes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_model_copy_part_vertex_bytes((CNA_CnbModelDataHandle)model, (uint64_t)index, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopySkeletonHierarchy(
    JNIEnv* environment, jclass declaring_class, jlong model, jintArray destination, jlongArray out_index_count)
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
    uint64_t out_index_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_skeleton_hierarchy((CNA_CnbModelDataHandle)model, destination_values, (uint64_t)destination_size, &out_index_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCopySkeletonMatrices(
    JNIEnv* environment, jclass declaring_class, jlong model, jint set, jfloatArray destination, jlongArray out_value_count)
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
    uint64_t out_value_count_value = 0;
    CNA_Result call_result = cna.cnb_model_copy_skeleton_matrices((CNA_CnbModelDataHandle)model, (CNA_CnbSkeletonMatrixSet)set, destination_values, (uint64_t)destination_size, &out_value_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jfloat)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_count_element = (jlong)out_value_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value_count, 0, 1, &out_value_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbModelDataHandle out_model_value = 0;
    CNA_Result call_result = cna.cnb_model_create(&out_model_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelDestroy(
    JNIEnv* environment, jclass declaring_class, jlong model)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_destroy((CNA_CnbModelDataHandle)model);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjCopyAbsorbedFile(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_copy_absorbed_file((CNA_CnbModelFromCnjHandle)result, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjCopyExternalReference(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_copy_external_reference((CNA_CnbModelFromCnjHandle)result, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjDestroy(
    JNIEnv* environment, jclass declaring_class, jlong result)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_from_cnj_destroy((CNA_CnbModelFromCnjHandle)result);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjGetAbsorbedFileCount(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_get_absorbed_file_count((CNA_CnbModelFromCnjHandle)result, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjGetAbsorbedFileSize(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_get_absorbed_file_size((CNA_CnbModelFromCnjHandle)result, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjGetExternalReferenceCount(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_get_external_reference_count((CNA_CnbModelFromCnjHandle)result, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjGetExternalReferenceSize(
    JNIEnv* environment, jclass declaring_class, jlong result, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_get_external_reference_size((CNA_CnbModelFromCnjHandle)result, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelFromCnjTakeModel(
    JNIEnv* environment, jclass declaring_class, jlong result, jlongArray out_model)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbModelDataHandle out_model_value = 0;
    CNA_Result call_result = cna.cnb_model_from_cnj_take_model((CNA_CnbModelFromCnjHandle)result, &out_model_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_model_element = (jlong)out_model_value;
        (*environment)->SetLongArrayRegion(
            environment, out_model, 0, 1, &out_model_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetAnimation(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jdoubleArray out_duration_seconds, jlongArray out_track_count, jintArray out_target_space)
{
    (void)environment;
    (void)declaring_class;
    double out_duration_seconds_value = 0;
    uint64_t out_track_count_value = 0;
    CNA_ClipTargetSpaceEXT out_target_space_value = 0;
    CNA_Result call_result = cna.cnb_model_get_animation((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_duration_seconds_value, &out_track_count_value, &out_target_space_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetAnimationNameSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_animation_name_size((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetAnimationTrack(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlong track, jintArray out_bone_index, jlongArray out_keyframe_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_bone_index_value = 0;
    uint64_t out_keyframe_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_animation_track((CNA_CnbModelDataHandle)model, (uint64_t)index, (uint64_t)track, &out_bone_index_value, &out_keyframe_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetBone(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_boneIntegral, jfloatArray out_boneFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_boneIntegral == NULL || out_boneFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelBone out_bone_value;
    memset(&out_bone_value, 0, sizeof out_bone_value);
    out_bone_value.struct_size = (uint32_t)(sizeof out_bone_value);
    out_bone_value.struct_version = CNA_CNB_MODEL_BONE_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_bone((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_bone_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_boneIntegral_values[] = {
            (jlong)out_bone_value.parent,
            (jlong)out_bone_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_boneIntegral, 0,
            (jsize)(sizeof out_boneIntegral_values / sizeof out_boneIntegral_values[0]),
            out_boneIntegral_values);
        jfloat out_boneFloating_values[] = {
            (jfloat)out_bone_value.transform[0],
            (jfloat)out_bone_value.transform[1],
            (jfloat)out_bone_value.transform[2],
            (jfloat)out_bone_value.transform[3],
            (jfloat)out_bone_value.transform[4],
            (jfloat)out_bone_value.transform[5],
            (jfloat)out_bone_value.transform[6],
            (jfloat)out_bone_value.transform[7],
            (jfloat)out_bone_value.transform[8],
            (jfloat)out_bone_value.transform[9],
            (jfloat)out_bone_value.transform[10],
            (jfloat)out_bone_value.transform[11],
            (jfloat)out_bone_value.transform[12],
            (jfloat)out_bone_value.transform[13],
            (jfloat)out_bone_value.transform[14],
            (jfloat)out_bone_value.transform[15]
        };
        (*environment)->SetFloatArrayRegion(environment, out_boneFloating, 0,
            (jsize)(sizeof out_boneFloating_values / sizeof out_boneFloating_values[0]),
            out_boneFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetBoneNameSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_bone_name_size((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong model, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_info((CNA_CnbModelDataHandle)model, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.bone_count,
            (jlong)out_info_value.part_count,
            (jlong)out_info_value.mesh_count,
            (jlong)out_info_value.animation_count,
            (jlong)out_info_value.light_count,
            (jlong)out_info_value.has_skeleton,
            (jlong)out_info_value.applies_gltf_lighting_policy,
            (jlong)out_info_value.has_bone_hierarchy,
            (jlong)out_info_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetLight(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jfloatArray out_lightFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_lightFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelLight out_light_value;
    memset(&out_light_value, 0, sizeof out_light_value);
    CNA_Result call_result = cna.cnb_model_get_light((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_light_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_lightFloating_values[] = {
            (jfloat)out_light_value.direction[0],
            (jfloat)out_light_value.direction[1],
            (jfloat)out_light_value.direction[2],
            (jfloat)out_light_value.diffuse_color[0],
            (jfloat)out_light_value.diffuse_color[1],
            (jfloat)out_light_value.diffuse_color[2]
        };
        (*environment)->SetFloatArrayRegion(environment, out_lightFloating, 0,
            (jsize)(sizeof out_lightFloating_values / sizeof out_lightFloating_values[0]),
            out_lightFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMaterial(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jbyteArray out_infoBytes, jlongArray out_infoIntegral, jfloatArray out_infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL || out_infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMaterialInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MATERIAL_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_material((CNA_CnbModelDataHandle)model, (uint64_t)part, &out_info_value);
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
            (jlong)out_info_value.alpha_mode,
            (jlong)out_info_value.double_sided
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jfloat out_infoFloating_values[] = {
            (jfloat)out_info_value.base_color_factor[0],
            (jfloat)out_info_value.base_color_factor[1],
            (jfloat)out_info_value.base_color_factor[2],
            (jfloat)out_info_value.base_color_factor[3],
            (jfloat)out_info_value.emissive_factor[0],
            (jfloat)out_info_value.emissive_factor[1],
            (jfloat)out_info_value.emissive_factor[2],
            (jfloat)out_info_value.specular_color_factor[0],
            (jfloat)out_info_value.specular_color_factor[1],
            (jfloat)out_info_value.specular_color_factor[2],
            (jfloat)out_info_value.metallic_factor,
            (jfloat)out_info_value.roughness_factor,
            (jfloat)out_info_value.ior,
            (jfloat)out_info_value.specular_factor,
            (jfloat)out_info_value.normal_scale,
            (jfloat)out_info_value.occlusion_strength,
            (jfloat)out_info_value.alpha_cutoff
        };
        (*environment)->SetFloatArrayRegion(environment, out_infoFloating, 0,
            (jsize)(sizeof out_infoFloating_values / sizeof out_infoFloating_values[0]),
            out_infoFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMaterialSampler(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jbyteArray out_samplerBytes, jlongArray out_samplerIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_samplerBytes == NULL || out_samplerIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSamplerState out_sampler_value;
    memset(&out_sampler_value, 0, sizeof out_sampler_value);
    CNA_Result call_result = cna.cnb_model_get_material_sampler((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, &out_sampler_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_samplerBytes_values[] = {
            (jbyte)out_sampler_value.reserved[0],
            (jbyte)out_sampler_value.reserved[1],
            (jbyte)out_sampler_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_samplerBytes, 0,
            (jsize)(sizeof out_samplerBytes_values / sizeof out_samplerBytes_values[0]),
            out_samplerBytes_values);
        jlong out_samplerIntegral_values[] = {
            (jlong)out_sampler_value.filter,
            (jlong)out_sampler_value.address_u,
            (jlong)out_sampler_value.address_v,
            (jlong)out_sampler_value.declared
        };
        (*environment)->SetLongArrayRegion(environment, out_samplerIntegral, 0,
            (jsize)(sizeof out_samplerIntegral_values / sizeof out_samplerIntegral_values[0]),
            out_samplerIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMaterialTextureCoordinateSet(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jbyteArray out_set)
{
    (void)environment;
    (void)declaring_class;
    uint8_t out_set_value = 0;
    CNA_Result call_result = cna.cnb_model_get_material_texture_coordinate_set((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, &out_set_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_set_element = (jbyte)out_set_value;
        (*environment)->SetByteArrayRegion(
            environment, out_set, 0, 1, &out_set_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMaterialTextureSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jint slot, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_material_texture_size((CNA_CnbModelDataHandle)model, (uint64_t)part, (CNA_CnbMaterialTextureSlot)slot, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMaterialTextureTransform(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jfloatArray out_transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbTextureTransform out_transform_value;
    memset(&out_transform_value, 0, sizeof out_transform_value);
    CNA_Result call_result = cna.cnb_model_get_material_texture_transform((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, &out_transform_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_transformFloating_values[] = {
            (jfloat)out_transform_value.offset_x,
            (jfloat)out_transform_value.offset_y,
            (jfloat)out_transform_value.scale_x,
            (jfloat)out_transform_value.scale_y,
            (jfloat)out_transform_value.rotation
        };
        (*environment)->SetFloatArrayRegion(environment, out_transformFloating, 0,
            (jsize)(sizeof out_transformFloating_values / sizeof out_transformFloating_values[0]),
            out_transformFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMesh(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMeshInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MESH_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_mesh((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.parent_bone,
            (jlong)out_info_value.reserved,
            (jlong)out_info_value.part_index_count
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMeshNameSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_mesh_name_size((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMorph(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMorphInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MORPH_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_morph((CNA_CnbModelDataHandle)model, (uint64_t)part, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved2[0],
            (jbyte)out_info_value.reserved2[1],
            (jbyte)out_info_value.reserved2[2],
            (jbyte)out_info_value.reserved2[3],
            (jbyte)out_info_value.reserved2[4]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.vertex_count,
            (jlong)out_info_value.reserved,
            (jlong)out_info_value.target_count,
            (jlong)out_info_value.weight_count,
            (jlong)out_info_value.weight_track_key_count,
            (jlong)out_info_value.recompute_flat_normals,
            (jlong)out_info_value.weight_track_step_interpolation,
            (jlong)out_info_value.weight_track_cubic_spline
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetMorphWeightKey(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong key, jlongArray out_infoIntegral, jdoubleArray out_infoDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL || out_infoDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMorphWeightKeyInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MORPH_WEIGHT_KEY_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_morph_weight_key((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)key, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.weight_count,
            (jlong)out_info_value.in_tangent_count,
            (jlong)out_info_value.out_tangent_count
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jdouble out_infoDoubles_values[] = {
            (jdouble)out_info_value.time_seconds
        };
        (*environment)->SetDoubleArrayRegion(environment, out_infoDoubles, 0,
            (jsize)(sizeof out_infoDoubles_values / sizeof out_infoDoubles_values[0]),
            out_infoDoubles_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetPart(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelPartInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_part((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.vertex_stride,
            (jlong)out_info_value.vertex_count,
            (jlong)out_info_value.index_count,
            (jlong)out_info_value.index_element_size,
            (jlong)out_info_value.primitive_topology,
            (jlong)out_info_value.primitive_count,
            (jlong)out_info_value.effect_kind,
            (jlong)out_info_value.vertex_color_enabled,
            (jlong)out_info_value.unlit
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetPartExternalEffectSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_part_external_effect_size((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetPartNameSize(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_model_get_part_name_size((CNA_CnbModelDataHandle)model, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelGetSkeleton(
    JNIEnv* environment, jclass declaring_class, jlong model, jbyteArray out_infoBytes, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSkeletonInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_SKELETON_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_model_get_skeleton((CNA_CnbModelDataHandle)model, &out_info_value);
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
            (jlong)out_info_value.joint_count,
            (jlong)out_info_value.has_root_prefix
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelHasMorph(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jbooleanArray out_present)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_present_value = 0;
    CNA_Result call_result = cna.cnb_model_has_morph((CNA_CnbModelDataHandle)model, (uint64_t)part, &out_present_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_present_element = (jboolean)out_present_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_present, 0, 1, &out_present_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetFlags(
    JNIEnv* environment, jclass declaring_class, jlong model, jboolean applies_gltf_lighting_policy, jboolean has_bone_hierarchy)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_set_flags((CNA_CnbModelDataHandle)model, (CNA_Bool)applies_gltf_lighting_policy, (CNA_Bool)has_bone_hierarchy);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMaterial(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jbyteArray infoBytes, jlongArray infoIntegral, jfloatArray infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (infoBytes == NULL || infoIntegral == NULL || infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMaterialInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_MATERIAL_INFO_STRUCT_VERSION;
    {
        jbyte infoBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, infoBytes, 0, 3, infoBytes_values);
        info_value.reserved[0] = (uint8_t)infoBytes_values[0];
        info_value.reserved[1] = (uint8_t)infoBytes_values[1];
        info_value.reserved[2] = (uint8_t)infoBytes_values[2];
    }
    {
        jlong infoIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 2, infoIntegral_values);
        info_value.alpha_mode = (uint32_t)infoIntegral_values[0];
        info_value.double_sided = (CNA_Bool)infoIntegral_values[1];
    }
    {
        jfloat infoFloating_values[17];
        (*environment)->GetFloatArrayRegion(environment, infoFloating, 0, 17, infoFloating_values);
        info_value.base_color_factor[0] = (float)infoFloating_values[0];
        info_value.base_color_factor[1] = (float)infoFloating_values[1];
        info_value.base_color_factor[2] = (float)infoFloating_values[2];
        info_value.base_color_factor[3] = (float)infoFloating_values[3];
        info_value.emissive_factor[0] = (float)infoFloating_values[4];
        info_value.emissive_factor[1] = (float)infoFloating_values[5];
        info_value.emissive_factor[2] = (float)infoFloating_values[6];
        info_value.specular_color_factor[0] = (float)infoFloating_values[7];
        info_value.specular_color_factor[1] = (float)infoFloating_values[8];
        info_value.specular_color_factor[2] = (float)infoFloating_values[9];
        info_value.metallic_factor = (float)infoFloating_values[10];
        info_value.roughness_factor = (float)infoFloating_values[11];
        info_value.ior = (float)infoFloating_values[12];
        info_value.specular_factor = (float)infoFloating_values[13];
        info_value.normal_scale = (float)infoFloating_values[14];
        info_value.occlusion_strength = (float)infoFloating_values[15];
        info_value.alpha_cutoff = (float)infoFloating_values[16];
    }
    CNA_Result call_result = cna.cnb_model_set_material((CNA_CnbModelDataHandle)model, (uint64_t)part, &info_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMaterialSampler(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jbyteArray samplerBytes, jlongArray samplerIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (samplerBytes == NULL || samplerIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSamplerState sampler_value;
    memset(&sampler_value, 0, sizeof sampler_value);
    {
        jbyte samplerBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, samplerBytes, 0, 3, samplerBytes_values);
        sampler_value.reserved[0] = (uint8_t)samplerBytes_values[0];
        sampler_value.reserved[1] = (uint8_t)samplerBytes_values[1];
        sampler_value.reserved[2] = (uint8_t)samplerBytes_values[2];
    }
    {
        jlong samplerIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, samplerIntegral, 0, 4, samplerIntegral_values);
        sampler_value.filter = (uint32_t)samplerIntegral_values[0];
        sampler_value.address_u = (uint32_t)samplerIntegral_values[1];
        sampler_value.address_v = (uint32_t)samplerIntegral_values[2];
        sampler_value.declared = (CNA_Bool)samplerIntegral_values[3];
    }
    CNA_Result call_result = cna.cnb_model_set_material_sampler((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, &sampler_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMaterialTexture(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jint slot, jbyteArray asset_name)
{
    (void)environment;
    (void)declaring_class;
    jsize asset_name_size = (*environment)->GetArrayLength(environment, asset_name);
    jbyte* asset_name_bytes = (*environment)->GetByteArrayElements(environment, asset_name, NULL);
    if (asset_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView asset_name_view = {(const char*)asset_name_bytes, (uint64_t)asset_name_size};
    CNA_Result call_result = cna.cnb_model_set_material_texture((CNA_CnbModelDataHandle)model, (uint64_t)part, (CNA_CnbMaterialTextureSlot)slot, asset_name_view);
    (*environment)->ReleaseByteArrayElements(environment, asset_name, asset_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMaterialTextureCoordinateSet(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jbyte coordinate_set)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_model_set_material_texture_coordinate_set((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, (uint8_t)coordinate_set);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMaterialTextureTransform(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong slot, jfloatArray transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbTextureTransform transform_value;
    memset(&transform_value, 0, sizeof transform_value);
    {
        jfloat transformFloating_values[5];
        (*environment)->GetFloatArrayRegion(environment, transformFloating, 0, 5, transformFloating_values);
        transform_value.offset_x = (float)transformFloating_values[0];
        transform_value.offset_y = (float)transformFloating_values[1];
        transform_value.scale_x = (float)transformFloating_values[2];
        transform_value.scale_y = (float)transformFloating_values[3];
        transform_value.rotation = (float)transformFloating_values[4];
    }
    CNA_Result call_result = cna.cnb_model_set_material_texture_transform((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)slot, &transform_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMorph(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jbyteArray infoBytes, jlongArray infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (infoBytes == NULL || infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbMorphInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_MORPH_INFO_STRUCT_VERSION;
    {
        jbyte infoBytes_values[5];
        (*environment)->GetByteArrayRegion(environment, infoBytes, 0, 5, infoBytes_values);
        info_value.reserved2[0] = (uint8_t)infoBytes_values[0];
        info_value.reserved2[1] = (uint8_t)infoBytes_values[1];
        info_value.reserved2[2] = (uint8_t)infoBytes_values[2];
        info_value.reserved2[3] = (uint8_t)infoBytes_values[3];
        info_value.reserved2[4] = (uint8_t)infoBytes_values[4];
    }
    {
        jlong infoIntegral_values[8];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 8, infoIntegral_values);
        info_value.vertex_count = (uint32_t)infoIntegral_values[0];
        info_value.reserved = (uint32_t)infoIntegral_values[1];
        info_value.target_count = (uint64_t)infoIntegral_values[2];
        info_value.weight_count = (uint64_t)infoIntegral_values[3];
        info_value.weight_track_key_count = (uint64_t)infoIntegral_values[4];
        info_value.recompute_flat_normals = (CNA_Bool)infoIntegral_values[5];
        info_value.weight_track_step_interpolation = (CNA_Bool)infoIntegral_values[6];
        info_value.weight_track_cubic_spline = (CNA_Bool)infoIntegral_values[7];
    }
    CNA_Result call_result = cna.cnb_model_set_morph((CNA_CnbModelDataHandle)model, (uint64_t)part, &info_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMorphTargetDeltas(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jlong target, jint stream, jfloatArray values)
{
    (void)environment;
    (void)declaring_class;
    jsize values_size = (*environment)->GetArrayLength(environment, values);
    jfloat* values_elements = (*environment)->GetFloatArrayElements(environment, values, NULL);
    if (values_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* values_values = (float*)malloc(
        ((size_t)values_size + 1U) * sizeof(*values_values));
    if (values_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, values, values_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < values_size; ++index) {
        values_values[index] = (float)values_elements[index];
    }
    CNA_Result call_result = cna.cnb_model_set_morph_target_deltas((CNA_CnbModelDataHandle)model, (uint64_t)part, (uint64_t)target, (CNA_CnbMorphDeltaStream)stream, values_values, (uint64_t)values_size);
    free(values_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, values, values_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetMorphWeights(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong part, jfloatArray values)
{
    (void)environment;
    (void)declaring_class;
    jsize values_size = (*environment)->GetArrayLength(environment, values);
    jfloat* values_elements = (*environment)->GetFloatArrayElements(environment, values, NULL);
    if (values_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* values_values = (float*)malloc(
        ((size_t)values_size + 1U) * sizeof(*values_values));
    if (values_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, values, values_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < values_size; ++index) {
        values_values[index] = (float)values_elements[index];
    }
    CNA_Result call_result = cna.cnb_model_set_morph_weights((CNA_CnbModelDataHandle)model, (uint64_t)part, values_values, (uint64_t)values_size);
    free(values_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, values, values_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetPart(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray infoBytes, jlongArray infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (infoBytes == NULL || infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbModelPartInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    {
        jbyte infoBytes_values[2];
        (*environment)->GetByteArrayRegion(environment, infoBytes, 0, 2, infoBytes_values);
        info_value.reserved[0] = (uint8_t)infoBytes_values[0];
        info_value.reserved[1] = (uint8_t)infoBytes_values[1];
    }
    {
        jlong infoIntegral_values[9];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 9, infoIntegral_values);
        info_value.vertex_stride = (uint32_t)infoIntegral_values[0];
        info_value.vertex_count = (uint32_t)infoIntegral_values[1];
        info_value.index_count = (uint32_t)infoIntegral_values[2];
        info_value.index_element_size = (uint32_t)infoIntegral_values[3];
        info_value.primitive_topology = (uint32_t)infoIntegral_values[4];
        info_value.primitive_count = (uint32_t)infoIntegral_values[5];
        info_value.effect_kind = (CNA_CnbEffectKind)infoIntegral_values[6];
        info_value.vertex_color_enabled = (CNA_Bool)infoIntegral_values[7];
        info_value.unlit = (CNA_Bool)infoIntegral_values[8];
    }
    CNA_Result call_result = cna.cnb_model_set_part((CNA_CnbModelDataHandle)model, (uint64_t)index, &info_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetPartIndexBytes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    CNA_Result call_result = cna.cnb_model_set_part_index_bytes((CNA_CnbModelDataHandle)model, (uint64_t)index, bytes_values, (uint64_t)bytes_size);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetPartVertexBytes(
    JNIEnv* environment, jclass declaring_class, jlong model, jlong index, jbyteArray bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    CNA_Result call_result = cna.cnb_model_set_part_vertex_bytes((CNA_CnbModelDataHandle)model, (uint64_t)index, bytes_values, (uint64_t)bytes_size);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbModelSetSkeleton(
    JNIEnv* environment, jclass declaring_class, jlong model, jintArray hierarchy, jfloatArray bind_pose, jfloatArray inverse_bind_pose, jfloatArray root_prefix)
{
    (void)environment;
    (void)declaring_class;
    if (bind_pose == NULL || (*environment)->GetArrayLength(environment, bind_pose) != (jsize)(16 * (*environment)->GetArrayLength(environment, hierarchy))) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (inverse_bind_pose == NULL || (*environment)->GetArrayLength(environment, inverse_bind_pose) != (jsize)(16 * (*environment)->GetArrayLength(environment, hierarchy))) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (root_prefix != NULL && (*environment)->GetArrayLength(environment, root_prefix) != (jsize)(16 * (*environment)->GetArrayLength(environment, hierarchy))) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize hierarchy_size = (*environment)->GetArrayLength(environment, hierarchy);
    jint* hierarchy_elements = (*environment)->GetIntArrayElements(environment, hierarchy, NULL);
    if (hierarchy_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    int32_t* hierarchy_values = (int32_t*)malloc(
        ((size_t)hierarchy_size + 1U) * sizeof(*hierarchy_values));
    if (hierarchy_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, hierarchy, hierarchy_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < hierarchy_size; ++index) {
        hierarchy_values[index] = (int32_t)hierarchy_elements[index];
    }
    float* bind_pose_values = NULL;
    jfloat* bind_pose_elements = NULL;
    if (bind_pose != NULL) {
        jsize bind_pose_size = (*environment)->GetArrayLength(environment, bind_pose);
        bind_pose_elements = (*environment)->GetFloatArrayElements(environment, bind_pose, NULL);
        if (bind_pose_elements == NULL) {
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        bind_pose_values = (float*)malloc(
            ((size_t)bind_pose_size + 1U) * sizeof(*bind_pose_values));
        if (bind_pose_values == NULL) {
            (*environment)->ReleaseFloatArrayElements(
                environment, bind_pose, bind_pose_elements, JNI_ABORT);
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        for (jsize index = 0; index < bind_pose_size; ++index) {
            bind_pose_values[index] = (float)bind_pose_elements[index];
        }
    }
    float* inverse_bind_pose_values = NULL;
    jfloat* inverse_bind_pose_elements = NULL;
    if (inverse_bind_pose != NULL) {
        jsize inverse_bind_pose_size = (*environment)->GetArrayLength(environment, inverse_bind_pose);
        inverse_bind_pose_elements = (*environment)->GetFloatArrayElements(environment, inverse_bind_pose, NULL);
        if (inverse_bind_pose_elements == NULL) {
            if (bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, bind_pose, bind_pose_elements, JNI_ABORT);
            }
            free(bind_pose_values);
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        inverse_bind_pose_values = (float*)malloc(
            ((size_t)inverse_bind_pose_size + 1U) * sizeof(*inverse_bind_pose_values));
        if (inverse_bind_pose_values == NULL) {
            (*environment)->ReleaseFloatArrayElements(
                environment, inverse_bind_pose, inverse_bind_pose_elements, JNI_ABORT);
            if (bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, bind_pose, bind_pose_elements, JNI_ABORT);
            }
            free(bind_pose_values);
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        for (jsize index = 0; index < inverse_bind_pose_size; ++index) {
            inverse_bind_pose_values[index] = (float)inverse_bind_pose_elements[index];
        }
    }
    float* root_prefix_values = NULL;
    jfloat* root_prefix_elements = NULL;
    if (root_prefix != NULL) {
        jsize root_prefix_size = (*environment)->GetArrayLength(environment, root_prefix);
        root_prefix_elements = (*environment)->GetFloatArrayElements(environment, root_prefix, NULL);
        if (root_prefix_elements == NULL) {
            if (inverse_bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, inverse_bind_pose, inverse_bind_pose_elements, JNI_ABORT);
            }
            free(inverse_bind_pose_values);
            if (bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, bind_pose, bind_pose_elements, JNI_ABORT);
            }
            free(bind_pose_values);
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        root_prefix_values = (float*)malloc(
            ((size_t)root_prefix_size + 1U) * sizeof(*root_prefix_values));
        if (root_prefix_values == NULL) {
            (*environment)->ReleaseFloatArrayElements(
                environment, root_prefix, root_prefix_elements, JNI_ABORT);
            if (inverse_bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, inverse_bind_pose, inverse_bind_pose_elements, JNI_ABORT);
            }
            free(inverse_bind_pose_values);
            if (bind_pose_elements != NULL) {
                (*environment)->ReleaseFloatArrayElements(
                    environment, bind_pose, bind_pose_elements, JNI_ABORT);
            }
            free(bind_pose_values);
            free(hierarchy_values);
            (*environment)->ReleaseIntArrayElements(
                environment, hierarchy, hierarchy_elements, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        for (jsize index = 0; index < root_prefix_size; ++index) {
            root_prefix_values[index] = (float)root_prefix_elements[index];
        }
    }
    CNA_Result call_result = cna.cnb_model_set_skeleton((CNA_CnbModelDataHandle)model, hierarchy_values, (uint64_t)hierarchy_size, bind_pose_values, inverse_bind_pose_values, root_prefix_values);
    free(hierarchy_values);
    (*environment)->ReleaseIntArrayElements(
        environment, hierarchy, hierarchy_elements, JNI_ABORT);
    if (bind_pose_elements != NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, bind_pose, bind_pose_elements, JNI_ABORT);
    }
    free(bind_pose_values);
    if (inverse_bind_pose_elements != NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, inverse_bind_pose, inverse_bind_pose_elements, JNI_ABORT);
    }
    free(inverse_bind_pose_values);
    if (root_prefix_elements != NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, root_prefix, root_prefix_elements, JNI_ABORT);
    }
    free(root_prefix_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReadLimitsInit(
    JNIEnv* environment, jclass declaring_class, jlongArray out_limitsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbReadLimits out_limits_value;
    memset(&out_limits_value, 0, sizeof out_limits_value);
    out_limits_value.struct_size = (uint32_t)(sizeof out_limits_value);
    out_limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_read_limits_init(&out_limits_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_limitsIntegral_values[] = {
            (jlong)out_limits_value.max_file_size,
            (jlong)out_limits_value.max_chunk_size,
            (jlong)out_limits_value.max_total_uncompressed_size,
            (jlong)out_limits_value.max_chunk_count,
            (jlong)out_limits_value.max_string_bytes,
            (jlong)out_limits_value.max_array_element_count,
            (jlong)out_limits_value.max_chunk_alignment
        };
        (*environment)->SetLongArrayRegion(environment, out_limitsIntegral, 0,
            (jsize)(sizeof out_limitsIntegral_values / sizeof out_limitsIntegral_values[0]),
            out_limitsIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderCopyContext(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_reader_copy_context((CNA_CnbReaderHandle)reader, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderCopyString(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.cnb_reader_copy_string((CNA_CnbReaderHandle)reader, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderCreate(
    JNIEnv* environment, jclass declaring_class, jbyteArray data, jbyteArray context, jlongArray limitsIntegral, jlongArray out_reader)
{
    (void)environment;
    (void)declaring_class;
    if (limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize data_size = (*environment)->GetArrayLength(environment, data);
    jbyte* data_elements = (*environment)->GetByteArrayElements(environment, data, NULL);
    if (data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* data_values = (uint8_t*)malloc(
        ((size_t)data_size + 1U) * sizeof(*data_values));
    if (data_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < data_size; ++index) {
        data_values[index] = (uint8_t)data_elements[index];
    }
    jsize context_size = (*environment)->GetArrayLength(environment, context);
    jbyte* context_bytes = (*environment)->GetByteArrayElements(environment, context, NULL);
    if (context_bytes == NULL) {
        free(data_values);
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView context_view = {(const char*)context_bytes, (uint64_t)context_size};
    CNA_CnbReadLimits limits_value;
    memset(&limits_value, 0, sizeof limits_value);
    limits_value.struct_size = (uint32_t)(sizeof limits_value);
    limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    {
        jlong limitsIntegral_values[7];
        (*environment)->GetLongArrayRegion(environment, limitsIntegral, 0, 7, limitsIntegral_values);
        limits_value.max_file_size = (uint64_t)limitsIntegral_values[0];
        limits_value.max_chunk_size = (uint64_t)limitsIntegral_values[1];
        limits_value.max_total_uncompressed_size = (uint64_t)limitsIntegral_values[2];
        limits_value.max_chunk_count = (uint32_t)limitsIntegral_values[3];
        limits_value.max_string_bytes = (uint32_t)limitsIntegral_values[4];
        limits_value.max_array_element_count = (uint32_t)limitsIntegral_values[5];
        limits_value.max_chunk_alignment = (uint32_t)limitsIntegral_values[6];
    }
    CNA_CnbReaderHandle out_reader_value = 0;
    CNA_Result call_result = cna.cnb_reader_create(data_values, (uint64_t)data_size, context_view, &limits_value, &out_reader_value);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, context, context_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_reader_element = (jlong)out_reader_value;
        (*environment)->SetLongArrayRegion(
            environment, out_reader, 0, 1, &out_reader_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderDestroy(
    JNIEnv* environment, jclass declaring_class, jlong reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_reader_destroy((CNA_CnbReaderHandle)reader);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderFail(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray detail)
{
    (void)environment;
    (void)declaring_class;
    jsize detail_size = (*environment)->GetArrayLength(environment, detail);
    jbyte* detail_bytes = (*environment)->GetByteArrayElements(environment, detail, NULL);
    if (detail_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView detail_view = {(const char*)detail_bytes, (uint64_t)detail_size};
    CNA_Result call_result = cna.cnb_reader_fail((CNA_CnbReaderHandle)reader, detail_view);
    (*environment)->ReleaseByteArrayElements(environment, detail, detail_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderGetContextSize(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_reader_get_context_size((CNA_CnbReaderHandle)reader, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderGetPosition(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_position)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_position_value = 0;
    CNA_Result call_result = cna.cnb_reader_get_position((CNA_CnbReaderHandle)reader, &out_position_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_position_element = (jlong)out_position_value;
        (*environment)->SetLongArrayRegion(
            environment, out_position, 0, 1, &out_position_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderGetRemaining(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_remaining)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_remaining_value = 0;
    CNA_Result call_result = cna.cnb_reader_get_remaining((CNA_CnbReaderHandle)reader, &out_remaining_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_remaining_element = (jlong)out_remaining_value;
        (*environment)->SetLongArrayRegion(
            environment, out_remaining, 0, 1, &out_remaining_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderGetSize(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_size)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_size_value = 0;
    CNA_Result call_result = cna.cnb_reader_get_size((CNA_CnbReaderHandle)reader, &out_size_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_size_element = (jlong)out_size_value;
        (*environment)->SetLongArrayRegion(
            environment, out_size, 0, 1, &out_size_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadBytes(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlong byte_count, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_reader_read_bytes((CNA_CnbReaderHandle)reader, (uint64_t)byte_count, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadCount(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlong element_size, jbyteArray what_is_being_counted, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize what_is_being_counted_size = (*environment)->GetArrayLength(environment, what_is_being_counted);
    jbyte* what_is_being_counted_bytes = (*environment)->GetByteArrayElements(environment, what_is_being_counted, NULL);
    if (what_is_being_counted_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView what_is_being_counted_view = {(const char*)what_is_being_counted_bytes, (uint64_t)what_is_being_counted_size};
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_count((CNA_CnbReaderHandle)reader, (uint64_t)element_size, what_is_being_counted_view, &out_count_value);
    (*environment)->ReleaseByteArrayElements(environment, what_is_being_counted, what_is_being_counted_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadF32(
    JNIEnv* environment, jclass declaring_class, jlong reader, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_f32((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadF64(
    JNIEnv* environment, jclass declaring_class, jlong reader, jdoubleArray out_value)
{
    (void)environment;
    (void)declaring_class;
    double out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_f64((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_value_element = (jdouble)out_value_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadI32(
    JNIEnv* environment, jclass declaring_class, jlong reader, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_i32((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadKeyframe(
    JNIEnv* environment, jclass declaring_class, jlong reader, jfloatArray out_keyframeFloating, jdoubleArray out_keyframeDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (out_keyframeFloating == NULL || out_keyframeDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_KeyframeEXT out_keyframe_value;
    memset(&out_keyframe_value, 0, sizeof out_keyframe_value);
    CNA_Result call_result = cna.cnb_reader_read_keyframe((CNA_CnbReaderHandle)reader, &out_keyframe_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_keyframeFloating_values[] = {
            (jfloat)out_keyframe_value.translation.x,
            (jfloat)out_keyframe_value.translation.y,
            (jfloat)out_keyframe_value.translation.z,
            (jfloat)out_keyframe_value.rotation.x,
            (jfloat)out_keyframe_value.rotation.y,
            (jfloat)out_keyframe_value.rotation.z,
            (jfloat)out_keyframe_value.rotation.w,
            (jfloat)out_keyframe_value.scale.x,
            (jfloat)out_keyframe_value.scale.y,
            (jfloat)out_keyframe_value.scale.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_keyframeFloating, 0,
            (jsize)(sizeof out_keyframeFloating_values / sizeof out_keyframeFloating_values[0]),
            out_keyframeFloating_values);
        jdouble out_keyframeDoubles_values[] = {
            (jdouble)out_keyframe_value.time_seconds
        };
        (*environment)->SetDoubleArrayRegion(environment, out_keyframeDoubles, 0,
            (jsize)(sizeof out_keyframeDoubles_values / sizeof out_keyframeDoubles_values[0]),
            out_keyframeDoubles_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadSeconds(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray what, jdoubleArray out_seconds)
{
    (void)environment;
    (void)declaring_class;
    jsize what_size = (*environment)->GetArrayLength(environment, what);
    jbyte* what_bytes = (*environment)->GetByteArrayElements(environment, what, NULL);
    if (what_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView what_view = {(const char*)what_bytes, (uint64_t)what_size};
    double out_seconds_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_seconds((CNA_CnbReaderHandle)reader, what_view, &out_seconds_value);
    (*environment)->ReleaseByteArrayElements(environment, what, what_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jdouble out_seconds_element = (jdouble)out_seconds_value;
        (*environment)->SetDoubleArrayRegion(
            environment, out_seconds, 0, 1, &out_seconds_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadString(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_string((CNA_CnbReaderHandle)reader, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadU16(
    JNIEnv* environment, jclass declaring_class, jlong reader, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint16_t out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_u16((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadU32(
    JNIEnv* environment, jclass declaring_class, jlong reader, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_u32((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadU64(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlongArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_u64((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_value_element = (jlong)out_value_value;
        (*environment)->SetLongArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderReadU8(
    JNIEnv* environment, jclass declaring_class, jlong reader, jbyteArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint8_t out_value_value = 0;
    CNA_Result call_result = cna.cnb_reader_read_u8((CNA_CnbReaderHandle)reader, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_value_element = (jbyte)out_value_value;
        (*environment)->SetByteArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderRequireExhausted(
    JNIEnv* environment, jclass declaring_class, jlong reader)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_reader_require_exhausted((CNA_CnbReaderHandle)reader);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbReaderSkip(
    JNIEnv* environment, jclass declaring_class, jlong reader, jlong byte_count)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_reader_skip((CNA_CnbReaderHandle)reader, (uint64_t)byte_count);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSoundEffectDataCopySamples(
    JNIEnv* environment, jclass declaring_class, jlong sound, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_sound_effect_data_copy_samples((CNA_CnbSoundEffectDataHandle)sound, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSoundEffectDataCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray infoIntegral, jbyteArray samples, jlongArray out_sound)
{
    (void)environment;
    (void)declaring_class;
    if (infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSoundEffectInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    {
        jlong infoIntegral_values[6];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 6, infoIntegral_values);
        info_value.format = (CNA_CnbAudioFormat)infoIntegral_values[0];
        info_value.sample_rate = (uint32_t)infoIntegral_values[1];
        info_value.channels = (uint32_t)infoIntegral_values[2];
        info_value.frame_count = (uint32_t)infoIntegral_values[3];
        info_value.loop_start = (uint32_t)infoIntegral_values[4];
        info_value.loop_length = (uint32_t)infoIntegral_values[5];
    }
    jsize samples_size = (*environment)->GetArrayLength(environment, samples);
    jbyte* samples_elements = (*environment)->GetByteArrayElements(environment, samples, NULL);
    if (samples_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* samples_values = (uint8_t*)malloc(
        ((size_t)samples_size + 1U) * sizeof(*samples_values));
    if (samples_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, samples, samples_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < samples_size; ++index) {
        samples_values[index] = (uint8_t)samples_elements[index];
    }
    CNA_CnbSoundEffectDataHandle out_sound_value = 0;
    CNA_Result call_result = cna.cnb_sound_effect_data_create(&info_value, samples_values, (uint64_t)samples_size, &out_sound_value);
    free(samples_values);
    (*environment)->ReleaseByteArrayElements(
        environment, samples, samples_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sound_element = (jlong)out_sound_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sound, 0, 1, &out_sound_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSoundEffectDataDestroy(
    JNIEnv* environment, jclass declaring_class, jlong sound)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_sound_effect_data_destroy((CNA_CnbSoundEffectDataHandle)sound);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSoundEffectDataGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong sound, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSoundEffectInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_sound_effect_data_get_info((CNA_CnbSoundEffectDataHandle)sound, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.format,
            (jlong)out_info_value.sample_rate,
            (jlong)out_info_value.channels,
            (jlong)out_info_value.frame_count,
            (jlong)out_info_value.loop_start,
            (jlong)out_info_value.loop_length
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataAddGlyph(
    JNIEnv* environment, jclass declaring_class, jlong font, jlongArray glyphIntegral, jfloatArray glyphFloating, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    if (glyphIntegral == NULL || glyphFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SpriteFontGlyph glyph_value;
    memset(&glyph_value, 0, sizeof glyph_value);
    glyph_value.struct_size = (uint32_t)(sizeof glyph_value);
    glyph_value.struct_version = UINT32_C(1);
    {
        jlong glyphIntegral_values[10];
        (*environment)->GetLongArrayRegion(environment, glyphIntegral, 0, 10, glyphIntegral_values);
        glyph_value.glyph_bounds.x = (int32_t)glyphIntegral_values[0];
        glyph_value.glyph_bounds.y = (int32_t)glyphIntegral_values[1];
        glyph_value.glyph_bounds.width = (int32_t)glyphIntegral_values[2];
        glyph_value.glyph_bounds.height = (int32_t)glyphIntegral_values[3];
        glyph_value.cropping.x = (int32_t)glyphIntegral_values[4];
        glyph_value.cropping.y = (int32_t)glyphIntegral_values[5];
        glyph_value.cropping.width = (int32_t)glyphIntegral_values[6];
        glyph_value.cropping.height = (int32_t)glyphIntegral_values[7];
        glyph_value.character = (CNA_Char16)glyphIntegral_values[8];
        glyph_value.reserved = (uint16_t)glyphIntegral_values[9];
    }
    {
        jfloat glyphFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, glyphFloating, 0, 3, glyphFloating_values);
        glyph_value.kerning.x = (float)glyphFloating_values[0];
        glyph_value.kerning.y = (float)glyphFloating_values[1];
        glyph_value.kerning.z = (float)glyphFloating_values[2];
    }
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_sprite_font_data_add_glyph((CNA_CnbSpriteFontDataHandle)font, &glyph_value, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataCopyAtlas(
    JNIEnv* environment, jclass declaring_class, jlong font, jlongArray out_atlas)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureDataHandle out_atlas_value = 0;
    CNA_Result call_result = cna.cnb_sprite_font_data_copy_atlas((CNA_CnbSpriteFontDataHandle)font, &out_atlas_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_atlas_element = (jlong)out_atlas_value;
        (*environment)->SetLongArrayRegion(
            environment, out_atlas, 0, 1, &out_atlas_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray out_font)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbSpriteFontDataHandle out_font_value = 0;
    CNA_Result call_result = cna.cnb_sprite_font_data_create(&out_font_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_font_element = (jlong)out_font_value;
        (*environment)->SetLongArrayRegion(
            environment, out_font, 0, 1, &out_font_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataDestroy(
    JNIEnv* environment, jclass declaring_class, jlong font)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_sprite_font_data_destroy((CNA_CnbSpriteFontDataHandle)font);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataGetGlyph(
    JNIEnv* environment, jclass declaring_class, jlong font, jlong index, jlongArray out_glyphIntegral, jfloatArray out_glyphFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_glyphIntegral == NULL || out_glyphFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SpriteFontGlyph out_glyph_value;
    memset(&out_glyph_value, 0, sizeof out_glyph_value);
    out_glyph_value.struct_size = (uint32_t)(sizeof out_glyph_value);
    out_glyph_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.cnb_sprite_font_data_get_glyph((CNA_CnbSpriteFontDataHandle)font, (uint64_t)index, &out_glyph_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_glyphIntegral_values[] = {
            (jlong)out_glyph_value.glyph_bounds.x,
            (jlong)out_glyph_value.glyph_bounds.y,
            (jlong)out_glyph_value.glyph_bounds.width,
            (jlong)out_glyph_value.glyph_bounds.height,
            (jlong)out_glyph_value.cropping.x,
            (jlong)out_glyph_value.cropping.y,
            (jlong)out_glyph_value.cropping.width,
            (jlong)out_glyph_value.cropping.height,
            (jlong)out_glyph_value.character,
            (jlong)out_glyph_value.reserved
        };
        (*environment)->SetLongArrayRegion(environment, out_glyphIntegral, 0,
            (jsize)(sizeof out_glyphIntegral_values / sizeof out_glyphIntegral_values[0]),
            out_glyphIntegral_values);
        jfloat out_glyphFloating_values[] = {
            (jfloat)out_glyph_value.kerning.x,
            (jfloat)out_glyph_value.kerning.y,
            (jfloat)out_glyph_value.kerning.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_glyphFloating, 0,
            (jsize)(sizeof out_glyphFloating_values / sizeof out_glyphFloating_values[0]),
            out_glyphFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong font, jbyteArray out_infoBytes, jlongArray out_infoIntegral, jfloatArray out_infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoBytes == NULL || out_infoIntegral == NULL || out_infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSpriteFontInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_SPRITE_FONT_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_sprite_font_data_get_info((CNA_CnbSpriteFontDataHandle)font, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_infoBytes_values[] = {
            (jbyte)out_info_value.reserved[0],
            (jbyte)out_info_value.reserved[1],
            (jbyte)out_info_value.reserved[2],
            (jbyte)out_info_value.reserved[3],
            (jbyte)out_info_value.reserved[4]
        };
        (*environment)->SetByteArrayRegion(environment, out_infoBytes, 0,
            (jsize)(sizeof out_infoBytes_values / sizeof out_infoBytes_values[0]),
            out_infoBytes_values);
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.glyph_count,
            (jlong)out_info_value.line_spacing,
            (jlong)out_info_value.default_character,
            (jlong)out_info_value.has_default_character
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
        jfloat out_infoFloating_values[] = {
            (jfloat)out_info_value.spacing
        };
        (*environment)->SetFloatArrayRegion(environment, out_infoFloating, 0,
            (jsize)(sizeof out_infoFloating_values / sizeof out_infoFloating_values[0]),
            out_infoFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataSetAtlas(
    JNIEnv* environment, jclass declaring_class, jlong font, jlong atlas)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_sprite_font_data_set_atlas((CNA_CnbSpriteFontDataHandle)font, (CNA_CnbTextureDataHandle)atlas);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataSetGlyph(
    JNIEnv* environment, jclass declaring_class, jlong font, jlong index, jlongArray glyphIntegral, jfloatArray glyphFloating)
{
    (void)environment;
    (void)declaring_class;
    if (glyphIntegral == NULL || glyphFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SpriteFontGlyph glyph_value;
    memset(&glyph_value, 0, sizeof glyph_value);
    glyph_value.struct_size = (uint32_t)(sizeof glyph_value);
    glyph_value.struct_version = UINT32_C(1);
    {
        jlong glyphIntegral_values[10];
        (*environment)->GetLongArrayRegion(environment, glyphIntegral, 0, 10, glyphIntegral_values);
        glyph_value.glyph_bounds.x = (int32_t)glyphIntegral_values[0];
        glyph_value.glyph_bounds.y = (int32_t)glyphIntegral_values[1];
        glyph_value.glyph_bounds.width = (int32_t)glyphIntegral_values[2];
        glyph_value.glyph_bounds.height = (int32_t)glyphIntegral_values[3];
        glyph_value.cropping.x = (int32_t)glyphIntegral_values[4];
        glyph_value.cropping.y = (int32_t)glyphIntegral_values[5];
        glyph_value.cropping.width = (int32_t)glyphIntegral_values[6];
        glyph_value.cropping.height = (int32_t)glyphIntegral_values[7];
        glyph_value.character = (CNA_Char16)glyphIntegral_values[8];
        glyph_value.reserved = (uint16_t)glyphIntegral_values[9];
    }
    {
        jfloat glyphFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, glyphFloating, 0, 3, glyphFloating_values);
        glyph_value.kerning.x = (float)glyphFloating_values[0];
        glyph_value.kerning.y = (float)glyphFloating_values[1];
        glyph_value.kerning.z = (float)glyphFloating_values[2];
    }
    CNA_Result call_result = cna.cnb_sprite_font_data_set_glyph((CNA_CnbSpriteFontDataHandle)font, (uint64_t)index, &glyph_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbSpriteFontDataSetInfo(
    JNIEnv* environment, jclass declaring_class, jlong font, jbyteArray infoBytes, jlongArray infoIntegral, jfloatArray infoFloating)
{
    (void)environment;
    (void)declaring_class;
    if (infoBytes == NULL || infoIntegral == NULL || infoFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbSpriteFontInfo info_value;
    memset(&info_value, 0, sizeof info_value);
    info_value.struct_size = (uint32_t)(sizeof info_value);
    info_value.struct_version = CNA_CNB_SPRITE_FONT_INFO_STRUCT_VERSION;
    {
        jbyte infoBytes_values[5];
        (*environment)->GetByteArrayRegion(environment, infoBytes, 0, 5, infoBytes_values);
        info_value.reserved[0] = (uint8_t)infoBytes_values[0];
        info_value.reserved[1] = (uint8_t)infoBytes_values[1];
        info_value.reserved[2] = (uint8_t)infoBytes_values[2];
        info_value.reserved[3] = (uint8_t)infoBytes_values[3];
        info_value.reserved[4] = (uint8_t)infoBytes_values[4];
    }
    {
        jlong infoIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, infoIntegral, 0, 4, infoIntegral_values);
        info_value.glyph_count = (uint64_t)infoIntegral_values[0];
        info_value.line_spacing = (int32_t)infoIntegral_values[1];
        info_value.default_character = (CNA_Char16)infoIntegral_values[2];
        info_value.has_default_character = (CNA_Bool)infoIntegral_values[3];
    }
    {
        jfloat infoFloating_values[1];
        (*environment)->GetFloatArrayRegion(environment, infoFloating, 0, 1, infoFloating_values);
        info_value.spacing = (float)infoFloating_values[0];
    }
    CNA_Result call_result = cna.cnb_sprite_font_data_set_info((CNA_CnbSpriteFontDataHandle)font, &info_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataAddRepresentation(
    JNIEnv* environment, jclass declaring_class, jlong texture, jint format, jlongArray out_index)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_index_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_add_representation((CNA_CnbTextureDataHandle)texture, (CNA_CnbTextureFormat)format, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_index_element = (jlong)out_index_value;
        (*environment)->SetLongArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataCopyLevel(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlong representation, jlong level, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_texture_data_copy_level((CNA_CnbTextureDataHandle)texture, (uint64_t)representation, (uint64_t)level, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataCreate(
    JNIEnv* environment, jclass declaring_class, jint width, jint height, jint depth, jint face_count, jint mip_count, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_create((uint32_t)width, (uint32_t)height, (uint32_t)depth, (uint32_t)face_count, (uint32_t)mip_count, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataCreateRgba8(
    JNIEnv* environment, jclass declaring_class, jint width, jint height, jbyteArray rgba, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    jsize rgba_size = (*environment)->GetArrayLength(environment, rgba);
    jbyte* rgba_elements = (*environment)->GetByteArrayElements(environment, rgba, NULL);
    if (rgba_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* rgba_values = (uint8_t*)malloc(
        ((size_t)rgba_size + 1U) * sizeof(*rgba_values));
    if (rgba_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, rgba, rgba_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < rgba_size; ++index) {
        rgba_values[index] = (uint8_t)rgba_elements[index];
    }
    CNA_CnbTextureDataHandle out_texture_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_create_rgba8((uint32_t)width, (uint32_t)height, rgba_values, (uint64_t)rgba_size, &out_texture_value);
    free(rgba_values);
    (*environment)->ReleaseByteArrayElements(
        environment, rgba, rgba_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataDestroy(
    JNIEnv* environment, jclass declaring_class, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_texture_data_destroy((CNA_CnbTextureDataHandle)texture);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbTextureInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_texture_data_get_info((CNA_CnbTextureDataHandle)texture, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.width,
            (jlong)out_info_value.height,
            (jlong)out_info_value.depth,
            (jlong)out_info_value.face_count,
            (jlong)out_info_value.mip_count,
            (jlong)out_info_value.representation_count
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataGetLevelCount(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlong representation, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_get_level_count((CNA_CnbTextureDataHandle)texture, (uint64_t)representation, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataGetLevelDimensions(
    JNIEnv* environment, jclass declaring_class, jlong texture, jint level, jintArray out_width, jintArray out_height, jintArray out_depth)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_width_value = 0;
    uint32_t out_height_value = 0;
    uint32_t out_depth_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_get_level_dimensions((CNA_CnbTextureDataHandle)texture, (uint32_t)level, &out_width_value, &out_height_value, &out_depth_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_width_element = (jint)out_width_value;
        (*environment)->SetIntArrayRegion(
            environment, out_width, 0, 1, &out_width_element);
        jint out_height_element = (jint)out_height_value;
        (*environment)->SetIntArrayRegion(
            environment, out_height, 0, 1, &out_height_element);
        jint out_depth_element = (jint)out_depth_value;
        (*environment)->SetIntArrayRegion(
            environment, out_depth, 0, 1, &out_depth_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataGetRepresentationCount(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_get_representation_count((CNA_CnbTextureDataHandle)texture, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataGetRepresentationFormat(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlong representation, jintArray out_format)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureFormat out_format_value = 0;
    CNA_Result call_result = cna.cnb_texture_data_get_representation_format((CNA_CnbTextureDataHandle)texture, (uint64_t)representation, &out_format_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_format_element = (jint)out_format_value;
        (*environment)->SetIntArrayRegion(
            environment, out_format, 0, 1, &out_format_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureDataSetLevel(
    JNIEnv* environment, jclass declaring_class, jlong texture, jlong representation, jlong level, jbyteArray bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize bytes_size = (*environment)->GetArrayLength(environment, bytes);
    jbyte* bytes_elements = (*environment)->GetByteArrayElements(environment, bytes, NULL);
    if (bytes_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* bytes_values = (uint8_t*)malloc(
        ((size_t)bytes_size + 1U) * sizeof(*bytes_values));
    if (bytes_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, bytes, bytes_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < bytes_size; ++index) {
        bytes_values[index] = (uint8_t)bytes_elements[index];
    }
    CNA_Result call_result = cna.cnb_texture_data_set_level((CNA_CnbTextureDataHandle)texture, (uint64_t)representation, (uint64_t)level, bytes_values, (uint64_t)bytes_size);
    free(bytes_values);
    (*environment)->ReleaseByteArrayElements(
        environment, bytes, bytes_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureFormatFromSurfaceFormat(
    JNIEnv* environment, jclass declaring_class, jint surface_format, jintArray out_format)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbTextureFormat out_format_value = 0;
    CNA_Result call_result = cna.cnb_texture_format_from_surface_format((CNA_SurfaceFormat)surface_format, &out_format_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_format_element = (jint)out_format_value;
        (*environment)->SetIntArrayRegion(
            environment, out_format, 0, 1, &out_format_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbTextureFormatToSurfaceFormat(
    JNIEnv* environment, jclass declaring_class, jint format, jintArray out_surface_format)
{
    (void)environment;
    (void)declaring_class;
    CNA_SurfaceFormat out_surface_format_value = 0;
    CNA_Result call_result = cna.cnb_texture_format_to_surface_format((CNA_CnbTextureFormat)format, &out_surface_format_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_surface_format_element = (jint)out_surface_format_value;
        (*environment)->SetIntArrayRegion(
            environment, out_surface_format, 0, 1, &out_surface_format_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterAddChunk(
    JNIEnv* environment, jclass declaring_class, jlong writer, jint type, jbyteArray data, jint flags, jint alignment)
{
    (void)environment;
    (void)declaring_class;
    jsize data_size = (*environment)->GetArrayLength(environment, data);
    jbyte* data_elements = (*environment)->GetByteArrayElements(environment, data, NULL);
    if (data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint8_t* data_values = (uint8_t*)malloc(
        ((size_t)data_size + 1U) * sizeof(*data_values));
    if (data_values == NULL) {
        (*environment)->ReleaseByteArrayElements(
            environment, data, data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < data_size; ++index) {
        data_values[index] = (uint8_t)data_elements[index];
    }
    CNA_Result call_result = cna.cnb_writer_add_chunk((CNA_CnbWriterHandle)writer, (CNA_CnbChunkId)type, data_values, (uint64_t)data_size, (uint32_t)flags, (uint32_t)alignment);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterAddExternalReference(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlongArray referenceIntegral, jbyteArray logical_name)
{
    (void)environment;
    (void)declaring_class;
    if (referenceIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbExternalReference reference_value;
    memset(&reference_value, 0, sizeof reference_value);
    reference_value.struct_size = (uint32_t)(sizeof reference_value);
    reference_value.struct_version = CNA_CNB_EXTERNAL_REFERENCE_STRUCT_VERSION;
    {
        jlong referenceIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, referenceIntegral, 0, 2, referenceIntegral_values);
        reference_value.flags = (uint32_t)referenceIntegral_values[0];
        reference_value.expected_asset_type_id = (uint32_t)referenceIntegral_values[1];
    }
    jsize logical_name_size = (*environment)->GetArrayLength(environment, logical_name);
    jbyte* logical_name_bytes = (*environment)->GetByteArrayElements(environment, logical_name, NULL);
    if (logical_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView logical_name_view = {(const char*)logical_name_bytes, (uint64_t)logical_name_size};
    CNA_Result call_result = cna.cnb_writer_add_external_reference((CNA_CnbWriterHandle)writer, &reference_value, logical_name_view);
    (*environment)->ReleaseByteArrayElements(environment, logical_name, logical_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterAppendEmbeddedTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlong texture, jbyteArray label)
{
    (void)environment;
    (void)declaring_class;
    jsize label_size = (*environment)->GetArrayLength(environment, label);
    jbyte* label_bytes = (*environment)->GetByteArrayElements(environment, label, NULL);
    if (label_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView label_view = {(const char*)label_bytes, (uint64_t)label_size};
    CNA_Result call_result = cna.cnb_writer_append_embedded_texture2d((CNA_CnbWriterHandle)writer, (CNA_CnbTextureDataHandle)texture, label_view);
    (*environment)->ReleaseByteArrayElements(environment, label, label_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterBuild(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray destination, jlongArray out_byte_count)
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
    CNA_Result call_result = cna.cnb_writer_build((CNA_CnbWriterHandle)writer, destination_values, (uint64_t)destination_size, &out_byte_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterClearExternalReferences(
    JNIEnv* environment, jclass declaring_class, jlong writer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_writer_clear_external_references((CNA_CnbWriterHandle)writer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterCreate(
    JNIEnv* environment, jclass declaring_class, jint asset_type_id, jint asset_schema_version, jlongArray out_writer)
{
    (void)environment;
    (void)declaring_class;
    CNA_CnbWriterHandle out_writer_value = 0;
    CNA_Result call_result = cna.cnb_writer_create((uint32_t)asset_type_id, (uint32_t)asset_schema_version, &out_writer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_writer_element = (jlong)out_writer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_writer, 0, 1, &out_writer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterDestroy(
    JNIEnv* environment, jclass declaring_class, jlong writer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_writer_destroy((CNA_CnbWriterHandle)writer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterGetLimits(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlongArray out_limitsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbReadLimits out_limits_value;
    memset(&out_limits_value, 0, sizeof out_limits_value);
    out_limits_value.struct_size = (uint32_t)(sizeof out_limits_value);
    out_limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    CNA_Result call_result = cna.cnb_writer_get_limits((CNA_CnbWriterHandle)writer, &out_limits_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_limitsIntegral_values[] = {
            (jlong)out_limits_value.max_file_size,
            (jlong)out_limits_value.max_chunk_size,
            (jlong)out_limits_value.max_total_uncompressed_size,
            (jlong)out_limits_value.max_chunk_count,
            (jlong)out_limits_value.max_string_bytes,
            (jlong)out_limits_value.max_array_element_count,
            (jlong)out_limits_value.max_chunk_alignment
        };
        (*environment)->SetLongArrayRegion(environment, out_limitsIntegral, 0,
            (jsize)(sizeof out_limitsIntegral_values / sizeof out_limitsIntegral_values[0]),
            out_limitsIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterGetSchemaChunkCount(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.cnb_writer_get_schema_chunk_count((CNA_CnbWriterHandle)writer, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterSetCompression(
    JNIEnv* environment, jclass declaring_class, jlong writer, jint codec, jint level)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.cnb_writer_set_compression((CNA_CnbWriterHandle)writer, (CNA_CnbCompression)codec, (int32_t)level);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterSetLimits(
    JNIEnv* environment, jclass declaring_class, jlong writer, jlongArray limitsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (limitsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CnbReadLimits limits_value;
    memset(&limits_value, 0, sizeof limits_value);
    limits_value.struct_size = (uint32_t)(sizeof limits_value);
    limits_value.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    {
        jlong limitsIntegral_values[7];
        (*environment)->GetLongArrayRegion(environment, limitsIntegral, 0, 7, limitsIntegral_values);
        limits_value.max_file_size = (uint64_t)limitsIntegral_values[0];
        limits_value.max_chunk_size = (uint64_t)limitsIntegral_values[1];
        limits_value.max_total_uncompressed_size = (uint64_t)limitsIntegral_values[2];
        limits_value.max_chunk_count = (uint32_t)limitsIntegral_values[3];
        limits_value.max_string_bytes = (uint32_t)limitsIntegral_values[4];
        limits_value.max_array_element_count = (uint32_t)limitsIntegral_values[5];
        limits_value.max_chunk_alignment = (uint32_t)limitsIntegral_values[6];
    }
    CNA_Result call_result = cna.cnb_writer_set_limits((CNA_CnbWriterHandle)writer, &limits_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterSetMetadata(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray asset_type_name, jbyteArray content_name)
{
    (void)environment;
    (void)declaring_class;
    jsize asset_type_name_size = (*environment)->GetArrayLength(environment, asset_type_name);
    jbyte* asset_type_name_bytes = (*environment)->GetByteArrayElements(environment, asset_type_name, NULL);
    if (asset_type_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView asset_type_name_view = {(const char*)asset_type_name_bytes, (uint64_t)asset_type_name_size};
    jsize content_name_size = (*environment)->GetArrayLength(environment, content_name);
    jbyte* content_name_bytes = (*environment)->GetByteArrayElements(environment, content_name, NULL);
    if (content_name_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, asset_type_name, asset_type_name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView content_name_view = {(const char*)content_name_bytes, (uint64_t)content_name_size};
    CNA_Result call_result = cna.cnb_writer_set_metadata((CNA_CnbWriterHandle)writer, asset_type_name_view, content_name_view);
    (*environment)->ReleaseByteArrayElements(environment, asset_type_name, asset_type_name_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, content_name, content_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_cnbWriterWriteToFile(
    JNIEnv* environment, jclass declaring_class, jlong writer, jbyteArray path)
{
    (void)environment;
    (void)declaring_class;
    jsize path_size = (*environment)->GetArrayLength(environment, path);
    jbyte* path_bytes = (*environment)->GetByteArrayElements(environment, path, NULL);
    if (path_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView path_view = {(const char*)path_bytes, (uint64_t)path_size};
    CNA_Result call_result = cna.cnb_writer_write_to_file((CNA_CnbWriterHandle)writer, path_view);
    (*environment)->ReleaseByteArrayElements(environment, path, path_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray out_curve)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveHandle out_curve_value = 0;
    CNA_Result call_result = cna.curve_create(&out_curve_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_curve_element = (jlong)out_curve_value;
        (*environment)->SetLongArrayRegion(
            environment, out_curve, 0, 1, &out_curve_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveDestroy(
    JNIEnv* environment, jclass declaring_class, jlong curve)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.curve_destroy((CNA_CurveHandle)curve);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveGetKeys(
    JNIEnv* environment, jclass declaring_class, jlong curve, jlongArray out_keys)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveKeyCollectionHandle out_keys_value = 0;
    CNA_Result call_result = cna.curve_get_keys((CNA_CurveHandle)curve, &out_keys_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_keys_element = (jlong)out_keys_value;
        (*environment)->SetLongArrayRegion(
            environment, out_keys, 0, 1, &out_keys_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveGetPostLoop(
    JNIEnv* environment, jclass declaring_class, jlong curve, jintArray out_loop_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveLoopType out_loop_type_value = 0;
    CNA_Result call_result = cna.curve_get_post_loop((CNA_CurveHandle)curve, &out_loop_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_loop_type_element = (jint)out_loop_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_loop_type, 0, 1, &out_loop_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveGetPreLoop(
    JNIEnv* environment, jclass declaring_class, jlong curve, jintArray out_loop_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveLoopType out_loop_type_value = 0;
    CNA_Result call_result = cna.curve_get_pre_loop((CNA_CurveHandle)curve, &out_loop_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_loop_type_element = (jint)out_loop_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_loop_type, 0, 1, &out_loop_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveKeyCollectionAdd(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray keyIntegral, jfloatArray keyFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_CurveKey key_value;
    memset(&key_value, 0, sizeof key_value);
    {
        jlong keyIntegral_values[1];
        (*environment)->GetLongArrayRegion(environment, keyIntegral, 0, 1, keyIntegral_values);
        key_value.continuity = (CNA_CurveContinuity)keyIntegral_values[0];
    }
    {
        jfloat keyFloating_values[4];
        (*environment)->GetFloatArrayRegion(environment, keyFloating, 0, 4, keyFloating_values);
        key_value.position = (float)keyFloating_values[0];
        key_value.value = (float)keyFloating_values[1];
        key_value.tangent_in = (float)keyFloating_values[2];
        key_value.tangent_out = (float)keyFloating_values[3];
    }
    CNA_Result call_result = cna.curve_key_collection_add((CNA_CurveKeyCollectionHandle)collection, key_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveKeyCollectionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.curve_key_collection_destroy((CNA_CurveKeyCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveKeyCollectionGet(
    JNIEnv* environment, jclass declaring_class, jlong collection, jint index, jlongArray out_keyIntegral, jfloatArray out_keyFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_keyIntegral == NULL || out_keyFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CurveKey out_key_value;
    memset(&out_key_value, 0, sizeof out_key_value);
    CNA_Result call_result = cna.curve_key_collection_get((CNA_CurveKeyCollectionHandle)collection, (int32_t)index, &out_key_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_keyIntegral_values[] = {
            (jlong)out_key_value.continuity
        };
        (*environment)->SetLongArrayRegion(environment, out_keyIntegral, 0,
            (jsize)(sizeof out_keyIntegral_values / sizeof out_keyIntegral_values[0]),
            out_keyIntegral_values);
        jfloat out_keyFloating_values[] = {
            (jfloat)out_key_value.position,
            (jfloat)out_key_value.value,
            (jfloat)out_key_value.tangent_in,
            (jfloat)out_key_value.tangent_out
        };
        (*environment)->SetFloatArrayRegion(environment, out_keyFloating, 0,
            (jsize)(sizeof out_keyFloating_values / sizeof out_keyFloating_values[0]),
            out_keyFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveKeyCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.curve_key_collection_get_count((CNA_CurveKeyCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveKeyInitFull(
    JNIEnv* environment, jclass declaring_class, jfloat position, jfloat value, jfloat tangent_in, jfloat tangent_out, jint continuity, jlongArray out_keyIntegral, jfloatArray out_keyFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_keyIntegral == NULL || out_keyFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CurveKey out_key_value;
    memset(&out_key_value, 0, sizeof out_key_value);
    CNA_Result call_result = cna.curve_key_init_full((float)position, (float)value, (float)tangent_in, (float)tangent_out, (CNA_CurveContinuity)continuity, &out_key_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_keyIntegral_values[] = {
            (jlong)out_key_value.continuity
        };
        (*environment)->SetLongArrayRegion(environment, out_keyIntegral, 0,
            (jsize)(sizeof out_keyIntegral_values / sizeof out_keyIntegral_values[0]),
            out_keyIntegral_values);
        jfloat out_keyFloating_values[] = {
            (jfloat)out_key_value.position,
            (jfloat)out_key_value.value,
            (jfloat)out_key_value.tangent_in,
            (jfloat)out_key_value.tangent_out
        };
        (*environment)->SetFloatArrayRegion(environment, out_keyFloating, 0,
            (jsize)(sizeof out_keyFloating_values / sizeof out_keyFloating_values[0]),
            out_keyFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveSetPostLoop(
    JNIEnv* environment, jclass declaring_class, jlong curve, jint loop_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.curve_set_post_loop((CNA_CurveHandle)curve, (CNA_CurveLoopType)loop_type);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeCnbRoutes_curveSetPreLoop(
    JNIEnv* environment, jclass declaring_class, jlong curve, jint loop_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.curve_set_pre_loop((CNA_CurveHandle)curve, (CNA_CurveLoopType)loop_type);
    return (jint)call_result;
}
