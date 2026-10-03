// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.Texture2D` projection.
    ///
    /// `open`, not `final`: XNA leaves the class derivable and
    /// `RenderTarget2D` is the derived class that proves it. Its native
    /// storage, its disposal and its whole `GraphicsResource` surface come
    /// from the base, so a subclass inherits one handle with one owner and one
    /// destruction path rather than acquiring a second of each.
    open class Texture2D: Texture {
        public let Width: Int32
        public let Height: Int32

        // `get_Bounds` is
        //     ldloca V_0; ldc.i4.0; ldc.i4.0;
        //     ldfld _width; ldfld _height;
        //     call Rectangle::.ctor(int32,int32,int32,int32); ldloc.0; ret
        // -- a rectangle at the origin over the texture's own dimensions, with
        // no branch, no call out and no failure path. Both dimensions are
        // already CNA members read once from `cna_texture2d_get_info`, so this
        // composes them exactly as XNA does and reaches no native surface of
        // its own.
        public var Bounds: Microsoft.Xna.Framework.Rectangle {
            Microsoft.Xna.Framework.Rectangle(0, 0, Width, Height)
        }

        /// The designated initializer every concrete 2D texture uses.
        ///
        /// The destroy route is a parameter rather than a constant, because a
        /// `RenderTarget2D` is released through `cna_render_target_destroy`
        /// and an ordinary texture through `cna_texture2d_destroy`. One
        /// storage, one owner, one route — chosen by the concrete class.
        internal init(
            handle: UInt64,
            runtime: RuntimeState,
            device: GraphicsDevice?,
            typeName: String,
            destroy: @escaping NativeDestroyRoute,
            width: Int32,
            height: Int32,
            levelCount: Int32,
            format: SurfaceFormat
        ) {
            Width = width
            Height = height
            super.init(
                storage: NativeHandleStorage(
                    handle: handle,
                    typeName: typeName,
                    ownership: .owned,
                    runtime: runtime,
                    destroy: destroy
                ),
                device: device,
                levelCount: levelCount,
                format: format
            )
            runtime.register(self)
        }

        /// `FrameworkResources.ResourcesMustBeGreaterThanZeroSize`, read out
        /// of the registered `Microsoft.Xna.Framework.dll`. One message serves
        /// both dimensions; only the parameter name differs.
        internal static let resourcesMustBeGreaterThanZeroSizeMessage =
            "Resource size must be greater than zero."

        /// `Texture2D(GraphicsDevice graphicsDevice, Int32 width, Int32 height)`.
        ///
        /// Thirty bytes, and every one of them forwards:
        ///
        /// ```text
        /// CreateTexture(graphicsDevice, width, height,
        ///               mipMap: false, 0, 1, format: SurfaceFormat.Color)
        /// ```
        ///
        /// `ldc.i4.0` for `mipMap` and `ldc.i4.0` for `format` — which is
        /// `SurfaceFormat.Color`, the zero-valued case — so the short
        /// constructor is the long one with two literals, not a different
        /// creation path.
        ///
        /// The two arguments between them are `ldc.i4.0` and `ldc.i4.1` in
        /// **both** constructors, so neither is a projected parameter; they
        /// are `CreateTexture`'s own D3D usage and pool, and CNA's create
        /// info has no counterpart for either.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            width: Int32,
            height: Int32
        ) throws {
            try self.init(graphicsDevice: graphicsDevice, width: width,
                          height: height, mipMap: false, format: .Color)
        }

        /// `Texture2D(GraphicsDevice, Int32, Int32, Boolean, SurfaceFormat)`.
        ///
        /// Both constructors wrap `CreateTexture` in a `try`/`finally` whose
        /// handler is `this.Dispose(true)`: a texture whose creation throws
        /// disposes itself on the way out. There is nothing to dispose here —
        /// the Swift initializer has no handle until the native create
        /// succeeds, and a failed `init` never produces an object — so the
        /// handler has no counterpart rather than an empty one.
        ///
        /// CNA reports back what it actually granted, and those values become
        /// `Width`, `Height`, `LevelCount` and `Format` rather than the ones
        /// that were asked for. `RenderTarget2D` reads its own grants the same
        /// way, and for the same reason: "preferred" is what the parameter
        /// name means.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            width: Int32,
            height: Int32,
            mipMap: Bool,
            format: SurfaceFormat
        ) throws {
            // `Texture2D.ValidateCreationParameters(width, height, format,
            // mipMap)` opens with two `bgt` tests against zero, each naming
            // its own parameter:
            //
            //     if (width  <= 0) throw new ArgumentOutOfRangeException(
            //         "width",  ResourcesMustBeGreaterThanZeroSize);
            //     if (height <= 0) throw new ArgumentOutOfRangeException(
            //         "height", ResourcesMustBeGreaterThanZeroSize);
            //
            // One message, two parameter names, and the width is tested
            // first -- so a texture that is invalid in both dimensions blames
            // the width.
            //
            // What follows in that method is the GraphicsProfile capability
            // family -- DXT alignment, power-of-two rules, the maximum size
            // for the profile in force, and format support. None of it is
            // projected, and the blocker is named in
            // `tools/api_compat/recorded-message-absences.json`: the checks
            // read the device's profile, and `GraphicsDevice.GraphicsProfile`
            // is not a projected member. Inventing a profile to check against
            // would fabricate the fact the check depends on.
            guard width > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "width",
                    message: Texture2D.resourcesMustBeGreaterThanZeroSizeMessage)
            }
            guard height > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "height",
                    message: Texture2D.resourcesMustBeGreaterThanZeroSizeMessage)
            }
            // The rest of `ValidateCreationParameters` is the GraphicsProfile
            // capability family, and since Foundation 62 it is projected: the
            // device publishes its profile through
            // `cna_graphics_device_get_graphics_profile`, and the per-profile
            // limits are extracted from the assembly's own class constructor
            // rather than transcribed. Six messages left
            // `recorded-message-absences.json` here.
            try graphicsDevice.profileCapabilities.validateTextureCreation(
                width: width, height: height, format: format, mipMap: mipMap)
            let deviceHandle = try graphicsDevice.validatedHandle("Texture2D.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_Texture2DCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DCreateInfo>.size)
            create.struct_version = 1
            create.width = UInt32(bitPattern: width)
            create.height = UInt32(bitPattern: height)
            create.mip_map = mipMap ? 1 : 0
            create.format = UInt32(bitPattern: format.rawValue)

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.textureCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_texture2d_create"
            )

            var info = CNASwift_Texture2DInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DInfo>.size)
            info.struct_version = 1
            do {
                try runtime.functions.check(
                    runtime.functions.textureGetInfo(handle, &info),
                    operation: "cna_texture2d_get_info"
                )
                guard info.width <= UInt32(Int32.max), info.height <= UInt32(Int32.max),
                      info.level_count <= UInt32(Int32.max) else {
                    throw CNAError.nativeFailure(
                        operation: "Texture2D dimensions", result: 10,
                        message: "dimensions exceed the XNA Int32 range")
                }
                guard let grantedFormat = SurfaceFormat(
                    rawValue: Int32(bitPattern: info.format)) else {
                    throw CNAError.nativeFailure(
                        operation: "Texture2D.Format", result: 1,
                        message: "native surface format \(info.format) is not an XNA SurfaceFormat")
                }
                self.init(
                    handle: handle,
                    runtime: runtime,
                    device: graphicsDevice,
                    typeName: "Texture2D",
                    destroy: runtime.functions.textureDestroy,
                    width: Int32(info.width),
                    height: Int32(info.height),
                    levelCount: Int32(info.level_count),
                    format: grantedFormat
                )
            } catch {
                // The native texture exists and this initializer will not
                // return an object to own it, so it is released here. This is
                // the counterpart of XNA's `finally`, at the only place a
                // Swift initializer can put one.
                _ = runtime.functions.textureDestroy(handle)
                throw error
            }
        }

        // ------------------------------------------------------------------
        // SetData and GetData.
        //
        // XNA's are generic over any `T : struct` and blit `sizeof(T)` bytes;
        // the surface's format decides what those bytes mean. Three helpers
        // guard every overload, and each raises its own message:
        //
        //   GetAndValidateSizes<T>  InvalidDataSize    sizeof(T) against the format
        //   GetAndValidateRect      InvalidRectangle   the region against the surface
        //   ValidateTotalSize       InvalidTotalSize   the array against the region
        //
        // All three are transcribed below. CNA's transfer asks for an element
        // TYPE rather than a byte count, so the projection names the one that
        // matches the texture's own format and converts the element count into
        // that type's units -- which is the same reinterpretation XNA performs
        // implicitly. `build-probe/f56_texdata.c` measures the round trip, and
        // measures that a byte-typed transfer into a Color texture is refused,
        // which is why the conversion goes through the format's type and never
        // through `CNA_TEXTURE_DATA_BYTE`.

        /// `SetData<T>(T[] data)`.
        public func SetData<T>(_ data: [T]) throws {
            try SetData(0, rect: nil, data: data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try SetData(0, rect: nil, data: data, startIndex: startIndex,
                        elementCount: elementCount)
        }

        /// `SetData<T>(Int32 level, Nullable<Rectangle> rect, T[] data,
        /// Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            data: [T],
            startIndex: Int32,
            elementCount: Int32
        ) throws {
            // `CopyData`'s first instruction is `Helpers.CheckDisposed(this,
            // pComPtr)`, ahead of all three validations -- so a disposed
            // texture reports the disposal even when the arguments are also
            // wrong, and it reports it as an `ObjectDisposedException` naming
            // the dynamic type. Until Foundation 59 this read the handle
            // through the storage instead, which raises the runtime channel's
            // `CNAError.disposedObject`, and it read it last, so a disposed
            // texture with a bad element size reported the size.
            let handle = try validatedHandle("Texture2D.SetData")
            let plan = try transferPlan(
                T.self, level: level, rect: rect, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                isSetting: true)
            try data.withUnsafeBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                try nativeStorage.runtime.functions.check(
                    withUnsafePointer(to: plan.transfer) { transfer in
                        nativeStorage.runtime.functions.textureSetData(
                            handle, plan.dataType, transfer,
                            base + plan.byteOffset, plan.nativeElementCount)
                    },
                    operation: "cna_texture2d_set_data")
            }
        }

        /// `GetData<T>(T[] data)`.
        public func GetData<T>(_ data: inout [T]) throws {
            try GetData(0, rect: nil, data: &data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `GetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try GetData(0, rect: nil, data: &data, startIndex: startIndex,
                        elementCount: elementCount)
        }

        /// `GetData<T>(Int32 level, Nullable<Rectangle> rect, T[] data,
        /// Int32 startIndex, Int32 elementCount)`.
        ///
        /// The array is `inout` because CLR array parameters that a method
        /// writes through project that way here -- the same
        /// `arrayMutationParameterNames` rule `Vector3.Transform`'s
        /// `destinationArray` follows.
        public func GetData<T>(
            _ level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            data: inout [T],
            startIndex: Int32,
            elementCount: Int32
        ) throws {
            // The same `CopyData` prologue: disposal first, arguments after.
            let handle = try validatedHandle("Texture2D.GetData")
            let plan = try transferPlan(
                T.self, level: level, rect: rect, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                isSetting: false)
            try data.withUnsafeMutableBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                var required: UInt64 = 0
                try nativeStorage.runtime.functions.check(
                    withUnsafePointer(to: plan.transfer) { transfer in
                        nativeStorage.runtime.functions.textureGetData(
                            handle, plan.dataType, transfer,
                            base + plan.byteOffset, plan.nativeElementCount,
                            &required)
                    },
                    operation: "cna_texture2d_get_data")
            }
        }

        private struct TransferPlan {
            var transfer: CNASwift_Texture2DTransfer
            var dataType: UInt32
            var byteOffset: Int
            var nativeElementCount: UInt64
        }

        /// `CopyData`'s validations in `CopyData`'s order, then the conversion
        /// into CNA's units.
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);              // in the caller
        /// if (data == null || data.Length == 0)
        ///     throw new ArgumentNullException("data", NullNotAllowed);
        /// if (isActiveRenderTarget)
        ///     throw new InvalidOperationException(MustResolveRenderTarget);
        /// if (isSetting) {
        ///     for (i = 0; i < device.Textures._maxTextures; i++)
        ///         if (device.Textures[i] == this)
        ///             throw GetExceptionFromResult(E_ABORT);   // ResourceInUse
        ///     for (i = 0; i < device.VertexTextures._maxTextures; i++)
        ///         if (device.VertexTextures[i] == this)
        ///             throw GetExceptionFromResult(E_ABORT);
        /// }
        /// GetLevelDesc(level, &desc);
        /// Helpers.ValidateCopyParameters(data.Length, startIndex, elementCount);
        /// GetAndValidateSizes<T>(&desc, ...);   // InvalidDataSize
        /// GetAndValidateRect(&desc, ..., ref rect);        // InvalidRect
        /// ValidateTotalSize(&desc, ...);                   // InvalidTotalSize
        /// ```
        ///
        /// **An empty array is an `ArgumentNullException`.** `IL_001b: ldlen;
        /// IL_001d: brfalse IL_0444` branches to the same throw the null test
        /// uses, so a zero-length array reports `"data"` as null — the same
        /// shape `VertexBuffer.CopyData` has, and XNA's own behaviour rather
        /// than a tidied one.
        ///
        /// **The array window is `ValidateCopyParameters`, not the total-size
        /// test.** Until Foundation 64 this checked `startIndex` and
        /// `elementCount` against the array inline and raised
        /// `ArgumentException(InvalidTotalSize)` for both, *after* the element
        /// size and the rectangle. XNA raises
        /// `ArgumentOutOfRangeException(MustBeValidIndex)` naming `dataIndex`
        /// or `elementCount`, and raises it **before** either — so a bad window
        /// and a bad element size together reported the wrong one.
        ///
        /// Two of `CopyData`'s tests have no reachable input yet and are
        /// recorded in `recorded-message-absences.json` rather than written as
        /// code that cannot run: `isActiveRenderTarget` is set by
        /// `SetRenderTarget`, and the bound-texture scan reads
        /// `GraphicsDevice.Textures`. Neither is projected.
        private func transferPlan<T>(
            _ element: T.Type,
            level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            arrayCount: Int,
            startIndex: Int32,
            elementCount: Int32,
            isSetting: Bool
        ) throws -> TransferPlan {
            guard arrayCount > 0 else {
                throw CNAArgumentNullException(
                    paramName: "data",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            try checkNotBoundToTheDevice(
                isSetting: isSetting, checksRenderTarget: true)
            try Microsoft.Xna.Framework.Graphics.validateCopyParameters(
                dataLength: arrayCount, dataIndex: startIndex,
                elementCount: elementCount)

            // `GetAndValidateSizes<T>`:
            //     if (elementSize == formatSize) ok
            //     else if (formatSize <= elementSize) throw InvalidDataSize
            //     else if (formatSize % elementSize != 0) throw InvalidDataSize
            // -- so T may be exactly the format's size, or a size that divides
            // it exactly. `sizeof` in CIL is the unpadded size, which is what
            // `MemoryLayout<T>.size` reports.
            guard let formatSize = Microsoft.Xna.Framework.Graphics
                .expectedByteSize(of: Format) else {
                throw CNAError.producerInvariant(
                    "block-compressed transfers are not projected")
            }
            let elementSize = Int32(MemoryLayout<T>.size)
            if elementSize != formatSize {
                guard formatSize > elementSize, formatSize % elementSize == 0 else {
                    throw CNAArgumentException(message: invalidDataSizeMessage)
                }
            }

            // `GetAndValidateRect`: the region defaults to the whole surface,
            // and a supplied rectangle is checked twice -- once for a negative
            // origin or a non-positive extent, once for an edge past the
            // surface. That second comparison is `bgt.un`, UNSIGNED, so an
            // origin plus extent that overflows wraps to a huge value and is
            // caught there rather than slipping through.
            var regionWidth = Width
            var regionHeight = Height
            if let rect {
                guard rect.X >= 0, rect.Width > 0, rect.Y >= 0, rect.Height > 0 else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                let right = UInt32(bitPattern: rect.X &+ rect.Width)
                let bottom = UInt32(bitPattern: rect.Y &+ rect.Height)
                guard right <= UInt32(bitPattern: Width),
                      bottom <= UInt32(bitPattern: Height) else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                regionWidth = rect.Width
                regionHeight = rect.Height
            }

            // `ValidateTotalSize`: the region's byte count must equal the
            // array window's, exactly.
            let regionBytes = Int64(regionWidth) * Int64(regionHeight) * Int64(formatSize)
            let windowBytes = Int64(elementSize) * Int64(elementCount)
            guard regionBytes == windowBytes else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }

            guard let dataType = Microsoft.Xna.Framework.Graphics
                .nativeDataType(for: Format) else {
                throw CNAError.producerInvariant(
                    "CNA names no element type for \(Format)")
            }

            // CNA counts in elements of the FORMAT's type, and the array
            // window is counted in T. The two agree in bytes, which is the
            // only thing XNA guarantees, so the count converts through bytes.
            // The offset must land on a format element; a T smaller than the
            // format can otherwise start mid-texel, which CNA's element-indexed
            // transfer cannot express. XNA can, so that input is refused here
            // rather than silently rounded -- an honest refusal in place of a
            // wrong write.
            let byteOffset = Int(startIndex) * Int(elementSize)
            guard byteOffset % Int(formatSize) == 0 else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }

            var transfer = CNASwift_Texture2DTransfer()
            transfer.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DTransfer>.size)
            transfer.struct_version = 1
            transfer.level = level
            if let rect {
                transfer.has_rectangle = 1
                transfer.rectangle = CNASwift_Rectangle(
                    x: rect.X, y: rect.Y, width: rect.Width, height: rect.Height)
            }
            transfer.start_index = 0
            transfer.element_count = UInt64(windowBytes / Int64(formatSize))
            return TransferPlan(
                transfer: transfer, dataType: dataType, byteOffset: byteOffset,
                nativeElementCount: UInt64(windowBytes / Int64(formatSize)))
        }

        /// Adopts a texture the content manager loaded.
        ///
        /// The handle arrives already made, so the dimensions, level count and
        /// format are read from it exactly as `fromStream` reads them from a
        /// texture it just decoded -- the same two routes, the same Int32
        /// range check.
        internal static func adoptLoaded(
            handle: UInt64, runtime: RuntimeState
        ) throws -> Texture2D {
            var info = CNASwift_Texture2DInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DInfo>.size)
            info.struct_version = 1
            try runtime.functions.check(
                runtime.functions.textureGetInfo(handle, &info),
                operation: "cna_texture2d_get_info")
            guard info.width <= UInt32(Int32.max), info.height <= UInt32(Int32.max) else {
                throw CNAError.nativeFailure(
                    operation: "Texture2D dimensions", result: 10,
                    message: "dimensions exceed XNA Int32 range")
            }
            let common = try Texture.readCommonInfo(handle: handle, runtime: runtime)
            return Texture2D(
                handle: handle, runtime: runtime, device: nil,
                typeName: "Texture2D", destroy: runtime.functions.textureDestroy,
                width: Int32(info.width), height: Int32(info.height),
                levelCount: common.levelCount, format: common.format)
        }

        public static func FromStream(
            _ graphicsDevice: GraphicsDevice,
            stream: InputStream
        ) throws -> Texture2D {
            try fromStream(graphicsDevice, stream: stream, decode: nil)
        }

        /// `FromStream(GraphicsDevice, Stream, Int32 width, Int32 height, Boolean zoom)`.
        ///
        /// Twenty-three bytes of IL, and every one of them selects an image
        /// operation and forwards:
        ///
        /// ```text
        /// XnaImageOperation op = zoom ? (Scale | Crop) : Scale;   // 3 : 1
        /// return new Texture2D(graphicsDevice, stream, width, height, op);
        /// ```
        ///
        /// `XnaImageOperation` is `Nothing = 0, Scale = 1, Crop = 2` in the
        /// registered `Microsoft.Xna.Framework.dll`, so `zoom` is exactly the
        /// difference between *fit inside width×height* and *cover width×height
        /// and crop the overflow*. The two-argument `FromStream` above passes
        /// `Nothing` with the profile's `MaxTextureSize` for both dimensions,
        /// which is why it resizes nothing.
        ///
        /// `CNA_Texture2DDecodeInfo` carries `width`, `height` and a `zoom`
        /// boolean documented as *"true to cover-and-crop; false to fit while
        /// preserving aspect ratio"* — the same two operations under the same
        /// two names — so this overload is the same call as the other with a
        /// decode block supplied instead of null. Neither dimension is silently
        /// dropped: CNA reports the granted size back and `Width`/`Height` are
        /// read from that report, exactly as every other creation path here.
        public static func FromStream(
            _ graphicsDevice: GraphicsDevice,
            stream: InputStream,
            width: Int32,
            height: Int32,
            zoom: Bool
        ) throws -> Texture2D {
            var decode = CNASwift_Texture2DDecodeInfo()
            decode.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DDecodeInfo>.size)
            decode.struct_version = 1
            decode.width = UInt32(bitPattern: width)
            decode.height = UInt32(bitPattern: height)
            decode.zoom = zoom ? 1 : 0
            return try fromStream(graphicsDevice, stream: stream, decode: decode)
        }

        private static func fromStream(
            _ graphicsDevice: GraphicsDevice,
            stream: InputStream,
            decode: CNASwift_Texture2DDecodeInfo?
        ) throws -> Texture2D {
            let deviceHandle = try graphicsDevice.validatedHandle("Texture2D.FromStream")
            var encoded = Data()
            if stream.streamStatus == .notOpen { stream.open() }
            var buffer = [UInt8](repeating: 0, count: 16_384)
            while true {
                let count = stream.read(&buffer, maxLength: buffer.count)
                if count == 0 { break }
                if count < 0 {
                    throw CNAError.streamFailure(stream.streamError?.localizedDescription ?? "unknown read error")
                }
                encoded.append(buffer, count: count)
            }
            guard !encoded.isEmpty else { throw CNAError.streamFailure("encoded image stream is empty") }

            let runtime = graphicsDevice.runtimeState
            var handle: UInt64 = 0
            var decodeInfo = decode
            let result = encoded.withUnsafeBytes { rawBuffer -> UInt32 in
                let call = { (block: UnsafePointer<CNASwift_Texture2DDecodeInfo>?) -> UInt32 in
                    runtime.functions.textureCreateMemory(
                        deviceHandle,
                        rawBuffer.bindMemory(to: UInt8.self).baseAddress,
                        UInt64(rawBuffer.count),
                        block,
                        &handle
                    )
                }
                if decodeInfo != nil {
                    return withUnsafePointer(to: &decodeInfo!) { call($0) }
                }
                return call(nil)
            }
            try runtime.functions.check(result, operation: "cna_texture2d_create_from_encoded_memory")

            var info = CNASwift_Texture2DInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_Texture2DInfo>.size)
            info.struct_version = 1
            do {
                try runtime.functions.check(
                    runtime.functions.textureGetInfo(handle, &info),
                    operation: "cna_texture2d_get_info"
                )
            } catch {
                _ = runtime.functions.textureDestroy(handle)
                throw error
            }
            guard info.width <= UInt32(Int32.max), info.height <= UInt32(Int32.max) else {
                _ = runtime.functions.textureDestroy(handle)
                throw CNAError.nativeFailure(operation: "Texture2D dimensions", result: 10, message: "dimensions exceed XNA Int32 range")
            }
            let common: (levelCount: Int32, format: SurfaceFormat)
            do {
                common = try Texture.readCommonInfo(handle: handle, runtime: runtime)
            } catch {
                _ = runtime.functions.textureDestroy(handle)
                throw error
            }
            return Texture2D(
                handle: handle,
                runtime: runtime,
                device: graphicsDevice,
                typeName: "Texture2D",
                destroy: runtime.functions.textureDestroy,
                width: Int32(info.width),
                height: Int32(info.height),
                levelCount: common.levelCount,
                format: common.format
            )
        }

        // ------------------------------------------------------------------
        // SaveAsPng and SaveAsJpeg.
        //
        // Both are eleven bytes of IL that forward to the private
        // `SaveAsImage(Stream, XnaImageFormat, Int32, Int32)`, differing only
        // in the literal they push: `ldc.i4.2` for PNG and `ldc.i4.0` for
        // JPEG. Everything below is `SaveAsImage`.

        /// `SaveAsPng(Stream stream, Int32 width, Int32 height)`.
        public func SaveAsPng(
            _ stream: OutputStream, width: Int32, height: Int32
        ) throws {
            try saveAsImage(stream, format: Texture2D.nativeImageFormatPng,
                            width: width, height: height)
        }

        /// `SaveAsJpeg(Stream stream, Int32 width, Int32 height)`.
        public func SaveAsJpeg(
            _ stream: OutputStream, width: Int32, height: Int32
        ) throws {
            try saveAsImage(stream, format: Texture2D.nativeImageFormatJpeg,
                            width: width, height: height)
        }

        /// `CNA_TEXTURE_IMAGE_FORMAT_PNG`.
        private static let nativeImageFormatPng: UInt32 = 0
        /// `CNA_TEXTURE_IMAGE_FORMAT_JPEG`.
        private static let nativeImageFormatJpeg: UInt32 = 1

        /// `Texture2D.SaveAsImage(Stream, XnaImageFormat, Int32, Int32)`.
        ///
        /// ```text
        /// if (stream == null) throw ArgumentNullException("stream", NullNotAllowed);
        /// if (!stream.CanWrite) throw new ArgumentException("stream");
        /// if (format != Jpeg && format != Png) throw new ArgumentException("format");
        /// Color[] colors = <read the surface, converted to Color>;
        /// for (int i = 0; i < colors.Length; i++)
        ///     if (colors[i].A == 0) colors[i] = Color.Transparent;
        /// using (ImageStream image = ImageStream.FromColors(
        ///            colors, _width, _height, format, width, height))
        ///     stream.Write(new BinaryReader(image).ReadBytes((int)image.Length),
        ///                  0, ...);
        /// ```
        ///
        /// Four things in that body decide this projection.
        ///
        /// **The two `ArgumentException`s carry a parameter name as their
        /// *message*.** `newobj ArgumentException::.ctor(string)` is the
        /// one-argument overload, so `Message` is literally `"stream"` and
        /// `"format"` and `ParamName` is null. That is XNA's own slip and it is
        /// reproduced, not corrected: this binding projects what the assembly
        /// does. The `format` check cannot be reached here — the two public
        /// entry points are the only callers and each passes a constant — so it
        /// has no counterpart, and the `stream` null check has none either
        /// because a Swift `OutputStream` parameter cannot be nil. Both are
        /// recorded in `recorded-message-absences.json`.
        ///
        /// **`CanWrite` is a `System.IO.Stream` property Foundation does not
        /// have.** A `Foundation.OutputStream` is a writing stream by
        /// construction; what it can still be is *closed* or *failed*, which is
        /// the state `CanWrite == false` describes, so those two statuses raise
        /// the `ArgumentException` and nothing else does.
        ///
        /// **Every alpha-zero texel becomes `Color.Transparent` before
        /// encoding.** This is not cosmetic and CNA does not do it:
        /// `build-probe/f59_encode.c` encodes a texel authored `(200,100,50,0)`
        /// and an independent decode of the PNG reads `(200,100,50,0)` back,
        /// where XNA would have written `(0,0,0,0)`. The rewrite is therefore
        /// performed here, on a copy, and the copy is what is encoded — which
        /// is also what XNA does, since `ImageStream.FromColors` encodes the
        /// rewritten array and never the texture.
        ///
        /// **The encode target is a CPU-only texture built from that copy.**
        /// `cna_texture2d_create_cpu_only_rgba8` needs no device and no
        /// callback scope, which matches a `SaveAsPng` that XNA lets a caller
        /// make at any time; the intermediate is created, encoded and destroyed
        /// inside this call and is never reachable. It is the direct
        /// counterpart of the `ImageStream` XNA disposes in its `finally`.
        private func saveAsImage(
            _ stream: OutputStream, format: UInt32, width: Int32, height: Int32
        ) throws {
            guard stream.streamStatus != .closed, stream.streamStatus != .error else {
                throw CNAArgumentException(message: "stream")
            }

            // XNA's switch has one arm per SurfaceFormat and a `default` that
            // raises InvalidOperationException. Only `Color` can be created in
            // this environment -- nineteen of the twenty formats answer
            // CNA_RESULT_NOT_SUPPORTED, measured in `build-probe/f55_grants.c`
            // -- so the Color arm is the reachable one and the rest are
            // recorded rather than written as code that cannot run.
            guard Format == .Color else {
                throw CNAError.producerInvariant(
                    "SaveAsImage reads \(Format) through a converter CNA cannot "
                    + "supply; only SurfaceFormat.Color is creatable here")
            }

            var colors = [Microsoft.Xna.Framework.Color](
                repeating: Microsoft.Xna.Framework.Color.Transparent,
                count: Int(Width) * Int(Height))
            try GetData(&colors)
            for index in colors.indices where colors[index].A == 0 {
                colors[index] = Microsoft.Xna.Framework.Color.Transparent
            }

            let runtime = nativeStorage.runtime
            let pixels = colors.map {
                CNASwift_Color(r: $0.R, g: $0.G, b: $0.B, a: $0.A)
            }
            var source: UInt64 = 0
            try runtime.functions.check(
                pixels.withUnsafeBufferPointer { buffer in
                    runtime.functions.textureCreateCpuOnly(
                        UInt32(bitPattern: Width), UInt32(bitPattern: Height),
                        UInt32(bitPattern: SurfaceFormat.Color.rawValue),
                        buffer.baseAddress, UInt64(buffer.count), &source)
                },
                operation: "cna_texture2d_create_cpu_only_rgba8")
            defer { _ = runtime.functions.textureDestroy(source) }

            var byteCount: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.textureGetEncodedByteCount(
                    source, format, UInt32(bitPattern: width),
                    UInt32(bitPattern: height), &byteCount),
                operation: "cna_texture2d_get_encoded_byte_count")

            var encoded = [UInt8](repeating: 0, count: Int(byteCount))
            var written: UInt64 = 0
            try runtime.functions.check(
                encoded.withUnsafeMutableBufferPointer { buffer in
                    runtime.functions.textureCopyEncoded(
                        source, format, UInt32(bitPattern: width),
                        UInt32(bitPattern: height), buffer.baseAddress,
                        UInt64(buffer.count), &written)
                },
                operation: "cna_texture2d_copy_encoded")

            if stream.streamStatus == .notOpen { stream.open() }
            var offset = 0
            while offset < Int(written) {
                let count = encoded[offset...].withUnsafeBufferPointer { buffer in
                    stream.write(buffer.baseAddress!, maxLength: Int(written) - offset)
                }
                if count <= 0 {
                    throw CNAError.streamFailure(
                        stream.streamError?.localizedDescription ?? "unknown write error")
                }
                offset += count
            }
        }

        /// `protected override void Dispose(bool)`.
        ///
        /// ```text
        /// if (disposing) { try { ~Texture2D(); }
        ///                  finally { base.Dispose(true); } }
        /// else           { try { !Texture2D(); }
        ///                  finally { base.Dispose(false); } }
        /// ```
        ///
        /// `~Texture2D()` is a one-line forward to `!Texture2D()`, so both arms
        /// run the same body and differ only in the flag they hand the base:
        /// release the native texture if it is not already released, drop the
        /// device-lost recreation cache, then `GraphicsResource.Dispose(flag)`.
        ///
        /// Neither half has a counterpart of its own here. This projection's
        /// native handle is owned by `GraphicsResource`, which releases it as
        /// the first thing `Dispose` does — the same order, one level up — and
        /// `CleanupSavedData` frees `_savedData`, the CPU copy XNA keeps to
        /// recreate a D3D9 texture after a device loss, which CNA neither
        /// exposes nor requires. The override exists because XNA declares it
        /// and a subclass overriding it must reach this link of the chain.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
