// SPDX-License-Identifier: MS-PL

#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>

_Static_assert(sizeof(CNA_Result) == 4U, "CNA_Result must be uint32_t");
_Static_assert(sizeof(CNA_Bool) == 1U, "CNA_Bool must be uint8_t");
_Static_assert(sizeof(CNA_Handle) == 8U, "CNA_Handle must be uint64_t");
_Static_assert(sizeof(CNA_StorageDeviceHandle) == 8U,
    "CNA_StorageDeviceHandle must be uint64_t");
_Static_assert(sizeof(CNA_StorageContainerHandle) == 8U,
    "CNA_StorageContainerHandle must be uint64_t");
_Static_assert(sizeof(CNA_StorageStreamHandle) == 8U,
    "CNA_StorageStreamHandle must be uint64_t");
_Static_assert(sizeof(CNA_FileMode) == 4U, "CNA_FileMode must be uint32_t");
_Static_assert(sizeof(CNA_FileAccess) == 4U, "CNA_FileAccess must be uint32_t");
_Static_assert(sizeof(CNA_FileShare) == 4U, "CNA_FileShare must be uint32_t");
_Static_assert(sizeof(CNA_SeekOrigin) == 4U, "CNA_SeekOrigin must be uint32_t");
_Static_assert(sizeof(CNA_MediaState) == 4U, "CNA_MediaState must be uint32_t");
_Static_assert(sizeof(CNA_MediaSourceType) == 4U, "CNA_MediaSourceType must be uint32_t");
_Static_assert(sizeof(CNA_VideoSoundtrackType) == 4U,
    "CNA_VideoSoundtrackType must be uint32_t");
_Static_assert(sizeof(CNA_VisualizationData) == 2056U,
    "CNA_VisualizationData layout changed");
_Static_assert(offsetof(CNA_VisualizationData, frequencies) == 8U,
    "CNA_VisualizationData.frequencies offset");
_Static_assert(offsetof(CNA_VisualizationData, samples) == 1032U,
    "CNA_VisualizationData.samples offset");
_Static_assert(sizeof(CNA_VideoInfo) == 24U, "CNA_VideoInfo layout changed");
_Static_assert(offsetof(CNA_VideoInfo, width) == 8U, "CNA_VideoInfo.width offset");
_Static_assert(offsetof(CNA_VideoInfo, fps) == 16U, "CNA_VideoInfo.fps offset");
_Static_assert(sizeof(CNA_Color) == 4U, "CNA_Color must be four bytes");
_Static_assert(sizeof(CNA_DisplayOrientation) == 4U, "CNA_DisplayOrientation must be uint32_t");
_Static_assert(sizeof(CNA_Rectangle) == 16U, "CNA_Rectangle must be four int32 values");
_Static_assert(offsetof(CNA_Rectangle, x) == 0U, "CNA_Rectangle.x offset");
_Static_assert(offsetof(CNA_Rectangle, y) == 4U, "CNA_Rectangle.y offset");
_Static_assert(offsetof(CNA_Rectangle, width) == 8U, "CNA_Rectangle.width offset");
_Static_assert(offsetof(CNA_Rectangle, height) == 12U, "CNA_Rectangle.height offset");
_Static_assert(sizeof(CNA_KeyboardState) == 40U, "CNA_KeyboardState layout changed");
_Static_assert(offsetof(CNA_KeyboardState, struct_size) == 0U, "CNA_KeyboardState.size offset");
_Static_assert(offsetof(CNA_KeyboardState, struct_version) == 4U, "CNA_KeyboardState.version offset");
_Static_assert(offsetof(CNA_KeyboardState, pressed_key_words) == 8U, "CNA_KeyboardState.words offset");
_Static_assert(sizeof(CNA_MouseState) == 32U, "CNA_MouseState layout changed");
_Static_assert(offsetof(CNA_MouseState, x) == 8U, "CNA_MouseState.x offset");
_Static_assert(offsetof(CNA_MouseState, y) == 12U, "CNA_MouseState.y offset");
_Static_assert(offsetof(CNA_MouseState, scroll_wheel) == 16U, "CNA_MouseState.wheel offset");
_Static_assert(offsetof(CNA_MouseState, pressed_buttons) == 24U, "CNA_MouseState.buttons offset");
_Static_assert(sizeof(CNA_GamePadState) == 48U, "CNA_GamePadState layout changed");
_Static_assert(offsetof(CNA_GamePadState, is_connected) == 8U, "CNA_GamePadState.connected offset");
_Static_assert(offsetof(CNA_GamePadState, packet_number) == 12U, "CNA_GamePadState.packet offset");
_Static_assert(offsetof(CNA_GamePadState, pressed_buttons) == 16U, "CNA_GamePadState.buttons offset");
_Static_assert(offsetof(CNA_GamePadState, analog) == 24U, "CNA_GamePadState.analog offset");
_Static_assert(sizeof(CNA_GamePadCapabilities) == 48U, "CNA_GamePadCapabilities layout changed");

/*
 * The sensor readings. Their field order is what the generated adapter flattens into the
 * Java carrier arrays, so a reordering upstream would silently change what each index
 * means; asserting the offsets is what turns that into a build failure.
 */
_Static_assert(sizeof(CNA_SensorState) == 4U, "CNA_SensorState must be uint32_t");
_Static_assert(sizeof(CNA_DateTimeOffset) == 16U, "CNA_DateTimeOffset layout changed");
_Static_assert(offsetof(CNA_DateTimeOffset, offset_ticks) == 8U,
    "CNA_DateTimeOffset.offset_ticks offset");
_Static_assert(sizeof(CNA_AccelerometerReading) == 40U,
    "CNA_AccelerometerReading layout changed");
_Static_assert(offsetof(CNA_AccelerometerReading, timestamp) == 8U,
    "CNA_AccelerometerReading.timestamp offset");
_Static_assert(offsetof(CNA_AccelerometerReading, acceleration) == 24U,
    "CNA_AccelerometerReading.acceleration offset");
_Static_assert(sizeof(CNA_GyroscopeReading) == 40U, "CNA_GyroscopeReading layout changed");
_Static_assert(offsetof(CNA_GyroscopeReading, timestamp) == 8U,
    "CNA_GyroscopeReading.timestamp offset");
_Static_assert(offsetof(CNA_GyroscopeReading, rotation_rate) == 24U,
    "CNA_GyroscopeReading.rotation_rate offset");
_Static_assert(sizeof(CNA_AttitudeReading) == 120U, "CNA_AttitudeReading layout changed");
_Static_assert(offsetof(CNA_AttitudeReading, timestamp) == 8U,
    "CNA_AttitudeReading.timestamp offset");
_Static_assert(offsetof(CNA_AttitudeReading, pitch) == 24U,
    "CNA_AttitudeReading.pitch offset");
_Static_assert(offsetof(CNA_AttitudeReading, quaternion) == 36U,
    "CNA_AttitudeReading.quaternion offset");
_Static_assert(offsetof(CNA_AttitudeReading, rotation_matrix) == 52U,
    "CNA_AttitudeReading.rotation_matrix offset");
_Static_assert(sizeof(CNA_CompassReading) == 64U, "CNA_CompassReading layout changed");
_Static_assert(offsetof(CNA_CompassReading, timestamp) == 8U,
    "CNA_CompassReading.timestamp offset");
_Static_assert(offsetof(CNA_CompassReading, heading_accuracy) == 24U,
    "CNA_CompassReading.heading_accuracy offset");
_Static_assert(offsetof(CNA_CompassReading, magnetometer_reading) == 48U,
    "CNA_CompassReading.magnetometer_reading offset");
_Static_assert(sizeof(CNA_MotionReading) == 184U, "CNA_MotionReading layout changed");
_Static_assert(offsetof(CNA_MotionReading, timestamp) == 8U,
    "CNA_MotionReading.timestamp offset");
_Static_assert(offsetof(CNA_MotionReading, attitude) == 24U,
    "CNA_MotionReading.attitude offset");
_Static_assert(offsetof(CNA_MotionReading, device_acceleration) == 144U,
    "CNA_MotionReading.device_acceleration offset");
_Static_assert(offsetof(CNA_MotionReading, gravity) == 168U,
    "CNA_MotionReading.gravity offset");

/*
 * The device-enumeration and raw-joystick values, for the same reason: field order is what the
 * generated adapter flattens into the Java carrier arrays.
 */
_Static_assert(sizeof(CNA_SensorType) == 4U, "CNA_SensorType must be uint32_t");
_Static_assert(sizeof(CNA_JoystickType) == 4U, "CNA_JoystickType must be uint32_t");
_Static_assert(sizeof(CNA_JoystickHatPosition) == 4U,
    "CNA_JoystickHatPosition must be uint32_t");
_Static_assert(sizeof(CNA_PowerState) == 4U, "CNA_PowerState must be uint32_t");
_Static_assert(sizeof(CNA_Point) == 8U, "CNA_Point layout changed");
_Static_assert(offsetof(CNA_Point, y) == 4U, "CNA_Point.y offset");
_Static_assert(sizeof(CNA_SensorInfo) == 16U, "CNA_SensorInfo layout changed");
_Static_assert(offsetof(CNA_SensorInfo, id) == 8U, "CNA_SensorInfo.id offset");
_Static_assert(offsetof(CNA_SensorInfo, type) == 12U, "CNA_SensorInfo.type offset");
_Static_assert(sizeof(CNA_InputDeviceInfo) == 16U, "CNA_InputDeviceInfo layout changed");
_Static_assert(offsetof(CNA_InputDeviceInfo, id) == 8U, "CNA_InputDeviceInfo.id offset");
_Static_assert(sizeof(CNA_JoystickInfo) == 16U, "CNA_JoystickInfo layout changed");
_Static_assert(offsetof(CNA_JoystickInfo, id) == 8U, "CNA_JoystickInfo.id offset");
_Static_assert(offsetof(CNA_JoystickInfo, type) == 12U, "CNA_JoystickInfo.type offset");
_Static_assert(sizeof(CNA_JoystickCapabilities) == 40U,
    "CNA_JoystickCapabilities layout changed");
_Static_assert(offsetof(CNA_JoystickCapabilities, axis_count) == 8U,
    "CNA_JoystickCapabilities.axis_count offset");
_Static_assert(offsetof(CNA_JoystickCapabilities, type) == 24U,
    "CNA_JoystickCapabilities.type offset");
_Static_assert(offsetof(CNA_JoystickCapabilities, power_state) == 28U,
    "CNA_JoystickCapabilities.power_state offset");
_Static_assert(offsetof(CNA_JoystickCapabilities, power_percent) == 32U,
    "CNA_JoystickCapabilities.power_percent offset");
_Static_assert(offsetof(CNA_JoystickCapabilities, is_connected) == 36U,
    "CNA_JoystickCapabilities.is_connected offset");

/*
 * The haptic values. CNA_HapticEffect is the widest flattened structure this binding sends, and
 * the Java side names its 44 leaves by index, so a reordering upstream would change the meaning
 * of every one of them.
 */
_Static_assert(sizeof(CNA_HapticFeature) == 4U, "CNA_HapticFeature must be uint32_t");
_Static_assert(sizeof(CNA_HapticEffectType) == 4U, "CNA_HapticEffectType must be uint32_t");
_Static_assert(sizeof(CNA_HapticDirectionType) == 4U,
    "CNA_HapticDirectionType must be uint32_t");
_Static_assert(sizeof(CNA_HapticDirection) == 16U, "CNA_HapticDirection layout changed");
_Static_assert(offsetof(CNA_HapticDirection, values) == 4U,
    "CNA_HapticDirection.values offset");
_Static_assert(sizeof(CNA_HapticCapabilities) == 28U, "CNA_HapticCapabilities layout changed");
_Static_assert(offsetof(CNA_HapticCapabilities, features) == 8U,
    "CNA_HapticCapabilities.features offset");
_Static_assert(offsetof(CNA_HapticCapabilities, axis_count) == 12U,
    "CNA_HapticCapabilities.axis_count offset");
_Static_assert(offsetof(CNA_HapticCapabilities, is_open) == 24U,
    "CNA_HapticCapabilities.is_open offset");
_Static_assert(sizeof(CNA_HapticEffect) == 108U, "CNA_HapticEffect layout changed");
_Static_assert(offsetof(CNA_HapticEffect, type) == 8U, "CNA_HapticEffect.type offset");
_Static_assert(offsetof(CNA_HapticEffect, direction) == 16U,
    "CNA_HapticEffect.direction offset");
_Static_assert(offsetof(CNA_HapticEffect, length) == 32U, "CNA_HapticEffect.length offset");
_Static_assert(offsetof(CNA_HapticEffect, level) == 42U, "CNA_HapticEffect.level offset");
_Static_assert(offsetof(CNA_HapticEffect, right_saturation) == 56U,
    "CNA_HapticEffect.right_saturation offset");
_Static_assert(offsetof(CNA_HapticEffect, center) == 86U, "CNA_HapticEffect.center offset");
_Static_assert(offsetof(CNA_HapticEffect, large_magnitude) == 92U,
    "CNA_HapticEffect.large_magnitude offset");
_Static_assert(offsetof(CNA_HapticEffect, custom_channels) == 98U,
    "CNA_HapticEffect.custom_channels offset");
_Static_assert(offsetof(CNA_HapticEffect, attack_length) == 100U,
    "CNA_HapticEffect.attack_length offset");
_Static_assert(offsetof(CNA_HapticEffect, fade_level) == 106U,
    "CNA_HapticEffect.fade_level offset");
_Static_assert(offsetof(CNA_GamePadCapabilities, gamepad_type) == 8U,
    "CNA_GamePadCapabilities.type offset");
_Static_assert(offsetof(CNA_GamePadCapabilities, is_connected) == 12U,
    "CNA_GamePadCapabilities.connected offset");
_Static_assert(sizeof(CNA_PresentationParameters) == 44U,
    "CNA_PresentationParameters layout changed");
_Static_assert(offsetof(CNA_PresentationParameters, back_buffer_format) == 8U,
    "CNA_PresentationParameters.format offset");
_Static_assert(offsetof(CNA_PresentationParameters, headless_ext) == 41U,
    "CNA_PresentationParameters.headless offset");
_Static_assert(sizeof(CNA_GraphicsDeviceInformation) == 60U,
    "CNA_GraphicsDeviceInformation layout changed");
_Static_assert(offsetof(CNA_GraphicsDeviceInformation, adapter_index) == 8U,
    "CNA_GraphicsDeviceInformation.adapter offset");
_Static_assert(offsetof(CNA_GraphicsDeviceInformation, presentation_parameters) == 16U,
    "CNA_GraphicsDeviceInformation.parameters offset");
_Static_assert(sizeof(CNA_DisplayMode) == 24U, "CNA_DisplayMode layout changed");
_Static_assert(offsetof(CNA_DisplayMode, aspect_ratio) == 16U,
    "CNA_DisplayMode.aspect offset");
_Static_assert(sizeof(CNA_GraphicsAdapterInfo) == 48U,
    "CNA_GraphicsAdapterInfo layout changed");
_Static_assert(offsetof(CNA_GraphicsAdapterInfo, vendor_id) == 16U,
    "CNA_GraphicsAdapterInfo.vendor offset");
_Static_assert(offsetof(CNA_GraphicsAdapterInfo, description_byte_length) == 32U,
    "CNA_GraphicsAdapterInfo.description length offset");
_Static_assert(sizeof(CNA_GraphicsFormatSelection) == 24U,
    "CNA_GraphicsFormatSelection layout changed");
_Static_assert(offsetof(CNA_GraphicsFormatSelection, format) == 12U,
    "CNA_GraphicsFormatSelection.format offset");
_Static_assert(sizeof(CNA_Viewport) == 24U, "CNA_Viewport layout changed");
_Static_assert(offsetof(CNA_Viewport, min_depth) == 16U, "CNA_Viewport.min depth offset");
_Static_assert(sizeof(CNA_BlendState) == 56U, "CNA_BlendState layout changed");
_Static_assert(offsetof(CNA_BlendState, alpha_blend_function) == 8U,
    "CNA_BlendState.alpha function offset");
_Static_assert(offsetof(CNA_BlendState, blend_factor) == 48U,
    "CNA_BlendState.blend factor offset");
_Static_assert(sizeof(CNA_DepthStencilState) == 64U,
    "CNA_DepthStencilState layout changed");
_Static_assert(offsetof(CNA_DepthStencilState, depth_buffer_enable) == 8U,
    "CNA_DepthStencilState.depth enable offset");
_Static_assert(offsetof(CNA_DepthStencilState, counter_clockwise_stencil_pass) == 56U,
    "CNA_DepthStencilState.counter-clockwise pass offset");
_Static_assert(sizeof(CNA_RasterizerState) == 28U, "CNA_RasterizerState layout changed");
_Static_assert(offsetof(CNA_RasterizerState, depth_bias) == 16U,
    "CNA_RasterizerState.depth bias offset");
_Static_assert(offsetof(CNA_RasterizerState, multi_sample_anti_alias) == 24U,
    "CNA_RasterizerState.multisample offset");
_Static_assert(sizeof(CNA_SamplerState) == 40U, "CNA_SamplerState layout changed");
_Static_assert(offsetof(CNA_SamplerState, address_u) == 8U,
    "CNA_SamplerState.address offset");
_Static_assert(offsetof(CNA_SamplerState, mip_map_level_of_detail_bias) == 32U,
    "CNA_SamplerState.bias offset");
_Static_assert(sizeof(CNA_TextureSlotInfo) == 24U, "CNA_TextureSlotInfo layout changed");
_Static_assert(offsetof(CNA_TextureSlotInfo, bound) == 8U,
    "CNA_TextureSlotInfo.bound offset");
_Static_assert(offsetof(CNA_TextureSlotInfo, texture) == 16U,
    "CNA_TextureSlotInfo.texture offset");
_Static_assert(sizeof(CNA_BackBufferInfo) == 24U, "CNA_BackBufferInfo layout changed");
_Static_assert(offsetof(CNA_BackBufferInfo, width) == 8U,
    "CNA_BackBufferInfo.width offset");
_Static_assert(offsetof(CNA_BackBufferInfo, format) == 16U,
    "CNA_BackBufferInfo.format offset");
_Static_assert(sizeof(CNA_BackBufferReadback) == 48U,
    "CNA_BackBufferReadback layout changed");
_Static_assert(offsetof(CNA_BackBufferReadback, has_source_rectangle) == 8U,
    "CNA_BackBufferReadback.rectangle flag offset");
_Static_assert(offsetof(CNA_BackBufferReadback, source_rectangle) == 12U,
    "CNA_BackBufferReadback.rectangle offset");
_Static_assert(offsetof(CNA_BackBufferReadback, start_index) == 32U,
    "CNA_BackBufferReadback.start offset");
_Static_assert(offsetof(CNA_BackBufferReadback, element_count) == 40U,
    "CNA_BackBufferReadback.count offset");
_Static_assert(sizeof(CNA_ResourceCreatedEventInfo) == 16U,
    "CNA_ResourceCreatedEventInfo layout changed");
_Static_assert(sizeof(CNA_ResourceDestroyedEventInfo) == 32U,
    "CNA_ResourceDestroyedEventInfo layout changed");
_Static_assert(offsetof(CNA_ResourceDestroyedEventInfo, name) == 16U,
    "CNA_ResourceDestroyedEventInfo.name offset");
_Static_assert(sizeof(CNA_Texture2DCreateInfo) == 24U, "CNA_Texture2DCreateInfo layout changed");
_Static_assert(offsetof(CNA_Texture2DCreateInfo, width) == 8U, "CNA_Texture2DCreateInfo.width offset");
_Static_assert(offsetof(CNA_Texture2DCreateInfo, format) == 20U, "CNA_Texture2DCreateInfo.format offset");
_Static_assert(sizeof(CNA_Texture2DInfo) == 24U, "CNA_Texture2DInfo layout changed");
_Static_assert(sizeof(CNA_Texture2DTransfer) == 48U, "CNA_Texture2DTransfer layout changed");
_Static_assert(offsetof(CNA_Texture2DTransfer, level) == 8U,
    "CNA_Texture2DTransfer.level offset");
_Static_assert(offsetof(CNA_Texture2DTransfer, rectangle) == 16U,
    "CNA_Texture2DTransfer.rectangle offset");
_Static_assert(offsetof(CNA_Texture2DTransfer, start_index) == 32U,
    "CNA_Texture2DTransfer.start offset");
_Static_assert(offsetof(CNA_Texture2DTransfer, element_count) == 40U,
    "CNA_Texture2DTransfer.count offset");
_Static_assert(sizeof(CNA_Texture2DDecodeInfo) == 24U, "CNA_Texture2DDecodeInfo layout changed");
_Static_assert(sizeof(CNA_TextureCubeCreateInfo) == 24U,
    "CNA_TextureCubeCreateInfo layout changed");
_Static_assert(sizeof(CNA_TextureCubeInfo) == 24U,
    "CNA_TextureCubeInfo layout changed");
_Static_assert(sizeof(CNA_TextureCubeTransfer) == 56U,
    "CNA_TextureCubeTransfer layout changed");
_Static_assert(offsetof(CNA_TextureCubeTransfer, rectangle) == 20U,
    "CNA_TextureCubeTransfer.rectangle offset");
_Static_assert(offsetof(CNA_TextureCubeTransfer, start_index) == 40U,
    "CNA_TextureCubeTransfer.start offset");
_Static_assert(sizeof(CNA_RenderTarget2DCreateInfo) == 40U,
    "CNA_RenderTarget2DCreateInfo layout changed");
_Static_assert(offsetof(CNA_RenderTarget2DCreateInfo, format) == 20U,
    "CNA_RenderTarget2DCreateInfo.format offset");
_Static_assert(sizeof(CNA_RenderTargetCubeCreateInfo) == 32U,
    "CNA_RenderTargetCubeCreateInfo layout changed");
_Static_assert(sizeof(CNA_RenderTargetInfo) == 44U,
    "CNA_RenderTargetInfo layout changed");
_Static_assert(offsetof(CNA_RenderTargetInfo, depth_format) == 28U,
    "CNA_RenderTargetInfo.depth offset");
_Static_assert(offsetof(CNA_RenderTargetInfo, is_content_lost) == 40U,
    "CNA_RenderTargetInfo.content-lost offset");
_Static_assert(sizeof(CNA_RenderTargetBinding) == 24U,
    "CNA_RenderTargetBinding layout changed");
_Static_assert(offsetof(CNA_RenderTargetBinding, render_target) == 8U,
    "CNA_RenderTargetBinding.handle offset");
_Static_assert(offsetof(CNA_RenderTargetBinding, cube_map_face) == 20U,
    "CNA_RenderTargetBinding.face offset");
_Static_assert(sizeof(CNA_VertexElement) == 16U, "CNA_VertexElement layout changed");
_Static_assert(offsetof(CNA_VertexElement, offset) == 0U,
    "CNA_VertexElement.offset offset");
_Static_assert(offsetof(CNA_VertexElement, usage_index) == 12U,
    "CNA_VertexElement.usage index offset");
_Static_assert(sizeof(CNA_VertexBufferCreateInfo) == 32U,
    "CNA_VertexBufferCreateInfo layout changed");
_Static_assert(offsetof(CNA_VertexBufferCreateInfo, vertex_declaration) == 8U,
    "CNA_VertexBufferCreateInfo.declaration offset");
_Static_assert(offsetof(CNA_VertexBufferCreateInfo, dynamic) == 24U,
    "CNA_VertexBufferCreateInfo.dynamic offset");
_Static_assert(sizeof(CNA_VertexBufferInfo) == 32U,
    "CNA_VertexBufferInfo layout changed");
_Static_assert(offsetof(CNA_VertexBufferInfo, vertex_stride) == 20U,
    "CNA_VertexBufferInfo.stride offset");
_Static_assert(offsetof(CNA_VertexBufferInfo, vertex_element_count) == 24U,
    "CNA_VertexBufferInfo.element count offset");
_Static_assert(sizeof(CNA_VertexBufferBinding) == 16U,
    "CNA_VertexBufferBinding layout changed");
_Static_assert(offsetof(CNA_VertexBufferBinding, vertex_offset) == 8U,
    "CNA_VertexBufferBinding.offset offset");
_Static_assert(sizeof(CNA_IndexBufferCreateInfo) == 24U,
    "CNA_IndexBufferCreateInfo layout changed");
_Static_assert(offsetof(CNA_IndexBufferCreateInfo, index_element_size) == 12U,
    "CNA_IndexBufferCreateInfo.element size offset");
_Static_assert(sizeof(CNA_IndexBufferInfo) == 24U,
    "CNA_IndexBufferInfo layout changed");
_Static_assert(sizeof(CNA_IndexBufferTransfer) == 32U,
    "CNA_IndexBufferTransfer layout changed");
_Static_assert(offsetof(CNA_IndexBufferTransfer, start_index) == 16U,
    "CNA_IndexBufferTransfer.start offset");
_Static_assert(sizeof(CNA_UserPrimitives) == 48U,
    "CNA_UserPrimitives layout changed");
_Static_assert(offsetof(CNA_UserPrimitives, vertex_data) == 16U,
    "CNA_UserPrimitives.vertex data offset");
_Static_assert(offsetof(CNA_UserPrimitives, vertex_declaration) == 24U,
    "CNA_UserPrimitives.declaration offset");
_Static_assert(sizeof(CNA_UserIndices) == 24U,
    "CNA_UserIndices layout changed");
_Static_assert(offsetof(CNA_UserIndices, index_data) == 16U,
    "CNA_UserIndices.index data offset");
_Static_assert(sizeof(CNA_VertexPositionColor) == 16U,
    "CNA_VertexPositionColor layout changed");
_Static_assert(sizeof(CNA_VertexPositionColorTexture) == 24U,
    "CNA_VertexPositionColorTexture layout changed");
_Static_assert(sizeof(CNA_VertexPositionTexture) == 20U,
    "CNA_VertexPositionTexture layout changed");
_Static_assert(sizeof(CNA_VertexPositionNormalTexture) == 32U,
    "CNA_VertexPositionNormalTexture layout changed");
_Static_assert(sizeof(CNA_SpriteBatchBeginInfo) == 16U, "CNA_SpriteBatchBeginInfo layout changed");
_Static_assert(offsetof(CNA_SpriteCommand, texture) == 8U, "CNA_SpriteCommand.texture offset");
_Static_assert(offsetof(CNA_SpriteScaledCommand, texture) == 8U, "CNA_SpriteScaledCommand.texture offset");
_Static_assert(sizeof(CNA_SpriteTextCommand) == 72U,
    "CNA_SpriteTextCommand layout changed");
_Static_assert(offsetof(CNA_SpriteTextCommand, sprite_font) == 8U,
    "CNA_SpriteTextCommand.font offset");
_Static_assert(offsetof(CNA_SpriteTextCommand, text) == 16U,
    "CNA_SpriteTextCommand.text offset");
_Static_assert(offsetof(CNA_SpriteTextCommand, scale) == 56U,
    "CNA_SpriteTextCommand.scale offset");
_Static_assert(sizeof(CNA_ContentManagerCreateInfo) == 32U,
    "CNA_ContentManagerCreateInfo layout changed");
_Static_assert(offsetof(CNA_ContentManagerCreateInfo, root_directory) == 8U,
    "CNA_ContentManagerCreateInfo.root offset");
_Static_assert(sizeof(CNA_SpriteFontInfo) == 32U,
    "CNA_SpriteFontInfo layout changed");
_Static_assert(offsetof(CNA_SpriteFontInfo, character_count) == 8U,
    "CNA_SpriteFontInfo.character count offset");
_Static_assert(offsetof(CNA_SpriteFontInfo, default_character) == 24U,
    "CNA_SpriteFontInfo.default character offset");
_Static_assert(sizeof(CNA_SoundEffectCreateInfo) == 24U,
    "CNA_SoundEffectCreateInfo layout changed");
_Static_assert(offsetof(CNA_SoundEffectCreateInfo, sample_rate) == 8U,
    "CNA_SoundEffectCreateInfo.sample rate offset");
_Static_assert(offsetof(CNA_SoundEffectCreateInfo, reserved) == 16U,
    "CNA_SoundEffectCreateInfo.reserved offset");
_Static_assert(sizeof(CNA_SoundEffectInstanceInfo) == 32U,
    "CNA_SoundEffectInstanceInfo layout changed");
_Static_assert(offsetof(CNA_SoundEffectInstanceInfo, state) == 8U,
    "CNA_SoundEffectInstanceInfo.state offset");
_Static_assert(offsetof(CNA_SoundEffectInstanceInfo, volume) == 16U,
    "CNA_SoundEffectInstanceInfo.volume offset");
_Static_assert(sizeof(CNA_AudioListener) == 56U,
    "CNA_AudioListener layout changed");
_Static_assert(offsetof(CNA_AudioListener, position) == 20U,
    "CNA_AudioListener.position offset");
_Static_assert(sizeof(CNA_AudioEmitter) == 60U,
    "CNA_AudioEmitter layout changed");
_Static_assert(offsetof(CNA_AudioEmitter, doppler_scale) == 8U,
    "CNA_AudioEmitter.doppler offset");
_Static_assert(offsetof(CNA_AudioEmitter, velocity) == 48U,
    "CNA_AudioEmitter.velocity offset");
_Static_assert(sizeof(CNA_CueInfo) == 16U, "CNA_CueInfo layout changed");
_Static_assert(offsetof(CNA_CueInfo, is_created) == 8U,
    "CNA_CueInfo.created offset");
_Static_assert(offsetof(CNA_CueInfo, is_stopping) == 15U,
    "CNA_CueInfo.stopping offset");
_Static_assert(sizeof(CNA_AudioEventRegistrationHandle) == 8U,
    "CNA audio event registration handle width changed");
_Static_assert(sizeof(CNA_GameTime) == 24U, "CNA_GameTime layout changed");
_Static_assert(offsetof(CNA_GameTime, total_game_time_ticks) == 0U, "CNA_GameTime.total offset");
_Static_assert(offsetof(CNA_GameTime, elapsed_game_time_ticks) == 8U, "CNA_GameTime.elapsed offset");
_Static_assert(offsetof(CNA_GameTime, is_running_slowly) == 16U, "CNA_GameTime.bool offset");
_Static_assert(offsetof(CNA_StringView, data) == 0U, "CNA_StringView.data offset");
_Static_assert(offsetof(CNA_StringView, byte_length) >= sizeof(void*), "CNA_StringView.length offset");
_Static_assert(CNA_ABI_VERSION == CNA_ABI_VERSION_ENCODE(0, 35, 0), "unexpected CNA header ABI");

/* The indirect-draw wire formats and the capability identities Java hard-codes.
 *
 * IndirectDrawArguments and IndirectDrawIndexedArguments are not marshalled field by field the
 * way every other structure here is: they are the GPU's own command format, and Java's toBytes()
 * lays out four and five little-endian words respectively, and nothing in the generated boundary
 * would notice if CNA reordered or resized them. These assertions are what does.
 *
 * GraphicsCapability is a Java enum whose values are its ordinals, which is only correct while
 * CNA's own constants are what they are today, so the check belongs here, against the live
 * header, where a change stops the build instead of silently asking the wrong question.
 */
_Static_assert(sizeof(CNA_IndirectDrawArguments) == 16U,
    "CNA_IndirectDrawArguments must be four 32-bit words");
_Static_assert(offsetof(CNA_IndirectDrawArguments, vertex_count) == 0U,
    "CNA_IndirectDrawArguments.vertex_count offset");
_Static_assert(offsetof(CNA_IndirectDrawArguments, instance_count) == 4U,
    "CNA_IndirectDrawArguments.instance_count offset");
_Static_assert(offsetof(CNA_IndirectDrawArguments, first_vertex) == 8U,
    "CNA_IndirectDrawArguments.first_vertex offset");
_Static_assert(offsetof(CNA_IndirectDrawArguments, base_instance) == 12U,
    "CNA_IndirectDrawArguments.base_instance offset");
_Static_assert(sizeof(CNA_IndirectDrawIndexedArguments) == 20U,
    "CNA_IndirectDrawIndexedArguments must be five 32-bit words");
_Static_assert(offsetof(CNA_IndirectDrawIndexedArguments, index_count) == 0U,
    "CNA_IndirectDrawIndexedArguments.index_count offset");
_Static_assert(offsetof(CNA_IndirectDrawIndexedArguments, instance_count) == 4U,
    "CNA_IndirectDrawIndexedArguments.instance_count offset");
_Static_assert(offsetof(CNA_IndirectDrawIndexedArguments, first_index) == 8U,
    "CNA_IndirectDrawIndexedArguments.first_index offset");
_Static_assert(offsetof(CNA_IndirectDrawIndexedArguments, base_vertex) == 12U,
    "CNA_IndirectDrawIndexedArguments.base_vertex offset");
_Static_assert(offsetof(CNA_IndirectDrawIndexedArguments, base_instance) == 16U,
    "CNA_IndirectDrawIndexedArguments.base_instance offset");

_Static_assert(CNA_GRAPHICS_CAPABILITY_THREE_D == 0U, "GraphicsCapability.ThreeD ordinal");
_Static_assert(CNA_GRAPHICS_CAPABILITY_COMPUTE_SHADERS == 17U,
    "GraphicsCapability.ComputeShaders ordinal");
_Static_assert(CNA_GRAPHICS_CAPABILITY_INDIRECT_DRAW == 18U,
    "GraphicsCapability.IndirectDraw ordinal");
_Static_assert(CNA_GRAPHICS_CAPABILITY_MAXIMUM == CNA_GRAPHICS_CAPABILITY_INDIRECT_DRAW,
    "GraphicsCapability has gained a constant Java has no enum for");


#define ASSERT_SIGNATURE(name, ...) \
    _Static_assert(_Generic(&(name), __VA_ARGS__: 1, default: 0), \
        #name " signature changed")

typedef CNA_Result (*AudioUnarySignature)(CNA_Handle);
typedef CNA_Result (*AudioHandleBoolOutSignature)(CNA_Handle, CNA_Bool*);
typedef CNA_Result (*AudioHandleFloatOutSignature)(CNA_Handle, float*);
typedef CNA_Result (*AudioHandleFloatSignature)(CNA_Handle, float);
typedef CNA_Result (*AudioHandleSizeOutSignature)(CNA_Handle, uint64_t*);
typedef CNA_Result (*AudioHandleCopyStringSignature)(
    CNA_Handle, char*, uint64_t, uint64_t*);
typedef CNA_Result (*AudioHandleStringSignature)(CNA_Handle, CNA_StringView);
typedef CNA_Result (*AudioHandleOutSignature)(CNA_Handle, CNA_Handle*);
typedef CNA_Result (*AudioHandleStringHandleOutSignature)(
    CNA_Handle, CNA_StringView, CNA_Handle*);
typedef CNA_Result (*AudioHandleStringFloatOutSignature)(
    CNA_Handle, CNA_StringView, float*);
typedef CNA_Result (*AudioHandleStringFloatSignature)(
    CNA_Handle, CNA_StringView, float);
typedef CNA_Result (*AudioIndexedSizeSignature)(CNA_Handle, uint64_t, uint64_t*);
typedef CNA_Result (*AudioIndexedCopyStringSignature)(
    CNA_Handle, uint64_t, char*, uint64_t, uint64_t*);

ASSERT_SIGNATURE(cna_sound_effect_create_pcm16_range_ext,
    CNA_Result (*)(CNA_Handle, const CNA_SoundEffectCreateInfo*, const uint8_t*, uint64_t,
        int32_t, int32_t, int32_t, int32_t, CNA_Handle*));
ASSERT_SIGNATURE(cna_sound_effect_create_from_encoded_ext,
    CNA_Result (*)(CNA_Handle, const uint8_t*, uint64_t, CNA_Handle*));
ASSERT_SIGNATURE(cna_sound_effect_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_effect_create_instance, AudioHandleOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_play, AudioHandleBoolOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_play_with_settings,
    CNA_Result (*)(CNA_Handle, float, float, float, CNA_Bool*));
ASSERT_SIGNATURE(cna_sound_effect_get_duration_ticks,
    CNA_Result (*)(CNA_Handle, int64_t*));
ASSERT_SIGNATURE(cna_sound_effect_get_name_size, AudioHandleSizeOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_copy_name, AudioHandleCopyStringSignature);
ASSERT_SIGNATURE(cna_sound_effect_set_name, AudioHandleStringSignature);
ASSERT_SIGNATURE(cna_sound_effect_get_master_volume, AudioHandleFloatOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_set_master_volume, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_get_distance_scale, AudioHandleFloatOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_set_distance_scale, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_get_doppler_scale, AudioHandleFloatOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_set_doppler_scale, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_get_speed_of_sound, AudioHandleFloatOutSignature);
ASSERT_SIGNATURE(cna_sound_effect_set_speed_of_sound, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_play, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_pause, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_resume, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_stop,
    CNA_Result (*)(CNA_Handle, CNA_Bool));
ASSERT_SIGNATURE(cna_sound_effect_instance_get_info,
    CNA_Result (*)(CNA_Handle, CNA_SoundEffectInstanceInfo*));
ASSERT_SIGNATURE(cna_sound_effect_instance_set_volume, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_set_pitch, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_set_pan, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_set_is_looped,
    CNA_Result (*)(CNA_Handle, CNA_Bool));
ASSERT_SIGNATURE(cna_sound_effect_instance_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_effect_instance_apply_3d,
    CNA_Result (*)(CNA_Handle, const CNA_AudioListener*, const CNA_AudioEmitter*));
ASSERT_SIGNATURE(cna_sound_effect_instance_apply_3d_multi_ext,
    CNA_Result (*)(CNA_Handle, const CNA_AudioListener*, uint64_t,
        const CNA_AudioEmitter*));
ASSERT_SIGNATURE(cna_dynamic_sound_effect_instance_create,
    CNA_Result (*)(CNA_Handle, int32_t, CNA_AudioChannels, CNA_Handle*));
ASSERT_SIGNATURE(cna_dynamic_sound_effect_instance_get_pending_buffer_count,
    CNA_Result (*)(CNA_Handle, int32_t*));
ASSERT_SIGNATURE(cna_dynamic_sound_effect_instance_submit_buffer,
    CNA_Result (*)(CNA_Handle, const uint8_t*, uint64_t, int32_t, int32_t));
ASSERT_SIGNATURE(cna_dynamic_sound_effect_instance_subscribe_buffer_needed,
    CNA_Result (*)(CNA_Handle, CNA_AudioEventCallback, void*,
        CNA_AudioEventRegistrationHandle*));
ASSERT_SIGNATURE(cna_audio_unsubscribe_ext, AudioUnarySignature);
ASSERT_SIGNATURE(cna_microphone_get_count, AudioHandleSizeOutSignature);
ASSERT_SIGNATURE(cna_microphone_get_default_index_ext,
    CNA_Result (*)(CNA_Handle, uint64_t*, CNA_Bool*));
ASSERT_SIGNATURE(cna_microphone_get_name_size_at, AudioIndexedSizeSignature);
ASSERT_SIGNATURE(cna_microphone_copy_name_at, AudioIndexedCopyStringSignature);
ASSERT_SIGNATURE(cna_microphone_get_buffer_duration_ticks_at,
    CNA_Result (*)(CNA_Handle, uint64_t, int64_t*));
ASSERT_SIGNATURE(cna_microphone_set_buffer_duration_ticks_at,
    CNA_Result (*)(CNA_Handle, uint64_t, int64_t));
ASSERT_SIGNATURE(cna_microphone_get_is_headset_at,
    CNA_Result (*)(CNA_Handle, uint64_t, CNA_Bool*));
ASSERT_SIGNATURE(cna_microphone_get_sample_rate_at,
    CNA_Result (*)(CNA_Handle, uint64_t, int32_t*));
ASSERT_SIGNATURE(cna_microphone_get_state_at,
    CNA_Result (*)(CNA_Handle, uint64_t, CNA_MicrophoneState*));
ASSERT_SIGNATURE(cna_microphone_start_at,
    CNA_Result (*)(CNA_Handle, uint64_t));
ASSERT_SIGNATURE(cna_microphone_stop_at,
    CNA_Result (*)(CNA_Handle, uint64_t));
ASSERT_SIGNATURE(cna_microphone_get_data_at,
    CNA_Result (*)(CNA_Handle, uint64_t, uint8_t*, uint64_t, uint64_t*));
ASSERT_SIGNATURE(cna_microphone_subscribe_buffer_ready_at,
    CNA_Result (*)(CNA_Handle, uint64_t, CNA_AudioEventCallback, void*,
        CNA_AudioEventRegistrationHandle*));
ASSERT_SIGNATURE(cna_audio_engine_create_with_renderer,
    CNA_Result (*)(CNA_Handle, CNA_StringView, int64_t, CNA_StringView, CNA_Handle*));
ASSERT_SIGNATURE(cna_audio_engine_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_audio_engine_get_renderer_count, AudioHandleSizeOutSignature);
ASSERT_SIGNATURE(cna_audio_engine_get_renderer_friendly_name_size, AudioIndexedSizeSignature);
ASSERT_SIGNATURE(cna_audio_engine_copy_renderer_friendly_name,
    AudioIndexedCopyStringSignature);
ASSERT_SIGNATURE(cna_audio_engine_get_renderer_id_size, AudioIndexedSizeSignature);
ASSERT_SIGNATURE(cna_audio_engine_copy_renderer_id, AudioIndexedCopyStringSignature);
ASSERT_SIGNATURE(cna_audio_engine_get_category, AudioHandleStringHandleOutSignature);
ASSERT_SIGNATURE(cna_audio_engine_get_global_variable, AudioHandleStringFloatOutSignature);
ASSERT_SIGNATURE(cna_audio_engine_set_global_variable, AudioHandleStringFloatSignature);
ASSERT_SIGNATURE(cna_audio_engine_update, AudioUnarySignature);
ASSERT_SIGNATURE(cna_audio_category_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_audio_category_get_name_size, AudioHandleSizeOutSignature);
ASSERT_SIGNATURE(cna_audio_category_copy_name, AudioHandleCopyStringSignature);
ASSERT_SIGNATURE(cna_audio_category_pause, AudioUnarySignature);
ASSERT_SIGNATURE(cna_audio_category_resume, AudioUnarySignature);
ASSERT_SIGNATURE(cna_audio_category_set_volume, AudioHandleFloatSignature);
ASSERT_SIGNATURE(cna_audio_category_stop,
    CNA_Result (*)(CNA_Handle, CNA_AudioStopOptions));
ASSERT_SIGNATURE(cna_audio_category_equals,
    CNA_Result (*)(CNA_Handle, CNA_Handle, CNA_Bool*));
ASSERT_SIGNATURE(cna_audio_category_get_hash_code,
    CNA_Result (*)(CNA_Handle, int32_t*));
ASSERT_SIGNATURE(cna_wave_bank_create, AudioHandleStringHandleOutSignature);
ASSERT_SIGNATURE(cna_wave_bank_create_streaming,
    CNA_Result (*)(CNA_Handle, CNA_StringView, int32_t, int16_t, CNA_Handle*));
ASSERT_SIGNATURE(cna_wave_bank_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_wave_bank_get_is_prepared, AudioHandleBoolOutSignature);
ASSERT_SIGNATURE(cna_wave_bank_get_is_in_use, AudioHandleBoolOutSignature);
ASSERT_SIGNATURE(cna_sound_bank_create, AudioHandleStringHandleOutSignature);
ASSERT_SIGNATURE(cna_sound_bank_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_sound_bank_get_is_in_use, AudioHandleBoolOutSignature);
ASSERT_SIGNATURE(cna_sound_bank_get_cue, AudioHandleStringHandleOutSignature);
ASSERT_SIGNATURE(cna_sound_bank_play_cue, AudioHandleStringSignature);
ASSERT_SIGNATURE(cna_sound_bank_play_cue_3d,
    CNA_Result (*)(CNA_Handle, CNA_StringView,
        const CNA_AudioListener*, const CNA_AudioEmitter*));
ASSERT_SIGNATURE(cna_cue_destroy, AudioUnarySignature);
ASSERT_SIGNATURE(cna_cue_get_info, CNA_Result (*)(CNA_Handle, CNA_CueInfo*));
ASSERT_SIGNATURE(cna_cue_apply_3d,
    CNA_Result (*)(CNA_Handle, const CNA_AudioListener*, const CNA_AudioEmitter*));
ASSERT_SIGNATURE(cna_cue_get_variable, AudioHandleStringFloatOutSignature);
ASSERT_SIGNATURE(cna_cue_set_variable, AudioHandleStringFloatSignature);
ASSERT_SIGNATURE(cna_cue_play, AudioUnarySignature);
ASSERT_SIGNATURE(cna_cue_pause, AudioUnarySignature);
ASSERT_SIGNATURE(cna_cue_resume, AudioUnarySignature);
ASSERT_SIGNATURE(cna_cue_stop, CNA_Result (*)(CNA_Handle, CNA_AudioStopOptions));
ASSERT_SIGNATURE(cna_media_source_get_available_count, CNA_Result (*)(CNA_Handle game, uint32_t* out_count));
ASSERT_SIGNATURE(cna_media_source_get_type_at, CNA_Result (*)(CNA_Handle game, uint32_t index, CNA_MediaSourceType* out_type));
ASSERT_SIGNATURE(cna_media_source_get_name_size_at, CNA_Result (*)(CNA_Handle game, uint32_t index, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_media_source_copy_name_at, CNA_Result (*)(CNA_Handle game, uint32_t index, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_media_library_create, CNA_Result (*)(CNA_Handle game, CNA_MediaLibraryHandle* out_library));
ASSERT_SIGNATURE(cna_media_library_create_from_source, CNA_Result (*)(CNA_Handle game, uint32_t source_index, CNA_MediaLibraryHandle* out_library));
ASSERT_SIGNATURE(cna_media_library_get_is_disposed, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_media_library_dispose, CNA_Result (*)(CNA_MediaLibraryHandle library));
ASSERT_SIGNATURE(cna_media_library_destroy, CNA_Result (*)(CNA_MediaLibraryHandle library));
ASSERT_SIGNATURE(cna_media_library_get_media_source_type, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_MediaSourceType* out_type));
ASSERT_SIGNATURE(cna_media_library_get_media_source_name_size, CNA_Result (*)(CNA_MediaLibraryHandle library, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_media_library_copy_media_source_name, CNA_Result (*)(CNA_MediaLibraryHandle library, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_media_library_get_songs, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_SongCollectionHandle* out_songs));
ASSERT_SIGNATURE(cna_media_library_get_albums, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_AlbumCollectionHandle* out_albums));
ASSERT_SIGNATURE(cna_media_library_get_artists, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_ArtistCollectionHandle* out_artists));
ASSERT_SIGNATURE(cna_media_library_get_genres, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_GenreCollectionHandle* out_genres));
ASSERT_SIGNATURE(cna_media_library_get_playlists, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_PlaylistCollectionHandle* out_playlists));
ASSERT_SIGNATURE(cna_media_library_get_pictures, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_PictureCollectionHandle* out_pictures));
ASSERT_SIGNATURE(cna_media_library_get_saved_pictures, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_PictureCollectionHandle* out_pictures));
ASSERT_SIGNATURE(cna_media_library_get_root_picture_album, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_PictureAlbumHandle* out_album, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_media_library_get_picture_from_token, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_StringView token, CNA_PictureHandle* out_picture, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_media_library_save_picture, CNA_Result (*)(CNA_MediaLibraryHandle library, CNA_StringView name, const uint8_t* image_data, uint64_t image_byte_count, CNA_PictureHandle* out_picture));
ASSERT_SIGNATURE(cna_album_get_name_size, CNA_Result (*)(CNA_AlbumHandle album, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_copy_name, CNA_Result (*)(CNA_AlbumHandle album, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_get_is_disposed, CNA_Result (*)(CNA_AlbumHandle album, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_album_dispose, CNA_Result (*)(CNA_AlbumHandle album));
ASSERT_SIGNATURE(cna_album_destroy, CNA_Result (*)(CNA_AlbumHandle album));
ASSERT_SIGNATURE(cna_album_equals, CNA_Result (*)(CNA_AlbumHandle left, CNA_AlbumHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_album_get_hash_code, CNA_Result (*)(CNA_AlbumHandle album, int32_t* out_hash));
ASSERT_SIGNATURE(cna_artist_get_name_size, CNA_Result (*)(CNA_ArtistHandle artist, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_artist_copy_name, CNA_Result (*)(CNA_ArtistHandle artist, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_artist_get_is_disposed, CNA_Result (*)(CNA_ArtistHandle artist, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_artist_dispose, CNA_Result (*)(CNA_ArtistHandle artist));
ASSERT_SIGNATURE(cna_artist_destroy, CNA_Result (*)(CNA_ArtistHandle artist));
ASSERT_SIGNATURE(cna_artist_equals, CNA_Result (*)(CNA_ArtistHandle left, CNA_ArtistHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_artist_get_hash_code, CNA_Result (*)(CNA_ArtistHandle artist, int32_t* out_hash));
ASSERT_SIGNATURE(cna_genre_get_name_size, CNA_Result (*)(CNA_GenreHandle genre, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_genre_copy_name, CNA_Result (*)(CNA_GenreHandle genre, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_genre_get_is_disposed, CNA_Result (*)(CNA_GenreHandle genre, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_genre_dispose, CNA_Result (*)(CNA_GenreHandle genre));
ASSERT_SIGNATURE(cna_genre_destroy, CNA_Result (*)(CNA_GenreHandle genre));
ASSERT_SIGNATURE(cna_genre_equals, CNA_Result (*)(CNA_GenreHandle left, CNA_GenreHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_genre_get_hash_code, CNA_Result (*)(CNA_GenreHandle genre, int32_t* out_hash));
ASSERT_SIGNATURE(cna_playlist_get_name_size, CNA_Result (*)(CNA_PlaylistHandle playlist, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_playlist_copy_name, CNA_Result (*)(CNA_PlaylistHandle playlist, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_playlist_get_is_disposed, CNA_Result (*)(CNA_PlaylistHandle playlist, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_playlist_dispose, CNA_Result (*)(CNA_PlaylistHandle playlist));
ASSERT_SIGNATURE(cna_playlist_destroy, CNA_Result (*)(CNA_PlaylistHandle playlist));
ASSERT_SIGNATURE(cna_playlist_equals, CNA_Result (*)(CNA_PlaylistHandle left, CNA_PlaylistHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_playlist_get_hash_code, CNA_Result (*)(CNA_PlaylistHandle playlist, int32_t* out_hash));
ASSERT_SIGNATURE(cna_picture_get_name_size, CNA_Result (*)(CNA_PictureHandle picture, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_copy_name, CNA_Result (*)(CNA_PictureHandle picture, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_get_is_disposed, CNA_Result (*)(CNA_PictureHandle picture, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_picture_dispose, CNA_Result (*)(CNA_PictureHandle picture));
ASSERT_SIGNATURE(cna_picture_destroy, CNA_Result (*)(CNA_PictureHandle picture));
ASSERT_SIGNATURE(cna_picture_equals, CNA_Result (*)(CNA_PictureHandle left, CNA_PictureHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_picture_get_hash_code, CNA_Result (*)(CNA_PictureHandle picture, int32_t* out_hash));
ASSERT_SIGNATURE(cna_picture_album_get_name_size, CNA_Result (*)(CNA_PictureAlbumHandle album, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_album_copy_name, CNA_Result (*)(CNA_PictureAlbumHandle album, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_album_get_is_disposed, CNA_Result (*)(CNA_PictureAlbumHandle album, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_picture_album_dispose, CNA_Result (*)(CNA_PictureAlbumHandle album));
ASSERT_SIGNATURE(cna_picture_album_destroy, CNA_Result (*)(CNA_PictureAlbumHandle album));
ASSERT_SIGNATURE(cna_picture_album_equals, CNA_Result (*)(CNA_PictureAlbumHandle left, CNA_PictureAlbumHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_picture_album_get_hash_code, CNA_Result (*)(CNA_PictureAlbumHandle album, int32_t* out_hash));
ASSERT_SIGNATURE(cna_song_get_name_size, CNA_Result (*)(CNA_SongHandle song, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_song_copy_name, CNA_Result (*)(CNA_SongHandle song, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_song_get_is_disposed, CNA_Result (*)(CNA_SongHandle song, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_song_dispose, CNA_Result (*)(CNA_SongHandle song));
ASSERT_SIGNATURE(cna_song_destroy, CNA_Result (*)(CNA_SongHandle song));
ASSERT_SIGNATURE(cna_song_equals, CNA_Result (*)(CNA_SongHandle left, CNA_SongHandle right, CNA_Bool* out_equal));
ASSERT_SIGNATURE(cna_song_get_hash_code, CNA_Result (*)(CNA_SongHandle song, int32_t* out_hash));
ASSERT_SIGNATURE(cna_album_collection_get_count, CNA_Result (*)(CNA_AlbumCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_album_collection_get_at, CNA_Result (*)(CNA_AlbumCollectionHandle collection, int32_t index, CNA_AlbumHandle* out_album));
ASSERT_SIGNATURE(cna_album_collection_get_is_disposed, CNA_Result (*)(CNA_AlbumCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_album_collection_dispose, CNA_Result (*)(CNA_AlbumCollectionHandle collection));
ASSERT_SIGNATURE(cna_album_collection_destroy, CNA_Result (*)(CNA_AlbumCollectionHandle collection));
ASSERT_SIGNATURE(cna_artist_collection_get_count, CNA_Result (*)(CNA_ArtistCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_artist_collection_get_at, CNA_Result (*)(CNA_ArtistCollectionHandle collection, int32_t index, CNA_ArtistHandle* out_artist));
ASSERT_SIGNATURE(cna_artist_collection_get_is_disposed, CNA_Result (*)(CNA_ArtistCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_artist_collection_dispose, CNA_Result (*)(CNA_ArtistCollectionHandle collection));
ASSERT_SIGNATURE(cna_artist_collection_destroy, CNA_Result (*)(CNA_ArtistCollectionHandle collection));
ASSERT_SIGNATURE(cna_genre_collection_get_count, CNA_Result (*)(CNA_GenreCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_genre_collection_get_at, CNA_Result (*)(CNA_GenreCollectionHandle collection, int32_t index, CNA_GenreHandle* out_genre));
ASSERT_SIGNATURE(cna_genre_collection_get_is_disposed, CNA_Result (*)(CNA_GenreCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_genre_collection_dispose, CNA_Result (*)(CNA_GenreCollectionHandle collection));
ASSERT_SIGNATURE(cna_genre_collection_destroy, CNA_Result (*)(CNA_GenreCollectionHandle collection));
ASSERT_SIGNATURE(cna_playlist_collection_get_count, CNA_Result (*)(CNA_PlaylistCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_playlist_collection_get_at, CNA_Result (*)(CNA_PlaylistCollectionHandle collection, int32_t index, CNA_PlaylistHandle* out_playlist));
ASSERT_SIGNATURE(cna_playlist_collection_get_is_disposed, CNA_Result (*)(CNA_PlaylistCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_playlist_collection_dispose, CNA_Result (*)(CNA_PlaylistCollectionHandle collection));
ASSERT_SIGNATURE(cna_playlist_collection_destroy, CNA_Result (*)(CNA_PlaylistCollectionHandle collection));
ASSERT_SIGNATURE(cna_picture_collection_get_count, CNA_Result (*)(CNA_PictureCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_picture_collection_get_at, CNA_Result (*)(CNA_PictureCollectionHandle collection, int32_t index, CNA_PictureHandle* out_picture));
ASSERT_SIGNATURE(cna_picture_collection_get_is_disposed, CNA_Result (*)(CNA_PictureCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_picture_collection_dispose, CNA_Result (*)(CNA_PictureCollectionHandle collection));
ASSERT_SIGNATURE(cna_picture_collection_destroy, CNA_Result (*)(CNA_PictureCollectionHandle collection));
ASSERT_SIGNATURE(cna_picture_album_collection_get_count, CNA_Result (*)(CNA_PictureAlbumCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_picture_album_collection_get_at, CNA_Result (*)(CNA_PictureAlbumCollectionHandle collection, int32_t index, CNA_PictureAlbumHandle* out_album));
ASSERT_SIGNATURE(cna_picture_album_collection_get_is_disposed, CNA_Result (*)(CNA_PictureAlbumCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_picture_album_collection_dispose, CNA_Result (*)(CNA_PictureAlbumCollectionHandle collection));
ASSERT_SIGNATURE(cna_picture_album_collection_destroy, CNA_Result (*)(CNA_PictureAlbumCollectionHandle collection));
ASSERT_SIGNATURE(cna_song_collection_get_count, CNA_Result (*)(CNA_SongCollectionHandle collection, int32_t* out_count));
ASSERT_SIGNATURE(cna_song_collection_get_at, CNA_Result (*)(CNA_SongCollectionHandle collection, int32_t index, CNA_SongHandle* out_song));
ASSERT_SIGNATURE(cna_song_collection_get_is_disposed, CNA_Result (*)(CNA_SongCollectionHandle collection, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_song_collection_dispose, CNA_Result (*)(CNA_SongCollectionHandle collection));
ASSERT_SIGNATURE(cna_song_collection_destroy, CNA_Result (*)(CNA_SongCollectionHandle collection));
ASSERT_SIGNATURE(cna_album_get_artist, CNA_Result (*)(CNA_AlbumHandle album, CNA_ArtistHandle* out_artist, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_album_get_genre, CNA_Result (*)(CNA_AlbumHandle album, CNA_GenreHandle* out_genre, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_album_get_duration, CNA_Result (*)(CNA_AlbumHandle album, int64_t* out_ticks));
ASSERT_SIGNATURE(cna_album_get_has_art, CNA_Result (*)(CNA_AlbumHandle album, CNA_Bool* out_has_art));
ASSERT_SIGNATURE(cna_album_get_art_size, CNA_Result (*)(CNA_AlbumHandle album, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_copy_art, CNA_Result (*)(CNA_AlbumHandle album, uint8_t* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_get_thumbnail_size, CNA_Result (*)(CNA_AlbumHandle album, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_copy_thumbnail, CNA_Result (*)(CNA_AlbumHandle album, uint8_t* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_album_get_songs, CNA_Result (*)(CNA_AlbumHandle album, CNA_SongCollectionHandle* out_songs));
ASSERT_SIGNATURE(cna_artist_get_albums, CNA_Result (*)(CNA_ArtistHandle artist, CNA_AlbumCollectionHandle* out_albums));
ASSERT_SIGNATURE(cna_artist_get_songs, CNA_Result (*)(CNA_ArtistHandle artist, CNA_SongCollectionHandle* out_songs));
ASSERT_SIGNATURE(cna_genre_get_albums, CNA_Result (*)(CNA_GenreHandle genre, CNA_AlbumCollectionHandle* out_albums));
ASSERT_SIGNATURE(cna_genre_get_songs, CNA_Result (*)(CNA_GenreHandle genre, CNA_SongCollectionHandle* out_songs));
ASSERT_SIGNATURE(cna_playlist_get_duration, CNA_Result (*)(CNA_PlaylistHandle playlist, int64_t* out_ticks));
ASSERT_SIGNATURE(cna_playlist_get_songs, CNA_Result (*)(CNA_PlaylistHandle playlist, CNA_SongCollectionHandle* out_songs));
ASSERT_SIGNATURE(cna_picture_get_album, CNA_Result (*)(CNA_PictureHandle picture, CNA_PictureAlbumHandle* out_album, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_picture_get_date_unix_ticks, CNA_Result (*)(CNA_PictureHandle picture, int64_t* out_unix_ticks));
ASSERT_SIGNATURE(cna_picture_get_width, CNA_Result (*)(CNA_PictureHandle picture, int32_t* out_width));
ASSERT_SIGNATURE(cna_picture_get_height, CNA_Result (*)(CNA_PictureHandle picture, int32_t* out_height));
ASSERT_SIGNATURE(cna_picture_get_image_size, CNA_Result (*)(CNA_PictureHandle picture, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_copy_image, CNA_Result (*)(CNA_PictureHandle picture, uint8_t* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_get_thumbnail_size, CNA_Result (*)(CNA_PictureHandle picture, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_copy_thumbnail, CNA_Result (*)(CNA_PictureHandle picture, uint8_t* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_picture_album_get_parent, CNA_Result (*)(CNA_PictureAlbumHandle album, CNA_PictureAlbumHandle* out_parent, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_picture_album_get_albums, CNA_Result (*)(CNA_PictureAlbumHandle album, CNA_PictureAlbumCollectionHandle* out_albums));
ASSERT_SIGNATURE(cna_picture_album_get_pictures, CNA_Result (*)(CNA_PictureAlbumHandle album, CNA_PictureCollectionHandle* out_pictures));
ASSERT_SIGNATURE(cna_song_get_album, CNA_Result (*)(CNA_SongHandle song, CNA_AlbumHandle* out_album, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_song_get_artist, CNA_Result (*)(CNA_SongHandle song, CNA_ArtistHandle* out_artist, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_song_get_genre, CNA_Result (*)(CNA_SongHandle song, CNA_GenreHandle* out_genre, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_song_get_duration, CNA_Result (*)(CNA_SongHandle song, int64_t* out_ticks));
ASSERT_SIGNATURE(cna_song_get_is_protected, CNA_Result (*)(CNA_SongHandle song, CNA_Bool* out_protected));
ASSERT_SIGNATURE(cna_song_get_is_rated, CNA_Result (*)(CNA_SongHandle song, CNA_Bool* out_rated));
ASSERT_SIGNATURE(cna_song_get_play_count, CNA_Result (*)(CNA_SongHandle song, int32_t* out_play_count));
ASSERT_SIGNATURE(cna_song_get_rating, CNA_Result (*)(CNA_SongHandle song, int32_t* out_rating));
ASSERT_SIGNATURE(cna_song_get_track_number, CNA_Result (*)(CNA_SongHandle song, int32_t* out_track_number));
ASSERT_SIGNATURE(cna_song_create_from_uri, CNA_Result (*)(CNA_Handle game, CNA_StringView name, CNA_StringView uri, CNA_SongHandle* out_song));
ASSERT_SIGNATURE(cna_media_player_get_game_has_control, CNA_Result (*)(CNA_Handle game, CNA_Bool* out_has_control));
ASSERT_SIGNATURE(cna_media_player_get_is_muted, CNA_Result (*)(CNA_Handle game, CNA_Bool* out_muted));
ASSERT_SIGNATURE(cna_media_player_set_is_muted, CNA_Result (*)(CNA_Handle game, CNA_Bool muted));
ASSERT_SIGNATURE(cna_media_player_get_is_repeating, CNA_Result (*)(CNA_Handle game, CNA_Bool* out_repeating));
ASSERT_SIGNATURE(cna_media_player_set_is_repeating, CNA_Result (*)(CNA_Handle game, CNA_Bool repeating));
ASSERT_SIGNATURE(cna_media_player_get_is_shuffled, CNA_Result (*)(CNA_Handle game, CNA_Bool* out_shuffled));
ASSERT_SIGNATURE(cna_media_player_set_is_shuffled, CNA_Result (*)(CNA_Handle game, CNA_Bool shuffled));
ASSERT_SIGNATURE(cna_media_player_get_play_position_ticks, CNA_Result (*)(CNA_Handle game, int64_t* out_ticks));
ASSERT_SIGNATURE(cna_media_player_get_state, CNA_Result (*)(CNA_Handle game, CNA_MediaState* out_state));
ASSERT_SIGNATURE(cna_media_player_get_volume, CNA_Result (*)(CNA_Handle game, float* out_volume));
ASSERT_SIGNATURE(cna_media_player_set_volume, CNA_Result (*)(CNA_Handle game, float volume));
ASSERT_SIGNATURE(cna_media_player_get_is_visualization_enabled, CNA_Result (*)(CNA_Handle game, CNA_Bool* out_enabled));
ASSERT_SIGNATURE(cna_media_player_set_is_visualization_enabled, CNA_Result (*)(CNA_Handle game, CNA_Bool enabled));
ASSERT_SIGNATURE(cna_media_player_get_visualization_data, CNA_Result (*)(CNA_Handle game, CNA_VisualizationData* data));
ASSERT_SIGNATURE(cna_media_player_get_queue, CNA_Result (*)(CNA_Handle game, CNA_MediaQueueHandle* out_queue));
ASSERT_SIGNATURE(cna_media_player_play_song, CNA_Result (*)(CNA_Handle game, CNA_SongHandle song));
ASSERT_SIGNATURE(cna_media_player_play_songs, CNA_Result (*)(CNA_Handle game, CNA_SongCollectionHandle songs));
ASSERT_SIGNATURE(cna_media_player_play_songs_from, CNA_Result (*)(CNA_Handle game, CNA_SongCollectionHandle songs, int32_t index));
ASSERT_SIGNATURE(cna_media_player_move_next, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_move_previous, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_pause, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_resume, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_stop, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_program_exit_ext, CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_raise_active_song_changed_ext,
    CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_raise_media_state_changed_ext,
    CNA_Result (*)(CNA_Handle game));
ASSERT_SIGNATURE(cna_media_player_subscribe_active_song_changed_ext, CNA_Result (*)(CNA_MediaPlayerEventCallback callback, void* context, CNA_MediaPlayerEventRegistrationHandle* out_registration));
ASSERT_SIGNATURE(cna_media_player_subscribe_media_state_changed_ext, CNA_Result (*)(CNA_MediaPlayerEventCallback callback, void* context, CNA_MediaPlayerEventRegistrationHandle* out_registration));
ASSERT_SIGNATURE(cna_media_player_unsubscribe_ext, CNA_Result (*)(CNA_MediaPlayerEventRegistrationHandle registration));
ASSERT_SIGNATURE(cna_media_queue_get_count, CNA_Result (*)(CNA_MediaQueueHandle queue, int32_t* out_count));
ASSERT_SIGNATURE(cna_media_queue_get_active_song_index, CNA_Result (*)(CNA_MediaQueueHandle queue, int32_t* out_index));
ASSERT_SIGNATURE(cna_media_queue_set_active_song_index, CNA_Result (*)(CNA_MediaQueueHandle queue, int32_t index));
ASSERT_SIGNATURE(cna_media_queue_get_at, CNA_Result (*)(CNA_MediaQueueHandle queue, int32_t index, CNA_SongHandle* out_song));
ASSERT_SIGNATURE(cna_media_queue_destroy, CNA_Result (*)(CNA_MediaQueueHandle queue));
ASSERT_SIGNATURE(cna_video_create_with_metadata, CNA_Result (*)(CNA_Handle graphics_device, CNA_StringView file_name, int32_t duration_milliseconds, int32_t width, int32_t height, float frames_per_second, CNA_VideoSoundtrackType soundtrack_type, CNA_VideoHandle* out_video));
ASSERT_SIGNATURE(cna_video_destroy, CNA_Result (*)(CNA_VideoHandle video));
ASSERT_SIGNATURE(cna_video_player_create, CNA_Result (*)(CNA_Handle game, CNA_VideoPlayerHandle* out_player));
ASSERT_SIGNATURE(cna_video_player_get_is_disposed, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_Bool* out_disposed));
ASSERT_SIGNATURE(cna_video_player_set_is_looped, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_Bool looped));
ASSERT_SIGNATURE(cna_video_player_set_is_muted, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_Bool muted));
ASSERT_SIGNATURE(cna_video_player_get_play_position_ticks, CNA_Result (*)(CNA_VideoPlayerHandle player, int64_t* out_ticks));
ASSERT_SIGNATURE(cna_video_player_get_state, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_MediaState* out_state));
ASSERT_SIGNATURE(cna_video_player_set_volume, CNA_Result (*)(CNA_VideoPlayerHandle player, float volume));
ASSERT_SIGNATURE(cna_video_player_get_texture, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_Handle* out_texture, CNA_Bool* out_available));
ASSERT_SIGNATURE(cna_video_player_play, CNA_Result (*)(CNA_VideoPlayerHandle player, CNA_VideoHandle video));
ASSERT_SIGNATURE(cna_video_player_stop, CNA_Result (*)(CNA_VideoPlayerHandle player));
ASSERT_SIGNATURE(cna_video_player_pause, CNA_Result (*)(CNA_VideoPlayerHandle player));
ASSERT_SIGNATURE(cna_video_player_resume, CNA_Result (*)(CNA_VideoPlayerHandle player));
ASSERT_SIGNATURE(cna_video_player_dispose, CNA_Result (*)(CNA_VideoPlayerHandle player));
ASSERT_SIGNATURE(cna_video_player_destroy, CNA_Result (*)(CNA_VideoPlayerHandle player));
ASSERT_SIGNATURE(cna_storage_device_show_selector, CNA_Result (*)(CNA_StorageCompletionCallback callback, void* context, CNA_StorageDeviceHandle* out_device));
ASSERT_SIGNATURE(cna_storage_device_show_selector_for_player, CNA_Result (*)(CNA_PlayerIndex player, CNA_StorageCompletionCallback callback, void* context, CNA_StorageDeviceHandle* out_device));
ASSERT_SIGNATURE(cna_storage_device_show_selector_with_space, CNA_Result (*)(int32_t size_in_bytes, int32_t directory_count, CNA_StorageCompletionCallback callback, void* context, CNA_StorageDeviceHandle* out_device));
ASSERT_SIGNATURE(cna_storage_device_show_selector_for_player_with_space, CNA_Result (*)(CNA_PlayerIndex player, int32_t size_in_bytes, int32_t directory_count, CNA_StorageCompletionCallback callback, void* context, CNA_StorageDeviceHandle* out_device));
ASSERT_SIGNATURE(cna_storage_device_get_free_space, CNA_Result (*)(CNA_StorageDeviceHandle device, int64_t* out_free_space));
ASSERT_SIGNATURE(cna_storage_device_get_is_connected, CNA_Result (*)(CNA_StorageDeviceHandle device, CNA_Bool* out_is_connected));
ASSERT_SIGNATURE(cna_storage_device_get_total_space, CNA_Result (*)(CNA_StorageDeviceHandle device, int64_t* out_total_space));
ASSERT_SIGNATURE(cna_storage_device_delete_container, CNA_Result (*)(CNA_StorageDeviceHandle device, CNA_StringView title_name));
ASSERT_SIGNATURE(cna_storage_device_subscribe_device_changed, CNA_Result (*)(CNA_StorageCompletionCallback callback, void* context, CNA_Handle* out_registration));
ASSERT_SIGNATURE(cna_storage_device_destroy, CNA_Result (*)(CNA_StorageDeviceHandle device));
ASSERT_SIGNATURE(cna_storage_container_open, CNA_Result (*)(CNA_StorageDeviceHandle device, CNA_StringView display_name, CNA_StorageCompletionCallback callback, void* context, CNA_StorageContainerHandle* out_container));
ASSERT_SIGNATURE(cna_storage_container_get_display_name_size, CNA_Result (*)(CNA_StorageContainerHandle container, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_storage_container_copy_display_name, CNA_Result (*)(CNA_StorageContainerHandle container, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_storage_container_dispose, CNA_Result (*)(CNA_StorageContainerHandle container));
ASSERT_SIGNATURE(cna_storage_container_subscribe_disposing, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StorageCompletionCallback callback, void* context, CNA_Handle* out_registration));
ASSERT_SIGNATURE(cna_storage_container_unsubscribe_disposing, CNA_Result (*)(CNA_Handle registration));
ASSERT_SIGNATURE(cna_storage_container_create_directory, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView directory));
ASSERT_SIGNATURE(cna_storage_container_directory_exists, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView directory, CNA_Bool* out_exists));
ASSERT_SIGNATURE(cna_storage_container_delete_directory, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView directory));
ASSERT_SIGNATURE(cna_storage_container_file_exists, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file, CNA_Bool* out_exists));
ASSERT_SIGNATURE(cna_storage_container_delete_file, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file));
ASSERT_SIGNATURE(cna_storage_container_get_directory_name_count, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView search_pattern, uint64_t* out_count));
ASSERT_SIGNATURE(cna_storage_container_copy_directory_name, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView search_pattern, uint64_t index, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_storage_container_get_file_name_count, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView search_pattern, uint64_t* out_count));
ASSERT_SIGNATURE(cna_storage_container_copy_file_name, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView search_pattern, uint64_t index, char* destination, uint64_t capacity, uint64_t* out_bytes));
ASSERT_SIGNATURE(cna_storage_container_create_file, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file, CNA_StorageStreamHandle* out_stream));
ASSERT_SIGNATURE(cna_storage_container_open_file, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file, CNA_FileMode mode, CNA_StorageStreamHandle* out_stream));
ASSERT_SIGNATURE(cna_storage_container_open_file_access, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file, CNA_FileMode mode, CNA_FileAccess access, CNA_StorageStreamHandle* out_stream));
ASSERT_SIGNATURE(cna_storage_container_open_file_share, CNA_Result (*)(CNA_StorageContainerHandle container, CNA_StringView file, CNA_FileMode mode, CNA_FileAccess access, CNA_FileShare share, CNA_StorageStreamHandle* out_stream));
ASSERT_SIGNATURE(cna_storage_container_destroy, CNA_Result (*)(CNA_StorageContainerHandle container));
ASSERT_SIGNATURE(cna_storage_stream_read, CNA_Result (*)(CNA_StorageStreamHandle stream, uint8_t* destination, uint64_t capacity, uint64_t* out_read));
ASSERT_SIGNATURE(cna_storage_stream_write, CNA_Result (*)(CNA_StorageStreamHandle stream, const uint8_t* data, uint64_t count));
ASSERT_SIGNATURE(cna_storage_stream_seek, CNA_Result (*)(CNA_StorageStreamHandle stream, int64_t offset, CNA_SeekOrigin origin, int64_t* out_position));
ASSERT_SIGNATURE(cna_storage_stream_get_position, CNA_Result (*)(CNA_StorageStreamHandle stream, int64_t* out_position));
ASSERT_SIGNATURE(cna_storage_stream_get_length, CNA_Result (*)(CNA_StorageStreamHandle stream, int64_t* out_length));
ASSERT_SIGNATURE(cna_storage_stream_set_length, CNA_Result (*)(CNA_StorageStreamHandle stream, int64_t length));
ASSERT_SIGNATURE(cna_storage_stream_get_can_read, CNA_Result (*)(CNA_StorageStreamHandle stream, CNA_Bool* out_can_read));
ASSERT_SIGNATURE(cna_storage_stream_get_can_write, CNA_Result (*)(CNA_StorageStreamHandle stream, CNA_Bool* out_can_write));
ASSERT_SIGNATURE(cna_storage_stream_get_can_seek, CNA_Result (*)(CNA_StorageStreamHandle stream, CNA_Bool* out_can_seek));
ASSERT_SIGNATURE(cna_storage_stream_flush, CNA_Result (*)(CNA_StorageStreamHandle stream));
ASSERT_SIGNATURE(cna_storage_stream_close, CNA_Result (*)(CNA_StorageStreamHandle stream));

#undef ASSERT_SIGNATURE

static uint32_t (*const get_abi_version_function)(void) = cna_get_abi_version;
static CNA_Result (*const error_size_function)(uint64_t*) = cna_error_get_last_message_size;
static CNA_Result (*const error_copy_function)(char*, uint64_t, uint64_t*) = cna_error_copy_last_message;
static CNA_Result (*const game_create_function)(const CNA_GameCreateInfo*, CNA_Handle*) = cna_game_create;
static CNA_Result (*const game_hooks_function)(CNA_Handle, const CNA_GameFrameHooks*) = cna_game_set_frame_hooks_ext;
static CNA_Result (*const game_run_function)(CNA_Handle) = cna_game_run;
static CNA_Result (*const game_run_one_frame_function)(CNA_Handle) = cna_game_run_one_frame;
static CNA_Result (*const gamer_services_set_window_function)(uint64_t) =
    cna_gamer_services_dispatcher_set_window_handle;
static CNA_Result (*const gamer_services_initialize_function)(CNA_Handle) =
    cna_gamer_services_dispatcher_initialize;
static CNA_Result (*const gamer_services_update_function)(void) =
    cna_gamer_services_dispatcher_update;
static CNA_Result (*const game_exit_function)(CNA_Handle) = cna_game_request_exit;
static CNA_Result (*const game_reset_elapsed_time_function)(CNA_Handle) = cna_game_reset_elapsed_time;
static CNA_Result (*const game_suppress_draw_function)(CNA_Handle) = cna_game_suppress_draw;
static CNA_Result (*const game_tick_function)(CNA_Handle) = cna_game_tick;
static CNA_Result (*const game_destroy_function)(CNA_Handle) = cna_game_destroy;
static CNA_Result (*const game_clear_function)(CNA_Handle, CNA_Color) = cna_game_clear;
static CNA_Result (*const game_set_mouse_function)(CNA_Handle, CNA_Bool) = cna_game_set_is_mouse_visible;
static CNA_Result (*const game_get_mouse_function)(CNA_Handle, CNA_Bool*) = cna_game_get_is_mouse_visible;
static CNA_Result (*const game_get_active_function)(CNA_Handle, CNA_Bool*) = cna_game_get_is_active;
static CNA_Result (*const game_set_fixed_function)(CNA_Handle, CNA_Bool) = cna_game_set_is_fixed_time_step;
static CNA_Result (*const game_get_fixed_function)(CNA_Handle, CNA_Bool*) = cna_game_get_is_fixed_time_step;
static CNA_Result (*const game_set_target_time_function)(CNA_Handle, int64_t) =
    cna_game_set_target_elapsed_time_ticks;
static CNA_Result (*const game_get_target_time_function)(CNA_Handle, int64_t*) =
    cna_game_get_target_elapsed_time_ticks;
static CNA_Result (*const game_set_inactive_time_function)(CNA_Handle, int64_t) =
    cna_game_set_inactive_sleep_time_ticks;
static CNA_Result (*const game_get_inactive_time_function)(CNA_Handle, int64_t*) =
    cna_game_get_inactive_sleep_time_ticks;
static CNA_Result (*const window_get_resizing_function)(CNA_Handle, CNA_Bool*) =
    cna_game_window_get_allow_user_resizing;
static CNA_Result (*const window_set_resizing_function)(CNA_Handle, CNA_Bool) =
    cna_game_window_set_allow_user_resizing;
static CNA_Result (*const window_get_bounds_function)(CNA_Handle, CNA_Rectangle*) =
    cna_game_window_get_client_bounds;
static CNA_Result (*const window_get_orientation_function)(CNA_Handle, CNA_DisplayOrientation*) =
    cna_game_window_get_current_orientation;
static CNA_Result (*const window_get_handle_function)(CNA_Handle, uint64_t*) =
    cna_game_window_get_native_handle_ext;
static CNA_Result (*const window_get_screen_size_function)(CNA_Handle, uint64_t*) =
    cna_game_window_get_screen_device_name_size;
static CNA_Result (*const window_copy_screen_function)(CNA_Handle, char*, uint64_t, uint64_t*) =
    cna_game_window_copy_screen_device_name;
static CNA_Result (*const window_set_title_function)(CNA_Handle, CNA_StringView) =
    cna_game_set_window_title;
static CNA_Result (*const window_begin_change_function)(CNA_Handle, CNA_Bool) =
    cna_game_window_begin_screen_device_change;
static CNA_Result (*const window_end_change_function)(CNA_Handle, CNA_StringView, int32_t, int32_t) =
    cna_game_window_end_screen_device_change;
static CNA_Result (*const window_subscribe_function)(
    CNA_Handle, CNA_GameWindowEvent, CNA_GameEventCallback, void*,
    CNA_GameEventRegistrationHandle*) = cna_game_window_subscribe;
static CNA_Result (*const game_unsubscribe_function)(CNA_GameEventRegistrationHandle) =
    cna_game_unsubscribe;
static CNA_Result (*const keyboard_get_state_function)(CNA_Handle, CNA_KeyboardState*) =
    cna_keyboard_get_state;
static CNA_Result (*const keyboard_get_state_for_player_function)(
    CNA_Handle, CNA_PlayerIndex, CNA_KeyboardState*) = cna_keyboard_get_state_for_player;
static CNA_Result (*const gamepad_get_state_function)(
    CNA_Handle, CNA_PlayerIndex, CNA_GamePadState*) = cna_gamepad_get_state;
static CNA_Result (*const gamepad_get_state_with_dead_zone_function)(
    CNA_Handle, CNA_PlayerIndex, CNA_GamePadDeadZone, CNA_GamePadState*) =
    cna_gamepad_get_state_with_dead_zone;
static CNA_Result (*const gamepad_get_capabilities_function)(
    CNA_Handle, CNA_PlayerIndex, CNA_GamePadCapabilities*) = cna_gamepad_get_capabilities;
static CNA_Result (*const gamepad_set_vibration_function)(
    CNA_Handle, CNA_PlayerIndex, float, float, CNA_Bool*) = cna_gamepad_set_vibration;
static CNA_Result (*const mouse_get_state_function)(CNA_Handle, CNA_MouseState*) =
    cna_mouse_get_state;
static CNA_Result (*const mouse_set_position_function)(CNA_Handle, int32_t, int32_t) =
    cna_mouse_set_position;
static CNA_Result (*const mouse_get_window_handle_function)(CNA_Handle, uint64_t*) =
    cna_mouse_get_window_handle;
static CNA_Result (*const mouse_set_window_handle_function)(CNA_Handle, uint64_t) =
    cna_mouse_set_window_handle;
static CNA_Result (*const game_get_graphics_device_function)(CNA_Handle, CNA_Handle*) =
    cna_game_get_graphics_device;
static CNA_Result (*const manager_create_function)(
    CNA_Handle, CNA_GraphicsDeviceManagerHandle*) = cna_graphics_device_manager_create;
static CNA_Result (*const manager_get_profile_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_GraphicsProfile*) =
    cna_graphics_device_manager_get_graphics_profile;
static CNA_Result (*const manager_set_profile_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_GraphicsProfile) =
    cna_graphics_device_manager_set_graphics_profile;
static CNA_Result (*const manager_get_full_screen_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool*) =
    cna_graphics_device_manager_get_is_full_screen;
static CNA_Result (*const manager_set_full_screen_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool) =
    cna_graphics_device_manager_set_is_full_screen;
static CNA_Result (*const manager_get_multisample_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool*) =
    cna_graphics_device_manager_get_prefer_multi_sampling;
static CNA_Result (*const manager_set_multisample_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool) =
    cna_graphics_device_manager_set_prefer_multi_sampling;
static CNA_Result (*const manager_get_back_buffer_format_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_SurfaceFormat*) =
    cna_graphics_device_manager_get_preferred_back_buffer_format;
static CNA_Result (*const manager_set_back_buffer_format_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_SurfaceFormat) =
    cna_graphics_device_manager_set_preferred_back_buffer_format;
static CNA_Result (*const manager_get_back_buffer_width_function)(
    CNA_GraphicsDeviceManagerHandle, int32_t*) =
    cna_graphics_device_manager_get_preferred_back_buffer_width;
static CNA_Result (*const manager_set_back_buffer_width_function)(
    CNA_GraphicsDeviceManagerHandle, int32_t) =
    cna_graphics_device_manager_set_preferred_back_buffer_width;
static CNA_Result (*const manager_get_back_buffer_height_function)(
    CNA_GraphicsDeviceManagerHandle, int32_t*) =
    cna_graphics_device_manager_get_preferred_back_buffer_height;
static CNA_Result (*const manager_set_back_buffer_height_function)(
    CNA_GraphicsDeviceManagerHandle, int32_t) =
    cna_graphics_device_manager_set_preferred_back_buffer_height;
static CNA_Result (*const manager_get_depth_format_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_DepthFormat*) =
    cna_graphics_device_manager_get_preferred_depth_stencil_format;
static CNA_Result (*const manager_set_depth_format_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_DepthFormat) =
    cna_graphics_device_manager_set_preferred_depth_stencil_format;
static CNA_Result (*const manager_get_vsync_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool*) =
    cna_graphics_device_manager_get_synchronize_with_vertical_retrace;
static CNA_Result (*const manager_set_vsync_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool) =
    cna_graphics_device_manager_set_synchronize_with_vertical_retrace;
static CNA_Result (*const manager_get_orientations_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_DisplayOrientation*) =
    cna_graphics_device_manager_get_supported_orientations;
static CNA_Result (*const manager_set_orientations_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_DisplayOrientation) =
    cna_graphics_device_manager_set_supported_orientations;
static CNA_Result (*const manager_apply_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_apply_changes;
static CNA_Result (*const manager_toggle_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_toggle_full_screen;
static CNA_Result (*const manager_create_device_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_create_device;
static CNA_Result (*const manager_begin_draw_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_Bool*) = cna_graphics_device_manager_begin_draw;
static CNA_Result (*const manager_end_draw_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_end_draw;
static CNA_Result (*const manager_dispose_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_dispose;
static CNA_Result (*const manager_subscribe_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_GraphicsDeviceManagerEvent,
    CNA_GameEventCallback, void*, CNA_GameEventRegistrationHandle*) =
    cna_graphics_device_manager_subscribe;
static CNA_Result (*const manager_subscribe_preparing_function)(
    CNA_GraphicsDeviceManagerHandle, CNA_PreparingDeviceSettingsMutatorEXT,
    void*, CNA_GameEventRegistrationHandle*) =
    cna_graphics_device_manager_subscribe_preparing_device_settings_ext;
static CNA_Result (*const manager_destroy_function)(CNA_GraphicsDeviceManagerHandle) =
    cna_graphics_device_manager_destroy;
static CNA_Result (*const adapter_count_function)(CNA_Handle, uint64_t*) =
    cna_graphics_adapter_get_count;
static CNA_Result (*const adapter_info_function)(
    CNA_Handle, uint32_t, CNA_GraphicsAdapterInfo*) = cna_graphics_adapter_get_info;
static CNA_Result (*const adapter_description_function)(
    CNA_Handle, uint32_t, char*, uint64_t, uint64_t*) =
    cna_graphics_adapter_copy_description;
static CNA_Result (*const adapter_device_name_function)(
    CNA_Handle, uint32_t, char*, uint64_t, uint64_t*) =
    cna_graphics_adapter_copy_device_name;
static CNA_Result (*const adapter_current_mode_function)(
    CNA_Handle, uint32_t, CNA_DisplayMode*) =
    cna_graphics_adapter_get_current_display_mode;
static CNA_Result (*const adapter_mode_count_function)(
    CNA_Handle, uint32_t, CNA_Bool, CNA_SurfaceFormat, uint64_t*) =
    cna_graphics_adapter_get_display_mode_count;
static CNA_Result (*const adapter_copy_modes_function)(
    CNA_Handle, uint32_t, CNA_Bool, CNA_SurfaceFormat,
    CNA_DisplayMode*, uint64_t, uint64_t*) = cna_graphics_adapter_copy_display_modes;
static CNA_Result (*const adapter_preferences_function)(
    CNA_Handle, uint32_t, CNA_Bool, CNA_Bool) =
    cna_graphics_adapter_set_device_preferences;
static CNA_Result (*const adapter_profile_function)(
    CNA_Handle, uint32_t, CNA_GraphicsProfile, CNA_Bool*) =
    cna_graphics_adapter_is_profile_supported;
static CNA_Result (*const adapter_render_target_format_function)(
    CNA_Handle, uint32_t, CNA_GraphicsProfile, CNA_SurfaceFormat,
    CNA_DepthFormat, int32_t, CNA_GraphicsFormatSelection*) =
    cna_graphics_adapter_query_render_target_format;
static CNA_Result (*const adapter_backbuffer_format_function)(
    CNA_Handle, uint32_t, CNA_GraphicsProfile, CNA_SurfaceFormat,
    CNA_DepthFormat, int32_t, CNA_GraphicsFormatSelection*) =
    cna_graphics_adapter_query_backbuffer_format;
static CNA_Result (*const adapter_monitor_function)(
    CNA_Handle, uint32_t, CNA_NativeHandleValue*) =
    cna_graphics_adapter_get_native_monitor_handle;
static CNA_Result (*const device_get_disposed_function)(CNA_Handle, CNA_Bool*) =
    cna_graphics_device_get_is_disposed;
static CNA_Result (*const device_get_status_function)(CNA_Handle, CNA_GraphicsDeviceStatus*) =
    cna_graphics_device_get_status;
static CNA_Result (*const device_get_adapter_function)(CNA_Handle, uint32_t*) =
    cna_graphics_device_get_adapter_index;
static CNA_Result (*const device_get_profile_function)(CNA_Handle, CNA_GraphicsProfile*) =
    cna_graphics_device_get_graphics_profile;
static CNA_Result (*const device_set_profile_function)(CNA_Handle, CNA_GraphicsProfile) =
    cna_graphics_device_set_graphics_profile_ext;
static CNA_Result (*const device_get_scissor_function)(CNA_Handle, CNA_Rectangle*) =
    cna_graphics_device_get_scissor_rectangle;
static CNA_Result (*const device_set_scissor_function)(CNA_Handle, CNA_Rectangle) =
    cna_graphics_device_set_scissor_rectangle;
static CNA_Result (*const device_get_viewport_function)(CNA_Handle, CNA_Viewport*) =
    cna_graphics_device_get_viewport;
static CNA_Result (*const device_set_viewport_function)(CNA_Handle, CNA_Viewport) =
    cna_graphics_device_set_viewport;
static CNA_Result (*const device_get_blend_factor_function)(CNA_Handle, CNA_Color*) =
    cna_graphics_device_get_blend_factor;
static CNA_Result (*const device_set_blend_factor_function)(CNA_Handle, CNA_Color) =
    cna_graphics_device_set_blend_factor;
static CNA_Result (*const device_get_blend_state_function)(CNA_Handle, CNA_BlendState*) =
    cna_graphics_device_get_blend_state;
static CNA_Result (*const device_set_blend_state_function)(CNA_Handle, const CNA_BlendState*) =
    cna_graphics_device_set_blend_state;
static CNA_Result (*const device_get_depth_state_function)(CNA_Handle, CNA_DepthStencilState*) =
    cna_graphics_device_get_depth_stencil_state;
static CNA_Result (*const device_set_depth_state_function)(
    CNA_Handle, const CNA_DepthStencilState*) = cna_graphics_device_set_depth_stencil_state;
static CNA_Result (*const device_get_rasterizer_state_function)(
    CNA_Handle, CNA_RasterizerState*) = cna_graphics_device_get_rasterizer_state;
static CNA_Result (*const device_set_rasterizer_state_function)(
    CNA_Handle, const CNA_RasterizerState*) = cna_graphics_device_set_rasterizer_state;
static CNA_Result (*const device_get_sampler_state_function)(
    CNA_Handle, CNA_ShaderStage, uint32_t, CNA_SamplerState*) =
    cna_graphics_device_get_sampler_state;
static CNA_Result (*const device_set_sampler_state_function)(
    CNA_Handle, CNA_ShaderStage, uint32_t, const CNA_SamplerState*) =
    cna_graphics_device_set_sampler_state;
static CNA_Result (*const device_get_texture_function)(
    CNA_Handle, CNA_ShaderStage, uint32_t, CNA_TextureSlotInfo*) =
    cna_graphics_device_get_texture;
static CNA_Result (*const device_set_texture_function)(
    CNA_Handle, CNA_ShaderStage, uint32_t, CNA_Handle) =
    cna_graphics_device_set_texture;
static CNA_Result (*const device_get_multisample_mask_function)(CNA_Handle, int32_t*) =
    cna_graphics_device_get_multi_sample_mask;
static CNA_Result (*const device_set_multisample_mask_function)(CNA_Handle, int32_t) =
    cna_graphics_device_set_multi_sample_mask;
static CNA_Result (*const device_get_reference_stencil_function)(CNA_Handle, int32_t*) =
    cna_graphics_device_get_reference_stencil;
static CNA_Result (*const device_set_reference_stencil_function)(CNA_Handle, int32_t) =
    cna_graphics_device_set_reference_stencil;
static CNA_Result (*const device_get_presentation_function)(
    CNA_Handle, CNA_PresentationParameters*) =
    cna_graphics_device_get_presentation_parameters;
static CNA_Result (*const device_get_display_mode_function)(CNA_Handle, CNA_DisplayMode*) =
    cna_graphics_device_get_display_mode;
static CNA_Result (*const device_get_backbuffer_info_function)(CNA_Handle, CNA_BackBufferInfo*) =
    cna_graphics_device_get_backbuffer_info;
static CNA_Result (*const device_get_backbuffer_data_function)(
    CNA_Handle, const CNA_BackBufferReadback*, CNA_Color*, uint64_t) =
    cna_graphics_device_get_backbuffer_data_window;
static CNA_Result (*const device_clear_function)(
    CNA_Handle, CNA_ClearOptions, CNA_Color, float, int32_t) =
    cna_graphics_device_clear_options;
static CNA_Result (*const device_present_function)(CNA_Handle) =
    cna_graphics_device_present;
static CNA_Result (*const device_reset_function)(CNA_Handle) =
    cna_graphics_device_reset;
static CNA_Result (*const device_reset_parameters_function)(
    CNA_Handle, const CNA_PresentationParameters*, const uint32_t*) =
    cna_graphics_device_reset_with_parameters;
static CNA_Result (*const device_subscribe_function)(
    CNA_Handle, CNA_GraphicsDeviceEvent, CNA_GraphicsDeviceEventCallback,
    void*, CNA_GraphicsDeviceEventRegistrationHandle*) =
    cna_graphics_device_subscribe_event;
static CNA_Result (*const device_subscribe_created_function)(
    CNA_Handle, CNA_GraphicsDeviceResourceCreatedCallback,
    void*, CNA_GraphicsDeviceEventRegistrationHandle*) =
    cna_graphics_device_subscribe_resource_created;
static CNA_Result (*const device_subscribe_destroyed_function)(
    CNA_Handle, CNA_GraphicsDeviceResourceDestroyedCallback,
    void*, CNA_GraphicsDeviceEventRegistrationHandle*) =
    cna_graphics_device_subscribe_resource_destroyed;
static CNA_Result (*const device_unsubscribe_function)(
    CNA_GraphicsDeviceEventRegistrationHandle) = cna_graphics_device_unsubscribe;
static CNA_Result (*const texture_create_function)(
    CNA_Handle, const CNA_Texture2DCreateInfo*, CNA_Handle*) = cna_texture2d_create;
static CNA_Result (*const texture_decode_function)(
    CNA_Handle, const uint8_t*, uint64_t, const CNA_Texture2DDecodeInfo*, CNA_Handle*) =
    cna_texture2d_create_from_encoded_memory;
static CNA_Result (*const texture_info_function)(CNA_Handle, CNA_Texture2DInfo*) =
    cna_texture2d_get_info;
static CNA_Result (*const texture_set_function)(CNA_Handle, const CNA_Color*, uint64_t) =
    cna_texture2d_set_data_rgba8;
static CNA_Result (*const texture_get_function)(CNA_Handle, CNA_Color*, uint64_t, uint64_t*) =
    cna_texture2d_get_data_rgba8;
static CNA_Result (*const texture_set_typed_function)(
    CNA_Handle, CNA_TextureDataType, const CNA_Texture2DTransfer*, const void*, uint64_t) =
    cna_texture2d_set_data;
static CNA_Result (*const texture_get_typed_function)(
    CNA_Handle, CNA_TextureDataType, const CNA_Texture2DTransfer*, void*, uint64_t, uint64_t*) =
    cna_texture2d_get_data;
static CNA_Result (*const texture_encoded_size_function)(
    CNA_Handle, CNA_TextureImageFormat, uint32_t, uint32_t, uint64_t*) =
    cna_texture2d_get_encoded_byte_count;
static CNA_Result (*const texture_copy_encoded_function)(
    CNA_Handle, CNA_TextureImageFormat, uint32_t, uint32_t, uint8_t*, uint64_t, uint64_t*) =
    cna_texture2d_copy_encoded;
static CNA_Result (*const texture_destroy_function)(CNA_Handle) = cna_texture2d_destroy;
static CNA_Result (*const texture_cube_create_function)(
    CNA_Handle, const CNA_TextureCubeCreateInfo*, CNA_Handle*) = cna_texturecube_create;
static CNA_Result (*const texture_cube_info_function)(CNA_Handle, CNA_TextureCubeInfo*) =
    cna_texturecube_get_info;
static CNA_Result (*const texture_cube_set_function)(
    CNA_Handle, const CNA_TextureCubeTransfer*, const CNA_Color*, uint64_t) =
    cna_texturecube_set_data;
static CNA_Result (*const texture_cube_get_function)(
    CNA_Handle, const CNA_TextureCubeTransfer*, CNA_Color*, uint64_t, uint64_t*) =
    cna_texturecube_get_data;
static CNA_Result (*const texture_cube_destroy_function)(CNA_Handle) =
    cna_texturecube_destroy;
static CNA_Result (*const render_target_2d_create_function)(
    CNA_Handle, const CNA_RenderTarget2DCreateInfo*, CNA_Handle*) =
    cna_render_target2d_create;
static CNA_Result (*const render_target_cube_create_function)(
    CNA_Handle, const CNA_RenderTargetCubeCreateInfo*, CNA_Handle*) =
    cna_render_target_cube_create;
static CNA_Result (*const render_target_info_function)(CNA_Handle, CNA_RenderTargetInfo*) =
    cna_render_target_get_info;
static CNA_Result (*const device_set_render_target_2d_function)(CNA_Handle, CNA_Handle) =
    cna_graphics_device_set_render_target2d;
static CNA_Result (*const device_set_render_target_cube_function)(
    CNA_Handle, CNA_Handle, CNA_CubeMapFace) =
    cna_graphics_device_set_render_target_cube;
static CNA_Result (*const device_set_render_targets_function)(
    CNA_Handle, const CNA_RenderTargetBinding*, uint64_t) =
    cna_graphics_device_set_render_targets;
static CNA_Result (*const device_get_render_target_count_function)(CNA_Handle, uint64_t*) =
    cna_graphics_device_get_render_target_count;
static CNA_Result (*const device_copy_render_targets_function)(
    CNA_Handle, CNA_RenderTargetBinding*, uint64_t, uint64_t*) =
    cna_graphics_device_copy_render_targets;
static CNA_Result (*const render_target_destroy_function)(CNA_Handle) =
    cna_render_target_destroy;
static CNA_Result (*const vertex_declaration_create_function)(
    int32_t, const CNA_VertexElement*, uint64_t, CNA_VertexDeclarationHandle*) =
    cna_vertex_declaration_create_with_stride;
static CNA_Result (*const vertex_declaration_destroy_function)(
    CNA_VertexDeclarationHandle) = cna_vertex_declaration_destroy;
static CNA_Result (*const vertex_buffer_create_function)(
    CNA_Handle, const CNA_VertexBufferCreateInfo*, CNA_VertexBufferHandle*) =
    cna_vertex_buffer_create;
static CNA_Result (*const vertex_buffer_info_function)(
    CNA_VertexBufferHandle, CNA_VertexBufferInfo*) = cna_vertex_buffer_get_info;
static CNA_Result (*const vertex_buffer_set_raw_function)(
    CNA_VertexBufferHandle, const void*, uint64_t, uint64_t, uint32_t) =
    cna_vertex_buffer_set_data_raw;
static CNA_Result (*const vertex_buffer_set_raw_at_function)(
    CNA_VertexBufferHandle, uint64_t, const void*, uint64_t, uint64_t, uint32_t) =
    cna_vertex_buffer_set_data_raw_at;
static CNA_Result (*const vertex_buffer_get_raw_function)(
    CNA_VertexBufferHandle, uint64_t, void*, uint64_t, uint64_t, uint32_t) =
    cna_vertex_buffer_get_data_raw;
static CNA_Result (*const vertex_buffer_destroy_function)(CNA_VertexBufferHandle) =
    cna_vertex_buffer_destroy;
static CNA_Result (*const index_buffer_create_function)(
    CNA_Handle, const CNA_IndexBufferCreateInfo*, CNA_IndexBufferHandle*) =
    cna_index_buffer_create;
static CNA_Result (*const index_buffer_info_function)(
    CNA_IndexBufferHandle, CNA_IndexBufferInfo*) = cna_index_buffer_get_info;
static CNA_Result (*const index_buffer_set_function)(
    CNA_IndexBufferHandle, const CNA_IndexBufferTransfer*, const void*, uint64_t) =
    cna_index_buffer_set_data;
static CNA_Result (*const index_buffer_set_at_function)(
    CNA_IndexBufferHandle, uint64_t, const CNA_IndexBufferTransfer*, const void*, uint64_t) =
    cna_index_buffer_set_data_at;
static CNA_Result (*const index_buffer_get_function)(
    CNA_IndexBufferHandle, const CNA_IndexBufferTransfer*, void*, uint64_t, uint64_t*) =
    cna_index_buffer_get_data;
static CNA_Result (*const index_buffer_destroy_function)(CNA_IndexBufferHandle) =
    cna_index_buffer_destroy;
static CNA_Result (*const device_set_vertex_buffer_function)(
    CNA_Handle, CNA_VertexBufferHandle) = cna_graphics_device_set_vertex_buffer;
static CNA_Result (*const device_set_vertex_buffer_offset_function)(
    CNA_Handle, CNA_VertexBufferHandle, int32_t) =
    cna_graphics_device_set_vertex_buffer_offset;
static CNA_Result (*const device_set_vertex_buffers_function)(
    CNA_Handle, const CNA_VertexBufferBinding*, uint64_t) =
    cna_graphics_device_set_vertex_buffers;
static CNA_Result (*const device_get_vertex_buffer_count_function)(CNA_Handle, uint64_t*) =
    cna_graphics_device_get_vertex_buffer_count;
static CNA_Result (*const device_copy_vertex_buffers_function)(
    CNA_Handle, CNA_VertexBufferBinding*, uint64_t, uint64_t*) =
    cna_graphics_device_copy_vertex_buffers;
static CNA_Result (*const device_set_index_buffer_function)(
    CNA_Handle, CNA_IndexBufferHandle) = cna_graphics_device_set_index_buffer;
static CNA_Result (*const device_get_index_buffer_function)(
    CNA_Handle, CNA_IndexBufferHandle*) = cna_graphics_device_get_index_buffer;
static CNA_Result (*const device_draw_primitives_function)(
    CNA_Handle, CNA_PrimitiveType, int32_t, int32_t) =
    cna_graphics_device_draw_primitives;
static CNA_Result (*const device_draw_indexed_primitives_function)(
    CNA_Handle, CNA_PrimitiveType, int32_t, int32_t, int32_t, int32_t, int32_t) =
    cna_graphics_device_draw_indexed_primitives;
static CNA_Result (*const device_draw_instanced_primitives_function)(
    CNA_Handle, CNA_PrimitiveType, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t) =
    cna_graphics_device_draw_instanced_primitives;
static CNA_Result (*const device_draw_user_primitives_function)(
    CNA_Handle, const CNA_UserPrimitives*) = cna_graphics_device_draw_user_primitives;
static CNA_Result (*const device_draw_user_indexed_primitives_function)(
    CNA_Handle, const CNA_UserPrimitives*, const CNA_UserIndices*) =
    cna_graphics_device_draw_user_indexed_primitives;
static CNA_Result (*const sprite_batch_create_function)(CNA_Handle, CNA_Handle*) =
    cna_sprite_batch_create;
static CNA_Result (*const sprite_batch_begin_function)(
    CNA_Handle, const CNA_SpriteBatchBeginInfo*) = cna_sprite_batch_begin;
static CNA_Result (*const sprite_batch_begin_states_function)(
    CNA_Handle, CNA_SpriteSortMode, const CNA_BlendState*, const CNA_SamplerState*,
    const CNA_DepthStencilState*, const CNA_RasterizerState*) =
    cna_sprite_batch_begin_with_states;
static CNA_Result (*const sprite_batch_submit_function)(
    CNA_Handle, const CNA_SpriteCommand*, uint64_t) = cna_sprite_batch_submit_many;
static CNA_Result (*const sprite_batch_submit_scaled_function)(
    CNA_Handle, const CNA_SpriteScaledCommand*, uint64_t) = cna_sprite_batch_submit_scaled_many;
static CNA_Result (*const sprite_batch_draw_string_function)(
    CNA_Handle, const CNA_SpriteTextCommand*) = cna_sprite_batch_draw_string;
static CNA_Result (*const sprite_batch_end_function)(CNA_Handle) = cna_sprite_batch_end;
static CNA_Result (*const sprite_batch_destroy_function)(CNA_Handle) = cna_sprite_batch_destroy;
static CNA_Result (*const content_manager_create_function)(
    CNA_Handle, const CNA_ContentManagerCreateInfo*, CNA_Handle*) = cna_content_manager_create;
static CNA_Result (*const content_manager_set_root_function)(CNA_Handle, CNA_StringView) =
    cna_content_manager_set_root_directory;
static CNA_Result (*const content_manager_load_texture_function)(
    CNA_Handle, CNA_StringView, CNA_Handle*) = cna_content_manager_load_texture2d;
static CNA_Result (*const content_manager_load_font_function)(
    CNA_Handle, CNA_StringView, CNA_Handle*, CNA_Handle*) =
    cna_content_manager_load_sprite_font;
static CNA_Result (*const content_manager_unload_function)(CNA_Handle) =
    cna_content_manager_unload;
static CNA_Result (*const content_manager_register_builtin_function)(CNA_Handle) =
    cna_content_manager_register_builtin_loaders;
static CNA_Result (*const content_manager_destroy_function)(CNA_Handle) =
    cna_content_manager_destroy;
static CNA_Result (*const sprite_font_info_function)(CNA_Handle, CNA_SpriteFontInfo*) =
    cna_sprite_font_get_info;
static CNA_Result (*const sprite_font_characters_function)(
    CNA_Handle, CNA_Char16*, uint64_t, uint64_t*) = cna_sprite_font_copy_characters;
static CNA_Result (*const sprite_font_default_function)(CNA_Handle, CNA_Bool, CNA_Char16) =
    cna_sprite_font_set_default_character;
static CNA_Result (*const sprite_font_line_spacing_function)(CNA_Handle, int32_t) =
    cna_sprite_font_set_line_spacing;
static CNA_Result (*const sprite_font_spacing_function)(CNA_Handle, float) =
    cna_sprite_font_set_spacing;
static CNA_Result (*const sprite_font_measure_function)(
    CNA_Handle, CNA_StringView, CNA_Vector2*) = cna_sprite_font_measure_utf8;
static CNA_Result (*const sprite_font_destroy_function)(CNA_Handle) = cna_sprite_font_destroy;

int cna_java_abi_probe(void)
{
    return get_abi_version_function != NULL && error_size_function != NULL && error_copy_function != NULL &&
        game_create_function != NULL && game_hooks_function != NULL && game_run_function != NULL &&
        game_run_one_frame_function != NULL &&
        gamer_services_set_window_function != NULL &&
        gamer_services_initialize_function != NULL &&
        gamer_services_update_function != NULL && game_exit_function != NULL &&
        game_reset_elapsed_time_function != NULL && game_suppress_draw_function != NULL &&
        game_tick_function != NULL && game_destroy_function != NULL && game_clear_function != NULL &&
        game_set_mouse_function != NULL && game_get_mouse_function != NULL &&
        game_get_active_function != NULL && game_set_fixed_function != NULL &&
        game_get_fixed_function != NULL && game_set_target_time_function != NULL &&
        game_get_target_time_function != NULL && game_set_inactive_time_function != NULL &&
        game_get_inactive_time_function != NULL && window_get_resizing_function != NULL &&
        window_set_resizing_function != NULL && window_get_bounds_function != NULL &&
        window_get_orientation_function != NULL && window_get_handle_function != NULL &&
        window_get_screen_size_function != NULL && window_copy_screen_function != NULL &&
        window_set_title_function != NULL && window_begin_change_function != NULL &&
        window_end_change_function != NULL && window_subscribe_function != NULL &&
        game_unsubscribe_function != NULL && keyboard_get_state_function != NULL &&
        keyboard_get_state_for_player_function != NULL && gamepad_get_state_function != NULL &&
        gamepad_get_state_with_dead_zone_function != NULL &&
        gamepad_get_capabilities_function != NULL && gamepad_set_vibration_function != NULL &&
        mouse_get_state_function != NULL &&
        mouse_set_position_function != NULL && mouse_get_window_handle_function != NULL &&
        mouse_set_window_handle_function != NULL && game_get_graphics_device_function != NULL &&
        manager_create_function != NULL && manager_get_profile_function != NULL &&
        manager_set_profile_function != NULL && manager_get_full_screen_function != NULL &&
        manager_set_full_screen_function != NULL && manager_get_multisample_function != NULL &&
        manager_set_multisample_function != NULL &&
        manager_get_back_buffer_format_function != NULL &&
        manager_set_back_buffer_format_function != NULL &&
        manager_get_back_buffer_width_function != NULL &&
        manager_set_back_buffer_width_function != NULL &&
        manager_get_back_buffer_height_function != NULL &&
        manager_set_back_buffer_height_function != NULL &&
        manager_get_depth_format_function != NULL && manager_set_depth_format_function != NULL &&
        manager_get_vsync_function != NULL && manager_set_vsync_function != NULL &&
        manager_get_orientations_function != NULL && manager_set_orientations_function != NULL &&
        manager_apply_function != NULL && manager_toggle_function != NULL &&
        manager_create_device_function != NULL && manager_begin_draw_function != NULL &&
        manager_end_draw_function != NULL && manager_dispose_function != NULL &&
        manager_subscribe_function != NULL && manager_subscribe_preparing_function != NULL &&
        manager_destroy_function != NULL &&
        adapter_count_function != NULL && adapter_info_function != NULL &&
        adapter_description_function != NULL && adapter_device_name_function != NULL &&
        adapter_current_mode_function != NULL && adapter_mode_count_function != NULL &&
        adapter_copy_modes_function != NULL && adapter_preferences_function != NULL &&
        adapter_profile_function != NULL && adapter_render_target_format_function != NULL &&
        adapter_backbuffer_format_function != NULL && adapter_monitor_function != NULL &&
        device_get_disposed_function != NULL && device_get_status_function != NULL &&
        device_get_adapter_function != NULL && device_get_profile_function != NULL &&
        device_set_profile_function != NULL && device_get_scissor_function != NULL &&
        device_set_scissor_function != NULL && device_get_viewport_function != NULL &&
        device_set_viewport_function != NULL && device_get_blend_factor_function != NULL &&
        device_set_blend_factor_function != NULL && device_get_blend_state_function != NULL &&
        device_set_blend_state_function != NULL && device_get_depth_state_function != NULL &&
        device_set_depth_state_function != NULL &&
        device_get_rasterizer_state_function != NULL &&
        device_set_rasterizer_state_function != NULL &&
        device_get_sampler_state_function != NULL && device_set_sampler_state_function != NULL &&
        device_get_texture_function != NULL && device_set_texture_function != NULL &&
        device_get_multisample_mask_function != NULL &&
        device_set_multisample_mask_function != NULL &&
        device_get_reference_stencil_function != NULL &&
        device_set_reference_stencil_function != NULL &&
        device_get_presentation_function != NULL && device_get_display_mode_function != NULL &&
        device_get_backbuffer_info_function != NULL &&
        device_get_backbuffer_data_function != NULL &&
        device_clear_function != NULL && device_present_function != NULL &&
        device_reset_function != NULL && device_reset_parameters_function != NULL &&
        device_subscribe_function != NULL && device_subscribe_created_function != NULL &&
        device_subscribe_destroyed_function != NULL && device_unsubscribe_function != NULL &&
        texture_create_function != NULL && texture_decode_function != NULL &&
        texture_info_function != NULL && texture_set_function != NULL &&
        texture_get_function != NULL && texture_set_typed_function != NULL &&
        texture_get_typed_function != NULL && texture_encoded_size_function != NULL &&
        texture_copy_encoded_function != NULL && texture_destroy_function != NULL &&
        texture_cube_create_function != NULL && texture_cube_info_function != NULL &&
        texture_cube_set_function != NULL && texture_cube_get_function != NULL &&
        texture_cube_destroy_function != NULL && render_target_2d_create_function != NULL &&
        render_target_cube_create_function != NULL && render_target_info_function != NULL &&
        device_set_render_target_2d_function != NULL &&
        device_set_render_target_cube_function != NULL &&
        device_set_render_targets_function != NULL &&
        device_get_render_target_count_function != NULL &&
        device_copy_render_targets_function != NULL &&
        render_target_destroy_function != NULL &&
        vertex_declaration_create_function != NULL &&
        vertex_declaration_destroy_function != NULL &&
        vertex_buffer_create_function != NULL && vertex_buffer_info_function != NULL &&
        vertex_buffer_set_raw_function != NULL &&
        vertex_buffer_set_raw_at_function != NULL &&
        vertex_buffer_get_raw_function != NULL && vertex_buffer_destroy_function != NULL &&
        index_buffer_create_function != NULL && index_buffer_info_function != NULL &&
        index_buffer_set_function != NULL && index_buffer_set_at_function != NULL &&
        index_buffer_get_function != NULL && index_buffer_destroy_function != NULL &&
        device_set_vertex_buffer_function != NULL &&
        device_set_vertex_buffer_offset_function != NULL &&
        device_set_vertex_buffers_function != NULL &&
        device_get_vertex_buffer_count_function != NULL &&
        device_copy_vertex_buffers_function != NULL &&
        device_set_index_buffer_function != NULL &&
        device_get_index_buffer_function != NULL &&
        device_draw_primitives_function != NULL &&
        device_draw_indexed_primitives_function != NULL &&
        device_draw_instanced_primitives_function != NULL &&
        device_draw_user_primitives_function != NULL &&
        device_draw_user_indexed_primitives_function != NULL &&
        sprite_batch_create_function != NULL && sprite_batch_begin_function != NULL &&
        sprite_batch_begin_states_function != NULL &&
        sprite_batch_submit_function != NULL && sprite_batch_submit_scaled_function != NULL &&
        sprite_batch_draw_string_function != NULL &&
        sprite_batch_end_function != NULL && sprite_batch_destroy_function != NULL &&
        content_manager_create_function != NULL &&
        content_manager_set_root_function != NULL &&
        content_manager_load_texture_function != NULL &&
        content_manager_load_font_function != NULL &&
        content_manager_unload_function != NULL &&
        content_manager_register_builtin_function != NULL &&
        content_manager_destroy_function != NULL &&
        sprite_font_info_function != NULL && sprite_font_characters_function != NULL &&
        sprite_font_default_function != NULL && sprite_font_line_spacing_function != NULL &&
        sprite_font_spacing_function != NULL && sprite_font_measure_function != NULL &&
        sprite_font_destroy_function != NULL ? 0 : 1;
}
