/* SPDX-License-Identifier: MS-PL */
/*
 * Can a camera be qualified on a machine with no camera?
 *
 * This projection had fifteen camera routes unbound behind the reason "camera ... are CNA device
 * extensions beyond XNA 4.0", which explains why they are an extension and not why they are
 * absent. The assumption behind leaving them was that a webcam API needs a webcam. It does not:
 * `cna_camera_create_with_test_backend_ext` builds one whose frames the caller supplies through
 * `cna_camera_set_test_frame_ext` and whose state the caller drives through
 * `cna_camera_set_test_state_ext`, so every route in the family can be exercised, and the pixels
 * that come back out can be compared against the pixels that went in.
 *
 * What this probe establishes before any Java is written:
 *
 *   1. does the test backend construct where the real one does not?
 *   2. does a frame set on it come back through try_acquire_frame into a texture, pixel for pixel?
 *   3. is the size mismatch the header describes really a refusal, and which one?
 *   4. is "no frame ready" an ordinary CNA_FALSE rather than an error, as documented?
 *   5. what does each state do to acquisition -- and can the test backend reach all of them?
 *
 * Needs a graphics device, because the frame lands in a Texture2D.
 *
 * The answer is that the family is qualifiable and is not safe to project yet. Two of the five
 * questions came back with defects, and one of those is a segfault:
 *
 *   ./camera_test_backend                      the full sequence
 *   PROBE_CASE=0 ./camera_test_backend         SIGSEGV, on HEADLESS and OPENGL33 alike, 3 of 3
 *   PROBE_CASE=0 PROBE_SKIP=camera ...         clean, exit 0 -- one cna_camera_destroy fewer
 *   ./camera_test_backend minimal              two cameras, created and destroyed: clean
 *   ./camera_test_backend minimal-acquire      one camera, used, destroyed: clean
 *   ./camera_test_backend minimal-two-used     A used, B created and destroyed: clean
 *
 * The reduced sequences are kept even though none of them reproduces it, because what they rule
 * out is most of what a reader would otherwise guess: it is not two cameras, not a camera that
 * was used, and not a second camera destroyed while the first lives. The `PROBE_SKIP=camera`
 * pair is the reproduction, and it names the call.
 *
 * A second, smaller defect the reduced modes DO show: try_acquire_frame_ext returns SUCCESS and
 * leaves out_acquired unwritten when it acquires nothing. The probe writes a poison value first
 * so that "not written" is visible rather than being read back as whatever the caller had there
 * -- which is a lesson this session learned twice, the first time by reporting a finding that was
 * only its own initialiser.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CNA/C/cna.h"
#include "CNA/C/devices.h"
#include "CNA/C/texture.h"

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

/* A value neither CNA_TRUE nor CNA_FALSE, so a route that leaves an output alone is visible
   instead of being read back as whatever the probe happened to put there. Twice in this session a
   probe reported a finding that was only its own initialiser: `CNA_Bool acquired = CNA_TRUE`
   before a call that does not write on its "nothing to give" path reads back as "acquired". */
#define PROBE_POISON ((CNA_Bool)0xAB)

static const char* took_it(const CNA_Bool flag)
{
    if (flag == PROBE_POISON) return "UNTOUCHED";
    return flag ? "yes" : "no";
}

static const char* state_name(const CNA_CameraState state)
{
    switch (state) {
        case CNA_CAMERA_STATE_NOT_SUPPORTED: return "NOT_SUPPORTED";
        case CNA_CAMERA_STATE_CLOSED: return "CLOSED";
        case CNA_CAMERA_STATE_OPENING: return "OPENING";
        case CNA_CAMERA_STATE_DENIED: return "DENIED";
        case CNA_CAMERA_STATE_READY: return "READY";
        case CNA_CAMERA_STATE_LOST: return "LOST";
        default: return "(other)";
    }
}

/* The smallest sequence that crashes, isolated from everything else this probe does.
   Selected with `./camera_test_backend minimal`. */
static CNA_Result minimal(CNA_Handle game)
{
    printf("minimal: two test-backend cameras, destroy one\n");
    fflush(stdout);
    CNA_CameraHandle first = 0;
    CNA_CameraHandle second = 0;
    printf("  create first          %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &first)));
    printf("  create second         %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &second)));
    fflush(stdout);
    printf("  destroy second        %s\n", name_of(cna_camera_destroy(second)));
    fflush(stdout);
    printf("  destroy first         %s\n", name_of(cna_camera_destroy(first)));
    fflush(stdout);
    printf("minimal: returned\n");
    fflush(stdout);
    return CNA_RESULT_SUCCESS;
}

/* And the same shape with the real backend, to say whether the test backend is what is special. */
static CNA_Result minimal_real(CNA_Handle game)
{
    printf("minimal-real: two real-backend cameras, destroy both\n");
    fflush(stdout);
    CNA_CameraHandle first = 0;
    CNA_CameraHandle second = 0;
    printf("  create first          %s\n", name_of(cna_camera_create(game, &first)));
    printf("  create second         %s\n", name_of(cna_camera_create(game, &second)));
    fflush(stdout);
    printf("  destroy second        %s\n", name_of(cna_camera_destroy(second)));
    fflush(stdout);
    printf("  destroy first         %s\n", name_of(cna_camera_destroy(first)));
    fflush(stdout);
    printf("minimal-real: returned\n");
    fflush(stdout);
    return CNA_RESULT_SUCCESS;
}

/* One test-backend camera, created and destroyed, which the rest of this probe does happily. */
static CNA_Result minimal_one(CNA_Handle game)
{
    printf("minimal-one: one test-backend camera\n");
    fflush(stdout);
    CNA_CameraHandle only = 0;
    printf("  create                %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &only)));
    fflush(stdout);
    printf("  destroy               %s\n", name_of(cna_camera_destroy(only)));
    fflush(stdout);
    printf("minimal-one: returned\n");
    fflush(stdout);
    return CNA_RESULT_SUCCESS;
}

/* One test-backend camera that is asked for a frame before it is destroyed. */
static CNA_Result minimal_acquire(CNA_Handle game, CNA_Handle device, int set_frame)
{
    printf("minimal-acquire: acquire before destroy, frame set: %s\n", set_frame ? "yes" : "no");
    fflush(stdout);
    CNA_CameraHandle only = 0;
    printf("  create                %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &only)));
    if (set_frame) {
        CNA_Color four[4];
        for (int j = 0; j < 4; j++) {
            four[j].r = 200U; four[j].g = 100U; four[j].b = 50U; four[j].a = 255U;
        }
        cna_camera_set_test_state_ext(only, CNA_CAMERA_STATE_READY);
        printf("  set_test_frame        %s\n",
               name_of(cna_camera_set_test_frame_ext(only, 2, 2, four, 4U)));
    }
    CNA_Color blank[4];
    memset(blank, 0, sizeof blank);
    CNA_Handle target = 0;
    printf("  texture               %s\n",
           name_of(cna_texture2d_create_from_rgba8(device, 2U, 2U, blank, 4U, &target)));
    CNA_Bool took = PROBE_POISON;
    {
        const CNA_Result result = cna_camera_try_acquire_frame_ext(only, target, &took);
        printf("  acquire               %s acquired=%s\n",
               name_of(result), took_it(took));
    }
    fflush(stdout);
    printf("  destroy texture       %s\n", name_of(cna_texture2d_destroy(target)));
    fflush(stdout);
    printf("  destroy camera        %s\n", name_of(cna_camera_destroy(only)));
    fflush(stdout);
    printf("minimal-acquire: returned\n");
    fflush(stdout);
    return CNA_RESULT_SUCCESS;
}

/* Camera A is used, then camera B is created and destroyed while A is still alive. This is the
   shape the full probe crashes in, reduced to its two cameras and one acquisition. */
static CNA_Result minimal_two_used(CNA_Handle game, CNA_Handle device)
{
    printf("minimal-two-used: use A, then create and destroy B while A lives\n");
    fflush(stdout);
    CNA_Color four[4];
    for (int j = 0; j < 4; j++) {
        four[j].r = 200U; four[j].g = 100U; four[j].b = 50U; four[j].a = 255U;
    }
    CNA_Color blank[4];
    memset(blank, 0, sizeof blank);

    CNA_CameraHandle a = 0;
    printf("  create A              %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &a)));
    cna_camera_set_test_state_ext(a, CNA_CAMERA_STATE_READY);
    printf("  frame into A          %s\n",
           name_of(cna_camera_set_test_frame_ext(a, 2, 2, four, 4U)));
    CNA_Handle target_a = 0;
    cna_texture2d_create_from_rgba8(device, 2U, 2U, blank, 4U, &target_a);
    CNA_Bool took_a = PROBE_POISON;
    {
        const CNA_Result result = cna_camera_try_acquire_frame_ext(a, target_a, &took_a);
        printf("  acquire from A        %s acquired=%s\n",
               name_of(result), took_it(took_a));
    }
    fflush(stdout);

    CNA_CameraHandle b = 0;
    printf("  create B              %s\n",
           name_of(cna_camera_create_with_test_backend_ext(game, &b)));
    CNA_Handle target_b = 0;
    cna_texture2d_create_from_rgba8(device, 2U, 2U, blank, 4U, &target_b);
    CNA_Bool took_b = PROBE_POISON;
    {
        const CNA_Result result = cna_camera_try_acquire_frame_ext(b, target_b, &took_b);
        printf("  acquire from B        %s acquired=%s\n",
               name_of(result), took_it(took_b));
    }
    fflush(stdout);
    printf("  destroy B's texture   %s\n", name_of(cna_texture2d_destroy(target_b)));
    fflush(stdout);
    printf("  destroy B             %s\n", name_of(cna_camera_destroy(b)));
    fflush(stdout);
    printf("  destroy A's texture   %s\n", name_of(cna_texture2d_destroy(target_a)));
    printf("  destroy A             %s\n", name_of(cna_camera_destroy(a)));
    fflush(stdout);
    printf("minimal-two-used: returned\n");
    fflush(stdout);
    return CNA_RESULT_SUCCESS;
}

static const char* probe_mode = NULL;

static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    if (ran) return CNA_RESULT_SUCCESS;
    ran = 1;

    if (probe_mode != NULL) {
        if (strcmp(probe_mode, "minimal") == 0) return minimal(game);
        if (strcmp(probe_mode, "minimal-real") == 0) return minimal_real(game);
        if (strcmp(probe_mode, "minimal-one") == 0) return minimal_one(game);
        if (strcmp(probe_mode, "minimal-two-used") == 0) {
            CNA_Handle two_device = CNA_INVALID_HANDLE;
            if (cna_game_get_graphics_device(game, &two_device) != CNA_RESULT_SUCCESS) {
                printf("no device\n");
                return CNA_RESULT_SUCCESS;
            }
            return minimal_two_used(game, two_device);
        }
        if (strncmp(probe_mode, "minimal-acquire", 15) == 0) {
            CNA_Handle only_device = CNA_INVALID_HANDLE;
            if (cna_game_get_graphics_device(game, &only_device) != CNA_RESULT_SUCCESS) {
                printf("no device\n");
                return CNA_RESULT_SUCCESS;
            }
            return minimal_acquire(game, only_device,
                                   strcmp(probe_mode, "minimal-acquire-frame") == 0);
        }
    }

    CNA_Handle device = CNA_INVALID_HANDLE;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS) {
        printf("no device\n");
        return CNA_RESULT_SUCCESS;
    }

    printf("== what this host really has ==\n");
    {
        CNA_Bool supported = CNA_FALSE;
        uint64_t count = 0;
        {
            const CNA_Result result = cna_camera_get_is_supported_ext(game, &supported);
            printf("  is_supported          %s %s\n",
                   name_of(result),
                   supported ? "yes" : "no");
        }
        {
            const CNA_Result result = cna_camera_get_count_ext(game, &count);
            printf("  count                 %s %llu\n",
                   name_of(result), (unsigned long long)count);
        }
        for (uint64_t i = 0; i < count; i++) {
            CNA_CameraDeviceInfo info;
            memset(&info, 0, sizeof info);
            info.struct_size = (uint32_t)(sizeof info);
            info.struct_version = 1U;
            const CNA_Result got = cna_camera_get_info_at_ext(game, i, &info);
            uint64_t bytes = 0;
            char name[128];
            memset(name, 0, sizeof name);
            cna_camera_copy_name_at_ext(game, i, name, sizeof name, &bytes);
            printf("    [%llu] %s position %u \"%s\"\n", (unsigned long long)i, name_of(got),
                   (unsigned)info.position, name);
        }
        CNA_CameraHandle real = 0;
        const CNA_Result made = cna_camera_create(game, &real);
        printf("  create (real backend) %s\n", name_of(made));
        if (made == CNA_RESULT_SUCCESS) cna_camera_destroy(real);

        CNA_CameraDeviceInfo defaults;
        memset(&defaults, 0xEE, sizeof defaults);
        const CNA_Result init = cna_camera_device_info_init(&defaults);
        printf("  device_info_init      %s size=%u version=%u position=%u\n", name_of(init),
               defaults.struct_size, defaults.struct_version, (unsigned)defaults.position);
    }

    printf("\n== the test backend ==\n");
    CNA_CameraHandle camera = 0;
    const CNA_Result made = cna_camera_create_with_test_backend_ext(game, &camera);
    printf("  create_with_test      %s\n", name_of(made));
    if (made != CNA_RESULT_SUCCESS) {
        printf("PROBE INCOMPLETE\n");
        return CNA_RESULT_SUCCESS;
    }
    {
        CNA_CameraState state = 0;
        {
            const CNA_Result result = cna_camera_get_state_ext(camera, &state);
            printf("  state at birth        %s %s\n",
                   name_of(result), state_name(state));
        }
        int32_t width = -1, height = -1;
        {
            const CNA_Result result = cna_camera_get_frame_width_ext(camera, &width);
            printf("  frame size at birth   %s %d x %s %d\n",
                   name_of(result), width,
                   name_of(cna_camera_get_frame_height_ext(camera, &height)), height);
        }
    }

    printf("\n== every state the test backend can be put in ==\n");
    for (CNA_CameraState wanted = CNA_CAMERA_STATE_NOT_SUPPORTED;
         wanted <= CNA_CAMERA_STATE_MAXIMUM + 1U; wanted++) {
        const CNA_Result set = cna_camera_set_test_state_ext(camera, wanted);
        CNA_CameraState read = 0;
        const CNA_Result got = cna_camera_get_state_ext(camera, &read);
        printf("  set %-14s    %s  reads back %s %s\n", state_name(wanted), name_of(set),
               name_of(got), state_name(read));
    }

    printf("\n== a frame in, a frame out ==\n");
    {
        cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_READY);

        /* A frame nothing could produce by accident: each texel's red channel is its index. */
        enum { W = 4, H = 3, N = W * H };
        CNA_Color frame[N];
        for (int i = 0; i < N; i++) {
            frame[i].r = (uint8_t)(i * 17);
            frame[i].g = (uint8_t)(255 - i * 17);
            frame[i].b = (uint8_t)(i * 3);
            frame[i].a = 255U;
        }
        printf("  set_test_frame        %s\n",
               name_of(cna_camera_set_test_frame_ext(camera, W, H, frame, (uint64_t)N)));
        int32_t width = 0, height = 0;
        cna_camera_get_frame_width_ext(camera, &width);
        cna_camera_get_frame_height_ext(camera, &height);
        printf("  frame size now        %d x %d\n", width, height);

        /* The header says a texture whose size does not match is refused and not resized, so
           establish which refusal that is before writing a Java facade that has to explain it. */
        CNA_Color blank[N];
        memset(blank, 0, sizeof blank);
        CNA_Handle wrong = 0;
        cna_texture2d_create_from_rgba8(device, 2U, 2U, blank, 4U, &wrong);
        CNA_Bool acquired = PROBE_POISON;
        {
            const CNA_Result result = cna_camera_try_acquire_frame_ext(camera, wrong, &acquired);
            printf("  acquire, wrong size   %s acquired=%s\n",
                   name_of(result),
                   took_it(acquired));
        }
        if (wrong != 0) cna_texture2d_destroy(wrong);

        CNA_Handle texture = 0;
        cna_texture2d_create_from_rgba8(device, (uint32_t)W, (uint32_t)H, blank, (uint64_t)N,
                                        &texture);
        acquired = PROBE_POISON;
        const CNA_Result got = cna_camera_try_acquire_frame_ext(camera, texture, &acquired);
        printf("  acquire, right size   %s acquired=%s\n", name_of(got), took_it(acquired));

        CNA_Color read[N];
        memset(read, 0, sizeof read);
        CNA_Texture2DTransfer transfer;
        memset(&transfer, 0, sizeof transfer);
        transfer.struct_size = (uint32_t)(sizeof transfer);
        transfer.struct_version = 1U;
        transfer.element_count = (uint64_t)N;
        uint64_t written = 0;
        const CNA_Result readback =
            cna_texture2d_get_data(texture, CNA_TEXTURE_DATA_COLOR, &transfer, read,
                                   (uint64_t)N, &written);
        int same = 1;
        for (int i = 0; i < N; i++) {
            if (read[i].r != frame[i].r || read[i].g != frame[i].g || read[i].b != frame[i].b) {
                same = 0;
            }
        }
        printf("  readback              %s  pixels match what went in: %s\n", name_of(readback),
               same ? "yes" : "NO");
        printf("  first texel in/out    %u,%u,%u -> %u,%u,%u\n", frame[0].r, frame[0].g,
               frame[0].b, read[0].r, read[0].g, read[0].b);
        printf("  last  texel in/out    %u,%u,%u -> %u,%u,%u\n", frame[N - 1].r, frame[N - 1].g,
               frame[N - 1].b, read[N - 1].r, read[N - 1].g, read[N - 1].b);

        /* Documented: no frame ready is an ordinary CNA_FALSE, not a failure. Acquiring twice
           is how to find out whether a frame is consumed or kept. */
        acquired = PROBE_POISON;
        const CNA_Result again = cna_camera_try_acquire_frame_ext(camera, texture, &acquired);
        printf("  acquire again         %s acquired=%s%s\n", name_of(again), took_it(acquired),
               acquired == CNA_TRUE ? " -- the frame is kept" : " -- the frame was consumed");

        /* And what each non-READY state does to acquisition. */
        for (CNA_CameraState wanted = CNA_CAMERA_STATE_NOT_SUPPORTED;
             wanted <= CNA_CAMERA_STATE_MAXIMUM; wanted++) {
            if (wanted == CNA_CAMERA_STATE_READY) continue;
            cna_camera_set_test_state_ext(camera, wanted);
            cna_camera_set_test_frame_ext(camera, W, H, frame, (uint64_t)N);
            acquired = PROBE_POISON;
            const CNA_Result in_state =
                cna_camera_try_acquire_frame_ext(camera, texture, &acquired);
            printf("  acquire in %-14s %s acquired=%s\n", state_name(wanted), name_of(in_state),
                   took_it(acquired));
        }
        if (texture != 0) cna_texture2d_destroy(texture);
    }

    printf("\n== the size mismatch the header says is refused ==\n");
    {
        /* "a texture whose size does not match the frame is refused the same way -- the canonical
           route neither resizes it nor reports why." Measured, a 4x3 frame goes into a 2x2
           texture and answers SUCCESS. The question that decides how serious that is: does it
           write twelve texels into a four-texel texture? */
        cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_READY);
        enum { BIG = 64, BIG_N = BIG * BIG };
        CNA_Color* big = (CNA_Color*)malloc(sizeof(CNA_Color) * BIG_N);
        for (int i = 0; i < BIG_N; i++) {
            big[i].r = 200U; big[i].g = 100U; big[i].b = 50U; big[i].a = 255U;
        }
        printf("  set a %dx%d frame     %s\n", BIG, BIG,
               name_of(cna_camera_set_test_frame_ext(camera, BIG, BIG, big, (uint64_t)BIG_N)));

        CNA_Color small[4];
        memset(small, 0, sizeof small);
        CNA_Handle tiny = 0;
        cna_texture2d_create_from_rgba8(device, 2U, 2U, small, 4U, &tiny);
        CNA_Bool acquired = PROBE_POISON;
        const CNA_Result got = cna_camera_try_acquire_frame_ext(camera, tiny, &acquired);
        printf("  acquire into 2x2      %s acquired=%s\n", name_of(got), took_it(acquired));

        /* If it wrote at all, the 2x2 now holds the frame's colour rather than the black it was
           created with -- and whether it wrote FOUR texels or four thousand is the difference
           between a documentation defect and a heap overflow. Read back exactly four. */
        CNA_Color out[4];
        memset(out, 0, sizeof out);
        CNA_Texture2DTransfer tiny_transfer;
        memset(&tiny_transfer, 0, sizeof tiny_transfer);
        tiny_transfer.struct_size = (uint32_t)(sizeof tiny_transfer);
        tiny_transfer.struct_version = 1U;
        tiny_transfer.element_count = 4U;
        uint64_t tiny_written = 0;
        printf("  readback of the 2x2   %s  first texel %u,%u,%u\n",
               name_of(cna_texture2d_get_data(tiny, CNA_TEXTURE_DATA_COLOR, &tiny_transfer, out,
                                              4U, &tiny_written)),
               out[0].r, out[0].g, out[0].b);
        printf("  wrote the frame in    %s\n",
               out[0].r == 200U ? "YES -- and the header says this is refused" : "no");
        if (tiny != 0) cna_texture2d_destroy(tiny);
        free(big);

        /* 4x3 into a 2x2 answered "acquired" a moment ago and 64x64 into the same 2x2 did not,
           so the check is not simply "the sizes match". Walk the frame size against one fixed
           2x2 texture and read the four texels back each time -- an "acquired" that also wrote
           is the case that matters. */
        printf("  frame size vs a fixed 2x2 texture:\n");
        const int sizes[][2] = { {2,2}, {3,2}, {2,3}, {3,3}, {4,3}, {4,4}, {8,8}, {16,16} };
        for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
            const int w = sizes[i][0], h = sizes[i][1];
            CNA_Color* pixels = (CNA_Color*)malloc(sizeof(CNA_Color) * (size_t)(w * h));
            for (int j = 0; j < w * h; j++) {
                pixels[j].r = 200U; pixels[j].g = 100U; pixels[j].b = 50U; pixels[j].a = 255U;
            }
            cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_READY);
            const CNA_Result set =
                cna_camera_set_test_frame_ext(camera, w, h, pixels, (uint64_t)(w * h));
            CNA_Color blank2[4];
            memset(blank2, 0, sizeof blank2);
            CNA_Handle target = 0;
            cna_texture2d_create_from_rgba8(device, 2U, 2U, blank2, 4U, &target);
            CNA_Bool took = PROBE_POISON;
            const CNA_Result got2 = cna_camera_try_acquire_frame_ext(camera, target, &took);
            CNA_Color out2[4];
            memset(out2, 0, sizeof out2);
            CNA_Texture2DTransfer t2;
            memset(&t2, 0, sizeof t2);
            t2.struct_size = (uint32_t)(sizeof t2);
            t2.struct_version = 1U;
            t2.element_count = 4U;
            uint64_t w2 = 0;
            cna_texture2d_get_data(target, CNA_TEXTURE_DATA_COLOR, &t2, out2, 4U, &w2);
            printf("    %2dx%-2d  set=%-16s acquire=%-8s acquired=%-3s wrote=%s\n", w, h,
                   name_of(set), name_of(got2), took_it(took),
                   out2[0].r == 200U ? "YES" : "no");
            if (target != 0) cna_texture2d_destroy(target);
            free(pixels);
        }
    }

    printf("\n== when does the route return SUCCESS without writing its output ==\n");
    {
        /* One mismatched acquire answered UNTOUCHED and eight others answered "no", for what look
           like the same inputs. The difference has to be found rather than guessed: a route that
           sometimes leaves its only output unwritten is worse than one that always does, because a
           caller that tested it once will believe it. */
        struct { const char* what; int set_state; int set_frame; } cases[] = {
            { "fresh camera, no state, no frame", 0, 0 },
            { "fresh camera, READY, no frame",    1, 0 },
            { "fresh camera, no state, a frame",  0, 1 },
            { "fresh camera, READY, a frame",     1, 1 },
        };
        const char* only = getenv("PROBE_CASE");
        for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            if (only != NULL && (size_t)atoi(only) != i) continue;
            printf("  trying case %zu: %s\n", i, cases[i].what);
            fflush(stdout);
            CNA_CameraHandle fresh = 0;
            if (cna_camera_create_with_test_backend_ext(game, &fresh) != CNA_RESULT_SUCCESS) {
                continue;
            }
            if (cases[i].set_state) {
                cna_camera_set_test_state_ext(fresh, CNA_CAMERA_STATE_READY);
            }
            if (cases[i].set_frame) {
                /* Deliberately 4x3, which does not match the 2x2 texture below. */
                CNA_Color twelve[12];
                for (int j = 0; j < 12; j++) {
                    twelve[j].r = 200U; twelve[j].g = 100U; twelve[j].b = 50U; twelve[j].a = 255U;
                }
                cna_camera_set_test_frame_ext(fresh, 4, 3, twelve, 12U);
            }
            CNA_Color blank3[4];
            memset(blank3, 0, sizeof blank3);
            CNA_Handle target = 0;
            cna_texture2d_create_from_rgba8(device, 2U, 2U, blank3, 4U, &target);
            CNA_Bool took = PROBE_POISON;
            const CNA_Result got = cna_camera_try_acquire_frame_ext(fresh, target, &took);
            printf("  %-34s %s acquired=%s\n", cases[i].what, name_of(got), took_it(took));
            fflush(stdout);
            const char* skip = getenv("PROBE_SKIP");
            if (skip == NULL || strcmp(skip, "texture") != 0) {
                printf("    destroying the texture\n"); fflush(stdout);
                if (target != 0) cna_texture2d_destroy(target);
            }
            if (skip == NULL || strcmp(skip, "camera") != 0) {
                printf("    destroying the camera\n"); fflush(stdout);
                cna_camera_destroy(fresh);
            }
            printf("    case done\n"); fflush(stdout);
        }
    }

    printf("\n== refusals ==\n");
    {
        CNA_Color one = { 1U, 2U, 3U, 4U };
        printf("  frame with a bad count      %s\n",
               name_of(cna_camera_set_test_frame_ext(camera, 4, 3, &one, 1U)));
        printf("  frame with a zero dimension %s\n",
               name_of(cna_camera_set_test_frame_ext(camera, 0, 3, &one, 1U)));
        CNA_Bool acquired = PROBE_POISON;
        printf("  acquire into no texture     %s\n",
               name_of(cna_camera_try_acquire_frame_ext(camera, CNA_INVALID_HANDLE, &acquired)));
        CNA_CameraDeviceInfo info;
        printf("  info at an index past the end %s\n",
               name_of(cna_camera_get_info_at_ext(game, 9999U, &info)));
    }

    printf("\n  destroy               %s\n", name_of(cna_camera_destroy(camera)));
    printf("  destroy again         %s\n", name_of(cna_camera_destroy(camera)));
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    probe_mode = argc >= 2 ? argv[1] : NULL;
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
    info.window_title.data = "camera test backend";
    info.window_title.byte_length = 19U;
    info.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) {
        printf("game create failed\n");
        return 1;
    }
    CNA_GraphicsDeviceManagerHandle manager = 0;
    (void)cna_graphics_device_manager_create(game, &manager);
    (void)cna_game_run_one_frame(game);
    if (manager != 0) (void)cna_graphics_device_manager_destroy(manager);
    printf("\ngame destroy       %s\n", name_of(cna_game_destroy(game)));
    printf("PROBE %s\n", ran ? "OK" : "INCOMPLETE");
    return ran ? 0 : 1;
}
