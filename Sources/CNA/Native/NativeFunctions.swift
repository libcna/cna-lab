// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

/// The admitted CNA C ABI window.
///
/// The rule is CNA's own, taken from `docs/c-api/ABI_VERSIONING.md` and from
/// the installed package's `SameMajorVersion` compatibility file rather than
/// invented here: *a consumer must reject a different major and may require a
/// minimum minor.* Under ABI `0.x` an incompatible change is what moves the
/// minor, so the minimum minor is the generation this binding is qualified
/// against; a later minor is admitted by the published rule, and every route it
/// might have removed still has to resolve by name before the runtime starts.
internal enum NativeABI {
    /// Packed as CNA packs it: bits 31..16 major, 15..8 minor, 7..0 patch.
    static func encode(major: UInt32, minor: UInt32, patch: UInt32) -> UInt32 {
        ((major & 0xFFFF) << 16) | ((minor & 0xFF) << 8) | (patch & 0xFF)
    }

    static func major(_ encoded: UInt32) -> UInt32 { (encoded >> 16) & 0xFFFF }
    static func minor(_ encoded: UInt32) -> UInt32 { (encoded >> 8) & 0xFF }
    static func patch(_ encoded: UInt32) -> UInt32 { encoded & 0xFF }

    static func describe(_ encoded: UInt32) -> String {
        "\(major(encoded)).\(minor(encoded)).\(patch(encoded))"
    }

    /// The only admitted major. A different major is rejected outright.
    static let admittedMajor: UInt32 = 0
    /// The lowest admitted minor within `admittedMajor`.
    static let minimumMinor: UInt32 = 35
    /// The exact version this binding's native gates were qualified against.
    static let qualifiedVersion: UInt32 = encode(major: 0, minor: 35, patch: 0)

    static func admits(_ encoded: UInt32) -> Bool {
        major(encoded) == admittedMajor && minor(encoded) >= minimumMinor
    }

    /// The admission window, spelled the way the diagnostic reports it.
    static var admittedDescription: String {
        "major \(admittedMajor) with minor \(minimumMinor) or later (qualified against \(describe(qualifiedVersion)))"
    }
}

internal final class NativeFunctions {
    // Every route has a type of its own. Two routes never share one, even when
    // their C prototypes coincide, so tools/native_abi can pair each stored
    // property with exactly one canonical symbol instead of accepting any
    // structurally compatible alias.
    typealias GetAbiVersionRoute = @convention(c) () -> UInt32
    typealias ErrorGetLastMessageSizeRoute = @convention(c) (UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ErrorCopyLastMessageRoute = @convention(c) (UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameCreateRoute = @convention(c) (UnsafePointer<CNASwift_GameCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameSetFrameHooksExtRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_GameFrameHooks>?) -> UInt32
    typealias FrameworkDispatcherUpdateRoute = @convention(c) (UInt64) -> UInt32
    typealias TitleContainerReadExtRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameRunRoute = @convention(c) (UInt64) -> UInt32
    typealias GameRunOneFrameRoute = @convention(c) (UInt64) -> UInt32
    typealias GameRequestExitRoute = @convention(c) (UInt64) -> UInt32
    typealias GameDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GameGetGraphicsDeviceRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceManagerCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceManagerApplyChangesRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceManagerSetGraphicsProfileRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias GraphicsDeviceManagerSetIsFullScreenRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GraphicsDeviceManagerSetPreferMultiSamplingRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GraphicsDeviceManagerSetPreferredBackBufferFormatRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias GraphicsDeviceManagerSetPreferredBackBufferWidthRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias GraphicsDeviceManagerSetPreferredBackBufferHeightRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias GraphicsDeviceManagerSetPreferredDepthStencilFormatRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias GraphicsDeviceManagerSetSynchronizeWithVerticalRetraceRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GraphicsDeviceManagerSetSupportedOrientationsRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias GraphicsDeviceManagerDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceGetViewportRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Viewport>?) -> UInt32
    typealias Texture2dCreateFromEncodedMemoryRoute = @convention(c) (UInt64, UnsafePointer<UInt8>?, UInt64, UnsafePointer<CNASwift_Texture2DDecodeInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture2dGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Texture2DInfo>?) -> UInt32
    typealias Texture2dCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Texture2DCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture2dDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias Texture2dSetDataRoute = @convention(c) (UInt64, UInt32, UnsafePointer<CNASwift_Texture2DTransfer>?, UnsafeRawPointer?, UInt64) -> UInt32
    typealias Texture2dGetDataRoute = @convention(c) (UInt64, UInt32, UnsafePointer<CNASwift_Texture2DTransfer>?, UnsafeMutableRawPointer?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture2dGetEncodedByteCountRoute = @convention(c) (UInt64, UInt32, UInt32, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture2dCopyEncodedRoute = @convention(c) (UInt64, UInt32, UInt32, UInt32, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture2dCreateCpuOnlyRgba8Route = @convention(c) (UInt32, UInt32, UInt32, UnsafePointer<CNASwift_Color>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VertexDeclarationCreateWithStrideRoute = @convention(c) (Int32, UnsafePointer<CNASwift_VertexElement>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VertexDeclarationDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias VertexBufferCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_VertexBufferCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VertexBufferDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias VertexBufferSetDataRawAtRoute = @convention(c) (UInt64, UInt64, UnsafeRawPointer?, UInt64, UInt64, UInt32) -> UInt32
    typealias VertexBufferGetDataRawRoute = @convention(c) (UInt64, UInt64, UnsafeMutableRawPointer?, UInt64, UInt64, UInt32) -> UInt32
    typealias IndexBufferCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_IndexBufferCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias IndexBufferDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias IndexBufferSetDataAtRoute = @convention(c) (UInt64, UInt64, UnsafePointer<CNASwift_IndexBufferTransfer>?, UnsafeRawPointer?, UInt64) -> UInt32
    typealias IndexBufferGetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_IndexBufferTransfer>?, UnsafeMutableRawPointer?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VertexBufferSetDataRawAtWithOptionsRoute = @convention(c) (UInt64, UInt64, UnsafeRawPointer?, UInt64, UInt64, UInt32, UInt32) -> UInt32
    typealias VertexBufferSubscribeContentLostRoute = @convention(c) (UInt64, CNASwift_VertexBufferContentLostCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VertexBufferUnsubscribeContentLostRoute = @convention(c) (UInt64) -> UInt32
    typealias IndexBufferSetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_IndexBufferTransfer>?, UnsafeRawPointer?, UInt64) -> UInt32
    typealias IndexBufferSubscribeContentLostRoute = @convention(c) (UInt64, CNASwift_IndexBufferContentLostCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias IndexBufferUnsubscribeContentLostRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceGetGraphicsProfileRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias GraphicsDeviceGetBackbufferDataWindowRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_BackBufferReadback>?, UnsafeMutablePointer<CNASwift_Color>?, UInt64) -> UInt32
    typealias GraphicsDeviceSetVertexBuffersRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_VertexBufferBinding>?, UInt64) -> UInt32
    typealias GraphicsDeviceSetIndexBufferRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias TexturecubeCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_TextureCubeCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias TexturecubeDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias TexturecubeGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_TextureCubeInfo>?) -> UInt32
    typealias TexturecubeSetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_TextureCubeTransfer>?, UnsafePointer<CNASwift_Color>?, UInt64) -> UInt32
    typealias TexturecubeGetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_TextureCubeTransfer>?, UnsafeMutablePointer<CNASwift_Color>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture3dCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Texture3DCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias Texture3dDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias Texture3dGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Texture3DInfo>?) -> UInt32
    typealias Texture3dSetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Texture3DTransfer>?, UnsafePointer<CNASwift_Color>?, UInt64) -> UInt32
    typealias Texture3dGetDataRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Texture3DTransfer>?, UnsafeMutablePointer<CNASwift_Color>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SpriteBatchCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SpriteBatchBeginWithEffectRoute = @convention(c) (UInt64, UInt32, UnsafePointer<CNASwift_BlendState>?, UnsafePointer<CNASwift_SamplerState>?, UnsafePointer<CNASwift_DepthStencilState>?, UnsafePointer<CNASwift_RasterizerState>?, UInt64, UnsafePointer<CNASwift_Matrix>?) -> UInt32
    typealias SpriteBatchBeginWithStatesRoute = @convention(c) (UInt64, UInt32, UnsafePointer<CNASwift_BlendState>?, UnsafePointer<CNASwift_SamplerState>?, UnsafePointer<CNASwift_DepthStencilState>?, UnsafePointer<CNASwift_RasterizerState>?) -> UInt32
    typealias SpriteFontCreateRoute = @convention(c) (UnsafePointer<CNASwift_SpriteFontCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SpriteFontDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias SpriteFontGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_SpriteFontInfo>?) -> UInt32
    typealias SpriteFontCopyCharactersRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt16>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SpriteFontCopyGlyphsRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_SpriteFontGlyph>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SpriteFontSetDefaultCharacterRoute = @convention(c) (UInt64, UInt8, UInt16) -> UInt32
    typealias SpriteFontSetLineSpacingRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias SpriteFontSetSpacingRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SpriteBatchDrawStringRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_SpriteTextCommand>?) -> UInt32
    typealias SpriteBatchSubmitScaledManyRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_SpriteScaledCommand>?, UInt64) -> UInt32
    typealias SpriteBatchSubmitManyRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_SpriteCommand>?, UInt64) -> UInt32
    typealias SpriteBatchEndRoute = @convention(c) (UInt64) -> UInt32
    typealias SpriteBatchDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsAdapterGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsAdapterGetInfoRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CNASwift_GraphicsAdapterInfo>?) -> UInt32
    typealias GraphicsAdapterCopyDescriptionRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsAdapterCopyDeviceNameRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsAdapterGetCurrentDisplayModeRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CNASwift_DisplayMode>?) -> UInt32
    typealias GraphicsAdapterGetDisplayModeCountRoute = @convention(c) (UInt64, UInt32, UInt8, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsAdapterCopyDisplayModesRoute = @convention(c) (UInt64, UInt32, UInt8, UInt32, UnsafeMutablePointer<CNASwift_DisplayMode>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsAdapterSetDevicePreferencesRoute = @convention(c) (UInt64, UInt32, UInt8, UInt8) -> UInt32
    typealias GraphicsAdapterIsProfileSupportedRoute = @convention(c) (UInt64, UInt32, UInt32, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GraphicsAdapterQueryBackbufferFormatRoute = @convention(c) (UInt64, UInt32, UInt32, UInt32, UInt32, Int32, UnsafeMutablePointer<CNASwift_GraphicsFormatSelection>?) -> UInt32
    typealias GraphicsAdapterQueryRenderTargetFormatRoute = @convention(c) (UInt64, UInt32, UInt32, UInt32, UInt32, Int32, UnsafeMutablePointer<CNASwift_GraphicsFormatSelection>?) -> UInt32
    typealias GameWindowGetTitleSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameWindowCopyTitleRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameWindowGetScreenDeviceNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameWindowCopyScreenDeviceNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameWindowGetNativeHandleExtRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameWindowGetClientBoundsRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Rectangle>?) -> UInt32
    typealias GameWindowGetCurrentOrientationRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias GameWindowGetAllowUserResizingRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GameWindowSetAllowUserResizingRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GameWindowBeginScreenDeviceChangeRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GameWindowEndScreenDeviceChangeRoute = @convention(c) (UInt64, CNASwift_StringView, Int32, Int32) -> UInt32
    typealias GameSetWindowTitleRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias ContentManagerCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_ContentManagerCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ContentManagerDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ContentManagerUnloadRoute = @convention(c) (UInt64) -> UInt32
    typealias ContentManagerGetRootDirectorySizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ContentManagerCopyRootDirectoryRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ContentManagerLoadTexture2dRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MouseGetStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_MouseState>?) -> UInt32
    typealias MouseSetPositionRoute = @convention(c) (UInt64, Int32, Int32) -> UInt32
    typealias MouseSetWindowHandleRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias KeyboardGetStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_KeyboardState>?) -> UInt32
    typealias KeyboardGetStateForPlayerRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CNASwift_KeyboardState>?) -> UInt32
    typealias GamepadGetStateRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CNASwift_GamePadState>?) -> UInt32
    typealias GamepadGetStateWithDeadZoneRoute = @convention(c) (UInt64, UInt32, UInt32, UnsafeMutablePointer<CNASwift_GamePadState>?) -> UInt32
    typealias GamepadGetCapabilitiesRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CNASwift_GamePadCapabilities>?) -> UInt32
    typealias GamepadSetVibrationRoute = @convention(c) (UInt64, UInt32, Float, Float, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias TextureGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_TextureInfo>?) -> UInt32
    typealias RenderTarget2dCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_RenderTarget2DCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectCreatePcm16Route = @convention(c) (UInt64, UnsafePointer<CNASwift_SoundEffectCreateInfo>?, UnsafePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectCreatePcm16RangeExtRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_SoundEffectCreateInfo>?, UnsafePointer<UInt8>?, UInt64, Int32, Int32, Int32, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectCreateFromEncodedExtRoute = @convention(c) (UInt64, UnsafePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundEffectGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundEffectGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectSetNameRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias SoundEffectGetDurationTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias SoundEffectPlayRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundEffectPlayWithSettingsRoute = @convention(c) (UInt64, Float, Float, Float, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundEffectCreateInstanceRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundEffectGetMasterVolumeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SoundEffectSetMasterVolumeRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectGetDistanceScaleRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SoundEffectSetDistanceScaleRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectGetDopplerScaleRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SoundEffectSetDopplerScaleRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectGetSpeedOfSoundRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SoundEffectSetSpeedOfSoundRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectGetSampleSizeInBytesRoute = @convention(c) (Int64, Int32, UInt32, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SoundEffectGetSampleDurationTicksRoute = @convention(c) (Int32, Int32, UInt32, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias SoundEffectInstanceGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_SoundEffectInstanceInfo>?) -> UInt32
    typealias SoundEffectInstanceSetVolumeRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectInstanceSetPitchRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectInstanceSetPanRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SoundEffectInstanceSetIsLoopedRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias SoundEffectInstancePlayRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundEffectInstancePauseRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundEffectInstanceResumeRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundEffectInstanceStopRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias SoundEffectInstanceGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundEffectInstanceDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundEffectInstanceApply3dRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_AudioListener>?, UnsafePointer<CNASwift_AudioEmitter>?) -> UInt32
    typealias SoundEffectInstanceApply3dMultiExtRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_AudioListener>?, UInt64, UnsafePointer<CNASwift_AudioEmitter>?) -> UInt32
    typealias DynamicSoundEffectInstanceCreateRoute = @convention(c) (UInt64, Int32, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias DynamicSoundEffectInstanceSubmitBufferRoute = @convention(c) (UInt64, UnsafePointer<UInt8>?, UInt64, Int32, Int32) -> UInt32
    typealias DynamicSoundEffectInstanceGetPendingBufferCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias DynamicSoundEffectInstanceGetSampleDurationTicksRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias DynamicSoundEffectInstanceGetSampleSizeInBytesRoute = @convention(c) (UInt64, Int64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias DynamicSoundEffectInstanceSubscribeBufferNeededRoute = @convention(c) (UInt64, CNASwift_AudioEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioUnsubscribeExtRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioEngineCreateRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineCreateWithRendererRoute = @convention(c) (UInt64, CNASwift_StringView, Int64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioEngineGetRendererCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineGetRendererFriendlyNameSizeRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineCopyRendererFriendlyNameRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineGetRendererIdSizeRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineCopyRendererIdRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioEngineGetGlobalVariableRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<Float>?) -> UInt32
    typealias AudioEngineSetGlobalVariableRoute = @convention(c) (UInt64, CNASwift_StringView, Float) -> UInt32
    typealias AudioEngineUpdateRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioEngineGetCategoryRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioCategoryDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioCategoryGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioCategoryCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AudioCategoryPauseRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioCategoryResumeRoute = @convention(c) (UInt64) -> UInt32
    typealias AudioCategorySetVolumeRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias AudioCategoryStopRoute = @convention(c) (UInt64, CNASwift_AudioStopOptions) -> UInt32
    typealias AudioCategoryEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AudioCategoryGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias WaveBankCreateRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias WaveBankCreateStreamingRoute = @convention(c) (UInt64, CNASwift_StringView, Int32, Int16, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias WaveBankDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias WaveBankGetIsInUseRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias WaveBankGetIsPreparedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundBankCreateRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundBankDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias SoundBankGetIsInUseRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SoundBankGetCueRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SoundBankPlayCueRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias SoundBankPlayCue3dRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafePointer<CNASwift_AudioListener>?, UnsafePointer<CNASwift_AudioEmitter>?) -> UInt32
    typealias CueDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias CueGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_CueInfo>?) -> UInt32
    typealias CueGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias CueCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias CueApply3dRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_AudioListener>?, UnsafePointer<CNASwift_AudioEmitter>?) -> UInt32
    typealias CueGetVariableRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<Float>?) -> UInt32
    typealias CueSetVariableRoute = @convention(c) (UInt64, CNASwift_StringView, Float) -> UInt32
    typealias CuePlayRoute = @convention(c) (UInt64) -> UInt32
    typealias CuePauseRoute = @convention(c) (UInt64) -> UInt32
    typealias CueResumeRoute = @convention(c) (UInt64) -> UInt32
    typealias CueStopRoute = @convention(c) (UInt64, CNASwift_AudioStopOptions) -> UInt32
    typealias MicrophoneGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MicrophoneGetDefaultIndexExtRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MicrophoneGetNameSizeAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MicrophoneCopyNameAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MicrophoneGetBufferDurationTicksAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias MicrophoneSetBufferDurationTicksAtRoute = @convention(c) (UInt64, UInt64, Int64) -> UInt32
    typealias MicrophoneGetIsHeadsetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MicrophoneGetSampleRateAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias MicrophoneGetStateAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<CNASwift_MicrophoneState>?) -> UInt32
    typealias MicrophoneStartAtRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias MicrophoneStopAtRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias MicrophoneGetDataAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MicrophoneGetSampleDurationTicksAtRoute = @convention(c) (UInt64, UInt64, Int32, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias MicrophoneGetSampleSizeInBytesAtRoute = @convention(c) (UInt64, UInt64, Int64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias MicrophoneSubscribeBufferReadyAtRoute = @convention(c) (UInt64, UInt64, CNASwift_AudioEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongCreateFromUriRoute = @convention(c) (UInt64, CNASwift_StringView, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias SongDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias SongGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongGetDurationRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias SongGetIsRatedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongGetRatingRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SongGetPlayCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SongGetTrackNumberRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SongGetIsProtectedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SongEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SongCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias SongCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ArtistCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ArtistCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ArtistCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ArtistCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias ArtistCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias AlbumCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias AlbumCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias AlbumCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GenreCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GenreCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias GenreCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GenreCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias GenreCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ArtistGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ArtistCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ArtistGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ArtistDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias ArtistDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ArtistEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ArtistGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ArtistGetSongsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias AlbumDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias AlbumEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias AlbumGetSongsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GenreGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GenreCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GenreGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GenreDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias GenreDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GenreEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GenreGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias GenreGetSongsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ArtistGetAlbumsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GenreGetAlbumsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumGetArtistRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumGetGenreRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumGetDurationRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias AlbumGetHasArtRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlbumGetArtSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumCopyArtRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumGetThumbnailSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlbumCopyThumbnailRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SongGetArtistRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongGetAlbumRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SongGetGenreRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaLibraryCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32

    // ---- Foundation 103: Model ----
    typealias VertexBufferGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_VertexBufferInfo>?) -> UInt32
    typealias VertexBufferCopyDeclarationElementsRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_VertexElement>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias IndexBufferGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_IndexBufferInfo>?) -> UInt32
    typealias ModelBoneCreateDefaultRoute = @convention(c) (UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCreateRoute = @convention(c) (Int32, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelBoneGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneGetIndexRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ModelBoneGetTransformRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?) -> UInt32
    typealias ModelBoneSetTransformRoute = @convention(c) (UInt64, CNASwift_Matrix) -> UInt32
    typealias ModelBoneGetParentRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneGetChildrenRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCollectionCreateRoute = @convention(c) (UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelBoneCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCollectionFindRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelBoneCollectionContainsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ModelMeshPartCreateDefaultRoute = @convention(c) (UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartCreateRoute = @convention(c) (UInt64, UInt64, Int32, Int32, Int32, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelMeshPartGetNumVerticesRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ModelMeshPartGetPrimitiveCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ModelMeshPartGetStartIndexRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ModelMeshPartGetVertexOffsetRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias ModelMeshPartGetEffectRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartSetEffectRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias ModelMeshPartGetVertexBufferRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartGetIndexBufferRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartCollectionCreateRoute = @convention(c) (UnsafePointer<UInt64>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelMeshPartCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshPartCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCreateRoute = @convention(c) (UInt64, UnsafePointer<UInt64>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCreateNamedRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafePointer<UInt64>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelMeshGetBoundingSphereRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_BoundingSphere>?) -> UInt32
    typealias ModelMeshGetMeshPartsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshGetEffectsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshGetParentBoneRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshDrawRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelMeshCollectionCreateRoute = @convention(c) (UnsafePointer<UInt64>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelMeshCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCollectionFindRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelMeshCollectionContainsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ModelEffectCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelEffectCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelEffectCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelEffectCollectionContainsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias ModelCreateDefaultRoute = @convention(c) (UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias ModelGetBonesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelGetMeshesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelGetRootRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelGetBoneTransformCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelCopyAbsoluteBoneTransformsRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelSetBoneTransformsRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Matrix>?, UInt64) -> UInt32
    typealias ModelCopyBoneTransformsRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias ModelDrawRoute = @convention(c) (UInt64, CNASwift_Matrix, CNASwift_Matrix, CNASwift_Matrix) -> UInt32

    // ---- Foundation 102: Storage ----
    typealias StorageDeviceShowSelectorRoute = @convention(c) (CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageDeviceShowSelectorForPlayerRoute = @convention(c) (UInt32, CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageDeviceShowSelectorWithSpaceRoute = @convention(c) (Int32, Int32, CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageDeviceShowSelectorForPlayerWithSpaceRoute = @convention(c) (UInt32, Int32, Int32, CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageDeviceGetFreeSpaceRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias StorageDeviceGetIsConnectedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageDeviceGetTotalSpaceRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias StorageDeviceDeleteContainerRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias StorageDeviceSubscribeDeviceChangedRoute = @convention(c) (CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageDeviceUnsubscribeDeviceChangedRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageDeviceDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageContainerOpenRoute = @convention(c) (UInt64, CNASwift_StringView, CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerGetDisplayNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerCopyDisplayNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageContainerGetStorageDeviceRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageContainerSubscribeDisposingRoute = @convention(c) (UInt64, CNASwift_StorageCompletionCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerUnsubscribeDisposingRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageContainerCreateDirectoryRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias StorageContainerDirectoryExistsRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageContainerDeleteDirectoryRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias StorageContainerFileExistsRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageContainerDeleteFileRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias StorageContainerGetDirectoryNameCountRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerCopyDirectoryNameRoute = @convention(c) (UInt64, CNASwift_StringView, UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerGetFileNameCountRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerCopyFileNameRoute = @convention(c) (UInt64, CNASwift_StringView, UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerCreateFileRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerOpenFileRoute = @convention(c) (UInt64, CNASwift_StringView, CNASwift_FileMode, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerOpenFileAccessRoute = @convention(c) (UInt64, CNASwift_StringView, CNASwift_FileMode, CNASwift_FileAccess, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageContainerOpenFileShareRoute = @convention(c) (UInt64, CNASwift_StringView, CNASwift_FileMode, CNASwift_FileAccess, CNASwift_FileShare, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageStreamReadRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias StorageStreamWriteRoute = @convention(c) (UInt64, UnsafePointer<UInt8>?, UInt64) -> UInt32
    typealias StorageStreamSeekRoute = @convention(c) (UInt64, Int64, CNASwift_SeekOrigin, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias StorageStreamGetPositionRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias StorageStreamGetLengthRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias StorageStreamSetLengthRoute = @convention(c) (UInt64, Int64) -> UInt32
    typealias StorageStreamGetCanReadRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageStreamGetCanWriteRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageStreamGetCanSeekRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias StorageStreamFlushRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageStreamCloseRoute = @convention(c) (UInt64) -> UInt32
    typealias StorageContainerDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaLibraryDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaLibraryDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaLibraryGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaLibraryGetSongsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryGetArtistsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryGetAlbumsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryGetGenresRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureAlbumCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureAlbumCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureAlbumCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureAlbumCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureAlbumGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureAlbumDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureAlbumDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PictureAlbumEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureAlbumGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureGetAlbumRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PictureGetWidthRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureGetHeightRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PictureGetDateUnixTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias PictureGetImageSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureCopyImageRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureGetThumbnailSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureCopyThumbnailRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumGetAlbumsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumGetPicturesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PictureAlbumGetParentRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaLibraryGetPicturesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryGetSavedPicturesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryGetRootPictureAlbumRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaLibraryGetPictureFromTokenRoute = @convention(c) (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PlaylistCollectionGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PlaylistCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PlaylistCollectionGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PlaylistCollectionDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PlaylistCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PlaylistGetNameSizeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PlaylistCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PlaylistGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PlaylistDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias PlaylistDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias PlaylistEqualsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias PlaylistGetHashCodeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias PlaylistGetSongsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias PlaylistGetDurationRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias MediaLibraryGetPlaylistsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaLibraryCreateFromSourceRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaSourceGetAvailableCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias MediaSourceGetNameSizeAtRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaSourceCopyNameAtRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaSourceGetTypeAtRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias MediaQueueGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias MediaQueueGetActiveSongIndexRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias MediaQueueSetActiveSongIndexRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias MediaQueueGetActiveSongRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaQueueGetAtRoute = @convention(c) (UInt64, Int32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaQueueDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerPlaySongRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias MediaPlayerPlaySongsRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias MediaPlayerPlaySongsFromRoute = @convention(c) (UInt64, UInt64, Int32) -> UInt32
    typealias MediaPlayerPauseRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerResumeRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerStopRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerMoveNextRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerMovePreviousRoute = @convention(c) (UInt64) -> UInt32
    typealias MediaPlayerGetVisualizationDataRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_VisualizationData>?) -> UInt32
    typealias MediaPlayerGetQueueRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaPlayerGetStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias MediaPlayerGetPlayPositionTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias MediaPlayerGetVolumeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias MediaPlayerSetVolumeRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias MediaPlayerGetIsMutedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaPlayerSetIsMutedRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias MediaPlayerGetIsRepeatingRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaPlayerSetIsRepeatingRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias MediaPlayerGetIsShuffledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaPlayerSetIsShuffledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias MediaPlayerGetIsVisualizationEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaPlayerSetIsVisualizationEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias MediaPlayerGetGameHasControlRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias MediaPlayerSubscribeActiveSongChangedExtRoute = @convention(c) (CNASwift_MediaPlayerEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaPlayerSubscribeMediaStateChangedExtRoute = @convention(c) (CNASwift_MediaPlayerEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias MediaPlayerUnsubscribeExtRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias VideoPlayerDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias VideoPlayerPlayRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias VideoPlayerPauseRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerResumeRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerStopRoute = @convention(c) (UInt64) -> UInt32
    typealias VideoPlayerGetVideoRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias VideoPlayerGetTextureRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias VideoPlayerGetStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias VideoPlayerGetPlayPositionTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias VideoPlayerGetIsLoopedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias VideoPlayerSetIsLoopedRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias VideoPlayerGetIsMutedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias VideoPlayerSetIsMutedRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias VideoPlayerGetVolumeRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias VideoPlayerSetVolumeRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias VideoGetDurationRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias VideoGetWidthRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias VideoGetHeightRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias VideoGetFramesPerSecondRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias VideoGetSoundtrackTypeRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias GamerServicesDispatcherInitializeRoute = @convention(c) (UInt64) -> UInt32
    typealias GamerServicesDispatcherUpdateRoute = @convention(c) () -> UInt32
    typealias TouchGetCapabilitiesRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_TouchCapabilities>?) -> UInt32
    typealias TouchGetStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_TouchState>?) -> UInt32
    typealias TouchPanelReadGestureRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_GestureSample>?) -> UInt32
    typealias TouchPanelGetIsGestureAvailableRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias TouchPanelGetEnabledGesturesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias TouchPanelSetEnabledGesturesRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias TouchPanelGetWindowHandleRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias TouchPanelSetWindowHandleRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias TouchPanelGetDisplayOrientationRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias TouchPanelSetDisplayOrientationRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias TouchPanelGetDisplayWidthRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias TouchPanelSetDisplayWidthRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias TouchPanelGetDisplayHeightRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias TouchPanelSetDisplayHeightRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias OcclusionQueryCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias OcclusionQueryDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias OcclusionQueryBeginRoute = @convention(c) (UInt64) -> UInt32
    typealias OcclusionQueryEndRoute = @convention(c) (UInt64) -> UInt32
    typealias OcclusionQueryGetIsCompleteRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias OcclusionQueryGetPixelCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias RenderTargetGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_RenderTargetInfo>?) -> UInt32
    typealias RenderTargetDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceSetRenderTarget2dRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias RenderTargetCubeCreateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_RenderTargetCubeCreateInfo>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceSetRenderTargetCubeRoute = @convention(c) (UInt64, UInt64, UInt32) -> UInt32
    typealias GraphicsDeviceSetRenderTargetsRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_RenderTargetBinding>?, UInt64) -> UInt32
    typealias GraphicsDeviceGetRenderTargetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceGetTextureRoute = @convention(c) (UInt64, UInt32, UInt32, UnsafeMutablePointer<CNASwift_TextureSlotInfo>?) -> UInt32
    typealias GraphicsDeviceSetTextureRoute = @convention(c) (UInt64, UInt32, UInt32, UInt64) -> UInt32
    typealias GraphicsDeviceDrawPrimitivesRoute = @convention(c) (UInt64, UInt32, Int32, Int32) -> UInt32
    typealias GraphicsDeviceDrawIndexedPrimitivesRoute = @convention(c) (UInt64, UInt32, Int32, Int32, Int32, Int32, Int32) -> UInt32
    typealias GraphicsDeviceDrawInstancedPrimitivesRoute = @convention(c) (UInt64, UInt32, Int32, Int32, Int32, Int32, Int32, Int32) -> UInt32
    typealias GraphicsDeviceDrawUserPrimitivesRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_UserPrimitives>?) -> UInt32
    typealias GraphicsDeviceDrawUserIndexedPrimitivesRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_UserPrimitives>?, UnsafePointer<CNASwift_UserIndices>?) -> UInt32
    typealias PrimitiveTypeGetVertexCountRoute = @convention(c) (UInt32, Int32, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias EffectMaterialCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias DirectionalLightDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias DirectionalLightGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias DirectionalLightSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias DirectionalLightGetDirectionRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias DirectionalLightSetDirectionRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias DirectionalLightGetSpecularColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias DirectionalLightSetSpecularColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias DirectionalLightGetEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias DirectionalLightSetEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias BasicEffectCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectMatricesGetWorldRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?) -> UInt32
    typealias EffectMatricesSetWorldRoute = @convention(c) (UInt64, CNASwift_Matrix) -> UInt32
    typealias EffectMatricesGetViewRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?) -> UInt32
    typealias EffectMatricesSetViewRoute = @convention(c) (UInt64, CNASwift_Matrix) -> UInt32
    typealias EffectMatricesGetProjectionRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?) -> UInt32
    typealias EffectMatricesSetProjectionRoute = @convention(c) (UInt64, CNASwift_Matrix) -> UInt32
    typealias EffectFogGetColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EffectFogSetColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias EffectFogGetEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias EffectFogSetEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias EffectFogGetStartRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EffectFogSetStartRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias EffectFogGetEndRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EffectFogSetEndRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias EffectLightsGetAmbientColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EffectLightsSetAmbientColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias EffectLightsGetDirectionalLightRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectLightsGetEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias EffectLightsSetEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias BasicEffectGetVertexColorEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias BasicEffectSetVertexColorEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias BasicEffectGetPreferPerPixelLightingRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias BasicEffectSetPreferPerPixelLightingRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias BasicEffectGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias BasicEffectSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias BasicEffectGetEmissiveColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias BasicEffectSetEmissiveColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias BasicEffectGetSpecularColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias BasicEffectSetSpecularColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias BasicEffectGetSpecularPowerRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias BasicEffectSetSpecularPowerRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias BasicEffectGetAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias BasicEffectSetAlphaRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias BasicEffectGetTextureEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias BasicEffectSetTextureEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias BasicEffectGetTextureRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias BasicEffectSetTextureRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias AlphaTestEffectCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlphaTestEffectGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias AlphaTestEffectSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias AlphaTestEffectGetAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias AlphaTestEffectSetAlphaRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias AlphaTestEffectGetTextureRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias AlphaTestEffectSetTextureRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias AlphaTestEffectGetVertexColorEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias AlphaTestEffectSetVertexColorEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias AlphaTestEffectGetAlphaFunctionRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias AlphaTestEffectSetAlphaFunctionRoute = @convention(c) (UInt64, UInt32) -> UInt32
    typealias AlphaTestEffectGetReferenceAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias AlphaTestEffectSetReferenceAlphaRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias DualTextureEffectCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias DualTextureEffectGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias DualTextureEffectSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias DualTextureEffectGetAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias DualTextureEffectSetAlphaRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias DualTextureEffectGetTextureRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias DualTextureEffectSetTextureRoute = @convention(c) (UInt64, UInt32, UInt64) -> UInt32
    typealias DualTextureEffectGetVertexColorEnabledRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias DualTextureEffectSetVertexColorEnabledRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias EnvironmentMapEffectCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EnvironmentMapEffectGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EnvironmentMapEffectSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias EnvironmentMapEffectGetEmissiveColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EnvironmentMapEffectSetEmissiveColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias EnvironmentMapEffectGetAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EnvironmentMapEffectSetAlphaRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias EnvironmentMapEffectGetTextureRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EnvironmentMapEffectSetTextureRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias EnvironmentMapEffectGetEnvironmentMapRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EnvironmentMapEffectSetEnvironmentMapRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias EnvironmentMapEffectGetAmountRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EnvironmentMapEffectSetAmountRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias EnvironmentMapEffectGetSpecularRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EnvironmentMapEffectSetSpecularRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias EnvironmentMapEffectGetFresnelFactorRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EnvironmentMapEffectSetFresnelFactorRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SkinnedEffectCreateRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SkinnedEffectGetDiffuseColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias SkinnedEffectSetDiffuseColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias SkinnedEffectGetEmissiveColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias SkinnedEffectSetEmissiveColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias SkinnedEffectGetSpecularColorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias SkinnedEffectSetSpecularColorRoute = @convention(c) (UInt64, CNASwift_Vector3) -> UInt32
    typealias SkinnedEffectGetSpecularPowerRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SkinnedEffectSetSpecularPowerRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SkinnedEffectGetAlphaRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias SkinnedEffectSetAlphaRoute = @convention(c) (UInt64, Float) -> UInt32
    typealias SkinnedEffectGetPreferPerPixelLightingRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias SkinnedEffectSetPreferPerPixelLightingRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias SkinnedEffectGetTextureRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias SkinnedEffectSetTextureRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias SkinnedEffectGetWeightsPerVertexRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias SkinnedEffectSetWeightsPerVertexRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias SkinnedEffectSetBoneTransformsRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_Matrix>?, UInt64) -> UInt32
    typealias SkinnedEffectCopyBoneTransformsRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<CNASwift_Matrix>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectCreateEmptyRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectCreateCompiledRoute = @convention(c) (UInt64, UnsafePointer<UInt8>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectCloneRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectApplyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectGetParametersRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectGetTechniquesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectGetCurrentTechniqueRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectSetCurrentTechniqueRoute = @convention(c) (UInt64, UInt64) -> UInt32
    typealias EffectGetGraphicsDeviceRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectTechniqueGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueGetPassesRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueGetAnnotationsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectTechniqueCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectTechniqueCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectPassDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectPassGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectPassCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectPassGetAnnotationsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectPassApplyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectPassCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectPassCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectPassCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectParameterGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_EffectParameterInfo>?) -> UInt32
    typealias EffectParameterGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterGetSemanticByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterCopySemanticRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterGetElementsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterGetStructureMembersRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterGetAnnotationsRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterGetValueRoute = @convention(c) (UInt64, UInt32, UnsafeMutableRawPointer?) -> UInt32
    typealias EffectParameterSetValueRoute = @convention(c) (UInt64, UInt32, UnsafeRawPointer?) -> UInt32
    typealias EffectParameterGetValuesRoute = @convention(c) (UInt64, UInt32, UInt64, UnsafeMutableRawPointer?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterSetValuesRoute = @convention(c) (UInt64, UInt32, UnsafeRawPointer?, UInt64) -> UInt32
    typealias EffectParameterGetValueStringByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterCopyValueStringRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterSetValueStringRoute = @convention(c) (UInt64, CNASwift_StringView) -> UInt32
    typealias EffectParameterGetValueTextureRoute = @convention(c) (UInt64, UInt32, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterSetValueTextureRoute = @convention(c) (UInt64, UInt32, UInt64) -> UInt32
    typealias EffectParameterCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectParameterCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectParameterCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectAnnotationGetInfoRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_EffectAnnotationInfo>?) -> UInt32
    typealias EffectAnnotationGetNameByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationCopyNameRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationGetSemanticByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationCopySemanticRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationGetValueBooleanRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias EffectAnnotationGetValueInt32Route = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias EffectAnnotationGetValueSingleRoute = @convention(c) (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    typealias EffectAnnotationGetValueVector2Route = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector2>?) -> UInt32
    typealias EffectAnnotationGetValueVector3Route = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    typealias EffectAnnotationGetValueVector4Route = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Vector4>?) -> UInt32
    typealias EffectAnnotationGetValueMatrixRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Matrix>?) -> UInt32
    typealias EffectAnnotationGetValueStringByteCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationCopyValueStringRoute = @convention(c) (UInt64, UnsafeMutablePointer<CChar>?, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationCollectionDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias EffectAnnotationCollectionGetCountRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias EffectAnnotationCollectionGetAtRoute = @convention(c) (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceClearOptionsRoute = @convention(c) (UInt64, UInt32, CNASwift_Color, Float, Int32) -> UInt32
    typealias GraphicsDeviceCreateRoute = @convention(c) (UInt32, UInt32, UnsafePointer<CNASwift_PresentationParameters>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceDestroyRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceGetIsDisposedRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GraphicsDeviceDisposeRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDevicePresentRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceResetRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceResetWithParametersRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_PresentationParameters>?, UnsafePointer<UInt32>?) -> UInt32
    typealias GraphicsDeviceGetPresentationParametersRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_PresentationParameters>?) -> UInt32
    typealias GraphicsDeviceSetViewportRoute = @convention(c) (UInt64, CNASwift_Viewport) -> UInt32
    typealias GraphicsDeviceGetScissorRectangleRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Rectangle>?) -> UInt32
    typealias GraphicsDeviceSetScissorRectangleRoute = @convention(c) (UInt64, CNASwift_Rectangle) -> UInt32
    typealias GraphicsDeviceGetStatusRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt32>?) -> UInt32
    typealias GraphicsDeviceGetBlendFactorRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_Color>?) -> UInt32
    typealias GraphicsDeviceSetBlendFactorRoute = @convention(c) (UInt64, CNASwift_Color) -> UInt32
    typealias GraphicsDeviceGetMultiSampleMaskRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias GraphicsDeviceSetMultiSampleMaskRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias GraphicsDeviceGetReferenceStencilRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int32>?) -> UInt32
    typealias GraphicsDeviceSetReferenceStencilRoute = @convention(c) (UInt64, Int32) -> UInt32
    typealias GraphicsDeviceGetBlendStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_BlendState>?) -> UInt32
    typealias GraphicsDeviceSetBlendStateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_BlendState>?) -> UInt32
    typealias GraphicsDeviceGetDepthStencilStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_DepthStencilState>?) -> UInt32
    typealias GraphicsDeviceSetDepthStencilStateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_DepthStencilState>?) -> UInt32
    typealias GraphicsDeviceGetRasterizerStateRoute = @convention(c) (UInt64, UnsafeMutablePointer<CNASwift_RasterizerState>?) -> UInt32
    typealias GraphicsDeviceSetRasterizerStateRoute = @convention(c) (UInt64, UnsafePointer<CNASwift_RasterizerState>?) -> UInt32
    typealias GraphicsDeviceGetSamplerStateRoute = @convention(c) (UInt64, UInt32, UInt32, UnsafeMutablePointer<CNASwift_SamplerState>?) -> UInt32
    typealias GraphicsDeviceSetSamplerStateRoute = @convention(c) (UInt64, UInt32, UInt32, UnsafePointer<CNASwift_SamplerState>?) -> UInt32
    typealias RenderTargetSubscribeContentLostRoute = @convention(c) (UInt64, CNASwift_RenderTargetContentLostCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias RenderTargetUnsubscribeContentLostRoute = @convention(c) (UInt64) -> UInt32
    typealias GameTickRoute = @convention(c) (UInt64) -> UInt32
    typealias GameSuppressDrawRoute = @convention(c) (UInt64) -> UInt32
    typealias GameResetElapsedTimeRoute = @convention(c) (UInt64) -> UInt32
    typealias GameGetIsActiveRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GameGetIsMouseVisibleRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GameSetIsMouseVisibleRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GameGetIsFixedTimeStepRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GameSetIsFixedTimeStepRoute = @convention(c) (UInt64, UInt8) -> UInt32
    typealias GameGetTargetElapsedTimeTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias GameSetTargetElapsedTimeTicksRoute = @convention(c) (UInt64, Int64) -> UInt32
    typealias GameGetInactiveSleepTimeTicksRoute = @convention(c) (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32
    typealias GameSetInactiveSleepTimeTicksRoute = @convention(c) (UInt64, Int64) -> UInt32
    typealias GameSubscribeRoute = @convention(c) (UInt64, UInt32, CNASwift_GameEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GameUnsubscribeRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceManagerCreateDeviceRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceManagerBeginDrawRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
    typealias GraphicsDeviceManagerEndDrawRoute = @convention(c) (UInt64) -> UInt32
    typealias GraphicsDeviceManagerGetGraphicsDeviceRoute = @convention(c) (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    typealias GraphicsDeviceManagerSubscribeRoute = @convention(c) (UInt64, UInt32, CNASwift_GameEventCallback?, UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt64>?) -> UInt32

    private static let lock = NSLock()
    private static var cached: Result<NativeFunctions, Error>?

    let library: NativeLibrary
    /// The ABI the admitted library actually reported, kept so diagnostics and
    /// tests can state the measured generation rather than the required one.
    let abiVersion: UInt32
    let getABIVersion: GetAbiVersionRoute
    let errorMessageSize: ErrorGetLastMessageSizeRoute
    let errorMessageCopy: ErrorCopyLastMessageRoute
    let gameCreate: GameCreateRoute
    let gameSetFrameHooks: GameSetFrameHooksExtRoute
    let gameRun: GameRunRoute
    let gameRunOneFrame: GameRunOneFrameRoute
    let frameworkDispatcherUpdate: FrameworkDispatcherUpdateRoute
    let titleContainerRead: TitleContainerReadExtRoute
    let gameRequestExit: GameRequestExitRoute
    let gameDestroy: GameDestroyRoute
    let gameGetGraphicsDevice: GameGetGraphicsDeviceRoute
    let graphicsManagerCreate: GraphicsDeviceManagerCreateRoute
    let graphicsManagerApplyChanges: GraphicsDeviceManagerApplyChangesRoute
    let graphicsManagerSetGraphicsProfile: GraphicsDeviceManagerSetGraphicsProfileRoute
    let graphicsManagerSetIsFullScreen: GraphicsDeviceManagerSetIsFullScreenRoute
    let graphicsManagerSetPreferMultiSampling: GraphicsDeviceManagerSetPreferMultiSamplingRoute
    let graphicsManagerSetPreferredBackBufferFormat: GraphicsDeviceManagerSetPreferredBackBufferFormatRoute
    let graphicsManagerSetPreferredBackBufferWidth: GraphicsDeviceManagerSetPreferredBackBufferWidthRoute
    let graphicsManagerSetPreferredBackBufferHeight: GraphicsDeviceManagerSetPreferredBackBufferHeightRoute
    let graphicsManagerSetPreferredDepthStencilFormat: GraphicsDeviceManagerSetPreferredDepthStencilFormatRoute
    let graphicsManagerSetSynchronizeWithVerticalRetrace: GraphicsDeviceManagerSetSynchronizeWithVerticalRetraceRoute
    let graphicsManagerSetSupportedOrientations: GraphicsDeviceManagerSetSupportedOrientationsRoute
    let graphicsManagerDestroy: GraphicsDeviceManagerDestroyRoute
    let graphicsDeviceGetViewport: GraphicsDeviceGetViewportRoute
    let textureCreateMemory: Texture2dCreateFromEncodedMemoryRoute
    let textureGetInfo: Texture2dGetInfoRoute
    let textureCreate: Texture2dCreateRoute
    let textureDestroy: Texture2dDestroyRoute
    let textureSetData: Texture2dSetDataRoute
    let textureGetData: Texture2dGetDataRoute
    let textureGetEncodedByteCount: Texture2dGetEncodedByteCountRoute
    let textureCopyEncoded: Texture2dCopyEncodedRoute
    let textureCreateCpuOnly: Texture2dCreateCpuOnlyRgba8Route
    let vertexDeclarationCreateWithStride: VertexDeclarationCreateWithStrideRoute
    let vertexDeclarationDestroy: VertexDeclarationDestroyRoute
    let vertexBufferCreate: VertexBufferCreateRoute
    let vertexBufferDestroy: VertexBufferDestroyRoute
    let vertexBufferSetDataRawAt: VertexBufferSetDataRawAtRoute
    let vertexBufferGetDataRaw: VertexBufferGetDataRawRoute
    let indexBufferCreate: IndexBufferCreateRoute
    let indexBufferDestroy: IndexBufferDestroyRoute
    let indexBufferSetDataAt: IndexBufferSetDataAtRoute
    let indexBufferGetData: IndexBufferGetDataRoute
    let vertexBufferSetDataRawAtWithOptions: VertexBufferSetDataRawAtWithOptionsRoute
    let vertexBufferSubscribeContentLost: VertexBufferSubscribeContentLostRoute
    let vertexBufferUnsubscribeContentLost: VertexBufferUnsubscribeContentLostRoute
    let indexBufferSetData: IndexBufferSetDataRoute
    let indexBufferSubscribeContentLost: IndexBufferSubscribeContentLostRoute
    let indexBufferUnsubscribeContentLost: IndexBufferUnsubscribeContentLostRoute
    let graphicsDeviceGetGraphicsProfile: GraphicsDeviceGetGraphicsProfileRoute
    let graphicsDeviceGetBackBufferData: GraphicsDeviceGetBackbufferDataWindowRoute
    let graphicsDeviceSetVertexBuffers: GraphicsDeviceSetVertexBuffersRoute
    let graphicsDeviceSetIndexBuffer: GraphicsDeviceSetIndexBufferRoute
    let textureCubeCreate: TexturecubeCreateRoute
    let textureCubeDestroy: TexturecubeDestroyRoute
    let textureCubeGetInfo: TexturecubeGetInfoRoute
    let textureCubeSetData: TexturecubeSetDataRoute
    let textureCubeGetData: TexturecubeGetDataRoute
    let texture3DCreate: Texture3dCreateRoute
    let texture3DDestroy: Texture3dDestroyRoute
    let texture3DGetInfo: Texture3dGetInfoRoute
    let texture3DSetData: Texture3dSetDataRoute
    let texture3DGetData: Texture3dGetDataRoute
    let spriteBatchCreate: SpriteBatchCreateRoute
    let spriteBatchBeginWithEffect: SpriteBatchBeginWithEffectRoute
    let spriteBatchBeginWithStates: SpriteBatchBeginWithStatesRoute
    let spriteBatchSubmitScaled: SpriteBatchSubmitScaledManyRoute
    let spriteFontCreate: SpriteFontCreateRoute
    let spriteFontDestroy: SpriteFontDestroyRoute
    let spriteFontGetInfo: SpriteFontGetInfoRoute
    let spriteFontCopyCharacters: SpriteFontCopyCharactersRoute
    let spriteFontCopyGlyphs: SpriteFontCopyGlyphsRoute
    let spriteFontSetDefaultCharacter: SpriteFontSetDefaultCharacterRoute
    let spriteFontSetLineSpacing: SpriteFontSetLineSpacingRoute
    let spriteFontSetSpacing: SpriteFontSetSpacingRoute
    let spriteBatchDrawString: SpriteBatchDrawStringRoute
    let spriteBatchSubmit: SpriteBatchSubmitManyRoute
    let spriteBatchEnd: SpriteBatchEndRoute
    let spriteBatchDestroy: SpriteBatchDestroyRoute
    let graphicsAdapterGetCount: GraphicsAdapterGetCountRoute
    let graphicsAdapterGetInfo: GraphicsAdapterGetInfoRoute
    let graphicsAdapterCopyDescription: GraphicsAdapterCopyDescriptionRoute
    let graphicsAdapterCopyDeviceName: GraphicsAdapterCopyDeviceNameRoute
    let graphicsAdapterGetCurrentDisplayMode: GraphicsAdapterGetCurrentDisplayModeRoute
    let graphicsAdapterGetDisplayModeCount: GraphicsAdapterGetDisplayModeCountRoute
    let graphicsAdapterCopyDisplayModes: GraphicsAdapterCopyDisplayModesRoute
    let graphicsAdapterSetDevicePreferences: GraphicsAdapterSetDevicePreferencesRoute
    let graphicsAdapterIsProfileSupported: GraphicsAdapterIsProfileSupportedRoute
    let graphicsAdapterQueryBackbufferFormat: GraphicsAdapterQueryBackbufferFormatRoute
    let graphicsAdapterQueryRenderTargetFormat: GraphicsAdapterQueryRenderTargetFormatRoute
    let gameWindowGetTitleSize: GameWindowGetTitleSizeRoute
    let gameWindowCopyTitle: GameWindowCopyTitleRoute
    let gameWindowGetScreenDeviceNameSize: GameWindowGetScreenDeviceNameSizeRoute
    let gameWindowCopyScreenDeviceName: GameWindowCopyScreenDeviceNameRoute
    let gameWindowGetNativeHandle: GameWindowGetNativeHandleExtRoute
    let gameWindowGetClientBounds: GameWindowGetClientBoundsRoute
    let gameWindowGetCurrentOrientation: GameWindowGetCurrentOrientationRoute
    let gameWindowGetAllowUserResizing: GameWindowGetAllowUserResizingRoute
    let gameWindowSetAllowUserResizing: GameWindowSetAllowUserResizingRoute
    let gameWindowBeginScreenDeviceChange: GameWindowBeginScreenDeviceChangeRoute
    let gameWindowEndScreenDeviceChange: GameWindowEndScreenDeviceChangeRoute
    let gameSetWindowTitle: GameSetWindowTitleRoute
    let contentManagerCreate: ContentManagerCreateRoute
    let contentManagerDestroy: ContentManagerDestroyRoute
    let contentManagerUnload: ContentManagerUnloadRoute
    let contentManagerGetRootDirectorySize: ContentManagerGetRootDirectorySizeRoute
    let contentManagerCopyRootDirectory: ContentManagerCopyRootDirectoryRoute
    let contentManagerLoadTexture2d: ContentManagerLoadTexture2dRoute
    let mouseGetState: MouseGetStateRoute
    let mouseSetPosition: MouseSetPositionRoute
    let mouseSetWindowHandle: MouseSetWindowHandleRoute
    let keyboardGetState: KeyboardGetStateRoute
    let keyboardGetStateForPlayer: KeyboardGetStateForPlayerRoute
    let gamePadGetState: GamepadGetStateRoute
    let gamePadGetStateWithDeadZone: GamepadGetStateWithDeadZoneRoute
    let gamePadGetCapabilities: GamepadGetCapabilitiesRoute
    let gamePadSetVibration: GamepadSetVibrationRoute
    let textureCommonGetInfo: TextureGetInfoRoute
    let renderTarget2DCreate: RenderTarget2dCreateRoute
    let occlusionQueryCreate: OcclusionQueryCreateRoute
    let touchGetCapabilities: TouchGetCapabilitiesRoute
    let touchGetState: TouchGetStateRoute
    let touchPanelReadGesture: TouchPanelReadGestureRoute
    let touchPanelGetIsGestureAvailable: TouchPanelGetIsGestureAvailableRoute
    let touchPanelGetEnabledGestures: TouchPanelGetEnabledGesturesRoute
    let touchPanelSetEnabledGestures: TouchPanelSetEnabledGesturesRoute
    let touchPanelGetWindowHandle: TouchPanelGetWindowHandleRoute
    let touchPanelSetWindowHandle: TouchPanelSetWindowHandleRoute
    let touchPanelGetDisplayOrientation: TouchPanelGetDisplayOrientationRoute
    let touchPanelSetDisplayOrientation: TouchPanelSetDisplayOrientationRoute
    let touchPanelGetDisplayWidth: TouchPanelGetDisplayWidthRoute
    let touchPanelSetDisplayWidth: TouchPanelSetDisplayWidthRoute
    let touchPanelGetDisplayHeight: TouchPanelGetDisplayHeightRoute
    let touchPanelSetDisplayHeight: TouchPanelSetDisplayHeightRoute
    let gamerServicesDispatcherInitialize: GamerServicesDispatcherInitializeRoute
    let gamerServicesDispatcherUpdate: GamerServicesDispatcherUpdateRoute
    let videoPlayerCreate: VideoPlayerCreateRoute
    let videoPlayerDispose: VideoPlayerDisposeRoute
    let videoPlayerDestroy: VideoPlayerDestroyRoute
    let videoPlayerGetIsDisposed: VideoPlayerGetIsDisposedRoute
    let videoPlayerPlay: VideoPlayerPlayRoute
    let videoPlayerPause: VideoPlayerPauseRoute
    let videoPlayerResume: VideoPlayerResumeRoute
    let videoPlayerStop: VideoPlayerStopRoute
    let videoPlayerGetVideo: VideoPlayerGetVideoRoute
    let videoPlayerGetTexture: VideoPlayerGetTextureRoute
    let videoPlayerGetState: VideoPlayerGetStateRoute
    let videoPlayerGetPlayPositionTicks: VideoPlayerGetPlayPositionTicksRoute
    let videoPlayerGetIsLooped: VideoPlayerGetIsLoopedRoute
    let videoPlayerSetIsLooped: VideoPlayerSetIsLoopedRoute
    let videoPlayerGetIsMuted: VideoPlayerGetIsMutedRoute
    let videoPlayerSetIsMuted: VideoPlayerSetIsMutedRoute
    let videoPlayerGetVolume: VideoPlayerGetVolumeRoute
    let videoPlayerSetVolume: VideoPlayerSetVolumeRoute
    let videoGetDuration: VideoGetDurationRoute
    let videoGetWidth: VideoGetWidthRoute
    let videoGetHeight: VideoGetHeightRoute
    let videoGetFramesPerSecond: VideoGetFramesPerSecondRoute
    let videoGetSoundtrackType: VideoGetSoundtrackTypeRoute
    let mediaQueueGetCount: MediaQueueGetCountRoute
    let mediaQueueGetActiveSongIndex: MediaQueueGetActiveSongIndexRoute
    let mediaQueueSetActiveSongIndex: MediaQueueSetActiveSongIndexRoute
    let mediaQueueGetActiveSong: MediaQueueGetActiveSongRoute
    let mediaQueueGetAt: MediaQueueGetAtRoute
    let mediaQueueDestroy: MediaQueueDestroyRoute
    let mediaPlayerPlaySong: MediaPlayerPlaySongRoute
    let mediaPlayerPlaySongs: MediaPlayerPlaySongsRoute
    let mediaPlayerPlaySongsFrom: MediaPlayerPlaySongsFromRoute
    let mediaPlayerPause: MediaPlayerPauseRoute
    let mediaPlayerResume: MediaPlayerResumeRoute
    let mediaPlayerStop: MediaPlayerStopRoute
    let mediaPlayerMoveNext: MediaPlayerMoveNextRoute
    let mediaPlayerMovePrevious: MediaPlayerMovePreviousRoute
    let mediaPlayerGetVisualizationData: MediaPlayerGetVisualizationDataRoute
    let mediaPlayerGetQueue: MediaPlayerGetQueueRoute
    let mediaPlayerGetState: MediaPlayerGetStateRoute
    let mediaPlayerGetPlayPositionTicks: MediaPlayerGetPlayPositionTicksRoute
    let mediaPlayerGetVolume: MediaPlayerGetVolumeRoute
    let mediaPlayerSetVolume: MediaPlayerSetVolumeRoute
    let mediaPlayerGetIsMuted: MediaPlayerGetIsMutedRoute
    let mediaPlayerSetIsMuted: MediaPlayerSetIsMutedRoute
    let mediaPlayerGetIsRepeating: MediaPlayerGetIsRepeatingRoute
    let mediaPlayerSetIsRepeating: MediaPlayerSetIsRepeatingRoute
    let mediaPlayerGetIsShuffled: MediaPlayerGetIsShuffledRoute
    let mediaPlayerSetIsShuffled: MediaPlayerSetIsShuffledRoute
    let mediaPlayerGetIsVisualizationEnabled: MediaPlayerGetIsVisualizationEnabledRoute
    let mediaPlayerSetIsVisualizationEnabled: MediaPlayerSetIsVisualizationEnabledRoute
    let mediaPlayerGetGameHasControl: MediaPlayerGetGameHasControlRoute
    let mediaPlayerSubscribeActiveSongChanged: MediaPlayerSubscribeActiveSongChangedExtRoute
    let mediaPlayerSubscribeMediaStateChanged: MediaPlayerSubscribeMediaStateChangedExtRoute
    let mediaPlayerUnsubscribe: MediaPlayerUnsubscribeExtRoute
    let playlistCollectionGetAt: PlaylistCollectionGetAtRoute
    let playlistCollectionGetCount: PlaylistCollectionGetCountRoute
    let playlistCollectionGetIsDisposed: PlaylistCollectionGetIsDisposedRoute
    let playlistCollectionDispose: PlaylistCollectionDisposeRoute
    let playlistCollectionDestroy: PlaylistCollectionDestroyRoute
    let playlistGetNameSize: PlaylistGetNameSizeRoute
    let playlistCopyName: PlaylistCopyNameRoute
    let playlistGetIsDisposed: PlaylistGetIsDisposedRoute
    let playlistDispose: PlaylistDisposeRoute
    let playlistDestroy: PlaylistDestroyRoute
    let playlistEquals: PlaylistEqualsRoute
    let playlistGetHashCode: PlaylistGetHashCodeRoute
    let playlistGetSongs: PlaylistGetSongsRoute
    let playlistGetDuration: PlaylistGetDurationRoute
    let mediaLibraryGetPlaylists: MediaLibraryGetPlaylistsRoute
    let mediaLibraryCreateFromSource: MediaLibraryCreateFromSourceRoute
    let mediaSourceGetAvailableCount: MediaSourceGetAvailableCountRoute
    let mediaSourceGetNameSizeAt: MediaSourceGetNameSizeAtRoute
    let mediaSourceCopyNameAt: MediaSourceCopyNameAtRoute
    let mediaSourceGetTypeAt: MediaSourceGetTypeAtRoute
    let pictureCollectionGetAt: PictureCollectionGetAtRoute
    let pictureCollectionGetCount: PictureCollectionGetCountRoute
    let pictureCollectionGetIsDisposed: PictureCollectionGetIsDisposedRoute
    let pictureCollectionDispose: PictureCollectionDisposeRoute
    let pictureCollectionDestroy: PictureCollectionDestroyRoute
    let pictureAlbumCollectionGetAt: PictureAlbumCollectionGetAtRoute
    let pictureAlbumCollectionGetCount: PictureAlbumCollectionGetCountRoute
    let pictureAlbumCollectionGetIsDisposed: PictureAlbumCollectionGetIsDisposedRoute
    let pictureAlbumCollectionDispose: PictureAlbumCollectionDisposeRoute
    let pictureAlbumCollectionDestroy: PictureAlbumCollectionDestroyRoute
    let pictureGetNameSize: PictureGetNameSizeRoute
    let pictureCopyName: PictureCopyNameRoute
    let pictureGetIsDisposed: PictureGetIsDisposedRoute
    let pictureDispose: PictureDisposeRoute
    let pictureDestroy: PictureDestroyRoute
    let pictureEquals: PictureEqualsRoute
    let pictureGetHashCode: PictureGetHashCodeRoute
    let pictureAlbumGetNameSize: PictureAlbumGetNameSizeRoute
    let pictureAlbumCopyName: PictureAlbumCopyNameRoute
    let pictureAlbumGetIsDisposed: PictureAlbumGetIsDisposedRoute
    let pictureAlbumDispose: PictureAlbumDisposeRoute
    let pictureAlbumDestroy: PictureAlbumDestroyRoute
    let pictureAlbumEquals: PictureAlbumEqualsRoute
    let pictureAlbumGetHashCode: PictureAlbumGetHashCodeRoute
    let pictureGetAlbum: PictureGetAlbumRoute
    let pictureGetWidth: PictureGetWidthRoute
    let pictureGetHeight: PictureGetHeightRoute
    let pictureGetDateUnixTicks: PictureGetDateUnixTicksRoute
    let pictureGetImageSize: PictureGetImageSizeRoute
    let pictureCopyImage: PictureCopyImageRoute
    let pictureGetThumbnailSize: PictureGetThumbnailSizeRoute
    let pictureCopyThumbnail: PictureCopyThumbnailRoute
    let pictureAlbumGetAlbums: PictureAlbumGetAlbumsRoute
    let pictureAlbumGetPictures: PictureAlbumGetPicturesRoute
    let pictureAlbumGetParent: PictureAlbumGetParentRoute
    let mediaLibraryGetPictures: MediaLibraryGetPicturesRoute
    let mediaLibraryGetSavedPictures: MediaLibraryGetSavedPicturesRoute
    let mediaLibraryGetRootPictureAlbum: MediaLibraryGetRootPictureAlbumRoute
    let mediaLibraryGetPictureFromToken: MediaLibraryGetPictureFromTokenRoute
    let mediaLibraryCreate: MediaLibraryCreateRoute

    let modelBoneCreateDefault: ModelBoneCreateDefaultRoute
    let modelBoneCreate: ModelBoneCreateRoute
    let modelBoneDestroy: ModelBoneDestroyRoute
    let modelBoneGetNameByteCount: ModelBoneGetNameByteCountRoute
    let modelBoneCopyName: ModelBoneCopyNameRoute
    let modelBoneGetIndex: ModelBoneGetIndexRoute
    let modelBoneGetTransform: ModelBoneGetTransformRoute
    let modelBoneSetTransform: ModelBoneSetTransformRoute
    let modelBoneGetParent: ModelBoneGetParentRoute
    let modelBoneGetChildren: ModelBoneGetChildrenRoute
    let modelBoneCollectionCreate: ModelBoneCollectionCreateRoute
    let modelBoneCollectionDestroy: ModelBoneCollectionDestroyRoute
    let modelBoneCollectionGetCount: ModelBoneCollectionGetCountRoute
    let modelBoneCollectionGetAt: ModelBoneCollectionGetAtRoute
    let modelBoneCollectionFind: ModelBoneCollectionFindRoute
    let modelBoneCollectionContains: ModelBoneCollectionContainsRoute
    let modelMeshPartCreateDefault: ModelMeshPartCreateDefaultRoute
    let modelMeshPartCreate: ModelMeshPartCreateRoute
    let modelMeshPartDestroy: ModelMeshPartDestroyRoute
    let modelMeshPartGetNumVertices: ModelMeshPartGetNumVerticesRoute
    let modelMeshPartGetPrimitiveCount: ModelMeshPartGetPrimitiveCountRoute
    let modelMeshPartGetStartIndex: ModelMeshPartGetStartIndexRoute
    let modelMeshPartGetVertexOffset: ModelMeshPartGetVertexOffsetRoute
    let modelMeshPartGetEffect: ModelMeshPartGetEffectRoute
    let modelMeshPartSetEffect: ModelMeshPartSetEffectRoute
    let modelMeshPartGetVertexBuffer: ModelMeshPartGetVertexBufferRoute
    let modelMeshPartGetIndexBuffer: ModelMeshPartGetIndexBufferRoute
    let modelMeshPartCollectionCreate: ModelMeshPartCollectionCreateRoute
    let modelMeshPartCollectionDestroy: ModelMeshPartCollectionDestroyRoute
    let modelMeshPartCollectionGetCount: ModelMeshPartCollectionGetCountRoute
    let modelMeshPartCollectionGetAt: ModelMeshPartCollectionGetAtRoute
    let modelMeshCreate: ModelMeshCreateRoute
    let modelMeshCreateNamed: ModelMeshCreateNamedRoute
    let modelMeshDestroy: ModelMeshDestroyRoute
    let modelMeshGetBoundingSphere: ModelMeshGetBoundingSphereRoute
    let modelMeshGetMeshParts: ModelMeshGetMeshPartsRoute
    let modelMeshGetEffects: ModelMeshGetEffectsRoute
    let modelMeshGetNameByteCount: ModelMeshGetNameByteCountRoute
    let modelMeshCopyName: ModelMeshCopyNameRoute
    let modelMeshGetParentBone: ModelMeshGetParentBoneRoute
    let modelMeshDraw: ModelMeshDrawRoute
    let modelMeshCollectionCreate: ModelMeshCollectionCreateRoute
    let modelMeshCollectionDestroy: ModelMeshCollectionDestroyRoute
    let modelMeshCollectionGetCount: ModelMeshCollectionGetCountRoute
    let modelMeshCollectionGetAt: ModelMeshCollectionGetAtRoute
    let modelMeshCollectionFind: ModelMeshCollectionFindRoute
    let modelMeshCollectionContains: ModelMeshCollectionContainsRoute
    let modelEffectCollectionDestroy: ModelEffectCollectionDestroyRoute
    let modelEffectCollectionGetCount: ModelEffectCollectionGetCountRoute
    let modelEffectCollectionGetAt: ModelEffectCollectionGetAtRoute
    let modelEffectCollectionContains: ModelEffectCollectionContainsRoute
    let vertexBufferGetInfo: VertexBufferGetInfoRoute
    let vertexBufferCopyDeclarationElements: VertexBufferCopyDeclarationElementsRoute
    let indexBufferGetInfo: IndexBufferGetInfoRoute
    let modelCreateDefault: ModelCreateDefaultRoute
    let modelDestroy: ModelDestroyRoute
    let modelGetBones: ModelGetBonesRoute
    let modelGetMeshes: ModelGetMeshesRoute
    let modelGetRoot: ModelGetRootRoute
    let modelGetBoneTransformCount: ModelGetBoneTransformCountRoute
    let modelCopyAbsoluteBoneTransforms: ModelCopyAbsoluteBoneTransformsRoute
    let modelSetBoneTransforms: ModelSetBoneTransformsRoute
    let modelCopyBoneTransforms: ModelCopyBoneTransformsRoute
    let modelDraw: ModelDrawRoute
    let storageDeviceShowSelector: StorageDeviceShowSelectorRoute
    let storageDeviceShowSelectorForPlayer: StorageDeviceShowSelectorForPlayerRoute
    let storageDeviceShowSelectorWithSpace: StorageDeviceShowSelectorWithSpaceRoute
    let storageDeviceShowSelectorForPlayerWithSpace: StorageDeviceShowSelectorForPlayerWithSpaceRoute
    let storageDeviceGetFreeSpace: StorageDeviceGetFreeSpaceRoute
    let storageDeviceGetIsConnected: StorageDeviceGetIsConnectedRoute
    let storageDeviceGetTotalSpace: StorageDeviceGetTotalSpaceRoute
    let storageDeviceDeleteContainer: StorageDeviceDeleteContainerRoute
    let storageDeviceSubscribeDeviceChanged: StorageDeviceSubscribeDeviceChangedRoute
    let storageDeviceUnsubscribeDeviceChanged: StorageDeviceUnsubscribeDeviceChangedRoute
    let storageDeviceDestroy: StorageDeviceDestroyRoute
    let storageContainerOpen: StorageContainerOpenRoute
    let storageContainerGetDisplayNameSize: StorageContainerGetDisplayNameSizeRoute
    let storageContainerCopyDisplayName: StorageContainerCopyDisplayNameRoute
    let storageContainerGetIsDisposed: StorageContainerGetIsDisposedRoute
    let storageContainerGetStorageDevice: StorageContainerGetStorageDeviceRoute
    let storageContainerDispose: StorageContainerDisposeRoute
    let storageContainerSubscribeDisposing: StorageContainerSubscribeDisposingRoute
    let storageContainerUnsubscribeDisposing: StorageContainerUnsubscribeDisposingRoute
    let storageContainerCreateDirectory: StorageContainerCreateDirectoryRoute
    let storageContainerDirectoryExists: StorageContainerDirectoryExistsRoute
    let storageContainerDeleteDirectory: StorageContainerDeleteDirectoryRoute
    let storageContainerFileExists: StorageContainerFileExistsRoute
    let storageContainerDeleteFile: StorageContainerDeleteFileRoute
    let storageContainerGetDirectoryNameCount: StorageContainerGetDirectoryNameCountRoute
    let storageContainerCopyDirectoryName: StorageContainerCopyDirectoryNameRoute
    let storageContainerGetFileNameCount: StorageContainerGetFileNameCountRoute
    let storageContainerCopyFileName: StorageContainerCopyFileNameRoute
    let storageContainerCreateFile: StorageContainerCreateFileRoute
    let storageContainerOpenFile: StorageContainerOpenFileRoute
    let storageContainerOpenFileAccess: StorageContainerOpenFileAccessRoute
    let storageContainerOpenFileShare: StorageContainerOpenFileShareRoute
    let storageStreamRead: StorageStreamReadRoute
    let storageStreamWrite: StorageStreamWriteRoute
    let storageStreamSeek: StorageStreamSeekRoute
    let storageStreamGetPosition: StorageStreamGetPositionRoute
    let storageStreamGetLength: StorageStreamGetLengthRoute
    let storageStreamSetLength: StorageStreamSetLengthRoute
    let storageStreamGetCanRead: StorageStreamGetCanReadRoute
    let storageStreamGetCanWrite: StorageStreamGetCanWriteRoute
    let storageStreamGetCanSeek: StorageStreamGetCanSeekRoute
    let storageStreamFlush: StorageStreamFlushRoute
    let storageStreamClose: StorageStreamCloseRoute
    let storageContainerDestroy: StorageContainerDestroyRoute
    let mediaLibraryDispose: MediaLibraryDisposeRoute
    let mediaLibraryDestroy: MediaLibraryDestroyRoute
    let mediaLibraryGetIsDisposed: MediaLibraryGetIsDisposedRoute
    let mediaLibraryGetSongs: MediaLibraryGetSongsRoute
    let mediaLibraryGetArtists: MediaLibraryGetArtistsRoute
    let mediaLibraryGetAlbums: MediaLibraryGetAlbumsRoute
    let mediaLibraryGetGenres: MediaLibraryGetGenresRoute
    let songGetArtist: SongGetArtistRoute
    let songGetAlbum: SongGetAlbumRoute
    let songGetGenre: SongGetGenreRoute
    let songCollectionGetAt: SongCollectionGetAtRoute
    let songCollectionGetCount: SongCollectionGetCountRoute
    let songCollectionGetIsDisposed: SongCollectionGetIsDisposedRoute
    let songCollectionDispose: SongCollectionDisposeRoute
    let songCollectionDestroy: SongCollectionDestroyRoute
    let artistCollectionGetAt: ArtistCollectionGetAtRoute
    let artistCollectionGetCount: ArtistCollectionGetCountRoute
    let artistCollectionGetIsDisposed: ArtistCollectionGetIsDisposedRoute
    let artistCollectionDispose: ArtistCollectionDisposeRoute
    let artistCollectionDestroy: ArtistCollectionDestroyRoute
    let albumCollectionGetAt: AlbumCollectionGetAtRoute
    let albumCollectionGetCount: AlbumCollectionGetCountRoute
    let albumCollectionGetIsDisposed: AlbumCollectionGetIsDisposedRoute
    let albumCollectionDispose: AlbumCollectionDisposeRoute
    let albumCollectionDestroy: AlbumCollectionDestroyRoute
    let genreCollectionGetAt: GenreCollectionGetAtRoute
    let genreCollectionGetCount: GenreCollectionGetCountRoute
    let genreCollectionGetIsDisposed: GenreCollectionGetIsDisposedRoute
    let genreCollectionDispose: GenreCollectionDisposeRoute
    let genreCollectionDestroy: GenreCollectionDestroyRoute
    let artistGetNameSize: ArtistGetNameSizeRoute
    let artistCopyName: ArtistCopyNameRoute
    let artistGetIsDisposed: ArtistGetIsDisposedRoute
    let artistDispose: ArtistDisposeRoute
    let artistDestroy: ArtistDestroyRoute
    let artistEquals: ArtistEqualsRoute
    let artistGetHashCode: ArtistGetHashCodeRoute
    let artistGetSongs: ArtistGetSongsRoute
    let albumGetNameSize: AlbumGetNameSizeRoute
    let albumCopyName: AlbumCopyNameRoute
    let albumGetIsDisposed: AlbumGetIsDisposedRoute
    let albumDispose: AlbumDisposeRoute
    let albumDestroy: AlbumDestroyRoute
    let albumEquals: AlbumEqualsRoute
    let albumGetHashCode: AlbumGetHashCodeRoute
    let albumGetSongs: AlbumGetSongsRoute
    let genreGetNameSize: GenreGetNameSizeRoute
    let genreCopyName: GenreCopyNameRoute
    let genreGetIsDisposed: GenreGetIsDisposedRoute
    let genreDispose: GenreDisposeRoute
    let genreDestroy: GenreDestroyRoute
    let genreEquals: GenreEqualsRoute
    let genreGetHashCode: GenreGetHashCodeRoute
    let genreGetSongs: GenreGetSongsRoute
    let artistGetAlbums: ArtistGetAlbumsRoute
    let genreGetAlbums: GenreGetAlbumsRoute
    let albumGetArtist: AlbumGetArtistRoute
    let albumGetGenre: AlbumGetGenreRoute
    let albumGetDuration: AlbumGetDurationRoute
    let albumGetHasArt: AlbumGetHasArtRoute
    let albumGetArtSize: AlbumGetArtSizeRoute
    let albumCopyArt: AlbumCopyArtRoute
    let albumGetThumbnailSize: AlbumGetThumbnailSizeRoute
    let albumCopyThumbnail: AlbumCopyThumbnailRoute
    let songCreateFromUri: SongCreateFromUriRoute
    let songDestroy: SongDestroyRoute
    let songDispose: SongDisposeRoute
    let songGetIsDisposed: SongGetIsDisposedRoute
    let songGetNameSize: SongGetNameSizeRoute
    let songCopyName: SongCopyNameRoute
    let songGetDuration: SongGetDurationRoute
    let songGetIsRated: SongGetIsRatedRoute
    let songGetRating: SongGetRatingRoute
    let songGetPlayCount: SongGetPlayCountRoute
    let songGetTrackNumber: SongGetTrackNumberRoute
    let songGetIsProtected: SongGetIsProtectedRoute
    let songGetHashCode: SongGetHashCodeRoute
    let songEquals: SongEqualsRoute
    let dynamicSoundEffectInstanceCreate: DynamicSoundEffectInstanceCreateRoute
    let dynamicSoundEffectInstanceSubmitBuffer: DynamicSoundEffectInstanceSubmitBufferRoute
    let dynamicSoundEffectInstanceGetPendingBufferCount: DynamicSoundEffectInstanceGetPendingBufferCountRoute
    let dynamicSoundEffectInstanceGetSampleDurationTicks: DynamicSoundEffectInstanceGetSampleDurationTicksRoute
    let dynamicSoundEffectInstanceGetSampleSizeInBytes: DynamicSoundEffectInstanceGetSampleSizeInBytesRoute
    let dynamicSoundEffectInstanceSubscribeBufferNeeded: DynamicSoundEffectInstanceSubscribeBufferNeededRoute
    let audioUnsubscribe: AudioUnsubscribeExtRoute
    let audioEngineCreate: AudioEngineCreateRoute
    let audioEngineCreateWithRenderer: AudioEngineCreateWithRendererRoute
    let audioEngineDestroy: AudioEngineDestroyRoute
    let audioEngineGetRendererCount: AudioEngineGetRendererCountRoute
    let audioEngineGetRendererFriendlyNameSize: AudioEngineGetRendererFriendlyNameSizeRoute
    let audioEngineCopyRendererFriendlyName: AudioEngineCopyRendererFriendlyNameRoute
    let audioEngineGetRendererIdSize: AudioEngineGetRendererIdSizeRoute
    let audioEngineCopyRendererId: AudioEngineCopyRendererIdRoute
    let audioEngineGetGlobalVariable: AudioEngineGetGlobalVariableRoute
    let audioEngineSetGlobalVariable: AudioEngineSetGlobalVariableRoute
    let audioEngineUpdate: AudioEngineUpdateRoute
    let audioEngineGetCategory: AudioEngineGetCategoryRoute
    let audioCategoryDestroy: AudioCategoryDestroyRoute
    let audioCategoryGetNameSize: AudioCategoryGetNameSizeRoute
    let audioCategoryCopyName: AudioCategoryCopyNameRoute
    let audioCategoryPause: AudioCategoryPauseRoute
    let audioCategoryResume: AudioCategoryResumeRoute
    let audioCategorySetVolume: AudioCategorySetVolumeRoute
    let audioCategoryStop: AudioCategoryStopRoute
    let audioCategoryEquals: AudioCategoryEqualsRoute
    let audioCategoryGetHashCode: AudioCategoryGetHashCodeRoute
    let waveBankCreate: WaveBankCreateRoute
    let waveBankCreateStreaming: WaveBankCreateStreamingRoute
    let waveBankDestroy: WaveBankDestroyRoute
    let waveBankGetIsInUse: WaveBankGetIsInUseRoute
    let waveBankGetIsPrepared: WaveBankGetIsPreparedRoute
    let soundBankCreate: SoundBankCreateRoute
    let soundBankDestroy: SoundBankDestroyRoute
    let soundBankGetIsInUse: SoundBankGetIsInUseRoute
    let soundBankGetCue: SoundBankGetCueRoute
    let soundBankPlayCue: SoundBankPlayCueRoute
    let soundBankPlayCue3D: SoundBankPlayCue3dRoute
    let cueDestroy: CueDestroyRoute
    let cueGetInfo: CueGetInfoRoute
    let cueGetNameSize: CueGetNameSizeRoute
    let cueCopyName: CueCopyNameRoute
    let cueApply3D: CueApply3dRoute
    let cueGetVariable: CueGetVariableRoute
    let cueSetVariable: CueSetVariableRoute
    let cuePlay: CuePlayRoute
    let cuePause: CuePauseRoute
    let cueResume: CueResumeRoute
    let cueStop: CueStopRoute
    let microphoneGetCount: MicrophoneGetCountRoute
    let microphoneGetDefaultIndex: MicrophoneGetDefaultIndexExtRoute
    let microphoneGetNameSize: MicrophoneGetNameSizeAtRoute
    let microphoneCopyName: MicrophoneCopyNameAtRoute
    let microphoneGetBufferDuration: MicrophoneGetBufferDurationTicksAtRoute
    let microphoneSetBufferDuration: MicrophoneSetBufferDurationTicksAtRoute
    let microphoneGetIsHeadset: MicrophoneGetIsHeadsetAtRoute
    let microphoneGetSampleRate: MicrophoneGetSampleRateAtRoute
    let microphoneGetState: MicrophoneGetStateAtRoute
    let microphoneStart: MicrophoneStartAtRoute
    let microphoneStop: MicrophoneStopAtRoute
    let microphoneGetData: MicrophoneGetDataAtRoute
    let microphoneGetSampleDuration: MicrophoneGetSampleDurationTicksAtRoute
    let microphoneGetSampleSize: MicrophoneGetSampleSizeInBytesAtRoute
    let microphoneSubscribeBufferReady: MicrophoneSubscribeBufferReadyAtRoute
    let soundEffectInstanceApply3D: SoundEffectInstanceApply3dRoute
    let soundEffectInstanceApply3DMulti: SoundEffectInstanceApply3dMultiExtRoute
    let soundEffectCreatePcm16: SoundEffectCreatePcm16Route
    let soundEffectCreatePcm16Range: SoundEffectCreatePcm16RangeExtRoute
    let soundEffectCreateFromEncoded: SoundEffectCreateFromEncodedExtRoute
    let soundEffectDestroy: SoundEffectDestroyRoute
    let soundEffectGetIsDisposed: SoundEffectGetIsDisposedRoute
    let soundEffectGetNameSize: SoundEffectGetNameSizeRoute
    let soundEffectCopyName: SoundEffectCopyNameRoute
    let soundEffectSetName: SoundEffectSetNameRoute
    let soundEffectGetDurationTicks: SoundEffectGetDurationTicksRoute
    let soundEffectPlay: SoundEffectPlayRoute
    let soundEffectPlayWithSettings: SoundEffectPlayWithSettingsRoute
    let soundEffectCreateInstance: SoundEffectCreateInstanceRoute
    let soundEffectGetMasterVolume: SoundEffectGetMasterVolumeRoute
    let soundEffectSetMasterVolume: SoundEffectSetMasterVolumeRoute
    let soundEffectGetDistanceScale: SoundEffectGetDistanceScaleRoute
    let soundEffectSetDistanceScale: SoundEffectSetDistanceScaleRoute
    let soundEffectGetDopplerScale: SoundEffectGetDopplerScaleRoute
    let soundEffectSetDopplerScale: SoundEffectSetDopplerScaleRoute
    let soundEffectGetSpeedOfSound: SoundEffectGetSpeedOfSoundRoute
    let soundEffectSetSpeedOfSound: SoundEffectSetSpeedOfSoundRoute
    let soundEffectGetSampleSizeInBytes: SoundEffectGetSampleSizeInBytesRoute
    let soundEffectGetSampleDurationTicks: SoundEffectGetSampleDurationTicksRoute
    let soundEffectInstanceGetInfo: SoundEffectInstanceGetInfoRoute
    let soundEffectInstanceSetVolume: SoundEffectInstanceSetVolumeRoute
    let soundEffectInstanceSetPitch: SoundEffectInstanceSetPitchRoute
    let soundEffectInstanceSetPan: SoundEffectInstanceSetPanRoute
    let soundEffectInstanceSetIsLooped: SoundEffectInstanceSetIsLoopedRoute
    let soundEffectInstancePlay: SoundEffectInstancePlayRoute
    let soundEffectInstancePause: SoundEffectInstancePauseRoute
    let soundEffectInstanceResume: SoundEffectInstanceResumeRoute
    let soundEffectInstanceStop: SoundEffectInstanceStopRoute
    let soundEffectInstanceGetIsDisposed: SoundEffectInstanceGetIsDisposedRoute
    let soundEffectInstanceDestroy: SoundEffectInstanceDestroyRoute
    let occlusionQueryDestroy: OcclusionQueryDestroyRoute
    let occlusionQueryBegin: OcclusionQueryBeginRoute
    let occlusionQueryEnd: OcclusionQueryEndRoute
    let occlusionQueryGetIsComplete: OcclusionQueryGetIsCompleteRoute
    let occlusionQueryGetPixelCount: OcclusionQueryGetPixelCountRoute
    let renderTargetGetInfo: RenderTargetGetInfoRoute
    let renderTargetDestroy: RenderTargetDestroyRoute
    let graphicsDeviceSetRenderTarget2D: GraphicsDeviceSetRenderTarget2dRoute
    let renderTargetCubeCreate: RenderTargetCubeCreateRoute
    let graphicsDeviceSetRenderTargetCube: GraphicsDeviceSetRenderTargetCubeRoute
    let graphicsDeviceSetRenderTargets: GraphicsDeviceSetRenderTargetsRoute
    let graphicsDeviceGetRenderTargetCount: GraphicsDeviceGetRenderTargetCountRoute
    let graphicsDeviceGetTexture: GraphicsDeviceGetTextureRoute
    let graphicsDeviceSetTexture: GraphicsDeviceSetTextureRoute
    let graphicsDeviceDrawPrimitives: GraphicsDeviceDrawPrimitivesRoute
    let graphicsDeviceDrawIndexedPrimitives: GraphicsDeviceDrawIndexedPrimitivesRoute
    let graphicsDeviceDrawInstancedPrimitives: GraphicsDeviceDrawInstancedPrimitivesRoute
    let graphicsDeviceDrawUserPrimitives: GraphicsDeviceDrawUserPrimitivesRoute
    let graphicsDeviceDrawUserIndexedPrimitives: GraphicsDeviceDrawUserIndexedPrimitivesRoute
    let primitiveTypeGetVertexCount: PrimitiveTypeGetVertexCountRoute
    let effectMaterialCreate: EffectMaterialCreateRoute
    let directionalLightDestroy: DirectionalLightDestroyRoute
    let directionalLightGetDiffuseColor: DirectionalLightGetDiffuseColorRoute
    let directionalLightSetDiffuseColor: DirectionalLightSetDiffuseColorRoute
    let directionalLightGetDirection: DirectionalLightGetDirectionRoute
    let directionalLightSetDirection: DirectionalLightSetDirectionRoute
    let directionalLightGetSpecularColor: DirectionalLightGetSpecularColorRoute
    let directionalLightSetSpecularColor: DirectionalLightSetSpecularColorRoute
    let directionalLightGetEnabled: DirectionalLightGetEnabledRoute
    let directionalLightSetEnabled: DirectionalLightSetEnabledRoute
    let basicEffectCreate: BasicEffectCreateRoute
    let effectMatricesGetWorld: EffectMatricesGetWorldRoute
    let effectMatricesSetWorld: EffectMatricesSetWorldRoute
    let effectMatricesGetView: EffectMatricesGetViewRoute
    let effectMatricesSetView: EffectMatricesSetViewRoute
    let effectMatricesGetProjection: EffectMatricesGetProjectionRoute
    let effectMatricesSetProjection: EffectMatricesSetProjectionRoute
    let effectFogGetColor: EffectFogGetColorRoute
    let effectFogSetColor: EffectFogSetColorRoute
    let effectFogGetEnabled: EffectFogGetEnabledRoute
    let effectFogSetEnabled: EffectFogSetEnabledRoute
    let effectFogGetStart: EffectFogGetStartRoute
    let effectFogSetStart: EffectFogSetStartRoute
    let effectFogGetEnd: EffectFogGetEndRoute
    let effectFogSetEnd: EffectFogSetEndRoute
    let effectLightsGetAmbientColor: EffectLightsGetAmbientColorRoute
    let effectLightsSetAmbientColor: EffectLightsSetAmbientColorRoute
    let effectLightsGetDirectionalLight: EffectLightsGetDirectionalLightRoute
    let effectLightsGetEnabled: EffectLightsGetEnabledRoute
    let effectLightsSetEnabled: EffectLightsSetEnabledRoute
    let basicEffectGetVertexColorEnabled: BasicEffectGetVertexColorEnabledRoute
    let basicEffectSetVertexColorEnabled: BasicEffectSetVertexColorEnabledRoute
    let basicEffectGetPreferPerPixelLighting: BasicEffectGetPreferPerPixelLightingRoute
    let basicEffectSetPreferPerPixelLighting: BasicEffectSetPreferPerPixelLightingRoute
    let basicEffectGetDiffuseColor: BasicEffectGetDiffuseColorRoute
    let basicEffectSetDiffuseColor: BasicEffectSetDiffuseColorRoute
    let basicEffectGetEmissiveColor: BasicEffectGetEmissiveColorRoute
    let basicEffectSetEmissiveColor: BasicEffectSetEmissiveColorRoute
    let basicEffectGetSpecularColor: BasicEffectGetSpecularColorRoute
    let basicEffectSetSpecularColor: BasicEffectSetSpecularColorRoute
    let basicEffectGetSpecularPower: BasicEffectGetSpecularPowerRoute
    let basicEffectSetSpecularPower: BasicEffectSetSpecularPowerRoute
    let basicEffectGetAlpha: BasicEffectGetAlphaRoute
    let basicEffectSetAlpha: BasicEffectSetAlphaRoute
    let basicEffectGetTextureEnabled: BasicEffectGetTextureEnabledRoute
    let basicEffectSetTextureEnabled: BasicEffectSetTextureEnabledRoute
    let basicEffectGetTexture: BasicEffectGetTextureRoute
    let basicEffectSetTexture: BasicEffectSetTextureRoute
    let alphaTestEffectCreate: AlphaTestEffectCreateRoute
    let alphaTestEffectGetDiffuseColor: AlphaTestEffectGetDiffuseColorRoute
    let alphaTestEffectSetDiffuseColor: AlphaTestEffectSetDiffuseColorRoute
    let alphaTestEffectGetAlpha: AlphaTestEffectGetAlphaRoute
    let alphaTestEffectSetAlpha: AlphaTestEffectSetAlphaRoute
    let alphaTestEffectGetTexture: AlphaTestEffectGetTextureRoute
    let alphaTestEffectSetTexture: AlphaTestEffectSetTextureRoute
    let alphaTestEffectGetVertexColorEnabled: AlphaTestEffectGetVertexColorEnabledRoute
    let alphaTestEffectSetVertexColorEnabled: AlphaTestEffectSetVertexColorEnabledRoute
    let alphaTestEffectGetAlphaFunction: AlphaTestEffectGetAlphaFunctionRoute
    let alphaTestEffectSetAlphaFunction: AlphaTestEffectSetAlphaFunctionRoute
    let alphaTestEffectGetReferenceAlpha: AlphaTestEffectGetReferenceAlphaRoute
    let alphaTestEffectSetReferenceAlpha: AlphaTestEffectSetReferenceAlphaRoute
    let dualTextureEffectCreate: DualTextureEffectCreateRoute
    let dualTextureEffectGetDiffuseColor: DualTextureEffectGetDiffuseColorRoute
    let dualTextureEffectSetDiffuseColor: DualTextureEffectSetDiffuseColorRoute
    let dualTextureEffectGetAlpha: DualTextureEffectGetAlphaRoute
    let dualTextureEffectSetAlpha: DualTextureEffectSetAlphaRoute
    let dualTextureEffectGetTexture: DualTextureEffectGetTextureRoute
    let dualTextureEffectSetTexture: DualTextureEffectSetTextureRoute
    let dualTextureEffectGetVertexColorEnabled: DualTextureEffectGetVertexColorEnabledRoute
    let dualTextureEffectSetVertexColorEnabled: DualTextureEffectSetVertexColorEnabledRoute
    let environmentMapEffectCreate: EnvironmentMapEffectCreateRoute
    let environmentMapEffectGetDiffuseColor: EnvironmentMapEffectGetDiffuseColorRoute
    let environmentMapEffectSetDiffuseColor: EnvironmentMapEffectSetDiffuseColorRoute
    let environmentMapEffectGetEmissiveColor: EnvironmentMapEffectGetEmissiveColorRoute
    let environmentMapEffectSetEmissiveColor: EnvironmentMapEffectSetEmissiveColorRoute
    let environmentMapEffectGetAlpha: EnvironmentMapEffectGetAlphaRoute
    let environmentMapEffectSetAlpha: EnvironmentMapEffectSetAlphaRoute
    let environmentMapEffectGetTexture: EnvironmentMapEffectGetTextureRoute
    let environmentMapEffectSetTexture: EnvironmentMapEffectSetTextureRoute
    let environmentMapEffectGetEnvironmentMap: EnvironmentMapEffectGetEnvironmentMapRoute
    let environmentMapEffectSetEnvironmentMap: EnvironmentMapEffectSetEnvironmentMapRoute
    let environmentMapEffectGetAmount: EnvironmentMapEffectGetAmountRoute
    let environmentMapEffectSetAmount: EnvironmentMapEffectSetAmountRoute
    let environmentMapEffectGetSpecular: EnvironmentMapEffectGetSpecularRoute
    let environmentMapEffectSetSpecular: EnvironmentMapEffectSetSpecularRoute
    let environmentMapEffectGetFresnelFactor: EnvironmentMapEffectGetFresnelFactorRoute
    let environmentMapEffectSetFresnelFactor: EnvironmentMapEffectSetFresnelFactorRoute
    let skinnedEffectCreate: SkinnedEffectCreateRoute
    let skinnedEffectGetDiffuseColor: SkinnedEffectGetDiffuseColorRoute
    let skinnedEffectSetDiffuseColor: SkinnedEffectSetDiffuseColorRoute
    let skinnedEffectGetEmissiveColor: SkinnedEffectGetEmissiveColorRoute
    let skinnedEffectSetEmissiveColor: SkinnedEffectSetEmissiveColorRoute
    let skinnedEffectGetSpecularColor: SkinnedEffectGetSpecularColorRoute
    let skinnedEffectSetSpecularColor: SkinnedEffectSetSpecularColorRoute
    let skinnedEffectGetSpecularPower: SkinnedEffectGetSpecularPowerRoute
    let skinnedEffectSetSpecularPower: SkinnedEffectSetSpecularPowerRoute
    let skinnedEffectGetAlpha: SkinnedEffectGetAlphaRoute
    let skinnedEffectSetAlpha: SkinnedEffectSetAlphaRoute
    let skinnedEffectGetPreferPerPixelLighting: SkinnedEffectGetPreferPerPixelLightingRoute
    let skinnedEffectSetPreferPerPixelLighting: SkinnedEffectSetPreferPerPixelLightingRoute
    let skinnedEffectGetTexture: SkinnedEffectGetTextureRoute
    let skinnedEffectSetTexture: SkinnedEffectSetTextureRoute
    let skinnedEffectGetWeightsPerVertex: SkinnedEffectGetWeightsPerVertexRoute
    let skinnedEffectSetWeightsPerVertex: SkinnedEffectSetWeightsPerVertexRoute
    let skinnedEffectSetBoneTransforms: SkinnedEffectSetBoneTransformsRoute
    let skinnedEffectCopyBoneTransforms: SkinnedEffectCopyBoneTransformsRoute
    let effectCreateEmpty: EffectCreateEmptyRoute
    let effectCreateCompiled: EffectCreateCompiledRoute
    let effectDestroy: EffectDestroyRoute
    let effectClone: EffectCloneRoute
    let effectDisposeResources: EffectDisposeRoute
    let effectApply: EffectApplyRoute
    let effectGetParameters: EffectGetParametersRoute
    let effectGetTechniques: EffectGetTechniquesRoute
    let effectGetCurrentTechnique: EffectGetCurrentTechniqueRoute
    let effectSetCurrentTechnique: EffectSetCurrentTechniqueRoute
    let effectGetGraphicsDevice: EffectGetGraphicsDeviceRoute
    let effectTechniqueDestroy: EffectTechniqueDestroyRoute
    let effectTechniqueGetNameByteCount: EffectTechniqueGetNameByteCountRoute
    let effectTechniqueCopyName: EffectTechniqueCopyNameRoute
    let effectTechniqueGetPasses: EffectTechniqueGetPassesRoute
    let effectTechniqueGetAnnotations: EffectTechniqueGetAnnotationsRoute
    let effectTechniqueCollectionDestroy: EffectTechniqueCollectionDestroyRoute
    let effectTechniqueCollectionGetCount: EffectTechniqueCollectionGetCountRoute
    let effectTechniqueCollectionGetAt: EffectTechniqueCollectionGetAtRoute
    let effectPassDestroy: EffectPassDestroyRoute
    let effectPassGetNameByteCount: EffectPassGetNameByteCountRoute
    let effectPassCopyName: EffectPassCopyNameRoute
    let effectPassGetAnnotations: EffectPassGetAnnotationsRoute
    let effectPassApply: EffectPassApplyRoute
    let effectPassCollectionDestroy: EffectPassCollectionDestroyRoute
    let effectPassCollectionGetCount: EffectPassCollectionGetCountRoute
    let effectPassCollectionGetAt: EffectPassCollectionGetAtRoute
    let effectParameterDestroy: EffectParameterDestroyRoute
    let effectParameterGetInfo: EffectParameterGetInfoRoute
    let effectParameterGetNameByteCount: EffectParameterGetNameByteCountRoute
    let effectParameterCopyName: EffectParameterCopyNameRoute
    let effectParameterGetSemanticByteCount: EffectParameterGetSemanticByteCountRoute
    let effectParameterCopySemantic: EffectParameterCopySemanticRoute
    let effectParameterGetElements: EffectParameterGetElementsRoute
    let effectParameterGetStructureMembers: EffectParameterGetStructureMembersRoute
    let effectParameterGetAnnotations: EffectParameterGetAnnotationsRoute
    let effectParameterGetValue: EffectParameterGetValueRoute
    let effectParameterSetValue: EffectParameterSetValueRoute
    let effectParameterGetValues: EffectParameterGetValuesRoute
    let effectParameterSetValues: EffectParameterSetValuesRoute
    let effectParameterGetValueStringByteCount: EffectParameterGetValueStringByteCountRoute
    let effectParameterCopyValueString: EffectParameterCopyValueStringRoute
    let effectParameterSetValueString: EffectParameterSetValueStringRoute
    let effectParameterGetValueTexture: EffectParameterGetValueTextureRoute
    let effectParameterSetValueTexture: EffectParameterSetValueTextureRoute
    let effectParameterCollectionDestroy: EffectParameterCollectionDestroyRoute
    let effectParameterCollectionGetCount: EffectParameterCollectionGetCountRoute
    let effectParameterCollectionGetAt: EffectParameterCollectionGetAtRoute
    let effectAnnotationDestroy: EffectAnnotationDestroyRoute
    let effectAnnotationGetInfo: EffectAnnotationGetInfoRoute
    let effectAnnotationGetNameByteCount: EffectAnnotationGetNameByteCountRoute
    let effectAnnotationCopyName: EffectAnnotationCopyNameRoute
    let effectAnnotationGetSemanticByteCount: EffectAnnotationGetSemanticByteCountRoute
    let effectAnnotationCopySemantic: EffectAnnotationCopySemanticRoute
    let effectAnnotationGetValueBoolean: EffectAnnotationGetValueBooleanRoute
    let effectAnnotationGetValueInt32: EffectAnnotationGetValueInt32Route
    let effectAnnotationGetValueSingle: EffectAnnotationGetValueSingleRoute
    let effectAnnotationGetValueVector2: EffectAnnotationGetValueVector2Route
    let effectAnnotationGetValueVector3: EffectAnnotationGetValueVector3Route
    let effectAnnotationGetValueVector4: EffectAnnotationGetValueVector4Route
    let effectAnnotationGetValueMatrix: EffectAnnotationGetValueMatrixRoute
    let effectAnnotationGetValueStringByteCount: EffectAnnotationGetValueStringByteCountRoute
    let effectAnnotationCopyValueString: EffectAnnotationCopyValueStringRoute
    let effectAnnotationCollectionDestroy: EffectAnnotationCollectionDestroyRoute
    let effectAnnotationCollectionGetCount: EffectAnnotationCollectionGetCountRoute
    let effectAnnotationCollectionGetAt: EffectAnnotationCollectionGetAtRoute
    let graphicsDeviceClearOptions: GraphicsDeviceClearOptionsRoute
    let graphicsDeviceCreate: GraphicsDeviceCreateRoute
    let graphicsDeviceDestroy: GraphicsDeviceDestroyRoute
    let graphicsDeviceGetIsDisposed: GraphicsDeviceGetIsDisposedRoute
    let graphicsDeviceDispose: GraphicsDeviceDisposeRoute
    let graphicsDevicePresent: GraphicsDevicePresentRoute
    let graphicsDeviceReset: GraphicsDeviceResetRoute
    let graphicsDeviceResetWithParameters: GraphicsDeviceResetWithParametersRoute
    let graphicsDeviceGetPresentationParameters: GraphicsDeviceGetPresentationParametersRoute
    let graphicsDeviceSetViewport: GraphicsDeviceSetViewportRoute
    let graphicsDeviceGetScissorRectangle: GraphicsDeviceGetScissorRectangleRoute
    let graphicsDeviceSetScissorRectangle: GraphicsDeviceSetScissorRectangleRoute
    let graphicsDeviceGetStatus: GraphicsDeviceGetStatusRoute
    let graphicsDeviceGetBlendFactor: GraphicsDeviceGetBlendFactorRoute
    let graphicsDeviceSetBlendFactor: GraphicsDeviceSetBlendFactorRoute
    let graphicsDeviceGetMultiSampleMask: GraphicsDeviceGetMultiSampleMaskRoute
    let graphicsDeviceSetMultiSampleMask: GraphicsDeviceSetMultiSampleMaskRoute
    let graphicsDeviceGetReferenceStencil: GraphicsDeviceGetReferenceStencilRoute
    let graphicsDeviceSetReferenceStencil: GraphicsDeviceSetReferenceStencilRoute
    let graphicsDeviceGetBlendState: GraphicsDeviceGetBlendStateRoute
    let graphicsDeviceSetBlendState: GraphicsDeviceSetBlendStateRoute
    let graphicsDeviceGetDepthStencilState: GraphicsDeviceGetDepthStencilStateRoute
    let graphicsDeviceSetDepthStencilState: GraphicsDeviceSetDepthStencilStateRoute
    let graphicsDeviceGetRasterizerState: GraphicsDeviceGetRasterizerStateRoute
    let graphicsDeviceSetRasterizerState: GraphicsDeviceSetRasterizerStateRoute
    let graphicsDeviceGetSamplerState: GraphicsDeviceGetSamplerStateRoute
    let graphicsDeviceSetSamplerState: GraphicsDeviceSetSamplerStateRoute
    let renderTargetSubscribeContentLost: RenderTargetSubscribeContentLostRoute
    let renderTargetUnsubscribeContentLost: RenderTargetUnsubscribeContentLostRoute
    let gameTick: GameTickRoute
    let gameSuppressDraw: GameSuppressDrawRoute
    let gameResetElapsedTime: GameResetElapsedTimeRoute
    let gameGetIsActive: GameGetIsActiveRoute
    let gameGetIsMouseVisible: GameGetIsMouseVisibleRoute
    let gameSetIsMouseVisible: GameSetIsMouseVisibleRoute
    let gameGetIsFixedTimeStep: GameGetIsFixedTimeStepRoute
    let gameSetIsFixedTimeStep: GameSetIsFixedTimeStepRoute
    let gameGetTargetElapsedTimeTicks: GameGetTargetElapsedTimeTicksRoute
    let gameSetTargetElapsedTimeTicks: GameSetTargetElapsedTimeTicksRoute
    let gameGetInactiveSleepTimeTicks: GameGetInactiveSleepTimeTicksRoute
    let gameSetInactiveSleepTimeTicks: GameSetInactiveSleepTimeTicksRoute
    let gameSubscribe: GameSubscribeRoute
    let gameUnsubscribe: GameUnsubscribeRoute
    let graphicsManagerCreateDevice: GraphicsDeviceManagerCreateDeviceRoute
    let graphicsManagerBeginDraw: GraphicsDeviceManagerBeginDrawRoute
    let graphicsManagerEndDraw: GraphicsDeviceManagerEndDrawRoute
    let graphicsManagerGetGraphicsDevice: GraphicsDeviceManagerGetGraphicsDeviceRoute
    let graphicsManagerSubscribe: GraphicsDeviceManagerSubscribeRoute

    static func load() throws -> NativeFunctions {
        lock.lock()
        defer { lock.unlock() }
        if let cached { return try cached.get() }
        do {
            let functions = try NativeFunctions()
            cached = .success(functions)
            return functions
        } catch {
            cached = .failure(error)
            throw error
        }
    }

    private init() throws {
        library = try NativeLibrary()
        getABIVersion = try library.resolve("cna_get_abi_version", as: GetAbiVersionRoute.self)
        let actual = getABIVersion()
        guard NativeABI.admits(actual) else {
            throw CNAError.unsupportedABIVersion(
                admitted: NativeABI.admittedDescription,
                actual: NativeABI.describe(actual),
                actualEncoded: actual,
                path: library.admittedPath
            )
        }
        abiVersion = actual

        errorMessageSize = try library.resolve("cna_error_get_last_message_size", as: ErrorGetLastMessageSizeRoute.self)
        errorMessageCopy = try library.resolve("cna_error_copy_last_message", as: ErrorCopyLastMessageRoute.self)
        gameCreate = try library.resolve("cna_game_create", as: GameCreateRoute.self)
        gameSetFrameHooks = try library.resolve("cna_game_set_frame_hooks_ext", as: GameSetFrameHooksExtRoute.self)
        gameRun = try library.resolve("cna_game_run", as: GameRunRoute.self)
        gameRunOneFrame = try library.resolve("cna_game_run_one_frame", as: GameRunOneFrameRoute.self)
        frameworkDispatcherUpdate = try library.resolve("cna_framework_dispatcher_update", as: FrameworkDispatcherUpdateRoute.self)
        titleContainerRead = try library.resolve("cna_title_container_read_ext", as: TitleContainerReadExtRoute.self)
        gameRequestExit = try library.resolve("cna_game_request_exit", as: GameRequestExitRoute.self)
        gameDestroy = try library.resolve("cna_game_destroy", as: GameDestroyRoute.self)
        gameGetGraphicsDevice = try library.resolve("cna_game_get_graphics_device", as: GameGetGraphicsDeviceRoute.self)
        graphicsManagerCreate = try library.resolve("cna_graphics_device_manager_create", as: GraphicsDeviceManagerCreateRoute.self)
        graphicsManagerApplyChanges = try library.resolve("cna_graphics_device_manager_apply_changes", as: GraphicsDeviceManagerApplyChangesRoute.self)
        graphicsManagerSetGraphicsProfile = try library.resolve("cna_graphics_device_manager_set_graphics_profile", as: GraphicsDeviceManagerSetGraphicsProfileRoute.self)
        graphicsManagerSetIsFullScreen = try library.resolve("cna_graphics_device_manager_set_is_full_screen", as: GraphicsDeviceManagerSetIsFullScreenRoute.self)
        graphicsManagerSetPreferMultiSampling = try library.resolve("cna_graphics_device_manager_set_prefer_multi_sampling", as: GraphicsDeviceManagerSetPreferMultiSamplingRoute.self)
        graphicsManagerSetPreferredBackBufferFormat = try library.resolve("cna_graphics_device_manager_set_preferred_back_buffer_format", as: GraphicsDeviceManagerSetPreferredBackBufferFormatRoute.self)
        graphicsManagerSetPreferredBackBufferWidth = try library.resolve("cna_graphics_device_manager_set_preferred_back_buffer_width", as: GraphicsDeviceManagerSetPreferredBackBufferWidthRoute.self)
        graphicsManagerSetPreferredBackBufferHeight = try library.resolve("cna_graphics_device_manager_set_preferred_back_buffer_height", as: GraphicsDeviceManagerSetPreferredBackBufferHeightRoute.self)
        graphicsManagerSetPreferredDepthStencilFormat = try library.resolve("cna_graphics_device_manager_set_preferred_depth_stencil_format", as: GraphicsDeviceManagerSetPreferredDepthStencilFormatRoute.self)
        graphicsManagerSetSynchronizeWithVerticalRetrace = try library.resolve("cna_graphics_device_manager_set_synchronize_with_vertical_retrace", as: GraphicsDeviceManagerSetSynchronizeWithVerticalRetraceRoute.self)
        graphicsManagerSetSupportedOrientations = try library.resolve("cna_graphics_device_manager_set_supported_orientations", as: GraphicsDeviceManagerSetSupportedOrientationsRoute.self)
        graphicsManagerDestroy = try library.resolve("cna_graphics_device_manager_destroy", as: GraphicsDeviceManagerDestroyRoute.self)
        graphicsDeviceGetViewport = try library.resolve("cna_graphics_device_get_viewport", as: GraphicsDeviceGetViewportRoute.self)
        textureCreateMemory = try library.resolve("cna_texture2d_create_from_encoded_memory", as: Texture2dCreateFromEncodedMemoryRoute.self)
        textureGetInfo = try library.resolve("cna_texture2d_get_info", as: Texture2dGetInfoRoute.self)
        textureCreate = try library.resolve("cna_texture2d_create", as: Texture2dCreateRoute.self)
        textureDestroy = try library.resolve("cna_texture2d_destroy", as: Texture2dDestroyRoute.self)
        textureSetData = try library.resolve("cna_texture2d_set_data", as: Texture2dSetDataRoute.self)
        textureGetData = try library.resolve("cna_texture2d_get_data", as: Texture2dGetDataRoute.self)
        textureGetEncodedByteCount = try library.resolve("cna_texture2d_get_encoded_byte_count", as: Texture2dGetEncodedByteCountRoute.self)
        textureCopyEncoded = try library.resolve("cna_texture2d_copy_encoded", as: Texture2dCopyEncodedRoute.self)
        textureCreateCpuOnly = try library.resolve("cna_texture2d_create_cpu_only_rgba8", as: Texture2dCreateCpuOnlyRgba8Route.self)
        vertexDeclarationCreateWithStride = try library.resolve("cna_vertex_declaration_create_with_stride", as: VertexDeclarationCreateWithStrideRoute.self)
        vertexDeclarationDestroy = try library.resolve("cna_vertex_declaration_destroy", as: VertexDeclarationDestroyRoute.self)
        vertexBufferCreate = try library.resolve("cna_vertex_buffer_create", as: VertexBufferCreateRoute.self)
        vertexBufferDestroy = try library.resolve("cna_vertex_buffer_destroy", as: VertexBufferDestroyRoute.self)
        vertexBufferSetDataRawAt = try library.resolve("cna_vertex_buffer_set_data_raw_at", as: VertexBufferSetDataRawAtRoute.self)
        vertexBufferGetDataRaw = try library.resolve("cna_vertex_buffer_get_data_raw", as: VertexBufferGetDataRawRoute.self)
        indexBufferCreate = try library.resolve("cna_index_buffer_create", as: IndexBufferCreateRoute.self)
        indexBufferDestroy = try library.resolve("cna_index_buffer_destroy", as: IndexBufferDestroyRoute.self)
        indexBufferSetDataAt = try library.resolve("cna_index_buffer_set_data_at", as: IndexBufferSetDataAtRoute.self)
        indexBufferGetData = try library.resolve("cna_index_buffer_get_data", as: IndexBufferGetDataRoute.self)
        vertexBufferSetDataRawAtWithOptions = try library.resolve("cna_vertex_buffer_set_data_raw_at_with_options", as: VertexBufferSetDataRawAtWithOptionsRoute.self)
        vertexBufferSubscribeContentLost = try library.resolve("cna_vertex_buffer_subscribe_content_lost", as: VertexBufferSubscribeContentLostRoute.self)
        vertexBufferUnsubscribeContentLost = try library.resolve("cna_vertex_buffer_unsubscribe_content_lost", as: VertexBufferUnsubscribeContentLostRoute.self)
        indexBufferSetData = try library.resolve("cna_index_buffer_set_data", as: IndexBufferSetDataRoute.self)
        indexBufferSubscribeContentLost = try library.resolve("cna_index_buffer_subscribe_content_lost", as: IndexBufferSubscribeContentLostRoute.self)
        indexBufferUnsubscribeContentLost = try library.resolve("cna_index_buffer_unsubscribe_content_lost", as: IndexBufferUnsubscribeContentLostRoute.self)
        graphicsDeviceGetGraphicsProfile = try library.resolve("cna_graphics_device_get_graphics_profile", as: GraphicsDeviceGetGraphicsProfileRoute.self)
        graphicsDeviceGetBackBufferData = try library.resolve("cna_graphics_device_get_backbuffer_data_window", as: GraphicsDeviceGetBackbufferDataWindowRoute.self)
        graphicsDeviceSetVertexBuffers = try library.resolve("cna_graphics_device_set_vertex_buffers", as: GraphicsDeviceSetVertexBuffersRoute.self)
        graphicsDeviceSetIndexBuffer = try library.resolve("cna_graphics_device_set_index_buffer", as: GraphicsDeviceSetIndexBufferRoute.self)
        textureCubeCreate = try library.resolve("cna_texturecube_create", as: TexturecubeCreateRoute.self)
        textureCubeDestroy = try library.resolve("cna_texturecube_destroy", as: TexturecubeDestroyRoute.self)
        textureCubeGetInfo = try library.resolve("cna_texturecube_get_info", as: TexturecubeGetInfoRoute.self)
        textureCubeSetData = try library.resolve("cna_texturecube_set_data", as: TexturecubeSetDataRoute.self)
        textureCubeGetData = try library.resolve("cna_texturecube_get_data", as: TexturecubeGetDataRoute.self)
        texture3DCreate = try library.resolve("cna_texture3d_create", as: Texture3dCreateRoute.self)
        texture3DDestroy = try library.resolve("cna_texture3d_destroy", as: Texture3dDestroyRoute.self)
        texture3DGetInfo = try library.resolve("cna_texture3d_get_info", as: Texture3dGetInfoRoute.self)
        texture3DSetData = try library.resolve("cna_texture3d_set_data", as: Texture3dSetDataRoute.self)
        texture3DGetData = try library.resolve("cna_texture3d_get_data", as: Texture3dGetDataRoute.self)
        spriteBatchCreate = try library.resolve("cna_sprite_batch_create", as: SpriteBatchCreateRoute.self)
        spriteBatchBeginWithEffect = try library.resolve("cna_sprite_batch_begin_with_effect", as: SpriteBatchBeginWithEffectRoute.self)
        spriteBatchBeginWithStates = try library.resolve("cna_sprite_batch_begin_with_states", as: SpriteBatchBeginWithStatesRoute.self)
        spriteBatchSubmitScaled = try library.resolve("cna_sprite_batch_submit_scaled_many", as: SpriteBatchSubmitScaledManyRoute.self)
        spriteFontCreate = try library.resolve("cna_sprite_font_create", as: SpriteFontCreateRoute.self)
        spriteFontDestroy = try library.resolve("cna_sprite_font_destroy", as: SpriteFontDestroyRoute.self)
        spriteFontGetInfo = try library.resolve("cna_sprite_font_get_info", as: SpriteFontGetInfoRoute.self)
        spriteFontCopyCharacters = try library.resolve("cna_sprite_font_copy_characters", as: SpriteFontCopyCharactersRoute.self)
        spriteFontCopyGlyphs = try library.resolve("cna_sprite_font_copy_glyphs", as: SpriteFontCopyGlyphsRoute.self)
        spriteFontSetDefaultCharacter = try library.resolve("cna_sprite_font_set_default_character", as: SpriteFontSetDefaultCharacterRoute.self)
        spriteFontSetLineSpacing = try library.resolve("cna_sprite_font_set_line_spacing", as: SpriteFontSetLineSpacingRoute.self)
        spriteFontSetSpacing = try library.resolve("cna_sprite_font_set_spacing", as: SpriteFontSetSpacingRoute.self)
        spriteBatchDrawString = try library.resolve("cna_sprite_batch_draw_string", as: SpriteBatchDrawStringRoute.self)
        spriteBatchSubmit = try library.resolve("cna_sprite_batch_submit_many", as: SpriteBatchSubmitManyRoute.self)
        spriteBatchEnd = try library.resolve("cna_sprite_batch_end", as: SpriteBatchEndRoute.self)
        spriteBatchDestroy = try library.resolve("cna_sprite_batch_destroy", as: SpriteBatchDestroyRoute.self)
        graphicsAdapterGetCount = try library.resolve("cna_graphics_adapter_get_count", as: GraphicsAdapterGetCountRoute.self)
        graphicsAdapterGetInfo = try library.resolve("cna_graphics_adapter_get_info", as: GraphicsAdapterGetInfoRoute.self)
        graphicsAdapterCopyDescription = try library.resolve("cna_graphics_adapter_copy_description", as: GraphicsAdapterCopyDescriptionRoute.self)
        graphicsAdapterCopyDeviceName = try library.resolve("cna_graphics_adapter_copy_device_name", as: GraphicsAdapterCopyDeviceNameRoute.self)
        graphicsAdapterGetCurrentDisplayMode = try library.resolve("cna_graphics_adapter_get_current_display_mode", as: GraphicsAdapterGetCurrentDisplayModeRoute.self)
        graphicsAdapterGetDisplayModeCount = try library.resolve("cna_graphics_adapter_get_display_mode_count", as: GraphicsAdapterGetDisplayModeCountRoute.self)
        graphicsAdapterCopyDisplayModes = try library.resolve("cna_graphics_adapter_copy_display_modes", as: GraphicsAdapterCopyDisplayModesRoute.self)
        graphicsAdapterSetDevicePreferences = try library.resolve("cna_graphics_adapter_set_device_preferences", as: GraphicsAdapterSetDevicePreferencesRoute.self)
        graphicsAdapterIsProfileSupported = try library.resolve("cna_graphics_adapter_is_profile_supported", as: GraphicsAdapterIsProfileSupportedRoute.self)
        graphicsAdapterQueryBackbufferFormat = try library.resolve("cna_graphics_adapter_query_backbuffer_format", as: GraphicsAdapterQueryBackbufferFormatRoute.self)
        graphicsAdapterQueryRenderTargetFormat = try library.resolve("cna_graphics_adapter_query_render_target_format", as: GraphicsAdapterQueryRenderTargetFormatRoute.self)
        gameWindowGetTitleSize = try library.resolve("cna_game_window_get_title_size", as: GameWindowGetTitleSizeRoute.self)
        gameWindowCopyTitle = try library.resolve("cna_game_window_copy_title", as: GameWindowCopyTitleRoute.self)
        gameWindowGetScreenDeviceNameSize = try library.resolve("cna_game_window_get_screen_device_name_size", as: GameWindowGetScreenDeviceNameSizeRoute.self)
        gameWindowCopyScreenDeviceName = try library.resolve("cna_game_window_copy_screen_device_name", as: GameWindowCopyScreenDeviceNameRoute.self)
        gameWindowGetNativeHandle = try library.resolve("cna_game_window_get_native_handle_ext", as: GameWindowGetNativeHandleExtRoute.self)
        gameWindowGetClientBounds = try library.resolve("cna_game_window_get_client_bounds", as: GameWindowGetClientBoundsRoute.self)
        gameWindowGetCurrentOrientation = try library.resolve("cna_game_window_get_current_orientation", as: GameWindowGetCurrentOrientationRoute.self)
        gameWindowGetAllowUserResizing = try library.resolve("cna_game_window_get_allow_user_resizing", as: GameWindowGetAllowUserResizingRoute.self)
        gameWindowSetAllowUserResizing = try library.resolve("cna_game_window_set_allow_user_resizing", as: GameWindowSetAllowUserResizingRoute.self)
        gameWindowBeginScreenDeviceChange = try library.resolve("cna_game_window_begin_screen_device_change", as: GameWindowBeginScreenDeviceChangeRoute.self)
        gameWindowEndScreenDeviceChange = try library.resolve("cna_game_window_end_screen_device_change", as: GameWindowEndScreenDeviceChangeRoute.self)
        gameSetWindowTitle = try library.resolve("cna_game_set_window_title", as: GameSetWindowTitleRoute.self)
        contentManagerCreate = try library.resolve("cna_content_manager_create", as: ContentManagerCreateRoute.self)
        contentManagerDestroy = try library.resolve("cna_content_manager_destroy", as: ContentManagerDestroyRoute.self)
        contentManagerUnload = try library.resolve("cna_content_manager_unload", as: ContentManagerUnloadRoute.self)
        contentManagerGetRootDirectorySize = try library.resolve("cna_content_manager_get_root_directory_size", as: ContentManagerGetRootDirectorySizeRoute.self)
        contentManagerCopyRootDirectory = try library.resolve("cna_content_manager_copy_root_directory", as: ContentManagerCopyRootDirectoryRoute.self)
        contentManagerLoadTexture2d = try library.resolve("cna_content_manager_load_texture2d", as: ContentManagerLoadTexture2dRoute.self)
        mouseGetState = try library.resolve("cna_mouse_get_state", as: MouseGetStateRoute.self)
        mouseSetPosition = try library.resolve("cna_mouse_set_position", as: MouseSetPositionRoute.self)
        mouseSetWindowHandle = try library.resolve("cna_mouse_set_window_handle", as: MouseSetWindowHandleRoute.self)
        keyboardGetState = try library.resolve("cna_keyboard_get_state", as: KeyboardGetStateRoute.self)
        keyboardGetStateForPlayer = try library.resolve("cna_keyboard_get_state_for_player", as: KeyboardGetStateForPlayerRoute.self)
        gamePadGetState = try library.resolve("cna_gamepad_get_state", as: GamepadGetStateRoute.self)
        gamePadGetStateWithDeadZone = try library.resolve("cna_gamepad_get_state_with_dead_zone", as: GamepadGetStateWithDeadZoneRoute.self)
        gamePadGetCapabilities = try library.resolve("cna_gamepad_get_capabilities", as: GamepadGetCapabilitiesRoute.self)
        gamePadSetVibration = try library.resolve("cna_gamepad_set_vibration", as: GamepadSetVibrationRoute.self)
        textureCommonGetInfo = try library.resolve("cna_texture_get_info", as: TextureGetInfoRoute.self)
        renderTarget2DCreate = try library.resolve("cna_render_target2d_create", as: RenderTarget2dCreateRoute.self)
        occlusionQueryCreate = try library.resolve("cna_occlusion_query_create", as: OcclusionQueryCreateRoute.self)
        touchGetCapabilities = try library.resolve("cna_touch_get_capabilities", as: TouchGetCapabilitiesRoute.self)
        touchGetState = try library.resolve("cna_touch_get_state", as: TouchGetStateRoute.self)
        touchPanelReadGesture = try library.resolve("cna_touch_panel_read_gesture", as: TouchPanelReadGestureRoute.self)
        touchPanelGetIsGestureAvailable = try library.resolve("cna_touch_panel_get_is_gesture_available", as: TouchPanelGetIsGestureAvailableRoute.self)
        touchPanelGetEnabledGestures = try library.resolve("cna_touch_panel_get_enabled_gestures", as: TouchPanelGetEnabledGesturesRoute.self)
        touchPanelSetEnabledGestures = try library.resolve("cna_touch_panel_set_enabled_gestures", as: TouchPanelSetEnabledGesturesRoute.self)
        touchPanelGetWindowHandle = try library.resolve("cna_touch_panel_get_window_handle", as: TouchPanelGetWindowHandleRoute.self)
        touchPanelSetWindowHandle = try library.resolve("cna_touch_panel_set_window_handle", as: TouchPanelSetWindowHandleRoute.self)
        touchPanelGetDisplayOrientation = try library.resolve("cna_touch_panel_get_display_orientation", as: TouchPanelGetDisplayOrientationRoute.self)
        touchPanelSetDisplayOrientation = try library.resolve("cna_touch_panel_set_display_orientation", as: TouchPanelSetDisplayOrientationRoute.self)
        touchPanelGetDisplayWidth = try library.resolve("cna_touch_panel_get_display_width", as: TouchPanelGetDisplayWidthRoute.self)
        touchPanelSetDisplayWidth = try library.resolve("cna_touch_panel_set_display_width", as: TouchPanelSetDisplayWidthRoute.self)
        touchPanelGetDisplayHeight = try library.resolve("cna_touch_panel_get_display_height", as: TouchPanelGetDisplayHeightRoute.self)
        touchPanelSetDisplayHeight = try library.resolve("cna_touch_panel_set_display_height", as: TouchPanelSetDisplayHeightRoute.self)
        gamerServicesDispatcherInitialize = try library.resolve("cna_gamer_services_dispatcher_initialize", as: GamerServicesDispatcherInitializeRoute.self)
        gamerServicesDispatcherUpdate = try library.resolve("cna_gamer_services_dispatcher_update", as: GamerServicesDispatcherUpdateRoute.self)
        videoPlayerCreate = try library.resolve("cna_video_player_create", as: VideoPlayerCreateRoute.self)
        videoPlayerDispose = try library.resolve("cna_video_player_dispose", as: VideoPlayerDisposeRoute.self)
        videoPlayerDestroy = try library.resolve("cna_video_player_destroy", as: VideoPlayerDestroyRoute.self)
        videoPlayerGetIsDisposed = try library.resolve("cna_video_player_get_is_disposed", as: VideoPlayerGetIsDisposedRoute.self)
        videoPlayerPlay = try library.resolve("cna_video_player_play", as: VideoPlayerPlayRoute.self)
        videoPlayerPause = try library.resolve("cna_video_player_pause", as: VideoPlayerPauseRoute.self)
        videoPlayerResume = try library.resolve("cna_video_player_resume", as: VideoPlayerResumeRoute.self)
        videoPlayerStop = try library.resolve("cna_video_player_stop", as: VideoPlayerStopRoute.self)
        videoPlayerGetVideo = try library.resolve("cna_video_player_get_video", as: VideoPlayerGetVideoRoute.self)
        videoPlayerGetTexture = try library.resolve("cna_video_player_get_texture", as: VideoPlayerGetTextureRoute.self)
        videoPlayerGetState = try library.resolve("cna_video_player_get_state", as: VideoPlayerGetStateRoute.self)
        videoPlayerGetPlayPositionTicks = try library.resolve("cna_video_player_get_play_position_ticks", as: VideoPlayerGetPlayPositionTicksRoute.self)
        videoPlayerGetIsLooped = try library.resolve("cna_video_player_get_is_looped", as: VideoPlayerGetIsLoopedRoute.self)
        videoPlayerSetIsLooped = try library.resolve("cna_video_player_set_is_looped", as: VideoPlayerSetIsLoopedRoute.self)
        videoPlayerGetIsMuted = try library.resolve("cna_video_player_get_is_muted", as: VideoPlayerGetIsMutedRoute.self)
        videoPlayerSetIsMuted = try library.resolve("cna_video_player_set_is_muted", as: VideoPlayerSetIsMutedRoute.self)
        videoPlayerGetVolume = try library.resolve("cna_video_player_get_volume", as: VideoPlayerGetVolumeRoute.self)
        videoPlayerSetVolume = try library.resolve("cna_video_player_set_volume", as: VideoPlayerSetVolumeRoute.self)
        videoGetDuration = try library.resolve("cna_video_get_duration", as: VideoGetDurationRoute.self)
        videoGetWidth = try library.resolve("cna_video_get_width", as: VideoGetWidthRoute.self)
        videoGetHeight = try library.resolve("cna_video_get_height", as: VideoGetHeightRoute.self)
        videoGetFramesPerSecond = try library.resolve("cna_video_get_frames_per_second", as: VideoGetFramesPerSecondRoute.self)
        videoGetSoundtrackType = try library.resolve("cna_video_get_soundtrack_type", as: VideoGetSoundtrackTypeRoute.self)
        mediaQueueGetCount = try library.resolve("cna_media_queue_get_count", as: MediaQueueGetCountRoute.self)
        mediaQueueGetActiveSongIndex = try library.resolve("cna_media_queue_get_active_song_index", as: MediaQueueGetActiveSongIndexRoute.self)
        mediaQueueSetActiveSongIndex = try library.resolve("cna_media_queue_set_active_song_index", as: MediaQueueSetActiveSongIndexRoute.self)
        mediaQueueGetActiveSong = try library.resolve("cna_media_queue_get_active_song", as: MediaQueueGetActiveSongRoute.self)
        mediaQueueGetAt = try library.resolve("cna_media_queue_get_at", as: MediaQueueGetAtRoute.self)
        mediaQueueDestroy = try library.resolve("cna_media_queue_destroy", as: MediaQueueDestroyRoute.self)
        mediaPlayerPlaySong = try library.resolve("cna_media_player_play_song", as: MediaPlayerPlaySongRoute.self)
        mediaPlayerPlaySongs = try library.resolve("cna_media_player_play_songs", as: MediaPlayerPlaySongsRoute.self)
        mediaPlayerPlaySongsFrom = try library.resolve("cna_media_player_play_songs_from", as: MediaPlayerPlaySongsFromRoute.self)
        mediaPlayerPause = try library.resolve("cna_media_player_pause", as: MediaPlayerPauseRoute.self)
        mediaPlayerResume = try library.resolve("cna_media_player_resume", as: MediaPlayerResumeRoute.self)
        mediaPlayerStop = try library.resolve("cna_media_player_stop", as: MediaPlayerStopRoute.self)
        mediaPlayerMoveNext = try library.resolve("cna_media_player_move_next", as: MediaPlayerMoveNextRoute.self)
        mediaPlayerMovePrevious = try library.resolve("cna_media_player_move_previous", as: MediaPlayerMovePreviousRoute.self)
        mediaPlayerGetVisualizationData = try library.resolve("cna_media_player_get_visualization_data", as: MediaPlayerGetVisualizationDataRoute.self)
        mediaPlayerGetQueue = try library.resolve("cna_media_player_get_queue", as: MediaPlayerGetQueueRoute.self)
        mediaPlayerGetState = try library.resolve("cna_media_player_get_state", as: MediaPlayerGetStateRoute.self)
        mediaPlayerGetPlayPositionTicks = try library.resolve("cna_media_player_get_play_position_ticks", as: MediaPlayerGetPlayPositionTicksRoute.self)
        mediaPlayerGetVolume = try library.resolve("cna_media_player_get_volume", as: MediaPlayerGetVolumeRoute.self)
        mediaPlayerSetVolume = try library.resolve("cna_media_player_set_volume", as: MediaPlayerSetVolumeRoute.self)
        mediaPlayerGetIsMuted = try library.resolve("cna_media_player_get_is_muted", as: MediaPlayerGetIsMutedRoute.self)
        mediaPlayerSetIsMuted = try library.resolve("cna_media_player_set_is_muted", as: MediaPlayerSetIsMutedRoute.self)
        mediaPlayerGetIsRepeating = try library.resolve("cna_media_player_get_is_repeating", as: MediaPlayerGetIsRepeatingRoute.self)
        mediaPlayerSetIsRepeating = try library.resolve("cna_media_player_set_is_repeating", as: MediaPlayerSetIsRepeatingRoute.self)
        mediaPlayerGetIsShuffled = try library.resolve("cna_media_player_get_is_shuffled", as: MediaPlayerGetIsShuffledRoute.self)
        mediaPlayerSetIsShuffled = try library.resolve("cna_media_player_set_is_shuffled", as: MediaPlayerSetIsShuffledRoute.self)
        mediaPlayerGetIsVisualizationEnabled = try library.resolve("cna_media_player_get_is_visualization_enabled", as: MediaPlayerGetIsVisualizationEnabledRoute.self)
        mediaPlayerSetIsVisualizationEnabled = try library.resolve("cna_media_player_set_is_visualization_enabled", as: MediaPlayerSetIsVisualizationEnabledRoute.self)
        mediaPlayerGetGameHasControl = try library.resolve("cna_media_player_get_game_has_control", as: MediaPlayerGetGameHasControlRoute.self)
        mediaPlayerSubscribeActiveSongChanged = try library.resolve("cna_media_player_subscribe_active_song_changed_ext", as: MediaPlayerSubscribeActiveSongChangedExtRoute.self)
        mediaPlayerSubscribeMediaStateChanged = try library.resolve("cna_media_player_subscribe_media_state_changed_ext", as: MediaPlayerSubscribeMediaStateChangedExtRoute.self)
        mediaPlayerUnsubscribe = try library.resolve("cna_media_player_unsubscribe_ext", as: MediaPlayerUnsubscribeExtRoute.self)
        playlistCollectionGetAt = try library.resolve("cna_playlist_collection_get_at", as: PlaylistCollectionGetAtRoute.self)
        playlistCollectionGetCount = try library.resolve("cna_playlist_collection_get_count", as: PlaylistCollectionGetCountRoute.self)
        playlistCollectionGetIsDisposed = try library.resolve("cna_playlist_collection_get_is_disposed", as: PlaylistCollectionGetIsDisposedRoute.self)
        playlistCollectionDispose = try library.resolve("cna_playlist_collection_dispose", as: PlaylistCollectionDisposeRoute.self)
        playlistCollectionDestroy = try library.resolve("cna_playlist_collection_destroy", as: PlaylistCollectionDestroyRoute.self)
        playlistGetNameSize = try library.resolve("cna_playlist_get_name_size", as: PlaylistGetNameSizeRoute.self)
        playlistCopyName = try library.resolve("cna_playlist_copy_name", as: PlaylistCopyNameRoute.self)
        playlistGetIsDisposed = try library.resolve("cna_playlist_get_is_disposed", as: PlaylistGetIsDisposedRoute.self)
        playlistDispose = try library.resolve("cna_playlist_dispose", as: PlaylistDisposeRoute.self)
        playlistDestroy = try library.resolve("cna_playlist_destroy", as: PlaylistDestroyRoute.self)
        playlistEquals = try library.resolve("cna_playlist_equals", as: PlaylistEqualsRoute.self)
        playlistGetHashCode = try library.resolve("cna_playlist_get_hash_code", as: PlaylistGetHashCodeRoute.self)
        playlistGetSongs = try library.resolve("cna_playlist_get_songs", as: PlaylistGetSongsRoute.self)
        playlistGetDuration = try library.resolve("cna_playlist_get_duration", as: PlaylistGetDurationRoute.self)
        mediaLibraryGetPlaylists = try library.resolve("cna_media_library_get_playlists", as: MediaLibraryGetPlaylistsRoute.self)
        mediaLibraryCreateFromSource = try library.resolve("cna_media_library_create_from_source", as: MediaLibraryCreateFromSourceRoute.self)
        modelBoneCreateDefault = try library.resolve("cna_model_bone_create_default", as: ModelBoneCreateDefaultRoute.self)
        modelBoneCreate = try library.resolve("cna_model_bone_create", as: ModelBoneCreateRoute.self)
        modelBoneDestroy = try library.resolve("cna_model_bone_destroy", as: ModelBoneDestroyRoute.self)
        modelBoneGetNameByteCount = try library.resolve("cna_model_bone_get_name_byte_count", as: ModelBoneGetNameByteCountRoute.self)
        modelBoneCopyName = try library.resolve("cna_model_bone_copy_name", as: ModelBoneCopyNameRoute.self)
        modelBoneGetIndex = try library.resolve("cna_model_bone_get_index", as: ModelBoneGetIndexRoute.self)
        modelBoneGetTransform = try library.resolve("cna_model_bone_get_transform", as: ModelBoneGetTransformRoute.self)
        modelBoneSetTransform = try library.resolve("cna_model_bone_set_transform", as: ModelBoneSetTransformRoute.self)
        modelBoneGetParent = try library.resolve("cna_model_bone_get_parent", as: ModelBoneGetParentRoute.self)
        modelBoneGetChildren = try library.resolve("cna_model_bone_get_children", as: ModelBoneGetChildrenRoute.self)
        modelBoneCollectionCreate = try library.resolve("cna_model_bone_collection_create", as: ModelBoneCollectionCreateRoute.self)
        modelBoneCollectionDestroy = try library.resolve("cna_model_bone_collection_destroy", as: ModelBoneCollectionDestroyRoute.self)
        modelBoneCollectionGetCount = try library.resolve("cna_model_bone_collection_get_count", as: ModelBoneCollectionGetCountRoute.self)
        modelBoneCollectionGetAt = try library.resolve("cna_model_bone_collection_get_at", as: ModelBoneCollectionGetAtRoute.self)
        modelBoneCollectionFind = try library.resolve("cna_model_bone_collection_find", as: ModelBoneCollectionFindRoute.self)
        modelBoneCollectionContains = try library.resolve("cna_model_bone_collection_contains", as: ModelBoneCollectionContainsRoute.self)
        modelMeshPartCreateDefault = try library.resolve("cna_model_mesh_part_create_default", as: ModelMeshPartCreateDefaultRoute.self)
        modelMeshPartCreate = try library.resolve("cna_model_mesh_part_create", as: ModelMeshPartCreateRoute.self)
        modelMeshPartDestroy = try library.resolve("cna_model_mesh_part_destroy", as: ModelMeshPartDestroyRoute.self)
        modelMeshPartGetNumVertices = try library.resolve("cna_model_mesh_part_get_num_vertices", as: ModelMeshPartGetNumVerticesRoute.self)
        modelMeshPartGetPrimitiveCount = try library.resolve("cna_model_mesh_part_get_primitive_count", as: ModelMeshPartGetPrimitiveCountRoute.self)
        modelMeshPartGetStartIndex = try library.resolve("cna_model_mesh_part_get_start_index", as: ModelMeshPartGetStartIndexRoute.self)
        modelMeshPartGetVertexOffset = try library.resolve("cna_model_mesh_part_get_vertex_offset", as: ModelMeshPartGetVertexOffsetRoute.self)
        modelMeshPartGetEffect = try library.resolve("cna_model_mesh_part_get_effect", as: ModelMeshPartGetEffectRoute.self)
        modelMeshPartSetEffect = try library.resolve("cna_model_mesh_part_set_effect", as: ModelMeshPartSetEffectRoute.self)
        modelMeshPartGetVertexBuffer = try library.resolve("cna_model_mesh_part_get_vertex_buffer", as: ModelMeshPartGetVertexBufferRoute.self)
        modelMeshPartGetIndexBuffer = try library.resolve("cna_model_mesh_part_get_index_buffer", as: ModelMeshPartGetIndexBufferRoute.self)
        modelMeshPartCollectionCreate = try library.resolve("cna_model_mesh_part_collection_create", as: ModelMeshPartCollectionCreateRoute.self)
        modelMeshPartCollectionDestroy = try library.resolve("cna_model_mesh_part_collection_destroy", as: ModelMeshPartCollectionDestroyRoute.self)
        modelMeshPartCollectionGetCount = try library.resolve("cna_model_mesh_part_collection_get_count", as: ModelMeshPartCollectionGetCountRoute.self)
        modelMeshPartCollectionGetAt = try library.resolve("cna_model_mesh_part_collection_get_at", as: ModelMeshPartCollectionGetAtRoute.self)
        modelMeshCreate = try library.resolve("cna_model_mesh_create", as: ModelMeshCreateRoute.self)
        modelMeshCreateNamed = try library.resolve("cna_model_mesh_create_named", as: ModelMeshCreateNamedRoute.self)
        modelMeshDestroy = try library.resolve("cna_model_mesh_destroy", as: ModelMeshDestroyRoute.self)
        modelMeshGetBoundingSphere = try library.resolve("cna_model_mesh_get_bounding_sphere", as: ModelMeshGetBoundingSphereRoute.self)
        modelMeshGetMeshParts = try library.resolve("cna_model_mesh_get_mesh_parts", as: ModelMeshGetMeshPartsRoute.self)
        modelMeshGetEffects = try library.resolve("cna_model_mesh_get_effects", as: ModelMeshGetEffectsRoute.self)
        modelMeshGetNameByteCount = try library.resolve("cna_model_mesh_get_name_byte_count", as: ModelMeshGetNameByteCountRoute.self)
        modelMeshCopyName = try library.resolve("cna_model_mesh_copy_name", as: ModelMeshCopyNameRoute.self)
        modelMeshGetParentBone = try library.resolve("cna_model_mesh_get_parent_bone", as: ModelMeshGetParentBoneRoute.self)
        modelMeshDraw = try library.resolve("cna_model_mesh_draw", as: ModelMeshDrawRoute.self)
        modelMeshCollectionCreate = try library.resolve("cna_model_mesh_collection_create", as: ModelMeshCollectionCreateRoute.self)
        modelMeshCollectionDestroy = try library.resolve("cna_model_mesh_collection_destroy", as: ModelMeshCollectionDestroyRoute.self)
        modelMeshCollectionGetCount = try library.resolve("cna_model_mesh_collection_get_count", as: ModelMeshCollectionGetCountRoute.self)
        modelMeshCollectionGetAt = try library.resolve("cna_model_mesh_collection_get_at", as: ModelMeshCollectionGetAtRoute.self)
        modelMeshCollectionFind = try library.resolve("cna_model_mesh_collection_find", as: ModelMeshCollectionFindRoute.self)
        modelMeshCollectionContains = try library.resolve("cna_model_mesh_collection_contains", as: ModelMeshCollectionContainsRoute.self)
        modelEffectCollectionDestroy = try library.resolve("cna_model_effect_collection_destroy", as: ModelEffectCollectionDestroyRoute.self)
        modelEffectCollectionGetCount = try library.resolve("cna_model_effect_collection_get_count", as: ModelEffectCollectionGetCountRoute.self)
        modelEffectCollectionGetAt = try library.resolve("cna_model_effect_collection_get_at", as: ModelEffectCollectionGetAtRoute.self)
        modelEffectCollectionContains = try library.resolve("cna_model_effect_collection_contains", as: ModelEffectCollectionContainsRoute.self)
        vertexBufferGetInfo = try library.resolve("cna_vertex_buffer_get_info", as: VertexBufferGetInfoRoute.self)
        vertexBufferCopyDeclarationElements = try library.resolve("cna_vertex_buffer_copy_declaration_elements", as: VertexBufferCopyDeclarationElementsRoute.self)
        indexBufferGetInfo = try library.resolve("cna_index_buffer_get_info", as: IndexBufferGetInfoRoute.self)
        modelCreateDefault = try library.resolve("cna_model_create_default", as: ModelCreateDefaultRoute.self)
        modelDestroy = try library.resolve("cna_model_destroy", as: ModelDestroyRoute.self)
        modelGetBones = try library.resolve("cna_model_get_bones", as: ModelGetBonesRoute.self)
        modelGetMeshes = try library.resolve("cna_model_get_meshes", as: ModelGetMeshesRoute.self)
        modelGetRoot = try library.resolve("cna_model_get_root", as: ModelGetRootRoute.self)
        modelGetBoneTransformCount = try library.resolve("cna_model_get_bone_transform_count", as: ModelGetBoneTransformCountRoute.self)
        modelCopyAbsoluteBoneTransforms = try library.resolve("cna_model_copy_absolute_bone_transforms", as: ModelCopyAbsoluteBoneTransformsRoute.self)
        modelSetBoneTransforms = try library.resolve("cna_model_set_bone_transforms", as: ModelSetBoneTransformsRoute.self)
        modelCopyBoneTransforms = try library.resolve("cna_model_copy_bone_transforms", as: ModelCopyBoneTransformsRoute.self)
        modelDraw = try library.resolve("cna_model_draw", as: ModelDrawRoute.self)
        storageDeviceShowSelector = try library.resolve("cna_storage_device_show_selector", as: StorageDeviceShowSelectorRoute.self)
        storageDeviceShowSelectorForPlayer = try library.resolve("cna_storage_device_show_selector_for_player", as: StorageDeviceShowSelectorForPlayerRoute.self)
        storageDeviceShowSelectorWithSpace = try library.resolve("cna_storage_device_show_selector_with_space", as: StorageDeviceShowSelectorWithSpaceRoute.self)
        storageDeviceShowSelectorForPlayerWithSpace = try library.resolve("cna_storage_device_show_selector_for_player_with_space", as: StorageDeviceShowSelectorForPlayerWithSpaceRoute.self)
        storageDeviceGetFreeSpace = try library.resolve("cna_storage_device_get_free_space", as: StorageDeviceGetFreeSpaceRoute.self)
        storageDeviceGetIsConnected = try library.resolve("cna_storage_device_get_is_connected", as: StorageDeviceGetIsConnectedRoute.self)
        storageDeviceGetTotalSpace = try library.resolve("cna_storage_device_get_total_space", as: StorageDeviceGetTotalSpaceRoute.self)
        storageDeviceDeleteContainer = try library.resolve("cna_storage_device_delete_container", as: StorageDeviceDeleteContainerRoute.self)
        storageDeviceSubscribeDeviceChanged = try library.resolve("cna_storage_device_subscribe_device_changed", as: StorageDeviceSubscribeDeviceChangedRoute.self)
        storageDeviceUnsubscribeDeviceChanged = try library.resolve("cna_storage_device_unsubscribe_device_changed", as: StorageDeviceUnsubscribeDeviceChangedRoute.self)
        storageDeviceDestroy = try library.resolve("cna_storage_device_destroy", as: StorageDeviceDestroyRoute.self)
        storageContainerOpen = try library.resolve("cna_storage_container_open", as: StorageContainerOpenRoute.self)
        storageContainerGetDisplayNameSize = try library.resolve("cna_storage_container_get_display_name_size", as: StorageContainerGetDisplayNameSizeRoute.self)
        storageContainerCopyDisplayName = try library.resolve("cna_storage_container_copy_display_name", as: StorageContainerCopyDisplayNameRoute.self)
        storageContainerGetIsDisposed = try library.resolve("cna_storage_container_get_is_disposed", as: StorageContainerGetIsDisposedRoute.self)
        storageContainerGetStorageDevice = try library.resolve("cna_storage_container_get_storage_device", as: StorageContainerGetStorageDeviceRoute.self)
        storageContainerDispose = try library.resolve("cna_storage_container_dispose", as: StorageContainerDisposeRoute.self)
        storageContainerSubscribeDisposing = try library.resolve("cna_storage_container_subscribe_disposing", as: StorageContainerSubscribeDisposingRoute.self)
        storageContainerUnsubscribeDisposing = try library.resolve("cna_storage_container_unsubscribe_disposing", as: StorageContainerUnsubscribeDisposingRoute.self)
        storageContainerCreateDirectory = try library.resolve("cna_storage_container_create_directory", as: StorageContainerCreateDirectoryRoute.self)
        storageContainerDirectoryExists = try library.resolve("cna_storage_container_directory_exists", as: StorageContainerDirectoryExistsRoute.self)
        storageContainerDeleteDirectory = try library.resolve("cna_storage_container_delete_directory", as: StorageContainerDeleteDirectoryRoute.self)
        storageContainerFileExists = try library.resolve("cna_storage_container_file_exists", as: StorageContainerFileExistsRoute.self)
        storageContainerDeleteFile = try library.resolve("cna_storage_container_delete_file", as: StorageContainerDeleteFileRoute.self)
        storageContainerGetDirectoryNameCount = try library.resolve("cna_storage_container_get_directory_name_count", as: StorageContainerGetDirectoryNameCountRoute.self)
        storageContainerCopyDirectoryName = try library.resolve("cna_storage_container_copy_directory_name", as: StorageContainerCopyDirectoryNameRoute.self)
        storageContainerGetFileNameCount = try library.resolve("cna_storage_container_get_file_name_count", as: StorageContainerGetFileNameCountRoute.self)
        storageContainerCopyFileName = try library.resolve("cna_storage_container_copy_file_name", as: StorageContainerCopyFileNameRoute.self)
        storageContainerCreateFile = try library.resolve("cna_storage_container_create_file", as: StorageContainerCreateFileRoute.self)
        storageContainerOpenFile = try library.resolve("cna_storage_container_open_file", as: StorageContainerOpenFileRoute.self)
        storageContainerOpenFileAccess = try library.resolve("cna_storage_container_open_file_access", as: StorageContainerOpenFileAccessRoute.self)
        storageContainerOpenFileShare = try library.resolve("cna_storage_container_open_file_share", as: StorageContainerOpenFileShareRoute.self)
        storageStreamRead = try library.resolve("cna_storage_stream_read", as: StorageStreamReadRoute.self)
        storageStreamWrite = try library.resolve("cna_storage_stream_write", as: StorageStreamWriteRoute.self)
        storageStreamSeek = try library.resolve("cna_storage_stream_seek", as: StorageStreamSeekRoute.self)
        storageStreamGetPosition = try library.resolve("cna_storage_stream_get_position", as: StorageStreamGetPositionRoute.self)
        storageStreamGetLength = try library.resolve("cna_storage_stream_get_length", as: StorageStreamGetLengthRoute.self)
        storageStreamSetLength = try library.resolve("cna_storage_stream_set_length", as: StorageStreamSetLengthRoute.self)
        storageStreamGetCanRead = try library.resolve("cna_storage_stream_get_can_read", as: StorageStreamGetCanReadRoute.self)
        storageStreamGetCanWrite = try library.resolve("cna_storage_stream_get_can_write", as: StorageStreamGetCanWriteRoute.self)
        storageStreamGetCanSeek = try library.resolve("cna_storage_stream_get_can_seek", as: StorageStreamGetCanSeekRoute.self)
        storageStreamFlush = try library.resolve("cna_storage_stream_flush", as: StorageStreamFlushRoute.self)
        storageStreamClose = try library.resolve("cna_storage_stream_close", as: StorageStreamCloseRoute.self)
        storageContainerDestroy = try library.resolve("cna_storage_container_destroy", as: StorageContainerDestroyRoute.self)
        mediaSourceGetAvailableCount = try library.resolve("cna_media_source_get_available_count", as: MediaSourceGetAvailableCountRoute.self)
        mediaSourceGetNameSizeAt = try library.resolve("cna_media_source_get_name_size_at", as: MediaSourceGetNameSizeAtRoute.self)
        mediaSourceCopyNameAt = try library.resolve("cna_media_source_copy_name_at", as: MediaSourceCopyNameAtRoute.self)
        mediaSourceGetTypeAt = try library.resolve("cna_media_source_get_type_at", as: MediaSourceGetTypeAtRoute.self)
        pictureCollectionGetAt = try library.resolve("cna_picture_collection_get_at", as: PictureCollectionGetAtRoute.self)
        pictureCollectionGetCount = try library.resolve("cna_picture_collection_get_count", as: PictureCollectionGetCountRoute.self)
        pictureCollectionGetIsDisposed = try library.resolve("cna_picture_collection_get_is_disposed", as: PictureCollectionGetIsDisposedRoute.self)
        pictureCollectionDispose = try library.resolve("cna_picture_collection_dispose", as: PictureCollectionDisposeRoute.self)
        pictureCollectionDestroy = try library.resolve("cna_picture_collection_destroy", as: PictureCollectionDestroyRoute.self)
        pictureAlbumCollectionGetAt = try library.resolve("cna_picture_album_collection_get_at", as: PictureAlbumCollectionGetAtRoute.self)
        pictureAlbumCollectionGetCount = try library.resolve("cna_picture_album_collection_get_count", as: PictureAlbumCollectionGetCountRoute.self)
        pictureAlbumCollectionGetIsDisposed = try library.resolve("cna_picture_album_collection_get_is_disposed", as: PictureAlbumCollectionGetIsDisposedRoute.self)
        pictureAlbumCollectionDispose = try library.resolve("cna_picture_album_collection_dispose", as: PictureAlbumCollectionDisposeRoute.self)
        pictureAlbumCollectionDestroy = try library.resolve("cna_picture_album_collection_destroy", as: PictureAlbumCollectionDestroyRoute.self)
        pictureGetNameSize = try library.resolve("cna_picture_get_name_size", as: PictureGetNameSizeRoute.self)
        pictureCopyName = try library.resolve("cna_picture_copy_name", as: PictureCopyNameRoute.self)
        pictureGetIsDisposed = try library.resolve("cna_picture_get_is_disposed", as: PictureGetIsDisposedRoute.self)
        pictureDispose = try library.resolve("cna_picture_dispose", as: PictureDisposeRoute.self)
        pictureDestroy = try library.resolve("cna_picture_destroy", as: PictureDestroyRoute.self)
        pictureEquals = try library.resolve("cna_picture_equals", as: PictureEqualsRoute.self)
        pictureGetHashCode = try library.resolve("cna_picture_get_hash_code", as: PictureGetHashCodeRoute.self)
        pictureAlbumGetNameSize = try library.resolve("cna_picture_album_get_name_size", as: PictureAlbumGetNameSizeRoute.self)
        pictureAlbumCopyName = try library.resolve("cna_picture_album_copy_name", as: PictureAlbumCopyNameRoute.self)
        pictureAlbumGetIsDisposed = try library.resolve("cna_picture_album_get_is_disposed", as: PictureAlbumGetIsDisposedRoute.self)
        pictureAlbumDispose = try library.resolve("cna_picture_album_dispose", as: PictureAlbumDisposeRoute.self)
        pictureAlbumDestroy = try library.resolve("cna_picture_album_destroy", as: PictureAlbumDestroyRoute.self)
        pictureAlbumEquals = try library.resolve("cna_picture_album_equals", as: PictureAlbumEqualsRoute.self)
        pictureAlbumGetHashCode = try library.resolve("cna_picture_album_get_hash_code", as: PictureAlbumGetHashCodeRoute.self)
        pictureGetAlbum = try library.resolve("cna_picture_get_album", as: PictureGetAlbumRoute.self)
        pictureGetWidth = try library.resolve("cna_picture_get_width", as: PictureGetWidthRoute.self)
        pictureGetHeight = try library.resolve("cna_picture_get_height", as: PictureGetHeightRoute.self)
        pictureGetDateUnixTicks = try library.resolve("cna_picture_get_date_unix_ticks", as: PictureGetDateUnixTicksRoute.self)
        pictureGetImageSize = try library.resolve("cna_picture_get_image_size", as: PictureGetImageSizeRoute.self)
        pictureCopyImage = try library.resolve("cna_picture_copy_image", as: PictureCopyImageRoute.self)
        pictureGetThumbnailSize = try library.resolve("cna_picture_get_thumbnail_size", as: PictureGetThumbnailSizeRoute.self)
        pictureCopyThumbnail = try library.resolve("cna_picture_copy_thumbnail", as: PictureCopyThumbnailRoute.self)
        pictureAlbumGetAlbums = try library.resolve("cna_picture_album_get_albums", as: PictureAlbumGetAlbumsRoute.self)
        pictureAlbumGetPictures = try library.resolve("cna_picture_album_get_pictures", as: PictureAlbumGetPicturesRoute.self)
        pictureAlbumGetParent = try library.resolve("cna_picture_album_get_parent", as: PictureAlbumGetParentRoute.self)
        mediaLibraryGetPictures = try library.resolve("cna_media_library_get_pictures", as: MediaLibraryGetPicturesRoute.self)
        mediaLibraryGetSavedPictures = try library.resolve("cna_media_library_get_saved_pictures", as: MediaLibraryGetSavedPicturesRoute.self)
        mediaLibraryGetRootPictureAlbum = try library.resolve("cna_media_library_get_root_picture_album", as: MediaLibraryGetRootPictureAlbumRoute.self)
        mediaLibraryGetPictureFromToken = try library.resolve("cna_media_library_get_picture_from_token", as: MediaLibraryGetPictureFromTokenRoute.self)
        mediaLibraryCreate = try library.resolve("cna_media_library_create", as: MediaLibraryCreateRoute.self)
        mediaLibraryDispose = try library.resolve("cna_media_library_dispose", as: MediaLibraryDisposeRoute.self)
        mediaLibraryDestroy = try library.resolve("cna_media_library_destroy", as: MediaLibraryDestroyRoute.self)
        mediaLibraryGetIsDisposed = try library.resolve("cna_media_library_get_is_disposed", as: MediaLibraryGetIsDisposedRoute.self)
        mediaLibraryGetSongs = try library.resolve("cna_media_library_get_songs", as: MediaLibraryGetSongsRoute.self)
        mediaLibraryGetArtists = try library.resolve("cna_media_library_get_artists", as: MediaLibraryGetArtistsRoute.self)
        mediaLibraryGetAlbums = try library.resolve("cna_media_library_get_albums", as: MediaLibraryGetAlbumsRoute.self)
        mediaLibraryGetGenres = try library.resolve("cna_media_library_get_genres", as: MediaLibraryGetGenresRoute.self)
        songGetArtist = try library.resolve("cna_song_get_artist", as: SongGetArtistRoute.self)
        songGetAlbum = try library.resolve("cna_song_get_album", as: SongGetAlbumRoute.self)
        songGetGenre = try library.resolve("cna_song_get_genre", as: SongGetGenreRoute.self)
        songCollectionGetAt = try library.resolve("cna_song_collection_get_at", as: SongCollectionGetAtRoute.self)
        songCollectionGetCount = try library.resolve("cna_song_collection_get_count", as: SongCollectionGetCountRoute.self)
        songCollectionGetIsDisposed = try library.resolve("cna_song_collection_get_is_disposed", as: SongCollectionGetIsDisposedRoute.self)
        songCollectionDispose = try library.resolve("cna_song_collection_dispose", as: SongCollectionDisposeRoute.self)
        songCollectionDestroy = try library.resolve("cna_song_collection_destroy", as: SongCollectionDestroyRoute.self)
        artistCollectionGetAt = try library.resolve("cna_artist_collection_get_at", as: ArtistCollectionGetAtRoute.self)
        artistCollectionGetCount = try library.resolve("cna_artist_collection_get_count", as: ArtistCollectionGetCountRoute.self)
        artistCollectionGetIsDisposed = try library.resolve("cna_artist_collection_get_is_disposed", as: ArtistCollectionGetIsDisposedRoute.self)
        artistCollectionDispose = try library.resolve("cna_artist_collection_dispose", as: ArtistCollectionDisposeRoute.self)
        artistCollectionDestroy = try library.resolve("cna_artist_collection_destroy", as: ArtistCollectionDestroyRoute.self)
        albumCollectionGetAt = try library.resolve("cna_album_collection_get_at", as: AlbumCollectionGetAtRoute.self)
        albumCollectionGetCount = try library.resolve("cna_album_collection_get_count", as: AlbumCollectionGetCountRoute.self)
        albumCollectionGetIsDisposed = try library.resolve("cna_album_collection_get_is_disposed", as: AlbumCollectionGetIsDisposedRoute.self)
        albumCollectionDispose = try library.resolve("cna_album_collection_dispose", as: AlbumCollectionDisposeRoute.self)
        albumCollectionDestroy = try library.resolve("cna_album_collection_destroy", as: AlbumCollectionDestroyRoute.self)
        genreCollectionGetAt = try library.resolve("cna_genre_collection_get_at", as: GenreCollectionGetAtRoute.self)
        genreCollectionGetCount = try library.resolve("cna_genre_collection_get_count", as: GenreCollectionGetCountRoute.self)
        genreCollectionGetIsDisposed = try library.resolve("cna_genre_collection_get_is_disposed", as: GenreCollectionGetIsDisposedRoute.self)
        genreCollectionDispose = try library.resolve("cna_genre_collection_dispose", as: GenreCollectionDisposeRoute.self)
        genreCollectionDestroy = try library.resolve("cna_genre_collection_destroy", as: GenreCollectionDestroyRoute.self)
        artistGetNameSize = try library.resolve("cna_artist_get_name_size", as: ArtistGetNameSizeRoute.self)
        artistCopyName = try library.resolve("cna_artist_copy_name", as: ArtistCopyNameRoute.self)
        artistGetIsDisposed = try library.resolve("cna_artist_get_is_disposed", as: ArtistGetIsDisposedRoute.self)
        artistDispose = try library.resolve("cna_artist_dispose", as: ArtistDisposeRoute.self)
        artistDestroy = try library.resolve("cna_artist_destroy", as: ArtistDestroyRoute.self)
        artistEquals = try library.resolve("cna_artist_equals", as: ArtistEqualsRoute.self)
        artistGetHashCode = try library.resolve("cna_artist_get_hash_code", as: ArtistGetHashCodeRoute.self)
        artistGetSongs = try library.resolve("cna_artist_get_songs", as: ArtistGetSongsRoute.self)
        albumGetNameSize = try library.resolve("cna_album_get_name_size", as: AlbumGetNameSizeRoute.self)
        albumCopyName = try library.resolve("cna_album_copy_name", as: AlbumCopyNameRoute.self)
        albumGetIsDisposed = try library.resolve("cna_album_get_is_disposed", as: AlbumGetIsDisposedRoute.self)
        albumDispose = try library.resolve("cna_album_dispose", as: AlbumDisposeRoute.self)
        albumDestroy = try library.resolve("cna_album_destroy", as: AlbumDestroyRoute.self)
        albumEquals = try library.resolve("cna_album_equals", as: AlbumEqualsRoute.self)
        albumGetHashCode = try library.resolve("cna_album_get_hash_code", as: AlbumGetHashCodeRoute.self)
        albumGetSongs = try library.resolve("cna_album_get_songs", as: AlbumGetSongsRoute.self)
        genreGetNameSize = try library.resolve("cna_genre_get_name_size", as: GenreGetNameSizeRoute.self)
        genreCopyName = try library.resolve("cna_genre_copy_name", as: GenreCopyNameRoute.self)
        genreGetIsDisposed = try library.resolve("cna_genre_get_is_disposed", as: GenreGetIsDisposedRoute.self)
        genreDispose = try library.resolve("cna_genre_dispose", as: GenreDisposeRoute.self)
        genreDestroy = try library.resolve("cna_genre_destroy", as: GenreDestroyRoute.self)
        genreEquals = try library.resolve("cna_genre_equals", as: GenreEqualsRoute.self)
        genreGetHashCode = try library.resolve("cna_genre_get_hash_code", as: GenreGetHashCodeRoute.self)
        genreGetSongs = try library.resolve("cna_genre_get_songs", as: GenreGetSongsRoute.self)
        artistGetAlbums = try library.resolve("cna_artist_get_albums", as: ArtistGetAlbumsRoute.self)
        genreGetAlbums = try library.resolve("cna_genre_get_albums", as: GenreGetAlbumsRoute.self)
        albumGetArtist = try library.resolve("cna_album_get_artist", as: AlbumGetArtistRoute.self)
        albumGetGenre = try library.resolve("cna_album_get_genre", as: AlbumGetGenreRoute.self)
        albumGetDuration = try library.resolve("cna_album_get_duration", as: AlbumGetDurationRoute.self)
        albumGetHasArt = try library.resolve("cna_album_get_has_art", as: AlbumGetHasArtRoute.self)
        albumGetArtSize = try library.resolve("cna_album_get_art_size", as: AlbumGetArtSizeRoute.self)
        albumCopyArt = try library.resolve("cna_album_copy_art", as: AlbumCopyArtRoute.self)
        albumGetThumbnailSize = try library.resolve("cna_album_get_thumbnail_size", as: AlbumGetThumbnailSizeRoute.self)
        albumCopyThumbnail = try library.resolve("cna_album_copy_thumbnail", as: AlbumCopyThumbnailRoute.self)
        songCreateFromUri = try library.resolve("cna_song_create_from_uri", as: SongCreateFromUriRoute.self)
        songDestroy = try library.resolve("cna_song_destroy", as: SongDestroyRoute.self)
        songDispose = try library.resolve("cna_song_dispose", as: SongDisposeRoute.self)
        songGetIsDisposed = try library.resolve("cna_song_get_is_disposed", as: SongGetIsDisposedRoute.self)
        songGetNameSize = try library.resolve("cna_song_get_name_size", as: SongGetNameSizeRoute.self)
        songCopyName = try library.resolve("cna_song_copy_name", as: SongCopyNameRoute.self)
        songGetDuration = try library.resolve("cna_song_get_duration", as: SongGetDurationRoute.self)
        songGetIsRated = try library.resolve("cna_song_get_is_rated", as: SongGetIsRatedRoute.self)
        songGetRating = try library.resolve("cna_song_get_rating", as: SongGetRatingRoute.self)
        songGetPlayCount = try library.resolve("cna_song_get_play_count", as: SongGetPlayCountRoute.self)
        songGetTrackNumber = try library.resolve("cna_song_get_track_number", as: SongGetTrackNumberRoute.self)
        songGetIsProtected = try library.resolve("cna_song_get_is_protected", as: SongGetIsProtectedRoute.self)
        songGetHashCode = try library.resolve("cna_song_get_hash_code", as: SongGetHashCodeRoute.self)
        songEquals = try library.resolve("cna_song_equals", as: SongEqualsRoute.self)
        dynamicSoundEffectInstanceCreate = try library.resolve("cna_dynamic_sound_effect_instance_create", as: DynamicSoundEffectInstanceCreateRoute.self)
        dynamicSoundEffectInstanceSubmitBuffer = try library.resolve("cna_dynamic_sound_effect_instance_submit_buffer", as: DynamicSoundEffectInstanceSubmitBufferRoute.self)
        dynamicSoundEffectInstanceGetPendingBufferCount = try library.resolve("cna_dynamic_sound_effect_instance_get_pending_buffer_count", as: DynamicSoundEffectInstanceGetPendingBufferCountRoute.self)
        dynamicSoundEffectInstanceGetSampleDurationTicks = try library.resolve("cna_dynamic_sound_effect_instance_get_sample_duration_ticks", as: DynamicSoundEffectInstanceGetSampleDurationTicksRoute.self)
        dynamicSoundEffectInstanceGetSampleSizeInBytes = try library.resolve("cna_dynamic_sound_effect_instance_get_sample_size_in_bytes", as: DynamicSoundEffectInstanceGetSampleSizeInBytesRoute.self)
        dynamicSoundEffectInstanceSubscribeBufferNeeded = try library.resolve("cna_dynamic_sound_effect_instance_subscribe_buffer_needed", as: DynamicSoundEffectInstanceSubscribeBufferNeededRoute.self)
        audioUnsubscribe = try library.resolve("cna_audio_unsubscribe_ext", as: AudioUnsubscribeExtRoute.self)
        audioEngineCreate = try library.resolve("cna_audio_engine_create", as: AudioEngineCreateRoute.self)
        audioEngineCreateWithRenderer = try library.resolve("cna_audio_engine_create_with_renderer", as: AudioEngineCreateWithRendererRoute.self)
        audioEngineDestroy = try library.resolve("cna_audio_engine_destroy", as: AudioEngineDestroyRoute.self)
        audioEngineGetRendererCount = try library.resolve("cna_audio_engine_get_renderer_count", as: AudioEngineGetRendererCountRoute.self)
        audioEngineGetRendererFriendlyNameSize = try library.resolve("cna_audio_engine_get_renderer_friendly_name_size", as: AudioEngineGetRendererFriendlyNameSizeRoute.self)
        audioEngineCopyRendererFriendlyName = try library.resolve("cna_audio_engine_copy_renderer_friendly_name", as: AudioEngineCopyRendererFriendlyNameRoute.self)
        audioEngineGetRendererIdSize = try library.resolve("cna_audio_engine_get_renderer_id_size", as: AudioEngineGetRendererIdSizeRoute.self)
        audioEngineCopyRendererId = try library.resolve("cna_audio_engine_copy_renderer_id", as: AudioEngineCopyRendererIdRoute.self)
        audioEngineGetGlobalVariable = try library.resolve("cna_audio_engine_get_global_variable", as: AudioEngineGetGlobalVariableRoute.self)
        audioEngineSetGlobalVariable = try library.resolve("cna_audio_engine_set_global_variable", as: AudioEngineSetGlobalVariableRoute.self)
        audioEngineUpdate = try library.resolve("cna_audio_engine_update", as: AudioEngineUpdateRoute.self)
        audioEngineGetCategory = try library.resolve("cna_audio_engine_get_category", as: AudioEngineGetCategoryRoute.self)
        audioCategoryDestroy = try library.resolve("cna_audio_category_destroy", as: AudioCategoryDestroyRoute.self)
        audioCategoryGetNameSize = try library.resolve("cna_audio_category_get_name_size", as: AudioCategoryGetNameSizeRoute.self)
        audioCategoryCopyName = try library.resolve("cna_audio_category_copy_name", as: AudioCategoryCopyNameRoute.self)
        audioCategoryPause = try library.resolve("cna_audio_category_pause", as: AudioCategoryPauseRoute.self)
        audioCategoryResume = try library.resolve("cna_audio_category_resume", as: AudioCategoryResumeRoute.self)
        audioCategorySetVolume = try library.resolve("cna_audio_category_set_volume", as: AudioCategorySetVolumeRoute.self)
        audioCategoryStop = try library.resolve("cna_audio_category_stop", as: AudioCategoryStopRoute.self)
        audioCategoryEquals = try library.resolve("cna_audio_category_equals", as: AudioCategoryEqualsRoute.self)
        audioCategoryGetHashCode = try library.resolve("cna_audio_category_get_hash_code", as: AudioCategoryGetHashCodeRoute.self)
        waveBankCreate = try library.resolve("cna_wave_bank_create", as: WaveBankCreateRoute.self)
        waveBankCreateStreaming = try library.resolve("cna_wave_bank_create_streaming", as: WaveBankCreateStreamingRoute.self)
        waveBankDestroy = try library.resolve("cna_wave_bank_destroy", as: WaveBankDestroyRoute.self)
        waveBankGetIsInUse = try library.resolve("cna_wave_bank_get_is_in_use", as: WaveBankGetIsInUseRoute.self)
        waveBankGetIsPrepared = try library.resolve("cna_wave_bank_get_is_prepared", as: WaveBankGetIsPreparedRoute.self)
        soundBankCreate = try library.resolve("cna_sound_bank_create", as: SoundBankCreateRoute.self)
        soundBankDestroy = try library.resolve("cna_sound_bank_destroy", as: SoundBankDestroyRoute.self)
        soundBankGetIsInUse = try library.resolve("cna_sound_bank_get_is_in_use", as: SoundBankGetIsInUseRoute.self)
        soundBankGetCue = try library.resolve("cna_sound_bank_get_cue", as: SoundBankGetCueRoute.self)
        soundBankPlayCue = try library.resolve("cna_sound_bank_play_cue", as: SoundBankPlayCueRoute.self)
        soundBankPlayCue3D = try library.resolve("cna_sound_bank_play_cue_3d", as: SoundBankPlayCue3dRoute.self)
        cueDestroy = try library.resolve("cna_cue_destroy", as: CueDestroyRoute.self)
        cueGetInfo = try library.resolve("cna_cue_get_info", as: CueGetInfoRoute.self)
        cueGetNameSize = try library.resolve("cna_cue_get_name_size", as: CueGetNameSizeRoute.self)
        cueCopyName = try library.resolve("cna_cue_copy_name", as: CueCopyNameRoute.self)
        cueApply3D = try library.resolve("cna_cue_apply_3d", as: CueApply3dRoute.self)
        cueGetVariable = try library.resolve("cna_cue_get_variable", as: CueGetVariableRoute.self)
        cueSetVariable = try library.resolve("cna_cue_set_variable", as: CueSetVariableRoute.self)
        cuePlay = try library.resolve("cna_cue_play", as: CuePlayRoute.self)
        cuePause = try library.resolve("cna_cue_pause", as: CuePauseRoute.self)
        cueResume = try library.resolve("cna_cue_resume", as: CueResumeRoute.self)
        cueStop = try library.resolve("cna_cue_stop", as: CueStopRoute.self)
        microphoneGetCount = try library.resolve("cna_microphone_get_count", as: MicrophoneGetCountRoute.self)
        microphoneGetDefaultIndex = try library.resolve("cna_microphone_get_default_index_ext", as: MicrophoneGetDefaultIndexExtRoute.self)
        microphoneGetNameSize = try library.resolve("cna_microphone_get_name_size_at", as: MicrophoneGetNameSizeAtRoute.self)
        microphoneCopyName = try library.resolve("cna_microphone_copy_name_at", as: MicrophoneCopyNameAtRoute.self)
        microphoneGetBufferDuration = try library.resolve("cna_microphone_get_buffer_duration_ticks_at", as: MicrophoneGetBufferDurationTicksAtRoute.self)
        microphoneSetBufferDuration = try library.resolve("cna_microphone_set_buffer_duration_ticks_at", as: MicrophoneSetBufferDurationTicksAtRoute.self)
        microphoneGetIsHeadset = try library.resolve("cna_microphone_get_is_headset_at", as: MicrophoneGetIsHeadsetAtRoute.self)
        microphoneGetSampleRate = try library.resolve("cna_microphone_get_sample_rate_at", as: MicrophoneGetSampleRateAtRoute.self)
        microphoneGetState = try library.resolve("cna_microphone_get_state_at", as: MicrophoneGetStateAtRoute.self)
        microphoneStart = try library.resolve("cna_microphone_start_at", as: MicrophoneStartAtRoute.self)
        microphoneStop = try library.resolve("cna_microphone_stop_at", as: MicrophoneStopAtRoute.self)
        microphoneGetData = try library.resolve("cna_microphone_get_data_at", as: MicrophoneGetDataAtRoute.self)
        microphoneGetSampleDuration = try library.resolve("cna_microphone_get_sample_duration_ticks_at", as: MicrophoneGetSampleDurationTicksAtRoute.self)
        microphoneGetSampleSize = try library.resolve("cna_microphone_get_sample_size_in_bytes_at", as: MicrophoneGetSampleSizeInBytesAtRoute.self)
        microphoneSubscribeBufferReady = try library.resolve("cna_microphone_subscribe_buffer_ready_at", as: MicrophoneSubscribeBufferReadyAtRoute.self)
        soundEffectInstanceApply3D = try library.resolve("cna_sound_effect_instance_apply_3d", as: SoundEffectInstanceApply3dRoute.self)
        soundEffectInstanceApply3DMulti = try library.resolve("cna_sound_effect_instance_apply_3d_multi_ext", as: SoundEffectInstanceApply3dMultiExtRoute.self)
        soundEffectCreatePcm16 = try library.resolve("cna_sound_effect_create_pcm16", as: SoundEffectCreatePcm16Route.self)
        soundEffectCreatePcm16Range = try library.resolve("cna_sound_effect_create_pcm16_range_ext", as: SoundEffectCreatePcm16RangeExtRoute.self)
        soundEffectCreateFromEncoded = try library.resolve("cna_sound_effect_create_from_encoded_ext", as: SoundEffectCreateFromEncodedExtRoute.self)
        soundEffectDestroy = try library.resolve("cna_sound_effect_destroy", as: SoundEffectDestroyRoute.self)
        soundEffectGetIsDisposed = try library.resolve("cna_sound_effect_get_is_disposed", as: SoundEffectGetIsDisposedRoute.self)
        soundEffectGetNameSize = try library.resolve("cna_sound_effect_get_name_size", as: SoundEffectGetNameSizeRoute.self)
        soundEffectCopyName = try library.resolve("cna_sound_effect_copy_name", as: SoundEffectCopyNameRoute.self)
        soundEffectSetName = try library.resolve("cna_sound_effect_set_name", as: SoundEffectSetNameRoute.self)
        soundEffectGetDurationTicks = try library.resolve("cna_sound_effect_get_duration_ticks", as: SoundEffectGetDurationTicksRoute.self)
        soundEffectPlay = try library.resolve("cna_sound_effect_play", as: SoundEffectPlayRoute.self)
        soundEffectPlayWithSettings = try library.resolve("cna_sound_effect_play_with_settings", as: SoundEffectPlayWithSettingsRoute.self)
        soundEffectCreateInstance = try library.resolve("cna_sound_effect_create_instance", as: SoundEffectCreateInstanceRoute.self)
        soundEffectGetMasterVolume = try library.resolve("cna_sound_effect_get_master_volume", as: SoundEffectGetMasterVolumeRoute.self)
        soundEffectSetMasterVolume = try library.resolve("cna_sound_effect_set_master_volume", as: SoundEffectSetMasterVolumeRoute.self)
        soundEffectGetDistanceScale = try library.resolve("cna_sound_effect_get_distance_scale", as: SoundEffectGetDistanceScaleRoute.self)
        soundEffectSetDistanceScale = try library.resolve("cna_sound_effect_set_distance_scale", as: SoundEffectSetDistanceScaleRoute.self)
        soundEffectGetDopplerScale = try library.resolve("cna_sound_effect_get_doppler_scale", as: SoundEffectGetDopplerScaleRoute.self)
        soundEffectSetDopplerScale = try library.resolve("cna_sound_effect_set_doppler_scale", as: SoundEffectSetDopplerScaleRoute.self)
        soundEffectGetSpeedOfSound = try library.resolve("cna_sound_effect_get_speed_of_sound", as: SoundEffectGetSpeedOfSoundRoute.self)
        soundEffectSetSpeedOfSound = try library.resolve("cna_sound_effect_set_speed_of_sound", as: SoundEffectSetSpeedOfSoundRoute.self)
        soundEffectGetSampleSizeInBytes = try library.resolve("cna_sound_effect_get_sample_size_in_bytes", as: SoundEffectGetSampleSizeInBytesRoute.self)
        soundEffectGetSampleDurationTicks = try library.resolve("cna_sound_effect_get_sample_duration_ticks", as: SoundEffectGetSampleDurationTicksRoute.self)
        soundEffectInstanceGetInfo = try library.resolve("cna_sound_effect_instance_get_info", as: SoundEffectInstanceGetInfoRoute.self)
        soundEffectInstanceSetVolume = try library.resolve("cna_sound_effect_instance_set_volume", as: SoundEffectInstanceSetVolumeRoute.self)
        soundEffectInstanceSetPitch = try library.resolve("cna_sound_effect_instance_set_pitch", as: SoundEffectInstanceSetPitchRoute.self)
        soundEffectInstanceSetPan = try library.resolve("cna_sound_effect_instance_set_pan", as: SoundEffectInstanceSetPanRoute.self)
        soundEffectInstanceSetIsLooped = try library.resolve("cna_sound_effect_instance_set_is_looped", as: SoundEffectInstanceSetIsLoopedRoute.self)
        soundEffectInstancePlay = try library.resolve("cna_sound_effect_instance_play", as: SoundEffectInstancePlayRoute.self)
        soundEffectInstancePause = try library.resolve("cna_sound_effect_instance_pause", as: SoundEffectInstancePauseRoute.self)
        soundEffectInstanceResume = try library.resolve("cna_sound_effect_instance_resume", as: SoundEffectInstanceResumeRoute.self)
        soundEffectInstanceStop = try library.resolve("cna_sound_effect_instance_stop", as: SoundEffectInstanceStopRoute.self)
        soundEffectInstanceGetIsDisposed = try library.resolve("cna_sound_effect_instance_get_is_disposed", as: SoundEffectInstanceGetIsDisposedRoute.self)
        soundEffectInstanceDestroy = try library.resolve("cna_sound_effect_instance_destroy", as: SoundEffectInstanceDestroyRoute.self)
        occlusionQueryDestroy = try library.resolve("cna_occlusion_query_destroy", as: OcclusionQueryDestroyRoute.self)
        occlusionQueryBegin = try library.resolve("cna_occlusion_query_begin", as: OcclusionQueryBeginRoute.self)
        occlusionQueryEnd = try library.resolve("cna_occlusion_query_end", as: OcclusionQueryEndRoute.self)
        occlusionQueryGetIsComplete = try library.resolve("cna_occlusion_query_get_is_complete", as: OcclusionQueryGetIsCompleteRoute.self)
        occlusionQueryGetPixelCount = try library.resolve("cna_occlusion_query_get_pixel_count", as: OcclusionQueryGetPixelCountRoute.self)
        renderTargetGetInfo = try library.resolve("cna_render_target_get_info", as: RenderTargetGetInfoRoute.self)
        renderTargetDestroy = try library.resolve("cna_render_target_destroy", as: RenderTargetDestroyRoute.self)
        graphicsDeviceSetRenderTarget2D = try library.resolve("cna_graphics_device_set_render_target2d", as: GraphicsDeviceSetRenderTarget2dRoute.self)
        renderTargetCubeCreate = try library.resolve("cna_render_target_cube_create", as: RenderTargetCubeCreateRoute.self)
        graphicsDeviceSetRenderTargetCube = try library.resolve("cna_graphics_device_set_render_target_cube", as: GraphicsDeviceSetRenderTargetCubeRoute.self)
        graphicsDeviceSetRenderTargets = try library.resolve("cna_graphics_device_set_render_targets", as: GraphicsDeviceSetRenderTargetsRoute.self)
        graphicsDeviceGetRenderTargetCount = try library.resolve("cna_graphics_device_get_render_target_count", as: GraphicsDeviceGetRenderTargetCountRoute.self)
        graphicsDeviceGetTexture = try library.resolve("cna_graphics_device_get_texture", as: GraphicsDeviceGetTextureRoute.self)
        graphicsDeviceSetTexture = try library.resolve("cna_graphics_device_set_texture", as: GraphicsDeviceSetTextureRoute.self)
        graphicsDeviceDrawPrimitives = try library.resolve("cna_graphics_device_draw_primitives", as: GraphicsDeviceDrawPrimitivesRoute.self)
        graphicsDeviceDrawIndexedPrimitives = try library.resolve("cna_graphics_device_draw_indexed_primitives", as: GraphicsDeviceDrawIndexedPrimitivesRoute.self)
        graphicsDeviceDrawInstancedPrimitives = try library.resolve("cna_graphics_device_draw_instanced_primitives", as: GraphicsDeviceDrawInstancedPrimitivesRoute.self)
        graphicsDeviceDrawUserPrimitives = try library.resolve("cna_graphics_device_draw_user_primitives", as: GraphicsDeviceDrawUserPrimitivesRoute.self)
        graphicsDeviceDrawUserIndexedPrimitives = try library.resolve("cna_graphics_device_draw_user_indexed_primitives", as: GraphicsDeviceDrawUserIndexedPrimitivesRoute.self)
        primitiveTypeGetVertexCount = try library.resolve("cna_primitive_type_get_vertex_count", as: PrimitiveTypeGetVertexCountRoute.self)
        effectMaterialCreate = try library.resolve("cna_effect_material_create", as: EffectMaterialCreateRoute.self)
        directionalLightDestroy = try library.resolve("cna_directional_light_destroy", as: DirectionalLightDestroyRoute.self)
        directionalLightGetDiffuseColor = try library.resolve("cna_directional_light_get_diffuse_color", as: DirectionalLightGetDiffuseColorRoute.self)
        directionalLightSetDiffuseColor = try library.resolve("cna_directional_light_set_diffuse_color", as: DirectionalLightSetDiffuseColorRoute.self)
        directionalLightGetDirection = try library.resolve("cna_directional_light_get_direction", as: DirectionalLightGetDirectionRoute.self)
        directionalLightSetDirection = try library.resolve("cna_directional_light_set_direction", as: DirectionalLightSetDirectionRoute.self)
        directionalLightGetSpecularColor = try library.resolve("cna_directional_light_get_specular_color", as: DirectionalLightGetSpecularColorRoute.self)
        directionalLightSetSpecularColor = try library.resolve("cna_directional_light_set_specular_color", as: DirectionalLightSetSpecularColorRoute.self)
        directionalLightGetEnabled = try library.resolve("cna_directional_light_get_enabled", as: DirectionalLightGetEnabledRoute.self)
        directionalLightSetEnabled = try library.resolve("cna_directional_light_set_enabled", as: DirectionalLightSetEnabledRoute.self)
        basicEffectCreate = try library.resolve("cna_basic_effect_create", as: BasicEffectCreateRoute.self)
        effectMatricesGetWorld = try library.resolve("cna_effect_matrices_get_world", as: EffectMatricesGetWorldRoute.self)
        effectMatricesSetWorld = try library.resolve("cna_effect_matrices_set_world", as: EffectMatricesSetWorldRoute.self)
        effectMatricesGetView = try library.resolve("cna_effect_matrices_get_view", as: EffectMatricesGetViewRoute.self)
        effectMatricesSetView = try library.resolve("cna_effect_matrices_set_view", as: EffectMatricesSetViewRoute.self)
        effectMatricesGetProjection = try library.resolve("cna_effect_matrices_get_projection", as: EffectMatricesGetProjectionRoute.self)
        effectMatricesSetProjection = try library.resolve("cna_effect_matrices_set_projection", as: EffectMatricesSetProjectionRoute.self)
        effectFogGetColor = try library.resolve("cna_effect_fog_get_color", as: EffectFogGetColorRoute.self)
        effectFogSetColor = try library.resolve("cna_effect_fog_set_color", as: EffectFogSetColorRoute.self)
        effectFogGetEnabled = try library.resolve("cna_effect_fog_get_enabled", as: EffectFogGetEnabledRoute.self)
        effectFogSetEnabled = try library.resolve("cna_effect_fog_set_enabled", as: EffectFogSetEnabledRoute.self)
        effectFogGetStart = try library.resolve("cna_effect_fog_get_start", as: EffectFogGetStartRoute.self)
        effectFogSetStart = try library.resolve("cna_effect_fog_set_start", as: EffectFogSetStartRoute.self)
        effectFogGetEnd = try library.resolve("cna_effect_fog_get_end", as: EffectFogGetEndRoute.self)
        effectFogSetEnd = try library.resolve("cna_effect_fog_set_end", as: EffectFogSetEndRoute.self)
        effectLightsGetAmbientColor = try library.resolve("cna_effect_lights_get_ambient_color", as: EffectLightsGetAmbientColorRoute.self)
        effectLightsSetAmbientColor = try library.resolve("cna_effect_lights_set_ambient_color", as: EffectLightsSetAmbientColorRoute.self)
        effectLightsGetDirectionalLight = try library.resolve("cna_effect_lights_get_directional_light", as: EffectLightsGetDirectionalLightRoute.self)
        effectLightsGetEnabled = try library.resolve("cna_effect_lights_get_enabled", as: EffectLightsGetEnabledRoute.self)
        effectLightsSetEnabled = try library.resolve("cna_effect_lights_set_enabled", as: EffectLightsSetEnabledRoute.self)
        basicEffectGetVertexColorEnabled = try library.resolve("cna_basic_effect_get_vertex_color_enabled", as: BasicEffectGetVertexColorEnabledRoute.self)
        basicEffectSetVertexColorEnabled = try library.resolve("cna_basic_effect_set_vertex_color_enabled", as: BasicEffectSetVertexColorEnabledRoute.self)
        basicEffectGetPreferPerPixelLighting = try library.resolve("cna_basic_effect_get_prefer_per_pixel_lighting", as: BasicEffectGetPreferPerPixelLightingRoute.self)
        basicEffectSetPreferPerPixelLighting = try library.resolve("cna_basic_effect_set_prefer_per_pixel_lighting", as: BasicEffectSetPreferPerPixelLightingRoute.self)
        basicEffectGetDiffuseColor = try library.resolve("cna_basic_effect_get_diffuse_color", as: BasicEffectGetDiffuseColorRoute.self)
        basicEffectSetDiffuseColor = try library.resolve("cna_basic_effect_set_diffuse_color", as: BasicEffectSetDiffuseColorRoute.self)
        basicEffectGetEmissiveColor = try library.resolve("cna_basic_effect_get_emissive_color", as: BasicEffectGetEmissiveColorRoute.self)
        basicEffectSetEmissiveColor = try library.resolve("cna_basic_effect_set_emissive_color", as: BasicEffectSetEmissiveColorRoute.self)
        basicEffectGetSpecularColor = try library.resolve("cna_basic_effect_get_specular_color", as: BasicEffectGetSpecularColorRoute.self)
        basicEffectSetSpecularColor = try library.resolve("cna_basic_effect_set_specular_color", as: BasicEffectSetSpecularColorRoute.self)
        basicEffectGetSpecularPower = try library.resolve("cna_basic_effect_get_specular_power", as: BasicEffectGetSpecularPowerRoute.self)
        basicEffectSetSpecularPower = try library.resolve("cna_basic_effect_set_specular_power", as: BasicEffectSetSpecularPowerRoute.self)
        basicEffectGetAlpha = try library.resolve("cna_basic_effect_get_alpha", as: BasicEffectGetAlphaRoute.self)
        basicEffectSetAlpha = try library.resolve("cna_basic_effect_set_alpha", as: BasicEffectSetAlphaRoute.self)
        basicEffectGetTextureEnabled = try library.resolve("cna_basic_effect_get_texture_enabled", as: BasicEffectGetTextureEnabledRoute.self)
        basicEffectSetTextureEnabled = try library.resolve("cna_basic_effect_set_texture_enabled", as: BasicEffectSetTextureEnabledRoute.self)
        basicEffectGetTexture = try library.resolve("cna_basic_effect_get_texture", as: BasicEffectGetTextureRoute.self)
        basicEffectSetTexture = try library.resolve("cna_basic_effect_set_texture", as: BasicEffectSetTextureRoute.self)
        alphaTestEffectCreate = try library.resolve("cna_alpha_test_effect_create", as: AlphaTestEffectCreateRoute.self)
        alphaTestEffectGetDiffuseColor = try library.resolve("cna_alpha_test_effect_get_diffuse_color", as: AlphaTestEffectGetDiffuseColorRoute.self)
        alphaTestEffectSetDiffuseColor = try library.resolve("cna_alpha_test_effect_set_diffuse_color", as: AlphaTestEffectSetDiffuseColorRoute.self)
        alphaTestEffectGetAlpha = try library.resolve("cna_alpha_test_effect_get_alpha", as: AlphaTestEffectGetAlphaRoute.self)
        alphaTestEffectSetAlpha = try library.resolve("cna_alpha_test_effect_set_alpha", as: AlphaTestEffectSetAlphaRoute.self)
        alphaTestEffectGetTexture = try library.resolve("cna_alpha_test_effect_get_texture", as: AlphaTestEffectGetTextureRoute.self)
        alphaTestEffectSetTexture = try library.resolve("cna_alpha_test_effect_set_texture", as: AlphaTestEffectSetTextureRoute.self)
        alphaTestEffectGetVertexColorEnabled = try library.resolve("cna_alpha_test_effect_get_vertex_color_enabled", as: AlphaTestEffectGetVertexColorEnabledRoute.self)
        alphaTestEffectSetVertexColorEnabled = try library.resolve("cna_alpha_test_effect_set_vertex_color_enabled", as: AlphaTestEffectSetVertexColorEnabledRoute.self)
        alphaTestEffectGetAlphaFunction = try library.resolve("cna_alpha_test_effect_get_alpha_function", as: AlphaTestEffectGetAlphaFunctionRoute.self)
        alphaTestEffectSetAlphaFunction = try library.resolve("cna_alpha_test_effect_set_alpha_function", as: AlphaTestEffectSetAlphaFunctionRoute.self)
        alphaTestEffectGetReferenceAlpha = try library.resolve("cna_alpha_test_effect_get_reference_alpha", as: AlphaTestEffectGetReferenceAlphaRoute.self)
        alphaTestEffectSetReferenceAlpha = try library.resolve("cna_alpha_test_effect_set_reference_alpha", as: AlphaTestEffectSetReferenceAlphaRoute.self)
        dualTextureEffectCreate = try library.resolve("cna_dual_texture_effect_create", as: DualTextureEffectCreateRoute.self)
        dualTextureEffectGetDiffuseColor = try library.resolve("cna_dual_texture_effect_get_diffuse_color", as: DualTextureEffectGetDiffuseColorRoute.self)
        dualTextureEffectSetDiffuseColor = try library.resolve("cna_dual_texture_effect_set_diffuse_color", as: DualTextureEffectSetDiffuseColorRoute.self)
        dualTextureEffectGetAlpha = try library.resolve("cna_dual_texture_effect_get_alpha", as: DualTextureEffectGetAlphaRoute.self)
        dualTextureEffectSetAlpha = try library.resolve("cna_dual_texture_effect_set_alpha", as: DualTextureEffectSetAlphaRoute.self)
        dualTextureEffectGetTexture = try library.resolve("cna_dual_texture_effect_get_texture", as: DualTextureEffectGetTextureRoute.self)
        dualTextureEffectSetTexture = try library.resolve("cna_dual_texture_effect_set_texture", as: DualTextureEffectSetTextureRoute.self)
        dualTextureEffectGetVertexColorEnabled = try library.resolve("cna_dual_texture_effect_get_vertex_color_enabled", as: DualTextureEffectGetVertexColorEnabledRoute.self)
        dualTextureEffectSetVertexColorEnabled = try library.resolve("cna_dual_texture_effect_set_vertex_color_enabled", as: DualTextureEffectSetVertexColorEnabledRoute.self)
        environmentMapEffectCreate = try library.resolve("cna_environment_map_effect_create", as: EnvironmentMapEffectCreateRoute.self)
        environmentMapEffectGetDiffuseColor = try library.resolve("cna_environment_map_effect_get_diffuse_color", as: EnvironmentMapEffectGetDiffuseColorRoute.self)
        environmentMapEffectSetDiffuseColor = try library.resolve("cna_environment_map_effect_set_diffuse_color", as: EnvironmentMapEffectSetDiffuseColorRoute.self)
        environmentMapEffectGetEmissiveColor = try library.resolve("cna_environment_map_effect_get_emissive_color", as: EnvironmentMapEffectGetEmissiveColorRoute.self)
        environmentMapEffectSetEmissiveColor = try library.resolve("cna_environment_map_effect_set_emissive_color", as: EnvironmentMapEffectSetEmissiveColorRoute.self)
        environmentMapEffectGetAlpha = try library.resolve("cna_environment_map_effect_get_alpha", as: EnvironmentMapEffectGetAlphaRoute.self)
        environmentMapEffectSetAlpha = try library.resolve("cna_environment_map_effect_set_alpha", as: EnvironmentMapEffectSetAlphaRoute.self)
        environmentMapEffectGetTexture = try library.resolve("cna_environment_map_effect_get_texture", as: EnvironmentMapEffectGetTextureRoute.self)
        environmentMapEffectSetTexture = try library.resolve("cna_environment_map_effect_set_texture", as: EnvironmentMapEffectSetTextureRoute.self)
        environmentMapEffectGetEnvironmentMap = try library.resolve("cna_environment_map_effect_get_environment_map", as: EnvironmentMapEffectGetEnvironmentMapRoute.self)
        environmentMapEffectSetEnvironmentMap = try library.resolve("cna_environment_map_effect_set_environment_map", as: EnvironmentMapEffectSetEnvironmentMapRoute.self)
        environmentMapEffectGetAmount = try library.resolve("cna_environment_map_effect_get_amount", as: EnvironmentMapEffectGetAmountRoute.self)
        environmentMapEffectSetAmount = try library.resolve("cna_environment_map_effect_set_amount", as: EnvironmentMapEffectSetAmountRoute.self)
        environmentMapEffectGetSpecular = try library.resolve("cna_environment_map_effect_get_specular", as: EnvironmentMapEffectGetSpecularRoute.self)
        environmentMapEffectSetSpecular = try library.resolve("cna_environment_map_effect_set_specular", as: EnvironmentMapEffectSetSpecularRoute.self)
        environmentMapEffectGetFresnelFactor = try library.resolve("cna_environment_map_effect_get_fresnel_factor", as: EnvironmentMapEffectGetFresnelFactorRoute.self)
        environmentMapEffectSetFresnelFactor = try library.resolve("cna_environment_map_effect_set_fresnel_factor", as: EnvironmentMapEffectSetFresnelFactorRoute.self)
        skinnedEffectCreate = try library.resolve("cna_skinned_effect_create", as: SkinnedEffectCreateRoute.self)
        skinnedEffectGetDiffuseColor = try library.resolve("cna_skinned_effect_get_diffuse_color", as: SkinnedEffectGetDiffuseColorRoute.self)
        skinnedEffectSetDiffuseColor = try library.resolve("cna_skinned_effect_set_diffuse_color", as: SkinnedEffectSetDiffuseColorRoute.self)
        skinnedEffectGetEmissiveColor = try library.resolve("cna_skinned_effect_get_emissive_color", as: SkinnedEffectGetEmissiveColorRoute.self)
        skinnedEffectSetEmissiveColor = try library.resolve("cna_skinned_effect_set_emissive_color", as: SkinnedEffectSetEmissiveColorRoute.self)
        skinnedEffectGetSpecularColor = try library.resolve("cna_skinned_effect_get_specular_color", as: SkinnedEffectGetSpecularColorRoute.self)
        skinnedEffectSetSpecularColor = try library.resolve("cna_skinned_effect_set_specular_color", as: SkinnedEffectSetSpecularColorRoute.self)
        skinnedEffectGetSpecularPower = try library.resolve("cna_skinned_effect_get_specular_power", as: SkinnedEffectGetSpecularPowerRoute.self)
        skinnedEffectSetSpecularPower = try library.resolve("cna_skinned_effect_set_specular_power", as: SkinnedEffectSetSpecularPowerRoute.self)
        skinnedEffectGetAlpha = try library.resolve("cna_skinned_effect_get_alpha", as: SkinnedEffectGetAlphaRoute.self)
        skinnedEffectSetAlpha = try library.resolve("cna_skinned_effect_set_alpha", as: SkinnedEffectSetAlphaRoute.self)
        skinnedEffectGetPreferPerPixelLighting = try library.resolve("cna_skinned_effect_get_prefer_per_pixel_lighting", as: SkinnedEffectGetPreferPerPixelLightingRoute.self)
        skinnedEffectSetPreferPerPixelLighting = try library.resolve("cna_skinned_effect_set_prefer_per_pixel_lighting", as: SkinnedEffectSetPreferPerPixelLightingRoute.self)
        skinnedEffectGetTexture = try library.resolve("cna_skinned_effect_get_texture", as: SkinnedEffectGetTextureRoute.self)
        skinnedEffectSetTexture = try library.resolve("cna_skinned_effect_set_texture", as: SkinnedEffectSetTextureRoute.self)
        skinnedEffectGetWeightsPerVertex = try library.resolve("cna_skinned_effect_get_weights_per_vertex", as: SkinnedEffectGetWeightsPerVertexRoute.self)
        skinnedEffectSetWeightsPerVertex = try library.resolve("cna_skinned_effect_set_weights_per_vertex", as: SkinnedEffectSetWeightsPerVertexRoute.self)
        skinnedEffectSetBoneTransforms = try library.resolve("cna_skinned_effect_set_bone_transforms", as: SkinnedEffectSetBoneTransformsRoute.self)
        skinnedEffectCopyBoneTransforms = try library.resolve("cna_skinned_effect_copy_bone_transforms", as: SkinnedEffectCopyBoneTransformsRoute.self)
        effectCreateEmpty = try library.resolve("cna_effect_create_empty", as: EffectCreateEmptyRoute.self)
        effectCreateCompiled = try library.resolve("cna_effect_create_compiled", as: EffectCreateCompiledRoute.self)
        effectDestroy = try library.resolve("cna_effect_destroy", as: EffectDestroyRoute.self)
        effectClone = try library.resolve("cna_effect_clone", as: EffectCloneRoute.self)
        effectDisposeResources = try library.resolve("cna_effect_dispose", as: EffectDisposeRoute.self)
        effectApply = try library.resolve("cna_effect_apply", as: EffectApplyRoute.self)
        effectGetParameters = try library.resolve("cna_effect_get_parameters", as: EffectGetParametersRoute.self)
        effectGetTechniques = try library.resolve("cna_effect_get_techniques", as: EffectGetTechniquesRoute.self)
        effectGetCurrentTechnique = try library.resolve("cna_effect_get_current_technique", as: EffectGetCurrentTechniqueRoute.self)
        effectSetCurrentTechnique = try library.resolve("cna_effect_set_current_technique", as: EffectSetCurrentTechniqueRoute.self)
        effectGetGraphicsDevice = try library.resolve("cna_effect_get_graphics_device", as: EffectGetGraphicsDeviceRoute.self)
        effectTechniqueDestroy = try library.resolve("cna_effect_technique_destroy", as: EffectTechniqueDestroyRoute.self)
        effectTechniqueGetNameByteCount = try library.resolve("cna_effect_technique_get_name_byte_count", as: EffectTechniqueGetNameByteCountRoute.self)
        effectTechniqueCopyName = try library.resolve("cna_effect_technique_copy_name", as: EffectTechniqueCopyNameRoute.self)
        effectTechniqueGetPasses = try library.resolve("cna_effect_technique_get_passes", as: EffectTechniqueGetPassesRoute.self)
        effectTechniqueGetAnnotations = try library.resolve("cna_effect_technique_get_annotations", as: EffectTechniqueGetAnnotationsRoute.self)
        effectTechniqueCollectionDestroy = try library.resolve("cna_effect_technique_collection_destroy", as: EffectTechniqueCollectionDestroyRoute.self)
        effectTechniqueCollectionGetCount = try library.resolve("cna_effect_technique_collection_get_count", as: EffectTechniqueCollectionGetCountRoute.self)
        effectTechniqueCollectionGetAt = try library.resolve("cna_effect_technique_collection_get_at", as: EffectTechniqueCollectionGetAtRoute.self)
        effectPassDestroy = try library.resolve("cna_effect_pass_destroy", as: EffectPassDestroyRoute.self)
        effectPassGetNameByteCount = try library.resolve("cna_effect_pass_get_name_byte_count", as: EffectPassGetNameByteCountRoute.self)
        effectPassCopyName = try library.resolve("cna_effect_pass_copy_name", as: EffectPassCopyNameRoute.self)
        effectPassGetAnnotations = try library.resolve("cna_effect_pass_get_annotations", as: EffectPassGetAnnotationsRoute.self)
        effectPassApply = try library.resolve("cna_effect_pass_apply", as: EffectPassApplyRoute.self)
        effectPassCollectionDestroy = try library.resolve("cna_effect_pass_collection_destroy", as: EffectPassCollectionDestroyRoute.self)
        effectPassCollectionGetCount = try library.resolve("cna_effect_pass_collection_get_count", as: EffectPassCollectionGetCountRoute.self)
        effectPassCollectionGetAt = try library.resolve("cna_effect_pass_collection_get_at", as: EffectPassCollectionGetAtRoute.self)
        effectParameterDestroy = try library.resolve("cna_effect_parameter_destroy", as: EffectParameterDestroyRoute.self)
        effectParameterGetInfo = try library.resolve("cna_effect_parameter_get_info", as: EffectParameterGetInfoRoute.self)
        effectParameterGetNameByteCount = try library.resolve("cna_effect_parameter_get_name_byte_count", as: EffectParameterGetNameByteCountRoute.self)
        effectParameterCopyName = try library.resolve("cna_effect_parameter_copy_name", as: EffectParameterCopyNameRoute.self)
        effectParameterGetSemanticByteCount = try library.resolve("cna_effect_parameter_get_semantic_byte_count", as: EffectParameterGetSemanticByteCountRoute.self)
        effectParameterCopySemantic = try library.resolve("cna_effect_parameter_copy_semantic", as: EffectParameterCopySemanticRoute.self)
        effectParameterGetElements = try library.resolve("cna_effect_parameter_get_elements", as: EffectParameterGetElementsRoute.self)
        effectParameterGetStructureMembers = try library.resolve("cna_effect_parameter_get_structure_members", as: EffectParameterGetStructureMembersRoute.self)
        effectParameterGetAnnotations = try library.resolve("cna_effect_parameter_get_annotations", as: EffectParameterGetAnnotationsRoute.self)
        effectParameterGetValue = try library.resolve("cna_effect_parameter_get_value", as: EffectParameterGetValueRoute.self)
        effectParameterSetValue = try library.resolve("cna_effect_parameter_set_value", as: EffectParameterSetValueRoute.self)
        effectParameterGetValues = try library.resolve("cna_effect_parameter_get_values", as: EffectParameterGetValuesRoute.self)
        effectParameterSetValues = try library.resolve("cna_effect_parameter_set_values", as: EffectParameterSetValuesRoute.self)
        effectParameterGetValueStringByteCount = try library.resolve("cna_effect_parameter_get_value_string_byte_count", as: EffectParameterGetValueStringByteCountRoute.self)
        effectParameterCopyValueString = try library.resolve("cna_effect_parameter_copy_value_string", as: EffectParameterCopyValueStringRoute.self)
        effectParameterSetValueString = try library.resolve("cna_effect_parameter_set_value_string", as: EffectParameterSetValueStringRoute.self)
        effectParameterGetValueTexture = try library.resolve("cna_effect_parameter_get_value_texture", as: EffectParameterGetValueTextureRoute.self)
        effectParameterSetValueTexture = try library.resolve("cna_effect_parameter_set_value_texture", as: EffectParameterSetValueTextureRoute.self)
        effectParameterCollectionDestroy = try library.resolve("cna_effect_parameter_collection_destroy", as: EffectParameterCollectionDestroyRoute.self)
        effectParameterCollectionGetCount = try library.resolve("cna_effect_parameter_collection_get_count", as: EffectParameterCollectionGetCountRoute.self)
        effectParameterCollectionGetAt = try library.resolve("cna_effect_parameter_collection_get_at", as: EffectParameterCollectionGetAtRoute.self)
        effectAnnotationDestroy = try library.resolve("cna_effect_annotation_destroy", as: EffectAnnotationDestroyRoute.self)
        effectAnnotationGetInfo = try library.resolve("cna_effect_annotation_get_info", as: EffectAnnotationGetInfoRoute.self)
        effectAnnotationGetNameByteCount = try library.resolve("cna_effect_annotation_get_name_byte_count", as: EffectAnnotationGetNameByteCountRoute.self)
        effectAnnotationCopyName = try library.resolve("cna_effect_annotation_copy_name", as: EffectAnnotationCopyNameRoute.self)
        effectAnnotationGetSemanticByteCount = try library.resolve("cna_effect_annotation_get_semantic_byte_count", as: EffectAnnotationGetSemanticByteCountRoute.self)
        effectAnnotationCopySemantic = try library.resolve("cna_effect_annotation_copy_semantic", as: EffectAnnotationCopySemanticRoute.self)
        effectAnnotationGetValueBoolean = try library.resolve("cna_effect_annotation_get_value_boolean", as: EffectAnnotationGetValueBooleanRoute.self)
        effectAnnotationGetValueInt32 = try library.resolve("cna_effect_annotation_get_value_int32", as: EffectAnnotationGetValueInt32Route.self)
        effectAnnotationGetValueSingle = try library.resolve("cna_effect_annotation_get_value_single", as: EffectAnnotationGetValueSingleRoute.self)
        effectAnnotationGetValueVector2 = try library.resolve("cna_effect_annotation_get_value_vector2", as: EffectAnnotationGetValueVector2Route.self)
        effectAnnotationGetValueVector3 = try library.resolve("cna_effect_annotation_get_value_vector3", as: EffectAnnotationGetValueVector3Route.self)
        effectAnnotationGetValueVector4 = try library.resolve("cna_effect_annotation_get_value_vector4", as: EffectAnnotationGetValueVector4Route.self)
        effectAnnotationGetValueMatrix = try library.resolve("cna_effect_annotation_get_value_matrix", as: EffectAnnotationGetValueMatrixRoute.self)
        effectAnnotationGetValueStringByteCount = try library.resolve("cna_effect_annotation_get_value_string_byte_count", as: EffectAnnotationGetValueStringByteCountRoute.self)
        effectAnnotationCopyValueString = try library.resolve("cna_effect_annotation_copy_value_string", as: EffectAnnotationCopyValueStringRoute.self)
        effectAnnotationCollectionDestroy = try library.resolve("cna_effect_annotation_collection_destroy", as: EffectAnnotationCollectionDestroyRoute.self)
        effectAnnotationCollectionGetCount = try library.resolve("cna_effect_annotation_collection_get_count", as: EffectAnnotationCollectionGetCountRoute.self)
        effectAnnotationCollectionGetAt = try library.resolve("cna_effect_annotation_collection_get_at", as: EffectAnnotationCollectionGetAtRoute.self)
        graphicsDeviceClearOptions = try library.resolve("cna_graphics_device_clear_options", as: GraphicsDeviceClearOptionsRoute.self)
        graphicsDeviceCreate = try library.resolve("cna_graphics_device_create", as: GraphicsDeviceCreateRoute.self)
        graphicsDeviceDestroy = try library.resolve("cna_graphics_device_destroy", as: GraphicsDeviceDestroyRoute.self)
        graphicsDeviceGetIsDisposed = try library.resolve("cna_graphics_device_get_is_disposed", as: GraphicsDeviceGetIsDisposedRoute.self)
        graphicsDeviceDispose = try library.resolve("cna_graphics_device_dispose", as: GraphicsDeviceDisposeRoute.self)
        graphicsDevicePresent = try library.resolve("cna_graphics_device_present", as: GraphicsDevicePresentRoute.self)
        graphicsDeviceReset = try library.resolve("cna_graphics_device_reset", as: GraphicsDeviceResetRoute.self)
        graphicsDeviceResetWithParameters = try library.resolve("cna_graphics_device_reset_with_parameters", as: GraphicsDeviceResetWithParametersRoute.self)
        graphicsDeviceGetPresentationParameters = try library.resolve("cna_graphics_device_get_presentation_parameters", as: GraphicsDeviceGetPresentationParametersRoute.self)
        graphicsDeviceSetViewport = try library.resolve("cna_graphics_device_set_viewport", as: GraphicsDeviceSetViewportRoute.self)
        graphicsDeviceGetScissorRectangle = try library.resolve("cna_graphics_device_get_scissor_rectangle", as: GraphicsDeviceGetScissorRectangleRoute.self)
        graphicsDeviceSetScissorRectangle = try library.resolve("cna_graphics_device_set_scissor_rectangle", as: GraphicsDeviceSetScissorRectangleRoute.self)
        graphicsDeviceGetStatus = try library.resolve("cna_graphics_device_get_status", as: GraphicsDeviceGetStatusRoute.self)
        graphicsDeviceGetBlendFactor = try library.resolve("cna_graphics_device_get_blend_factor", as: GraphicsDeviceGetBlendFactorRoute.self)
        graphicsDeviceSetBlendFactor = try library.resolve("cna_graphics_device_set_blend_factor", as: GraphicsDeviceSetBlendFactorRoute.self)
        graphicsDeviceGetMultiSampleMask = try library.resolve("cna_graphics_device_get_multi_sample_mask", as: GraphicsDeviceGetMultiSampleMaskRoute.self)
        graphicsDeviceSetMultiSampleMask = try library.resolve("cna_graphics_device_set_multi_sample_mask", as: GraphicsDeviceSetMultiSampleMaskRoute.self)
        graphicsDeviceGetReferenceStencil = try library.resolve("cna_graphics_device_get_reference_stencil", as: GraphicsDeviceGetReferenceStencilRoute.self)
        graphicsDeviceSetReferenceStencil = try library.resolve("cna_graphics_device_set_reference_stencil", as: GraphicsDeviceSetReferenceStencilRoute.self)
        graphicsDeviceGetBlendState = try library.resolve("cna_graphics_device_get_blend_state", as: GraphicsDeviceGetBlendStateRoute.self)
        graphicsDeviceSetBlendState = try library.resolve("cna_graphics_device_set_blend_state", as: GraphicsDeviceSetBlendStateRoute.self)
        graphicsDeviceGetDepthStencilState = try library.resolve("cna_graphics_device_get_depth_stencil_state", as: GraphicsDeviceGetDepthStencilStateRoute.self)
        graphicsDeviceSetDepthStencilState = try library.resolve("cna_graphics_device_set_depth_stencil_state", as: GraphicsDeviceSetDepthStencilStateRoute.self)
        graphicsDeviceGetRasterizerState = try library.resolve("cna_graphics_device_get_rasterizer_state", as: GraphicsDeviceGetRasterizerStateRoute.self)
        graphicsDeviceSetRasterizerState = try library.resolve("cna_graphics_device_set_rasterizer_state", as: GraphicsDeviceSetRasterizerStateRoute.self)
        graphicsDeviceGetSamplerState = try library.resolve("cna_graphics_device_get_sampler_state", as: GraphicsDeviceGetSamplerStateRoute.self)
        graphicsDeviceSetSamplerState = try library.resolve("cna_graphics_device_set_sampler_state", as: GraphicsDeviceSetSamplerStateRoute.self)
        renderTargetSubscribeContentLost = try library.resolve("cna_render_target_subscribe_content_lost", as: RenderTargetSubscribeContentLostRoute.self)
        renderTargetUnsubscribeContentLost = try library.resolve("cna_render_target_unsubscribe_content_lost", as: RenderTargetUnsubscribeContentLostRoute.self)
        gameTick = try library.resolve("cna_game_tick", as: GameTickRoute.self)
        gameSuppressDraw = try library.resolve("cna_game_suppress_draw", as: GameSuppressDrawRoute.self)
        gameResetElapsedTime = try library.resolve("cna_game_reset_elapsed_time", as: GameResetElapsedTimeRoute.self)
        gameGetIsActive = try library.resolve("cna_game_get_is_active", as: GameGetIsActiveRoute.self)
        gameGetIsMouseVisible = try library.resolve("cna_game_get_is_mouse_visible", as: GameGetIsMouseVisibleRoute.self)
        gameSetIsMouseVisible = try library.resolve("cna_game_set_is_mouse_visible", as: GameSetIsMouseVisibleRoute.self)
        gameGetIsFixedTimeStep = try library.resolve("cna_game_get_is_fixed_time_step", as: GameGetIsFixedTimeStepRoute.self)
        gameSetIsFixedTimeStep = try library.resolve("cna_game_set_is_fixed_time_step", as: GameSetIsFixedTimeStepRoute.self)
        gameGetTargetElapsedTimeTicks = try library.resolve("cna_game_get_target_elapsed_time_ticks", as: GameGetTargetElapsedTimeTicksRoute.self)
        gameSetTargetElapsedTimeTicks = try library.resolve("cna_game_set_target_elapsed_time_ticks", as: GameSetTargetElapsedTimeTicksRoute.self)
        gameGetInactiveSleepTimeTicks = try library.resolve("cna_game_get_inactive_sleep_time_ticks", as: GameGetInactiveSleepTimeTicksRoute.self)
        gameSetInactiveSleepTimeTicks = try library.resolve("cna_game_set_inactive_sleep_time_ticks", as: GameSetInactiveSleepTimeTicksRoute.self)
        gameSubscribe = try library.resolve("cna_game_subscribe", as: GameSubscribeRoute.self)
        gameUnsubscribe = try library.resolve("cna_game_unsubscribe", as: GameUnsubscribeRoute.self)
        graphicsManagerCreateDevice = try library.resolve("cna_graphics_device_manager_create_device", as: GraphicsDeviceManagerCreateDeviceRoute.self)
        graphicsManagerBeginDraw = try library.resolve("cna_graphics_device_manager_begin_draw", as: GraphicsDeviceManagerBeginDrawRoute.self)
        graphicsManagerEndDraw = try library.resolve("cna_graphics_device_manager_end_draw", as: GraphicsDeviceManagerEndDrawRoute.self)
        graphicsManagerGetGraphicsDevice = try library.resolve("cna_graphics_device_manager_get_graphics_device", as: GraphicsDeviceManagerGetGraphicsDeviceRoute.self)
        graphicsManagerSubscribe = try library.resolve("cna_graphics_device_manager_subscribe", as: GraphicsDeviceManagerSubscribeRoute.self)
    }

    func check(_ result: UInt32, operation: String) throws {
        guard result == 0 else {
            throw CNAError.nativeFailure(operation: operation, result: result, message: lastErrorMessage())
        }
    }

    private func lastErrorMessage() -> String {
        var size: UInt64 = 0
        guard errorMessageSize(&size) == 0, size > 0, size <= UInt64(Int.max) else { return "no native diagnostic" }
        var bytes = [CChar](repeating: 0, count: Int(size) + 1)
        var required: UInt64 = 0
        let result = bytes.withUnsafeMutableBufferPointer {
            errorMessageCopy($0.baseAddress, size, &required)
        }
        guard result == 0 else { return "native diagnostic copy failed with result \(result)" }
        return String(decoding: bytes.prefix(Int(required)).map { UInt8(bitPattern: $0) }, as: UTF8.self)
    }
}
