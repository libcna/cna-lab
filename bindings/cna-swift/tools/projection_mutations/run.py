#!/usr/bin/env python3
"""Falsifiability controls for the projected exception payloads.

The payload of a projected CLR/XNA exception has four separately observable
parts — the class, the composed `Message`, `ParamName` and `HResult` — and a
test suite that asserted only the message would pass on a defect in any of the
other three. This harness plants one realistic defect at a time, runs the
tests, and requires them to fail; a surviving mutation is reported as a failure
of this gate.

Every mutation is one exact textual substitution, undone in a `finally`, and
the tree is proven byte-identical afterwards.
"""

from __future__ import annotations

import argparse
import json
import fcntl
import os
import re
import signal
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXCEPTIONS = ROOT / "Sources/CNA/CNAExceptions.swift"
COLLECTIONS = ROOT / "Sources/CNA/CNACollections.swift"
DICTIONARY = ROOT / "Sources/CNA/CNADictionary.swift"
SERVICES = ROOT / "Sources/CNA/Xna/Framework/GameServiceContainer.swift"
RESOURCE = ROOT / "Sources/CNA/Xna/Graphics/GraphicsResource.swift"
TEXTURE2D = ROOT / "Sources/CNA/Xna/Graphics/Texture2D.swift"
RENDER_TARGET = ROOT / "Sources/CNA/Xna/Graphics/RenderTarget2D.swift"
GAME = ROOT / "Sources/CNA/Xna/Framework/Game.swift"
DISPATCHER = ROOT / "Sources/CNA/Xna/Framework/FrameworkDispatcher.swift"
TITLE = ROOT / "Sources/CNA/Xna/Framework/TitleContainer.swift"
MOUSE = ROOT / "Sources/CNA/Xna/Input/Mouse.swift"
ADAPTER = ROOT / "Sources/CNA/Xna/Graphics/GraphicsAdapter.swift"
CONTENT = ROOT / "Sources/CNA/Xna/Content/ContentManager.swift"
MODEL = ROOT / "Sources/CNA/Xna/Graphics/Model/Model.swift"
MODEL_BONE = ROOT / "Sources/CNA/Xna/Graphics/Model/ModelBone.swift"
MODEL_MESH = ROOT / "Sources/CNA/Xna/Graphics/Model/ModelMesh.swift"
MODEL_PART = ROOT / "Sources/CNA/Xna/Graphics/Model/ModelMeshPart.swift"
MODEL_BONES = ROOT / "Sources/CNA/Xna/Graphics/Model/ModelBoneCollection.swift"
MODEL_SUPPORT = ROOT / "Sources/CNA/Xna/Graphics/Model/ModelSupport.swift"
STORAGE_DEVICE = ROOT / "Sources/CNA/Xna/Storage/StorageDevice.swift"
STORAGE_CONTAINER = ROOT / "Sources/CNA/Xna/Storage/StorageContainer.swift"
STORAGE_STREAM = ROOT / "Sources/CNA/CNAStorageStream.swift"
STORAGE_ASYNC = ROOT / "Sources/CNA/Xna/Storage/StorageAsyncResult.swift"
OCCLUSION = ROOT / "Sources/CNA/Xna/Graphics/OcclusionQuery.swift"
SOUND = ROOT / "Sources/CNA/Xna/Audio/SoundEffect.swift"
SOUND_INSTANCE = ROOT / "Sources/CNA/Xna/Audio/SoundEffectInstance.swift"
DYNAMIC_SOUND = ROOT / "Sources/CNA/Xna/Audio/DynamicSoundEffectInstance.swift"
RENDERER_DETAIL = ROOT / "Sources/CNA/Xna/Audio/RendererDetail.swift"
SONG = ROOT / "Sources/CNA/Xna/Media/Song.swift"
MEDIA_COLLECTIONS = ROOT / "Sources/CNA/Xna/Media/MediaCollections.swift"
MEDIA_ENTITIES = ROOT / "Sources/CNA/Xna/Media/MediaEntities.swift"
MEDIA_LIBRARY = ROOT / "Sources/CNA/Xna/Media/MediaLibrary.swift"
PICTURE = ROOT / "Sources/CNA/Xna/Media/PictureEntities.swift"
MEDIA_SOURCE = ROOT / "Sources/CNA/Xna/Media/MediaSource.swift"
MEDIA_PLAYER = ROOT / "Sources/CNA/Xna/Media/MediaPlayer.swift"
MEDIA_QUEUE = ROOT / "Sources/CNA/Xna/Media/MediaQueue.swift"
VIDEO_PLAYER = ROOT / "Sources/CNA/Xna/Media/VideoPlayer.swift"
GAMER_SERVICES = ROOT / "Sources/CNA/Xna/GamerServices/GamerServicesComponent.swift"
TOUCH_PANEL = ROOT / "Sources/CNA/Xna/Input/Touch/TouchPanel.swift"
DEVICEINFO = ROOT / "Sources/CNA/Xna/Framework/GraphicsDeviceInformation.swift"
RANKING = ROOT / "Sources/CNA/Xna/Graphics/GraphicsDeviceInformationComparer.swift"
WINDOW = ROOT / "Sources/CNA/Xna/Framework/GameWindow.swift"
CALLBACK_STATE = ROOT / "Sources/CNA/Runtime/CallbackState.swift"
MANAGER = ROOT / "Sources/CNA/Xna/Graphics/GraphicsDeviceManager.swift"
DRAWABLE = ROOT / "Sources/CNA/Xna/Framework/DrawableGameComponent.swift"
STATES = ROOT / "Sources/CNA/Xna/Graphics/GraphicsStates.swift"
RUNTIME_STATE = ROOT / "Sources/CNA/Runtime/RuntimeState.swift"
BATCH = ROOT / "Sources/CNA/Xna/Graphics/SpriteBatch.swift"
VERTEXDECL = ROOT / "Sources/CNA/Xna/Graphics/VertexDeclaration.swift"
VERTEXCOLOR = ROOT / "Sources/CNA/Xna/Graphics/VertexPositionColor.swift"
VERTEXNORMAL = ROOT / "Sources/CNA/Xna/Graphics/VertexPositionNormalTexture.swift"
DEVICE = ROOT / "Sources/CNA/Xna/Graphics/GraphicsDevice.swift"
STATEBRIDGE = ROOT / "Sources/CNA/Xna/Graphics/GraphicsStateNativeBridge.swift"
SAMPLERS = ROOT / "Sources/CNA/Xna/Graphics/SamplerStateCollection.swift"
DRAWABLE_COMPONENT = ROOT / "Sources/CNA/Xna/Framework/DrawableGameComponent.swift"
TEXTUREDATA = ROOT / "Sources/CNA/Xna/Graphics/TextureDataTransfer.swift"
VERTEXBUFFER = ROOT / "Sources/CNA/Xna/Graphics/VertexBuffer.swift"
INDEXBUFFER = ROOT / "Sources/CNA/Xna/Graphics/IndexBuffer.swift"
BUFFERSUPPORT = ROOT / "Sources/CNA/Xna/Graphics/BufferSupport.swift"
BUFFERBINDING = ROOT / "Sources/CNA/Xna/Graphics/VertexBufferBinding.swift"
DYNAMICVERTEX = ROOT / "Sources/CNA/Xna/Graphics/DynamicVertexBuffer.swift"
DYNAMICINDEX = ROOT / "Sources/CNA/Xna/Graphics/DynamicIndexBuffer.swift"
PROFILECAPS = ROOT / "Sources/CNA/Xna/Graphics/ProfileCapabilities.swift"
TEXTURECUBE = ROOT / "Sources/CNA/Xna/Graphics/TextureCube.swift"
TEXTURE3D = ROOT / "Sources/CNA/Xna/Graphics/Texture3D.swift"
RTCUBE = ROOT / "Sources/CNA/Xna/Graphics/RenderTargetCube.swift"
RTBINDING = ROOT / "Sources/CNA/Xna/Graphics/RenderTargetBinding.swift"
RTSUPPORT = ROOT / "Sources/CNA/Xna/Graphics/RenderTargetSupport.swift"
RUNTIME = ROOT / "Sources/CNA/Runtime/RuntimeState.swift"
TEXTURECOLLECTION = ROOT / "Sources/CNA/Xna/Graphics/TextureCollection.swift"
EFFECT = ROOT / "Sources/CNA/Xna/Graphics/Effect.swift"
EFFECTCOLLECTIONS = ROOT / "Sources/CNA/Xna/Graphics/EffectCollections.swift"
EFFECTSUPPORT = ROOT / "Sources/CNA/Xna/Graphics/EffectSupport.swift"
DRAW = ROOT / "Sources/CNA/Xna/Graphics/GraphicsDeviceDraw.swift"
FONT = ROOT / "Sources/CNA/Xna/Graphics/SpriteFont.swift"
BUILDER = ROOT / "Sources/CNA/CNAStringBuilder.swift"
LIGHT = ROOT / "Sources/CNA/Xna/Graphics/DirectionalLight.swift"
STOCKSUPPORT = ROOT / "Sources/CNA/Xna/Graphics/StockEffectSupport.swift"
BASICEFFECT = ROOT / "Sources/CNA/Xna/Graphics/BasicEffect.swift"
ALPHATEST = ROOT / "Sources/CNA/Xna/Graphics/AlphaTestEffect.swift"
DUALTEXTURE = ROOT / "Sources/CNA/Xna/Graphics/DualTextureEffect.swift"
ENVMAP = ROOT / "Sources/CNA/Xna/Graphics/EnvironmentMapEffect.swift"
SKINNED = ROOT / "Sources/CNA/Xna/Graphics/SkinnedEffect.swift"
BINARY_READER = ROOT / "Sources/CNA/CNABinaryReader.swift"
RESOURCE_MANAGER = ROOT / "Sources/CNA/CNAResourceManager.swift"
CONTENT_READER = ROOT / "Sources/CNA/Xna/Content/ContentReader.swift"
TYPE_READER = ROOT / "Sources/CNA/Xna/Content/ContentTypeReader.swift"
TYPE_READER_MANAGER = ROOT / "Sources/CNA/Xna/Content/ContentTypeReaderManager.swift"
RESOURCE_CONTENT_MANAGER = ROOT / "Sources/CNA/Xna/Content/ResourceContentManager.swift"
COMPONENT_MODEL = ROOT / "Sources/CNA/CNAComponentModel.swift"
DESIGN = ROOT / "Sources/CNA/Xna/Design/DesignConverters.swift"

# Two mutation harnesses editing the same working tree at once corrupts both.
# `tools/native_abi/mutations.py` mutates NativeManifest.swift,
# NativeFunctions.swift, CNAShim.h and Keyboard.swift; this one mutates twenty
# other files under Sources/ and runs the whole test suite for each. Run them
# together and a `swift test` here can compile the other harness's planted
# defect, reporting a CAUGHT the mutation under test did not earn -- a false
# pass, which is the direction that hides a survivor. That happened once,
# during Foundation 48, and cost a full re-run to be sure of the result.
#
# The lock is advisory and holds between these two scripts only. It is not a
# claim that the tree is otherwise untouched.
TREE_LOCK = ROOT / ".mutation-gate.lock"


def acquire_tree_lock(name: str):
    """Take the exclusive tree lock, or return None if another gate holds it."""
    handle = TREE_LOCK.open("w", encoding="utf-8")
    try:
        fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.close()
        return None
    handle.write(f"{os.getpid()} {name}\n")
    handle.flush()
    return handle


# The WHOLE suite runs for every mutation, deliberately.
#
# The first version of this harness filtered to the suites that assert a
# projected payload. `swift test --filter` in this toolchain honours only one
# pattern -- an alternation regex and repeated `--filter` flags both selected a
# single suite -- so the baseline and every mutation ran fifteen tests instead
# of four hundred, and three real defects came back SURVIVED. Running
# everything costs about fifteen seconds a mutation and cannot be wrong in that
# way.

MUTATIONS: list[tuple[str, str, Path, str, str]] = [
    # ---- Foundation 107: final strict-surface closure ----------------------
    (
        "dynamic-islooped-answers-true",
        "the dynamic subclass reports looping even though XNA's redeclared "
        "getter always answers false",
        DYNAMIC_SOUND,
        "        public func GetIsLooped() throws -> Bool {\n"
        "            _ = try validatedDynamicHandle()\n"
        "            return false\n"
        "        }",
        "        public func GetIsLooped() throws -> Bool {\n"
        "            _ = try validatedDynamicHandle()\n"
        "            return true\n"
        "        }",
    ),
    (
        "serialization-constructor-corrupts-hresult",
        "the admitted serialization carrier changes the exception's restored "
        "HRESULT instead of forwarding the serialized base state",
        EXCEPTIONS,
        "        self.HResult = info.hResult\n"
        "        _ = context.state",
        "        self.HResult = info.hResult &+ 1\n"
        "        _ = context.state",
    ),
    (
        "backbuffer-readback-skips-profile-refusal",
        "GetBackBufferData continues into array validation on Reach instead of "
        "raising XNA's profile refusal first",
        DEVICE,
        "            guard profileCapabilities.getBackBufferData else {\n"
        "                try profileCapabilities.throwNotSupported(",
        "            if false {\n"
        "                try profileCapabilities.throwNotSupported(",
    ),
    (
        "storage-stream-setlength-adds-one",
        "the projected duplex stream resizes a storage file to one byte beyond "
        "the requested length",
        STORAGE_STREAM,
        "            functions.storageStreamSetLength(live, value),",
        "            functions.storageStreamSetLength(live, value &+ 1),",
    ),
    # ---- Foundation 105: Design converters and InstanceDescriptor ----------
    (
        "design-invalid-component-count-accepted",
        "a vector string with one missing component is accepted",
        DESIGN,
        "    guard parts.count == names.count else {",
        "    guard parts.count + 1 == names.count else {",
    ),
    (
        "design-culture-list-separator-ignored",
        "formatted values always use comma instead of the culture separator",
        DESIGN,
        "    values.joined(separator: culture.TextInfo.ListSeparator + \" \")",
        "    values.joined(separator: \", \")",
    ),
    (
        "design-instance-descriptor-not-invoked",
        "TypeConverter.ConvertFrom returns a descriptor instead of invoking it",
        COMPONENT_MODEL,
        "        if let descriptor = value as? CNAInstanceDescriptor { return try descriptor.Invoke() }",
        "        if let descriptor = value as? CNAInstanceDescriptor { return descriptor }",
    ),
    (
        "design-property-order-reversed",
        "Vector3 publishes its property descriptors in Z,Y,X order",
        DESIGN,
        "                (\"X\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.X }),\n"
        "                (\"Y\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Y }),\n"
        "                (\"Z\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Z }),",
        "                (\"Z\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Z }),\n"
        "                (\"Y\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Y }),\n"
        "                (\"X\", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.X }),",
    ),
    # ---- Foundation 104: the managed ContentReader family -------------------
    (
        "binary-reader-endian-swap",
        "UInt16 decoding uses big endian, reversing every two-byte primitive",
        BINARY_READER,
        "        return UInt16(buffer[0]) | (UInt16(buffer[1]) << 8)",
        "        return UInt16(buffer[1]) | (UInt16(buffer[0]) << 8)",
    ),
    (
        "binary-reader-string-length-off-by-one",
        "ReadString consumes one byte beyond its seven-bit encoded payload",
        BINARY_READER,
        "        let length = try Read7BitEncodedInt()\n"
        "        guard length >= 0 else {",
        "        let length = try Read7BitEncodedInt() + 1\n"
        "        guard length >= 0 else {",
    ),
    (
        "content-reader-index-zero-based",
        "ReadObject treats XNA's one-based reader marker as a zero-based index",
        CONTENT_READER,
        "            let index = encodedIndex - 1\n"
        "            guard index >= 0, Int(index) < typeReaders.count else {\n"
        "                throw contentLoadException(Self.badXnb)\n"
        "            }\n"
        "            return try invokeReader(\n"
        "                typeReaders[Int(index)], existingInstance: existingInstance)",
        "            let index = encodedIndex\n"
        "            guard index >= 0, Int(index) < typeReaders.count else {\n"
        "                throw contentLoadException(Self.badXnb)\n"
        "            }\n"
        "            return try invokeReader(\n"
        "                typeReaders[Int(index)], existingInstance: existingInstance)",
    ),
    (
        "content-reader-skips-shared-resource",
        "the shared-resource pass reads no objects, leaving every fixup unresolved",
        CONTENT_READER,
        "            for _ in 0..<count {\n"
        "                resources.append(try readObjectErased())\n"
        "            }",
        "            for _ in 0..<0 {\n"
        "                resources.append(try readObjectErased())\n"
        "            }",
    ),
    (
        "content-reader-external-reference-not-normalized",
        "external references bypass XNA path cleaning and miss the shared cache key",
        CONTENT_READER,
        "            let combined = directory.isEmpty ? reference : directory + \"\\\\\" + reference\n"
        "            let clean = Microsoft.Xna.Framework.TitleContainer.GetCleanPath(combined)",
        "            let combined = reference\n"
        "            let clean = combined",
    ),
    (
        "generic-reader-target-type-wrong",
        "ContentTypeReader<T> advertises Any instead of its stable T identity",
        TYPE_READER,
        "            super.init(targetType: T.self)",
        "            super.init(targetType: Any.self)",
    ),
    (
        "reader-creator-registry-wrong-name",
        "manifest lookup folds a case-sensitive reader name before invoking its creator",
        TYPE_READER_MANAGER,
        "                        guard let creator = creators[name] else {",
        "                        guard let creator = creators[name.lowercased()] else {",
    ),
    (
        "content-manager-readasset-bypasses-cache",
        "Load calls ReadAsset again even when the normalized asset is already cached",
        CONTENT,
        "            if let cached = assets[key] {",
        "            if false, let cached = assets[key] {",
    ),
    (
        "content-manager-openstream-ignores-root",
        "OpenStream resolves every XNB outside the configured RootDirectory",
        CONTENT,
        "            let root = storedRootDirectory ?? \"\"",
        "            let root = \"\"",
    ),
    (
        "resource-content-manager-bypasses-resource-manager",
        "ResourceContentManager never dispatches the selected key to ResourceManager",
        RESOURCE_CONTENT_MANAGER,
        "try resourceManager.GetObject(assetName)",
        "Optional<Any>.none",
    ),
    (
        "resource-manager-wrong-key-lookup",
        "ResourceManager tests a fixed wrong key instead of the requested key",
        RESOURCE_MANAGER,
        "        if resourceFactories[name] != nil {",
        "        if resourceFactories[\"wrong-key\"] != nil {",
    ),
    # ---- Foundation 99: the touch panel --------------------------------------
    (
        "touch-state-reads-past-the-count",
        "GetState reading all eight native slots instead of the counted "
        "prefix, so last frame's leftovers arrive as live touches",
        TOUCH_PANEL,
        "            for raw in all.prefix(Int(native.touch_count)) {",
        "            for raw in all {",
    ),
    (
        "touch-width-setter-does-not-push",
        "DisplayWidth stored without reaching the runtime, so the panel and "
        "the property disagree about the display",
        TOUCH_PANEL,
        "                storedDisplayWidth = newValue\n"
        "                push { rt in\n"
        "                    rt.functions.touchPanelSetDisplayWidth(rt.gameHandle, newValue)\n"
        "                }",
        "                storedDisplayWidth = newValue",
    ),
    (
        "touch-height-reads-the-width",
        "DisplayHeight answering from the width route, so a non-square display "
        "reports itself square",
        TOUCH_PANEL,
        "                ? rt.functions.touchPanelGetDisplayWidth(rt.gameHandle, &value)\n"
        "                : rt.functions.touchPanelGetDisplayHeight(rt.gameHandle, &value)",
        "                ? rt.functions.touchPanelGetDisplayWidth(rt.gameHandle, &value)\n"
        "                : rt.functions.touchPanelGetDisplayWidth(rt.gameHandle, &value)",
    ),
    (
        "touch-gesture-mask-loses-its-bits",
        "SetEnabledGestures sending an empty mask, so no gesture is ever "
        "reported however many a caller asked for",
        TOUCH_PANEL,
        "                rt.functions.touchPanelSetEnabledGestures(\n"
        "                    rt.gameHandle, UInt32(bitPattern: value.rawValue)),",
        "                rt.functions.touchPanelSetEnabledGestures(\n"
        "                    rt.gameHandle, 0),",
    ),
    # ---- Foundation 98: the gamer-services component -------------------------
    # WITHDRAWN, both: "gamer-component-initialize-does-nothing" and
    # "gamer-component-update-initialises-instead". Neither is falsifiable
    # through the projected surface, and the reason is the PROFILE rather than
    # the code: the XNA 4.0 Windows contract declares exactly one type in this
    # namespace, GamerServicesComponent, and no GamerServicesDispatcher. CNA
    # does publish the dispatcher's state --
    # cna_gamer_services_dispatcher_get_is_initialized -- but no projected
    # member consumes it, so the Foundation 67 rule forbids binding it, and
    # without it Initialize and Update are indistinguishable from doing
    # nothing: both routes accept on this host and neither reports back.
    #
    # Reinstate if a profile that declares GamerServicesDispatcher is ever
    # admitted, which would give that route a member and this pair an
    # observable.

    # ---- Foundation 97: the video player ------------------------------------
    (
        "video-play-accepts-a-handleless-video",
        "Play passing a zero handle to the ABI for a video built from values, "
        "instead of saying it cannot be played",
        VIDEO_PLAYER,
        "            guard video.handle != 0 else {",
        "            if false {",
    ),
    (
        "video-texture-invented",
        "GetTexture building a texture from an out-parameter CNA never wrote, "
        "because nothing is playing",
        VIDEO_PLAYER,
        "            guard available != 0 else {\n"
        "                throw CNAError.nativeFailure(\n"
        "                    operation: \"VideoPlayer.GetTexture\", result: 1,",
        "            if false {\n"
        "                throw CNAError.nativeFailure(\n"
        "                    operation: \"VideoPlayer.GetTexture\", result: 1,",
    ),
    (
        "video-looped-writer-does-not-reach-the-runtime",
        "SetIsLooped accepted without reaching CNA, so the flag a caller sets "
        "and the flag the getter answers disagree",
        VIDEO_PLAYER,
        "                runtime.functions.videoPlayerSetIsLooped(live, value ? 1 : 0),",
        "                runtime.functions.videoPlayerSetIsLooped(live, 0),",
    ),
    (
        "video-volume-reads-the-mute-flag",
        "Volume answering from the mute route, so a muted player reports a "
        "volume of one and an unmuted one reports zero",
        VIDEO_PLAYER,
        "            guard runtime.functions.videoPlayerGetVolume(handle, &value) == 0",
        "            guard runtime.functions.videoPlayerGetVolume(handle, &value) == 1",
    ),
    # ---- Foundation 96: the media player ------------------------------------
    # WITHDRAWN: "player-play-index-overload-ignores-its-index" -- the
    # three-argument Play discarding its index. Reaching it needs a
    # SongCollection with two songs, and the only producer of one is a media
    # library; this machine's music store is empty and filling it is not the
    # suite's to do. Reinstate on a host with music.
    # WITHDRAWN: "player-visualization-copies-frequencies-twice" -- filling
    # the samples buffer from the frequencies. Measured unfalsifiable: this
    # renderer answers 256 zeros in BOTH buffers with nothing playing, so the
    # two are indistinguishable. The test records that measurement rather than
    # asserting a number it did not check. Reinstate where a renderer produces
    # real visualisation data.
    (
        "player-volume-writer-does-not-reach-the-runtime",
        "SetVolume accepted without reaching CNA, so the value a caller sets "
        "and the value the getter answers disagree",
        MEDIA_PLAYER,
        "                rt.functions.mediaPlayerSetVolume(rt.gameHandle, value),",
        "                rt.functions.mediaPlayerSetVolume(rt.gameHandle, 1),",
    ),
    (
        "queue-index-unchecked",
        "the queue's indexer accepting an index it does not have, so CNA's own "
        "failure reaches the caller instead of the exception XNA declares",
        MEDIA_QUEUE,
        "                guard index >= 0, index < total else {",
        "                if false {",
    ),
    # WITHDRAWN: "queue-active-song-invented" -- ignoring the availability
    # flag for the queue's active song. Falsifiable only against an EMPTY
    # queue, and the queue is process-wide: it outlives the game that filled
    # it, so whether it is empty depends on what ran before in the same
    # process. A test cannot arrange that, and clearing the queue is not a
    # member XNA declares.
    # ---- Foundation 95: playlists and media sources -------------------------
    (
        "media-source-list-stops-one-short",
        "the source enumeration dropping its last entry, so a caller cannot "
        "open the library that source names",
        MEDIA_SOURCE,
        "            for index in 0..<count {",
        "            for index in 0..<max(0, count - 1) {",
    ),
    # WITHDRAWN: "media-source-index-always-zero" -- every source carrying
    # index zero. This host publishes exactly one media source, so index zero
    # IS the right answer for it and the mutant is indistinguishable. It needs
    # a machine with two sources, which is not something a suite can arrange.
    (
        "library-playlists-come-from-the-wrong-route",
        "MediaLibrary.Playlists answering the songs collection, so a caller "
        "enumerating playlists walks songs",
        MEDIA_LIBRARY,
        "                    runtime.functions.mediaLibraryGetPlaylists(live, &produced),",
        "                    runtime.functions.mediaLibraryGetSongs(live, &produced),",
    ),
    (
        "library-own-source-never-refuses",
        "MediaLibrary.MediaSource answering nil for a disposed library instead "
        "of refusing, so a released library looks like one with no source",
        MEDIA_LIBRARY,
        "                _ = try validated()\n"
        "                return nil",
        "                return nil",
    ),
    # ---- Foundation 94: the picture branch ---------------------------------
    # WITHDRAWN, all three together: "library-saved-pictures-come-from-the-wrong-route",
    # "picture-thumbnail-reads-the-full-image" and "picture-date-loses-its-epoch".
    # Falsifying any of them needs a REAL picture, and the only way to get one
    # on this host is MediaLibrary.SavePicture -- which writes into the user's
    # own media library, where CNA publishes no route to remove what it put
    # there. That test was written, passed, and was removed along with the file
    # it left in ~/Pictures. A suite may not leave things in a person's photo
    # album. Reinstate all three on a host whose media store already holds
    # pictures the suite did not have to create.
    # WITHDRAWN: "library-root-album-invented" -- ignoring the availability
    # flag for the root picture album. Falsifiable only when the library has NO
    # pictures, because with any picture present the flag is true and the
    # mutant behaves identically. Whether this machine's media store is empty
    # is the user's business, and the suite must not arrange it either way --
    # emptying it is destructive and filling it leaves files behind, which is
    # exactly the mistake the three withdrawals above record.
    (
        "library-unknown-token-answers-a-picture",
        "GetPictureFromToken handing back whatever the out-parameter held for "
        "a token the library does not have",
        MEDIA_LIBRARY,
        "            guard available != 0 else {\n"
        "                throw CNAError.nativeFailure(\n"
        "                    operation: \"MediaLibrary.GetPictureFromToken\", result: 1,",
        "            if false {\n"
        "                throw CNAError.nativeFailure(\n"
        "                    operation: \"MediaLibrary.GetPictureFromToken\", result: 1,",
    ),
    # ---- Foundation 93: the media graph ------------------------------------
    (
        "media-collection-index-unchecked",
        "the indexer accepting an index the collection does not have, so CNA's "
        "own failure reaches the caller instead of the exception XNA declares",
        MEDIA_COLLECTIONS,
        "        guard index >= 0, index < total else {\n"
        "            throw CNAArgumentOutOfRangeException(paramName: \"index\")\n"
        "        }",
        "        if false {}",
    ),
    (
        "media-collection-negative-index-allowed",
        "the lower bound dropped, so a negative index is handed to the ABI",
        MEDIA_COLLECTIONS,
        "        guard index >= 0, index < total else {",
        "        guard index < total else {",
    ),
    # WITHDRAWN: "media-collection-disposal-skips-the-mark" -- releasing a
    # collection without CNA's own dispose. Measured rather than reasoned:
    # disposal here IS shared, so a second facade over the same collection sees
    # it -- and it still sees it with the dispose call removed, because
    # `cna_..._collection_destroy` alone already makes the collection report
    # disposed. The two halves are therefore not separable through anything
    # this projection publishes. The binding keeps calling both, in CNA's own
    # documented order, because the header says dispose is the canonical
    # disposal and destroy only releases a handle.
    # WITHDRAWN: "media-entity-borrowed-handle-destroyed" -- a borrowed entity
    # destroying a handle the library owns. Unreachable on this host, and the
    # reason is the machine rather than the code: a borrowed Artist, Album or
    # Genre comes only from a song or album that HAS a library context, and this
    # media store is empty -- Song.Artist on a file-created song reports no
    # context at all. Reinstate it on a host with media, where the difference
    # between disposing a borrowed handle and releasing an owned one is exactly
    # what a caller would trip over.
    (
        "song-relation-ignores-availability",
        "Song.Artist handing back whatever the out-parameter held when CNA "
        "reported no library context, which is an untouched handle",
        SONG,
        "                try Song.requireLibraryContext(available, \"Artist\")",
        "                _ = available",
    ),
    (
        "library-collection-comes-from-the-wrong-route",
        "MediaLibrary.Artists answering the songs collection, so a caller "
        "enumerating artists walks songs instead",
        MEDIA_LIBRARY,
        "                    runtime.functions.mediaLibraryGetArtists(live, &produced),",
        "                    runtime.functions.mediaLibraryGetSongs(live, &produced),",
    ),
    # ---- Foundation 92: the song -------------------------------------------
    (
        "song-equality-compares-handles",
        "Song.Equals testing handles instead of asking CNA, so two facades "
        "over one song read as two songs",
        SONG,
        "            var equal: UInt8 = 0\n"
        "            guard runtime.functions.songEquals(\n"
        "                handle, other.handle, &equal) == 0 else { return self === other }\n"
        "            return equal != 0",
        "            return handle == other.handle",
    ),
    (
        "song-hash-is-invented",
        "GetHashCode answering a local value instead of CNA's, so two equal "
        "songs hash differently",
        SONG,
        "            guard runtime.functions.songGetHashCode(handle, &value) == 0 else {\n"
        "                return 0\n"
        "            }\n"
        "            return value",
        "            _ = value\n"
        "            return Int32(truncatingIfNeeded: handle)",
    ),
    # WITHDRAWN: "song-dispose-skips-the-managed-half" -- releasing the handle
    # without CNA's own dispose. Unobservable through the projection today, and
    # the reason is the shape of the type rather than a missing test: the only
    # reference that could see the disposal MARK is the one being released in
    # the same call, and `cna_song_destroy` follows immediately. Two facades
    # over an equal song were measured to be separate objects with independent
    # disposal, so a second reference cannot see it either. It becomes
    # observable when SongCollection lands and can hold a song the caller also
    # disposes directly; reinstate it then.
    (
        "song-tostring-hides-a-refusal",
        "ToString answering the name for a disposed song, which cannot be read "
        "at all",
        SONG,
        "            (try? Name) ?? \"\"",
        "            (try? Name) ?? \"probe song\"",
    ),
    # ---- Foundation 91: the renderer detail --------------------------------
    (
        "renderer-detail-compares-the-label",
        "equality including the friendly name, so the same renderer under two "
        "labels reads as two renderers",
        RENDERER_DETAIL,
        "            lhs.rendererId == rhs.rendererId\n"
        "        }",
        "            lhs.rendererId == rhs.rendererId\n"
        "                && lhs.friendlyName == rhs.friendlyName\n"
        "        }",
    ),
    (
        "renderer-detail-hashes-the-label",
        "hashing the friendly name, so two equal details hash differently and "
        "the hash contract breaks",
        RENDERER_DETAIL,
        "            Microsoft.Xna.Framework.Audio.RendererDetail.stringHash(rendererId)",
        "            Microsoft.Xna.Framework.Audio.RendererDetail.stringHash(friendlyName)",
    ),
    (
        "renderer-detail-renders-the-id",
        "ToString answering the id, which is what a caller passes back rather "
        "than what a person reads",
        RENDERER_DETAIL,
        "        public func ToString() -> String { friendlyName }",
        "        public func ToString() -> String { rendererId }",
    ),
    # ---- Foundation 90: the dynamic instance -------------------------------
    (
        "dynamic-queue-grows-without-limit",
        "SubmitBuffer accepted past the pending-buffer limit, which this "
        "runtime allows and XNA does not -- the queue then fails somewhere "
        "else entirely",
        DYNAMIC_SOUND,
        "            guard try PendingBufferCount < DynamicSoundEffectInstance.pendingBufferLimit else {",
        "            if false {",
    ),
    (
        "dynamic-subscription-outlives-the-instance",
        "disposal leaving the BufferNeeded registration alive, so a native "
        "callback keeps addressing a box that has been released",
        DYNAMIC_SOUND,
        "            releaseSubscription()\n"
        "            dynamicHandle = 0",
        "            dynamicHandle = 0",
    ),
    # WITHDRAWN: "dynamic-submit-ignores-the-range" -- passing the whole buffer
    # where the caller gave a range. It cannot be falsified through the
    # projected surface, and the reason is structural rather than a missing
    # test: nothing CNA publishes reports how many BYTES are queued.
    # `cna_dynamic_sound_effect_instance_get_pending_buffer_count` counts
    # buffers, and one submit is one buffer whether it carried two bytes or
    # two hundred. The range reaches the ABI correctly -- the manifest and the
    # native ABI gate both check the argument positions -- but its effect is
    # invisible from Swift, so a mutation on it can only ever survive.
    (
        "dynamic-conversion-uses-a-fixed-rate",
        "GetSampleSizeInBytes answering from a static conversion instead of "
        "the instance's own sample rate and channel count",
        DYNAMIC_SOUND,
        "                    .dynamicSoundEffectInstanceGetSampleSizeInBytes(\n"
        "                        live,\n"
        "                        Microsoft.Xna.Framework.Audio.SoundEffect.ticks(from: duration),\n"
        "                        &bytes),",
        "                    .dynamicSoundEffectInstanceGetSampleSizeInBytes(\n"
        "                        live, 0, &bytes),",
    ),
    # ---- Foundation 89: the sound effect pair ------------------------------
    (
        "sound-buffer-alignment-unchecked",
        "the constructor accepting a buffer that is not a whole number of "
        "PCM16 frames, which CNA decodes as a shorter sound instead",
        SOUND,
        "            guard length > 0, length % blockAlign == 0 else {",
        "            if false {",
    ),
    (
        "sound-loop-region-unchecked",
        "a loop region past the end accepted, so the sound loops over samples "
        "the caller never supplied",
        SOUND,
        "            guard loopStart >= 0, loopLength >= 0,\n"
        "                  loopStart <= frames, loopStart <= frames - loopLength else {",
        "            if false {",
    ),
    (
        "sound-disposal-uses-the-bcl-message",
        "the disposal refusal carrying the BCL's generic text instead of the "
        "one XNA's audio types pass",
        SOUND,
        "                    message: Microsoft.Xna.Framework.Audio\n"
        "                        .objectDisposedMessage)",
        "                    message: nil)",
    ),
    (
        "instance-loop-changes-after-play",
        "SetIsLooped accepted once playback has started, so the flag changes "
        "where nothing can act on it",
        SOUND_INSTANCE,
        "            guard !hasPlayed else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: SoundEffectInstance.invalidIsLoopedCallMessage)\n"
        "            }",
        "            if false {}",
    ),
    (
        "instance-apply3d-after-play",
        "Apply3D accepted after the first Play, when it is too late to make "
        "the sound 3D at all",
        SOUND_INSTANCE,
        "            guard !hasPlayed else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: SoundEffectInstance.invalidApply3DCallMessage)\n"
        "            }",
        "            if false {}",
    ),
    (
        "instance-pan-on-a-3d-sound",
        "SetPan accepted on a sound placed in space, where the emitter's "
        "position is what decides where it is heard",
        SOUND_INSTANCE,
        "            guard !is3D, !hasPlayed else {",
        "            if false {",
    ),
    (
        "sound-range-constructor-takes-the-canonical-route",
        "the seven-argument constructor reaching the route that ignores offset, "
        "count and the loop points, so a caller's range is silently the whole "
        "buffer",
        SOUND,
        "                    if useRange {",
        "                    if false {",
    ),
    (
        "sound-name-writer-does-not-reach-the-runtime",
        "SetName updating only the cached copy, so the name the runtime holds "
        "and the name the getter answers disagree",
        SOUND,
        "                    runtime.functions.soundEffectSetName(live, view)",
        "                    UInt32(0 * view.byte_length)",
    ),
    (
        "sound-effect-never-joins-the-parent-registry",
        "an effect that does not register with the runtime, so one the caller "
        "forgot makes the game's teardown fail",
        SOUND,
        "            handle = created\n"
        "            rt.register(self)",
        "            handle = created",
    ),
    (
        "sound-conversions-swap-their-arguments",
        "GetSampleSizeInBytes passing the sample rate where the duration goes, "
        "so every size it answers is for the wrong sound",
        SOUND,
        "                    SoundEffect.ticks(from: duration), sampleRate,",
        "                    Int64(sampleRate), sampleRate,",
    ),
    (
        "instance-volume-accepts-anything",
        "SetVolume without its range check, which CNA passes through unclamped "
        "-- so an illegal volume becomes a real one",
        SOUND_INSTANCE,
        "            guard value >= 0, value <= 1 else {\n"
        "                throw CNAArgumentOutOfRangeException(paramName: \"value\")\n"
        "            }\n"
        "            let live = try validatedHandle(\"SoundEffectInstance.SetVolume\")",
        "            let live = try validatedHandle(\"SoundEffectInstance.SetVolume\")",
    ),
    (
        "instance-pitch-is-clamped-not-refused",
        "SetPitch without its range check, so CNA clamps an illegal pitch to "
        "the nearest legal one and the caller is never told",
        SOUND_INSTANCE,
        "            guard value >= -1, value <= 1 else {\n"
        "                throw CNAArgumentOutOfRangeException(paramName: \"value\")\n"
        "            }\n"
        "            let live = try validatedHandle(\"SoundEffectInstance.SetPitch\")",
        "            let live = try validatedHandle(\"SoundEffectInstance.SetPitch\")",
    ),
    (
        "instance-stop-is-never-immediate",
        "the parameterless Stop asking for a fade instead of an immediate stop",
        SOUND_INSTANCE,
        "        public func Stop() throws {\n"
        "            try Stop(true)\n"
        "        }",
        "        public func Stop() throws {\n"
        "            try Stop(false)\n"
        "        }",
    ),
    (
        "instance-apply3d-array-accepts-null",
        "the array Apply3D letting a null array through, so a caller's mistake "
        "reaches the ABI instead of being named",
        SOUND_INSTANCE,
        "            guard let listeners else {\n"
        "                throw CNAArgumentNullException(paramName: \"listeners\")\n"
        "            }",
        "            let listeners = listeners ?? []",
    ),
    # ---- Foundation 103: Model ---------------------------------------------
    (
        "model-mesh-draw-skips-the-effect-check",
        "ModelMesh.Draw forwarding to the runtime without walking its parts, "
        "so a part with no effect reaches the draw call instead of the "
        "InvalidOperationException XNA raises",
        MODEL_MESH,
        """            for index in 0..<parts.Count where
                (try? parts.Items.Item(index))?.Effect == nil {""",
        """            for index in 0..<parts.Count where
                index < 0 {""",
    ),
    (
        "model-mesh-no-effect-message-truncated",
        "the null-effect refusal carrying a message that is not the "
        "assembly's, which is how a caller loses the half of it that says what "
        "to do instead",
        MODEL_SUPPORT,
        '    static let modelHasNoEffect = "ModelMeshPart has a null Effect."',
        '    static let modelHasNoEffect = "ModelMeshPart has a null Effect"',
    ),
    (
        "model-bone-transform-not-pushed",
        "the bone storing the transform a caller set without sending it, so "
        "the projection and the runtime disagree about where the bone is",
        MODEL_BONE,
        """                let result = runtime.functions.modelBoneSetTransform(
                    nativeHandle, ModelSupport.native(newValue))""",
        """                let result: UInt32 = 0
                _ = ModelSupport.native(newValue)""",
    ),
    (
        "model-bone-index-answers-zero",
        "Index reporting zero for every bone, so a skeleton's bones all claim "
        "the root's slot",
        MODEL_BONE,
        "            guard runtime.functions.modelBoneGetIndex(nativeHandle, &value) == 0 else {",
        "            guard false, runtime.functions.modelBoneGetIndex(nativeHandle, &value) == 0 else {",
    ),
    (
        "model-bone-parent-ignores-the-availability",
        "Parent building a bone from the handle without testing out_has_parent, "
        "so the root reports a parent that is not there",
        MODEL_BONE,
        """            guard runtime.functions.modelBoneGetParent(nativeHandle, &has, &produced) == 0,
                  has != 0 else { return nil }""",
        """            guard runtime.functions.modelBoneGetParent(nativeHandle, &has, &produced) == 0,
                  has != 1 || true else { return nil }""",
    ),
    (
        "model-copy-transforms-uses-its-own-capacity",
        "CopyBoneTransformsTo telling the runtime how big its scratch buffer "
        "is rather than how big the caller's array is, so a short array is "
        "overrun instead of refused",
        MODEL,
        "                            UInt64(destination.count), &written)\n                        : runtime.functions.modelCopyBoneTransforms(",
        "                            UInt64(native.count), &written)\n                        : runtime.functions.modelCopyBoneTransforms(",
    ),
    (
        "model-tag-not-kept",
        "Tag dropping what a caller set, so the one property the projection "
        "holds on the Swift side stops holding it",
        MODEL,
        "            set { storedTag = newValue }",
        "            set { _ = newValue }",
    ),
    (
        "model-mesh-part-start-index-reads-the-offset",
        "StartIndex answering from the vertex-offset route, two properties "
        "that are both small integers and mean different things",
        MODEL_PART,
        """            self.runtime.functions.modelMeshPartGetStartIndex(
                self.nativeHandle, $0) } }""",
        """            self.runtime.functions.modelMeshPartGetVertexOffset(
                self.nativeHandle, $0) } }""",
    ),
    (
        "model-mesh-part-effect-not-pushed",
        "the part accepting an Effect and sending a null handle instead of it, "
        "so the mesh draws with whatever it had before. The FIRST version of "
        "this mutation was a NO-OP replaced rather than scored: it changed "
        "`let result = call(...)` into `let result: UInt32 = 0; _ = call(...)`, "
        "which still made the call and only discarded its answer. The effect "
        "was pushed either way and the mutation could not have been caught by "
        "any test",
        MODEL_PART,
        """                    nativeHandle, (try? newValue?.validatedHandle(
                        "cna_model_mesh_part_set_effect")) ?? 0)""",
        """                    nativeHandle, 0)
                _ = newValue""",
    ),
    # WITHDRAWN: "model-collection-enumerator-starts-at-zero" -- an enumerator
    # that starts ON the first element instead of before it is a real defect
    # and it cannot be falsified here, because no NON-EMPTY model collection
    # can be built. `cna_model_create` and `cna_model_bone_add_child` are the
    # two routes that would fill one, and neither is bound: XNA's Model has no
    # public constructor and its bone children are read-only, so under the
    # Foundation 67 rule they have no consuming member. Every collection this
    # environment can make is empty, and on an empty one a cursor at -1 and a
    # cursor at 0 both answer false to the first MoveNext.
    #
    # Reinstate it the day `ContentManager.Load<Model>` is wired and a loaded
    # model brings real bones with it.
    (
        "model-collection-try-get-value-always-succeeds",
        "TryGetValue answering true for a name it did not find, leaving the "
        "caller's slot holding whatever it held before",
        MODEL_BONES,
        "            guard let found = lookup(boneName) else { return false }",
        "            guard let found = lookup(boneName) else { return true }",
    ),
    # ---- Foundation 102: Storage ------------------------------------------
    (
        "storage-result-reports-it-is-still-running",
        "an IAsyncResult from a route that has already finished reporting "
        "IsCompleted false, so a caller polls for a completion that happened "
        "before Begin returned",
        STORAGE_ASYNC,
        "    var IsCompleted: Bool { true }",
        "    var IsCompleted: Bool { false }",
    ),
    (
        "storage-result-claims-asynchrony-it-does-not-have",
        "CompletedSynchronously answering false, which tells a caller the work "
        "happened on another thread when CNA ran it on this one",
        STORAGE_ASYNC,
        "    var CompletedSynchronously: Bool { true }",
        "    var CompletedSynchronously: Bool { false }",
    ),
    (
        "storage-begin-never-calls-its-callback",
        "Begin returning without invoking the callback it was given, so a "
        "caller that only reacts through the callback never learns the work is "
        "done",
        STORAGE_DEVICE,
        "            callback(result)",
        "            _ = callback",
    ),
    (
        "storage-end-accepts-a-result-from-the-other-begin",
        "EndShowSelector reading a container handle as a device, which the "
        "kind tag exists to refuse",
        STORAGE_DEVICE,
        """            guard let produced = result as? StorageAsyncResult,
                  produced.kind == .device else {""",
        """            guard let produced = result as? StorageAsyncResult,
                  produced.kind != .container || true else {""",
    ),
    (
        "storage-free-space-answers-the-total",
        "FreeSpace reading the total-space route, so a nearly full device "
        "reports itself empty",
        STORAGE_DEVICE,
        """                    functions.storageDeviceGetFreeSpace(live, &value),
                    operation: "cna_storage_device_get_free_space")""",
        """                    functions.storageDeviceGetTotalSpace(live, &value),
                    operation: "cna_storage_device_get_free_space")""",
    ),
    (
        "storage-container-dispose-does-not-mark-itself",
        "Dispose releasing the native container without setting the flag, so "
        "IsDisposed keeps answering false and every later call is made against "
        "a disposed handle",
        STORAGE_CONTAINER,
        "            disposed = true",
        "            _ = disposed",
    ),
    (
        "storage-container-search-pattern-ignored",
        "GetFileNames(pattern) counting with the caller's pattern and copying "
        "with a wildcard, so the names come back from a different set than the "
        "count measured",
        STORAGE_CONTAINER,
        "        private func names(_ pattern: String, directories: Bool) throws -> [String] {\n            let live = try validated()\n            var utf8 = Array(pattern.utf8)",
        "        private func names(_ pattern: String, directories: Bool) throws -> [String] {\n            let live = try validated()\n            var utf8 = Array(\"*\".utf8)",
    ),
    (
        "storage-container-handle-not-released",
        "a container dropped without Dispose leaving its native handle alive -- "
        "the defect Foundation 101 found in ContentManager, planted in the type "
        "written straight after it",
        STORAGE_CONTAINER,
        "            _ = functions.storageContainerDestroy(handle)",
        "            _ = handle",
    ),
    # ---- Foundation 101: the one owned type without a release --------------
    (
        "dropped-content-manager-leaks-its-handle",
        "a ContentManager deallocated without Dispose leaving its native "
        "handle alive, which makes cna_game_destroy refuse the whole game -- "
        "the defect the template's canary failed on every run",
        CONTENT,
        "            if runtime.functions.contentManagerDestroy(handle) == 0 { handle = 0 }",
        "            _ = handle",
    ),
    # ---- Foundation 88: messages the coverage gate named --------------------
    (
        "content-root-moves-after-a-load",
        "SetRootDirectory accepting a change once assets are cached, leaving "
        "every cached asset resolved against a path that no longer applies",
        CONTENT,
        "            if !(loadedAssets?.isEmpty ?? true) {",
        "            if false {",
    ),
    (
        "device-presents-over-a-bound-render-target",
        "Present accepted while a render target is bound, so a caller presents "
        "the backbuffer they were not drawing into",
        DEVICE,
        "            guard runtime.cachedRenderTargetBindings.isEmpty else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: GraphicsDevice.cannotPresentActiveRenderTargetsMessage)",
        "            if false {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: GraphicsDevice.cannotPresentActiveRenderTargetsMessage)",
    ),
    # ---- Foundation 88: the occlusion query --------------------------------
    (
        "occlusion-second-begin-accepted",
        "Begin accepted while a result is still unread, so a caller silently "
        "discards the query they were waiting on",
        OCCLUSION,
        "            guard !awaitingCompletionCheck else {",
        "            if false {",
    ),
    (
        "occlusion-check-does-not-rearm",
        "reading IsComplete leaving the query armed, so every later Begin is "
        "refused and the type can be used exactly once",
        OCCLUSION,
        "            awaitingCompletionCheck = false\n"
        "            return value != 0",
        "            return value != 0",
    ),
    (
        "occlusion-status-failure-hangs-the-wait",
        "a failed status read answering false, which is what turns a caller's "
        "wait loop into a hang with nothing to report",
        OCCLUSION,
        "            } catch {\n"
        "                lastStatusFailure = error\n"
        "                return true\n"
        "            }",
        "            } catch {\n"
        "                lastStatusFailure = error\n"
        "                return false\n"
        "            }",
    ),
    (
        "occlusion-status-swallows-the-failure",
        "a failed status read leaving no trace, so the divergence is described "
        "in a comment and provable nowhere",
        OCCLUSION,
        "            } catch {\n"
        "                lastStatusFailure = error\n"
        "                return true",
        "            } catch {\n"
        "                lastStatusFailure = nil\n"
        "                return true",
    ),
    (
        "occlusion-never-joins-the-parent-registry",
        "a query that does not register with the runtime, so an undisposed one "
        "makes the game's own teardown fail",
        OCCLUSION,
        "            runtime.register(self)",
        "            _ = runtime",
    ),
    # WITHDRAWN: "occlusion-pixel-count-cannot-fail" -- dropping the route's
    # own result check. It cannot be falsified on this host and the reason is
    # structural, not a missing test. Reaching that check requires a query whose
    # handle is valid AND whose IsComplete is true AND whose count route then
    # fails; the disposed path throws at validatedHandle first, and the
    # incomplete path is caught by the managed guard below it. CNA answers
    # success for every completed query this renderer can produce, so the third
    # condition has no input. It is replaced by the mutation on the guard that
    # IS reachable.
    (
        "occlusion-count-read-before-completion",
        "PixelCount answering a count for a query that never finished, which "
        "is the refusal XNA raises and CNA does not",
        OCCLUSION,
        "                guard IsComplete else {\n"
        "                    throw CNAInvalidOperationException(\n"
        "                        message: OcclusionQuery.dataNotAvailableMessage)\n"
        "                }",
        "                if false {\n"
        "                    throw CNAInvalidOperationException(\n"
        "                        message: OcclusionQuery.dataNotAvailableMessage)\n"
        "                }",
    ),
    (
        "occlusion-end-does-not-submit",
        "End accepted without reaching the runtime, so a query is waited on "
        "that was never submitted",
        OCCLUSION,
        "                nativeStorage.runtime.functions.occlusionQueryEnd(handle),\n"
        "                operation: \"cna_occlusion_query_end\")",
        "                nativeStorage.runtime.functions.occlusionQueryBegin(handle),\n"
        "                operation: \"cna_occlusion_query_end\")",
    ),
    # ---- Foundation 87: the content manager --------------------------------
    (
        "content-tests-the-name-before-disposal",
        "Load testing the asset name before the disposal it must report "
        "first, so a disposed manager reports a null name instead",
        CONTENT,
        "            guard var assets = loadedAssets else {\n"
        "                throw CNAObjectDisposedException(objectName: \"\\(type(of: self))\")\n"
        "            }\n"
        "            guard let assetName, !assetName.isEmpty else {",
        "            guard let assetName, !assetName.isEmpty else {\n"
        "                throw CNAArgumentNullException(paramName: \"assetName\")\n"
        "            }\n"
        "            guard var assets = loadedAssets else {",
    ),
    # `content-accepts-the-empty-asset-name` was planted and SURVIVED as a
    # NO-OP: every cache miss immediately reaches ReadAsset, which repeats the
    # empty-name refusal, while the public API cannot create an empty cache key.
    # It is replaced rather than scored by
    # `content-empty-name-reports-the-wrong-parameter`, which changes the
    # observable exception from the same public branch.
    (
        "content-empty-name-reports-the-wrong-parameter",
        "Load's null-or-empty branch naming a different parameter, so callers "
        "cannot identify the argument XNA rejected",
        CONTENT,
        "            guard let assetName, !assetName.isEmpty else {\n"
        "                throw CNAArgumentNullException(paramName: \"assetName\")\n"
        "            }",
        "            guard let assetName, !assetName.isEmpty else {\n"
        "                throw CNAArgumentNullException(paramName: \"name\")\n"
        "            }",
    ),
    (
        "content-unload-disposes-the-manager",
        "Unload nulling the cache the way Dispose does, so a manager that "
        "should still be usable reports itself disposed",
        CONTENT,
        "            defer {\n"
        "                loadedAssets = [:]\n"
        "                disposableAssets = []\n"
        "                openedStreamLengths.removeAll(keepingCapacity: false)\n"
        "            }",
        "            defer {\n"
        "                loadedAssets = nil\n"
        "                disposableAssets = []\n"
        "                openedStreamLengths.removeAll(keepingCapacity: false)\n"
        "            }",
    ),
    (
        "content-dispose-leaves-the-manager-usable",
        "Dispose releasing the handle without nulling the cache, so the "
        "manager keeps answering Load with a destroyed handle",
        CONTENT,
        "            guard loadedAssets != nil else { return }\n"
        "            if disposing { try Unload() }\n"
        "            loadedAssets = nil",
        "            guard loadedAssets != nil else { return }\n"
        "            if disposing { try Unload() }\n"
        "            loadedAssets = [:]",
    ),
    (
        "content-never-joins-the-parent-registry",
        "the manager not registering with the runtime, so a caller who never "
        "disposes one it still HOLDS leaves a live handle for the game "
        "teardown to trip over. Half the story, and Foundation 101 paid for "
        "the other half: the registry is weak, so it covers a manager that is "
        "alive and undisposed and does nothing for one already deallocated. "
        "That case is `dropped-content-manager-leaks-its-handle`",
        CONTENT,
        "            rt.register(self)",
        "            _ = rt",
    ),
    (
        "content-declares-the-structure-one-field-short",
        "the create call declaring the size the mirror had before its "
        "reserved field was added, which CNA refuses outright",
        CONTENT,
        "                        info.struct_size = UInt32(\n"
        "                            MemoryLayout<CNASwift_ContentManagerCreateInfo>.size)",
        "                        info.struct_size = UInt32(\n"
        "                            MemoryLayout<CNASwift_ContentManagerCreateInfo>.size - 8)",
    ),
    (
        "content-refuses-the-kind-it-can-load",
        "the type switch falling through for Texture2D as well, so the one "
        "wired loader is reported as having no route",
        CONTENT,
        "            case is Microsoft.Xna.Framework.Graphics.Texture2D.Type:",
        "            case is Microsoft.Xna.Framework.Graphics.SpriteFont.Type:",
    ),
    (
        "game-content-rebuilt-on-every-read",
        "Game.Content ignoring its cache, so each read builds another manager "
        "and a caller's installed one is silently replaced",
        GAME,
        "            if let cachedContent, cachedContentGeneration == currentGeneration {\n"
        "                return cachedContent\n"
        "            }",
        "            if false, let cachedContent { return cachedContent }",
    ),
    # ---- Foundation 86: the device ownership rule --------------------------
    (
        "own-device-disposed-through-the-refusing-route",
        "a caller-created device released through the route that only accepts "
        "a game's borrowed handle, so releasing it fails",
        DEVICE,
        "                runtime.functions.graphicsDeviceDestroy(handle),\n"
        "                operation: \"cna_graphics_device_destroy\")",
        "                runtime.functions.graphicsDeviceDispose(handle),\n"
        "                operation: \"cna_graphics_device_destroy\")",
    ),
    (
        "own-device-never-reports-itself-disposed",
        "a caller-created device answering the game device's staleness test, "
        "so it never reports disposed",
        DEVICE,
        "            callerCreated\n"
        "                ? callerCreatedDisposed\n"
        "                : runtime.generation != generation",
        "            runtime.generation != generation",
    ),
    # ---- Foundation 85: Present's refused arguments ------------------------
    (
        "present-widens-a-sub-rectangle",
        "the three-argument Present silently presenting the whole surface "
        "where a caller asked for part of it",
        DEVICE,
        "            guard sourceRectangle == nil, destinationRectangle == nil,\n"
        "                  overrideWindowHandle == 0 else {",
        "            guard true else {",
    ),
    # ---- Foundation 84: device disposal and its events ---------------------
    (
        "dispose-swallows-the-refusal",
        "Dispose reporting success where CNA refuses, so a caller believes the "
        "device was released while the game still draws into it",
        DEVICE,
        "                    runtime.functions.graphicsDeviceDispose(handle),\n"
        "                    operation: \"cna_graphics_device_dispose\")",
        "                    0,\n"
        "                    operation: \"cna_graphics_device_dispose\")",
    ),
    (
        "is-disposed-answers-yes-while-live",
        "a live facade reporting itself disposed, which would make every "
        "guarded member look unusable",
        DEVICE,
        "                : runtime.generation != generation",
        "                : runtime.generation == generation",
    ),
    # ---- Foundation 83: FindBestDevice -------------------------------------
    # WITHDRAWN: "find-best-restores-the-multisampling-flag" -- undoing the
    # destructive retry.
    #
    # The retry only runs when the FIRST pass finds nothing, and on this host it
    # never does: one adapter, supporting both profiles, always yields a
    # candidate. Restoring a flag on a path that is never taken changes no
    # answer. The behaviour is real and is documented on the member -- XNA
    # leaves PreferMultiSampling off after turning it off -- but the claim that
    # a test here observes it is withdrawn.
    (
        "add-devices-keeps-duplicates",
        "the Contains test dropped, so an adapter's current mode is offered "
        "twice when it is also a supported mode",
        MANAGER,
        "            for existing in 0 ..< foundDevices.Count\n"
        "            where try foundDevices.Item(existing).Equals(info) {\n"
        "                return\n"
        "            }",
        "            for existing in 0 ..< foundDevices.Count\n"
        "            where try foundDevices.Item(existing).Equals(info) && false {\n"
        "                return\n"
        "            }",
    ),
    # WITHDRAWN: "add-devices-drops-the-minimum-mode-size" -- removing the
    # 640x480 floor on full-screen modes.
    #
    # Measured: this host reports exactly one supported display mode, 800x480,
    # which clears the floor either way. With no mode below it there is nothing
    # the floor can exclude, so the mutation cannot change an answer. The floor
    # stays -- it is 0x280 by 0x1e0 in the IL -- and the claim that a test
    # reaches it does not.
    # ---- Foundation 82: GameWindow -----------------------------------------
    # WITHDRAWN: "window-title-skips-the-change-test" -- removing the
    # `storedTitle != value` guard so an unchanged title is pushed again.
    #
    # It survived, and the reason is structural rather than a missing test.
    # The guard's only effect is how many times `cna_game_set_window_title` is
    # called, and nothing on this host can count that: the Swift member IS the
    # overridable hook (the setter and XNA's protected SetTitle collapse into
    # one identity here), so an override sees both calls either way and there
    # is no second observation point behind it.
    #
    # The guard stays -- it is XNA's, in fifty-three bytes that test before
    # storing -- but the claim that a test can observe it is withdrawn rather
    # than dressed up in a mutation that passes for the wrong reason.
    (
        "window-title-accepts-nil",
        "a nil title stored instead of refused",
        WINDOW,
        "            guard let value else {\n"
        "                throw CNAArgumentNullException(",
        "            guard let value = value ?? \"\" as String? else {\n"
        "                throw CNAArgumentNullException(",
    ),
    (
        "window-facade-rebuilt-every-read",
        "Game.Window building a new facade per read, so the window a caller "
        "holds stops being the game's",
        GAME,
        "            if let cachedWindow, cachedWindowGeneration == currentGeneration {",
        "            if false, cachedWindowGeneration == currentGeneration {",
    ),
    # ---- Foundation 81: the device ranking ---------------------------------
    (
        "ranking-prefers-the-lower-profile",
        "the profile link ordering ascending, so Reach outranks HiDef",
        RANKING,
        "                return d1.GraphicsProfile.rawValue > d2.GraphicsProfile.rawValue ? -1 : 1",
        "                return d1.GraphicsProfile.rawValue < d2.GraphicsProfile.rawValue ? -1 : 1",
    ),
    (
        "ranking-full-screen-asks-the-candidate",
        "the full-screen link comparing the two candidates instead of asking "
        "the manager which it wants",
        RANKING,
        "                return graphics.IsFullScreen == p1.IsFullScreen ? -1 : 1",
        "                return p1.IsFullScreen ? -1 : 1",
    ),
    (
        "ranking-format-sorts-descending",
        "the format rank ordering descending, which puts Int32.max first",
        RANKING,
        "            if r1 != r2 { return r1 < r2 ? -1 : 1 }",
        "            if r1 != r2 { return r1 > r2 ? -1 : 1 }",
    ),
    (
        "ranking-multisample-prefers-fewer",
        "the multi-sample link preferring the lower count",
        RANKING,
        "                return p1.MultiSampleCount > p2.MultiSampleCount ? -1 : 1",
        "                return p1.MultiSampleCount < p2.MultiSampleCount ? -1 : 1",
    ),
    (
        "ranking-aspect-tolerance-dropped",
        "the 0.2 tolerance removed, so nearly-identical shapes decide the "
        "order instead of falling through to pixel count",
        RANKING,
        "            if abs(off1 - off2) > 0.2 { return off1 < off2 ? -1 : 1 }",
        "            if off1 != off2 { return off1 < off2 ? -1 : 1 }",
    ),
    (
        "bit-depth-table-widened",
        "a format XNA ranks zero given a real bit depth",
        RANKING,
        "            case 1, 2, 3: return 16",
        "            case 1, 2, 3, 4: return 16",
    ),
    # ---- Foundation 80: the manager's reset rule and settings event --------
    (
        "can-reset-device-also-compares-the-back-buffer",
        "CanResetDevice widened past the single profile comparison XNA's "
        "twenty-three bytes perform",
        MANAGER,
        "            return device.GraphicsProfile == newDeviceInfo.GraphicsProfile",
        "            return device.GraphicsProfile == newDeviceInfo.GraphicsProfile\n"
        "                && device.PresentationParameters?.BackBufferWidth\n"
        "                    == newDeviceInfo.PresentationParameters.BackBufferWidth",
    ),
    (
        "preparing-settings-raises-with-the-manager-as-sender",
        "the settings raiser substituting itself for the caller's sender, "
        "which is what Game's raisers do and this one does not",
        MANAGER,
        "            try preparingDeviceSettingsSource.Raise(sender, args: args)",
        "            try preparingDeviceSettingsSource.Raise(self, args: args)",
    ),
    # ---- Foundation 79: the two managed device carriers --------------------
    (
        "device-information-equals-compares-the-objects",
        "Equals comparing the two PresentationParameters objects instead of "
        "the ten properties XNA reads through them",
        DEVICEINFO,
        "            let mine = storedParameters\n            let theirs = other.storedParameters",
        "            guard other.storedParameters === storedParameters else { return false }\n"
        "            let mine = storedParameters\n            let theirs = other.storedParameters",
    ),
    (
        "device-information-clone-shares-the-parameters",
        "Clone copying the parameters by reference, so a change to the copy "
        "reaches the original",
        DEVICEINFO,
        "            copy.storedParameters = storedParameters.Clone()",
        "            copy.storedParameters = storedParameters",
    ),
    (
        "device-information-hash-drops-a-property",
        "one of the ten properties left out of the XOR",
        DEVICEINFO,
        "            hash ^= Int32(truncatingIfNeeded: p.BackBufferHeight.hashValue)",
        "",
    ),
    (
        "event-args-copy-the-information",
        "the event payload handing back a copy, so a handler's change never "
        "reaches the manager",
        DEVICEINFO,
        "            stored\n        }",
        "            stored.Clone()\n        }",
    ),
    # ---- Foundation 78: GraphicsDevice presentation ------------------------
    (
        "reset-caches-parameters-before-the-device-takes-them",
        "the reported parameters replaced before the reset is accepted, so a "
        "refused reset reports values the device does not have",
        DEVICE,
        "                operation: \"cna_graphics_device_reset_with_parameters\")\n"
        "            runtime.cachedPresentationParameters = presentationParameters\n"
        "        }\n\n        /// `GraphicsDevice.Reset(PresentationParameters, GraphicsAdapter)`.",
        "                operation: \"cna_graphics_device_reset_with_parameters\")\n"
        "        }\n\n        /// `GraphicsDevice.Reset(PresentationParameters, GraphicsAdapter)`.",
    ),
    (
        "device-adapter-read-fresh-every-time",
        "the adapter looked up per read instead of recorded once, so the "
        "device stops reporting a stable identity",
        DEVICE,
        "            runtime.cachedAdapter\n        }",
        "            Microsoft.Xna.Framework.Graphics.GraphicsAdapter\n"
        "                .Adapters.flatMap { try? $0.Item(0) }\n        }",
    ),
    (
        "reset-with-adapter-drops-the-index",
        "the three-argument Reset passing null where the adapter's index "
        "belongs, which is what a first draft did",
        DEVICE,
        "                    handle, &native, &index),",
        "                    handle, &native, nil),",
    ),
    # ---- Foundation 77: GraphicsAdapter ------------------------------------
    (
        "adapter-list-refilled-every-borrow",
        "the generation guard dropped, so every device facade re-enumerates "
        "and the adapters a caller holds stop being the ones the list has",
        ADAPTER,
        "            guard populatedGeneration != runtime.generation else { return }",
        "            if false { return }",
    ),
    # WITHDRAWN: "adapter-profile-support-answers-yes-by-default" -- flipping
    # the missing-key default in IsProfileSupported.
    #
    # It survived because the branch cannot be reached: XNA's GraphicsProfile
    # has exactly two members and the enumeration asks the route about both, so
    # every key is present and the `??` never fires. The default is there for
    # the shape of a dictionary lookup, not for a case any caller can produce.
    # A mutation that cannot change an answer is withdrawn rather than scored.
    (
        "adapter-query-drops-the-multisample-result",
        "the negotiated multi-sample count not written back through its inout "
        "parameter",
        ADAPTER,
        "            selectedMultiSampleCount = selection.multi_sample_count",
        "            _ = selection.multi_sample_count",
    ),
    # ---- Foundation 76: SpriteBatch.Begin with an effect -------------------
    (
        "begin-effect-skips-the-pair-check",
        "the effect overloads reaching the device without the inBeginEndPair "
        "test XNA does first",
        BATCH,
        "            guard !inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(",
        "            guard true else {\n"
        "                throw CNAInvalidOperationException(",
    ),
    (
        "begin-six-argument-invents-a-matrix",
        "the six-argument overload passing something other than the "
        "Matrix.Identity XNA pushes",
        BATCH,
        "                          rasterizerState, effect, .Identity)\n"
        "        }\n\n        /// `Begin(..., Effect, Matrix)`",
        "                          rasterizerState, effect, Microsoft.Xna.Framework.Matrix())\n"
        "        }\n\n        /// `Begin(..., Effect, Matrix)`",
    ),
    # ---- Foundation 74: Input.Mouse --------------------------------------
    (
        "mouse-middle-and-right-transposed",
        "the middle and right button bits exchanged, which reports a "
        "right-click as a middle-click",
        MOUSE,
        "                button(pressed, Mouse.buttonMiddle),\n"
        "                button(pressed, Mouse.buttonRight),",
        "                button(pressed, Mouse.buttonRight),\n"
        "                button(pressed, Mouse.buttonMiddle),",
    ),
    # WITHDRAWN: "mouse-window-handle-not-zero-initialised" -- poisoning the
    # handle buffer to prove the zero-initialisation load-bearing.
    #
    # It survived, twice, and measuring rather than re-guessing is what settled
    # it. `build-probe/f74_mouse.c` seeded the buffer with 123 and read 123 back
    # from a successful `cna_mouse_get_window_handle` with no window bound --
    # the route had not written, though its own header promises "zero when none
    # is bound". `build-probe/f74_when.c` then called the same route FIRST, in
    # both `load_content` and `update`, and it wrote zero every time. The only
    # difference is whether `cna_mouse_get_state` ran before it.
    #
    # So the no-write case is real but this host cannot say when it happens.
    #
    # It stopped mattering anyway, and for a better reason than the measurement:
    # the api-compat gate then showed that XNA's `get_WindowHandle` is
    # `ldsfld hHookedHandle; ret` -- infallible, reading its own static field
    # and never asking the platform. The projection reads a stored value now
    # and `cna_mouse_get_window_handle` has no consuming member at all, so it
    # is not bound. The buffer this mutation poisoned no longer exists.
    (
        "mouse-position-y-carries-x",
        "SetPosition passing x for both coordinates",
        MOUSE,
        "runtime.functions.mouseSetPosition(runtime.gameHandle, x, y)",
        "runtime.functions.mouseSetPosition(runtime.gameHandle, x, x)",
    ),
    (
        "mouse-state-reads-the-horizontal-wheel",
        "the horizontal wheel reported as XNA's single ScrollWheelValue, a "
        "number XNA never produces",
        MOUSE,
        "                scrollWheel,",
        "                horizontalScrollWheel,",
    ),
    # ---- Foundation 73: TitleContainer -----------------------------------
    (
        "clean-path-strips-leading-dot-once",
        "the .\\ loop running once instead of to exhaustion, so \".\\.\\a\" "
        "keeps a segment XNA removes",
        TITLE,
        'while path.hasPrefix(".\\\\") { path.removeFirst(2) }',
        'if path.hasPrefix(".\\\\") { path.removeFirst(2) }',
    ),
    (
        "clean-path-trailing-dot-loses-its-short-arm",
        "a path that IS \"\\.\" becoming empty instead of a lone separator",
        TITLE,
        'if path.count > 2 { path.removeLast(2) } else { path = "\\\\" }',
        'if path.count > 2 { path.removeLast(2) } else { path = "" }',
    ),
    (
        "collapse-parent-can-answer-zero",
        "CollapseParentDirectory losing its floor of 1, which is what stops "
        "GetCleanPath rescanning from the beginning",
        TITLE,
        "return max(start - 1, 1)",
        "return start - 1",
    ),
    (
        "absolute-test-forgets-the-bad-characters",
        "a name carrying one of the seven refused characters being accepted",
        TITLE,
        "if path.contains(where: { badCharacters.contains($0) }) { return true }",
        "if false { return true }",
    ),
    (
        "absolute-test-forgets-the-embedded-escape",
        "a\\..\\b being accepted, which escapes the title directory",
        TITLE,
        'if path.contains("\\\\..\\\\") { return true }',
        'if false { return true }',
    ),
    (
        "empty-name-not-refused",
        "an empty name reaching the route instead of ArgumentNullException",
        TITLE,
        "guard let name, !name.isEmpty else {",
        "guard let name, !name.isEmpty || true else {",
    ),
    (
        "missing-asset-reported-as-cannot-open",
        "CNA_RESULT_IO mapped to the wrong one of XNA's two branches",
        TITLE,
        "if result == TitleContainer.resultIO {",
        "if result != TitleContainer.resultIO {",
    ),
    (
        "read-ignores-the-reported-size",
        "the second read told a capacity the file does not fit in",
        TITLE,
        "runtime.gameHandle, view, buffer.baseAddress, byteCount, &byteCount)",
        "runtime.gameHandle, view, buffer.baseAddress, 0, &byteCount)",
    ),
    # ---- Foundation 72: FrameworkDispatcher ------------------------------
    #
    # Aimed at the handle, not at the pump. Whether the pump did any WORK is
    # not observable from here -- no song plays, no microphone is opened and no
    # storage device changes on this host -- so a mutation that skipped the
    # route entirely would survive, and one was replaced rather than scored for
    # exactly that reason. What IS observable is that the route was given the
    # runtime's real game handle: CNA validates it, so a wrong one turns an
    # accepted pump into a refused one.
    (
        "dispatcher-handle-not-carried",
        "the dispatcher pumping against a handle the runtime does not own",
        DISPATCHER,
        "runtime.functions.frameworkDispatcherUpdate(runtime.gameHandle)",
        "runtime.functions.frameworkDispatcherUpdate(0)",
    ),
    # ---- Foundation 63: vertex and index buffer binding ------------------
    (
        "binding-cache-not-updated",
        "the bound bindings not recorded, so GetVertexBuffers answers nothing",
        DEVICE,
        "            runtime.cachedVertexBufferBindings = bindings",
        "            runtime.cachedVertexBufferBindings = []",
    ),
    (
        "binding-identity-lost",
        "GetVertexBuffers rebuilding its answer instead of returning what was bound",
        DEVICE,
        "            runtime.cachedVertexBufferBindings\n        }\n\n        /// `GraphicsDevice.Indices`.",
        "            []\n        }\n\n        /// `GraphicsDevice.Indices`.",
    ),
    (
        "device-identity-compared-by-facade",
        "the InvalidDevice test comparing facades, which differ between callbacks",
        DEVICE,
        "                guard binding.VertexBuffer.nativeStorage.runtime === runtime else {",
        "                guard binding.VertexBuffer.GraphicsDevice === self else {",
    ),
    (
        "vertex-stream-limit-not-checked",
        "more simultaneous streams accepted than the profile allows",
        DEVICE,
        "            guard bindings.count <= Int(capabilities.maxVertexStreams) else {",
        "            guard bindings.count <= Int(capabilities.maxVertexStreams) * 4 else {",
    ),
    (
        "indices-cache-not-updated",
        "the bound index buffer not recorded, so Indices answers nil",
        DEVICE,
        "            runtime.cachedIndexBuffer = value",
        "            runtime.cachedIndexBuffer = nil",
    ),
    (
        "indices-setter-skips-the-disposal-check",
        "a disposed index buffer reaching the device",
        DEVICE,
        '            let bufferHandle = try value?.validatedHandle("GraphicsDevice.Indices") ?? 0',
        '            let bufferHandle = value?.nativeStorage.handle ?? 0',
    ),
    # `vertex-offset-not-carried` -- dropping the binding's vertex offset on the
    # way to CNA -- was planted here and SURVIVED, and it is withdrawn rather
    # than kept as a mutation nothing can catch. Nothing in the current public
    # surface observes the offset the DEVICE received: `GetVertexBuffers()`
    # reads the managed cache, as XNA's does, and the only thing that would
    # notice a dropped offset is a draw. `cna_graphics_device_copy_vertex_buffers`
    # would answer, but binding a route for a test alone is what
    # docs/native-abi.md forbids. The claim becomes falsifiable when the draw
    # family lands, and it is recorded as not-yet-evidence until then.

    # ---- Foundation 62: the profile and its capability table -------------
    (
        "profile-limit-read-from-the-wrong-profile",
        "the capability table answering HiDef's limits for a Reach device",
        PROFILECAPS,
        "            profile == .Reach ? reach : hidef",
        "            profile == .Reach ? hidef : reach",
    ),
    (
        "max-texture-size-off-by-a-power",
        "a single extracted limit changed, which the pinned table must refuse",
        PROFILECAPS,
        "            maxTextureSize: 2048,",
        "            maxTextureSize: 4096,",
    ),
    (
        "thirty-two-bit-indices-allowed-on-reach",
        "the IndexElementSize32 refusal dropped, so Reach accepts what XNA refuses",
        PROFILECAPS,
        "            if elementSizeInBytes == 4, !indexElementSize32 {",
        "            if false, !indexElementSize32 {",
    ),
    (
        "aspect-ratio-divides-the-wrong-way",
        "the ceiling division written as a floor, so a texture at the limit is refused",
        PROFILECAPS,
        "            guard (longer + shorter - 1) / shorter <= maxTextureAspectRatio else {\n"
        "                try throwNotSupported(\n"
        "                    ProfileCapabilities.profileAspectRatio,\n"
        "                    \"Texture2D\", \"\\(maxTextureAspectRatio)\")",
        "            guard (longer + shorter) / shorter <= maxTextureAspectRatio else {\n"
        "                try throwNotSupported(\n"
        "                    ProfileCapabilities.profileAspectRatio,\n"
        "                    \"Texture2D\", \"\\(maxTextureAspectRatio)\")",
    ),
    (
        "profile-message-omits-the-profile",
        "ThrowNotSupportedException formatting the limit without naming the profile",
        PROFILECAPS,
        '                of: "{0}", with: "\\(profile)")',
        '                of: "{0}", with: "")',
    ),
    (
        "texture-size-limit-not-checked",
        "a texture larger than the profile allows accepted",
        PROFILECAPS,
        "            guard width <= maxTextureSize, height <= maxTextureSize else {",
        "            guard width <= maxTextureSize * 4, height <= maxTextureSize * 4 else {",
    ),

    # ---- Foundation 61: the dynamic buffers ------------------------------
    (
        "static-upload-takes-the-option-route",
        "the option-taking upload used for SetDataOptions.None, which a static buffer refuses",
        VERTEXBUFFER,
        "                if options == .None {",
        "                if false {",
    ),
    (
        "index-option-rides-the-windowed-route",
        "a streaming option forwarded to the windowed index upload, which CNA refuses",
        INDEXBUFFER,
        "            let forwarded = (options != .None && wholeBuffer)",
        "            let forwarded = (options != .None || wholeBuffer)",
    ),
    (
        "dynamic-vertex-buffer-created-static",
        "DynamicVertexBuffer created with the static flag, so its option overloads are refused",
        DYNAMICVERTEX,
        "                vertexCount: vertexCount, usage: usage, dynamic: true,\n"
        '                typeName: "DynamicVertexBuffer")\n'
        "            try subscribeToNativeContentLost()\n"
        "        }\n"
        "\n"
        "        /// `DynamicVertexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.",
        "                vertexCount: vertexCount, usage: usage, dynamic: false,\n"
        '                typeName: "DynamicVertexBuffer")\n'
        "            try subscribeToNativeContentLost()\n"
        "        }\n"
        "\n"
        "        /// `DynamicVertexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.",
    ),
    (
        "content-lost-subscription-not-released",
        "disposal leaving the native ContentLost registration alive",
        DYNAMICVERTEX,
        "            guard !IsDisposed else { return }\n"
        "            unsubscribeFromNativeContentLost()\n"
        "            try super.Dispose(disposing)",
        "            guard !IsDisposed else { return }\n"
        "            try super.Dispose(disposing)",
    ),
    (
        "index-content-lost-never-subscribed",
        "a dynamic index buffer that never registers for ContentLost",
        DYNAMICINDEX,
        "                indexCount: indexCount, usage: usage, dynamic: true,\n"
        '                typeName: "DynamicIndexBuffer")\n'
        "            try subscribeToNativeContentLost()\n"
        "        }\n"
        "\n"
        "        /// `DynamicIndexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.",
        "                indexCount: indexCount, usage: usage, dynamic: true,\n"
        '                typeName: "DynamicIndexBuffer")\n'
        "        }\n"
        "\n"
        "        /// `DynamicIndexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.",
    ),
    (
        "content-lost-flag-reports-true",
        "IsContentLost answering true on a host whose renderer cannot lose a device",
        DYNAMICVERTEX,
        "        public var IsContentLost: Bool { contentLost }",
        "        public var IsContentLost: Bool { true }",
    ),

    # ---- Foundation 60: the vertex and index buffers ---------------------
    (
        "copy-parameters-name-the-callers-parameter",
        "ValidateCopyParameters naming startIndex where the helper names dataIndex",
        BUFFERSUPPORT,
        '                paramName: "dataIndex", message: BufferResources.mustBeValidIndex)',
        '                paramName: "startIndex", message: BufferResources.mustBeValidIndex)',
    ),
    (
        "copy-parameters-blame-the-count-not-the-index",
        "the window test moved ahead of the index test, so an index past the end is blamed on elementCount",
        BUFFERSUPPORT,
        "        if dataIndex < 0 || Int(dataIndex) > dataLength {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        '                paramName: "dataIndex", message: BufferResources.mustBeValidIndex)\n'
        "        }\n"
        "        if Int(elementCount) + Int(dataIndex) > dataLength {",
        "        if Int(elementCount) + Int(dataIndex) > dataLength {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        '                paramName: "elementCount", message: BufferResources.mustBeValidIndex)\n'
        "        }\n"
        "        if dataIndex < 0 || Int(dataIndex) > dataLength {",
    ),
    (
        "empty-array-not-reported-as-null",
        "a zero-length array raising ArgumentException where XNA's ldlen branch raises ArgumentNullException",
        VERTEXBUFFER,
        "            guard arrayCount > 0 else {\n"
        "                throw CNAArgumentNullException(",
        "            guard arrayCount > 0 else {\n"
        "                throw CNAArgumentException(",
    ),
    (
        "write-only-buffer-allows-getdata",
        "the WriteOnly guard dropped, so GetData reaches a buffer XNA refuses",
        VERTEXBUFFER,
        "            if !isSetting, BufferUsage.contains(.WriteOnly) {",
        "            if false, BufferUsage.contains(.WriteOnly) {",
    ),
    (
        "vertex-stride-too-small-accepted",
        "a vertexStride below sizeof(T) accepted",
        VERTEXBUFFER,
        "                guard slack >= 0 else {",
        "                guard slack >= -64 else {",
    ),
    (
        "buffer-size-comparison-off-by-one",
        "a transfer that exactly fills the buffer refused",
        VERTEXBUFFER,
        "            guard bytes + Int(offsetInBytes) <= sizeInBytes else {",
        "            guard bytes + Int(offsetInBytes) < sizeInBytes else {",
    ),
    (
        "vertex-array-window-offset-ignored",
        "startIndex dropped, so the transfer always reads from the array's front",
        VERTEXBUFFER,
        "                byteOffset: Int(startIndex) * elementSize,",
        "                byteOffset: 0,",
    ),
    (
        "vertex-buffer-offset-ignored",
        "offsetInBytes dropped, so a windowed write lands at the buffer's front",
        VERTEXBUFFER,
        "                offsetInBytes: Int(offsetInBytes),",
        "                offsetInBytes: 0,",
    ),
    (
        "index-element-width-transposed",
        "SixteenBits measured as four bytes and ThirtyTwoBits as two",
        INDEXBUFFER,
        "            size == .SixteenBits ? 2 : 4",
        "            size == .SixteenBits ? 4 : 2",
    ),
    (
        "index-type-overload-maps-the-wrong-width",
        "typeof(short) mapped to a 32-bit index buffer",
        INDEXBUFFER,
        "            case ObjectIdentifier(Int16.self), ObjectIdentifier(UInt16.self):\n"
        "                return 2",
        "            case ObjectIdentifier(Int16.self), ObjectIdentifier(UInt16.self):\n"
        "                return 4",
    ),
    (
        "index-window-offset-ignored",
        "a windowed index write landing at the buffer's front",
        INDEXBUFFER,
        "                            plan.handle, UInt64(plan.offsetInBytes), pointer,",
        "                            plan.handle, 0, pointer,",
    ),
    (
        "binding-offset-compared-signed",
        "the vertex-offset bound compared signed, where the IL's bge.un is unsigned",
        BUFFERBINDING,
        "                  UInt32(bitPattern: vertexOffset)\n"
        "                    < UInt32(bitPattern: vertexBuffer.VertexCount) else {",
        "                  vertexOffset <= vertexBuffer.VertexCount else {",
    ),
    (
        "binding-blames-the-frequency-first",
        "the instance frequency checked before the offset, so the wrong one is blamed",
        BUFFERBINDING,
        "            guard vertexOffset >= 0,\n"
        "                  UInt32(bitPattern: vertexOffset)\n"
        "                    < UInt32(bitPattern: vertexBuffer.VertexCount) else {\n"
        '                throw CNAArgumentOutOfRangeException(paramName: "vertexOffset")\n'
        "            }\n"
        "            guard instanceFrequency >= 0 else {\n"
        '                throw CNAArgumentOutOfRangeException(paramName: "instanceFrequency")\n'
        "            }",
        "            guard instanceFrequency >= 0 else {\n"
        '                throw CNAArgumentOutOfRangeException(paramName: "instanceFrequency")\n'
        "            }\n"
        "            guard vertexOffset >= 0,\n"
        "                  UInt32(bitPattern: vertexOffset)\n"
        "                    < UInt32(bitPattern: vertexBuffer.VertexCount) else {\n"
        '                throw CNAArgumentOutOfRangeException(paramName: "vertexOffset")\n'
        "            }",
    ),
    (
        "from-type-size-test-reads-the-wrong-size",
        "a registered vertex type's measured size disagreeing with its declared stride, which FromType must refuse",
        VERTEXDECL,
        "                    size: MemoryLayout<VertexPositionColor>.size),",
        "                    size: MemoryLayout<VertexPositionColor>.size + 4),",
    ),

    # ---- Foundation 59: the image members and the disposal chain ---------
    (
        "disposing-flag-ignored-on-the-finalizer-path",
        "Disposing raised from Dispose(false), which XNA raises to nobody",
        RESOURCE,
        "            guard disposing else { return }\n"
        "            try disposingSource.Raise(self, args: CNAEventArgs.Empty)",
        "            try disposingSource.Raise(self, args: CNAEventArgs.Empty)",
    ),
    (
        "alpha-zero-texels-not-rewritten",
        "SaveAsPng encodes an alpha-0 texel's colour, where XNA writes Transparent",
        TEXTURE2D,
        "            for index in colors.indices where colors[index].A == 0 {\n"
        "                colors[index] = Microsoft.Xna.Framework.Color.Transparent\n"
        "            }",
        "            for index in colors.indices where colors[index].A == 255 {\n"
        "                colors[index] = colors[index]\n"
        "            }",
    ),
    (
        "encode-dimensions-transposed",
        "the encoded width and height swapped",
        TEXTURE2D,
        "                runtime.functions.textureGetEncodedByteCount(\n"
        "                    source, format, UInt32(bitPattern: width),\n"
        "                    UInt32(bitPattern: height), &byteCount),",
        "                runtime.functions.textureGetEncodedByteCount(\n"
        "                    source, format, UInt32(bitPattern: height),\n"
        "                    UInt32(bitPattern: width), &byteCount),",
    ),
    (
        "jpeg-saved-through-the-png-format",
        "SaveAsJpeg passing the PNG image-format constant",
        TEXTURE2D,
        "            try saveAsImage(stream, format: Texture2D.nativeImageFormatJpeg,",
        "            try saveAsImage(stream, format: Texture2D.nativeImageFormatPng,",
    ),
    (
        "fromstream-zoom-ignored",
        "the zoom flag dropped, so Scale|Crop decodes as Scale",
        TEXTURE2D,
        "            decode.zoom = zoom ? 1 : 0",
        "            decode.zoom = 0",
    ),
    (
        "fromstream-decode-dimensions-transposed",
        "the requested width and height swapped on the way to the decoder",
        TEXTURE2D,
        "            decode.width = UInt32(bitPattern: width)\n"
        "            decode.height = UInt32(bitPattern: height)",
        "            decode.width = UInt32(bitPattern: height)\n"
        "            decode.height = UInt32(bitPattern: width)",
    ),
    (
        "setdata-disposal-check-on-the-runtime-channel",
        "a disposed texture reported as CNAError instead of ObjectDisposedException",
        TEXTURE2D,
        "            let handle = try validatedHandle(\"Texture2D.SetData\")",
        "            let handle = try nativeStorage.validatedHandle(\"Texture2D.SetData\")",
    ),
    (
        "getdata-disposal-check-after-the-arguments",
        "the disposal check moved behind the three validations",
        TEXTURE2D,
        "            let handle = try validatedHandle(\"Texture2D.GetData\")\n"
        "            let plan = try transferPlan(\n"
        "                T.self, level: level, rect: rect, arrayCount: data.count,\n"
        "                startIndex: startIndex, elementCount: elementCount,\n"
        "                isSetting: false)",
        "            let plan = try transferPlan(\n"
        "                T.self, level: level, rect: rect, arrayCount: data.count,\n"
        "                startIndex: startIndex, elementCount: elementCount,\n"
        "                isSetting: false)\n"
        "            let handle = try validatedHandle(\"Texture2D.GetData\")",
    ),
    (
        "closed-stream-accepted-by-save",
        "SaveAsPng writing to a stream XNA refuses for CanWrite",
        TEXTURE2D,
        "            guard stream.streamStatus != .closed, stream.streamStatus != .error else {",
        "            guard stream.streamStatus != .error else {",
    ),

    # ---- Foundation 57: SetData and GetData ------------------------------
    (
        "format-byte-size-wrong-for-color",
        "the one format this artifact can create given the wrong byte size",
        TEXTUREDATA,
        "        case .Color: return 4\n"
        "        case .Bgr565: return 2",
        "        case .Color: return 2\n"
        "        case .Bgr565: return 2",
    ),
    (
        "element-size-rule-demands-an-exact-match",
        "a T that divides the format's size refused, where XNA accepts it",
        TEXTURE2D,
        "                guard formatSize > elementSize, formatSize % elementSize == 0 else {",
        "                guard false else {",
    ),
    (
        "rectangle-origin-test-loosened",
        "a negative rectangle origin accepted",
        TEXTURE2D,
        "                guard rect.X >= 0, rect.Width > 0, rect.Y >= 0, rect.Height > 0 else {",
        "                guard rect.X >= -1, rect.Width > 0, rect.Y >= 0, rect.Height > 0 else {",
    ),
    (
        "rectangle-edge-test-signed",
        "the edge comparison made signed, so an overflowing rectangle slips through",
        TEXTURE2D,
        "                let right = UInt32(bitPattern: rect.X &+ rect.Width)\n"
        "                let bottom = UInt32(bitPattern: rect.Y &+ rect.Height)\n"
        "                guard right <= UInt32(bitPattern: Width),\n"
        "                      bottom <= UInt32(bitPattern: Height) else {",
        "                let right = rect.X &+ rect.Width\n"
        "                let bottom = rect.Y &+ rect.Height\n"
        "                guard right <= Width,\n"
        "                      bottom <= Height else {",
    ),
    (
        "total-size-allows-a-short-window",
        "a window smaller than the region accepted",
        TEXTURE2D,
        "            guard regionBytes == windowBytes else {",
        "            guard regionBytes >= windowBytes else {",
    ),
    (
        "array-window-offset-ignored",
        "startIndex dropped, so the transfer always reads from the array's front",
        TEXTURE2D,
        "            let byteOffset = Int(startIndex) * Int(elementSize)",
        "            let byteOffset = 0 * Int(elementSize)",
    ),
    (
        "rectangle-never-reaches-the-transfer",
        "the region validated and then not sent, so the whole surface moves",
        TEXTURE2D,
        "            if let rect {\n"
        "                transfer.has_rectangle = 1",
        "            if let rect, false {\n"
        "                transfer.has_rectangle = 1",
    ),
    # ---- Foundation 55: the Texture2D constructors -----------------------
    (
        "texture-dimension-guard-accepts-zero",
        "the dimension guard written as `>= 0`, so a zero-sized texture is asked for",
        TEXTURE2D,
        "            guard width > 0 else {",
        "            guard width >= 0 else {",
    ),
    (
        "texture-dimension-guard-blames-the-wrong-parameter",
        "the height's refusal naming the width",
        TEXTURE2D,
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"height\",",
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"width\",",
    ),
    (
        "short-texture-constructor-defaults-to-mipmaps",
        "the three-parameter constructor forwarding mipMap true, not the IL's ldc.i4.0",
        TEXTURE2D,
        "            try self.init(graphicsDevice: graphicsDevice, width: width,\n"
        "                          height: height, mipMap: false, format: .Color)",
        "            try self.init(graphicsDevice: graphicsDevice, width: width,\n"
        "                          height: height, mipMap: true, format: .Color)",
    ),
    (
        # Aimed at the level count and not at the dimensions, because
        # `build-probe/f55_grants.c` shows CNA granting the requested width and
        # height EXACTLY in every case tried -- 7x3 and 5000x5000 included --
        # so "reports what was asked for" and "reports what was granted" are
        # indistinguishable there and no test could separate them. The level
        # count is the one field where the grant is not the request: a mipped
        # 16x16 comes back with five levels and the request carries none.
        "created-texture-reports-a-level-count-of-its-own",
        "the granted mip level count replaced by a constant",
        TEXTURE2D,
        "                    levelCount: Int32(info.level_count),\n"
        "                    format: grantedFormat\n"
        "                )\n"
        "            } catch {",
        "                    levelCount: 1,\n"
        "                    format: grantedFormat\n"
        "                )\n"
        "            } catch {",
    ),
    # ---- Foundation 54: Begin's states and when they are applied ---------
    (
        "deferred-batch-applies-its-states-at-begin",
        "SetRenderState called from Begin for every sort mode, not just Immediate",
        BATCH,
        "            if sortMode == .Immediate {\n"
        "                try setRenderState()\n"
        "            }",
        "            try setRenderState()",
    ),
    (
        "immediate-batch-defers-its-states",
        "SetRenderState called from End for every sort mode, not just the deferred ones",
        BATCH,
        "            if spriteSortMode != .Immediate {\n"
        "                try setRenderState()\n"
        "            }",
        "            try setRenderState()",
    ),
    (
        "null-blend-state-defaults-to-opaque",
        "the blend default taken as Opaque, not SetRenderState's AlphaBlend",
        BATCH,
        "        private var resolvedBlendState: BlendState { blendState ?? .AlphaBlend }",
        "        private var resolvedBlendState: BlendState { blendState ?? .Opaque }",
    ),
    (
        "null-depth-state-defaults-to-default",
        "the depth default taken as DepthStencilState.Default, not None",
        BATCH,
        "            depthStencilState ?? .None",
        "            depthStencilState ?? .Default",
    ),
    (
        "sampler-applied-to-the-wrong-slot",
        "SetRenderState writing the sampler to slot one",
        BATCH,
        "            try device.SamplerStates?.SetItem(0, resolvedSamplerState)",
        "            try device.SamplerStates?.SetItem(1, resolvedSamplerState)",
    ),
    (
        "render-state-applied-around-the-device-members",
        "the states pushed to CNA without going through the device, so nothing binds",
        BATCH,
        "            try device.SetBlendState(resolvedBlendState)",
        "            _ = resolvedBlendState",
    ),
    # ---- Foundation 53: the SpriteBatch Draw family ----------------------
    #
    # Only one control here, and the reason is measured rather than assumed.
    # Three more were written -- a uniform scale written to one axis, a
    # transposed destination rectangle, and an absent source rectangle sent as
    # something other than CNA's zero-by-zero -- and all three survived a full
    # run, because NOTHING IN THIS ENVIRONMENT CAN SEE WHAT A SPRITE COMMAND
    # CONTAINS. The qualified HEADLESS artifact has no pixel readback of any
    # kind:
    #
    #   build-probe/f53_readback.c  back buffer      -> CNA_RESULT_NOT_SUPPORTED
    #   build-probe/f53_rtread.c    render target    -> CNA_RESULT_NOT_SUPPORTED
    #
    # and CNA accepts every field value those mutations produce, so the call
    # returns zero either way. The three are withdrawn rather than left here
    # claiming coverage they cannot have. What IS covered is the structure:
    # which command shape each overload uses, and the begin/end rule they all
    # obey.
    (
        "destination-draw-unguarded",
        "the destination family skipping the begin/end rule the position family keeps",
        BATCH,
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeDrawMessage)\n"
        "            }\n"
        "            let batchHandle = try validatedHandle(\"SpriteBatch.Draw\")\n"
        "            let textureHandle = try texture.validatedHandle(\"SpriteBatch.Draw texture\")\n"
        "            guard texture.runtimeState === nativeStorage.runtime else {\n"
        "                throw CNAError.staleRuntimeGeneration(\n"
        "                    expected: nativeStorage.generation,\n"
        "                    actual: texture.runtimeState.generation)\n"
        "            }\n"
        "            var command = CNASwift_SpriteCommand()",
        "            let batchHandle = try validatedHandle(\"SpriteBatch.Draw\")\n"
        "            let textureHandle = try texture.validatedHandle(\"SpriteBatch.Draw texture\")\n"
        "            guard texture.runtimeState === nativeStorage.runtime else {\n"
        "                throw CNAError.staleRuntimeGeneration(\n"
        "                    expected: nativeStorage.generation,\n"
        "                    actual: texture.runtimeState.generation)\n"
        "            }\n"
        "            var command = CNASwift_SpriteCommand()",
    ),
    # ---- Foundation 52: the manager's raisers and its disposal -----------
    (
        "raiser-substitutes-itself-for-the-sender",
        "a device raiser passing `self` where the IL passes its argument",
        MANAGER,
        "        open func OnDeviceCreated(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try deviceCreatedSource.Raise(sender, args: args)",
        "        open func OnDeviceCreated(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try deviceCreatedSource.Raise(self, args: args)",
    ),
    (
        "two-raisers-crossed",
        "OnDeviceReset raising the resetting event",
        MANAGER,
        "        open func OnDeviceReset(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try deviceResetSource.Raise(sender, args: args)",
        "        open func OnDeviceReset(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try deviceResettingSource.Raise(sender, args: args)",
    ),
    (
        "native-events-bypass-the-raisers",
        "the native device events raising the delegates directly, so an "
        "override never runs",
        MANAGER,
        "                case GraphicsDeviceManager.eventDeviceCreated:\n"
        "                    try OnDeviceCreated(self, args: CNAEventArgs.Empty)",
        "                case GraphicsDeviceManager.eventDeviceCreated:\n"
        "                    try deviceCreatedSource.Raise(self, args: CNAEventArgs.Empty)",
    ),
    (
        "dispose-false-still-disposes",
        "Dispose(false) doing the work `disposing` guards",
        MANAGER,
        "            guard disposing else { return }",
        "            guard !disposing || true else { return }",
    ),
    (
        "dispose-removes-the-manager-service",
        "Dispose removing IGraphicsDeviceManager, which XNA leaves registered",
        MANAGER,
        "                    game.Services.RemoveService(\n"
        "                        Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self)",
        "                    game.Services.RemoveService(\n"
        "                        Microsoft.Xna.Framework.IGraphicsDeviceManager.self)",
    ),
    (
        "disposed-raised-before-the-removal",
        "Disposed raised first, so a handler sees a service still registered",
        MANAGER,
        "            guard disposing else { return }\n"
        "            if let game {",
        "            guard disposing else { return }\n"
        "            try disposedSource.Raise(self, args: CNAEventArgs.Empty)\n"
        "            if let game {",
    ),
    # ---- Foundation 51: the GraphicsDeviceManager preferences ------------
    (
        "vsync-default-taken-from-the-clr",
        "SynchronizeWithVerticalRetrace defaulting to false, not the ctor's ldc.i4.1",
        MANAGER,
        "        private var synchronizeWithVerticalRetracePreference = true",
        "        private var synchronizeWithVerticalRetracePreference = false",
    ),
    (
        "depth-preference-default-taken-from-the-clr",
        "PreferredDepthStencilFormat defaulting to None, not the ctor's ldc.i4.2",
        MANAGER,
        "        private var depthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat = .Depth24",
        "        private var depthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat = .None",
    ),
    (
        "dimension-setter-accepts-zero",
        "the dimension guard written as `>= 0`, so zero is accepted",
        MANAGER,
        "        public func SetPreferredBackBufferWidth(_ value: Int32) throws {\n"
        "            guard value > 0 else {",
        "        public func SetPreferredBackBufferWidth(_ value: Int32) throws {\n"
        "            guard value >= 0 else {",
    ),
    (
        "dimension-refusal-still-stores",
        "the width stored before the guard, so a refused value takes effect",
        MANAGER,
        "            backBufferWidth = value\n"
        "            useResizedBackBuffer = false",
        "            backBufferWidth = max(value, backBufferWidth)\n"
        "            useResizedBackBuffer = false",
    ),
    (
        "apply-changes-loses-its-short-circuit",
        "ApplyChanges reconfiguring the device on every call again",
        MANAGER,
        "            if GraphicsDevice != nil, !isDeviceDirty { return }",
        "            if GraphicsDevice != nil, isDeviceDirty { return }",
    ),
    (
        "a-preference-setter-forgets-the-dirty-flag",
        "a setter that stores without marking the device dirty",
        MANAGER,
        "            set { preferMultiSamplingPreference = newValue; isDeviceDirty = true }",
        "            set { preferMultiSamplingPreference = newValue }",
    ),
    (
        "toggle-full-screen-does-not-apply",
        "ToggleFullScreen flipping the field and never changing the device",
        MANAGER,
        "            IsFullScreen = !IsFullScreen\n"
        "            try changeDevice()",
        "            IsFullScreen = !IsFullScreen",
    ),
    (
        "a-pushed-preference-is-dropped",
        "the back-buffer height never handed to CNA",
        MANAGER,
        "            try functions.check(\n"
        "                functions.graphicsManagerSetPreferredBackBufferHeight(handle, backBufferHeight),\n"
        "                operation: \"cna_graphics_device_manager_set_preferred_back_buffer_height\")",
        "            _ = backBufferHeight",
    ),
    # ---- Foundation 50: the messages implemented members raise -----------
    (
        "sprite-batch-second-begin-unguarded",
        "a second Begin left to CNA, which reports a result code and not XNA's rule",
        BATCH,
        "            guard !inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: endMustBeCalledBeforeBeginMessage)\n"
        "            }",
        "            if false {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: endMustBeCalledBeforeBeginMessage)\n"
        "            }",
    ),
    (
        "sprite-batch-rule-checked-after-the-handle",
        "the begin/end rule decided after the handle, so a disposed batch "
        "reports its disposal rather than the rule it broke",
        BATCH,
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeEndMessage)\n"
        "            }\n"
        "            let handle = try validatedHandle(\"SpriteBatch.End\")",
        "            let handle = try validatedHandle(\"SpriteBatch.End\")\n"
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeEndMessage)\n"
        "            }",
    ),
    (
        "sprite-batch-end-guard-inverted",
        "End refusing inside a pair and accepting outside one",
        BATCH,
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeEndMessage)\n"
        "            }",
        "            guard !inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeEndMessage)\n"
        "            }",
    ),
    (
        # Anchored on the scaled funnel's own next line: Foundation 53 gave the
        # destination funnel the identical guard, and an anchor that matches
        # both is a stale site, which the precondition catches before a run.
        "sprite-batch-draw-unguarded",
        "Draw outside a pair left to CNA",
        BATCH,
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeDrawMessage)\n"
        "            }\n"
        "            let batchHandle = try validatedHandle(\"SpriteBatch.Draw\")\n"
        "            let textureHandle = try texture.validatedHandle(\"SpriteBatch.Draw texture\")\n"
        "            guard texture.runtimeState === nativeStorage.runtime else {\n"
        "                throw CNAError.staleRuntimeGeneration(expected: nativeStorage.generation, actual: texture.runtimeState.generation)\n"
        "            }",
        "            let batchHandle = try validatedHandle(\"SpriteBatch.Draw\")\n"
        "            let textureHandle = try texture.validatedHandle(\"SpriteBatch.Draw texture\")\n"
        "            guard texture.runtimeState === nativeStorage.runtime else {\n"
        "                throw CNAError.staleRuntimeGeneration(expected: nativeStorage.generation, actual: texture.runtimeState.generation)\n"
        "            }",
    ),
    (
        "sprite-batch-pair-never-closes",
        "End leaving the pair open, so the next Begin is refused",
        BATCH,
        "            inBeginEndPair = false\n"
        "        }",
        "        }",
    ),
    (
        "sprite-batch-draw-and-end-messages-collapsed",
        "the Draw and End messages made the same string",
        BATCH,
        'internal let beginMustBeCalledBeforeDrawMessage =\n'
        '    "Begin must be called successfully before a Draw can be called."',
        'internal let beginMustBeCalledBeforeDrawMessage =\n'
        '    "Begin must be called successfully before End can be called."',
    ),
    (
        "drawable-device-getter-uses-its-siblings-message",
        "the defect Foundation 50 repaired, put back",
        DRAWABLE_COMPONENT,
        "                        message: DrawableGameComponent\n"
        "                            .propertyCannotBeCalledBeforeInitializeMessage)",
        "                        message: DrawableGameComponent\n"
        "                            .missingGraphicsDeviceServiceMessage)",
    ),
    # ---- Foundation 49: the validation Foundation 47 dropped -------------
    (
        "viewport-origin-test-loosened",
        "a negative viewport origin accepted",
        DEVICE,
        "            guard value.X >= 0, value.Y >= 0, value.Width > 0, value.Height > 0 else {",
        "            guard value.X >= -1, value.Y >= 0, value.Width > 0, value.Height > 0 else {",
    ),
    (
        "viewport-extent-test-uses-blt",
        "a zero-width viewport accepted, as `blt` rather than `ble` would",
        DEVICE,
        "            guard value.X >= 0, value.Y >= 0, value.Width > 0, value.Height > 0 else {",
        "            guard value.X >= 0, value.Y >= 0, value.Width >= 0, value.Height > 0 else {",
    ),
    (
        "viewport-bounds-checked-additively",
        "the viewport's extent compared without its origin",
        DEVICE,
        "            guard value.X &+ value.Width <= bounds.width,\n"
        "                  value.Y &+ value.Height <= bounds.height else {",
        "            guard value.Width <= bounds.width,\n"
        "                  value.Height <= bounds.height else {",
    ),
    (
        "viewport-depth-comparison-ordered",
        "MaxDepth >= MinDepth compared as an ordered branch, so NaN passes",
        DEVICE,
        "            if !(Double(value.MaxDepth) >= Double(value.MinDepth)) {",
        "            if Double(value.MaxDepth) < Double(value.MinDepth) {",
    ),
    # WITHDRAWN: "viewport-depth-range-rejects-nan-early". Rewriting the depth
    # RANGE test as a requirement rejects NaN one branch early — but it throws
    # the same exception with the same message, so which branch rejected it is
    # not observable from outside. The restructuring that this mutation was
    # written to protect is still worth having: it is what makes
    # "viewport-depth-comparison-ordered" catchable at all, and that one is a
    # real behavioural difference. This one was tried, survived, and is
    # recorded here rather than deleted quietly.
    (
        "viewport-bounds-always-the-backbuffer",
        "the bounds read from the presentation parameters even with a target bound",
        DEVICE,
        "            if let slotZero = boundRenderTargetSlotZero {\n"
        "                return (slotZero.renderTargetWidth, slotZero.renderTargetHeight)\n"
        "            }",
        "            if let slotZero = boundRenderTargetSlotZero, false {\n"
        "                return (slotZero.renderTargetWidth, slotZero.renderTargetHeight)\n"
        "            }",
    ),
    (
        "scissor-negative-test-uses-ble",
        "an empty scissor rectangle rejected, as `ble` rather than `blt` would",
        DEVICE,
        "            guard value.X >= 0, value.Width >= 0,\n"
        "                  value.Y >= 0, value.Height >= 0 else {",
        "            guard value.X >= 0, value.Width > 0,\n"
        "                  value.Y >= 0, value.Height >= 0 else {",
    ),
    # WITHDRAWN: "scissor-edge-test-drops-the-origin". Removing XNA's
    # `X <= targetW` / `Y <= targetH` comparisons changes nothing observable —
    # with X and Width already known non-negative, `right <= targetW` implies
    # `X <= targetW`, and the overflowing width that breaks the implication is
    # rejected by the following pair regardless. It survived a full run, which
    # is what a gate that cannot fail looks like. The comparisons stay in the
    # projection because XNA has them; the mutation does not stay here.
    (
        "scissor-overflow-pair-dropped",
        "the pair of tests only an int32 overflow reaches, removed as redundant",
        DEVICE,
        "            guard right &- value.X <= bounds.width,\n"
        "                  bottom &- value.Y <= bounds.height else {",
        "            guard right &- value.X <= Int32.max,\n"
        "                  bottom &- value.Y <= Int32.max else {",
    ),
    (
        "invalid-bounds-messages-exchanged",
        "the viewport and scissor messages swapped",
        STATEBRIDGE,
        'internal let scissorInvalidMessage =\n'
        '    "The scissor rectangle is invalid. The scissor rectangle cannot be larger "',
        'internal let scissorInvalidMessage =\n'
        '    "The viewport is invalid. The scissor rectangle cannot be larger "',
    ),
    # ---- Foundation 48: Clear, and the DefaultClearOptions rule ----------
    (
        "default-clear-options-ignores-the-render-target",
        "get_DefaultClearOptions always reading the presentation parameters",
        DEVICE,
        "                if let target = boundRenderTargetDepthFormat {",
        "                if let target = boundRenderTargetDepthFormat, false {",
    ),
    (
        "default-clear-options-drops-stencil",
        "Depth24Stencil8 clearing depth but not stencil",
        DEVICE,
        "                if format == .Depth24Stencil8 {\n"
        "                    return [.Target, .DepthBuffer, .Stencil]\n"
        "                }",
        "                if format == .Depth24Stencil8 {\n"
        "                    return [.Target, .DepthBuffer]\n"
        "                }",
    ),
    (
        "set-render-target-forgets-to-record",
        "SetRenderTarget not recording the target DefaultClearOptions reads",
        DEVICE,
        "            recordActiveRenderTargets([binding])\n"
        "        }\n"
        "\n"
        "        /// `GraphicsDevice.SetRenderTarget(RenderTargetCube renderTarget, CubeMapFace cubeMapFace)`.",
        "            recordActiveRenderTargets([])\n"
        "        }\n"
        "\n"
        "        /// `GraphicsDevice.SetRenderTarget(RenderTargetCube renderTarget, CubeMapFace cubeMapFace)`.",
    ),
    (
        "null-depth-diagnosis-fires-on-every-failure",
        "every failed clear blamed on an absent depth or stencil buffer",
        DEVICE,
        "            guard try defaultClearOptions.intersection(requested) == requested else {",
        "            guard try defaultClearOptions.intersection(requested) != requested else {",
    ),
    (
        "null-depth-diagnosis-never-fires",
        "a clear of buffers the device lacks reported as a bare native failure",
        DEVICE,
        "                throw CNAInvalidOperationException(message: cannotClearNullDepthMessage)",
        "                _ = cannotClearNullDepthMessage",
    ),
    (
        "clear-options-drops-undeclared-bits",
        "an option XNA does not declare silently narrowed away",
        STATEBRIDGE,
        "        native |= UInt32(bitPattern: value.rawValue & ~declared)",
        "        native |= 0",
    ),
    (
        "clear-options-swaps-depth-and-stencil",
        "DepthBuffer and Stencil mapped to each other's canonical bits",
        STATEBRIDGE,
        "        if value.contains(.DepthBuffer) { native |= 2 }\n"
        "        if value.contains(.Stencil) { native |= 4 }",
        "        if value.contains(.DepthBuffer) { native |= 4 }\n"
        "        if value.contains(.Stencil) { native |= 2 }",
    ),
    (
        "wrong-exception-class", "a raise site throwing a neighbouring class",
        DICTIONARY,
        "            throw CNAArgumentException(\n"
        "                message: CNADictionary.addingDuplicateMessage)",
        "            throw CNAArgumentOutOfRangeException(\n"
        "                message: CNADictionary.addingDuplicateMessage)",
    ),
    (
        "wrong-base-class", "a projected class on the wrong CLR base",
        EXCEPTIONS,
        "open class CNAKeyNotFoundException: CNASystemException {",
        "open class CNAKeyNotFoundException: CNAArgumentException {",
    ),
    (
        "dropped-message-composition", "Message no longer appending ParamName",
        EXCEPTIONS,
        "        let base = super.Message\n"
        "        guard let name = storedParamName, !name.isEmpty else { return base }",
        "        let base = super.Message\n"
        "        guard let name = storedParamName, name.isEmpty else { return base }",
    ),
    (
        "wrong-newline", "the composed separator taken from the host, not the IL",
        EXCEPTIONS,
        'internal static let environmentNewLine = "\\r\\n"',
        'internal static let environmentNewLine = "\\n"',
    ),
    (
        "inherited-hresult", "a subclass keeping the HResult its base assigned",
        EXCEPTIONS,
        "    public init(paramName: String?) {\n"
        "        super.init(\n"
        "            message: CNAArgumentNullException.argumentNullGenericMessage,\n"
        "            paramName: paramName)\n"
        "        HResult = CNAArgumentNullException.argumentNullHResult\n"
        "    }",
        "    public init(paramName: String?) {\n"
        "        super.init(\n"
        "            message: CNAArgumentNullException.argumentNullGenericMessage,\n"
        "            paramName: paramName)\n"
        "    }",
    ),
    (
        "transposed-constructor", "the two-argument overload's arguments swapped",
        EXCEPTIONS,
        "    public init(paramName: String?, message: String?) {\n"
        "        super.init(message: message, paramName: paramName)\n"
        "        HResult = CNAArgumentNullException.argumentNullHResult",
        "    public init(paramName: String?, message: String?) {\n"
        "        super.init(message: paramName, paramName: message)\n"
        "        HResult = CNAArgumentNullException.argumentNullHResult",
    ),
    (
        "confused-range-resource", "Insert reporting the indexer's message",
        COLLECTIONS,
        "    internal static var listInsertMessage: String {\n"
        '        "Index must be within the bounds of the List."\n'
        "    }",
        "    internal static var listInsertMessage: String {\n"
        '        "Index was out of range. Must be non-negative and less than "\n'
        '        + "the size of the collection."\n'
        "    }",
    ),
    (
        "confused-not-supported-resource",
        "the read-only guard reporting the parameterless default",
        COLLECTIONS,
        "    internal static var readOnlyCollectionMessage: String {\n"
        '        "Collection is read-only."\n'
        "    }",
        "    internal static var readOnlyCollectionMessage: String {\n"
        '        "Specified method is not supported."\n'
        "    }",
    ),
    (
        "swift-class-name-in-a-message",
        "a projected class reporting its Swift name to a user",
        EXCEPTIONS,
        '        "CNAArgumentException": "System.ArgumentException",\n',
        "",
    ),
    (
        "reverted-to-the-runtime-channel",
        "a CLR-shaped failure back on CNAError",
        SERVICES,
        "                throw CNAArgumentNullException(\n"
        "                    paramName: \"provider\",\n"
        "                    message: GameServiceContainer.serviceProviderCannotBeNullMessage)",
        "                throw CNAError.producerInvariant(\"provider\")",
    ),
    # The Foundation 38 derivability and ownership decisions.
    (
        "texture2d-resealed", "Texture2D final again, as it was", TEXTURE2D,
        "    open class Texture2D: Texture {",
        "    public final class Texture2D: Texture {",
    ),
    (
        "disposing-raised-before-release",
        "Disposing raised before the native release", RESOURCE,
        "            guard !IsDisposed else { return }\n"
        "            if let storage {\n"
        "                try storage.dispose(operation: \"\\(storage.typeName).Dispose\")\n"
        "            } else {\n"
        "                managedDisposed = true\n"
        "            }\n"
        "            guard disposing else { return }\n"
        "            try disposingSource.Raise(self, args: CNAEventArgs.Empty)",
        "            guard !IsDisposed else { return }\n"
        "            guard disposing else { return }\n"
        "            try disposingSource.Raise(self, args: CNAEventArgs.Empty)\n"
        "            if let storage {\n"
        "                try storage.dispose(operation: \"\\(storage.typeName).Dispose\")\n"
        "            } else {\n"
        "                managedDisposed = true\n"
        "            }",
    ),
    (
        "dispose-not-idempotent", "a second Dispose reaching the dead handle",
        RESOURCE,
        "            guard !IsDisposed else { return }\n"
        "            if let storage {\n"
        "                try storage.dispose(operation: \"\\(storage.typeName).Dispose\")",
        "            if let storage {\n"
        "                try storage.dispose(operation: \"\\(storage.typeName).Dispose\")",
    ),
    (
        "derived-type-name-lost",
        "the shared storage naming the base rather than the derived type",
        RENDER_TARGET,
        "                    typeName: \"RenderTarget2D\",",
        "                    typeName: \"Texture2D\",",
    ),
    (
        "content-lost-subscription-leaked",
        "disposal leaving the native subscription alive", RENDER_TARGET,
        "            contentLostSubscription?.release()\n"
        "            try super.Dispose(disposing)",
        "            try super.Dispose(disposing)",
    ),
    # The Foundation 39 Game surface.
    (
        "event-sender-is-the-parameter",
        "an On... method passing its sender parameter rather than the game",
        GAME,
        "        open func OnActivated(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try activatedSource.Raise(self, args: args)",
        "        open func OnActivated(_ sender: Any?, args: CNAEventArgs) throws {\n"
        "            try activatedSource.Raise(sender, args: args)",
    ),
    (
        "target-elapsed-accepts-zero",
        "the strict TargetElapsedTime bound loosened to the sleep-time one",
        GAME,
        "            guard value > .zero else {\n"
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"value\",\n"
        "                    message: Game.targetElapsedCannotBeZeroMessage)",
        "            guard value >= .zero else {\n"
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"value\",\n"
        "                    message: Game.targetElapsedCannotBeZeroMessage)",
    ),
    (
        "inactive-sleep-refuses-zero",
        "the sleep-time bound tightened to the target-elapsed one", GAME,
        "            guard value >= .zero else {\n"
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"value\",\n"
        "                    message: Game.inactiveSleepTimeCannotBeZeroMessage)",
        "            guard value > .zero else {\n"
        "                throw CNAArgumentOutOfRangeException(\n"
        "                    paramName: \"value\",\n"
        "                    message: Game.inactiveSleepTimeCannotBeZeroMessage)",
    ),
    (
        "mirror-moves-on-a-refused-write",
        "the mirror reporting a state the host never took", GAME,
        "                guard let handle = try? validatedHandle(\"Game.IsFixedTimeStep\") else { return }\n"
        "                guard runtime.functions.gameSetIsFixedTimeStep(handle, newValue ? 1 : 0) == 0\n"
        "                else { return }\n"
        "                mirroredIsFixedTimeStep = newValue",
        "                mirroredIsFixedTimeStep = newValue\n"
        "                guard let handle = try? validatedHandle(\"Game.IsFixedTimeStep\") else { return }\n"
        "                _ = runtime.functions.gameSetIsFixedTimeStep(handle, newValue ? 1 : 0)",
    ),
    (
        "teardown-callback-mapped-onto-exiting",
        "CNA's teardown notification mapped onto XNA's Exiting", CALLBACK_STATE,
        "        case .exiting:\n"
        "            // Deliberately NOT `OnExiting`.",
        "        case .exiting:\n"
        "            try game.OnExiting(game, args: CNAEventArgs.Empty)\n"
        "            // Deliberately NOT `OnExiting`.",
    ),
    # The Foundation 40 service producer and DrawableGameComponent.
    (
        "manager-registers-only-one-service",
        "the producer registered under one interface and not both", MANAGER,
        "            try game.Services.AddService(\n"
        "                Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self, provider: self)",
        "",
    ),
    (
        "duplicate-manager-accepted",
        "a second manager admitted where XNA refuses one", MANAGER,
        "            if game.Services.GetService(Microsoft.Xna.Framework.IGraphicsDeviceManager.self) != nil {",
        "            if false {",
    ),
    (
        "drawable-reload-guarded-by-the-one-time-flag",
        "the device-reset reload suppressed by Initialize's guard", DRAWABLE,
        "            deviceSubscriptions.append(deviceService.DeviceCreated.Add {\n"
        "                [weak self] _, _ in try self?.LoadContent()\n"
        "            })",
        "            deviceSubscriptions.append(deviceService.DeviceCreated.Add { _, _ in })",
    ),
    (
        "drawable-shares-the-game-message",
        "the two different missing-service messages confused", DRAWABLE,
        "        internal static let missingGraphicsDeviceServiceMessage =\n"
        "            \"Drawable components require a graphics device service in the game \"\n"
        "            + \"service container.\"",
        "        internal static let missingGraphicsDeviceServiceMessage =\n"
        "            Microsoft.Xna.Framework.Game.noGraphicsDeviceServiceMessage",
    ),
    (
        "drawable-visible-raises-without-a-change",
        "an unchanged write raising the change event", DRAWABLE,
        "                guard visible != newValue else { return }\n"
        "                visible = newValue",
        "                visible = newValue",
    ),
    (
        "host-subscriptions-leaked",
        "disposal leaving the four host subscriptions alive", GAME,
        "            releaseHostEventSubscriptions()\n"
        "            runtime.clearCallbackError()",
        "            runtime.clearCallbackError()",
    ),
    (
        "state-default-off-by-one", "a single IL-derived state default changed",
        STATES,
        "        private var maxAnisotropy: Int32 = 4",
        "        private var maxAnisotropy: Int32 = 1",
    ),
    (
        "multisample-antialias-default-inverted",
        "the RasterizerState default XNA turns ON, turned off",
        STATES,
        "        private var multiSampleAntiAlias = true",
        "        private var multiSampleAntiAlias = false",
    ),
    (
        "preset-blend-pair-transposed",
        "a preset's source and destination blend swapped",
        STATES,
        '        public static let Additive = BlendState(\n'
        '            source: .SourceAlpha, destination: .One, name: "BlendState.Additive")',
        '        public static let Additive = BlendState(\n'
        '            source: .One, destination: .SourceAlpha, name: "BlendState.Additive")',
    ),
    (
        "preset-address-mode-on-one-axis-only",
        "the presetting constructor writing only AddressU",
        STATES,
        "            addressU = address\n"
        "            addressV = address\n"
        "            addressW = address",
        "            addressU = address",
    ),
    (
        "bound-message-names-the-dynamic-type",
        "ThrowIfBound reading the dynamic class instead of the declaring one",
        STATES,
        "                of: \"{0}\", with: Self.boundStateTypeName))",
        "                of: \"{0}\", with: Microsoft.Xna.Framework.Graphics\n"
        "                    .GraphicsResource.clrTypeName(of: self)\n"
        "                    .split(separator: \".\").last.map(String.init) ?? \"\"))",
    ),
    (
        "setter-skips-the-bound-guard",
        "one state setter writing without calling ThrowIfBound",
        STATES,
        "        public func SetMultiSampleMask(_ value: Int32) throws {\n"
        "            try throwIfBound(); multiSampleMask = value",
        "        public func SetMultiSampleMask(_ value: Int32) throws {\n"
        "            multiSampleMask = value",
    ),
    (
        "preset-not-born-bound",
        "a static preset left mutable, so a caller can corrupt a shared global",
        STATES,
        "            alphaDestinationBlend = destination\n"
        "            Name = name\n"
        "            isBound = true",
        "            alphaDestinationBlend = destination\n"
        "            Name = name",
    ),
    (
        "fresh-state-born-bound",
        "the parameterless constructor binding, freezing a brand-new state",
        STATES,
        "        internal var isBound = false\n"
        "        internal weak var attachedDevice: "
        "Microsoft.Xna.Framework.Graphics.GraphicsDevice?\n"
        "        internal static let boundStateTypeName = \"SamplerState\"",
        "        internal var isBound = true\n"
        "        internal weak var attachedDevice: "
        "Microsoft.Xna.Framework.Graphics.GraphicsDevice?\n"
        "        internal static let boundStateTypeName = \"SamplerState\"",
    ),
    (
        "disposal-back-on-the-runtime-channel",
        "a use-after-dispose reported as a CNA runtime failure again",
        RESOURCE,
        "            guard !storage.isDisposed else {\n"
        "                throw CNAObjectDisposedException(objectName: storage.typeName)\n"
        "            }",
        "            guard !storage.isDisposed else {\n"
        "                throw CNAError.disposedObject(storage.typeName)\n"
        "            }",
    ),
    (
        "sprite-batch-guard-bypasses-the-class",
        "one resource reaching its handle past its own disposal guard",
        BATCH,
        '            let handle = try validatedHandle("SpriteBatch.Begin")',
        '            let handle = try nativeStorage.validatedHandle("SpriteBatch.Begin")',
    ),
    (
        "object-disposed-on-the-wrong-base",
        "ObjectDisposedException derived from SystemException, not "
        "InvalidOperationException",
        EXCEPTIONS,
        "open class CNAObjectDisposedException: CNAInvalidOperationException {",
        "open class CNAObjectDisposedException: CNASystemException {",
    ),
    (
        "object-name-not-collapsed-to-empty",
        "ObjectName answering nil where the CLR getter answers String.Empty",
        EXCEPTIONS,
        '    public var ObjectName: String { storedObjectName ?? "" }',
        '    public var ObjectName: String { storedObjectName ?? " " }',
    ),
    (
        "object-disposed-constructor-transposed",
        "the single-argument constructor treated as a message, not an object name",
        EXCEPTIONS,
        "    public init(objectName: String?) {\n"
        "        storedObjectName = objectName\n"
        "        super.init(\n"
        "            message: CNAObjectDisposedException.objectDisposedGenericMessage)",
        "    public init(objectName: String?) {\n"
        "        storedObjectName = nil\n"
        "        super.init(message: objectName)",
    ),
    (
        "stride-summed-instead-of-maximised",
        "the vertex stride as a sum of element sizes rather than the widest end",
        VERTEXDECL,
        "                let end = element.Offset + typeSize(element.VertexElementFormat)\n"
        "                if maximum < end { maximum = end }",
        "                maximum += typeSize(element.VertexElementFormat)",
    ),
    (
        "one-type-size-wrong",
        "a single VertexElementFormat size off by a half",
        VERTEXDECL,
        "            case .HalfVector4: return 8",
        "            case .HalfVector4: return 4",
    ),
    (
        "duplicate-check-ignores-the-usage-index",
        "two elements treated as duplicates on usage alone",
        VERTEXDECL,
        "                for earlier in elements[..<index] where\n"
        "                    earlier.VertexElementUsage == element.VertexElementUsage\n"
        "                    && earlier.UsageIndex == element.UsageIndex {",
        "                for earlier in elements[..<index] where\n"
        "                    earlier.VertexElementUsage == element.VertexElementUsage {",
    ),
    (
        "overlap-check-dropped",
        "two elements allowed to claim the same byte of the vertex",
        VERTEXDECL,
        "                    guard owner[byte] < 0 else {",
        "                    guard true else {",
    ),
    (
        "alignment-checked-before-the-stride-fit",
        "the two per-element checks in the wrong order, so a doubly-invalid "
        "element reports the wrong message",
        VERTEXDECL,
        "                guard element.Offset >= 0,\n"
        "                      element.Offset + size <= vertexStride else {",
        "                guard element.Offset & 3 == 0, element.Offset >= 0,\n"
        "                      element.Offset + size <= vertexStride else {",
    ),
    (
        "nonempty-element-array-erased",
        "every nonempty declaration treated as XNA's accepted empty special "
        "case, erasing its stride and elements",
        VERTEXDECL,
        "            guard !elements.isEmpty else {\n"
        "                storedElements = nil",
        "            guard !elements.isEmpty, false else {\n"
        "                storedElements = nil",
    ),
    (
        "vertex-element-quadruple-transposed",
        "a static declaration's element offset and format exchanged",
        VERTEXCOLOR,
        "                VertexElement(12, .Color, .Color, 0),",
        "                VertexElement(16, .Color, .Color, 0),",
    ),
    (
        "vertex-declaration-name-dropped",
        "a static declaration left unnamed where the class constructor names it",
        VERTEXCOLOR,
        '            .named("VertexPositionColor.VertexDeclaration")',
        "",
    ),
    (
        "vertex-hash-drops-a-word",
        "one word of a vertex layout left out of the folded hash",
        VERTEXNORMAL,
        "            let hash = Position.X.bitPattern\n"
        "                ^ Position.Y.bitPattern\n"
        "                ^ Position.Z.bitPattern\n"
        "                ^ Normal.X.bitPattern",
        "            let hash = Position.X.bitPattern\n"
        "                ^ Position.Y.bitPattern\n"
        "                ^ Position.Z.bitPattern",
    ),
    (
        "vertex-equality-ignores-a-field",
        "a vertex field left out of op_Equality",
        VERTEXNORMAL,
        "            lhs.Position == rhs.Position\n"
        "                && lhs.Normal == rhs.Normal\n"
        "                && lhs.TextureCoordinate == rhs.TextureCoordinate",
        "            lhs.Position == rhs.Position\n"
        "                && lhs.Normal == rhs.Normal",
    ),
    (
        "blend-function-passed-through-raw",
        "the one enum CNA numbers differently, cast instead of mapped",
        STATEBRIDGE,
        "            case .Min: return 4\n"
        "            case .Max: return 3",
        "            case .Min: return 3\n"
        "            case .Max: return 4",
    ),
    (
        "blend-descriptor-channels-transposed",
        "the colour and alpha channels written to each other's POD fields",
        STATEBRIDGE,
        "        native.alpha_destination_blend = Codes.blend(AlphaDestinationBlend)\n"
        "        native.alpha_source_blend = Codes.blend(AlphaSourceBlend)",
        "        native.alpha_destination_blend = Codes.blend(ColorDestinationBlend)\n"
        "        native.alpha_source_blend = Codes.blend(ColorSourceBlend)",
    ),
    (
        "device-setter-skips-the-attachment",
        "a state cached without being bound, so it stays writable afterwards",
        DEVICE,
        "            try value.attach(to: self)\n"
        "            var native = value.nativeDescriptor()\n"
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetDepthStencilState(handle, &native),",
        "            var native = value.nativeDescriptor()\n"
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetDepthStencilState(handle, &native),",
    ),
    (
        "device-setter-forgets-the-copied-value",
        "set_BlendState caching the state but not the value it copies out",
        DEVICE,
        "            runtime.cachedBlendFactor = value.BlendFactor",
        "",
    ),
    (
        "sampler-slot-identity-lost",
        "the collection storing a copy rather than the caller's own instance",
        SAMPLERS,
        "            slots[resolved] = value",
        "            slots[resolved] = SamplerState()",
    ),
    (
        "sampler-collection-rebuilt-per-access",
        "a new collection per device read, losing every slot already written",
        RUNTIME_STATE,
        "        if let existing = pixelSamplerStates {\n"
        "            existing.rebind(to: device)\n"
        "            return existing\n"
        "        }",
        "",
    ),
    (
        "copied-value-writer-skips-the-device",
        "a direct write cached without reaching the device it must push to",
        DEVICE,
        "            let handle = try validatedHandle(\"GraphicsDevice.MultiSampleMask\")\n"
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetMultiSampleMask(handle, value),\n"
        "                operation: \"cna_graphics_device_set_multi_sample_mask\")\n"
        "            runtime.cachedMultiSampleMask = value",
        "            runtime.cachedMultiSampleMask = value",
    ),
    (
        "viewport-writer-does-not-reach-the-device",
        "SetViewport accepted and discarded rather than pushed",
        DEVICE,
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetViewport(handle, native),\n"
        "                operation: \"cna_graphics_device_set_viewport\")",
        "            _ = native",
    ),
    (
        "scissor-rectangle-fields-transposed",
        "the scissor rectangle's origin and extent exchanged on the way out",
        DEVICE,
        "            native.x = value.X\n"
        "            native.y = value.Y\n"
        "            native.width = value.Width\n"
        "            native.height = value.Height\n"
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetScissorRectangle(handle, native),",
        "            native.x = value.Width\n"
        "            native.y = value.Height\n"
        "            native.width = value.X\n"
        "            native.height = value.Y\n"
        "            try runtime.functions.check(\n"
        "                runtime.functions.graphicsDeviceSetScissorRectangle(handle, native),",
    ),
    (
        "device-status-values-shifted",
        "the three GraphicsDeviceStatus values mapped one place along",
        DEVICE,
        "                case 0: return .Normal\n"
        "                case 1: return .Lost\n"
        "                case 2: return .NotReset",
        "                case 0: return .Lost\n"
        "                case 1: return .NotReset\n"
        "                case 2: return .Normal",
    ),
    (
        "default-back-buffer-width-transcribed-wrong",
        "the GraphicsDeviceManager default back-buffer width off by a digit",
        MANAGER,
        "        public static let DefaultBackBufferWidth: Int32 = 800",
        "        public static let DefaultBackBufferWidth: Int32 = 640",
    ),
    # ---- Foundation 64: TextureCube and Texture3D ------------------------
    (
        "cube-size-limit-is-the-texture-limit",
        "MaxCubeSize read as MaxTextureSize, so a 1024 cube is accepted",
        PROFILECAPS,
        "            guard size <= maxCubeSize else {",
        "            guard size <= maxTextureSize else {",
    ),
    (
        "cube-power-of-two-test-dropped",
        "a non-power-of-two cube accepted where Reach refuses one",
        PROFILECAPS,
        "            if !nonPow2Cube, !ProfileCapabilities.isPowerOfTwo(size) {",
        "            if false, !ProfileCapabilities.isPowerOfTwo(size) {",
    ),
    (
        "cube-format-list-is-the-texture-list",
        "ValidCubeFormats read as ValidTextureFormats, a different list",
        PROFILECAPS,
        "            guard validCubeFormats.contains(format) else {",
        "            guard validTextureFormats.contains(format) else {",
    ),
    (
        "volume-zero-extent-test-dropped",
        "a Texture3D allowed past the check that makes Reach refuse every one",
        PROFILECAPS,
        "            guard maxVolumeExtent != 0 else {",
        "            guard true else {",
    ),
    (
        "volume-extent-guards-run-after-the-profile-test",
        "a non-positive extent reported as a profile refusal, not its own name",
        PROFILECAPS,
        "            for (value, name) in [(width, \"width\"), (height, \"height\"), (depth, \"depth\")] {\n"
        "                guard value > 0 else {\n"
        "                    throw CNAArgumentOutOfRangeException(\n"
        "                        paramName: name,\n"
        "                        message: Microsoft.Xna.Framework.Graphics.Texture2D\n"
        "                            .resourcesMustBeGreaterThanZeroSizeMessage)\n"
        "                }\n"
        "            }",
        "",
    ),
    (
        "volume-aspect-ratio-ignores-the-depth",
        "Max/Min taken over width and height only, never over all three extents",
        PROFILECAPS,
        "            (max(max(width, height), depth), min(min(width, height), depth))",
        "            (max(width, height), min(width, height))",
    ),
    (
        "cube-empty-array-not-reported-as-null",
        "a zero-length array falling through to the size tests",
        TEXTURECUBE,
        "            guard arrayCount > 0 else {",
        "            guard arrayCount >= 0 else {",
    ),
    (
        "cube-window-not-validate-copy-parameters",
        "the array window silently accepted instead of raising MustBeValidIndex",
        TEXTURECUBE,
        "            try Microsoft.Xna.Framework.Graphics.validateCopyParameters(\n"
        "                dataLength: arrayCount, dataIndex: startIndex,\n"
        "                elementCount: elementCount)",
        "",
    ),
    (
        "cube-disposal-checked-after-the-arguments",
        "a disposed cube reporting a bad argument instead of the disposal",
        TEXTURECUBE,
        "            let handle = try validatedHandle(\n"
        "                isSetting ? \"TextureCube.SetData\" : \"TextureCube.GetData\")\n"
        "            guard arrayCount > 0 else {",
        "            guard arrayCount > 0 else {",
    ),
    (
        "cube-transfer-does-not-reach-the-route",
        "SetData accepted and discarded rather than handed to CNA",
        TEXTURECUBE,
        "                            functions.textureCubeSetData(\n"
        "                                plan.handle, transfer, $0, plan.elementCount)",
        "                            CNA_Result(0)",
    ),
    (
        "volume-box-test-signed-not-unsigned",
        "a negative box coordinate slipping past a comparison XNA makes unsigned",
        TEXTUREDATA,
        "        return uRight <= UInt32(bitPattern: width) && uLeft < uRight\n"
        "            && uBottom <= UInt32(bitPattern: height) && uTop < uBottom\n"
        "            && uBack <= UInt32(bitPattern: depth) && uFront < uBack",
        "        return right <= width && left < right\n"
        "            && bottom <= height && top < bottom\n"
        "            && back <= depth && front < back",
    ),
    (
        "texture2d-empty-array-not-reported-as-null",
        "Texture2D's zero-length array falling through to the size tests",
        TEXTURE2D,
        "            guard arrayCount > 0 else {",
        "            guard arrayCount >= 0 else {",
    ),
    (
        "texture2d-window-not-validate-copy-parameters",
        "Texture2D's array window silently accepted instead of MustBeValidIndex",
        TEXTURE2D,
        "            try Microsoft.Xna.Framework.Graphics.validateCopyParameters(\n"
        "                dataLength: arrayCount, dataIndex: startIndex,\n"
        "                elementCount: elementCount)",
        "",
    ),
    # ---- Foundation 65: the render-target family --------------------------
    (
        "cube-target-reads-the-cube-info-route",
        "RenderTargetCube's description read where CNA answers zeros",
        RTCUBE,
        "                    runtime.functions.renderTargetGetInfo(handle, &info),\n"
        "                    operation: \"cna_render_target_get_info\")",
        "                    runtime.functions.renderTargetGetInfo(handle, &info),\n"
        "                    operation: \"cna_render_target_get_info\")\n"
        "                info.width = 0; info.height = 0; info.level_count = 0",
    ),
    (
        "cube-target-skips-the-profile-checks",
        "a 1024 or non-power-of-two cube target accepted where Reach refuses one",
        RTCUBE,
        "            try graphicsDevice.profileCapabilities.validateCubeCreation(\n"
        "                size: size, format: preferredFormat)",
        "",
    ),
    (
        "cube-target-does-not-release-its-subscription",
        "a disposed cube target leaving a native callback addressing freed state",
        RTCUBE,
        "            contentLostSubscription?.release()\n"
        "            try super.Dispose(disposing)",
        "            try super.Dispose(disposing)",
    ),
    (
        "binding-2d-face-is-not-positive-x",
        "a 2D binding carrying a face where XNA stores ldc.i4.0",
        RTBINDING,
        "            storedRenderTarget = renderTarget\n"
        "            storedCubeMapFace = .PositiveX",
        "            storedRenderTarget = renderTarget\n"
        "            storedCubeMapFace = .NegativeZ",
    ),
    (
        "render-target-cache-not-updated",
        "what was bound not recorded, so GetRenderTargets answers the wrong array",
        DEVICE,
        "            recordActiveRenderTargets(renderTargets)",
        "            recordActiveRenderTargets([])",
    ),
    # `render-target-identity-lost` -- rebuilding each binding from the target it
    # already holds -- was planted here and SURVIVED, because it is a NO-OP: a
    # RenderTargetBinding is a struct with no identity of its own, so rebuilding
    # one around the same target is indistinguishable from returning it. It is
    # replaced rather than scored. What `GetRenderTargets` can actually get
    # wrong is covered: `render-target-cache-not-updated` catches the wrong
    # array, and `cube-face-not-carried-to-the-device` covers the face.
    #
    # `testTheReturnedArrayIsACopy` asserts a claim no mutation can falsify
    # either: Swift's Array is a value, so a returned array simply cannot alias
    # the cache. The test documents the guarantee rather than proving code.
    (
        "render-target-limit-unchecked",
        "two simultaneous targets accepted where Reach's MaxRenderTargets is 1",
        DEVICE,
        "            guard bindings.count <= Int(capabilities.maxRenderTargets) else {",
        "            guard bindings.count <= Int(capabilities.maxVertexStreams) else {",
    ),
    (
        "render-target-device-compared-by-facade",
        "the device-identity test comparing per-callback tokens",
        DEVICE,
        "                guard target.nativeStorage.runtime === runtime else {\n"
        "                    throw CNAInvalidOperationException(\n"
        "                        message: GraphicsDevice.invalidDeviceMessage)\n"
        "                }\n"
        "                guard index > 0 else { continue }",
        "                guard target.GraphicsDevice === self else {\n"
        "                    throw CNAInvalidOperationException(\n"
        "                        message: GraphicsDevice.invalidDeviceMessage)\n"
        "                }\n"
        "                guard index > 0 else { continue }",
    ),
    # `render-target-binder-skips-the-disposal-check` was planted here and
    # SURVIVED, and the reason is that the check is REDUNDANT on every reachable
    # input: whichever binder runs, the handle is validated again where it is
    # actually used, and that second validation raises the same
    # ObjectDisposedException with the same message. The check stays because
    # XNA's `Helpers.CheckDisposed` is there and because its ORDER matters --
    # ahead of the device-identity test, so a target that is both disposed and
    # foreign reports the disposal. That ordering needs two devices to observe
    # and CNA runs one game at a time, so the claim is recorded as not yet
    # evidence rather than kept as a mutation nothing can catch.
    # `cube-face-not-carried-to-the-device` -- binding the positive-X face
    # whatever was asked -- was planted here and SURVIVED, for exactly the
    # reason `vertex-offset-not-carried` was withdrawn in Foundation 63:
    # `GetRenderTargets()` reads the managed cache, as XNA's does, and nothing
    # in the public surface observes the face the DEVICE received.
    # `cna_graphics_device_copy_render_targets` does answer it -- the probe
    # reads face 3 back from a cube binding -- but binding a route for a test
    # alone is what docs/native-abi.md forbids. Recorded as not yet evidence;
    # it becomes falsifiable when something renders through a face.
    (
        "same-size-test-ignores-the-pixel-size",
        "IsSameSize comparing extents only, never the format's byte width",
        RTSUPPORT,
        "        left.width == right.width\n"
        "            && left.height == right.height\n"
        "            && left.samples == right.samples\n"
        "            && left.pixelSize == right.pixelSize",
        "        left.width == right.width\n"
        "            && left.height == right.height",
    ),
    (
        "bound-resources-not-released-at-shutdown",
        "the retain cycle Foundation 65 closed, reopened",
        RUNTIME,
        "        releaseBoundResources()\n"
        "        RuntimeRegistry.leave(self)",
        "        RuntimeRegistry.leave(self)",
    ),
    # ---- Foundation 66: TextureCollection and the three bound-state checks ---
    (
        "texture-collections-not-profile-sized",
        "the vertex collections 16 slots long on a profile that has none",
        RUNTIME,
        "        return Int(vertex ? capabilities.maxVertexSamplers : capabilities.maxSamplers)",
        "        return Int(capabilities.maxSamplers)",
    ),
    (
        "texture-slot-cache-not-updated",
        "what was bound not recorded, so Textures[i] answers nil",
        TEXTURECOLLECTION,
        "            slots[resolved] = value\n"
        "        }",
        "        }",
    ),
    (
        "texture-slot-value-checked-after-the-index",
        "an active render target reported as a bad index",
        TEXTURECOLLECTION,
        "        public func SetItem(_ index: Int32, _ value: Texture?) throws {\n"
        "            if let value {",
        "        public func SetItem(_ index: Int32, _ value: Texture?) throws {\n"
        "            _ = try checkedIndex(index)\n"
        "            if let value {",
    ),
    (
        "texture-slot-render-target-test-dropped",
        "the device's own render target accepted as a sampler source",
        TEXTURECOLLECTION,
        "                guard !value.isActiveRenderTarget else {",
        "                guard value.isActiveRenderTarget else {",
    ),
    (
        "vertex-texture-format-rule-applied-to-both-stages",
        "the pixel collection refusing every format the vertex rule refuses",
        TEXTURECOLLECTION,
        "                if textureOffset > 0 {",
        "                if textureOffset >= 0 {",
    ),
    (
        "texture-slot-binder-does-not-reach-the-device",
        "a slot recorded managed-side and never bound natively",
        TEXTURECOLLECTION,
        "            try device.runtimeState.functions.check(\n"
        "                device.runtimeState.functions.graphicsDeviceSetTexture(\n"
        "                    deviceHandle, stage, UInt32(resolved), textureHandle),\n"
        "                operation: \"cna_graphics_device_set_texture\")",
        "            _ = (deviceHandle, textureHandle)",
    ),
    (
        "disposed-texture-stays-in-the-collection",
        "a disposed texture still answered by Textures[i]",
        RESOURCE,
        "                for collection in storage.runtime.liveTextureCollections {\n"
        "                    collection.forget(self)\n"
        "                }",
        "",
    ),
    (
        "active-render-target-flag-never-set",
        "nothing ever marked as the device's target, so three checks never fire",
        DEVICE,
        "            for binding in bindings {\n"
        "                binding.RenderTarget.isActiveRenderTarget = true\n"
        "            }",
        "",
    ),
    (
        "active-render-target-flag-never-cleared",
        "a target that was unbound still refusing to be sampled or written",
        DEVICE,
        "            for previous in runtime.cachedRenderTargetBindings {\n"
        "                previous.RenderTarget.isActiveRenderTarget = false\n"
        "            }",
        "",
    ),
    (
        "resource-in-use-scan-dropped",
        "SetData accepted on a texture bound to a sampler",
        TEXTUREDATA,
        "        for collection in nativeStorage.runtime.liveTextureCollections\n"
        "        where collection.holds(self) {",
        "        for collection in nativeStorage.runtime.liveTextureCollections\n"
        "        where false && collection.holds(self) {",
    ),
    (
        "resource-in-use-scan-ignores-is-setting",
        "GetData refused on a bound texture, which XNA allows",
        TEXTUREDATA,
        "        guard isSetting else { return }",
        "        guard isSetting || true else { return }",
    ),
    (
        "must-resolve-render-target-not-checked-in-transfers",
        "SetData accepted on the device's own render target",
        TEXTUREDATA,
        "        if checksRenderTarget, isActiveRenderTarget {",
        "        if checksRenderTarget, false, isActiveRenderTarget {",
    ),
    # ---- Foundation 67: the Effect core -----------------------------------
    (
        "effect-collection-element-not-cached",
        "a fresh CNA view per index read, so collection[0] !== collection[0]",
        EFFECTSUPPORT,
        "            if let existing = cached[Int(index)] { return existing }",
        "            _ = cached[Int(index)]",
    ),
    # `effect-collection-index-throws-instead-of-nil` -- relaxing the bound from
    # `<` to `<=` -- was planted here and SURVIVED as a NO-OP: one past the end
    # gets past the guard and `get_at` then fails, so the answer is still nil.
    # It is replaced rather than scored, with one that changes the answer.
    (
        "effect-collection-index-clamps-instead-of-nil",
        "an out-of-range index answering element zero rather than null",
        EFFECTSUPPORT,
        "            guard index >= 0, index < count else { return nil }",
        "            guard index >= 0, index < count else {\n"
        "                return count > 0 ? element(at: 0) : nil\n"
        "            }",
    ),
    (
        "effect-name-scan-compares-the-wrong-key",
        "a name lookup that matches nothing, or the wrong element",
        EFFECTSUPPORT,
        "                if key(candidate) == name { return candidate }",
        "                if key(candidate) != name { return candidate }",
    ),
    (
        "effect-string-ignores-the-written-length",
        "a native string read back with its buffer padding attached",
        EFFECTSUPPORT,
        "        let used = buffer.prefix(Int(min(written, bytes))).map { UInt8(bitPattern: $0) }",
        "        let used = buffer.map { UInt8(bitPattern: $0) } + [UInt8(65)]",
    ),
    (
        "effect-pass-apply-skips-the-disposal-check",
        "a pass applying through a disposed effect",
        EFFECTCOLLECTIONS,
        "            _ = try effect.validatedHandle(\"EffectPass.Apply\")",
        "            _ = effect.nativeStorage.handle",
    ),
    (
        "effect-pass-apply-skips-the-current-technique-test",
        "a pass of a non-current technique applying anyway",
        EFFECTCOLLECTIONS,
        "            guard effect.CurrentTechnique === technique else {",
        "            guard effect.CurrentTechnique !== technique else {",
    ),
    (
        "effect-pass-apply-does-not-call-on-apply",
        "the derived hook never firing",
        EFFECTCOLLECTIONS,
        "            try effect.OnApply()\n"
        "            try box.runtime.functions.check(\n"
        "                box.runtime.functions.effectPassApply(",
        "            try box.runtime.functions.check(\n"
        "                box.runtime.functions.effectPassApply(",
    ),
    (
        "effect-pass-apply-does-not-reach-the-route",
        "Apply accepted and discarded rather than handed to CNA",
        EFFECTCOLLECTIONS,
        "                box.runtime.functions.effectPassApply(\n"
        "                    try box.validated(\"EffectPass.Apply\")),\n"
        "                operation: \"cna_effect_pass_apply\")",
        "                CNA_Result(0)),\n"
        "                operation: \"cna_effect_pass_apply\")",
    ),
    (
        "effect-technique-passes-not-cached",
        "a fresh pass collection per read, losing pass identity",
        EFFECTCOLLECTIONS,
        "            if let passes { return passes }",
        "            if let passes, false { return passes }",
    ),
    (
        "effect-current-technique-is-a-third-view",
        "CurrentTechnique answering an object the collection never handed out",
        EFFECT,
        "            let resolved = collection?[name] ?? collection?[Int32(0)]",
        "            let resolved = EffectTechnique(\n"
        "                handle: handle, runtime: nativeStorage.runtime, owner: self)",
    ),
    (
        "effect-techniques-not-cached",
        "a fresh technique collection per read, so Techniques !== Techniques",
        EFFECT,
        "            if let techniques { return techniques }",
        "            if let techniques, false { return techniques }",
    ),
    (
        "effect-code-length-not-a-multiple-of-four",
        "a badly sized effect array accepted and handed to CNA",
        EFFECT,
        "            guard effectCode.count % 4 == 0 else {",
        "            guard effectCode.count % 1 == 0 else {",
    ),
    (
        "effect-empty-code-not-reported-as-null",
        "an empty effect array reaching the native compiler",
        EFFECT,
        "            guard !effectCode.isEmpty else {",
        "            guard effectCode.isEmpty || true else {",
    ),
    (
        "effect-apply-also-calls-on-apply",
        "the derived hook firing twice for one application",
        EFFECT,
        "        internal func apply() throws {\n"
        "            try nativeStorage.runtime.functions.check(",
        "        internal func apply() throws {\n"
        "            try OnApply()\n"
        "            try nativeStorage.runtime.functions.check(",
    ),
    # ---- Foundation 68: the draw family -----------------------------------
    (
        "draw-accepts-zero-primitives",
        "a draw of nothing accepted where XNA raises MustDrawSomething",
        DRAW,
        "        guard primitiveCount > 0 else {",
        "        guard primitiveCount >= 0 else {",
    ),
    (
        "draw-primitive-limit-unchecked",
        "more primitives than the profile allows reaching the device",
        DRAW,
        "        guard primitiveCount <= capabilities.maxPrimitiveCount else {",
        "        guard primitiveCount <= capabilities.maxVertexBufferSize else {",
    ),
    (
        "draw-vertex-count-checked-after-the-primitive-count",
        "a bad numVertices reported as a bad primitiveCount",
        DRAW,
        "        let handle = try validatedHandle(\"GraphicsDevice.DrawIndexedPrimitives\")\n"
        "        guard numVertices > 0 else {",
        "        let handle = try validatedHandle(\"GraphicsDevice.DrawIndexedPrimitives\")\n"
        "        try validatePrimitiveCount(primitiveCount)\n"
        "        guard numVertices > 0 else {",
    ),
    (
        "draw-instance-stream-mask-always-zero",
        "a non-instanced draw accepted with an instanced stream bound",
        DRAW,
        "        for (index, binding) in runtimeState.cachedVertexBufferBindings.enumerated()\n"
        "        where binding.InstanceFrequency != 0 {",
        "        for (index, binding) in runtimeState.cachedVertexBufferBindings.enumerated()\n"
        "        where binding.InstanceFrequency == 0 {",
    ),
    (
        "instanced-draw-accepts-a-uniform-stream-set",
        "an instanced draw accepted with every stream instanced, or none",
        DRAW,
        "        guard mask != 0, !allOne else {",
        "        guard mask == 0 || allOne || true else {",
    ),
    (
        "user-draw-empty-array-not-reported-as-null",
        "an empty user vertex array falling through to the offset test",
        DRAW,
        "        guard count > 0 else {\n"
        "            throw CNAArgumentNullException(\n"
        "                paramName: \"vertexData\",",
        "        guard count >= 0 else {\n"
        "            throw CNAArgumentNullException(\n"
        "                paramName: \"vertexData\",",
    ),
    (
        "user-draw-offset-not-bounded-by-the-array",
        "a vertex offset past the end of the caller's array accepted",
        DRAW,
        "        guard vertexOffset >= 0, Int(vertexOffset) < count else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"vertexOffset\",",
        "        guard vertexOffset >= 0, Int(vertexOffset) <= count else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"vertexOffset\",",
    ),
    (
        "user-draw-element-count-test-is-signed",
        "an overflowing offset plus element count slipping past an unsigned test",
        DRAW,
        "        let end = UInt32(bitPattern: elements) &+ UInt32(bitPattern: offset)\n"
        "        return end <= UInt32(clamping: count)",
        "        let end = elements &+ offset\n"
        "        return end <= Int32(clamping: count)",
    ),
    (
        "user-draw-element-count-not-consulted",
        "a primitive count the array cannot supply accepted",
        DRAW,
        "        guard Microsoft.Xna.Framework.Graphics.GraphicsDevice.arrayHolds(\n"
        "            offset: vertexOffset, elements: needed, count: count) else {",
        "        guard Microsoft.Xna.Framework.Graphics.GraphicsDevice.arrayHolds(\n"
        "            offset: vertexOffset, elements: 0, count: count) else {",
    ),
    (
        "user-indexed-draw-allows-32-bit-indices-on-reach",
        "32-bit user indices accepted where Reach refuses them",
        DRAW,
        "            guard capabilities.indexElementSize32 else {",
        "            guard !capabilities.indexElementSize32 else {",
    ),
    (
        "draw-does-not-reach-the-route",
        "DrawPrimitives accepted and discarded rather than handed to CNA",
        DRAW,
        "            runtimeState.functions.graphicsDeviceDrawPrimitives(\n"
        "                handle, UInt32(bitPattern: primitiveType.rawValue),\n"
        "                startVertex, primitiveCount),",
        "            CNA_Result(0),",
    ),
    (
        "primitive-type-not-carried-to-the-route",
        "every draw submitted as a triangle list whatever was asked",
        DRAW,
        "                handle, UInt32(bitPattern: primitiveType.rawValue),\n"
        "                startVertex, primitiveCount),",
        "                handle, 0,\n"
        "                startVertex, primitiveCount),",
    ),

    # ---- Foundation 69: the stock effects --------------------------------
    (
        "light-getter-reads-the-wrong-cache",
        "a light's getters answering another field than the one XNA reads",
        LIGHT,
        "        public var DiffuseColor: Microsoft.Xna.Framework.Vector3 { cachedDiffuseColor }",
        "        public var DiffuseColor: Microsoft.Xna.Framework.Vector3 { cachedSpecularColor }",
    ),
    (
        "disabling-a-light-clears-its-cache",
        "set_Enabled(false) clearing what the light remembers, not just the parameter",
        LIGHT,
        "                try diffuseColorParam?.SetValue(\n"
        "                    Microsoft.Xna.Framework.Vector3.Zero)",
        "                cachedDiffuseColor = Microsoft.Xna.Framework.Vector3.Zero\n"
        "                try diffuseColorParam?.SetValue(\n"
        "                    Microsoft.Xna.Framework.Vector3.Zero)",
    ),
    (
        "enabled-setter-is-not-a-no-op",
        "set_Enabled re-pushing when the value has not changed",
        LIGHT,
        "            guard enabled != value else { return }",
        "            if enabled == value && false { return }",
    ),
    (
        "diffuse-written-while-disabled",
        "set_DiffuseColor writing its parameter regardless of Enabled",
        LIGHT,
        "            if enabled { try diffuseColorParam?.SetValue(value) }\n"
        "            cachedDiffuseColor = value",
        "            try diffuseColorParam?.SetValue(value)\n"
        "            cachedDiffuseColor = value",
    ),
    (
        "direction-written-only-while-enabled",
        "set_Direction consulting Enabled, which XNA's does not",
        LIGHT,
        "            try directionParam?.SetValue(value)\n"
        "            cachedDirection = value",
        "            if enabled { try directionParam?.SetValue(value) }\n"
        "            cachedDirection = value",
    ),
    (
        "clone-runs-the-setters",
        "the clone constructor assigning through the setters, so a clone writes parameters",
        LIGHT,
        "                enabled = cloneSource.enabled\n"
        "                cachedDirection = cloneSource.cachedDirection",
        "                try SetEnabled(cloneSource.enabled)\n"
        "                cachedDirection = cloneSource.cachedDirection",
    ),
    (
        "default-lighting-table-altered",
        "one component of EffectHelpers' default lighting table changed",
        STOCKSUPPORT,
        "            Microsoft.Xna.Framework.Vector3(-0.5265408, -0.5735765, -0.6275069))",
        "            Microsoft.Xna.Framework.Vector3(-0.5265408, -0.5735765, -0.6275))",
    ),
    (
        "default-lighting-specular-shared",
        "light1's specular set from its diffuse instead of Zero",
        STOCKSUPPORT,
        "        try light1.SetSpecularColor(Microsoft.Xna.Framework.Vector3.Zero)",
        "        try light1.SetSpecularColor(\n"
        "            Microsoft.Xna.Framework.Vector3(0.9647059, 0.7607844, 0.4078432))",
    ),
    (
        "default-lighting-leaves-lights-off",
        "EnableDefaultLighting setting the three lights' values but not enabling them",
        STOCKSUPPORT,
        "        try light0.SetEnabled(true)",
        "        try light0.SetEnabled(false)",
    ),
    (
        "default-lighting-does-not-enable-lighting",
        "BasicEffect.EnableDefaultLighting leaving LightingEnabled alone",
        BASICEFFECT,
        "            LightingEnabled = true\n"
        "            AmbientLightColor = try Microsoft.Xna.Framework.Graphics",
        "            AmbientLightColor = try Microsoft.Xna.Framework.Graphics",
    ),
    (
        "effect-lights-not-cached",
        "an effect fetching a fresh light view per call, so its lights lose identity",
        BASICEFFECT,
        "        public var DirectionalLight0: DirectionalLight? { light0 }",
        "        public var DirectionalLight0: DirectionalLight? {\n"
        "            try? Microsoft.Xna.Framework.Graphics.stockEffectLights(\n"
        "                handle: try validatedHandle(\"BasicEffect.DirectionalLight0\"),\n"
        "                runtime: nativeStorage.runtime).0\n"
        "        }",
    ),
    (
        "always-lit-effect-accepts-false",
        "SkinnedEffect agreeing to disable lighting instead of refusing",
        SKINNED,
        "            guard !value else { return }\n"
        "            throw CNANotSupportedException(",
        "            guard value else { return }\n"
        "            throw CNANotSupportedException(",
    ),
    (
        "always-lit-effect-names-the-wrong-type",
        "CantDisableLighting formatted with the other always-lit effect's name",
        SKINNED,
        "                    .replacingOccurrences(of: \"{0}\", with: \"SkinnedEffect\"))",
        "                    .replacingOccurrences(of: \"{0}\", with: \"EnvironmentMapEffect\"))",
    ),
    (
        "max-bones-off-by-one",
        "SetBoneTransforms accepting one more bone than MaxBones",
        SKINNED,
        "            guard boneTransforms.count <= Int(SkinnedEffect.MaxBones) else {",
        "            guard boneTransforms.count <= Int(SkinnedEffect.MaxBones) + 1 else {",
    ),
    (
        "empty-bone-array-accepted",
        "SetBoneTransforms accepting an empty array instead of refusing it as null",
        SKINNED,
        "            guard !boneTransforms.isEmpty else {",
        "            guard boneTransforms.count >= 0 else {",
    ),
    # `bone-reader-skips-the-fourth-diagonal` was planted here and WITHDRAWN,
    # not scored. It deleted `matrix.M44 = 1` from GetBoneTransforms, which
    # reproduces the loop at IL_0051; `build-probe/f69_bones.c` then measured
    # that CNA already answers `m44 = 1` for a bone written with `m44 = 0`,
    # keeping the rest of the fourth column. The managed restoration is
    # therefore invisible on this artifact and no test can be written that the
    # mutation would fail. The reason is recorded in SkinnedEffect.swift where
    # the line stands.
    (
        "weights-per-vertex-accepts-three",
        "WeightsPerVertex admitting a value XNA refuses",
        SKINNED,
        "            guard value == 1 || value == 2 || value == 4 else {",
        "            guard value == 1 || value == 2 || value == 3 || value == 4 else {",
    ),
    (
        "weights-per-vertex-writes-before-testing",
        "the refused value stored anyway, so the property changes on a throw",
        SKINNED,
        "            guard value == 1 || value == 2 || value == 4 else {\n"
        "                throw CNAArgumentOutOfRangeException(",
        "            state.weightsPerVertex = value\n"
        "            guard value == 1 || value == 2 || value == 4 else {\n"
        "                throw CNAArgumentOutOfRangeException(",
    ),
    (
        "dual-texture-layers-collapse",
        "Texture2 reading and writing layer zero, so the two properties are one",
        DUALTEXTURE,
        "        private static let secondLayer: UInt32 = 1",
        "        private static let secondLayer: UInt32 = 0",
    ),
    (
        "alpha-default-not-one",
        "the cached alpha starting at something other than what the constructor sets",
        STOCKSUPPORT,
        "        var alpha: Float = 1",
        "        var alpha: Float = 0",
    ),
    (
        "fog-end-default-not-one",
        "fogEnd starting at zero, which is not what the constructor's IL sets",
        STOCKSUPPORT,
        "        var fogEnd: Float = 1",
        "        var fogEnd: Float = 0",
    ),
    (
        "weights-per-vertex-default-not-four",
        "WeightsPerVertex starting at a value XNA's field default is not",
        STOCKSUPPORT,
        "        var weightsPerVertex: Int32 = 4",
        "        var weightsPerVertex: Int32 = 2",
    ),
    (
        "setter-does-not-mark-dirty",
        "an assignment that never reaches the device because it is not recorded",
        BASICEFFECT,
        "            set { state.alpha = newValue; state.mark(.alpha) }",
        "            set { state.alpha = newValue }",
    ),
    (
        "clone-loses-the-cached-state",
        "the clone constructor starting from defaults instead of the source's state",
        BASICEFFECT,
        "            state = cloneSource.state\n"
        "            texture = cloneSource.texture",
        "            texture = cloneSource.texture",
    ),
    (
        "texture-identity-not-tracked",
        "an assigned texture forgotten, so the getter answers nil for a set texture",
        BASICEFFECT,
        "                nativeStorage.runtime.functions.basicEffectSetTexture($0, $1)\n"
        "            }\n"
        "            texture = value",
        "                nativeStorage.runtime.functions.basicEffectSetTexture($0, $1)\n"
        "            }",
    ),
    (
        "alpha-test-effect-claims-lighting",
        "an unlit effect conforming to IEffectLights, which XNA's does not",
        ALPHATEST,
        "    open class AlphaTestEffect: Effect, IEffectMatrices, IEffectFog {",
        "    open class AlphaTestEffect: Effect, IEffectMatrices, IEffectFog, IEffectLights {",
    ),
    (
        "environment-map-lighting-reads-a-field",
        "the always-on getter answering stored state instead of true",
        ENVMAP,
        "        public var LightingEnabled: Bool { true }",
        "        public var LightingEnabled: Bool { state.lightingEnabled }",
    ),

    # ---- Foundation 70: SpriteFont and DrawString -------------------------
    (
        "measure-does-not-clamp-the-first-bearing",
        "the first glyph of a line adding a negative left bearing instead of clamping it",
        FONT,
        "                if firstOfLine {\n"
        "                    left = max(left, 0)",
        "                if firstOfLine {\n"
        "                    left = min(left, 0)",
    ),
    (
        "measure-clamps-every-bearing",
        "the clamp applied to every glyph, not only the first of a line",
        FONT,
        "                var left = glyph.kerning.x\n"
        "                if firstOfLine {",
        "                var left = max(glyph.kerning.x, 0)\n"
        "                if firstOfLine {",
    ),
    (
        "measure-omits-the-inter-glyph-spacing",
        "Spacing never added, so every string measures as if spacing were zero",
        FONT,
        "                    size.X += spacing + rightBearing",
        "                    size.X += rightBearing",
    ),
    (
        "measure-omits-the-trailing-bearing",
        "the last glyph's right bearing dropped from the width",
        FONT,
        "            size.X += max(rightBearing, 0)\n"
        "            size.Y += Float(lines * lineSpacing)",
        "            size.Y += Float(lines * lineSpacing)",
    ),
    (
        "measure-does-not-clamp-the-trailing-bearing",
        "a negative trailing bearing shrinking the measured width",
        FONT,
        "            size.X += max(rightBearing, 0)\n"
        "            size.Y += Float(lines * lineSpacing)",
        "            size.X += rightBearing\n"
        "            size.Y += Float(lines * lineSpacing)",
    ),
    (
        "measure-counts-the-carriage-return",
        "'\\r' measured as a glyph instead of skipped before every other test",
        FONT,
        "                if unit == 13 { continue }          // '\\r'",
        "                if unit == 13 && false { continue } // '\\r'",
    ),
    (
        "measure-height-uses-the-glyph-bounds",
        "the height taken from the atlas rectangle rather than the cropping one",
        FONT,
        "                size.Y = max(size.Y, Float(glyph.cropping.height))",
        "                size.Y = max(size.Y, Float(glyph.glyph_bounds.height))",
    ),
    (
        "measure-forgets-the-widest-line",
        "a multi-line measurement answering only the last line's width",
        FONT,
        "            size.X = max(size.X, widest)",
        "            size.X = size.X",
    ),
    (
        "measure-does-not-add-the-line-heights",
        "every extra line costing nothing, so one line and five measure the same",
        FONT,
        "            size.Y += Float(lines * lineSpacing)",
        "            size.Y += Float(0 * lineSpacing)",
    ),
    (
        "measure-of-an-empty-string-is-not-zero",
        "an empty string measured as one empty line instead of Vector2.Zero",
        FONT,
        "            guard !units.isEmpty else { return .Zero }",
        "            guard units.count < 0 else { return .Zero }",
    ),
    (
        "character-search-misses-the-ends",
        "a binary search that never examines the first or last character",
        FONT,
        "            var low = 0\n"
        "            var high = characterMap.count - 1",
        "            var low = 1\n"
        "            var high = characterMap.count - 2",
    ),
    # `character-fallback-recurses-without-a-guard` was planted here and
    # WITHDRAWN. It removed GetIndexForCharacter's `defaultCharacter != unit`
    # guard, which matters only for a font whose DEFAULT character is itself
    # absent from the glyph table -- the one state in which the fallback
    # recurses for ever. `build-probe/f70_default.c` measures that CNA refuses
    # to build such a font ("defaultCharacter is not present in characters"),
    # and `cna_sprite_font_set_default_character` refuses the same change
    # afterwards. No route this projection has can reach the state, so no test
    # can be written that the mutation would fail. The guard stays: it is
    # XNA's, and it is the difference between reporting a character and
    # hanging.
    (
        "character-refusal-drops-its-parameter-name",
        "GetIndexForCharacter's refusal losing the ParamName XNA gives it",
        FONT,
        "                message: SpriteFont.characterNotInFontMessage(unit),\n"
        "                paramName: \"character\")",
        "                message: SpriteFont.characterNotInFontMessage(unit))",
    ),
    (
        "default-character-refusal-gains-a-parameter-name",
        "set_DefaultCharacter's refusal naming a parameter, which XNA's does not",
        FONT,
        "                    throw CNAArgumentException(\n"
        "                        message: SpriteFont.characterNotInFontMessage(value))",
        "                    throw CNAArgumentException(\n"
        "                        message: SpriteFont.characterNotInFontMessage(value),\n"
        "                        paramName: \"value\")",
    ),
    (
        "default-character-accepts-anything",
        "a fallback character the font does not have accepted instead of refused",
        FONT,
        "                guard characterMap.contains(value) else {",
        "                guard characterMap.contains(value) || true else {",
    ),
    (
        "default-character-written-before-the-test",
        "the refused fallback stored anyway, so the property changes on a throw",
        FONT,
        "            if let value {\n"
        "                guard characterMap.contains(value) else {",
        "            defaultCharacter = value\n"
        "            if let value {\n"
        "                guard characterMap.contains(value) else {",
    ),
    (
        "character-message-hex-not-padded",
        "the numeric placeholder formatted without XNA's four-digit hex width",
        FONT,
        "                    of: \"{1:x4}\", with: String(format: \"%04x\", Int(value)))",
        "                    of: \"{1:x4}\", with: String(format: \"%x\", Int(value)))",
    ),
    (
        "characters-collection-rebuilt-per-call",
        "Characters answering a fresh collection each time, so it has no identity",
        FONT,
        "            if let charactersView { return charactersView }",
        "            if let charactersView, false { return charactersView }",
    ),
    (
        "layout-change-never-reaches-the-device",
        "an assigned LineSpacing that no draw ever pushes",
        FONT,
        "            if pendingLineSpacing {",
        "            if pendingLineSpacing && false {",
    ),
    # `layout-flush-does-not-clear-its-flag` was planted here and REPLACED,
    # not scored: leaving the flag set re-pushes the SAME value on the next
    # draw, and no answer changes. The replacement crosses the two flags,
    # which does change one.
    (
        "layout-flush-crosses-its-two-flags",
        "Spacing pushed under LineSpacing's flag, so setting one alone is lost",
        FONT,
        "            if pendingSpacing {",
        "            if pendingLineSpacing {",
    ),
    (
        "draw-string-skips-the-begin-check",
        "a draw outside a begin/end pair reaching CNA instead of reporting XNA's rule",
        BATCH,
        "            guard inBeginEndPair else {\n"
        "                throw CNAInvalidOperationException(\n"
        "                    message: beginMustBeCalledBeforeDrawMessage)\n"
        "            }\n"
        "            // Every character has to be in the font",
        "            // Every character has to be in the font",
    ),
    (
        "draw-string-refuses-an-empty-string",
        "an empty string refused outside a pair, which XNA accepts",
        BATCH,
        "            let drawsAGlyph = units.contains { $0 != 13 && $0 != 10 }",
        "            let drawsAGlyph = !units.isEmpty",
    ),
    (
        "draw-string-does-not-validate-its-characters",
        "an unknown character forwarded to CNA instead of reported the way XNA does",
        BATCH,
        "            _ = try spriteFont.measure(units)",
        "            _ = units",
    ),
    # `draw-string-uniform-scale-widened-wrongly` was planted here and
    # WITHDRAWN, for Foundation 65's reason and Foundation 68's: a submitted
    # sprite is not readable back. The uniform-scale overload widens one scale
    # into `Vector2(scale, scale)`, and a mutation to `Vector2(scale, 1)`
    # produces a command CNA accepts exactly as readily -- there is no query
    # for what a batch was given, no pixel readback to see the result, and no
    # scale the route refuses. The widening is asserted by reading, not by
    # testing, and this records that it is.

    # ---- Foundation 71: System.Text.StringBuilder -------------------------
    (
        "builder-length-counts-characters",
        "Length counting Swift Characters instead of UTF-16 code units",
        BUILDER,
        "    public var Length: Int32 { Int32(units.count) }",
        "    public var Length: Int32 { Int32(String(decoding: units, as: UTF16.self).count) }",
    ),
    (
        "builder-indexer-getter-throws-the-setter-s-exception",
        "get_Chars raising ArgumentOutOfRangeException where XNA raises a bare IndexOutOfRangeException",
        BUILDER,
        "            throw CNAIndexOutOfRangeException()",
        "            throw CNAArgumentOutOfRangeException(paramName: \"index\")",
    ),
    (
        "builder-indexer-setter-throws-the-getter-s-exception",
        "set_Chars raising IndexOutOfRangeException where XNA raises ArgumentOutOfRangeException",
        BUILDER,
        "    public func SetItem(_ index: Int32, _ value: UInt16) throws {\n"
        "        guard index >= 0, index < Length else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"index\", message: CNAStringBuilder.indexOutOfRange)",
        "    public func SetItem(_ index: Int32, _ value: UInt16) throws {\n"
        "        guard index >= 0, index < Length else {\n"
        "            throw CNAIndexOutOfRangeException()",
    ),
    (
        "builder-set-length-does-not-pad",
        "growing the length leaving the new units unwritten instead of NUL",
        BUILDER,
        "            units.append(contentsOf: repeatElement(0, count: wanted - units.count))",
        "            units.append(contentsOf: repeatElement(32, count: wanted - units.count))",
    ),
    (
        "builder-set-length-refusals-transposed",
        "set_Length reporting the ceiling message for a negative value and vice versa",
        BUILDER,
        "                paramName: \"value\", message: CNAStringBuilder.negativeLength)\n"
        "        }\n"
        "        guard value <= maxCapacity else {",
        "                paramName: \"value\", message: CNAStringBuilder.smallCapacity)\n"
        "        }\n"
        "        guard value <= maxCapacity else {",
    ),
    (
        "builder-append-ignores-max-capacity",
        "the shared growth guard removed, so a builder grows past MaxCapacity",
        BUILDER,
        "        guard required <= Int(maxCapacity) else {",
        "        guard required <= Int(maxCapacity) || true else {",
    ),
    # `builder-repeat-count-written-before-testing` was planted here and
    # REPLACED, not scored: it moved the append before the guard, and with a
    # NEGATIVE count there is nothing to append -- `max(0, -1)` writes zero
    # elements and the guard still throws, so no answer changes. The
    # replacement refuses a count XNA accepts, which does.
    (
        "builder-repeat-count-refuses-zero",
        "Append(char, 0) refused, though XNA's `>= 0` accepts it and writes nothing",
        BUILDER,
        "        guard repeatCount >= 0 else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"repeatCount\", message: CNAStringBuilder.negativeCount)",
        "        guard repeatCount > 0 else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"repeatCount\", message: CNAStringBuilder.negativeCount)",
    ),
    (
        "builder-append-line-uses-the-host-separator",
        "AppendLine emitting the host's newline instead of the Windows CLR's",
        BUILDER,
        "    internal static let newLine = \"\\r\\n\"",
        "    internal static let newLine = \"\\n\"",
    ),
    (
        "builder-insert-refuses-the-end",
        "Insert at Length refused, though XNA's unsigned bound accepts it",
        BUILDER,
        "        guard index >= 0, index <= Length else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"index\", message: CNAStringBuilder.indexOutOfRange)\n"
        "        }\n"
        "        let inserted = Array(value.utf16)",
        "        guard index >= 0, index < Length else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"index\", message: CNAStringBuilder.indexOutOfRange)\n"
        "        }\n"
        "        let inserted = Array(value.utf16)",
    ),
    (
        "builder-remove-names-start-index-for-its-range",
        "Remove's range refusal naming startIndex, where XNA's names index",
        BUILDER,
        "        guard length <= Length - startIndex else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"index\", message: CNAStringBuilder.indexOutOfRange)",
        "        guard length <= Length - startIndex else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"startIndex\", message: CNAStringBuilder.indexOutOfRange)",
    ),
    (
        "builder-clear-drops-the-capacity",
        "Clear rebuilding the store, so the capacity it exists to keep is lost",
        BUILDER,
        "        units.removeAll(keepingCapacity: true)\n"
        "        return self",
        "        units.removeAll(keepingCapacity: true)\n"
        "        capacity = CNAStringBuilder.defaultCapacity\n"
        "        return self",
    ),
    (
        "builder-to-string-range-refusals-reordered",
        "ToString testing its length before its start index, so a doubly wrong call misreports",
        BUILDER,
        "        guard startIndex >= 0 else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"startIndex\", message: CNAStringBuilder.startIndex)\n"
        "        }\n"
        "        guard startIndex <= Length else {",
        "        guard length >= 0 else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"length\", message: CNAStringBuilder.negativeLength)\n"
        "        }\n"
        "        guard startIndex >= 0 else {\n"
        "            throw CNAArgumentOutOfRangeException(\n"
        "                paramName: \"startIndex\", message: CNAStringBuilder.startIndex)\n"
        "        }\n"
        "        guard startIndex <= Length else {",
    ),
    (
        "builder-zero-capacity-ignores-the-ceiling",
        "a zero capacity taking sixteen even when MaxCapacity is smaller",
        BUILDER,
        "            ? min(CNAStringBuilder.defaultCapacity, maxCapacity)",
        "            ? CNAStringBuilder.defaultCapacity",
    ),
    (
        "builder-must-be-positive-not-formatted",
        "the one formatted mscorlib message left with its placeholder unfilled",
        BUILDER,
        "                message: CNAStringBuilder.mustBePositive\n"
        "                    .replacingOccurrences(of: \"{0}\", with: \"capacity\"))",
        "                message: CNAStringBuilder.mustBePositive)",
    ),
    (
        "measure-string-builder-reads-a-copy",
        "MeasureString(StringBuilder) measuring a rendered String, so code units are lost",
        FONT,
        "            try measure(text.codeUnits)",
        "            try measure(Array(text.ToString().utf16).filter { $0 < 0xD800 })",
    ),
]

CONTENT_READER_MUTATION_NAMES = {
    "binary-reader-endian-swap",
    "binary-reader-string-length-off-by-one",
    "content-reader-index-zero-based",
    "content-reader-skips-shared-resource",
    "content-reader-external-reference-not-normalized",
    "generic-reader-target-type-wrong",
    "reader-creator-registry-wrong-name",
    "content-manager-readasset-bypasses-cache",
    "content-manager-openstream-ignores-root",
    "resource-content-manager-bypasses-resource-manager",
    "resource-manager-wrong-key-lookup",
}


def restore_on_termination() -> None:
    """Turn SIGTERM and SIGHUP into an exception, so `finally` still runs.

    Every mutation is undone in a `finally`, which a normal exit or a Ctrl-C
    honours -- but a `timeout`, a killed background job or a closed terminal
    sends SIGTERM, whose default handler terminates the process outright and
    leaves the planted defect in the working tree. That has happened twice, and
    both times the next run reported the stranded mutation as an unrelated
    failure somewhere else entirely.

    Raising `KeyboardInterrupt` from the handler puts SIGTERM on the same
    footing as Ctrl-C: the `finally` unwinds, the tree is restored, and the
    caller still sees a nonzero exit. SIGKILL cannot be caught, which is why
    the site-staleness precondition above exists as the backstop.
    """
    def handler(signum: int, _frame: object) -> None:
        raise KeyboardInterrupt(f"terminated by signal {signum}")
    for received in (signal.SIGTERM, signal.SIGHUP):
        signal.signal(received, handler)


def require_native_library() -> str | None:
    """The runtime suites skip without an explicit native library.

    Sixteen of the mutations below are caught only by tests that start a CNA
    runtime. With `CNA_NATIVE_LIBRARY` unset those tests SKIP rather than fail,
    every one of those mutations comes back SURVIVED, and the gate reports a
    coverage loss as a projection defect. Refusing to run is the honest
    behaviour.
    """
    selected = os.environ.get("CNA_NATIVE_LIBRARY")
    if not selected:
        return "CNA_NATIVE_LIBRARY is not set"
    if not Path(selected).is_file():
        return f"CNA_NATIVE_LIBRARY={selected!r} is not a file"
    return None


# A mutated suite that HANGS is still a suite that does not pass, but without a
# deadline it stalls the whole run. The deadline is generous on purpose. It is
# not a performance budget; it is the line between "slow" and "never", and a
# mutation that trips it is scored as caught-by-hang rather than silently as
# caught, because the two are different facts about the projection.
TEST_TIMEOUT_SECONDS = 600


# Most mutations run the complete suite. This one historically invalidated a
# vertex type used by hundreds of later tests: its intended assertion failed,
# then an unrelated graphics teardown hung. The direct test below observes the
# exact private registration value before native allocation and gives the
# mutation a failure verdict without converting an already-caught defect into
# a ten-minute HUNG verdict.
MUTATION_TEST_FILTERS = {
    "dynamic-islooped-answers-true":
        "Foundation90DynamicSoundTests.testRedeclaredIsLoopedAccessorsPreserveDynamicBehavior",
    "serialization-constructor-corrupts-hresult":
        "PureValueTests.testXnaSerializationConstructorsRestoreObservableBaseState",
    "backbuffer-readback-skips-profile-refusal":
        "Foundation48ClearTests.testReachRefusesBackBufferReadbackBeforeArrayValidation",
    "storage-stream-setlength-adds-one":
        "Foundation102StorageTests.testDuplexStorageStreamWritesSeeksReadsAndResizes",
    "from-type-size-test-reads-the-wrong-size":
        "Foundation60BufferTests.testFromTypeAcceptsItsMatchingRegisteredSize",
    "nonempty-element-array-erased":
        "Foundation43VertexDeclarationTests.testTheStrideOnlyConstructorComputesTheStride",
    "vertex-element-quadruple-transposed":
        "Foundation44VertexTypeTests.testEachStaticDeclarationHasTheElementsTheCctorBuilds",
    "design-invalid-component-count-accepted":
        "Foundation105DesignTests.testInvalidStringsUseThePinnedXnaMessage",
    "design-culture-list-separator-ignored":
        "Foundation105DesignTests.testCultureControlsBothDecimalAndListSeparators",
    "design-instance-descriptor-not-invoked":
        "Foundation105DesignTests.testInstanceDescriptorCarriesConstructorIdentityAndInvokes",
    "design-property-order-reversed":
        "Foundation105DesignTests.testPropertyDescriptionsAreStableAndReadable",
}


# Compilation parallelism for the inner `swift test`.
#
# The DEFAULT follows the openeggbert build rules, whose Rule 5 says
# parallelism is not capped and that the old -j3 ceiling existed for a cooling
# fault repaired on 2026-08-22. This tool is shared, so its default is the
# project's, not one session's.
#
# `--jobs` exists because a full run is a few hundred rebuilds back to back and
# is one of the "heavy jobs" Rule 5 warns against running concurrently: a
# desktop session on this host froze under an unrestricted one on 2026-09-05
# and had to be restarted. A session told to cap itself passes `--jobs 3`,
# which costs roughly nothing in wall clock and leaves the machine usable.
DEFAULT_JOBS = os.cpu_count() or 4


# A line XCTest writes for a failed assertion or an uncaught error.
TEST_FAILURE_MARKER = re.compile(r"^.*: error: .*Tests\.", re.M)


def _decoded(stream: object) -> str:
    if isinstance(stream, bytes):
        return stream.decode("utf-8", "replace")
    return stream if isinstance(stream, str) else ""


def run_tests(swift_test: str,
              jobs: int = DEFAULT_JOBS,
              test_filter: str | None = None) -> tuple[int, bool, bool]:
    """(exit status, whether it ran out of time, whether a test failed).

    The third value exists because a run can do BOTH. A mutation is often
    caught by dozens of assertions and only then reaches some later test that
    hangs -- `from-type-size-test-reads-the-wrong-size` failed 28 assertions
    before hanging at test 346 of 809. Reporting that as HUNG threw away the
    verdict and left a caught mutation looking unmeasured, so the output is
    scanned for failures even when the deadline fires.
    """
    try:
        command = [swift_test, "-j", str(jobs)]
        if test_filter is not None:
            command.extend(["--filter", test_filter])
        result = subprocess.run(
            command,
            cwd=ROOT, text=True, capture_output=True,
            timeout=TEST_TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired as expired:
        # The child is killed by `subprocess.run`, but the test BINARY it
        # spawned is not its process group's only member and outlives it.
        subprocess.run(["pkill", "-9", "-f", "CNAPackageTests.xctest"],
                       capture_output=True)
        partial = _decoded(expired.stdout) + _decoded(expired.stderr)
        return 1, True, bool(TEST_FAILURE_MARKER.search(partial))
    failed = bool(TEST_FAILURE_MARKER.search(result.stdout + result.stderr))
    return result.returncode, False, failed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--swift-test", default="swift-test")
    parser.add_argument(
        "--changed-since", metavar="GIT_REF",
        help="run only the mutations whose target file changed since GIT_REF. "
             "A full run rebuilds and relinks the test binary once per "
             "mutation -- measured at ~1.3 MB/s, about 11 GB for all 290 -- "
             "and re-verifies mutations nothing has touched. This selects the "
             "ones that can actually have changed verdict.")
    parser.add_argument(
        "--jobs", type=int, default=DEFAULT_JOBS,
        help="compilation parallelism for the inner swift test "
             f"(default {DEFAULT_JOBS})")
    # Running one milestone's own mutations, while the STALENESS check below
    # still runs over all of them.
    #
    # The full run is what the handoff repeats; a milestone that has just
    # written twenty-seven of them needs to see those twenty-seven fail and
    # be fixed, and paying eighty minutes per iteration to learn it is how a
    # mutation ends up merged unrun. The filter narrows only which mutations
    # are PLANTED -- every site is still checked for staleness, so a selective
    # run cannot hide a mutation whose site has drifted away.
    # Auditing the tree for a mutation someone committed.
    #
    # A mutation is applied to the WORKING TREE while its test runs, so
    # anything that reads the tree during a run sees it -- and `git add -A`
    # during a run commits it. That is not hypothetical: this harness's own
    # `texture-size-limit-not-checked` reached a commit that way, weakening a
    # profile bound by four, and it was found by accident when the run was
    # killed and left the file's *repair* as the visible diff.
    #
    # "Do not commit during a run" is a rule, and rules of that shape get
    # broken. This is the mechanical version, and it is cheap: every
    # mutation's replacement text is a string this file already holds.
    parser.add_argument(
        "--audit-tree", action="store_true",
        help="report any mutation's replacement text present in the working "
             "tree, and change nothing")
    parser.add_argument(
        "--only", default=None,
        help="plant only the mutations whose name contains one of these "
             "comma-separated substrings")
    parser.add_argument(
        "--output", type=Path, default=None,
        help="write a full pass's outcome as JSON, so the caught and survivor "
             "counts reach the documents from a file rather than from a "
             "terminal line someone retyped. Ignored for --only runs.")
    args = parser.parse_args()

    if args.audit_tree:
        planted = []
        for name, _description, path, old_text, new_text in MUTATIONS:
            if not path.exists():
                continue
            text = path.read_text(encoding="utf-8")
            # The replacement present AND the original absent is what
            # distinguishes a planted mutation from a replacement string that
            # merely happens to occur -- several are substrings of ordinary
            # code.
            if new_text in text and old_text not in text:
                planted.append((name, path))
        print(f"PROJECTION_MUTATION_TREE_AUDIT={len(MUTATIONS)} "
              f"PLANTED={len(planted)}")
        for name, path in planted:
            print(f"  PLANTED {name}: {path} holds this mutation's replacement "
                  "text and not its original")
        return 1 if planted else 0

    tree_lock = acquire_tree_lock("projection_mutations")
    if tree_lock is None:
        print("MUTATION_GATE=BUSY — another mutation harness holds "
              f"{TREE_LOCK.name}; these two gates cannot share a working tree")
        return 1


    restore_on_termination()

    problem = require_native_library()
    if problem is not None:
        print(f"PROJECTION_MUTATION_PRECONDITION=FAILED — {problem}")
        return 1

    # Derived from MUTATIONS rather than listed beside it. The list used to be
    # written out by hand and drifted the moment Foundation 60 added mutations
    # in three new files: the run died on a KeyError instead of reporting a
    # stale site, which is the one failure this pre-check exists to avoid. A
    # file with no mutation does not need reading, and a file with one cannot
    # be forgotten.
    originals = {path: path.read_text(encoding="utf-8")
                 for path in {item[2] for item in MUTATIONS}}

    # Every mutation site is checked BEFORE the baseline runs. A site that has
    # drifted is reported as a stale gate rather than as a survivor forty
    # minutes later, and a run that would have been wasted is not started.
    # Three sites had drifted at least once when the code they aimed at was
    # legitimately edited, and each cost a full run to discover.
    stale = [
        f"{name}: the mutation site occurs {originals[path].count(old)} times, not once"
        for name, _description, path, old, _new in MUTATIONS
        if originals[path].count(old) != 1
    ]
    if stale:
        print(f"PROJECTION_MUTATION_SITES=STALE COUNT={len(stale)}")
        for item in stale:
            print(f"  STALE {item}")
        return 1

    baseline, baseline_hung, _ = run_tests(args.swift_test, args.jobs)
    if baseline != 0:
        print("PROJECTION_MUTATION_BASELINE="
              + ("HUNG — the unmutated tree did not finish in "
                 f"{TEST_TIMEOUT_SECONDS}s" if baseline_hung
                 else "RED — the unmutated tree already fails"))
        return 1

    wanted = None if args.only is None else [
        part for part in args.only.split(",") if part
    ]
    selected = [
        item for item in MUTATIONS
        if wanted is None or any(part in item[0] for part in wanted)
    ]
    if args.only is not None and not selected:
        print(f"PROJECTION_MUTATIONS=NONE no mutation name contains {args.only!r}")
        return 1

    if args.changed_since:
        changed = subprocess.run(
            ["git", "diff", "--name-only", args.changed_since, "--"],
            cwd=ROOT, text=True, capture_output=True)
        if changed.returncode != 0:
            print(f"PROJECTION_MUTATION_SELECTION=FAILED — git diff against "
                  f"{args.changed_since} did not run")
            return 1
        touched = {
            (ROOT / line.strip()).resolve()
            for line in changed.stdout.splitlines() if line.strip()
        }
        # A change under Tests/ can flip any verdict, because a mutation is
        # caught by whatever test happens to assert the behaviour. Only a
        # change confined to Sources/ narrows the selection honestly.
        if any("/Tests/" in str(path) for path in touched):
            print("PROJECTION_MUTATION_SELECTION=FULL — tests changed, so any "
                  "verdict can have moved")
        else:
            selected = [m for m in selected if m[2].resolve() in touched]
            print(f"PROJECTION_MUTATION_SELECTION={len(selected)} "
                  f"of {len(MUTATIONS)} — only files changed since "
                  f"{args.changed_since}")


    survivors: list[str] = []
    statuses: dict[str, str] = {}
    for name, description, path, old, new in selected:
        text = originals[path]
        if text.count(old) != 1:
            survivors.append(
                f"{name}: the mutation site occurs {text.count(old)} times, not once")
            continue
        mutated = text.replace(old, new)
        try:
            path.write_text(mutated, encoding="utf-8")
            code, hung, failed = run_tests(
                args.swift_test, args.jobs, MUTATION_TEST_FILTERS.get(name))
        finally:
            # Restore ONLY what this harness wrote.
            #
            # `originals` is a snapshot taken before the first mutation, and
            # restoring from it unconditionally overwrites anything that
            # changed the file meanwhile -- which is not hypothetical: an edit
            # made to a mutated file during a run was silently reverted, and
            # the loss was noticed only because the next build failed on a
            # mutation's own text. The tree lock stops a second HARNESS; it
            # cannot stop an editor, and this is the part that can.
            #
            # A file that no longer holds exactly what was written is left
            # alone and reported. Losing a mutation's score is recoverable;
            # losing someone's work is not.
            collided = path.read_text(encoding="utf-8") != mutated
            if not collided:
                path.write_text(text, encoding="utf-8")
        if collided:
            print(
                f"PROJECTION_MUTATION_COLLISION={name}\n"
                f"  {path} changed underneath the run and was NOT restored.\n"
                "  It still holds whatever overwrote the mutation, plus the\n"
                "  mutation itself. This harness's snapshot was discarded\n"
                "  rather than written over the change; recover the file by\n"
                "  hand and re-run.")
            return 1
        # A run that failed AND hung is CAUGHT, and says so: the hang is
        # reported alongside rather than instead of the verdict.
        if failed or (code != 0 and not hung):
            status = "CAUGHT+HUNG" if hung else "CAUGHT"
        elif hung:
            status = "HUNG"
        else:
            status = "SURVIVED"
        statuses[name] = status
        # flush: a full run takes hours and stdout is block-buffered when
        # redirected, so without this a redirected run shows NOTHING until
        # it finishes -- which is exactly when the progress stops mattering.
        print(f"{status:11} {name:34} {description}", flush=True)
        if status == "SURVIVED":
            survivors.append(f"{name}: {description}")

    for path, text in originals.items():
        if path.read_text(encoding="utf-8") != text:
            survivors.append(f"{path} was not restored")

    scope = "" if args.only is None else f" SELECTED={args.only!r}"
    caught = sum(status.startswith("CAUGHT") for status in statuses.values())
    hung_count = sum("HUNG" in status for status in statuses.values())
    unscored = len(selected) - len(statuses)
    content_selected = {
        item[0] for item in selected if item[0] in CONTENT_READER_MUTATION_NAMES
    }
    content_caught = sum(
        statuses.get(name, "").startswith("CAUGHT") for name in content_selected)
    content_hung = sum(
        "HUNG" in statuses.get(name, "") for name in content_selected)
    content_unscored = len(content_selected) - sum(
        name in statuses for name in content_selected)
    content_survivors = len(content_selected) - content_caught
    if args.output is not None and args.only is None:
        # Only a FULL pass may write the record: a --only run selects a subset
        # and its counts would understate the harness. The status gate derives
        # PROJECTION_MUTATIONS from this file's source too, so a record left
        # behind by an older tree disagrees with it and fails the gate.
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps({
            "PROJECTION_MUTATIONS": len(MUTATIONS),
            "PROJECTION_MUTATIONS_LAST_FULL_RUN": len(selected),
            "PROJECTION_MUTATIONS_CAUGHT": caught,
            "PROJECTION_MUTATION_SURVIVORS": len(survivors),
            "PROJECTION_MUTATION_HUNG": hung_count,
            "PROJECTION_MUTATION_UNSCORED": unscored,
            "CONTENT_READER_MUTATIONS": len(content_selected),
            "CONTENT_READER_MUTATIONS_CAUGHT": content_caught,
            "CONTENT_READER_MUTATION_SURVIVORS": content_survivors,
            "CONTENT_READER_MUTATION_HUNG": content_hung,
            "CONTENT_READER_MUTATION_UNSCORED": content_unscored,
            "survivors": survivors,
        }, indent=2) + "\n", encoding="utf-8")
    # Qualified names, not a bare CAUGHT=. Both this harness and the native
    # ABI one printed `CAUGHT=`, the documents quoted both, and a status-gate
    # fact key can only mean one thing -- so the gate could derive neither and
    # skipped both in silence. One key, one meaning (Foundation 100).
    print(f"PROJECTION_MUTATIONS={len(selected)} "
          f"PROJECTION_MUTATIONS_CAUGHT={caught} "
          f"PROJECTION_MUTATION_SURVIVORS={len(survivors)} "
          f"PROJECTION_MUTATION_HUNG={hung_count} "
          f"PROJECTION_MUTATION_UNSCORED={unscored} "
          f"CONTENT_READER_MUTATIONS={len(content_selected)} "
          f"CONTENT_READER_MUTATIONS_CAUGHT={content_caught} "
          f"CONTENT_READER_MUTATION_SURVIVORS={content_survivors} "
          f"CONTENT_READER_MUTATION_HUNG={content_hung} "
          f"CONTENT_READER_MUTATION_UNSCORED={content_unscored}"
          f"{scope} DECLARED={len(MUTATIONS)}")
    for survivor in survivors:
        print(f"  SURVIVOR {survivor}")
    return 1 if survivors else 0


if __name__ == "__main__":
    raise SystemExit(main())
