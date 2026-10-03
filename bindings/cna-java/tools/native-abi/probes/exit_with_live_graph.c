/* SPDX-License-Identifier: MS-PL
 *
 * What happens when a process exits with a CNA game still alive?
 *
 * A game that is never destroyed is not a hypothetical: a JVM exiting on an unhandled exception,
 * a `System.exit`, or simply a program that lets the operating system reclaim everything all
 * leave the native graph standing. CNA-Java has had a subprocess test for exactly this since the
 * ownership graph existed, and on the HEADLESS renderer the process exits zero.
 *
 * On a renderer with a real GL context it aborts -- `terminate called without an active
 * exception`, SIGABRT -- and this reproduces it with no Java anywhere in the picture, which is
 * the difference between a CNA finding and a JNI one.
 *
 * One case per process, named on the command line, because the interesting outcomes are crashes:
 *
 *   ./build-probe/exit_with_live_graph game        a game, never destroyed
 *   ./build-probe/exit_with_live_graph device      a game with a graphics device manager
 *   ./build-probe/exit_with_live_graph frame       and one frame run
 *   ./build-probe/exit_with_live_graph destroyed   the same, then destroyed properly
 *   ./build-probe/exit_with_live_graph buffer      a live static vertex buffer as well
 *   ./build-probe/exit_with_live_graph dynamic     a live DYNAMIC vertex buffer as well
 *   ./build-probe/exit_with_live_graph thread      the same, on a thread that exits first
 *   ./build-probe/exit_with_live_graph thread-bare and the same thread with no buffer at all
 *
 * The last case is the one that matters, and it is what a JVM does without saying so: the `java`
 * launcher runs `main` on a thread it creates, not on the process's initial thread, so the thread
 * that made the game and its GL context has already ended by the time the process exits. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "CNA/C/cna.h"

static int ran = 0;
static int want_buffer = 0;
static int want_dynamic = 0;

/* A live dynamic vertex buffer is what the Java subprocess has that a plain live game does not,
   and narrowing showed it is enough on its own to abort there. The device is only lent inside a
   lifecycle callback, so the buffer has to be made here. */
static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    ran = 1;
    if (!want_buffer) {
        return CNA_RESULT_SUCCESS;
    }
    CNA_Handle device = CNA_INVALID_HANDLE;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS) {
        printf("no device\n");
        return CNA_RESULT_SUCCESS;
    }
    /* A real declaration, because the empty one is only legal for a static buffer and it is the
       DYNAMIC one the Java case has. */
    CNA_VertexElement element;
    memset(&element, 0, sizeof element);
    element.offset = 0;
    element.format = CNA_VERTEX_ELEMENT_FORMAT_VECTOR3;
    element.usage = CNA_VERTEX_ELEMENT_USAGE_POSITION;
    element.usage_index = 0;
    CNA_VertexDeclarationHandle declaration = 0;
    printf("declaration create      %d\n",
           (int)cna_vertex_declaration_create(&element, 1U, &declaration));
    CNA_VertexBufferCreateInfo create_info;
    memset(&create_info, 0, sizeof create_info);
    create_info.struct_size = (uint32_t)(sizeof create_info);
    create_info.struct_version = 1U;
    create_info.vertex_declaration = declaration;
    create_info.vertex_count = 3;
    create_info.buffer_usage = CNA_BUFFER_USAGE_WRITE_ONLY;
    create_info.dynamic = want_dynamic ? CNA_TRUE : CNA_FALSE;
    CNA_VertexBufferHandle buffer = 0;
    printf("vertex buffer create    %d  dynamic=%d\n",
           (int)cna_vertex_buffer_create(device, &create_info, &buffer),
           want_dynamic);
    /* Deliberately not destroyed. */
    return CNA_RESULT_SUCCESS;
}

static void* body(void* argument);

int main(int argc, char** argv)
{
    const char* which = argc > 1 ? argv[1] : "frame";
    if (strcmp(which, "thread") == 0 || strcmp(which, "thread-bare") == 0) {
        pthread_t worker;
        if (pthread_create(&worker, NULL, body,
                           strcmp(which, "thread-bare") == 0 ? (void*)"frame" : NULL) != 0) {
            printf("cannot start a thread\n");
            return 1;
        }
        (void)pthread_join(worker, NULL);
        printf("THE OWNING THREAD HAS ENDED; EXITING\n");
        fflush(stdout);
        return 0;
    }
    return (int)(intptr_t)body((void*)which);
}

static void* body(void* argument)
{
    const char* which = argument == NULL ? "dynamic" : (const char*)argument;
    want_buffer = strcmp(which, "buffer") == 0 || strcmp(which, "dynamic") == 0
                  || argument == NULL;
    want_dynamic = strcmp(which, "dynamic") == 0 || argument == NULL;
    const char* renderer = getenv("CNA_GRAPHICS_RENDERER");
    printf("case %s on %s\n", which, renderer != NULL ? renderer : "<build default>");
    fflush(stdout);

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
    info.window_title.data = "exit with a live graph";
    info.window_title.byte_length = 22U;
    info.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) {
        printf("game create failed\n");
        return (void*)(intptr_t)1;
    }
    printf("game created\n");
    fflush(stdout);
    if (strcmp(which, "game") == 0) {
        printf("EXITING WITH A LIVE GAME\n");
        fflush(stdout);
        return NULL;
    }

    CNA_GraphicsDeviceManagerHandle manager = 0;
    (void)cna_graphics_device_manager_create(game, &manager);
    printf("device manager created\n");
    fflush(stdout);
    if (strcmp(which, "device") == 0) {
        printf("EXITING WITH A LIVE DEVICE MANAGER\n");
        fflush(stdout);
        return NULL;
    }

    (void)cna_game_run_one_frame(game);
    printf("frame run (update entered: %d)\n", ran);
    fflush(stdout);
    if (strcmp(which, "destroyed") == 0) {
        (void)cna_graphics_device_manager_destroy(manager);
        printf("device manager destroyed %d\n", (int)cna_game_destroy(game));
        printf("EXITING WITH NOTHING ALIVE\n");
        fflush(stdout);
        return NULL;
    }
    printf("EXITING WITH A LIVE GRAPH\n");
    fflush(stdout);
    return NULL;
}
