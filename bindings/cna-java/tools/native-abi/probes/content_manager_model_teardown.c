/* SPDX-License-Identifier: MS-PL */
/*
 * JAVA-UPSTREAM-004, and the reason `CnaModel.Load` does not exist.
 *
 * Loading a Model through CNA's own content manager and destroying it segfaults inside
 * `PartResource::~PartResource` for any asset whose meshes have parts -- which is every real
 * model. CNA's own content fixtures are models with one bone and no meshes, which is why the
 * path is uncovered upstream.
 *
 * Kept as a source probe rather than a note, because "still broken" is a measurement that has to
 * be retaken against each CNA this repository qualifies against, and a segfault is not something
 * a Java test can report.
 *
 *   ./build-probe/content_manager_model_teardown <root-directory> <asset-name>
 *
 * The root is a directory holding the asset and the name is its stem, the way a content manager
 * takes them. `PROBE_NO_DESTROY=1` leaves the model to the manager instead, and `PROBE_WALK=1`
 * walks the mesh collection first.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_root;
static const char* g_asset;

static CNA_StringView view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static uint32_t on_load(uint64_t game, const CNA_GameTime* time, void* context,
                        CNA_CallbackError* error)
{
    (void)time;
    (void)context;
    (void)error;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result devices = cna_game_get_graphics_device(game, &device);
    printf("get_graphics_device=%u\n", devices);
    if (devices != CNA_RESULT_SUCCESS) {
        return 0;
    }
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_ContentManagerCreateInfo info;
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)(sizeof info);
    info.struct_version = UINT32_C(1);
    info.root_directory = view(g_root);
    CNA_Result result = cna_content_manager_create(device, &info, &manager);
    printf("content_manager_create=%u\n", result);
    if (result != CNA_RESULT_SUCCESS) {
        return 0;
    }
    CNA_ModelHandle model = CNA_INVALID_HANDLE;
    result = cna_content_manager_load_model(manager, view(g_asset), &model);
    printf("load_model=%u model=%llu\n", result, (unsigned long long)model);
    if (result != CNA_RESULT_SUCCESS) {
        return 0;
    }
    if (getenv("PROBE_WALK") != 0) {
        CNA_ModelMeshCollectionHandle meshes = CNA_INVALID_HANDLE;
        uint64_t count = 0U;
        if (cna_model_get_meshes(model, &meshes) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_collection_get_count(meshes, &count) == CNA_RESULT_SUCCESS) {
            printf("meshes=%llu\n", (unsigned long long)count);
            (void)cna_model_mesh_collection_destroy(meshes);
        }
    }
    if (getenv("PROBE_NO_DESTROY") != 0) {
        printf("leaving the model to the content manager\n");
        fflush(stdout);
        return 0;
    }
    printf("destroying\n");
    fflush(stdout);
    result = cna_model_destroy(model);
    printf("model_destroy=%u\n", result);
    fflush(stdout);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("usage: %s <root-directory> <asset-name>\n", argv[0]);
        return 2;
    }
    g_root = argv[1];
    g_asset = argv[2];

    CNA_GameCallbacks callbacks;
    memset(&callbacks, 0, sizeof callbacks);
    callbacks.struct_size = (uint32_t)(sizeof callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = on_load;

    CNA_GameCreateInfo create;
    memset(&create, 0, sizeof create);
    create.struct_size = (uint32_t)(sizeof create);
    create.struct_version = UINT32_C(1);
    create.is_fixed_time_step = CNA_TRUE;
    create.target_elapsed_time_ticks = INT64_C(166667);
    create.window_title = view("model destroy reproduction");
    create.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Result result = cna_game_create(&create, &game);
    printf("game_create=%u\n", result);
    if (result != CNA_RESULT_SUCCESS) {
        return 3;
    }
    result = cna_game_run_one_frame(game);
    printf("run_one_frame=%u\n", result);
    (void)cna_game_destroy(game);
    printf("done\n");
    return 0;
}
