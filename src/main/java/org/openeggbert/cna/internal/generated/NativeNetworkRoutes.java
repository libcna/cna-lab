package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeNetworkRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeNetworkRoutes {

    private NativeNetworkRoutes() {
    }

    /**
     * cna_available_network_session_collection_copy_session (net_sessions.h).
     */
    public static native int availableNetworkSessionCollectionCopySession(long collection, int index, long[] outSession);

    /**
     * cna_available_network_session_collection_dispose (net_sessions.h).
     */
    public static native int availableNetworkSessionCollectionDispose(long collection);

    /**
     * cna_available_network_session_collection_get_count (net_sessions.h).
     */
    public static native int availableNetworkSessionCollectionGetCount(long collection, int[] outCount);

    /**
     * cna_available_network_session_collection_get_is_disposed (net_sessions.h).
     */
    public static native int availableNetworkSessionCollectionGetIsDisposed(long collection, boolean[] outIsDisposed);

    /**
     * cna_available_network_session_copy_connect_address_ext (net_sessions.h).
     */
    public static native int availableNetworkSessionCopyConnectAddressExt(long session, byte[] destination, long[] outBytes);

    /**
     * cna_available_network_session_copy_host_gamertag (net_sessions.h).
     */
    public static native int availableNetworkSessionCopyHostGamertag(long session, byte[] destination, long[] outBytes);

    /**
     * cna_available_network_session_copy_session_properties (net_sessions.h).
     */
    public static native int availableNetworkSessionCopySessionProperties(long session, long[] outProperties);

    /**
     * cna_available_network_session_create_ext (net_sessions.h).
     *
     * <p>createInfoBytes carries CNA_AvailableNetworkSessionCreateInfo in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     *   <li>{@code reserved[3]} (uint8_t)</li>
     *   <li>{@code reserved[4]} (uint8_t)</li>
     *   <li>{@code reserved[5]} (uint8_t)</li>
     * </ol>
     *
     * <p>createInfoIntegral carries CNA_AvailableNetworkSessionCreateInfo in this order:
     * <ol start="0">
     *   <li>{@code current_gamer_count} (int32_t)</li>
     *   <li>{@code open_private_gamer_slots} (int32_t)</li>
     *   <li>{@code open_public_gamer_slots} (int32_t)</li>
     *   <li>{@code session_type} (CNA_NetworkSessionType)</li>
     *   <li>{@code host_port} (uint16_t)</li>
     *   <li>{@code session_properties} (CNA_NetworkSessionPropertiesHandle)</li>
     * </ol>
     *
     * <p>createInfoHostGamertag carries CNA_AvailableNetworkSessionCreateInfo.host_gamertag as UTF-8 bytes, borrowed for the call.
     *
     * <p>createInfoHostAddress carries CNA_AvailableNetworkSessionCreateInfo.host_address as UTF-8 bytes, borrowed for the call.
     *
     * <p>qualityOfServiceBytes carries CNA_QualityOfService in this order:
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
     * <p>qualityOfServiceIntegral carries CNA_QualityOfService in this order:
     * <ol start="0">
     *   <li>{@code is_available} (CNA_Bool)</li>
     *   <li>{@code average_roundtrip_ticks} (int64_t)</li>
     *   <li>{@code minimum_roundtrip_ticks} (int64_t)</li>
     *   <li>{@code bytes_per_second_downstream} (int32_t)</li>
     *   <li>{@code bytes_per_second_upstream} (int32_t)</li>
     * </ol>
     */
    public static native int availableNetworkSessionCreateExt(byte[] createInfoBytes, long[] createInfoIntegral, byte[] createInfoHostGamertag, byte[] createInfoHostAddress, byte[] qualityOfServiceBytes, long[] qualityOfServiceIntegral, long[] outSession);

    /**
     * cna_available_network_session_destroy (net_sessions.h).
     */
    public static native int availableNetworkSessionDestroy(long session);

    /**
     * cna_available_network_session_get_connect_address_size_ext (net_sessions.h).
     */
    public static native int availableNetworkSessionGetConnectAddressSizeExt(long session, long[] outBytes);

    /**
     * cna_available_network_session_get_connect_port_ext (net_sessions.h).
     */
    public static native int availableNetworkSessionGetConnectPortExt(long session, int[] outValue);

    /**
     * cna_available_network_session_get_current_gamer_count (net_sessions.h).
     */
    public static native int availableNetworkSessionGetCurrentGamerCount(long session, int[] outValue);

    /**
     * cna_available_network_session_get_host_gamertag_size (net_sessions.h).
     */
    public static native int availableNetworkSessionGetHostGamertagSize(long session, long[] outBytes);

    /**
     * cna_available_network_session_get_open_private_gamer_slots (net_sessions.h).
     */
    public static native int availableNetworkSessionGetOpenPrivateGamerSlots(long session, int[] outValue);

    /**
     * cna_available_network_session_get_open_public_gamer_slots (net_sessions.h).
     */
    public static native int availableNetworkSessionGetOpenPublicGamerSlots(long session, int[] outValue);

    /**
     * cna_available_network_session_get_quality_of_service (net_sessions.h).
     *
     * <p>outValueBytes carries CNA_QualityOfService in this order:
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
     * <p>outValueIntegral carries CNA_QualityOfService in this order:
     * <ol start="0">
     *   <li>{@code is_available} (CNA_Bool)</li>
     *   <li>{@code average_roundtrip_ticks} (int64_t)</li>
     *   <li>{@code minimum_roundtrip_ticks} (int64_t)</li>
     *   <li>{@code bytes_per_second_downstream} (int32_t)</li>
     *   <li>{@code bytes_per_second_upstream} (int32_t)</li>
     * </ol>
     */
    public static native int availableNetworkSessionGetQualityOfService(long session, byte[] outValueBytes, long[] outValueIntegral);

    /**
     * cna_available_network_session_get_session_type_ext (net_sessions.h).
     */
    public static native int availableNetworkSessionGetSessionTypeExt(long session, int[] outValue);

    /**
     * cna_local_network_gamer_enable_send_voice (net_sessions.h).
     */
    public static native int localNetworkGamerEnableSendVoice(long gamer, long remoteGamer, boolean enable);

    /**
     * cna_local_network_gamer_get_is_data_available (net_sessions.h).
     */
    public static native int localNetworkGamerGetIsDataAvailable(long gamer, boolean[] outValue);

    /**
     * cna_local_network_gamer_get_signed_in_gamer (net_sessions.h).
     */
    public static native int localNetworkGamerGetSignedInGamer(long gamer, long[] outSignedInGamer);

    /**
     * cna_local_network_gamer_receive_data (net_sessions.h).
     */
    public static native int localNetworkGamerReceiveData(long gamer, byte[] destination, long[] outSender, long[] outReceived);

    /**
     * cna_local_network_gamer_receive_data_at (net_sessions.h).
     */
    public static native int localNetworkGamerReceiveDataAt(long gamer, byte[] destination, int offset, long[] outSender, long[] outReceived);

    /**
     * cna_local_network_gamer_send_data (net_sessions.h).
     */
    public static native int localNetworkGamerSendData(long gamer, byte[] data, int options);

    /**
     * cna_local_network_gamer_send_data_range (net_sessions.h).
     */
    public static native int localNetworkGamerSendDataRange(long gamer, byte[] data, int offset, int length, int options);

    /**
     * cna_local_network_gamer_send_data_range_to (net_sessions.h).
     */
    public static native int localNetworkGamerSendDataRangeTo(long gamer, byte[] data, int offset, int length, int options, long recipient);

    /**
     * cna_local_network_gamer_send_data_to (net_sessions.h).
     */
    public static native int localNetworkGamerSendDataTo(long gamer, byte[] data, int options, long recipient);

    /**
     * cna_local_network_gamer_send_party_invites (net_sessions.h).
     */
    public static native int localNetworkGamerSendPartyInvites(long gamer);

    /**
     * cna_net_get_last_join_error (net.h).
     */
    public static native int netGetLastJoinError(int[] outJoinError, boolean[] outHasJoinError);

    /**
     * cna_network_gamer_copy_machine (net_gamers.h).
     */
    public static native int networkGamerCopyMachine(long gamer, long[] outMachine);

    /**
     * cna_network_gamer_get_has_left_session (net_gamers.h).
     */
    public static native int networkGamerGetHasLeftSession(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_has_voice (net_gamers.h).
     */
    public static native int networkGamerGetHasVoice(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_id (net_gamers.h).
     */
    public static native int networkGamerGetId(long gamer, byte[] outValue);

    /**
     * cna_network_gamer_get_is_guest (net_gamers.h).
     */
    public static native int networkGamerGetIsGuest(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_host (net_gamers.h).
     */
    public static native int networkGamerGetIsHost(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_local (net_gamers.h).
     */
    public static native int networkGamerGetIsLocal(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_muted_by_local_user (net_gamers.h).
     */
    public static native int networkGamerGetIsMutedByLocalUser(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_private_slot (net_gamers.h).
     */
    public static native int networkGamerGetIsPrivateSlot(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_ready (net_gamers.h).
     */
    public static native int networkGamerGetIsReady(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_is_talking (net_gamers.h).
     */
    public static native int networkGamerGetIsTalking(long gamer, boolean[] outValue);

    /**
     * cna_network_gamer_get_roundtrip_ticks (net_gamers.h).
     */
    public static native int networkGamerGetRoundtripTicks(long gamer, long[] outTicks);

    /**
     * cna_network_gamer_get_session (net_gamers.h).
     */
    public static native int networkGamerGetSession(long gamer, long[] outSession);

    /**
     * cna_network_gamer_set_is_ready (net_gamers.h).
     */
    public static native int networkGamerSetIsReady(long gamer, boolean value);

    /**
     * cna_network_gamer_set_machine (net_gamers.h).
     */
    public static native int networkGamerSetMachine(long gamer, long machine);

    /**
     * cna_network_machine_get_gamer (net_gamers.h).
     */
    public static native int networkMachineGetGamer(long machine, int index, long[] outGamer);

    /**
     * cna_network_machine_get_gamer_count (net_gamers.h).
     */
    public static native int networkMachineGetGamerCount(long machine, int[] outCount);

    /**
     * cna_network_machine_remove_from_session (net_gamers.h).
     */
    public static native int networkMachineRemoveFromSession(long machine);

    /**
     * cna_network_session_add_local_gamer (net_sessions.h).
     */
    public static native int networkSessionAddLocalGamer(long session, long signedInGamer);

    /**
     * cna_network_session_copy_session_properties (net_sessions.h).
     */
    public static native int networkSessionCopySessionProperties(long session, long[] outProperties);

    /**
     * cna_network_session_create (net_sessions.h).
     */
    public static native int networkSessionCreate(int sessionType, int maxLocalGamers, int maxGamers, long[] outSession);

    /**
     * cna_network_session_create_with_local_gamers (net_sessions.h).
     */
    public static native int networkSessionCreateWithLocalGamers(int sessionType, long[] localGamers, int maxGamers, int privateGamerSlots, long sessionProperties, long[] outSession);

    /**
     * cna_network_session_create_with_properties (net_sessions.h).
     */
    public static native int networkSessionCreateWithProperties(int sessionType, int maxLocalGamers, int maxGamers, int privateGamerSlots, long sessionProperties, long[] outSession);

    /**
     * cna_network_session_destroy (net_sessions.h).
     */
    public static native int networkSessionDestroy(long session);

    /**
     * cna_network_session_dispose (net_sessions.h).
     */
    public static native int networkSessionDispose(long session);

    /**
     * cna_network_session_end_game (net_sessions.h).
     */
    public static native int networkSessionEndGame(long session);

    /**
     * cna_network_session_find (net_sessions.h).
     */
    public static native int networkSessionFind(int sessionType, int maxLocalGamers, long searchProperties, long[] outCollection);

    /**
     * cna_network_session_find_gamer_by_id (net_sessions.h).
     */
    public static native int networkSessionFindGamerById(long session, byte gamerId, long[] outGamer);

    /**
     * cna_network_session_find_with_local_gamers (net_sessions.h).
     */
    public static native int networkSessionFindWithLocalGamers(int sessionType, long[] localGamers, long searchProperties, long[] outCollection);

    /**
     * cna_network_session_get_allow_host_migration (net_sessions.h).
     */
    public static native int networkSessionGetAllowHostMigration(long session, boolean[] outValue);

    /**
     * cna_network_session_get_allow_join_in_progress (net_sessions.h).
     */
    public static native int networkSessionGetAllowJoinInProgress(long session, boolean[] outValue);

    /**
     * cna_network_session_get_bytes_per_second_received (net_sessions.h).
     */
    public static native int networkSessionGetBytesPerSecondReceived(long session, int[] outValue);

    /**
     * cna_network_session_get_bytes_per_second_sent (net_sessions.h).
     */
    public static native int networkSessionGetBytesPerSecondSent(long session, int[] outValue);

    /**
     * cna_network_session_get_gamer (net_sessions.h).
     */
    public static native int networkSessionGetGamer(long session, int roster, int index, long[] outGamer);

    /**
     * cna_network_session_get_gamer_count (net_sessions.h).
     */
    public static native int networkSessionGetGamerCount(long session, int roster, int[] outCount);

    /**
     * cna_network_session_get_host (net_sessions.h).
     */
    public static native int networkSessionGetHost(long session, long[] outGamer);

    /**
     * cna_network_session_get_is_disposed (net_sessions.h).
     */
    public static native int networkSessionGetIsDisposed(long session, boolean[] outIsDisposed);

    /**
     * cna_network_session_get_is_everyone_ready (net_sessions.h).
     */
    public static native int networkSessionGetIsEveryoneReady(long session, boolean[] outValue);

    /**
     * cna_network_session_get_is_host (net_sessions.h).
     */
    public static native int networkSessionGetIsHost(long session, boolean[] outValue);

    /**
     * cna_network_session_get_max_gamers (net_sessions.h).
     */
    public static native int networkSessionGetMaxGamers(long session, int[] outValue);

    /**
     * cna_network_session_get_private_gamer_slots (net_sessions.h).
     */
    public static native int networkSessionGetPrivateGamerSlots(long session, int[] outValue);

    /**
     * cna_network_session_get_session_state (net_sessions.h).
     */
    public static native int networkSessionGetSessionState(long session, int[] outValue);

    /**
     * cna_network_session_get_session_type (net_sessions.h).
     */
    public static native int networkSessionGetSessionType(long session, int[] outValue);

    /**
     * cna_network_session_get_simulated_latency_ticks (net_sessions.h).
     */
    public static native int networkSessionGetSimulatedLatencyTicks(long session, long[] outTicks);

    /**
     * cna_network_session_get_simulated_packet_loss (net_sessions.h).
     */
    public static native int networkSessionGetSimulatedPacketLoss(long session, float[] outValue);

    /**
     * cna_network_session_join (net_sessions.h).
     */
    public static native int networkSessionJoin(long availableSession, long[] outSession);

    /**
     * cna_network_session_join_invited (net_sessions.h).
     */
    public static native int networkSessionJoinInvited(int maxLocalGamers, long[] outSession);

    /**
     * cna_network_session_join_invited_with_local_gamers (net_sessions.h).
     */
    public static native int networkSessionJoinInvitedWithLocalGamers(long[] localGamers, long[] outSession);

    /**
     * cna_network_session_properties_contains (net.h).
     */
    public static native int networkSessionPropertiesContains(long properties, byte[] valueBytes, long[] valueIntegral, boolean[] outContains);

    /**
     * cna_network_session_properties_copy_to (net.h).
     */
    public static native int networkSessionPropertiesCopyTo(long properties, byte[] destinationBytes, long[] destinationIntegral, int index, long[] outCount);

    /**
     * cna_network_session_properties_create (net.h).
     */
    public static native int networkSessionPropertiesCreate(long[] outProperties);

    /**
     * cna_network_session_properties_destroy (net.h).
     */
    public static native int networkSessionPropertiesDestroy(long properties);

    /**
     * cna_network_session_properties_get_count (net.h).
     */
    public static native int networkSessionPropertiesGetCount(long properties, int[] outCount);

    /**
     * cna_network_session_properties_get_item (net.h).
     *
     * <p>outValueBytes carries CNA_OptionalInt32 in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outValueIntegral carries CNA_OptionalInt32 in this order:
     * <ol start="0">
     *   <li>{@code has_value} (CNA_Bool)</li>
     *   <li>{@code value} (int32_t)</li>
     * </ol>
     */
    public static native int networkSessionPropertiesGetItem(long properties, int index, byte[] outValueBytes, long[] outValueIntegral);

    /**
     * cna_network_session_properties_index_of (net.h).
     */
    public static native int networkSessionPropertiesIndexOf(long properties, byte[] valueBytes, long[] valueIntegral, int[] outIndex);

    /**
     * cna_network_session_properties_set_item (net.h).
     */
    public static native int networkSessionPropertiesSetItem(long properties, int index, byte[] valueBytes, long[] valueIntegral);

    /**
     * cna_network_session_reset_ready (net_sessions.h).
     */
    public static native int networkSessionResetReady(long session);

    /**
     * cna_network_session_set_allow_host_migration (net_sessions.h).
     */
    public static native int networkSessionSetAllowHostMigration(long session, boolean value);

    /**
     * cna_network_session_set_allow_join_in_progress (net_sessions.h).
     */
    public static native int networkSessionSetAllowJoinInProgress(long session, boolean value);

    /**
     * cna_network_session_set_max_gamers (net_sessions.h).
     */
    public static native int networkSessionSetMaxGamers(long session, int value);

    /**
     * cna_network_session_set_private_gamer_slots (net_sessions.h).
     */
    public static native int networkSessionSetPrivateGamerSlots(long session, int value);

    /**
     * cna_network_session_set_simulated_latency_ticks (net_sessions.h).
     */
    public static native int networkSessionSetSimulatedLatencyTicks(long session, long ticks);

    /**
     * cna_network_session_set_simulated_packet_loss (net_sessions.h).
     */
    public static native int networkSessionSetSimulatedPacketLoss(long session, float value);

    /**
     * cna_network_session_start_game (net_sessions.h).
     */
    public static native int networkSessionStartGame(long session);

    /**
     * cna_network_session_unsubscribe (net_sessions.h).
     */
    public static native int networkSessionUnsubscribe(long registration);

    /**
     * cna_network_session_update (net_sessions.h).
     */
    public static native int networkSessionUpdate(long session);
}
