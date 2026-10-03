/* RUST-UPSTREAM-031 reproducer: exiting while the shared avatar loader thread is assembling a model
 * crashes in static destruction. The first avatar is loaded to completion, which constructs the
 * function-local statics the build reads (tintColor's colour-slot map among them) *after* the
 * function-local Loader; more loads are then queued and the process returns from main at once. At
 * exit those statics are destroyed before ~Loader joins the still-running worker, which then reads
 * a destroyed std::map. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static CNA_AvatarRendererHandle make(void) {
    CNA_AvatarDescriptionHandle description = 0;
    CNA_AvatarRendererHandle renderer = 0;
    if (cna_avatar_description_create_random(&description) != CNA_RESULT_SUCCESS ||
        cna_avatar_renderer_create(description, CNA_TRUE, &renderer) != CNA_RESULT_SUCCESS) {
        exit(2);
    }
    cna_avatar_description_destroy(description);
    return renderer;
}
int main(int argc, char** argv) {
    const int count = argc > 1 ? atoi(argv[1]) : 8;
    CNA_AvatarRendererHandle first = make();
    CNA_AvatarRendererInfo info = {sizeof(CNA_AvatarRendererInfo), 1, 0, 0, {0, 0, 0}};
    for (int i = 0; i < 400; ++i) {
        cna_avatar_renderer_get_info(first, &info);
        if (info.state != CNA_AVATAR_RENDERER_STATE_LOADING) break;
        const struct timespec pause = {0, 10000000};
        nanosleep(&pause, 0);
    }
    printf("first avatar state %u\n", (unsigned)info.state);
    for (int i = 0; i < count; ++i) {
        (void)make();
    }
    printf("queued %d more avatar loads; returning from main\n", count);
    return 0;
}
