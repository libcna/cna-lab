/* SPDX-License-Identifier: MS-PL */
/*
 * Hand-written JNI for CNA's gamer-service and network-session events.
 *
 * These are the routes the generator refuses on purpose: each takes a real C callback, and a
 * generated wrapper cannot decide what that callback should do. What it does here is the same
 * thing the media and storage families already do -- record the event and let Java drain it --
 * rather than calling into the JVM from whichever thread CNA happened to raise it on.
 *
 * The queue is process-wide because the sign-in and invite events are: CNA raises them without
 * a session. Java drains it immediately after pumping the dispatcher or the session, so an
 * event is delivered on the game thread, during Update, which is where XNA raises it too.
 *
 * A full queue drops the oldest record and counts the drop. Growing without bound in a callback
 * CNA may raise from any thread would be the wrong trade, and reporting the loss is better than
 * hiding it.
 */

#define CNA_JAVA_EVENT_CAPACITY 256

/* Event kinds. Java reads these from NativeGamerServices, which declares the same numbers. */
#define CNA_JAVA_EVENT_SIGNED_IN 0
#define CNA_JAVA_EVENT_SIGNED_OUT 1
#define CNA_JAVA_EVENT_INSTALLING_TITLE_UPDATE 2
#define CNA_JAVA_EVENT_GAME_ENDED 10
#define CNA_JAVA_EVENT_GAME_STARTED 11
#define CNA_JAVA_EVENT_GAMER_JOINED 12
#define CNA_JAVA_EVENT_GAMER_LEFT 13
#define CNA_JAVA_EVENT_HOST_CHANGED 14
#define CNA_JAVA_EVENT_SESSION_ENDED 15
#define CNA_JAVA_EVENT_WRITE_ARBITRATED 16
#define CNA_JAVA_EVENT_WRITE_TRUE_SKILL 17
#define CNA_JAVA_EVENT_WRITE_UNARBITRATED 18
#define CNA_JAVA_EVENT_INVITE_ACCEPTED 19
#define CNA_JAVA_EVENT_TEXT_INPUT 30
#define CNA_JAVA_EVENT_MOUSE_CONNECTED 31
#define CNA_JAVA_EVENT_MOUSE_DISCONNECTED 32
#define CNA_JAVA_EVENT_KEYBOARD_CONNECTED 33
#define CNA_JAVA_EVENT_KEYBOARD_DISCONNECTED 34
#define CNA_JAVA_EVENT_JOYSTICK_CONNECTED 35
#define CNA_JAVA_EVENT_JOYSTICK_DISCONNECTED 36
#define CNA_JAVA_EVENT_MOUSE_CLICKED 37

typedef struct JavaGamerEvent {
    int64_t sequence;
    int64_t kind;
    int64_t session;
    int64_t first;
    int64_t second;
    int64_t flag;
} JavaGamerEvent;

static JavaGamerEvent java_gamer_events[CNA_JAVA_EVENT_CAPACITY];
static int java_gamer_event_head;
static int java_gamer_event_count;
static int64_t java_gamer_events_dropped;
static atomic_flag java_gamer_event_lock = ATOMIC_FLAG_INIT;

/*
 * One counter across both queues.
 *
 * The numeric queue and the string queue below hold different record shapes, so they cannot be
 * one array; but a committed character and the composition update that cleared it are two events
 * a game must see in the order CNA raised them. Every push stamps a sequence from this counter
 * and Java drains the two queues merged by it, which keeps one order out of two queues.
 */
static _Atomic int64_t java_event_sequence;

static int64_t java_next_sequence(void)
{
    return atomic_fetch_add(&java_event_sequence, 1);
}

static void java_gamer_event_enter(void)
{
    while (atomic_flag_test_and_set(&java_gamer_event_lock)) {
        /* The critical section is a handful of stores; spinning is cheaper than a mutex. */
    }
}

static void java_gamer_event_leave(void)
{
    atomic_flag_clear(&java_gamer_event_lock);
}

static void java_gamer_event_push(
    int64_t kind, int64_t session, int64_t first, int64_t second, int64_t flag)
{
    int64_t sequence = java_next_sequence();
    java_gamer_event_enter();
    if (java_gamer_event_count == CNA_JAVA_EVENT_CAPACITY) {
        java_gamer_event_head = (java_gamer_event_head + 1) % CNA_JAVA_EVENT_CAPACITY;
        java_gamer_event_count--;
        java_gamer_events_dropped++;
    }
    int index = (java_gamer_event_head + java_gamer_event_count) % CNA_JAVA_EVENT_CAPACITY;
    java_gamer_events[index].sequence = sequence;
    java_gamer_events[index].kind = kind;
    java_gamer_events[index].session = session;
    java_gamer_events[index].first = first;
    java_gamer_events[index].second = second;
    java_gamer_events[index].flag = flag;
    java_gamer_event_count++;
    java_gamer_event_leave();
}

static void java_signed_in_callback(void* context, const CNA_SignedInGamerEventInfo* info)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_SIGNED_IN, 0,
        info == NULL ? 0 : (int64_t)info->gamer, 0, 0);
}

static void java_signed_out_callback(void* context, const CNA_SignedInGamerEventInfo* info)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_SIGNED_OUT, 0,
        info == NULL ? 0 : (int64_t)info->gamer, 0, 0);
}

static void java_installing_title_update_callback(void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_INSTALLING_TITLE_UPDATE, 0, 0, 0, 0);
}

static void java_game_ended_callback(
    CNA_NetworkSessionHandle session, const CNA_GameEndedEventInfo* info, void* context)
{
    (void)info;
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_GAME_ENDED, (int64_t)session, 0, 0, 0);
}

static void java_game_started_callback(
    CNA_NetworkSessionHandle session, const CNA_GameStartedEventInfo* info, void* context)
{
    (void)info;
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_GAME_STARTED, (int64_t)session, 0, 0, 0);
}

static void java_gamer_joined_callback(
    CNA_NetworkSessionHandle session, const CNA_GamerJoinedEventInfo* info, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_GAMER_JOINED, (int64_t)session,
        info == NULL ? 0 : (int64_t)info->gamer, 0, 0);
}

static void java_gamer_left_callback(
    CNA_NetworkSessionHandle session, const CNA_GamerLeftEventInfo* info, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_GAMER_LEFT, (int64_t)session,
        info == NULL ? 0 : (int64_t)info->gamer, 0, 0);
}

static void java_host_changed_callback(
    CNA_NetworkSessionHandle session, const CNA_HostChangedEventInfo* info, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_HOST_CHANGED, (int64_t)session,
        info == NULL ? 0 : (int64_t)info->old_host,
        info == NULL ? 0 : (int64_t)info->new_host, 0);
}

static void java_session_ended_callback(
    CNA_NetworkSessionHandle session, const CNA_NetworkSessionEndedEventInfo* info, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_SESSION_ENDED, (int64_t)session, 0, 0,
        info == NULL ? 0 : (int64_t)info->end_reason);
}

static void java_write_leaderboards_push(
    int64_t kind, CNA_NetworkSessionHandle session, const CNA_WriteLeaderboardsEventInfo* info)
{
    java_gamer_event_push(kind, (int64_t)session,
        info == NULL ? 0 : (int64_t)info->gamer, 0,
        info == NULL ? 0 : (int64_t)info->is_leaving);
}

static void java_write_arbitrated_callback(
    CNA_NetworkSessionHandle session, const CNA_WriteLeaderboardsEventInfo* info, void* context)
{
    (void)context;
    java_write_leaderboards_push(CNA_JAVA_EVENT_WRITE_ARBITRATED, session, info);
}

static void java_write_true_skill_callback(
    CNA_NetworkSessionHandle session, const CNA_WriteLeaderboardsEventInfo* info, void* context)
{
    (void)context;
    java_write_leaderboards_push(CNA_JAVA_EVENT_WRITE_TRUE_SKILL, session, info);
}

static void java_write_unarbitrated_callback(
    CNA_NetworkSessionHandle session, const CNA_WriteLeaderboardsEventInfo* info, void* context)
{
    (void)context;
    java_write_leaderboards_push(CNA_JAVA_EVENT_WRITE_UNARBITRATED, session, info);
}

static void java_invite_accepted_callback(
    const CNA_InviteAcceptedEventInfo* info, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_INVITE_ACCEPTED, 0,
        info == NULL ? 0 : (int64_t)info->gamer, 0,
        info == NULL ? 0 : (int64_t)info->is_current_session);
}

/*
 * The string-carrying text events.
 *
 * The numeric record cannot hold a string, and CNA's composition and candidate views are
 * borrowed only for the duration of the callback: the header says in as many words that the
 * text must be copied before the callback returns. So this queue owns heap copies made inside
 * the callback, and hands them to Java as byte arrays in one call that frees the record before
 * it returns. Java never sees a native pointer, and a Java exception on the way out cannot
 * strand one.
 *
 * The copy is UTF-8 bytes with an explicit length, because CNA's views are not NUL-terminated
 * and a composition may legitimately contain a NUL-free but non-ASCII draft of any length.
 */
#define CNA_JAVA_TEXT_EVENT_CAPACITY 64

#define CNA_JAVA_TEXT_EVENT_COMPOSITION 0
#define CNA_JAVA_TEXT_EVENT_CANDIDATES 1

typedef struct JavaTextEvent {
    int64_t sequence;
    int32_t kind;
    int32_t start;
    int32_t length;
    int32_t selected;
    int32_t horizontal;
    int32_t count;
    char** texts;
    uint64_t* sizes;
} JavaTextEvent;

static JavaTextEvent java_text_events[CNA_JAVA_TEXT_EVENT_CAPACITY];
static int java_text_event_head;
static int java_text_event_count;
static int64_t java_text_events_dropped;
static atomic_flag java_text_event_lock = ATOMIC_FLAG_INIT;

static void java_text_event_enter(void)
{
    while (atomic_flag_test_and_set(&java_text_event_lock)) {
        /* As with the numeric queue, the critical section is a handful of stores. */
    }
}

static void java_text_event_leave(void)
{
    atomic_flag_clear(&java_text_event_lock);
}

/** Frees one record's owned copies. Safe on a zeroed record. */
static void java_text_event_release(JavaTextEvent* event)
{
    if (event->texts != NULL) {
        for (int32_t index = 0; index < event->count; ++index) {
            free(event->texts[index]);
        }
        free(event->texts);
    }
    free(event->sizes);
    event->texts = NULL;
    event->sizes = NULL;
    event->count = 0;
}

/**
 * Copies count borrowed views into a new record and enqueues it.
 *
 * Every allocation happens before the lock is taken, so a slow allocator never blocks the
 * thread CNA raised the event on for longer than it must. A failed allocation drops the event
 * and counts the drop rather than enqueueing half a record.
 */
static void java_text_event_push(
    int32_t kind, const CNA_StringView* views, int32_t count,
    int32_t start, int32_t length, int32_t selected, int32_t horizontal)
{
    JavaTextEvent event;
    memset(&event, 0, sizeof event);
    event.kind = kind;
    event.start = start;
    event.length = length;
    event.selected = selected;
    event.horizontal = horizontal;
    event.count = count;
    if (count > 0) {
        event.texts = (char**)calloc((size_t)count, sizeof(*event.texts));
        event.sizes = (uint64_t*)calloc((size_t)count, sizeof(*event.sizes));
        if (event.texts == NULL || event.sizes == NULL) {
            java_text_event_release(&event);
            java_text_event_enter();
            java_text_events_dropped++;
            java_text_event_leave();
            return;
        }
        for (int32_t index = 0; index < count; ++index) {
            uint64_t size = views[index].data == NULL ? 0 : views[index].byte_length;
            /* One spare byte so an empty view still gets a distinct allocation. */
            event.texts[index] = (char*)malloc((size_t)size + 1U);
            if (event.texts[index] == NULL) {
                event.count = index;
                java_text_event_release(&event);
                java_text_event_enter();
                java_text_events_dropped++;
                java_text_event_leave();
                return;
            }
            if (size > 0) {
                memcpy(event.texts[index], views[index].data, (size_t)size);
            }
            event.sizes[index] = size;
        }
    }
    event.sequence = java_next_sequence();

    java_text_event_enter();
    if (java_text_event_count == CNA_JAVA_TEXT_EVENT_CAPACITY) {
        /* Drop the oldest, and free what it owned: growing without bound in a callback CNA
         * may raise from any thread would be the wrong trade, and leaking would be worse. */
        java_text_event_release(&java_text_events[java_text_event_head]);
        java_text_event_head = (java_text_event_head + 1) % CNA_JAVA_TEXT_EVENT_CAPACITY;
        java_text_event_count--;
        java_text_events_dropped++;
    }
    int index = (java_text_event_head + java_text_event_count) % CNA_JAVA_TEXT_EVENT_CAPACITY;
    java_text_events[index] = event;
    java_text_event_count++;
    java_text_event_leave();
}

static void java_text_editing_callback(const CNA_TextEditingEventInfo* info, void* context)
{
    (void)context;
    if (info == NULL) {
        return;
    }
    java_text_event_push(CNA_JAVA_TEXT_EVENT_COMPOSITION, &info->text, 1,
        info->start, info->length, 0, 0);
}

static void java_text_editing_candidates_callback(
    const CNA_TextEditingCandidatesEventInfo* info, void* context)
{
    (void)context;
    if (info == NULL) {
        return;
    }
    int32_t count = info->candidate_count < 0 ? 0 : info->candidate_count;
    java_text_event_push(CNA_JAVA_TEXT_EVENT_CANDIDATES,
        count == 0 ? NULL : info->candidates, count,
        0, 0, info->selected, info->horizontal != CNA_FALSE ? 1 : 0);
}

/** Subscribes both string-carrying text events, or neither. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeTextComposition(
    JNIEnv* environment, jclass type, jlongArray out_registrations)
{
    (void)type;
    CNA_TextInputRegistrationHandle registrations[2];
    registrations[0] = CNA_INVALID_HANDLE;
    registrations[1] = CNA_INVALID_HANDLE;
    CNA_Result result = cna.text_input_subscribe_text_editing_ext(
        java_text_editing_callback, NULL, &registrations[0]);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.text_input_subscribe_text_editing_candidates_ext(
            java_text_editing_candidates_callback, NULL, &registrations[1]);
    }
    if (result != CNA_RESULT_SUCCESS) {
        for (int index = 1; index >= 0; --index) {
            if (registrations[index] != CNA_INVALID_HANDLE) {
                (void)cna.text_input_unsubscribe_ext(registrations[index]);
            }
        }
        return (jint)result;
    }
    jlong values[2] = {(jlong)registrations[0], (jlong)registrations[1]};
    (*environment)->SetLongArrayRegion(environment, out_registrations, 0, 2, values);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases both composition registrations, in reverse subscription order. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeTextComposition(
    JNIEnv* environment, jclass type, jlongArray registrations)
{
    (void)type;
    jlong values[2];
    (*environment)->GetLongArrayRegion(environment, registrations, 0, 2, values);
    CNA_Result first = CNA_RESULT_SUCCESS;
    for (int index = 1; index >= 0; --index) {
        if (values[index] == 0) {
            continue;
        }
        CNA_Result result = cna.text_input_unsubscribe_ext(
            (CNA_TextInputRegistrationHandle)values[index]);
        if (result != CNA_RESULT_SUCCESS && first == CNA_RESULT_SUCCESS) {
            first = result;
        }
    }
    return (jint)first;
}

/*
 * Drains one string-carrying event.
 *
 * Returns null when the queue is empty; otherwise an array of UTF-8 byte arrays and a header of
 * {sequence, kind, start, length, selected, horizontal}. The C record is freed before this
 * returns whatever happens, so nothing Java does afterwards -- including throwing -- can strand
 * a copy or leave a pointer Java could reach.
 */
JNIEXPORT jobjectArray JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativePollTextEvent(
    JNIEnv* environment, jclass type, jlongArray out_header)
{
    (void)type;
    JavaTextEvent event;
    java_text_event_enter();
    if (java_text_event_count == 0) {
        java_text_event_leave();
        return NULL;
    }
    event = java_text_events[java_text_event_head];
    memset(&java_text_events[java_text_event_head], 0, sizeof(JavaTextEvent));
    java_text_event_head = (java_text_event_head + 1) % CNA_JAVA_TEXT_EVENT_CAPACITY;
    java_text_event_count--;
    java_text_event_leave();

    jobjectArray payload = NULL;
    jclass byte_array_class = (*environment)->FindClass(environment, "[B");
    if (byte_array_class != NULL) {
        payload = (*environment)->NewObjectArray(
            environment, (jsize)event.count, byte_array_class, NULL);
    }
    for (int32_t index = 0; payload != NULL && index < event.count; ++index) {
        jbyteArray text = (*environment)->NewByteArray(
            environment, (jsize)event.sizes[index]);
        if (text == NULL) {
            payload = NULL;
            break;
        }
        if (event.sizes[index] > 0) {
            (*environment)->SetByteArrayRegion(environment, text, 0,
                (jsize)event.sizes[index], (const jbyte*)event.texts[index]);
        }
        (*environment)->SetObjectArrayElement(environment, payload, (jsize)index, text);
        (*environment)->DeleteLocalRef(environment, text);
    }
    java_text_event_release(&event);
    if (payload == NULL) {
        /* A failed JVM allocation leaves its own pending exception; the record is already
         * freed, so the loss is reported to Java rather than hidden. */
        return NULL;
    }

    jlong header[] = {event.sequence, event.kind, event.start, event.length,
        event.selected, event.horizontal};
    (*environment)->SetLongArrayRegion(environment, out_header, 0, 6, header);
    return payload;
}

/*
 * Raises CNA's candidate-list event from Java.
 *
 * The generator refuses this route on purpose: it takes an array of CNA_StringView, and each
 * view has to point at bytes that stay alive across the call. The views are built here over the
 * pinned Java arrays and released afterwards, so nothing outlives the dispatch -- which is safe
 * because CNA documents that it copies every candidate before dispatching.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeRaiseTextCandidates(
    JNIEnv* environment, jclass type, jlong game, jobjectArray candidates,
    jint selected, jboolean horizontal)
{
    (void)type;
    jsize count = (*environment)->GetArrayLength(environment, candidates);
    CNA_StringView* views = NULL;
    jbyteArray* arrays = NULL;
    jbyte** bytes = NULL;
    if (count > 0) {
        views = (CNA_StringView*)calloc((size_t)count, sizeof(*views));
        arrays = (jbyteArray*)calloc((size_t)count, sizeof(*arrays));
        bytes = (jbyte**)calloc((size_t)count, sizeof(*bytes));
        if (views == NULL || arrays == NULL || bytes == NULL) {
            free(views);
            free(arrays);
            free(bytes);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
    }
    CNA_Result result = CNA_RESULT_SUCCESS;
    jsize pinned = 0;
    for (; pinned < count; ++pinned) {
        arrays[pinned] = (jbyteArray)(*environment)->GetObjectArrayElement(
            environment, candidates, pinned);
        if (arrays[pinned] == NULL) {
            result = CNA_RESULT_INVALID_ARGUMENT;
            break;
        }
        bytes[pinned] = (*environment)->GetByteArrayElements(environment, arrays[pinned], NULL);
        if (bytes[pinned] == NULL) {
            result = CNA_RESULT_OUT_OF_MEMORY;
            break;
        }
        views[pinned].data = (const char*)bytes[pinned];
        views[pinned].byte_length =
            (uint64_t)(*environment)->GetArrayLength(environment, arrays[pinned]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        /* The caller already resolved the CNA game handle, exactly as a generated route
         * receives it, so this takes it directly rather than unwrapping an adapter token. */
        result = cna.text_input_raise_text_editing_candidates_ext(
            (CNA_Handle)game, count == 0 ? NULL : views, (int32_t)count,
            (int32_t)selected, horizontal == JNI_TRUE ? CNA_TRUE : CNA_FALSE);
    }
    for (jsize index = 0; index < pinned; ++index) {
        if (bytes[index] != NULL) {
            (*environment)->ReleaseByteArrayElements(
                environment, arrays[index], bytes[index], JNI_ABORT);
        }
        (*environment)->DeleteLocalRef(environment, arrays[index]);
    }
    free(views);
    free(arrays);
    free(bytes);
    return (jint)result;
}

/** Returns how many string events were dropped, for diagnostics. */
JNIEXPORT jlong JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeDroppedTextEventCount(
    JNIEnv* environment, jclass type)
{
    (void)environment;
    (void)type;
    java_text_event_enter();
    int64_t dropped = java_text_events_dropped;
    java_text_event_leave();
    return (jlong)dropped;
}

/** Discards every queued string event, freeing what each one owns. */
JNIEXPORT void JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeResetTextEvents(
    JNIEnv* environment, jclass type)
{
    (void)environment;
    (void)type;
    java_text_event_enter();
    while (java_text_event_count > 0) {
        java_text_event_release(&java_text_events[java_text_event_head]);
        java_text_event_head = (java_text_event_head + 1) % CNA_JAVA_TEXT_EVENT_CAPACITY;
        java_text_event_count--;
    }
    java_text_event_head = 0;
    java_text_events_dropped = 0;
    java_text_event_leave();
}

static void java_text_input_callback(uint16_t code_unit, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_TEXT_INPUT, 0, (int64_t)code_unit, 0, 0);
}

/*
 * The four mouse and keyboard hot-plug callbacks.
 *
 * CNA's hot-plug events are static, so these subscriptions belong to the process rather than to a
 * game, and the identifier each one carries is the only payload. That fits the numeric event
 * record exactly, so they ride the same queue and the same drain as every other event here and
 * arrive on the game thread during Update.
 */
static void java_mouse_connected_callback(uint32_t device_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_MOUSE_CONNECTED, 0, (int64_t)device_id, 0, 0);
}

static void java_mouse_disconnected_callback(uint32_t device_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_MOUSE_DISCONNECTED, 0, (int64_t)device_id, 0, 0);
}

static void java_keyboard_connected_callback(uint32_t device_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_KEYBOARD_CONNECTED, 0, (int64_t)device_id, 0, 0);
}

static void java_keyboard_disconnected_callback(uint32_t device_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_KEYBOARD_DISCONNECTED, 0, (int64_t)device_id, 0, 0);
}

/*
 * Subscribes the four mouse and keyboard hot-plug events, returning their registrations.
 *
 * All four or none, for the same reason the session events are all or none: a caller that got
 * some of them could not tell which it was missing.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeInputDeviceEvents(
    JNIEnv* environment, jclass type, jlongArray out_registrations)
{
    (void)type;
    CNA_InputDeviceEventRegistrationHandle registrations[4];
    for (int index = 0; index < 4; ++index) {
        registrations[index] = CNA_INVALID_HANDLE;
    }
    CNA_Result result = cna.input_devices_subscribe_mouse_connected_ext(
        java_mouse_connected_callback, NULL, &registrations[0]);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.input_devices_subscribe_mouse_disconnected_ext(
            java_mouse_disconnected_callback, NULL, &registrations[1]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.input_devices_subscribe_keyboard_connected_ext(
            java_keyboard_connected_callback, NULL, &registrations[2]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.input_devices_subscribe_keyboard_disconnected_ext(
            java_keyboard_disconnected_callback, NULL, &registrations[3]);
    }
    if (result != CNA_RESULT_SUCCESS) {
        for (int index = 3; index >= 0; --index) {
            if (registrations[index] != CNA_INVALID_HANDLE) {
                (void)cna.input_devices_unsubscribe_ext(registrations[index]);
            }
        }
        return (jint)result;
    }
    jlong values[4];
    for (int index = 0; index < 4; ++index) {
        values[index] = (jlong)registrations[index];
    }
    (*environment)->SetLongArrayRegion(environment, out_registrations, 0, 4, values);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases the four hot-plug registrations, in reverse subscription order. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeInputDeviceEvents(
    JNIEnv* environment, jclass type, jlongArray registrations)
{
    (void)type;
    jlong values[4];
    (*environment)->GetLongArrayRegion(environment, registrations, 0, 4, values);
    CNA_Result first = CNA_RESULT_SUCCESS;
    for (int index = 3; index >= 0; --index) {
        if (values[index] == 0) {
            continue;
        }
        CNA_Result result = cna.input_devices_unsubscribe_ext(
            (CNA_InputDeviceEventRegistrationHandle)values[index]);
        if (result != CNA_RESULT_SUCCESS && first == CNA_RESULT_SUCCESS) {
            first = result;
        }
    }
    return (jint)first;
}

/*
 * The two raw-joystick hot-plug callbacks.
 *
 * Same shape as the mouse and keyboard ones: a static CNA event carrying only an identifier, so
 * it rides the numeric record and the existing drain.
 */
static void java_joystick_connected_callback(uint32_t joystick_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_JOYSTICK_CONNECTED, 0, (int64_t)joystick_id, 0, 0);
}

static void java_joystick_disconnected_callback(uint32_t joystick_id, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_JOYSTICK_DISCONNECTED, 0, (int64_t)joystick_id, 0, 0);
}

/** Subscribes both raw-joystick hot-plug events, or neither. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeJoystickEvents(
    JNIEnv* environment, jclass type, jlongArray out_registrations)
{
    (void)type;
    CNA_JoystickEventRegistrationHandle registrations[2];
    registrations[0] = CNA_INVALID_HANDLE;
    registrations[1] = CNA_INVALID_HANDLE;
    CNA_Result result = cna.joysticks_subscribe_connected_ext(
        java_joystick_connected_callback, NULL, &registrations[0]);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.joysticks_subscribe_disconnected_ext(
            java_joystick_disconnected_callback, NULL, &registrations[1]);
    }
    if (result != CNA_RESULT_SUCCESS) {
        for (int index = 1; index >= 0; --index) {
            if (registrations[index] != CNA_INVALID_HANDLE) {
                (void)cna.joysticks_unsubscribe_ext(registrations[index]);
            }
        }
        return (jint)result;
    }
    jlong values[2] = {(jlong)registrations[0], (jlong)registrations[1]};
    (*environment)->SetLongArrayRegion(environment, out_registrations, 0, 2, values);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases both raw-joystick hot-plug registrations, in reverse subscription order. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeJoystickEvents(
    JNIEnv* environment, jclass type, jlongArray registrations)
{
    (void)type;
    jlong values[2];
    (*environment)->GetLongArrayRegion(environment, registrations, 0, 2, values);
    CNA_Result first = CNA_RESULT_SUCCESS;
    for (int index = 1; index >= 0; --index) {
        if (values[index] == 0) {
            continue;
        }
        CNA_Result result = cna.joysticks_unsubscribe_ext(
            (CNA_JoystickEventRegistrationHandle)values[index]);
        if (result != CNA_RESULT_SUCCESS && first == CNA_RESULT_SUCCESS) {
            first = result;
        }
    }
    return (jint)first;
}

/*
 * CNA's mouse-click event.
 *
 * The payload is the button index, so this rides the numeric record like the hot-plug events.
 */
static void java_mouse_clicked_callback(int32_t button, void* context)
{
    (void)context;
    java_gamer_event_push(CNA_JAVA_EVENT_MOUSE_CLICKED, 0, (int64_t)button, 0, 0);
}

/** Subscribes CNA's mouse-click event. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeMouseClicked(
    JNIEnv* environment, jclass type, jlongArray out_registration)
{
    (void)type;
    CNA_MouseEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_Result result = cna.mouse_subscribe_clicked_ext(
        java_mouse_clicked_callback, NULL, &registration);
    if (result != CNA_RESULT_SUCCESS) {
        return (jint)result;
    }
    jlong value = (jlong)registration;
    (*environment)->SetLongArrayRegion(environment, out_registration, 0, 1, &value);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases the mouse-click subscription. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeMouseClicked(
    JNIEnv* environment, jclass type, jlong registration)
{
    (void)environment;
    (void)type;
    return (jint)cna.mouse_unsubscribe_clicked_ext(
        (CNA_MouseEventRegistrationHandle)registration);
}

/*
 * Subscribes CNA's typed-character event.
 *
 * Composition text and candidate lists are deliberately not here: each carries a string, which
 * the fixed event record cannot hold, and inventing a second transport for them belongs with the
 * work that projects them rather than with this one.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeTextInput(
    JNIEnv* environment, jclass type, jlongArray out_registration)
{
    (void)type;
    CNA_TextInputRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_Result result = cna.text_input_subscribe_text_input_ext(
        java_text_input_callback, NULL, &registration);
    if (result != CNA_RESULT_SUCCESS) {
        return (jint)result;
    }
    jlong value = (jlong)registration;
    (*environment)->SetLongArrayRegion(environment, out_registration, 0, 1, &value);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases the typed-character subscription. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeTextInput(
    JNIEnv* environment, jclass type, jlong registration)
{
    (void)environment;
    (void)type;
    return (jint)cna.text_input_unsubscribe_ext(
        (CNA_TextInputRegistrationHandle)registration);
}

/*
 * Subscribes the three process-wide gamer-service events, returning their registrations.
 *
 * Java calls this once and keeps the registrations; a second call would deliver every event
 * twice, so the Java side, not this function, owns that decision.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeGamerEvents(
    JNIEnv* environment, jclass type, jlongArray out_registrations)
{
    (void)type;
    CNA_Handle signed_in = CNA_INVALID_HANDLE;
    CNA_Handle signed_out = CNA_INVALID_HANDLE;
    CNA_Handle title_update = CNA_INVALID_HANDLE;
    CNA_Result result = cna.signed_in_gamer_subscribe_signed_in_ext(
        java_signed_in_callback, NULL, &signed_in);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.signed_in_gamer_subscribe_signed_out_ext(
            java_signed_out_callback, NULL, &signed_out);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.gamer_services_dispatcher_subscribe_installing_title_update_ext(
            java_installing_title_update_callback, NULL, &title_update);
    }
    if (result != CNA_RESULT_SUCCESS) {
        /* Roll back whatever did register, so a partial subscription never survives. */
        if (title_update != CNA_INVALID_HANDLE) {
            (void)cna.gamer_unsubscribe_ext(title_update);
        }
        if (signed_out != CNA_INVALID_HANDLE) {
            (void)cna.gamer_unsubscribe_ext(signed_out);
        }
        if (signed_in != CNA_INVALID_HANDLE) {
            (void)cna.gamer_unsubscribe_ext(signed_in);
        }
        return (jint)result;
    }
    jlong registrations[] = {(jlong)signed_in, (jlong)signed_out, (jlong)title_update};
    (*environment)->SetLongArrayRegion(environment, out_registrations, 0, 3, registrations);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases the three process-wide gamer-service registrations. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeGamerEvents(
    JNIEnv* environment, jclass type, jlongArray registrations)
{
    (void)type;
    jlong values[3] = {0, 0, 0};
    (*environment)->GetLongArrayRegion(environment, registrations, 0, 3, values);
    CNA_Result first = CNA_RESULT_SUCCESS;
    for (int index = 2; index >= 0; --index) {
        if (values[index] == 0) {
            continue;
        }
        CNA_Result result = cna.gamer_unsubscribe_ext((CNA_Handle)values[index]);
        if (result != CNA_RESULT_SUCCESS && first == CNA_RESULT_SUCCESS) {
            first = result;
        }
    }
    return (jint)first;
}

/*
 * Subscribes all nine session events plus the static accepted-invite event.
 *
 * Every one of them or none: a session that raised only some of its events would be worse than
 * one that raised none, because a game cannot tell which it got.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeSubscribeSessionEvents(
    JNIEnv* environment, jclass type, jlong session, jlongArray out_registrations)
{
    (void)type;
    CNA_NetworkSessionHandle handle = (CNA_NetworkSessionHandle)session;
    CNA_NetworkSessionEventRegistrationHandle registrations[10];
    for (int index = 0; index < 10; ++index) {
        registrations[index] = CNA_INVALID_HANDLE;
    }
    CNA_Result result = cna.network_session_subscribe_game_ended(
        handle, java_game_ended_callback, NULL, &registrations[0]);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_game_started(
            handle, java_game_started_callback, NULL, &registrations[1]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_gamer_joined(
            handle, java_gamer_joined_callback, NULL, &registrations[2]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_gamer_left(
            handle, java_gamer_left_callback, NULL, &registrations[3]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_host_changed(
            handle, java_host_changed_callback, NULL, &registrations[4]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_session_ended(
            handle, java_session_ended_callback, NULL, &registrations[5]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_write_arbitrated_leaderboard(
            handle, java_write_arbitrated_callback, NULL, &registrations[6]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_write_true_skill(
            handle, java_write_true_skill_callback, NULL, &registrations[7]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_write_unarbitrated_leaderboard(
            handle, java_write_unarbitrated_callback, NULL, &registrations[8]);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna.network_session_subscribe_invite_accepted(
            java_invite_accepted_callback, NULL, &registrations[9]);
    }
    if (result != CNA_RESULT_SUCCESS) {
        for (int index = 9; index >= 0; --index) {
            if (registrations[index] != CNA_INVALID_HANDLE) {
                (void)cna.network_session_unsubscribe(registrations[index]);
            }
        }
        return (jint)result;
    }
    jlong values[10];
    for (int index = 0; index < 10; ++index) {
        values[index] = (jlong)registrations[index];
    }
    (*environment)->SetLongArrayRegion(environment, out_registrations, 0, 10, values);
    return (jint)CNA_RESULT_SUCCESS;
}

/** Releases one session's ten registrations, in reverse subscription order. */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeUnsubscribeSessionEvents(
    JNIEnv* environment, jclass type, jlongArray registrations)
{
    (void)type;
    jlong values[10];
    (*environment)->GetLongArrayRegion(environment, registrations, 0, 10, values);
    CNA_Result first = CNA_RESULT_SUCCESS;
    for (int index = 9; index >= 0; --index) {
        if (values[index] == 0) {
            continue;
        }
        CNA_Result result = cna.network_session_unsubscribe(
            (CNA_NetworkSessionEventRegistrationHandle)values[index]);
        if (result != CNA_RESULT_SUCCESS && first == CNA_RESULT_SUCCESS) {
            first = result;
        }
    }
    return (jint)first;
}

/*
 * Drains one queued event.
 *
 * Returns JNI_TRUE when a record was written. Java drains in a loop right after pumping, so
 * every event CNA raised during that pump is delivered on the game thread before the game sees
 * the next frame.
 */
JNIEXPORT jboolean JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativePollEvent(
    JNIEnv* environment, jclass type, jlongArray out_record)
{
    (void)type;
    JavaGamerEvent event;
    java_gamer_event_enter();
    if (java_gamer_event_count == 0) {
        java_gamer_event_leave();
        return JNI_FALSE;
    }
    event = java_gamer_events[java_gamer_event_head];
    java_gamer_event_head = (java_gamer_event_head + 1) % CNA_JAVA_EVENT_CAPACITY;
    java_gamer_event_count--;
    java_gamer_event_leave();

    jlong values[] = {event.sequence, event.kind, event.session, event.first, event.second,
        event.flag};
    (*environment)->SetLongArrayRegion(environment, out_record, 0, 6, values);
    return JNI_TRUE;
}

/** Returns how many events were dropped because the queue was full, for diagnostics. */
JNIEXPORT jlong JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeDroppedEventCount(
    JNIEnv* environment, jclass type)
{
    (void)environment;
    (void)type;
    java_gamer_event_enter();
    int64_t dropped = java_gamer_events_dropped;
    java_gamer_event_leave();
    return (jlong)dropped;
}

/** Discards every queued event. Used when a game lifetime ends. */
JNIEXPORT void JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeResetEvents(
    JNIEnv* environment, jclass type)
{
    (void)environment;
    (void)type;
    java_gamer_event_enter();
    java_gamer_event_head = 0;
    java_gamer_event_count = 0;
    java_gamer_events_dropped = 0;
    java_gamer_event_leave();
}

/*
 * Resolves the CNA graphics-device handle behind one game.
 *
 * The extension surface needs it because CNA's extended-layer factories take a device, while
 * the stock effect factories in this adapter take the game and resolve it themselves.
 */
JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeGraphicsDeviceHandle(
    JNIEnv* environment, jclass type, jlong game, jlongArray out_device)
{
    (void)type;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result = graphics_device_from_game(game, &device);
    if (result == CNA_RESULT_SUCCESS) {
        jlong value = (jlong)device;
        (*environment)->SetLongArrayRegion(environment, out_device, 0, 1, &value);
    }
    return (jint)result;
}

/*
 * Converts this adapter's game token into the CNA game handle.
 *
 * Most JNI entry points here take the adapter's own wrapper pointer and reach the CNA handle
 * inside it. A generated route takes the CNA handle directly, so the extension surface needs the
 * conversion rather than passing a pointer CNA would reject.
 */
JNIEXPORT jlong JNICALL
Java_org_openeggbert_cna_internal_NativeGamerServices_nativeCnaGameHandle(
    JNIEnv* environment, jclass type, jlong game)
{
    (void)environment;
    (void)type;
    return game == 0 ? 0 : (jlong)java_game(game)->cna_handle;
}
