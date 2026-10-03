/* SPDX-License-Identifier: MS-PL */
/*
 * Three CNA device families this projection left out behind "message box, file dialog, system
 * tray ... are CNA device extensions beyond XNA 4.0" -- which says why they are not XNA and not
 * one word about why they are unbound. The camera family was taken apart the same way one session
 * earlier and the assumption behind it turned out to be wrong in both directions at once. So
 * measure first.
 *
 * Each of the three is modal, host-owned or asynchronous, and each ships a test backend. The
 * questions that decide whether Java can have them are about the backends, not about the dialogs:
 *
 *   1. is each family supported on this platform at all, and what does the unsupported path do?
 *   2. does the message-box test backend record a request rather than blocking, and does the log
 *      distinguish a dismiss-only box from a button-answering one?
 *   3. is the file dialog's result callback SYNCHRONOUS under the test backend? The header says
 *      the real one "may be long afterwards, or never", and that difference is the difference
 *      between a call-duration context and a global reference that leaks by construction.
 *   4. does a tray entry's click callback survive to a later `click_entry_for_tests_ext`, and on
 *      which thread does it arrive?
 *   5. what does each family do to the game's owned-child count -- can the game still be
 *      destroyed afterwards? That is what disqualified the engine layer's lent-handle getters.
 *
 * Every out-parameter is poisoned before the call, because "the route did not write it" is a
 * third answer and it has twice been the finding in this projection.
 *
 * It nearly became a fourth, wrongly. The first draft of this probe wrote
 *
 *     printf("%s %s\n", name_of(cna_..._get_is_supported_ext(game, &flag)), flag_of(flag));
 *
 * and reported that five routes return SUCCESS and leave their output alone. C does not sequence
 * a function call's arguments, and this compiler evaluates them right to left, so `flag_of(flag)`
 * ran BEFORE the route did and read the poison every time. The finding was the probe's. Every
 * call below is therefore a statement of its own, and the output is read on the next line --
 * which is the only way a poison check means anything.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "CNA/C/cna.h"
#include "CNA/C/devices.h"

static int ran = 0;

static const char* name_of(const CNA_Result result)
{
    switch ((int)result) {
        case 0: return "SUCCESS";
        case 1: return "INVALID_ARGUMENT";
        case 2: return "INVALID_HANDLE";
        case 3: return "INVALID_STATE";
        case 6: return "NOT_SUPPORTED";
        case 12: return "INTERNAL";
        case 14: return "BUFFER_TOO_SMALL";
        default: return "OTHER";
    }
}

/* Neither CNA_TRUE nor CNA_FALSE, so "not written" is visible rather than read back as whatever
   the probe put there. */
#define PROBE_POISON ((CNA_Bool)0xAB)
#define POISON_I32 ((int32_t)-424242)
#define POISON_U64 ((uint64_t)0xDEADBEEFDEADBEEFULL)

static const char* flag_of(const CNA_Bool flag)
{
    if (flag == PROBE_POISON) return "UNTOUCHED";
    return flag ? "yes" : "no";
}

static CNA_StringView view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

/* ---- file dialog callback ---- */

static int dialog_calls = 0;
static int dialog_ran_inside_show = 0;
static int dialog_inside_show = 0;
static uint64_t dialog_last_count = 0;
static char dialog_first[256];
static pthread_t dialog_thread;
static pthread_t main_thread;

static void on_dialog_result(const CNA_StringView* files, uint64_t count, void* context)
{
    dialog_calls++;
    dialog_last_count = count;
    dialog_thread = pthread_self();
    if (dialog_inside_show) dialog_ran_inside_show = 1;
    memset(dialog_first, 0, sizeof dialog_first);
    if (count > 0 && files != NULL && files[0].data != NULL) {
        const uint64_t copy = files[0].byte_length < sizeof dialog_first - 1
                ? files[0].byte_length : sizeof dialog_first - 1;
        memcpy(dialog_first, files[0].data, (size_t)copy);
    }
    printf("      [callback] context=%s count=%llu first=\"%s\" inside_show=%s\n",
           context != NULL ? (const char*)context : "(null)",
           (unsigned long long)count, dialog_first, dialog_inside_show ? "yes" : "no");
    fflush(stdout);
}

/* ---- tray callback ---- */

static int tray_clicks = 0;
static pthread_t tray_thread;
static void* tray_last_context = NULL;

static void on_tray_click(void* context)
{
    tray_clicks++;
    tray_thread = pthread_self();
    tray_last_context = context;
    printf("      [tray click] context=%s\n",
           context != NULL ? (const char*)context : "(null)");
    fflush(stdout);
}

static void message_box(CNA_Handle game)
{
    printf("== message box ==\n");
    CNA_Bool supported = PROBE_POISON;
    const CNA_Result asked = cna_message_box_get_is_supported_ext(game, &supported);
    printf("  is_supported            %s %s\n", name_of(asked), flag_of(supported));

    /* The log before any backend is installed: the header says INVALID_STATE. */
    CNA_MessageBoxTestLog before;
    memset(&before, 0xEE, sizeof before);
    before.struct_size = (uint32_t)(sizeof before);
    before.struct_version = 1U;
    printf("  log with no backend     %s\n",
           name_of(cna_message_box_get_test_log_ext(game, &before)));

    printf("  install backend (btn 1) %s\n",
           name_of(cna_message_box_set_test_backend_ext(game, CNA_TRUE, 1)));

    CNA_MessageBoxTestLog fresh;
    memset(&fresh, 0xEE, sizeof fresh);
    fresh.struct_size = (uint32_t)(sizeof fresh);
    fresh.struct_version = 1U;
    const CNA_Result fresh_log = cna_message_box_get_test_log_ext(game, &fresh);
    printf("  log after install       %s simple=%u choice=%u type=%u buttons=%u\n",
           name_of(fresh_log), fresh.simple_calls, fresh.choice_calls,
           (unsigned)fresh.last_type, fresh.last_button_count);

    printf("  show_simple WARNING     %s\n",
           name_of(cna_message_box_show_simple_ext(game, CNA_MESSAGE_BOX_TYPE_WARNING,
                                                   view("title"), view("body"))));

    CNA_StringView labels[3];
    labels[0] = view("Yes");
    labels[1] = view("No");
    labels[2] = view("Maybe");
    int32_t chosen = POISON_I32;
    const CNA_Result three = cna_message_box_show_ext(game, CNA_MESSAGE_BOX_TYPE_ERROR, view("t"),
                                                     view("m"), labels, 3U, &chosen);
    printf("  show three buttons      %s chosen=%d\n", name_of(three), (int)chosen);

    CNA_MessageBoxTestLog after;
    memset(&after, 0xEE, sizeof after);
    after.struct_size = (uint32_t)(sizeof after);
    after.struct_version = 1U;
    const CNA_Result after_log = cna_message_box_get_test_log_ext(game, &after);
    printf("  log after two shows     %s simple=%u choice=%u type=%u buttons=%u\n",
           name_of(after_log), after.simple_calls, after.choice_calls,
           (unsigned)after.last_type, after.last_button_count);

    /* Refusals the header names. */
    int32_t refused = POISON_I32;
    const CNA_Result bad_type = cna_message_box_show_simple_ext(
            game, (CNA_MessageBoxType)99U, view("t"), view("m"));
    printf("  undefined severity      %s\n", name_of(bad_type));
    const CNA_Result no_buttons = cna_message_box_show_ext(
            game, CNA_MESSAGE_BOX_TYPE_INFORMATION, view("t"), view("m"), labels, 0U, &refused);
    printf("  zero buttons            %s chosen=%d %s\n", name_of(no_buttons), (int)refused,
           refused == POISON_I32 ? "(untouched)" : "");

    /* Reinstalling resets the log, the header says. */
    printf("  reinstall               %s\n",
           name_of(cna_message_box_set_test_backend_ext(game, CNA_TRUE, 0)));
    CNA_MessageBoxTestLog reset;
    memset(&reset, 0xEE, sizeof reset);
    reset.struct_size = (uint32_t)(sizeof reset);
    reset.struct_version = 1U;
    const CNA_Result reset_log = cna_message_box_get_test_log_ext(game, &reset);
    printf("  log after reinstall     %s simple=%u choice=%u\n",
           name_of(reset_log), reset.simple_calls, reset.choice_calls);

    printf("  remove backend          %s\n",
           name_of(cna_message_box_set_test_backend_ext(game, CNA_FALSE, 0)));
    CNA_MessageBoxTestLog gone;
    memset(&gone, 0xEE, sizeof gone);
    gone.struct_size = (uint32_t)(sizeof gone);
    gone.struct_version = 1U;
    printf("  log after removal       %s\n",
           name_of(cna_message_box_get_test_log_ext(game, &gone)));
    printf("\n");
}

static void file_dialog(CNA_Handle game)
{
    printf("== file dialog ==\n");
    CNA_Bool supported = PROBE_POISON;
    const CNA_Result asked = cna_file_dialog_get_is_supported_ext(game, &supported);
    printf("  is_supported            %s %s\n", name_of(asked), flag_of(supported));

    CNA_StringView results[2];
    results[0] = view("/tmp/chosen-one.txt");
    results[1] = view("/tmp/chosen-two.txt");
    printf("  install backend (2)     %s\n",
           name_of(cna_file_dialog_set_test_backend_ext(game, CNA_TRUE, results, 2U)));

    CNA_FileDialogFilter filters[1];
    memset(filters, 0, sizeof filters);
    filters[0].struct_size = (uint32_t)(sizeof filters[0]);
    filters[0].struct_version = 1U;
    filters[0].name = view("Text");
    filters[0].pattern = view("*.txt");

    dialog_calls = 0;
    dialog_ran_inside_show = 0;
    dialog_inside_show = 1;
    const CNA_Result opened = cna_file_dialog_show_open_file_ext(
            game, on_dialog_result, (void*)"open-context", filters, 1U, view("/tmp"), CNA_TRUE);
    dialog_inside_show = 0;
    printf("  show_open_file          %s calls=%d synchronous=%s\n",
           name_of(opened), dialog_calls, dialog_ran_inside_show ? "yes" : "no");
    printf("  callback thread == main %s\n",
           dialog_calls > 0 && pthread_equal(dialog_thread, main_thread) ? "yes" : "no");

    dialog_calls = 0;
    dialog_inside_show = 1;
    const CNA_Result saved = cna_file_dialog_show_save_file_ext(
            game, on_dialog_result, (void*)"save-context", NULL, 0U, view(""));
    dialog_inside_show = 0;
    printf("  show_save_file          %s calls=%d count=%llu\n",
           name_of(saved), dialog_calls, (unsigned long long)dialog_last_count);

    dialog_calls = 0;
    dialog_inside_show = 1;
    const CNA_Result folder = cna_file_dialog_show_open_folder_ext(
            game, on_dialog_result, (void*)"folder-context", view(""), CNA_FALSE);
    dialog_inside_show = 0;
    printf("  show_open_folder        %s calls=%d\n", name_of(folder), dialog_calls);

    /* Cancellation is an empty result, the header says. */
    printf("  install backend (none)  %s\n",
           name_of(cna_file_dialog_set_test_backend_ext(game, CNA_TRUE, NULL, 0U)));
    dialog_calls = 0;
    dialog_last_count = POISON_U64;
    dialog_inside_show = 1;
    const CNA_Result cancelled = cna_file_dialog_show_open_file_ext(
            game, on_dialog_result, NULL, NULL, 0U, view(""), CNA_FALSE);
    dialog_inside_show = 0;
    printf("  cancelled open          %s calls=%d count=%llu\n",
           name_of(cancelled), dialog_calls, (unsigned long long)dialog_last_count);

    printf("  null handler            %s\n",
           name_of(cna_file_dialog_show_open_file_ext(game, NULL, NULL, NULL, 0U,
                                                      view(""), CNA_FALSE)));

    printf("  remove backend          %s\n",
           name_of(cna_file_dialog_set_test_backend_ext(game, CNA_FALSE, NULL, 0U)));
    printf("\n");
}

static void system_tray(CNA_Handle game)
{
    printf("== system tray ==\n");
    CNA_Bool supported = PROBE_POISON;
    const CNA_Result asked = cna_system_tray_get_is_supported_ext(game, &supported);
    printf("  is_supported            %s %s\n", name_of(asked), flag_of(supported));

    CNA_SystemTrayHandle tray = (CNA_SystemTrayHandle)POISON_U64;
    const CNA_Result made = cna_system_tray_create_with_test_backend_ext(game, view("probe"),
                                                                        &tray);
    printf("  create (test backend)   %s handle=%s\n", name_of(made),
           tray == (CNA_SystemTrayHandle)POISON_U64 ? "UNTOUCHED" : "written");
    if (made != CNA_RESULT_SUCCESS) {
        printf("\n");
        return;
    }

    printf("  set_tooltip             %s\n",
           name_of(cna_system_tray_set_tooltip(tray, view("probe tray"))));

    uint64_t first = POISON_U64;
    uint64_t second = POISON_U64;
    const CNA_Result added_one = cna_system_tray_add_entry(
            tray, view("Sound"), CNA_TRUE, CNA_TRUE, CNA_TRUE, on_tray_click, (void*)"sound",
            &first);
    printf("  add checkable entry     %s index=%llu\n", name_of(added_one),
           (unsigned long long)first);
    const CNA_Result added_two = cna_system_tray_add_entry(
            tray, view("Quit"), CNA_FALSE, CNA_FALSE, CNA_FALSE, NULL, NULL, &second);
    printf("  add plain entry         %s index=%llu\n", name_of(added_two),
           (unsigned long long)second);

    CNA_Bool checked = PROBE_POISON;
    CNA_Bool enabled = PROBE_POISON;
    CNA_Result step = cna_system_tray_get_entry_checked(tray, first, &checked);
    printf("  entry 0 checked         %s %s\n", name_of(step), flag_of(checked));
    step = cna_system_tray_get_entry_enabled(tray, second, &enabled);
    printf("  entry 1 enabled         %s %s\n", name_of(step), flag_of(enabled));

    step = cna_system_tray_set_entry_checked(tray, first, CNA_FALSE);
    printf("  uncheck entry 0         %s\n", name_of(step));
    checked = PROBE_POISON;
    step = cna_system_tray_get_entry_checked(tray, first, &checked);
    printf("  entry 0 checked again   %s %s\n", name_of(step), flag_of(checked));

    step = cna_system_tray_set_entry_enabled(tray, second, CNA_TRUE);
    printf("  enable entry 1          %s\n", name_of(step));
    enabled = PROBE_POISON;
    step = cna_system_tray_get_entry_enabled(tray, second, &enabled);
    printf("  entry 1 enabled again   %s %s\n", name_of(step), flag_of(enabled));

    step = cna_system_tray_set_entry_label(tray, second, view("Exit"));
    printf("  relabel entry 1         %s\n", name_of(step));
    step = cna_system_tray_set_entry_label(tray, 99U, view("nowhere"));
    printf("  relabel past the end    %s\n", name_of(step));

    CNA_Bool past = PROBE_POISON;
    step = cna_system_tray_get_entry_checked(tray, 99U, &past);
    printf("  checked past the end    %s %s\n", name_of(step), flag_of(past));

    /* Which of the three flags actually decides what get_entry_checked answers. A projection
       that swapped `checkable` and `initially_checked` would round-trip perfectly on entries
       that pass the same value for both, so the combinations that differ are the ones that
       say anything. */
    {
        const CNA_Bool cases[4][2] = {
            {CNA_FALSE, CNA_FALSE}, {CNA_FALSE, CNA_TRUE},
            {CNA_TRUE, CNA_FALSE}, {CNA_TRUE, CNA_TRUE},
        };
        for (int which = 0; which < 4; ++which) {
            uint64_t added = POISON_U64;
            step = cna_system_tray_add_entry(tray, view("flags"), cases[which][0],
                                             cases[which][1], CNA_TRUE, NULL, NULL, &added);
            CNA_Bool state = PROBE_POISON;
            if (step == CNA_RESULT_SUCCESS) {
                step = cna_system_tray_get_entry_checked(tray, added, &state);
            }
            printf("  checkable=%d checked=%d -> %s\n", (int)cases[which][0],
                   (int)cases[which][1], flag_of(state));
        }
    }

    /* The question the whole family turns on: does a callback registered at add time still run
       later, and on which thread? */
    tray_clicks = 0;
    tray_last_context = NULL;
    step = cna_system_tray_click_entry_for_tests_ext(tray, first);
    printf("  click entry 0           %s clicks=%d\n", name_of(step), tray_clicks);
    printf("  click thread == main    %s\n",
           tray_clicks > 0 && pthread_equal(tray_thread, main_thread) ? "yes" : "no");
    printf("  click context preserved %s\n",
           tray_last_context != NULL && strcmp((const char*)tray_last_context, "sound") == 0
                   ? "yes" : "no");
    const int before_silent = tray_clicks;
    step = cna_system_tray_click_entry_for_tests_ext(tray, second);
    printf("  click entry with no cb  %s (fired %d)\n", name_of(step),
           tray_clicks - before_silent);
    step = cna_system_tray_click_entry_for_tests_ext(tray, 99U);
    printf("  click past the end      %s\n", name_of(step));

    /* A real-backend tray, to say whether the test one is what is special -- and whether a
       failed create leaves the output alone. */
    CNA_SystemTrayHandle real = (CNA_SystemTrayHandle)POISON_U64;
    const CNA_Result really = cna_system_tray_create(game, view("probe real"), &real);
    printf("  create (real backend)   %s handle=%s\n", name_of(really),
           real == (CNA_SystemTrayHandle)POISON_U64 ? "UNTOUCHED" : "written");
    if (really == CNA_RESULT_SUCCESS) {
        printf("  destroy real            %s\n", name_of(cna_system_tray_destroy(real)));
    }

    printf("  destroy                 %s\n", name_of(cna_system_tray_destroy(tray)));
    printf("  destroy twice           %s\n", name_of(cna_system_tray_destroy(tray)));
    printf("\n");
}

static void environment(void)
{
    printf("== environment ==\n");
    CNA_DeviceType kind = (CNA_DeviceType)0xEEEEEEEEU;
    const CNA_Result got = cna_environment_get_device_type(&kind);
    printf("  device_type             %s value=%u %s\n", name_of(got), (unsigned)kind,
           kind == (CNA_DeviceType)0xEEEEEEEEU ? "(untouched)" : "");
    printf("\n");
}

static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    if (ran) return CNA_RESULT_SUCCESS;
    ran = 1;

    environment();
    message_box(game);
    file_dialog(game);
    system_tray(game);
    return CNA_RESULT_SUCCESS;
}

int main(void)
{
    main_thread = pthread_self();
    const char* requested = getenv("CNA_GRAPHICS_RENDERER");
    printf("renderer requested %s\n\n", requested != NULL ? requested : "<build default>");

    CNA_GameCallbacks callbacks;
    memset(&callbacks, 0, sizeof callbacks);
    callbacks.struct_size = (uint32_t)(sizeof callbacks);
    callbacks.struct_version = 1U;
    callbacks.update = on_update;

    CNA_GameCreateInfo info;
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)(sizeof info);
    info.struct_version = 1U;
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = 166667;
    info.window_title = view("host dialogs and tray");
    info.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) {
        printf("game create failed\n");
        return 1;
    }
    (void)cna_game_run_one_frame(game);
    /* The question the engine layer's lent handles turned on: is the game still destroyable? */
    printf("game destroy       %s\n", name_of(cna_game_destroy(game)));
    printf("PROBE %s\n", ran ? "OK" : "INCOMPLETE");
    return ran ? 0 : 1;
}
