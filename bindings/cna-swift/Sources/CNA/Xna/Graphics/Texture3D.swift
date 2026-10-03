// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.Texture3D` projection.
    ///
    /// **No `Texture3D` can be constructed on this host, and that is XNA's
    /// rule.** `Texture3D.ValidateCreationParameters`' second test is
    /// `if (MaxVolumeExtent == 0) Throw(ProfileFeatureNotSupported, "Texture3D")`,
    /// and the capability table extracted from the assembly gives `Reach` a
    /// `MaxVolumeExtent` of **0** and an empty `ValidVolumeFormats`. The device
    /// reports `Reach`, so every constructor call raises
    /// `NotSupportedException` before any native route is reached.
    ///
    /// CNA reaches the same answer independently: `cna_texture3d_create`
    /// answers `CNA_RESULT_NOT_SUPPORTED` on the qualified artifact
    /// (`build-probe/f64_cube.c`). The two agree, so the refusal is XNA's
    /// message and not a native result code.
    ///
    /// The type is therefore **complete and implemented**, and unreachable on
    /// this profile rather than absent. On a `HiDef` device — `MaxVolumeExtent`
    /// 256, fifteen valid volume formats — the same code constructs one.
    open class Texture3D: Texture {
        /// `Texture3D.Width`.
        public let Width: Int32
        /// `Texture3D.Height`.
        public let Height: Int32
        /// `Texture3D.Depth`.
        public let Depth: Int32

        internal init(
            handle: UInt64,
            runtime: RuntimeState,
            device: GraphicsDevice?,
            typeName: String,
            width: Int32,
            height: Int32,
            depth: Int32,
            levelCount: Int32,
            format: SurfaceFormat
        ) {
            Width = width
            Height = height
            Depth = depth
            super.init(
                storage: NativeHandleStorage(
                    handle: handle, typeName: typeName, ownership: .owned,
                    runtime: runtime, destroy: runtime.functions.texture3DDestroy),
                device: device, levelCount: levelCount, format: format)
            runtime.register(self)
        }

        /// `Texture3D(GraphicsDevice, Int32 width, Int32 height, Int32 depth,
        /// Boolean mipMap, SurfaceFormat format)`.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            width: Int32,
            height: Int32,
            depth: Int32,
            mipMap: Bool,
            format: SurfaceFormat
        ) throws {
            try graphicsDevice.profileCapabilities.validateVolumeCreation(
                width: width, height: height, depth: depth, format: format)
            let deviceHandle = try graphicsDevice.validatedHandle("Texture3D.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_Texture3DCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_Texture3DCreateInfo>.size)
            create.struct_version = 1
            create.width = UInt32(bitPattern: width)
            create.height = UInt32(bitPattern: height)
            create.depth = UInt32(bitPattern: depth)
            create.mip_map = mipMap ? 1 : 0
            create.format = UInt32(bitPattern: format.rawValue)

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.texture3DCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_texture3d_create")
            do {
                var info = CNASwift_Texture3DInfo()
                info.struct_size = UInt32(MemoryLayout<CNASwift_Texture3DInfo>.size)
                info.struct_version = 1
                try runtime.functions.check(
                    runtime.functions.texture3DGetInfo(handle, &info),
                    operation: "cna_texture3d_get_info")
                guard info.width <= UInt32(Int32.max), info.height <= UInt32(Int32.max),
                      info.depth <= UInt32(Int32.max),
                      info.level_count <= UInt32(Int32.max) else {
                    throw CNAError.nativeFailure(
                        operation: "Texture3D dimensions", result: 10,
                        message: "dimensions exceed the XNA Int32 range")
                }
                guard let granted = SurfaceFormat(
                    rawValue: Int32(bitPattern: info.format)) else {
                    throw CNAError.nativeFailure(
                        operation: "Texture3D.Format", result: 1,
                        message: "native surface format \(info.format) is not an XNA SurfaceFormat")
                }
                self.init(
                    handle: handle, runtime: runtime, device: graphicsDevice,
                    typeName: "Texture3D", width: Int32(info.width),
                    height: Int32(info.height), depth: Int32(info.depth),
                    levelCount: Int32(info.level_count), format: granted)
            } catch {
                _ = runtime.functions.texture3DDestroy(handle)
                throw error
            }
        }

        // ------------------------------------------------------------------
        // SetData and GetData, over a mip box rather than a rectangle.

        /// `SetData<T>(T[] data)` — the whole volume of level zero.
        public func SetData<T>(_ data: [T]) throws {
            try SetData(0, left: 0, top: 0, right: Width, bottom: Height,
                        front: 0, back: Depth, data: data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try SetData(0, left: 0, top: 0, right: Width, bottom: Height,
                        front: 0, back: Depth, data: data,
                        startIndex: startIndex, elementCount: elementCount)
        }

        /// `SetData<T>(Int32 level, Int32 left, Int32 top, Int32 right,
        /// Int32 bottom, Int32 front, Int32 back, T[] data, Int32 startIndex,
        /// Int32 elementCount)`.
        public func SetData<T>(
            _ level: Int32, left: Int32, top: Int32, right: Int32,
            bottom: Int32, front: Int32, back: Int32, data: [T],
            startIndex: Int32, elementCount: Int32
        ) throws {
            let plan = try transferPlan(
                T.self, level: level, left: left, top: top, right: right,
                bottom: bottom, front: front, back: back,
                arrayCount: data.count, startIndex: startIndex,
                elementCount: elementCount, isSetting: true)
            let functions = nativeStorage.runtime.functions
            try data.withUnsafeBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                try functions.check(
                    withUnsafePointer(to: plan.transfer) { transfer in
                        (base + plan.byteOffset).withMemoryRebound(
                            to: CNASwift_Color.self, capacity: Int(plan.elementCount)) {
                            functions.texture3DSetData(
                                plan.handle, transfer, $0, plan.elementCount)
                        }
                    },
                    operation: "cna_texture3d_set_data")
            }
        }

        /// `GetData<T>(T[] data)`.
        public func GetData<T>(_ data: inout [T]) throws {
            try GetData(0, left: 0, top: 0, right: Width, bottom: Height,
                        front: 0, back: Depth, data: &data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `GetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try GetData(0, left: 0, top: 0, right: Width, bottom: Height,
                        front: 0, back: Depth, data: &data,
                        startIndex: startIndex, elementCount: elementCount)
        }

        /// `GetData<T>(Int32 level, Int32 left, Int32 top, Int32 right,
        /// Int32 bottom, Int32 front, Int32 back, T[] data, Int32 startIndex,
        /// Int32 elementCount)`.
        public func GetData<T>(
            _ level: Int32, left: Int32, top: Int32, right: Int32,
            bottom: Int32, front: Int32, back: Int32, data: inout [T],
            startIndex: Int32, elementCount: Int32
        ) throws {
            let plan = try transferPlan(
                T.self, level: level, left: left, top: top, right: right,
                bottom: bottom, front: front, back: back,
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
                            functions.texture3DGetData(
                                plan.handle, transfer, $0, plan.elementCount, &required)
                        }
                    },
                    operation: "cna_texture3d_get_data")
            }
        }

        private struct VolumePlan {
            var handle: UInt64
            var transfer: CNASwift_Texture3DTransfer
            var byteOffset: Int
            var elementCount: UInt64
        }

        /// `Texture3D.CopyData<T>`, which is `Texture2D`'s over a box.
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
        /// GetAndValidateBox(&desc, ..., &box);  // InvalidRectangle, "box"
        /// if (lockW * lockH * lockD * formatSize != elementSize * elementCount)
        ///     throw new ArgumentException(InvalidTotalSize);
        /// ```
        ///
        /// `GetAndValidateBox` is `GetAndValidateRect` in three dimensions and
        /// its six comparisons are all **unsigned** — `right > Width`,
        /// `left >= right`, `bottom > Height`, `top >= bottom`, `back > Depth`,
        /// `front >= back` — so a negative coordinate wraps to a huge value and
        /// is caught there rather than slipping through. They are written
        /// unsigned here for the same reason `Texture2D`'s are. Its message is
        /// `InvalidRectangle` with the parameter name **`"box"`**, which is not
        /// the name of any parameter these six overloads take; that is XNA's,
        /// and it is reproduced rather than corrected.
        private func transferPlan<T>(
            _ element: T.Type,
            level: Int32, left: Int32, top: Int32, right: Int32,
            bottom: Int32, front: Int32, back: Int32,
            arrayCount: Int, startIndex: Int32, elementCount: Int32,
            isSetting: Bool
        ) throws -> VolumePlan {
            let handle = try validatedHandle(
                isSetting ? "Texture3D.SetData" : "Texture3D.GetData")
            guard arrayCount > 0 else {
                throw CNAArgumentNullException(
                    paramName: "data",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            try checkNotBoundToTheDevice(
                isSetting: isSetting, checksRenderTarget: false)
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
                    throw CNAArgumentException(message: invalidDataSizeMessage)
                }
            }
            guard Microsoft.Xna.Framework.Graphics.volumeBoxIsValid(
                width: Width, height: Height, depth: Depth, left: left,
                top: top, right: right, bottom: bottom, front: front, back: back
            ) else {
                throw CNAArgumentException(
                    message: invalidRectangleMessage, paramName: "box")
            }

            let voxels = Int64(right - left) * Int64(bottom - top)
                * Int64(back - front)
            let regionBytes = voxels * Int64(formatSize)
            let windowBytes = Int64(elementSize) * Int64(elementCount)
            guard regionBytes == windowBytes else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }
            let byteOffset = Int(startIndex) * Int(elementSize)
            guard byteOffset % Int(formatSize) == 0 else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }

            var transfer = CNASwift_Texture3DTransfer()
            transfer.struct_size = UInt32(MemoryLayout<CNASwift_Texture3DTransfer>.size)
            transfer.struct_version = 1
            transfer.level = level
            transfer.left = left
            transfer.top = top
            transfer.right = right
            transfer.bottom = bottom
            transfer.front = front
            transfer.back = back
            transfer.start_index = 0
            transfer.element_count = UInt64(windowBytes / Int64(formatSize))
            return VolumePlan(
                handle: handle, transfer: transfer, byteOffset: byteOffset,
                elementCount: UInt64(windowBytes / Int64(formatSize)))
        }

        /// `protected override void Dispose(bool)`.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
