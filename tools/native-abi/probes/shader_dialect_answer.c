/* SPDX-License-Identifier: MS-PL */
/*
 * Does the route that exists so a caller need not guess the shading dialect ever answer one?
 *
 * `cna_shader_effect_create` takes renderer-specific text, and graphics.h is explicit about how a
 * caller is meant to choose it:
 *
 *     "the renderer identity is not a safe way to infer which text to supply: it is wrong in a
 *      build carrying several renderers, and meaningless for a renderer that picks its native API
 *      per process. Ask here instead."
 *
 * This build carries five renderers, which is exactly the case that sentence names. So ask.
 *
 * Run once per renderer:
 *
 *     ./shader_dialect_answer          (once per single-renderer artifact)
 *
 * Re-measured 2026-09-30 against CNA C ABI 0.35.0:
 *
 *     HEADLESS   dialect=UNKNOWN custom_effects=yes glsl_es_compiles=yes
 *     OPENGLES3  dialect=GLSL_ES custom_effects=yes glsl_es_compiles=yes
 *
 * EasyGL now reports its dialect (CNA 6ff9d1b95); HEADLESS still answers UNKNOWN, which is
 * accurate for a renderer that accepts source without executing it. The original measurement,
 * on CNA C ABI 0.21.0:
 *
 *     HEADLESS   dialect=UNKNOWN custom_effects=yes shader_compiles=yes
 *     SOFTWARE   dialect=UNKNOWN custom_effects=yes shader_compiles=yes
 *     OPENGL4    dialect=UNKNOWN custom_effects=yes shader_compiles=yes
 *     OPENGLES3  dialect=UNKNOWN custom_effects=yes shader_compiles=yes
 *     OPENGL33   dialect=UNKNOWN custom_effects=yes shader_compiles=yes
 *
 * All five answer UNKNOWN, which the header defines as "do not guess" -- the one answer a caller
 * is told to refuse to build sources from. Two of the five demonstrably compile and execute GLSL
 * ES: this projection's own shader-effect suite writes `#version 300 es` and reads the resulting
 * pixel back on OPENGL33 and OPENGLES3. So the answer is not merely unhelpful, it contradicts
 * what those renderers do.
 *
 * The cause is visible in IGraphicsRenderer.hpp: `GetShaderDialectEXT()` is a virtual with a
 * default body returning `Unknown`, and WebGPURenderer is the only renderer in the tree that
 * overrides it.
 *
 * `cna_graphics_device_supports_capability(CUSTOM_EFFECTS)` is no substitute and is measured here
 * beside it: it answers yes on all five, including the two that compile no shader at all.
 *
 * Filed as JAVA-UPSTREAM-022.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CNA/C/cna.h"
#include "CNA/C/graphics.h"
#include "CNA/C/effects.h"

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
        default: return "OTHER";
    }
}

/* Not a dialect this ABI names, so "the route did not write" is visible rather than read back as
   UNKNOWN -- which is itself a meaningful answer and must not be confused with silence. */
#define POISON_DIALECT ((CNA_ShaderDialect)0xEEEEEEEEU)

static const char* dialect_name(const CNA_ShaderDialect value)
{
    switch (value) {
        case CNA_SHADER_DIALECT_UNKNOWN: return "UNKNOWN";
        case CNA_SHADER_DIALECT_GLSL_DESKTOP: return "GLSL_DESKTOP";
        case CNA_SHADER_DIALECT_GLSL_ES: return "GLSL_ES";
        case CNA_SHADER_DIALECT_GLSL_VULKAN: return "GLSL_VULKAN";
        case CNA_SHADER_DIALECT_HLSL: return "HLSL";
        case CNA_SHADER_DIALECT_MSL: return "MSL";
        case CNA_SHADER_DIALECT_WGSL: return "WGSL";
        default: return "UNTOUCHED";
    }
}

static CNA_StringView view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static const char* const VERTEX =
    "#version 300 es\n"
    "in vec2 a_position;\n"
    "void main() { gl_Position = vec4(a_position, 0.0, 1.0); }\n";

static const char* const FRAGMENT =
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 fragment;\n"
    "void main() { fragment = vec4(1.0, 0.0, 0.0, 1.0); }\n";

static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    if (ran) return CNA_RESULT_SUCCESS;
    ran = 1;

    CNA_Handle device = CNA_INVALID_HANDLE;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS) {
        printf("no device\n");
        return CNA_RESULT_SUCCESS;
    }

    char renderer[64];
    memset(renderer, 0, sizeof renderer);
    uint64_t written = 0;
    (void)cna_graphics_device_copy_renderer_name(device, renderer, sizeof renderer, &written);

    CNA_ShaderDialect dialect = POISON_DIALECT;
    const CNA_Result asked = cna_graphics_device_get_shader_dialect_ext(device, &dialect);

    CNA_Bool custom = CNA_FALSE;
    const CNA_Result capability = cna_graphics_device_supports_capability(
        device, CNA_GRAPHICS_CAPABILITY_CUSTOM_EFFECTS, &custom);

    /* And whether a GLSL ES source actually compiles here, which is the behaviour the dialect
       answer is supposed to predict. */
    CNA_EffectHandle effect = 0;
    const CNA_Result made = cna_shader_effect_create(device, view(VERTEX), view(FRAGMENT),
                                                     &effect);
    CNA_Bool valid = CNA_FALSE;
    if (made == CNA_RESULT_SUCCESS) {
        (void)cna_shader_effect_is_valid(effect, &valid);
        (void)cna_effect_destroy(effect);
    }

    printf("%-10s dialect=%s (%s) custom_effects=%s (%s) glsl_es_compiles=%s\n",
           renderer, dialect_name(dialect), name_of(asked), custom ? "yes" : "no",
           name_of(capability),
           made != CNA_RESULT_SUCCESS ? "refused" : (valid ? "yes" : "no"));
    return CNA_RESULT_SUCCESS;
}

int main(void)
{
    const char* requested = getenv("CNA_GRAPHICS_RENDERER");
    printf("requested %s -> ", requested != NULL ? requested : "<build default>");
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
    info.window_title = view("shader dialect answer");
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
    (void)cna_game_destroy(game);
    return ran ? 0 : 1;
}
