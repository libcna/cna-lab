/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeNetworkRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCollectionCopySession(
    JNIEnv* environment, jclass declaring_class, jlong collection, jint index, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvailableNetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.available_network_session_collection_copy_session((CNA_AvailableNetworkSessionCollectionHandle)collection, (int32_t)index, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCollectionDispose(
    JNIEnv* environment, jclass declaring_class, jlong collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.available_network_session_collection_dispose((CNA_AvailableNetworkSessionCollectionHandle)collection);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCollectionGetCount(
    JNIEnv* environment, jclass declaring_class, jlong collection, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.available_network_session_collection_get_count((CNA_AvailableNetworkSessionCollectionHandle)collection, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCollectionGetIsDisposed(
    JNIEnv* environment, jclass declaring_class, jlong collection, jbooleanArray out_is_disposed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_disposed_value = 0;
    CNA_Result call_result = cna.available_network_session_collection_get_is_disposed((CNA_AvailableNetworkSessionCollectionHandle)collection, &out_is_disposed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_disposed_element = (jboolean)out_is_disposed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_disposed, 0, 1, &out_is_disposed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCopyConnectAddressExt(
    JNIEnv* environment, jclass declaring_class, jlong session, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.available_network_session_copy_connect_address_ext((CNA_AvailableNetworkSessionHandle)session, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCopyHostGamertag(
    JNIEnv* environment, jclass declaring_class, jlong session, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.available_network_session_copy_host_gamertag((CNA_AvailableNetworkSessionHandle)session, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCopySessionProperties(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_properties)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionPropertiesHandle out_properties_value = 0;
    CNA_Result call_result = cna.available_network_session_copy_session_properties((CNA_AvailableNetworkSessionHandle)session, &out_properties_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_properties_element = (jlong)out_properties_value;
        (*environment)->SetLongArrayRegion(
            environment, out_properties, 0, 1, &out_properties_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionCreateExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray create_infoBytes, jlongArray create_infoIntegral, jbyteArray createInfoHostGamertag, jbyteArray createInfoHostAddress, jbyteArray quality_of_serviceBytes, jlongArray quality_of_serviceIntegral, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    if (create_infoBytes == NULL || create_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (quality_of_serviceBytes == NULL || quality_of_serviceIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AvailableNetworkSessionCreateInfo create_info_value;
    memset(&create_info_value, 0, sizeof create_info_value);
    create_info_value.struct_size = (uint32_t)(sizeof create_info_value);
    create_info_value.struct_version = UINT32_C(1);
    {
        jbyte create_infoBytes_values[6];
        (*environment)->GetByteArrayRegion(environment, create_infoBytes, 0, 6, create_infoBytes_values);
        create_info_value.reserved[0] = (uint8_t)create_infoBytes_values[0];
        create_info_value.reserved[1] = (uint8_t)create_infoBytes_values[1];
        create_info_value.reserved[2] = (uint8_t)create_infoBytes_values[2];
        create_info_value.reserved[3] = (uint8_t)create_infoBytes_values[3];
        create_info_value.reserved[4] = (uint8_t)create_infoBytes_values[4];
        create_info_value.reserved[5] = (uint8_t)create_infoBytes_values[5];
    }
    {
        jlong create_infoIntegral_values[6];
        (*environment)->GetLongArrayRegion(environment, create_infoIntegral, 0, 6, create_infoIntegral_values);
        create_info_value.current_gamer_count = (int32_t)create_infoIntegral_values[0];
        create_info_value.open_private_gamer_slots = (int32_t)create_infoIntegral_values[1];
        create_info_value.open_public_gamer_slots = (int32_t)create_infoIntegral_values[2];
        create_info_value.session_type = (CNA_NetworkSessionType)create_infoIntegral_values[3];
        create_info_value.host_port = (uint16_t)create_infoIntegral_values[4];
        create_info_value.session_properties = (CNA_NetworkSessionPropertiesHandle)create_infoIntegral_values[5];
    }
    jsize createInfoHostGamertag_size = (*environment)->GetArrayLength(environment, createInfoHostGamertag);
    jbyte* createInfoHostGamertag_bytes = (*environment)->GetByteArrayElements(environment, createInfoHostGamertag, NULL);
    if (createInfoHostGamertag_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    create_info_value.host_gamertag.data = (const char*)createInfoHostGamertag_bytes;
    create_info_value.host_gamertag.byte_length = (uint64_t)createInfoHostGamertag_size;
    jsize createInfoHostAddress_size = (*environment)->GetArrayLength(environment, createInfoHostAddress);
    jbyte* createInfoHostAddress_bytes = (*environment)->GetByteArrayElements(environment, createInfoHostAddress, NULL);
    if (createInfoHostAddress_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, createInfoHostGamertag, createInfoHostGamertag_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    create_info_value.host_address.data = (const char*)createInfoHostAddress_bytes;
    create_info_value.host_address.byte_length = (uint64_t)createInfoHostAddress_size;
    CNA_QualityOfService quality_of_service_value;
    memset(&quality_of_service_value, 0, sizeof quality_of_service_value);
    quality_of_service_value.struct_size = (uint32_t)(sizeof quality_of_service_value);
    quality_of_service_value.struct_version = UINT32_C(1);
    {
        jbyte quality_of_serviceBytes_values[7];
        (*environment)->GetByteArrayRegion(environment, quality_of_serviceBytes, 0, 7, quality_of_serviceBytes_values);
        quality_of_service_value.reserved[0] = (uint8_t)quality_of_serviceBytes_values[0];
        quality_of_service_value.reserved[1] = (uint8_t)quality_of_serviceBytes_values[1];
        quality_of_service_value.reserved[2] = (uint8_t)quality_of_serviceBytes_values[2];
        quality_of_service_value.reserved[3] = (uint8_t)quality_of_serviceBytes_values[3];
        quality_of_service_value.reserved[4] = (uint8_t)quality_of_serviceBytes_values[4];
        quality_of_service_value.reserved[5] = (uint8_t)quality_of_serviceBytes_values[5];
        quality_of_service_value.reserved[6] = (uint8_t)quality_of_serviceBytes_values[6];
    }
    {
        jlong quality_of_serviceIntegral_values[5];
        (*environment)->GetLongArrayRegion(environment, quality_of_serviceIntegral, 0, 5, quality_of_serviceIntegral_values);
        quality_of_service_value.is_available = (CNA_Bool)quality_of_serviceIntegral_values[0];
        quality_of_service_value.average_roundtrip_ticks = (int64_t)quality_of_serviceIntegral_values[1];
        quality_of_service_value.minimum_roundtrip_ticks = (int64_t)quality_of_serviceIntegral_values[2];
        quality_of_service_value.bytes_per_second_downstream = (int32_t)quality_of_serviceIntegral_values[3];
        quality_of_service_value.bytes_per_second_upstream = (int32_t)quality_of_serviceIntegral_values[4];
    }
    CNA_AvailableNetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.available_network_session_create_ext(&create_info_value, &quality_of_service_value, &out_session_value);
    (*environment)->ReleaseByteArrayElements(environment, createInfoHostGamertag, createInfoHostGamertag_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, createInfoHostAddress, createInfoHostAddress_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.available_network_session_destroy((CNA_AvailableNetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetConnectAddressSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.available_network_session_get_connect_address_size_ext((CNA_AvailableNetworkSessionHandle)session, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetConnectPortExt(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint16_t out_value_value = 0;
    CNA_Result call_result = cna.available_network_session_get_connect_port_ext((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetCurrentGamerCount(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.available_network_session_get_current_gamer_count((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetHostGamertagSize(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.available_network_session_get_host_gamertag_size((CNA_AvailableNetworkSessionHandle)session, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetOpenPrivateGamerSlots(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.available_network_session_get_open_private_gamer_slots((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetOpenPublicGamerSlots(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.available_network_session_get_open_public_gamer_slots((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetQualityOfService(
    JNIEnv* environment, jclass declaring_class, jlong session, jbyteArray out_valueBytes, jlongArray out_valueIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueBytes == NULL || out_valueIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_QualityOfService out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    out_value_value.struct_size = (uint32_t)(sizeof out_value_value);
    out_value_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.available_network_session_get_quality_of_service((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_valueBytes_values[] = {
            (jbyte)out_value_value.reserved[0],
            (jbyte)out_value_value.reserved[1],
            (jbyte)out_value_value.reserved[2],
            (jbyte)out_value_value.reserved[3],
            (jbyte)out_value_value.reserved[4],
            (jbyte)out_value_value.reserved[5],
            (jbyte)out_value_value.reserved[6]
        };
        (*environment)->SetByteArrayRegion(environment, out_valueBytes, 0,
            (jsize)(sizeof out_valueBytes_values / sizeof out_valueBytes_values[0]),
            out_valueBytes_values);
        jlong out_valueIntegral_values[] = {
            (jlong)out_value_value.is_available,
            (jlong)out_value_value.average_roundtrip_ticks,
            (jlong)out_value_value.minimum_roundtrip_ticks,
            (jlong)out_value_value.bytes_per_second_downstream,
            (jlong)out_value_value.bytes_per_second_upstream
        };
        (*environment)->SetLongArrayRegion(environment, out_valueIntegral, 0,
            (jsize)(sizeof out_valueIntegral_values / sizeof out_valueIntegral_values[0]),
            out_valueIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_availableNetworkSessionGetSessionTypeExt(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionType out_value_value = 0;
    CNA_Result call_result = cna.available_network_session_get_session_type_ext((CNA_AvailableNetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerEnableSendVoice(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong remote_gamer, jboolean enable)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.local_network_gamer_enable_send_voice((CNA_NetworkGamerHandle)gamer, (CNA_NetworkGamerHandle)remote_gamer, (CNA_Bool)enable);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerGetIsDataAvailable(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.local_network_gamer_get_is_data_available((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerGetSignedInGamer(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_signed_in_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_SignedInGamerHandle out_signed_in_gamer_value = 0;
    CNA_Result call_result = cna.local_network_gamer_get_signed_in_gamer((CNA_NetworkGamerHandle)gamer, &out_signed_in_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_signed_in_gamer_element = (jlong)out_signed_in_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_signed_in_gamer, 0, 1, &out_signed_in_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerReceiveData(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jlongArray out_sender, jlongArray out_received)
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
    CNA_NetworkGamerHandle out_sender_value = 0;
    uint64_t out_received_value = 0;
    CNA_Result call_result = cna.local_network_gamer_receive_data((CNA_NetworkGamerHandle)gamer, destination_values, (uint64_t)destination_size, &out_sender_value, &out_received_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sender_element = (jlong)out_sender_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sender, 0, 1, &out_sender_element);
        jlong out_received_element = (jlong)out_received_value;
        (*environment)->SetLongArrayRegion(
            environment, out_received, 0, 1, &out_received_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerReceiveDataAt(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray destination, jint offset, jlongArray out_sender, jlongArray out_received)
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
    CNA_NetworkGamerHandle out_sender_value = 0;
    uint64_t out_received_value = 0;
    CNA_Result call_result = cna.local_network_gamer_receive_data_at((CNA_NetworkGamerHandle)gamer, destination_values, (uint64_t)destination_size, (int32_t)offset, &out_sender_value, &out_received_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jbyte)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sender_element = (jlong)out_sender_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sender, 0, 1, &out_sender_element);
        jlong out_received_element = (jlong)out_received_value;
        (*environment)->SetLongArrayRegion(
            environment, out_received, 0, 1, &out_received_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerSendData(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray data, jint options)
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
    CNA_Result call_result = cna.local_network_gamer_send_data((CNA_NetworkGamerHandle)gamer, data_values, (uint64_t)data_size, (CNA_SendDataOptions)options);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerSendDataRange(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray data, jint offset, jint length, jint options)
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
    CNA_Result call_result = cna.local_network_gamer_send_data_range((CNA_NetworkGamerHandle)gamer, data_values, (uint64_t)data_size, (int32_t)offset, (int32_t)length, (CNA_SendDataOptions)options);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerSendDataRangeTo(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray data, jint offset, jint length, jint options, jlong recipient)
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
    CNA_Result call_result = cna.local_network_gamer_send_data_range_to((CNA_NetworkGamerHandle)gamer, data_values, (uint64_t)data_size, (int32_t)offset, (int32_t)length, (CNA_SendDataOptions)options, (CNA_NetworkGamerHandle)recipient);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerSendDataTo(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray data, jint options, jlong recipient)
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
    CNA_Result call_result = cna.local_network_gamer_send_data_to((CNA_NetworkGamerHandle)gamer, data_values, (uint64_t)data_size, (CNA_SendDataOptions)options, (CNA_NetworkGamerHandle)recipient);
    free(data_values);
    (*environment)->ReleaseByteArrayElements(
        environment, data, data_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_localNetworkGamerSendPartyInvites(
    JNIEnv* environment, jclass declaring_class, jlong gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.local_network_gamer_send_party_invites((CNA_NetworkGamerHandle)gamer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_netGetLastJoinError(
    JNIEnv* environment, jclass declaring_class, jintArray out_join_error, jbooleanArray out_has_join_error)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionJoinError out_join_error_value = 0;
    CNA_Bool out_has_join_error_value = 0;
    CNA_Result call_result = cna.net_get_last_join_error(&out_join_error_value, &out_has_join_error_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_join_error_element = (jint)out_join_error_value;
        (*environment)->SetIntArrayRegion(
            environment, out_join_error, 0, 1, &out_join_error_element);
        jboolean out_has_join_error_element = (jboolean)out_has_join_error_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_join_error, 0, 1, &out_has_join_error_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerCopyMachine(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_machine)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkMachineHandle out_machine_value = 0;
    CNA_Result call_result = cna.network_gamer_copy_machine((CNA_NetworkGamerHandle)gamer, &out_machine_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_machine_element = (jlong)out_machine_value;
        (*environment)->SetLongArrayRegion(
            environment, out_machine, 0, 1, &out_machine_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetHasLeftSession(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_has_left_session((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetHasVoice(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_has_voice((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetId(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbyteArray out_value)
{
    (void)environment;
    (void)declaring_class;
    uint8_t out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_id((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_value_element = (jbyte)out_value_value;
        (*environment)->SetByteArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsGuest(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_guest((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsHost(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_host((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsLocal(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_local((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsMutedByLocalUser(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_muted_by_local_user((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsPrivateSlot(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_private_slot((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsReady(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_ready((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetIsTalking(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_gamer_get_is_talking((CNA_NetworkGamerHandle)gamer, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetRoundtripTicks(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.network_gamer_get_roundtrip_ticks((CNA_NetworkGamerHandle)gamer, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerGetSession(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Handle out_session_value = 0;
    CNA_Result call_result = cna.network_gamer_get_session((CNA_NetworkGamerHandle)gamer, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerSetIsReady(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_gamer_set_is_ready((CNA_NetworkGamerHandle)gamer, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkGamerSetMachine(
    JNIEnv* environment, jclass declaring_class, jlong gamer, jlong machine)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_gamer_set_machine((CNA_NetworkGamerHandle)gamer, (CNA_NetworkMachineHandle)machine);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkMachineGetGamer(
    JNIEnv* environment, jclass declaring_class, jlong machine, jint index, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.network_machine_get_gamer((CNA_NetworkMachineHandle)machine, (int32_t)index, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkMachineGetGamerCount(
    JNIEnv* environment, jclass declaring_class, jlong machine, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.network_machine_get_gamer_count((CNA_NetworkMachineHandle)machine, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkMachineRemoveFromSession(
    JNIEnv* environment, jclass declaring_class, jlong machine)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_machine_remove_from_session((CNA_NetworkMachineHandle)machine);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionAddLocalGamer(
    JNIEnv* environment, jclass declaring_class, jlong session, jlong signed_in_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_add_local_gamer((CNA_NetworkSessionHandle)session, (CNA_Handle)signed_in_gamer);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionCopySessionProperties(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_properties)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionPropertiesHandle out_properties_value = 0;
    CNA_Result call_result = cna.network_session_copy_session_properties((CNA_NetworkSessionHandle)session, &out_properties_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_properties_element = (jlong)out_properties_value;
        (*environment)->SetLongArrayRegion(
            environment, out_properties, 0, 1, &out_properties_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionCreate(
    JNIEnv* environment, jclass declaring_class, jint session_type, jint max_local_gamers, jint max_gamers, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_create((CNA_NetworkSessionType)session_type, (int32_t)max_local_gamers, (int32_t)max_gamers, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionCreateWithLocalGamers(
    JNIEnv* environment, jclass declaring_class, jint session_type, jlongArray local_gamers, jint max_gamers, jint private_gamer_slots, jlong session_properties, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    jsize local_gamers_size = (*environment)->GetArrayLength(environment, local_gamers);
    jlong* local_gamers_elements = (*environment)->GetLongArrayElements(environment, local_gamers, NULL);
    if (local_gamers_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_Handle* local_gamers_values = (CNA_Handle*)malloc(
        ((size_t)local_gamers_size + 1U) * sizeof(*local_gamers_values));
    if (local_gamers_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, local_gamers, local_gamers_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < local_gamers_size; ++index) {
        local_gamers_values[index] = (CNA_Handle)local_gamers_elements[index];
    }
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_create_with_local_gamers((CNA_NetworkSessionType)session_type, local_gamers_values, (uint64_t)local_gamers_size, (int32_t)max_gamers, (int32_t)private_gamer_slots, (CNA_NetworkSessionPropertiesHandle)session_properties, &out_session_value);
    free(local_gamers_values);
    (*environment)->ReleaseLongArrayElements(
        environment, local_gamers, local_gamers_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionCreateWithProperties(
    JNIEnv* environment, jclass declaring_class, jint session_type, jint max_local_gamers, jint max_gamers, jint private_gamer_slots, jlong session_properties, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_create_with_properties((CNA_NetworkSessionType)session_type, (int32_t)max_local_gamers, (int32_t)max_gamers, (int32_t)private_gamer_slots, (CNA_NetworkSessionPropertiesHandle)session_properties, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_destroy((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionDispose(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_dispose((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionEndGame(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_end_game((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionFind(
    JNIEnv* environment, jclass declaring_class, jint session_type, jint max_local_gamers, jlong search_properties, jlongArray out_collection)
{
    (void)environment;
    (void)declaring_class;
    CNA_AvailableNetworkSessionCollectionHandle out_collection_value = 0;
    CNA_Result call_result = cna.network_session_find((CNA_NetworkSessionType)session_type, (int32_t)max_local_gamers, (CNA_NetworkSessionPropertiesHandle)search_properties, &out_collection_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_collection_element = (jlong)out_collection_value;
        (*environment)->SetLongArrayRegion(
            environment, out_collection, 0, 1, &out_collection_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionFindGamerById(
    JNIEnv* environment, jclass declaring_class, jlong session, jbyte gamer_id, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.network_session_find_gamer_by_id((CNA_NetworkSessionHandle)session, (uint8_t)gamer_id, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionFindWithLocalGamers(
    JNIEnv* environment, jclass declaring_class, jint session_type, jlongArray local_gamers, jlong search_properties, jlongArray out_collection)
{
    (void)environment;
    (void)declaring_class;
    jsize local_gamers_size = (*environment)->GetArrayLength(environment, local_gamers);
    jlong* local_gamers_elements = (*environment)->GetLongArrayElements(environment, local_gamers, NULL);
    if (local_gamers_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_Handle* local_gamers_values = (CNA_Handle*)malloc(
        ((size_t)local_gamers_size + 1U) * sizeof(*local_gamers_values));
    if (local_gamers_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, local_gamers, local_gamers_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < local_gamers_size; ++index) {
        local_gamers_values[index] = (CNA_Handle)local_gamers_elements[index];
    }
    CNA_AvailableNetworkSessionCollectionHandle out_collection_value = 0;
    CNA_Result call_result = cna.network_session_find_with_local_gamers((CNA_NetworkSessionType)session_type, local_gamers_values, (uint64_t)local_gamers_size, (CNA_NetworkSessionPropertiesHandle)search_properties, &out_collection_value);
    free(local_gamers_values);
    (*environment)->ReleaseLongArrayElements(
        environment, local_gamers, local_gamers_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_collection_element = (jlong)out_collection_value;
        (*environment)->SetLongArrayRegion(
            environment, out_collection, 0, 1, &out_collection_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetAllowHostMigration(
    JNIEnv* environment, jclass declaring_class, jlong session, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_allow_host_migration((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetAllowJoinInProgress(
    JNIEnv* environment, jclass declaring_class, jlong session, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_allow_join_in_progress((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetBytesPerSecondReceived(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_bytes_per_second_received((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetBytesPerSecondSent(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_bytes_per_second_sent((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetGamer(
    JNIEnv* environment, jclass declaring_class, jlong session, jint roster, jint index, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.network_session_get_gamer((CNA_NetworkSessionHandle)session, (uint32_t)roster, (int32_t)index, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetGamerCount(
    JNIEnv* environment, jclass declaring_class, jlong session, jint roster, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.network_session_get_gamer_count((CNA_NetworkSessionHandle)session, (uint32_t)roster, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetHost(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_gamer)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkGamerHandle out_gamer_value = 0;
    CNA_Result call_result = cna.network_session_get_host((CNA_NetworkSessionHandle)session, &out_gamer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_gamer_element = (jlong)out_gamer_value;
        (*environment)->SetLongArrayRegion(
            environment, out_gamer, 0, 1, &out_gamer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetIsDisposed(
    JNIEnv* environment, jclass declaring_class, jlong session, jbooleanArray out_is_disposed)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_disposed_value = 0;
    CNA_Result call_result = cna.network_session_get_is_disposed((CNA_NetworkSessionHandle)session, &out_is_disposed_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_disposed_element = (jboolean)out_is_disposed_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_disposed, 0, 1, &out_is_disposed_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetIsEveryoneReady(
    JNIEnv* environment, jclass declaring_class, jlong session, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_is_everyone_ready((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetIsHost(
    JNIEnv* environment, jclass declaring_class, jlong session, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_is_host((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetMaxGamers(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_max_gamers((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetPrivateGamerSlots(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_private_gamer_slots((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetSessionState(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionState out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_session_state((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetSessionType(
    JNIEnv* environment, jclass declaring_class, jlong session, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionType out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_session_type((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetSimulatedLatencyTicks(
    JNIEnv* environment, jclass declaring_class, jlong session, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.network_session_get_simulated_latency_ticks((CNA_NetworkSessionHandle)session, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionGetSimulatedPacketLoss(
    JNIEnv* environment, jclass declaring_class, jlong session, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.network_session_get_simulated_packet_loss((CNA_NetworkSessionHandle)session, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionJoin(
    JNIEnv* environment, jclass declaring_class, jlong available_session, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_join((CNA_AvailableNetworkSessionHandle)available_session, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionJoinInvited(
    JNIEnv* environment, jclass declaring_class, jint max_local_gamers, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_join_invited((int32_t)max_local_gamers, &out_session_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionJoinInvitedWithLocalGamers(
    JNIEnv* environment, jclass declaring_class, jlongArray local_gamers, jlongArray out_session)
{
    (void)environment;
    (void)declaring_class;
    jsize local_gamers_size = (*environment)->GetArrayLength(environment, local_gamers);
    jlong* local_gamers_elements = (*environment)->GetLongArrayElements(environment, local_gamers, NULL);
    if (local_gamers_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_Handle* local_gamers_values = (CNA_Handle*)malloc(
        ((size_t)local_gamers_size + 1U) * sizeof(*local_gamers_values));
    if (local_gamers_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, local_gamers, local_gamers_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < local_gamers_size; ++index) {
        local_gamers_values[index] = (CNA_Handle)local_gamers_elements[index];
    }
    CNA_NetworkSessionHandle out_session_value = 0;
    CNA_Result call_result = cna.network_session_join_invited_with_local_gamers(local_gamers_values, (uint64_t)local_gamers_size, &out_session_value);
    free(local_gamers_values);
    (*environment)->ReleaseLongArrayElements(
        environment, local_gamers, local_gamers_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_session_element = (jlong)out_session_value;
        (*environment)->SetLongArrayRegion(
            environment, out_session, 0, 1, &out_session_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesContains(
    JNIEnv* environment, jclass declaring_class, jlong properties, jbyteArray valueBytes, jlongArray valueIntegral, jbooleanArray out_contains)
{
    (void)environment;
    (void)declaring_class;
    CNA_OptionalInt32 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jbyte valueBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, valueBytes, 0, 3, valueBytes_values);
        value_value.reserved[0] = (uint8_t)valueBytes_values[0];
        value_value.reserved[1] = (uint8_t)valueBytes_values[1];
        value_value.reserved[2] = (uint8_t)valueBytes_values[2];
    }
    {
        jlong valueIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, valueIntegral, 0, 2, valueIntegral_values);
        value_value.has_value = (CNA_Bool)valueIntegral_values[0];
        value_value.value = (int32_t)valueIntegral_values[1];
    }
    CNA_Bool out_contains_value = 0;
    CNA_Result call_result = cna.network_session_properties_contains((CNA_NetworkSessionPropertiesHandle)properties, value_value, &out_contains_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_contains_element = (jboolean)out_contains_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_contains, 0, 1, &out_contains_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesCopyTo(
    JNIEnv* environment, jclass declaring_class, jlong properties, jbyteArray destinationBytes, jlongArray destinationIntegral, jint index, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationBytes) / 3;
    if ((*environment)->GetArrayLength(environment, destinationBytes) != destination_count * 3 ||
        (*environment)->GetArrayLength(environment, destinationIntegral) != destination_count * 2) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_OptionalInt32* destination_values = (CNA_OptionalInt32*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationBytes_length = (*environment)->GetArrayLength(environment, destinationBytes);
        jbyte* destinationBytes_values = (jbyte*)malloc(
            ((size_t)destinationBytes_length + 1U) * sizeof(*destinationBytes_values));
        if (destinationBytes_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetByteArrayRegion(environment, destinationBytes, 0, destinationBytes_length, destinationBytes_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].reserved[0] = (uint8_t)destinationBytes_values[element * 3 + 0];
            destination_values[element].reserved[1] = (uint8_t)destinationBytes_values[element * 3 + 1];
            destination_values[element].reserved[2] = (uint8_t)destinationBytes_values[element * 3 + 2];
        }
        free(destinationBytes_values);
    }
    {
        jsize destinationIntegral_length = (*environment)->GetArrayLength(environment, destinationIntegral);
        jlong* destinationIntegral_values = (jlong*)malloc(
            ((size_t)destinationIntegral_length + 1U) * sizeof(*destinationIntegral_values));
        if (destinationIntegral_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetLongArrayRegion(environment, destinationIntegral, 0, destinationIntegral_length, destinationIntegral_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].has_value = (CNA_Bool)destinationIntegral_values[element * 2 + 0];
            destination_values[element].value = (int32_t)destinationIntegral_values[element * 2 + 1];
        }
        free(destinationIntegral_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.network_session_properties_copy_to((CNA_NetworkSessionPropertiesHandle)properties, destination_values, (uint64_t)destination_count, (int32_t)index, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jbyte* destinationBytes_out = (jbyte*)malloc(
                ((size_t)destination_count * 3 + 1U) * sizeof(*destinationBytes_out));
            if (destinationBytes_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationBytes_out[element * 3 + 0] = (jbyte)destination_values[element].reserved[0];
                    destinationBytes_out[element * 3 + 1] = (jbyte)destination_values[element].reserved[1];
                    destinationBytes_out[element * 3 + 2] = (jbyte)destination_values[element].reserved[2];
                }
                (*environment)->SetByteArrayRegion(
                    environment, destinationBytes, 0,
                    (jsize)(destination_count * 3), destinationBytes_out);
                free(destinationBytes_out);
            }
        }
        {
            jlong* destinationIntegral_out = (jlong*)malloc(
                ((size_t)destination_count * 2 + 1U) * sizeof(*destinationIntegral_out));
            if (destinationIntegral_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationIntegral_out[element * 2 + 0] = (jlong)destination_values[element].has_value;
                    destinationIntegral_out[element * 2 + 1] = (jlong)destination_values[element].value;
                }
                (*environment)->SetLongArrayRegion(
                    environment, destinationIntegral, 0,
                    (jsize)(destination_count * 2), destinationIntegral_out);
                free(destinationIntegral_out);
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
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesCreate(
    JNIEnv* environment, jclass declaring_class, jlongArray out_properties)
{
    (void)environment;
    (void)declaring_class;
    CNA_NetworkSessionPropertiesHandle out_properties_value = 0;
    CNA_Result call_result = cna.network_session_properties_create(&out_properties_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_properties_element = (jlong)out_properties_value;
        (*environment)->SetLongArrayRegion(
            environment, out_properties, 0, 1, &out_properties_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesDestroy(
    JNIEnv* environment, jclass declaring_class, jlong properties)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_properties_destroy((CNA_NetworkSessionPropertiesHandle)properties);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesGetCount(
    JNIEnv* environment, jclass declaring_class, jlong properties, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.network_session_properties_get_count((CNA_NetworkSessionPropertiesHandle)properties, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesGetItem(
    JNIEnv* environment, jclass declaring_class, jlong properties, jint index, jbyteArray out_valueBytes, jlongArray out_valueIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueBytes == NULL || out_valueIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_OptionalInt32 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.network_session_properties_get_item((CNA_NetworkSessionPropertiesHandle)properties, (int32_t)index, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_valueBytes_values[] = {
            (jbyte)out_value_value.reserved[0],
            (jbyte)out_value_value.reserved[1],
            (jbyte)out_value_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_valueBytes, 0,
            (jsize)(sizeof out_valueBytes_values / sizeof out_valueBytes_values[0]),
            out_valueBytes_values);
        jlong out_valueIntegral_values[] = {
            (jlong)out_value_value.has_value,
            (jlong)out_value_value.value
        };
        (*environment)->SetLongArrayRegion(environment, out_valueIntegral, 0,
            (jsize)(sizeof out_valueIntegral_values / sizeof out_valueIntegral_values[0]),
            out_valueIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesIndexOf(
    JNIEnv* environment, jclass declaring_class, jlong properties, jbyteArray valueBytes, jlongArray valueIntegral, jintArray out_index)
{
    (void)environment;
    (void)declaring_class;
    CNA_OptionalInt32 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jbyte valueBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, valueBytes, 0, 3, valueBytes_values);
        value_value.reserved[0] = (uint8_t)valueBytes_values[0];
        value_value.reserved[1] = (uint8_t)valueBytes_values[1];
        value_value.reserved[2] = (uint8_t)valueBytes_values[2];
    }
    {
        jlong valueIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, valueIntegral, 0, 2, valueIntegral_values);
        value_value.has_value = (CNA_Bool)valueIntegral_values[0];
        value_value.value = (int32_t)valueIntegral_values[1];
    }
    int32_t out_index_value = 0;
    CNA_Result call_result = cna.network_session_properties_index_of((CNA_NetworkSessionPropertiesHandle)properties, value_value, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_index_element = (jint)out_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionPropertiesSetItem(
    JNIEnv* environment, jclass declaring_class, jlong properties, jint index, jbyteArray valueBytes, jlongArray valueIntegral)
{
    (void)environment;
    (void)declaring_class;
    CNA_OptionalInt32 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jbyte valueBytes_values[3];
        (*environment)->GetByteArrayRegion(environment, valueBytes, 0, 3, valueBytes_values);
        value_value.reserved[0] = (uint8_t)valueBytes_values[0];
        value_value.reserved[1] = (uint8_t)valueBytes_values[1];
        value_value.reserved[2] = (uint8_t)valueBytes_values[2];
    }
    {
        jlong valueIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, valueIntegral, 0, 2, valueIntegral_values);
        value_value.has_value = (CNA_Bool)valueIntegral_values[0];
        value_value.value = (int32_t)valueIntegral_values[1];
    }
    CNA_Result call_result = cna.network_session_properties_set_item((CNA_NetworkSessionPropertiesHandle)properties, (int32_t)index, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionResetReady(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_reset_ready((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetAllowHostMigration(
    JNIEnv* environment, jclass declaring_class, jlong session, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_allow_host_migration((CNA_NetworkSessionHandle)session, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetAllowJoinInProgress(
    JNIEnv* environment, jclass declaring_class, jlong session, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_allow_join_in_progress((CNA_NetworkSessionHandle)session, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetMaxGamers(
    JNIEnv* environment, jclass declaring_class, jlong session, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_max_gamers((CNA_NetworkSessionHandle)session, (int32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetPrivateGamerSlots(
    JNIEnv* environment, jclass declaring_class, jlong session, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_private_gamer_slots((CNA_NetworkSessionHandle)session, (int32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetSimulatedLatencyTicks(
    JNIEnv* environment, jclass declaring_class, jlong session, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_simulated_latency_ticks((CNA_NetworkSessionHandle)session, (int64_t)ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionSetSimulatedPacketLoss(
    JNIEnv* environment, jclass declaring_class, jlong session, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_set_simulated_packet_loss((CNA_NetworkSessionHandle)session, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionStartGame(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_start_game((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionUnsubscribe(
    JNIEnv* environment, jclass declaring_class, jlong registration)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_unsubscribe((CNA_NetworkSessionEventRegistrationHandle)registration);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeNetworkRoutes_networkSessionUpdate(
    JNIEnv* environment, jclass declaring_class, jlong session)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.network_session_update((CNA_NetworkSessionHandle)session);
    return (jint)call_result;
}
