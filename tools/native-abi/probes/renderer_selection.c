/* SPDX-License-Identifier: MS-PL */
/*
 * Which renderers does this build actually have, and what happens when a caller asks for one it
 * does not?
 *
 * This projection learned the second half of that question the hard way. A qualification sweep
 * named OPENGLES2 -- a renderer this library was configured without -- and the JVM did not report
 * an error: it printed `terminate called after throwing an instance of
 * 'System::InvalidOperationException'` and died with signal 6, taking the whole test run with it.
 * The message was a good one. The delivery was a process abort across a C ABI whose entire
 * contract is that failures come back as a CNA_Result.
 *
 * So there are two paths to a renderer that is not there, and they are not the same path:
 *
 *   the API path  -- cna_graphics_renderer_set_preferred_ext / _by_name_ext, which the
 *                    declaration says answers INVALID_STATE for an identity not in this build;
 *   the env path  -- CNA_GRAPHICS_RENDERER, which is what aborted.
 *
 * The env path turns out not to be a call at all. Run this probe with CNA_GRAPHICS_RENDERER
 * naming a renderer that is not compiled in and **nothing in it runs**: not main, not even a
 * `__attribute__((constructor))` of the program itself. The selection is resolved while
 * libcna_c_api.so is being loaded, and it throws there. So no caller can guard it -- a Java
 * process dies inside System.loadLibrary, before there is a stack frame to catch anything in --
 * and `env` mode exists to show that the process produces no output of its own before dying.
 * Every other case runs in `main` and returns results.
 *
 * The first half of the question is the useful half: _copy_available_ext enumerates what is
 * compiled in, which is the answer this session's whole multi-renderer qualification needed and
 * read out of CMakeCache.txt instead.
 *
 * `identity` mode exists because the first reading of this family was wrong, and wrong in a way
 * worth keeping the shape of. Run from `main`, the query routes looked uniformly broken -- count
 * zero, selection UNKNOWN, latch flag inverted -- and the conclusion drawn was "write-only". But
 * `main` calls set_preferred and reset_selection_for_tests before it asks, so what was being
 * measured was the probe's own leftovers. `identity` touches the selection not at all, and the
 * real defect is narrower and sharper: the routes are correct until a GraphicsDevice exists and
 * three of them are reset by creating one. A probe that mutates the state it is about to measure
 * will always find something; the finding will just not be the one it reports.
 *
 *   cc renderer_selection.c -lcna_c_api -o renderer_selection
 *   ./renderer_selection             # the API path, and the inventory
 *   ./renderer_selection identity    # who am I, on a selection nothing has touched
 *   ./renderer_selection env NAME    # ask for NAME through the environment and see what happens
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CNA/C/cna.h"
#include "CNA/C/core_ext.h"
#include "CNA/C/graphics.h"

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

/* The identities this probe names by hand, so the output reads as names rather than integers.
   Deliberately not the whole table: these are the ones a Linux build plausibly has. */
static const char* renderer_name(const CNA_GraphicsRendererType type)
{
    switch (type) {
        case CNA_GRAPHICS_RENDERER_UNKNOWN: return "UNKNOWN";
        case CNA_GRAPHICS_RENDERER_SDL_RENDERER: return "SDL_RENDERER";
        case CNA_GRAPHICS_RENDERER_OPENGLES2: return "OPENGLES2";
        case CNA_GRAPHICS_RENDERER_OPENGLES3: return "OPENGLES3";
        case CNA_GRAPHICS_RENDERER_OPENGL33: return "OPENGL33";
        case CNA_GRAPHICS_RENDERER_VULKAN: return "VULKAN";
        case CNA_GRAPHICS_RENDERER_HEADLESS: return "HEADLESS";
        case CNA_GRAPHICS_RENDERER_SOFTWARE: return "SOFTWARE";
        case CNA_GRAPHICS_RENDERER_STUB: return "STUB";
        default: return "(other)";
    }
}

static CNA_StringView view_of(const char* text)
{
    CNA_StringView view;
    view.data = text;
    view.byte_length = (uint64_t)strlen(text);
    return view;
}

static int ran = 0;

static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game;
    (void)game_time;
    (void)context;
    (void)out_error;
    ran = 1;
    return CNA_RESULT_SUCCESS;
}

/* Runs one frame with a graphics device, which is what latches the selection. */
static int latch_a_renderer(void)
{
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
    info.window_title = view_of("renderer selection");
    info.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) {
        printf("game create failed\n");
        return 0;
    }
    CNA_GraphicsDeviceManagerHandle manager = 0;
    (void)cna_graphics_device_manager_create(game, &manager);
    (void)cna_game_run_one_frame(game);
    if (manager != 0) (void)cna_graphics_device_manager_destroy(manager);
    (void)cna_game_destroy(game);
    return ran;
}

int main(int argc, char** argv)
{
    if (argc >= 2 && strcmp(argv[1], "env") == 0) {
        /* The env path, in its own process because it may not return.
         *
         * Setting the variable HERE is deliberately too late -- the selection was resolved while
         * the library loaded -- and that is the point: this branch reaches its first printf only
         * when the variable the process STARTED with named something this build has. Run it as
         * `CNA_GRAPHICS_RENDERER=OPENGLES2 ./renderer_selection env` to see nothing at all. */
        const char* wanted = argc >= 3 ? argv[2] : "OPENGLES2";
        setenv("CNA_GRAPHICS_RENDERER", wanted, 1);
        printf("env path: reached main, so the loaded-in renderer was one this build has\n");
        printf("env path: setting %s now, after load, which the selection no longer reads\n",
               wanted);
        fflush(stdout);
        const int frame = latch_a_renderer();
        printf("env path: returned normally, frame ran %d\n", frame);
        CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
        {
            const CNA_Result result = cna_graphics_renderer_get_active_ext(&active);
            printf("env path: active %s %s\n", name_of(result),
                   renderer_name(active));
        }
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "identity") == 0) {
        /* Who am I, asked the four ways CNA offers, on a process that touches the selection not
           at all -- the rest of main calls set_preferred, which would make the answer its own
           doing rather than the environment's. */
        /* Before the device, on a selection nothing has touched. */
        CNA_Bool latched_before = CNA_TRUE;
        CNA_GraphicsRendererType selected_before = CNA_GRAPHICS_RENDERER_UNKNOWN;
        uint64_t count_before = 0;
        uint64_t enumerated_before = 0;
        const CNA_Result got_latched_before =
            cna_graphics_renderer_get_is_latched_ext(&latched_before);
        const CNA_Result got_selected_before =
            cna_graphics_renderer_get_selected_ext(&selected_before);
        const CNA_Result got_count_before =
            cna_graphics_renderer_get_available_count_ext(&count_before);
        cna_graphics_renderer_copy_available_ext(NULL, 0, &enumerated_before);
        printf("identity, before the device:\n");
        printf("  get_is_latched_ext             %s %s\n", name_of(got_latched_before),
               latched_before ? "latched" : "not latched");
        printf("  get_selected_ext               %s %s\n", name_of(got_selected_before),
               renderer_name(selected_before));
        printf("  get_available_count_ext        %s %llu   copy_available says %llu\n",
               name_of(got_count_before), (unsigned long long)count_before,
               (unsigned long long)enumerated_before);

        const int frame = latch_a_renderer();
        CNA_Bool latched_after = CNA_FALSE;
        CNA_GraphicsRendererType selected_after = CNA_GRAPHICS_RENDERER_UNKNOWN;
        uint64_t count_after = 0;
        /* Each route is called before its output is printed. Calling it inside the printf that
           prints its output is how JAVA-UPSTREAM-018 was "measured": C leaves the order of
           argument evaluation unspecified, GCC read the output variable before the call wrote
           it, and the initial values were reported as a CNA defect. */
        const CNA_Result got_latched_after =
            cna_graphics_renderer_get_is_latched_ext(&latched_after);
        const CNA_Result got_selected_after =
            cna_graphics_renderer_get_selected_ext(&selected_after);
        const CNA_Result got_count_after =
            cna_graphics_renderer_get_available_count_ext(&count_after);
        printf("identity, after the device:\n");
        printf("  get_is_latched_ext             %s %s\n", name_of(got_latched_after),
               latched_after ? "latched" : "not latched");
        printf("  get_selected_ext               %s %s\n", name_of(got_selected_after),
               renderer_name(selected_after));
        printf("  get_available_count_ext        %s %llu\n", name_of(got_count_after),
               (unsigned long long)count_after);
        char current_name[64];
        memset(current_name, 0, sizeof current_name);
        uint64_t bytes = 0;
        CNA_GraphicsRendererType current = CNA_GRAPHICS_RENDERER_UNKNOWN;
        CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
        CNA_GraphicsBackendCategory category = 0;
        CNA_GraphicsBackendMaturity maturity = 0;
        const CNA_Result got_name =
            cna_graphics_renderer_copy_current_name(current_name, sizeof current_name, &bytes);
        const CNA_Result got_type = cna_graphics_renderer_get_current_type(&current);
        const CNA_Result got_active = cna_graphics_renderer_get_active_ext(&active);
        const CNA_Result got_category = cna_graphics_backend_get_current_category(&category);
        const CNA_Result got_maturity = cna_graphics_backend_get_current_maturity(&maturity);
        printf("identity: frame ran %d\n", frame);
        printf("  copy_current_name              %s \"%s\"\n", name_of(got_name), current_name);
        printf("  get_current_type               %s %s\n", name_of(got_type),
               renderer_name(current));
        printf("  get_active_ext                 %s %s\n", name_of(got_active),
               renderer_name(active));
        printf("  backend_get_current_category   %s %u\n", name_of(got_category),
               (unsigned)category);
        printf("  backend_get_current_maturity   %s %u\n", name_of(got_maturity),
               (unsigned)maturity);
        /* And what the per-identity routes say about the renderer the name reports, which is the
           only one of the five that is right. */
        CNA_GraphicsRendererType named = CNA_GRAPHICS_RENDERER_UNKNOWN;
        CNA_Bool recognized = CNA_FALSE;
        cna_graphics_renderer_try_parse_name_ext(view_of(current_name), &named, &recognized);
        CNA_GraphicsBackendCategory named_category = 0;
        CNA_GraphicsBackendMaturity named_maturity = 0;
        cna_graphics_backend_get_category(named, &named_category);
        cna_graphics_backend_get_maturity(named, &named_maturity);
        printf("  per-identity for \"%s\"   category %u  maturity %u\n", current_name,
               (unsigned)named_category, (unsigned)named_maturity);
        return 0;
    }

    printf("== before anything is created ==\n");
    {
        CNA_Bool latched = CNA_TRUE;
        {
            const CNA_Result result = cna_graphics_renderer_get_is_latched_ext(&latched);
            printf("  is_latched            %s %s\n",
                   name_of(result),
                   latched ? "yes" : "no");
        }
        CNA_GraphicsRendererType selected = CNA_GRAPHICS_RENDERER_UNKNOWN;
        {
            const CNA_Result result = cna_graphics_renderer_get_selected_ext(&selected);
            printf("  selected              %s %s\n",
                   name_of(result),
                   renderer_name(selected));
        }
        CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
        /* Documented as refused rather than guessed while nothing has been created. */
        {
            const CNA_Result result = cna_graphics_renderer_get_active_ext(&active);
            printf("  active                %s %s\n",
                   name_of(result), renderer_name(active));
        }
        CNA_Bool automatic = CNA_FALSE;
        {
            const CNA_Result result = cna_graphics_renderer_get_automatic_fallback_ext(&automatic);
            printf("  automatic fallback    %s %s\n",
                   name_of(result),
                   automatic ? "on" : "off");
        }
        /* The two already-bound routes answer the same question by name. If the name says
           HEADLESS and the type says UNKNOWN, one of the pair is wrong. */
        CNA_GraphicsRendererType current = CNA_GRAPHICS_RENDERER_UNKNOWN;
        const CNA_Result got_type = cna_graphics_renderer_get_current_type(&current);
        char current_name[64];
        memset(current_name, 0, sizeof current_name);
        uint64_t name_bytes = 0;
        const CNA_Result got_name =
            cna_graphics_renderer_copy_current_name(current_name, sizeof current_name, &name_bytes);
        printf("  build default type    %s %s\n", name_of(got_type), renderer_name(current));
        printf("  build default name    %s \"%s\"\n", name_of(got_name), current_name);
    }

    printf("\n== what this build has ==\n");
    uint64_t available_count = 0;
    {
        {
            const CNA_Result result = cna_graphics_renderer_get_available_count_ext(&available_count);
            printf("  available_count       %s %llu\n",
                   name_of(result),
                   (unsigned long long)available_count);
        }

        /* The zero-capacity probe, which every count/copy pair in this API supports. */
        uint64_t needed = 0;
        const CNA_Result probed = cna_graphics_renderer_copy_available_ext(NULL, 0, &needed);
        printf("  zero-capacity probe   %s needs %llu\n", name_of(probed),
               (unsigned long long)needed);

        /* And one element short of enough, which must write nothing at all. */
        if (needed > 1) {
            CNA_GraphicsRendererType* few =
                (CNA_GraphicsRendererType*)calloc((size_t)needed - 1, sizeof *few);
            const CNA_Result short_copy =
                cna_graphics_renderer_copy_available_ext(few, needed - 1, &needed);
            int wrote = 0;
            for (uint64_t i = 0; i + 1 < needed; i++) {
                if (few[i] != CNA_GRAPHICS_RENDERER_UNKNOWN) wrote = 1;
            }
            printf("  one short             %s partial write: %s\n", name_of(short_copy),
                   wrote ? "YES -- and the declaration says no" : "no");
            free(few);
        }

        CNA_GraphicsRendererType* all =
            (CNA_GraphicsRendererType*)calloc((size_t)(needed == 0 ? 1 : needed), sizeof *all);
        uint64_t written = 0;
        const CNA_Result copied = cna_graphics_renderer_copy_available_ext(all, needed, &written);
        printf("  copy_available        %s %llu:", name_of(copied), (unsigned long long)written);
        for (uint64_t i = 0; i < written; i++) printf(" %s", renderer_name(all[i]));
        printf("\n");
        free(all);
    }

    printf("\n== is_available, one identity at a time ==\n");
    {
        const CNA_GraphicsRendererType asked[] = {
            CNA_GRAPHICS_RENDERER_HEADLESS, CNA_GRAPHICS_RENDERER_OPENGL33,
            CNA_GRAPHICS_RENDERER_OPENGLES2, CNA_GRAPHICS_RENDERER_VULKAN,
            CNA_GRAPHICS_RENDERER_UNKNOWN,
        };
        for (size_t i = 0; i < sizeof asked / sizeof asked[0]; i++) {
            CNA_Bool there = CNA_FALSE;
            const CNA_Result got = cna_graphics_renderer_get_is_available_ext(asked[i], &there);
            printf("  %-14s        %s %s\n", renderer_name(asked[i]), name_of(got),
                   there ? "compiled in" : "not compiled in");
        }
        /* And an identity outside the defined range, which must be refused rather than answered. */
        CNA_Bool there = CNA_FALSE;
        printf("  identity 9999         %s\n",
               name_of(cna_graphics_renderer_get_is_available_ext(9999U, &there)));
    }

    printf("\n== how CNA classifies each identity it has ==\n");
    {
        const CNA_GraphicsRendererType asked[] = {
            CNA_GRAPHICS_RENDERER_HEADLESS,  CNA_GRAPHICS_RENDERER_SOFTWARE,
            CNA_GRAPHICS_RENDERER_OPENGL33,  CNA_GRAPHICS_RENDERER_OPENGLES3,
            CNA_GRAPHICS_RENDERER_OPENGLES2,
            CNA_GRAPHICS_RENDERER_VULKAN,    CNA_GRAPHICS_RENDERER_STUB,
        };
        for (size_t i = 0; i < sizeof asked / sizeof asked[0]; i++) {
            CNA_GraphicsBackendCategory category = 0;
            CNA_GraphicsBackendMaturity maturity = 0;
            const CNA_Result got_category =
                cna_graphics_backend_get_category(asked[i], &category);
            const CNA_Result got_maturity =
                cna_graphics_backend_get_maturity(asked[i], &maturity);
            char category_name[64];
            char maturity_name[64];
            memset(category_name, 0, sizeof category_name);
            memset(maturity_name, 0, sizeof maturity_name);
            uint64_t bytes = 0;
            cna_graphics_backend_category_copy_name(category, category_name,
                                                    sizeof category_name, &bytes);
            cna_graphics_backend_maturity_copy_name(maturity, maturity_name,
                                                    sizeof maturity_name, &bytes);
            printf("  %-14s        %s %-18s %s %s\n", renderer_name(asked[i]),
                   name_of(got_category), category_name, name_of(got_maturity), maturity_name);
        }
        CNA_GraphicsBackendCategory category = 0;
        printf("  UNKNOWN               %s\n",
               name_of(cna_graphics_backend_get_category(CNA_GRAPHICS_RENDERER_UNKNOWN,
                                                          &category)));
    }

    printf("\n== try_parse_name ==\n");
    {
        const char* names[] = { "OPENGL33", "opengl33", "OpEnGl33", "SOFTWARE", "OPENGLES2",
                                "NOT_A_RENDERER", "" };
        for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
            CNA_GraphicsRendererType type = CNA_GRAPHICS_RENDERER_UNKNOWN;
            CNA_Bool recognized = CNA_FALSE;
            const CNA_Result got =
                cna_graphics_renderer_try_parse_name_ext(view_of(names[i]), &type, &recognized);
            printf("  %-16s      %s %-12s %s\n", names[i][0] == 0 ? "(empty)" : names[i],
                   name_of(got), recognized ? "recognized" : "unrecognized", renderer_name(type));
        }
    }

    printf("\n== the API path to a renderer that is not here ==\n");
    {
        /* OPENGLES2 is a real, defined identity that this build was configured without, which is
           exactly the case the declaration promises INVALID_STATE for. If this aborts, the abort
           is the finding. */
        printf("  set_preferred(OPENGLES2)          %s\n",
               name_of(cna_graphics_renderer_set_preferred_ext(CNA_GRAPHICS_RENDERER_OPENGLES2)));
        printf("  set_preferred_by_name(OPENGLES2)  %s\n",
               name_of(cna_graphics_renderer_set_preferred_by_name_ext(view_of("OPENGLES2"))));
        printf("  set_preferred_by_name(nonsense)   %s\n",
               name_of(cna_graphics_renderer_set_preferred_by_name_ext(view_of("NOT_A_RENDERER"))));
        printf("  set_preferred(identity 9999)      %s\n",
               name_of(cna_graphics_renderer_set_preferred_ext(9999U)));

        /* And one that is here, which must be accepted. */
        printf("  set_preferred_by_name(HEADLESS)   %s\n",
               name_of(cna_graphics_renderer_set_preferred_by_name_ext(view_of("HEADLESS"))));
        CNA_GraphicsRendererType selected = CNA_GRAPHICS_RENDERER_UNKNOWN;
        {
            const CNA_Result result = cna_graphics_renderer_get_selected_ext(&selected);
            printf("  selected now                      %s %s\n",
                   name_of(result),
                   renderer_name(selected));
        }
    }

    printf("\n== the fallback chain ==\n");
    {
        const CNA_GraphicsRendererType chain[] = { CNA_GRAPHICS_RENDERER_OPENGLES2,
                                                   CNA_GRAPHICS_RENDERER_HEADLESS };
        printf("  set_fallback_chain    %s\n",
               name_of(cna_graphics_renderer_set_fallback_chain_ext(chain, 2)));
        printf("  empty chain           %s\n",
               name_of(cna_graphics_renderer_set_fallback_chain_ext(NULL, 0)));
        const CNA_GraphicsRendererType undefined_chain[] = { 9999U };
        printf("  chain with 9999       %s\n",
               name_of(cna_graphics_renderer_set_fallback_chain_ext(undefined_chain, 1)));
        printf("  chain, null with 1    %s\n",
               name_of(cna_graphics_renderer_set_fallback_chain_ext(NULL, 1)));
        printf("  automatic on          %s\n",
               name_of(cna_graphics_renderer_set_automatic_fallback_ext(CNA_TRUE)));
        uint64_t history = 0;
        {
            const CNA_Result result = cna_graphics_renderer_get_fallback_count_ext(&history);
            printf("  fallback_count        %s %llu\n",
                   name_of(result),
                   (unsigned long long)history);
        }
    }

    printf("\n== the fallback reasons' names ==\n");
    for (CNA_GraphicsRendererFallbackReason reason = 0;
         reason <= CNA_GRAPHICS_RENDERER_FALLBACK_MAXIMUM + 1U; reason++) {
        uint64_t bytes = 0;
        const CNA_Result sized =
            cna_graphics_renderer_fallback_reason_get_name_size_ext(reason, &bytes);
        char text[64];
        memset(text, 0, sizeof text);
        uint64_t written = 0;
        const CNA_Result copied = cna_graphics_renderer_fallback_reason_copy_name_ext(
            reason, text, sizeof text, &written);
        printf("  reason %u              %s %s %s\n", (unsigned)reason, name_of(sized),
               name_of(copied), text);
    }

    printf("\n== after a device exists ==\n");
    {
        const int frame = latch_a_renderer();
        printf("  frame ran             %s\n", frame ? "yes" : "no");
        CNA_Bool latched = CNA_FALSE;
        {
            const CNA_Result result = cna_graphics_renderer_get_is_latched_ext(&latched);
            printf("  is_latched            %s %s\n",
                   name_of(result),
                   latched ? "yes" : "no");
        }
        CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
        {
            const CNA_Result result = cna_graphics_renderer_get_active_ext(&active);
            printf("  active                %s %s\n",
                   name_of(result), renderer_name(active));
        }
        CNA_GraphicsRendererType current = CNA_GRAPHICS_RENDERER_UNKNOWN;
        char current_name[64];
        memset(current_name, 0, sizeof current_name);
        uint64_t name_bytes = 0;
        {
            const CNA_Result result = cna_graphics_renderer_get_current_type(&current);
            printf("  current type          %s %s\n",
                   name_of(result), renderer_name(current));
        }
        printf("  current name          %s \"%s\"\n",
               name_of(cna_graphics_renderer_copy_current_name(current_name, sizeof current_name,
                                                               &name_bytes)),
               current_name);
        uint64_t count_again = 0;
        {
            const CNA_Result result = cna_graphics_renderer_get_available_count_ext(&count_again);
            printf("  available_count       %s %llu\n",
                   name_of(result),
                   (unsigned long long)count_again);
        }
        /* The process-wide classifiers, which say "current" and are the ones a game would reach
           for first. Compare them against the per-identity answer for the renderer that is
           actually running. */
        {
            CNA_GraphicsBackendCategory current_category = 0;
            CNA_GraphicsBackendMaturity current_maturity = 0;
            const CNA_Result gc = cna_graphics_backend_get_current_category(&current_category);
            const CNA_Result gm = cna_graphics_backend_get_current_maturity(&current_maturity);
            printf("  current category      %s %u   current maturity %s %u\n", name_of(gc),
                   (unsigned)current_category, name_of(gm), (unsigned)current_maturity);
        }
        /* And the per-identity classifiers again, because get_current_type answers correctly
           before the selection latches and UNKNOWN after -- so the question is whether these
           two are the same kind of route or a different one. */
        {
            const CNA_GraphicsRendererType asked[] = { CNA_GRAPHICS_RENDERER_HEADLESS,
                                                       CNA_GRAPHICS_RENDERER_SOFTWARE,
                                                       CNA_GRAPHICS_RENDERER_OPENGL33 };
            for (size_t i = 0; i < sizeof asked / sizeof asked[0]; i++) {
                CNA_GraphicsBackendCategory category = 0;
                CNA_GraphicsBackendMaturity maturity = 0;
                const CNA_Result gc = cna_graphics_backend_get_category(asked[i], &category);
                const CNA_Result gm = cna_graphics_backend_get_maturity(asked[i], &maturity);
                printf("  %-14s after   %s category=%u  %s maturity=%u\n",
                       renderer_name(asked[i]), name_of(gc), (unsigned)category, name_of(gm),
                       (unsigned)maturity);
            }
        }
        printf("  set_preferred now     %s\n",
               name_of(cna_graphics_renderer_set_preferred_ext(CNA_GRAPHICS_RENDERER_SOFTWARE)));
        printf("  set_chain now         %s\n",
               name_of(cna_graphics_renderer_set_fallback_chain_ext(NULL, 0)));
        uint64_t history = 0;
        cna_graphics_renderer_get_fallback_count_ext(&history);
        printf("  fallback_count        %llu\n", (unsigned long long)history);
        for (uint64_t i = 0; i < history; i++) {
            CNA_GraphicsRendererFallbackRecord record;
            memset(&record, 0, sizeof record);
            record.struct_size = (uint32_t)(sizeof record);
            record.struct_version = 1U;
            const CNA_Result got = cna_graphics_renderer_get_fallback_at_ext(i, &record);
            uint64_t bytes = 0;
            char message[512];
            memset(message, 0, sizeof message);
            cna_graphics_renderer_fallback_copy_message_ext(i, message, sizeof message, &bytes);
            printf("    [%llu] %s %s reason %u: %s\n", (unsigned long long)i, name_of(got),
                   renderer_name(record.type), (unsigned)record.reason, message);
        }
        printf("  reset_for_tests       %s\n",
               name_of(cna_graphics_renderer_reset_selection_for_tests_ext()));
        {
            const CNA_Result result = cna_graphics_renderer_get_is_latched_ext(&latched);
            printf("  is_latched after      %s %s\n",
                   name_of(result),
                   latched ? "yes" : "no");
        }
    }

    printf("\nPROBE OK\n");
    return 0;
}
