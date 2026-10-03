// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.TextureCube` projection.
    ///
    /// `open`, not `final`: XNA leaves it derivable and `RenderTargetCube` is
    /// the derived class. Its storage, disposal and whole `GraphicsResource`
    /// surface come from the base, exactly as `Texture2D`'s do.
    ///
    /// **The faces can be created here but not read or written.**
    /// `cna_texturecube_create` succeeds and `cna_texturecube_get_info` reports
    /// the granted size, level count and format, but `_set_data` and `_get_data`
    /// both answer `CNA_RESULT_NOT_SUPPORTED` on the qualified artifact for
    /// every one of the six faces (`build-probe/f64_cube.c`). The six transfer
    /// members are implemented and reach the route; the refusal is the
    /// renderer's and is reported on the runtime channel.
    open class TextureCube: Texture {
        /// `TextureCube.Size`, read once from `cna_texturecube_get_info`.
        public let Size: Int32

        internal init(
            handle: UInt64,
            runtime: RuntimeState,
            device: GraphicsDevice?,
            typeName: String,
            destroy: @escaping NativeDestroyRoute,
            size: Int32,
            levelCount: Int32,
            format: SurfaceFormat
        ) {
            Size = size
            super.init(
                storage: NativeHandleStorage(
                    handle: handle, typeName: typeName, ownership: .owned,
                    runtime: runtime, destroy: destroy),
                device: device, levelCount: levelCount, format: format)
            runtime.register(self)
        }

        /// `TextureCube(GraphicsDevice, Int32 size, Boolean mipMap, SurfaceFormat format)`.
        ///
        /// `ValidateCreationParameters` runs first — the size guard, the cube
        /// format list, `MaxCubeSize`, the power-of-two rule and the DXT
        /// alignment — and only then is the face storage created.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            size: Int32,
            mipMap: Bool,
            format: SurfaceFormat
        ) throws {
            try graphicsDevice.profileCapabilities.validateCubeCreation(
                size: size, format: format)
            let deviceHandle = try graphicsDevice.validatedHandle("TextureCube.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_TextureCubeCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_TextureCubeCreateInfo>.size)
            create.struct_version = 1
            create.size = UInt32(bitPattern: size)
            create.mip_map = mipMap ? 1 : 0
            create.format = UInt32(bitPattern: format.rawValue)

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.textureCubeCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_texturecube_create")
            do {
                let granted = try TextureCube.readInfo(handle: handle, runtime: runtime)
                self.init(
                    handle: handle, runtime: runtime, device: graphicsDevice,
                    typeName: "TextureCube",
                    destroy: runtime.functions.textureCubeDestroy,
                    size: granted.size, levelCount: granted.levelCount,
                    format: granted.format)
            } catch {
                // The native cube exists and this initializer will not return an
                // object to own it, so it is released here -- the counterpart of
                // XNA's `fault` handler, at the only place a Swift initializer
                // can put one.
                _ = runtime.functions.textureCubeDestroy(handle)
                throw error
            }
        }

        /// What CNA granted, which is what `Size`, `LevelCount` and `Format`
        /// report — the same rule every other creation path here follows.
        internal static func readInfo(handle: UInt64, runtime: RuntimeState) throws
            -> (size: Int32, levelCount: Int32, format: SurfaceFormat) {
            var info = CNASwift_TextureCubeInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_TextureCubeInfo>.size)
            info.struct_version = 1
            try runtime.functions.check(
                runtime.functions.textureCubeGetInfo(handle, &info),
                operation: "cna_texturecube_get_info")
            guard info.size <= UInt32(Int32.max), info.level_count <= UInt32(Int32.max) else {
                throw CNAError.nativeFailure(
                    operation: "TextureCube dimensions", result: 10,
                    message: "size or level count exceeds the XNA Int32 range")
            }
            guard let format = SurfaceFormat(rawValue: Int32(bitPattern: info.format)) else {
                throw CNAError.nativeFailure(
                    operation: "TextureCube.Format", result: 1,
                    message: "native surface format \(info.format) is not an XNA SurfaceFormat")
            }
            return (Int32(info.size), Int32(info.level_count), format)
        }

        // ------------------------------------------------------------------
        // SetData and GetData, one face at a time.

        /// `SetData<T>(CubeMapFace cubeMapFace, T[] data)`.
        public func SetData<T>(
            _ cubeMapFace: CubeMapFace, data: [T]
        ) throws {
            try SetData(cubeMapFace, level: 0, rect: nil, data: data,
                        startIndex: 0, elementCount: Int32(data.count))
        }

        /// `SetData<T>(CubeMapFace, T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ cubeMapFace: CubeMapFace, data: [T],
            startIndex: Int32, elementCount: Int32
        ) throws {
            try SetData(cubeMapFace, level: 0, rect: nil, data: data,
                        startIndex: startIndex, elementCount: elementCount)
        }

        /// `SetData<T>(CubeMapFace, Int32 level, Nullable<Rectangle> rect,
        /// T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ cubeMapFace: CubeMapFace,
            level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            data: [T],
            startIndex: Int32,
            elementCount: Int32
        ) throws {
            let plan = try transferPlan(
                T.self, face: cubeMapFace, level: level, rect: rect,
                arrayCount: data.count, startIndex: startIndex,
                elementCount: elementCount, isSetting: true)
            let functions = nativeStorage.runtime.functions
            try data.withUnsafeBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                try functions.check(
                    withUnsafePointer(to: plan.transfer) { transfer in
                        (base + plan.byteOffset).withMemoryRebound(
                            to: CNASwift_Color.self, capacity: Int(plan.elementCount)) {
                            functions.textureCubeSetData(
                                plan.handle, transfer, $0, plan.elementCount)
                        }
                    },
                    operation: "cna_texturecube_set_data")
            }
        }

        /// `GetData<T>(CubeMapFace cubeMapFace, T[] data)`.
        public func GetData<T>(
            _ cubeMapFace: CubeMapFace, data: inout [T]
        ) throws {
            try GetData(cubeMapFace, level: 0, rect: nil, data: &data,
                        startIndex: 0, elementCount: Int32(data.count))
        }

        /// `GetData<T>(CubeMapFace, T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ cubeMapFace: CubeMapFace, data: inout [T],
            startIndex: Int32, elementCount: Int32
        ) throws {
            try GetData(cubeMapFace, level: 0, rect: nil, data: &data,
                        startIndex: startIndex, elementCount: elementCount)
        }

        /// `GetData<T>(CubeMapFace, Int32 level, Nullable<Rectangle> rect,
        /// T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ cubeMapFace: CubeMapFace,
            level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            data: inout [T],
            startIndex: Int32,
            elementCount: Int32
        ) throws {
            let plan = try transferPlan(
                T.self, face: cubeMapFace, level: level, rect: rect,
                arrayCount: data.count, startIndex: startIndex,
                elementCount: elementCount, isSetting: false)
            let functions = nativeStorage.runtime.functions
            try data.withUnsafeMutableBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                var required: UInt64 = 0
                try functions.check(
                    withUnsafePointer(to: plan.transfer) { transfer in
                        (base + plan.byteOffset).withMemoryRebound(
                            to: CNASwift_Color.self, capacity: Int(plan.elementCount)) {
                            functions.textureCubeGetData(
                                plan.handle, transfer, $0, plan.elementCount, &required)
                        }
                    },
                    operation: "cna_texturecube_get_data")
            }
        }

        private struct CubePlan {
            var handle: UInt64
            var transfer: CNASwift_TextureCubeTransfer
            var byteOffset: Int
            var elementCount: UInt64
        }

        /// `TextureCube.CopyData<T>`, which is `Texture2D`'s with a face.
        ///
        /// The same tests in the same order:
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);
        /// if (data == null || data.Length == 0)
        ///     throw new ArgumentNullException("data", NullNotAllowed);
        /// if (isSetting)
        ///     for each sampler in Textures, VertexTextures
        ///         if (sampler == this) throw ResourceInUse;   // E_ABORT
        /// GetLevelDesc(level, &desc);
        /// Helpers.ValidateCopyParameters(data.Length, startIndex, elementCount);
        /// GetAndValidateSizes<T>(&desc, ...);   // InvalidDataSize
        /// GetAndValidateRect(&desc, ..., ref rect);        // InvalidRect
        /// ValidateTotalSize(&desc, ...);                   // InvalidTotalSize
        /// ```
        ///
        /// An empty array is an `ArgumentNullException` naming `"data"`, and
        /// the array window is `ValidateCopyParameters` — both exactly as
        /// `Texture2D.CopyData` has them. The bound-texture scan reads
        /// `GraphicsDevice.Textures`, which is not projected, and is recorded
        /// in `recorded-message-absences.json`.
        ///
        /// CNA's cube transfer is typed `CNA_Color` rather than carrying an
        /// element type, so `T` is reinterpreted through bytes exactly as
        /// `Texture2D`'s conversion does, and a window that does not land on a
        /// texel boundary is refused rather than rounded.
        private func transferPlan<T>(
            _ element: T.Type,
            face: CubeMapFace,
            level: Int32,
            rect: Microsoft.Xna.Framework.Rectangle?,
            arrayCount: Int,
            startIndex: Int32,
            elementCount: Int32,
            isSetting: Bool
        ) throws -> CubePlan {
            let handle = try validatedHandle(
                isSetting ? "TextureCube.SetData" : "TextureCube.GetData")
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
            guard let formatSize = Microsoft.Xna.Framework.Graphics
                .expectedByteSize(of: Format) else {
                throw CNAError.producerInvariant(
                    "block-compressed transfers are not projected")
            }
            let elementSize = Int32(MemoryLayout<T>.size)
            if elementSize != formatSize {
                guard formatSize > elementSize, formatSize % elementSize == 0 else {
                    throw CNAArgumentException(
                        message: invalidDataSizeMessage)
                }
            }

            var regionWidth = Size
            var regionHeight = Size
            if let rect {
                guard rect.X >= 0, rect.Width > 0, rect.Y >= 0, rect.Height > 0 else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                let right = UInt32(bitPattern: rect.X &+ rect.Width)
                let bottom = UInt32(bitPattern: rect.Y &+ rect.Height)
                guard right <= UInt32(bitPattern: Size),
                      bottom <= UInt32(bitPattern: Size) else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                regionWidth = rect.Width
                regionHeight = rect.Height
            }

            let regionBytes = Int64(regionWidth) * Int64(regionHeight) * Int64(formatSize)
            let windowBytes = Int64(elementSize) * Int64(elementCount)
            guard regionBytes == windowBytes else {
                throw CNAArgumentException(
                    message: invalidTotalSizeMessage)
            }
            let byteOffset = Int(startIndex) * Int(elementSize)
            guard byteOffset % Int(formatSize) == 0 else {
                throw CNAArgumentException(
                    message: invalidTotalSizeMessage)
            }

            var transfer = CNASwift_TextureCubeTransfer()
            transfer.struct_size = UInt32(MemoryLayout<CNASwift_TextureCubeTransfer>.size)
            transfer.struct_version = 1
            transfer.face = UInt32(bitPattern: face.rawValue)
            transfer.level = level
            if let rect {
                transfer.has_rectangle = 1
                transfer.rectangle = CNASwift_Rectangle(
                    x: rect.X, y: rect.Y, width: rect.Width, height: rect.Height)
            }
            transfer.start_index = 0
            transfer.element_count = UInt64(windowBytes / Int64(formatSize))
            return CubePlan(
                handle: handle, transfer: transfer, byteOffset: byteOffset,
                elementCount: UInt64(windowBytes / Int64(formatSize)))
        }

        /// `protected override void Dispose(bool)` — the same shape as
        /// `Texture2D`'s, and for the same reasons.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
