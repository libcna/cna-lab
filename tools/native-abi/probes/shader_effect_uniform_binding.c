/* SPDX-License-Identifier: MS-PL */
/*
 * JAVA-UPSTREAM-016. When does a uniform set on a ShaderEffect actually reach the shader?
 *
 * `cna_shader_effect_set_uniform_*` succeeds whatever else is going on, and on CNA's EasyGL
 * renderer the value is silently discarded unless the effect's own GL program happens to be the
 * current one. `EasyGLEffectRenderer::SetUniformFloat` and its eight siblings call
 * `program_.uniform_location(name)` and `program_.set_uniform(...)` without binding first, and
 * `glUniform*` writes to whichever program is current -- so a uniform set before the effect is
 * applied goes to another program or nowhere at all.
 *
 * The same renderer's compute path does bind first: `EasyGLComputeShaderRenderer::SetUniformInt`
 * opens with `program_.use()`. Two uniform setters in one renderer, one of which works from a
 * cold start and one of which does not.
 *
 * This measures it with no Java in the picture: a fragment shader that writes nothing but a
 * uniform, drawn through SpriteBatch into a render target and read back -- with the uniform set
 * and the effect never applied by the caller (SpriteBatch applies it, as XNA's does), set before
 * an explicit apply, and set after one. Every case uses a fresh effect, so the first draw through
 * a new ShaderEffect is measured each time.
 *
 * CNA fixed the EasyGL setters in BINDFIX-025 (they now make their own program current); on a
 * library with the fix all three cases paint the uniform.
 *
 *   ./build-probe/shader_effect_uniform_binding     (on a windowed EasyGL artifact)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CNA/C/cna.h"

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

static CNA_StringView view_of(const char* text)
{
    CNA_StringView view;
    view.data = text;
    view.byte_length = (uint64_t)strlen(text);
    return view;
}

/* SpriteBatch's vertex layout: position, texture coordinate and colour at locations nought, one
   and two, and a `projection` uniform SpriteBatch sets. A shader that names its attributes
   anything else compiles and draws nothing. */
static const char* const kVertex =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "layout(location = 2) in vec4 aColor;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 projection;\n"
    "void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); TexCoord = aTexCoord; }\n";

static const char* const kFragment =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform vec4 u_colour;\n"
    "void main() { FragColor = u_colour; }\n";

enum { kNoApply = 0, kApplyAfterUniform = 1, kApplyBeforeUniform = 2 };

static void one_case(CNA_Handle device, const char* label, int order)
{
    CNA_EffectHandle shader = 0;
    const CNA_Result made =
        cna_shader_effect_create(device, view_of(kVertex), view_of(kFragment), &shader);
    if (made != CNA_RESULT_SUCCESS) {
        printf("  %-22s create %s\n", label, name_of(made));
        return;
    }
    CNA_Bool valid = CNA_FALSE;
    cna_shader_effect_is_valid(shader, &valid);

    enum { kSize = 8 };
    CNA_Color source_pixels[kSize * kSize];
    for (int index = 0; index < kSize * kSize; ++index) {
        source_pixels[index].r = 0U; source_pixels[index].g = 255U;
        source_pixels[index].b = 0U; source_pixels[index].a = 255U;
    }
    CNA_Handle source = CNA_INVALID_HANDLE;
    cna_texture2d_create_from_rgba8(device, kSize, kSize, source_pixels,
                                    (uint64_t)(kSize * kSize), &source);

    CNA_RenderTarget2DCreateInfo target_info;
    memset(&target_info, 0, sizeof target_info);
    target_info.struct_size = (uint32_t)(sizeof target_info);
    target_info.struct_version = 1U;
    target_info.width = kSize;
    target_info.height = kSize;
    target_info.format = CNA_SURFACE_FORMAT_COLOR;
    target_info.depth_format = CNA_DEPTH_FORMAT_NONE;
    target_info.usage = CNA_RENDER_TARGET_USAGE_DISCARD_CONTENTS;
    CNA_Handle destination = CNA_INVALID_HANDLE;
    cna_render_target2d_create(device, &target_info, &destination);

    CNA_Handle batch = CNA_INVALID_HANDLE;
    cna_sprite_batch_create(device, &batch);

    CNA_Result applied = CNA_RESULT_SUCCESS;
    if (order == kApplyBeforeUniform) {
        applied = cna_effect_apply(shader);
    }
    CNA_Vector4 colour;
    colour.x = 1.0F; colour.y = 0.0F; colour.z = 0.0F; colour.w = 1.0F;
    const CNA_Result set =
        cna_shader_effect_set_uniform_vector4(shader, view_of("u_colour"), colour);
    if (order == kApplyAfterUniform) {
        applied = cna_effect_apply(shader);
    }

    CNA_SpriteCommand command;
    memset(&command, 0, sizeof command);
    command.struct_size = (uint32_t)(sizeof command);
    command.struct_version = 1U;
    command.texture = source;
    command.destination.width = kSize;
    command.destination.height = kSize;
    command.source.width = kSize;
    command.source.height = kSize;
    command.color.r = 255U; command.color.g = 255U; command.color.b = 255U; command.color.a = 255U;
    const CNA_Result bound = cna_graphics_device_set_render_target2d(device, destination);
    CNA_Result drew = cna_sprite_batch_begin_with_effect(batch, CNA_SPRITE_SORT_MODE_IMMEDIATE,
        NULL, NULL, NULL, NULL, shader, NULL);
    if (drew == CNA_RESULT_SUCCESS) {
        drew = cna_sprite_batch_submit_many(batch, &command, 1U);
        const CNA_Result ended = cna_sprite_batch_end(batch);
        if (drew == CNA_RESULT_SUCCESS) {
            drew = ended;
        }
    }
    (void)cna_graphics_device_set_render_target2d(device, CNA_INVALID_HANDLE);

    CNA_Color out[kSize * kSize];
    memset(out, 0, sizeof out);
    CNA_Texture2DTransfer transfer;
    memset(&transfer, 0, sizeof transfer);
    transfer.struct_size = (uint32_t)(sizeof transfer);
    transfer.struct_version = 1U;
    transfer.element_count = (uint64_t)(kSize * kSize);
    uint64_t written = 0;
    const CNA_Result read = cna_texture2d_get_data(destination, CNA_TEXTURE_DATA_COLOR, &transfer,
                                                   out, (uint64_t)(kSize * kSize), &written);
    printf("  %-26s valid=%s apply=%s set=%s bind=%s draw=%s read=%s -> %u,%u,%u,%u\n", label,
           valid ? "yes" : "no", name_of(applied), name_of(set), name_of(bound), name_of(drew),
           name_of(read), out[0].r, out[0].g, out[0].b, out[0].a);

    (void)cna_sprite_batch_destroy(batch);
    (void)cna_render_target_destroy(destination);
    (void)cna_texture2d_destroy(source);
    (void)cna_effect_destroy(shader);
}

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
    one_case(device, "uniform, SpriteBatch applies", kNoApply);
    one_case(device, "uniform, then apply", kApplyAfterUniform);
    one_case(device, "apply, then uniform", kApplyBeforeUniform);
    return CNA_RESULT_SUCCESS;
}

int main(void)
{
    const char* requested = getenv("CNA_GRAPHICS_RENDERER");
    printf("renderer %s\n", requested != NULL ? requested : "<build default>");

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
    info.window_title.data = "shader effect uniform binding";
    info.window_title.byte_length = 29U;
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
    printf("game destroy %s\n", name_of(cna_game_destroy(game)));
    printf("PROBE %s\n", ran ? "OK" : "INCOMPLETE");
    return ran ? 0 : 1;
}
