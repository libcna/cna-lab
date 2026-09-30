/* RUST-UPSTREAM-030 reproducer: a technique added to an effect's own collection through the C API
 * cannot be selected as that effect's current technique. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>
static void diag(const char* what, CNA_Result r) {
    char buf[512] = {0}; uint64_t n = 0, w = 0;
    cna_error_get_last_message_size(&n);
    if (n && n < sizeof buf) cna_error_copy_last_message(buf, sizeof buf, &w);
    printf("%s -> %u%s%.*s\n", what, (unsigned)r, r ? " : " : "", r ? (int)n : 0, buf);
}
static CNA_Result on_load(CNA_Handle game, const CNA_GameTime* gt, void* ctx, CNA_CallbackError* e) {
    (void)gt; (void)ctx; (void)e;
    CNA_Handle device = 0; CNA_EffectHandle effect = 0; CNA_EffectTechniqueCollectionHandle techniques = 0;
    CNA_EffectTechniqueHandle named = 0, fallback = 0;
    cna_game_get_graphics_device(game, &device);
    diag("cna_effect_create_empty", cna_effect_create_empty(device, &effect));
    diag("cna_effect_get_techniques", cna_effect_get_techniques(effect, &techniques));
    const CNA_StringView name = {"T0", 2};
    diag("cna_effect_technique_collection_add_named", cna_effect_technique_collection_add_named(techniques, name, &named));
    diag("cna_effect_set_current_technique(named)", cna_effect_set_current_technique(effect, named));
    diag("cna_effect_technique_collection_add_default", cna_effect_technique_collection_add_default(techniques, &fallback));
    diag("cna_effect_set_current_technique(default)", cna_effect_set_current_technique(effect, fallback));
    cna_effect_technique_destroy(fallback); cna_effect_technique_destroy(named);
    cna_effect_technique_collection_destroy(techniques); cna_effect_destroy(effect);
    return CNA_RESULT_SUCCESS;
}
int main(void) {
    const CNA_GameCallbacks cb = { sizeof(CNA_GameCallbacks), 1, on_load, 0, 0, 0, 0, 0 };
    const CNA_GameCreateInfo gi = { sizeof(CNA_GameCreateInfo), 1, CNA_TRUE, {0,0,0,0,0,0,0}, 166667, {"rs", 2}, &cb };
    CNA_Handle game = 0;
    diag("cna_game_create", cna_game_create(&gi, &game));
    diag("cna_game_run_one_frame", cna_game_run_one_frame(game));
    diag("cna_game_destroy", cna_game_destroy(game));
    return 0;
}
