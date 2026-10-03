// SPDX-License-Identifier: MIT
// Reviewed Swift-side declarations for the bound slice of the CNA C ABI.
// Every structure here mirrors a canonical CNA_* structure field for field, and
// every callback mirrors a canonical callback type. tools/native_abi verifies
// both independently against the canonical CNA headers: field names, order,
// offsets and widths, and __builtin_types_compatible_p on the callbacks.
#ifndef CNA_SWIFT_SHIM_H
#define CNA_SWIFT_SHIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t CNASwift_Result;
typedef uint8_t CNASwift_Bool;
typedef uint64_t CNASwift_Handle;
typedef uint32_t CNASwift_AudioStopOptions;
typedef uint32_t CNASwift_MicrophoneState;
typedef uint32_t CNASwift_FileMode;
typedef uint32_t CNASwift_FileAccess;
typedef uint32_t CNASwift_FileShare;
typedef uint32_t CNASwift_SeekOrigin;

typedef struct CNASwift_StringView {
    const char* data;
    uint64_t byte_length;
} CNASwift_StringView;

typedef struct CNASwift_Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} CNASwift_Color;

typedef struct CNASwift_Vector2 {
    float x;
    float y;
} CNASwift_Vector2;

typedef struct CNASwift_Rectangle {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} CNASwift_Rectangle;

typedef struct CNASwift_BackBufferReadback {
    uint32_t struct_size;
    uint32_t struct_version;
    uint8_t has_source_rectangle;
    uint8_t reserved[3];
    CNASwift_Rectangle source_rectangle;
    uint64_t start_index;
    uint64_t element_count;
} CNASwift_BackBufferReadback;

typedef struct CNASwift_GameTime {
    int64_t total_game_time_ticks;
    int64_t elapsed_game_time_ticks;
    CNASwift_Bool is_running_slowly;
    uint8_t reserved[7];
} CNASwift_GameTime;

typedef struct CNASwift_CallbackError {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_StringView message;
} CNASwift_CallbackError;

typedef CNASwift_Result (*CNASwift_GameLifecycleCallback)(
    CNASwift_Handle game,
    const CNASwift_GameTime* game_time,
    void* context,
    CNASwift_CallbackError* out_error);

typedef CNASwift_Result (*CNASwift_GameBeginDrawCallback)(
    CNASwift_Handle game,
    const CNASwift_GameTime* game_time,
    void* context,
    CNASwift_Bool* out_should_draw,
    CNASwift_CallbackError* out_error);

typedef struct CNASwift_GameCallbacks {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_GameLifecycleCallback load_content;
    CNASwift_GameLifecycleCallback update;
    CNASwift_GameLifecycleCallback draw;
    CNASwift_GameLifecycleCallback unload_content;
    CNASwift_GameLifecycleCallback exiting;
    void* context;
} CNASwift_GameCallbacks;

typedef struct CNASwift_GameFrameHooks {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_GameLifecycleCallback initialize;
    CNASwift_GameLifecycleCallback begin_run;
    CNASwift_GameLifecycleCallback end_run;
    CNASwift_GameBeginDrawCallback begin_draw;
    CNASwift_GameLifecycleCallback end_draw;
    void* context;
} CNASwift_GameFrameHooks;

typedef struct CNASwift_GameCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Bool is_fixed_time_step;
    uint8_t reserved[7];
    int64_t target_elapsed_time_ticks;
    CNASwift_StringView window_title;
    const CNASwift_GameCallbacks* callbacks;
} CNASwift_GameCreateInfo;

typedef struct CNASwift_Viewport {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    float min_depth;
    float max_depth;
} CNASwift_Viewport;

typedef struct CNASwift_Texture2DInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    uint32_t level_count;
    uint32_t format;
} CNASwift_Texture2DInfo;

typedef struct CNASwift_Texture2DCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    CNASwift_Bool mip_map;
    uint8_t reserved[3];
    uint32_t format;
} CNASwift_Texture2DCreateInfo;

typedef struct CNASwift_Texture2DTransfer {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t level;
    CNASwift_Bool has_rectangle;
    uint8_t reserved[3];
    CNASwift_Rectangle rectangle;
    uint64_t start_index;
    uint64_t element_count;
} CNASwift_Texture2DTransfer;

typedef struct CNASwift_TextureInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t level_count;
    uint32_t format;
} CNASwift_TextureInfo;

typedef struct CNASwift_RenderTarget2DCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    CNASwift_Bool mip_map;
    uint8_t reserved0[3];
    uint32_t format;
    uint32_t depth_format;
    int32_t multi_sample_count;
    uint32_t usage;
    uint32_t reserved1;
} CNASwift_RenderTarget2DCreateInfo;

typedef struct CNASwift_RenderTargetInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t kind;
    uint32_t width;
    uint32_t height;
    uint32_t level_count;
    uint32_t format;
    uint32_t depth_format;
    int32_t multi_sample_count;
    uint32_t usage;
    CNASwift_Bool is_content_lost;
    CNASwift_Bool renderer_available;
    uint8_t reserved[2];
} CNASwift_RenderTargetInfo;

typedef struct CNASwift_Vector3 {
    float x;
    float y;
    float z;
} CNASwift_Vector3;

typedef struct CNASwift_Vector4 {
    float x;
    float y;
    float z;
    float w;
} CNASwift_Vector4;

typedef struct CNASwift_Quaternion {
    float x;
    float y;
    float z;
    float w;
} CNASwift_Quaternion;

typedef struct CNASwift_Matrix {
    float m11;
    float m12;
    float m13;
    float m14;
    float m21;
    float m22;
    float m23;
    float m24;
    float m31;
    float m32;
    float m33;
    float m34;
    float m41;
    float m42;
    float m43;
    float m44;
} CNASwift_Matrix;

/* CNA_BoundingSphere. Mirrored for ModelMesh.BoundingSphere, which is the one
 * place the model family hands back a shape rather than a scalar or a handle.
 * The centre nests CNASwift_Vector3, exactly as the C struct nests
 * CNA_Vector3 and as CNASwift_AudioEmitter already does four times over. A
 * first draft here spelled the three floats out on the theory that the ABI
 * gate could not pair a nested typedef; AudioEmitter is the standing proof
 * that it can. */
typedef struct CNASwift_BoundingSphere {
    CNASwift_Vector3 center;
    float radius;
} CNASwift_BoundingSphere;

typedef struct CNASwift_UserPrimitives {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t primitive_type;
    uint32_t vertex_source;
    const void* vertex_data;
    uint64_t vertex_declaration;
    int32_t vertex_offset;
    int32_t num_vertices;
    int32_t primitive_count;
    uint32_t reserved;
} CNASwift_UserPrimitives;

typedef struct CNASwift_UserIndices {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t index_element_size;
    int32_t index_offset;
    const void* index_data;
} CNASwift_UserIndices;

typedef struct CNASwift_EffectParameterInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t row_count;
    int32_t column_count;
    uint32_t parameter_class;
    uint32_t parameter_type;
} CNASwift_EffectParameterInfo;

typedef struct CNASwift_EffectAnnotationInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t row_count;
    int32_t column_count;
    uint32_t parameter_class;
    uint32_t parameter_type;
} CNASwift_EffectAnnotationInfo;

typedef struct CNASwift_TextureSlotInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Bool bound;
    uint8_t reserved[7];
    uint64_t texture;
} CNASwift_TextureSlotInfo;

typedef struct CNASwift_RenderTargetCubeCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t size;
    CNASwift_Bool mip_map;
    uint8_t reserved[3];
    uint32_t format;
    uint32_t depth_format;
    int32_t multi_sample_count;
    uint32_t usage;
} CNASwift_RenderTargetCubeCreateInfo;

typedef struct CNASwift_RenderTargetBinding {
    uint32_t struct_size;
    uint32_t struct_version;
    uint64_t render_target;
    int32_t array_slice;
    uint32_t cube_map_face;
} CNASwift_RenderTargetBinding;

typedef struct CNASwift_BlendState {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t alpha_blend_function;
    uint32_t alpha_destination_blend;
    uint32_t alpha_source_blend;
    uint32_t color_blend_function;
    uint32_t color_destination_blend;
    uint32_t color_source_blend;
    uint32_t color_write_channels;
    uint32_t color_write_channels1;
    uint32_t color_write_channels2;
    uint32_t color_write_channels3;
    CNASwift_Color blend_factor;
    int32_t multi_sample_mask;
} CNASwift_BlendState;

typedef struct CNASwift_DepthStencilState {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Bool depth_buffer_enable;
    CNASwift_Bool depth_buffer_write_enable;
    CNASwift_Bool stencil_enable;
    CNASwift_Bool two_sided_stencil_mode;
    uint32_t depth_buffer_function;
    uint32_t stencil_function;
    int32_t stencil_mask;
    int32_t stencil_write_mask;
    int32_t reference_stencil;
    uint32_t stencil_fail;
    uint32_t stencil_depth_buffer_fail;
    uint32_t stencil_pass;
    uint32_t counter_clockwise_stencil_function;
    uint32_t counter_clockwise_stencil_fail;
    uint32_t counter_clockwise_stencil_depth_buffer_fail;
    uint32_t counter_clockwise_stencil_pass;
    uint32_t reserved;
} CNASwift_DepthStencilState;

typedef struct CNASwift_RasterizerState {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t cull_mode;
    uint32_t fill_mode;
    float depth_bias;
    float slope_scale_depth_bias;
    CNASwift_Bool multi_sample_anti_alias;
    CNASwift_Bool scissor_test_enable;
    uint8_t reserved[2];
} CNASwift_RasterizerState;

typedef struct CNASwift_SamplerState {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    uint32_t filter;
    int32_t max_anisotropy;
    int32_t max_mip_level;
    float mip_map_level_of_detail_bias;
    uint32_t reserved;
} CNASwift_SamplerState;

typedef struct CNASwift_PresentationParameters {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t back_buffer_format;
    int32_t back_buffer_width;
    int32_t back_buffer_height;
    uint32_t depth_stencil_format;
    int32_t multi_sample_count;
    uint32_t presentation_interval;
    uint32_t display_orientation;
    uint32_t render_target_usage;
    CNASwift_Bool is_full_screen;
    CNASwift_Bool headless_ext;
    uint8_t reserved[2];
} CNASwift_PresentationParameters;

typedef void (*CNASwift_GameEventCallback)(void* context);

typedef void (*CNASwift_RenderTargetContentLostCallback)(
    CNASwift_Handle render_target,
    void* context);

typedef CNASwift_Handle CNASwift_VertexBufferHandle;
typedef CNASwift_Handle CNASwift_IndexBufferHandle;

typedef void (*CNASwift_VertexBufferContentLostCallback)(
    CNASwift_VertexBufferHandle vertex_buffer,
    void* context);

typedef void (*CNASwift_IndexBufferContentLostCallback)(
    CNASwift_IndexBufferHandle index_buffer,
    void* context);

typedef struct CNASwift_Texture2DDecodeInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    CNASwift_Bool zoom;
    uint8_t reserved[7];
} CNASwift_Texture2DDecodeInfo;

typedef struct CNASwift_VertexElement {
    int32_t offset;
    uint32_t format;
    uint32_t usage;
    int32_t usage_index;
} CNASwift_VertexElement;

/* CNA_VertexBufferInfo and CNA_IndexBufferInfo. Mirrored at Foundation 103 so
 * ModelMeshPart can adopt the buffers a model already owns: the model routes
 * hand back a bare handle, and a VertexBuffer facade needs the count, the
 * usage and the declaration stride that only these structures carry. */
typedef struct CNASwift_VertexBufferInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t vertex_count;
    uint32_t buffer_usage;
    uint8_t dynamic;
    uint8_t is_content_lost;
    uint8_t has_renderer;
    uint8_t reserved0;
    int32_t vertex_stride;
    uint64_t vertex_element_count;
} CNASwift_VertexBufferInfo;

typedef struct CNASwift_IndexBufferInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t index_count;
    uint32_t index_element_size;
    uint32_t buffer_usage;
    uint8_t dynamic;
    uint8_t is_content_lost;
    uint8_t has_renderer;
    uint8_t reserved;
} CNASwift_IndexBufferInfo;

typedef struct CNASwift_VertexBufferCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Handle vertex_declaration;
    int32_t vertex_count;
    uint32_t buffer_usage;
    CNASwift_Bool dynamic;
    uint8_t reserved[7];
} CNASwift_VertexBufferCreateInfo;

typedef struct CNASwift_TextureCubeCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t size;
    CNASwift_Bool mip_map;
    uint8_t reserved0[3];
    uint32_t format;
    uint32_t reserved1;
} CNASwift_TextureCubeCreateInfo;

typedef struct CNASwift_TextureCubeInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t size;
    uint32_t level_count;
    uint32_t format;
    uint32_t reserved;
} CNASwift_TextureCubeInfo;

typedef struct CNASwift_TextureCubeTransfer {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t face;
    int32_t level;
    CNASwift_Bool has_rectangle;
    uint8_t reserved0[3];
    CNASwift_Rectangle rectangle;
    uint32_t reserved1;
    uint64_t start_index;
    uint64_t element_count;
} CNASwift_TextureCubeTransfer;

typedef struct CNASwift_Texture3DCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    CNASwift_Bool mip_map;
    uint8_t reserved0[3];
    uint32_t format;
    uint32_t reserved1;
} CNASwift_Texture3DCreateInfo;

typedef struct CNASwift_Texture3DInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t level_count;
    uint32_t format;
    uint32_t reserved;
} CNASwift_Texture3DInfo;

typedef struct CNASwift_Texture3DTransfer {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t level;
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    int32_t front;
    int32_t back;
    uint32_t reserved;
    uint64_t start_index;
    uint64_t element_count;
} CNASwift_Texture3DTransfer;

typedef struct CNASwift_VertexBufferBinding {
    CNASwift_VertexBufferHandle vertex_buffer;
    int32_t vertex_offset;
    int32_t instance_frequency;
} CNASwift_VertexBufferBinding;

typedef struct CNASwift_IndexBufferCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t index_count;
    uint32_t index_element_size;
    uint32_t buffer_usage;
    CNASwift_Bool dynamic;
    uint8_t reserved[3];
} CNASwift_IndexBufferCreateInfo;

typedef struct CNASwift_IndexBufferTransfer {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t index_element_size;
    uint32_t options;
    uint64_t start_index;
    uint64_t element_count;
} CNASwift_IndexBufferTransfer;

typedef struct CNASwift_SpriteCommand {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Handle texture;
    CNASwift_Rectangle destination;
    CNASwift_Rectangle source;
    CNASwift_Color color;
    float rotation;
    CNASwift_Vector2 origin;
    uint32_t effects;
    float layer_depth;
} CNASwift_SpriteCommand;

typedef struct CNASwift_SpriteScaledCommand {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Handle texture;
    CNASwift_Vector2 position;
    CNASwift_Rectangle source;
    CNASwift_Color color;
    float rotation;
    CNASwift_Vector2 origin;
    CNASwift_Vector2 scale;
    uint32_t effects;
    float layer_depth;
} CNASwift_SpriteScaledCommand;

typedef struct CNASwift_TouchCapabilities {
    uint32_t struct_size;
    uint32_t struct_version;
    uint8_t is_connected;
    uint8_t reserved[3];
    uint32_t maximum_touch_count;
} CNASwift_TouchCapabilities;

typedef struct CNASwift_TouchLocation {
    int32_t id;
    uint32_t state;
    CNASwift_Vector2 position;
    uint32_t previous_state;
    CNASwift_Vector2 previous_position;
    float pressure;
} CNASwift_TouchLocation;

typedef struct CNASwift_TouchState {
    uint32_t struct_size;
    uint32_t struct_version;
    uint8_t is_connected;
    uint8_t reserved[3];
    uint32_t touch_count;
    CNASwift_TouchLocation touches[8];
} CNASwift_TouchState;

typedef struct CNASwift_GestureSample {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t gesture_type;
    int32_t finger_id_ext;
    int32_t finger_id2_ext;
    uint32_t reserved;
    int64_t timestamp_ticks;
    CNASwift_Vector2 position;
    CNASwift_Vector2 position2;
    CNASwift_Vector2 delta;
    CNASwift_Vector2 delta2;
} CNASwift_GestureSample;

typedef void (*CNASwift_MediaPlayerEventCallback)(void* context);

typedef struct CNASwift_VisualizationData {
    uint32_t struct_size;
    uint32_t struct_version;
    float frequencies[256];
    float samples[256];
} CNASwift_VisualizationData;

typedef void (*CNASwift_AudioEventCallback)(void* context);

typedef struct CNASwift_CueInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint8_t is_created;
    uint8_t is_disposed;
    uint8_t is_paused;
    uint8_t is_playing;
    uint8_t is_prepared;
    uint8_t is_preparing;
    uint8_t is_stopped;
    uint8_t is_stopping;
} CNASwift_CueInfo;

/* CNA_StorageCompletionCallback. The same `void (*)(void*)` shape as the audio
 * one, and mirrored separately rather than reused: the ABI gate pairs a shim
 * typedef with a C prototype by NAME, so sharing one would leave the storage
 * routes' callback parameter unchecked. */
typedef void (*CNASwift_StorageCompletionCallback)(void* context);

typedef struct CNASwift_AudioEmitter {
    uint32_t struct_size;
    uint32_t struct_version;
    float doppler_scale;
    CNASwift_Vector3 forward;
    CNASwift_Vector3 position;
    CNASwift_Vector3 up;
    CNASwift_Vector3 velocity;
} CNASwift_AudioEmitter;

typedef struct CNASwift_AudioListener {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Vector3 forward;
    CNASwift_Vector3 position;
    CNASwift_Vector3 up;
    CNASwift_Vector3 velocity;
} CNASwift_AudioListener;

typedef struct CNASwift_SoundEffectCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t sample_rate;
    uint32_t channels;
    uint64_t reserved;
} CNASwift_SoundEffectCreateInfo;

typedef struct CNASwift_SoundEffectInstanceInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t state;
    uint8_t is_looped;
    uint8_t reserved0[3];
    float volume;
    float pitch;
    float pan;
    uint32_t reserved1;
} CNASwift_SoundEffectInstanceInfo;

typedef struct CNASwift_ContentManagerCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_StringView root_directory;
    /* Reserved; CNA rejects the whole structure unless this is zero, and it
       is part of sizeof, so omitting it also made struct_size too small. */
    uint64_t reserved;
} CNASwift_ContentManagerCreateInfo;

typedef struct CNASwift_DisplayMode {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t width;
    int32_t height;
    float aspect_ratio;
    uint32_t format;
} CNASwift_DisplayMode;

typedef struct CNASwift_GraphicsAdapterInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t adapter_index;
    uint8_t is_default_adapter;
    uint8_t is_wide_screen;
    uint8_t use_null_device;
    uint8_t use_reference_device;
    int32_t vendor_id;
    int32_t device_id;
    int32_t revision;
    int32_t subsystem_id;
    uint64_t description_byte_length;
    uint64_t device_name_byte_length;
} CNASwift_GraphicsAdapterInfo;

typedef struct CNASwift_GraphicsFormatSelection {
    uint32_t struct_size;
    uint32_t struct_version;
    uint8_t exact_match;
    uint8_t reserved[3];
    uint32_t format;
    uint32_t depth_format;
    int32_t multi_sample_count;
} CNASwift_GraphicsFormatSelection;

typedef struct CNASwift_MouseState {
    uint32_t struct_size;
    uint32_t struct_version;
    int32_t x;
    int32_t y;
    int32_t scroll_wheel;
    int32_t horizontal_scroll_wheel;
    uint32_t pressed_buttons;
    uint32_t reserved;
} CNASwift_MouseState;

typedef struct CNASwift_KeyboardState {
    uint32_t struct_size;
    uint32_t struct_version;
    uint64_t pressed_key_words[4];
} CNASwift_KeyboardState;

typedef struct CNASwift_GamePadAnalogState {
    CNASwift_Vector2 left_thumb_stick;
    CNASwift_Vector2 right_thumb_stick;
    float left_trigger;
    float right_trigger;
} CNASwift_GamePadAnalogState;

typedef struct CNASwift_GamePadState {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Bool is_connected;
    uint8_t reserved0[3];
    int32_t packet_number;
    uint32_t pressed_buttons;
    uint32_t reserved1;
    CNASwift_GamePadAnalogState analog;
} CNASwift_GamePadState;

typedef struct CNASwift_GamePadCapabilities {
    uint32_t struct_size;
    uint32_t struct_version;
    uint32_t gamepad_type;
    CNASwift_Bool is_connected;
    CNASwift_Bool has_a_button;
    CNASwift_Bool has_b_button;
    CNASwift_Bool has_x_button;
    CNASwift_Bool has_y_button;
    CNASwift_Bool has_back_button;
    CNASwift_Bool has_start_button;
    CNASwift_Bool has_big_button;
    CNASwift_Bool has_dpad_up_button;
    CNASwift_Bool has_dpad_down_button;
    CNASwift_Bool has_dpad_left_button;
    CNASwift_Bool has_dpad_right_button;
    CNASwift_Bool has_left_shoulder_button;
    CNASwift_Bool has_right_shoulder_button;
    CNASwift_Bool has_left_stick_button;
    CNASwift_Bool has_right_stick_button;
    CNASwift_Bool has_left_x_thumb_stick;
    CNASwift_Bool has_left_y_thumb_stick;
    CNASwift_Bool has_right_x_thumb_stick;
    CNASwift_Bool has_right_y_thumb_stick;
    CNASwift_Bool has_left_trigger;
    CNASwift_Bool has_right_trigger;
    CNASwift_Bool has_left_vibration_motor;
    CNASwift_Bool has_right_vibration_motor;
    CNASwift_Bool has_voice_support;
    CNASwift_Bool has_light_bar_ext;
    CNASwift_Bool has_trigger_vibration_motors_ext;
    CNASwift_Bool has_misc1_ext;
    CNASwift_Bool has_paddle1_ext;
    CNASwift_Bool has_paddle2_ext;
    CNASwift_Bool has_paddle3_ext;
    CNASwift_Bool has_paddle4_ext;
    CNASwift_Bool has_touchpad_ext;
    CNASwift_Bool has_gyro_ext;
    CNASwift_Bool has_accelerometer_ext;
    uint8_t reserved[1];
} CNASwift_GamePadCapabilities;

typedef struct CNASwift_SpriteFontGlyph {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Rectangle glyph_bounds;
    CNASwift_Rectangle cropping;
    uint16_t character;
    uint16_t reserved;
    CNASwift_Vector3 kerning;
} CNASwift_SpriteFontGlyph;

typedef struct CNASwift_SpriteFontCreateInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Handle texture;
    const CNASwift_SpriteFontGlyph* glyphs;
    uint64_t glyph_count;
    int32_t line_spacing;
    float spacing;
    uint16_t default_character;
    CNASwift_Bool has_default_character;
    uint8_t reserved[5];
} CNASwift_SpriteFontCreateInfo;

typedef struct CNASwift_SpriteFontInfo {
    uint32_t struct_size;
    uint32_t struct_version;
    uint64_t character_count;
    int32_t line_spacing;
    float spacing;
    uint16_t default_character;
    CNASwift_Bool has_default_character;
    uint8_t reserved[5];
} CNASwift_SpriteFontInfo;

typedef struct CNASwift_SpriteTextCommand {
    uint32_t struct_size;
    uint32_t struct_version;
    CNASwift_Handle sprite_font;
    CNASwift_StringView text;
    CNASwift_Vector2 position;
    CNASwift_Color color;
    float rotation;
    CNASwift_Vector2 origin;
    CNASwift_Vector2 scale;
    uint32_t effects;
    float layer_depth;
} CNASwift_SpriteTextCommand;

#ifdef __cplusplus
}
#endif

#endif
