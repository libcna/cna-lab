// SPDX-License-Identifier: MIT

import CNAShim

/// Shared plumbing for the model family.
///
/// Six types read a name through the same count/copy pair and three carry a
/// `Matrix`, so the conversions live here rather than six times over. It is
/// internal: nothing in it is XNA surface.
internal enum ModelSupport {

    /// `FrameworkResources.ModelHasNoEffect`.
    ///
    /// Held in `Microsoft.Xna.Framework.dll`'s embedded table, NOT in the
    /// Graphics assembly whose IL raises it -- Graphics carries no string
    /// table at all. Registered in `registered-assemblies.json` and pinned in
    /// `reference/xna40-selected-resource-strings.json`, so this is the
    /// assembly's own wording rather than a transcription.
    static let modelHasNoEffect = "ModelMeshPart has a null Effect."

    /// `FrameworkResources.ModelHasNoIEffectMatrices`.
    ///
    /// **One literal, deliberately, however long the line**, and the WHOLE
    /// message. Two drafts got this wrong in different ways. The first split
    /// it across a `+` for the margin; the gate matches the assembly's wording
    /// against a string in the source and a concatenation is not one. The
    /// second was worse: a debugging print had truncated the value at ninety
    /// characters and that truncation was typed in as if it were the message,
    /// losing the half that tells the caller to use `ModelMesh.Draw` instead.
    /// The gate refused both. A message is copied from the pinned extraction,
    /// never from something that displayed it.
    static let modelHasNoIEffectMatrices = "This model contains a custom effect which does not implement the IEffectMatrices interface, so it cannot be drawn using Model.Draw. Instead, call ModelMesh.Draw after setting the appropriate effect parameters."

    static func copyString(
        _ functions: NativeFunctions, _ handle: UInt64,
        _ size: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
        _ copy: (UInt64, UnsafeMutablePointer<CChar>?, UInt64,
                 UnsafeMutablePointer<UInt64>?) -> UInt32,
        _ sizeOperation: String, _ copyOperation: String
    ) throws -> String {
        var byteCount: UInt64 = 0
        try functions.check(size(handle, &byteCount), operation: sizeOperation)
        guard byteCount > 0 else { return "" }
        var bytes = [CChar](repeating: 0, count: Int(byteCount) + 1)
        var written: UInt64 = 0
        try functions.check(
            bytes.withUnsafeMutableBufferPointer { buffer in
                copy(handle, buffer.baseAddress, byteCount, &written)
            },
            operation: copyOperation)
        return String(decoding: bytes.prefix(Int(written))
                        .map { UInt8(bitPattern: $0) }, as: UTF8.self)
    }

    static func withStringView(
        _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
    ) -> UInt32 {
        utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
            var view = CNASwift_StringView()
            view.byte_length = UInt64(buffer.count)
            guard let base = buffer.baseAddress else { return body(view) }
            return base.withMemoryRebound(to: CChar.self, capacity: buffer.count) {
                view.data = UnsafePointer($0)
                return body(view)
            }
        }
    }

    /// The device the runtime is currently driving.
    ///
    /// A model's buffers belong to a device and the ABI does not say which:
    /// `cna_vertex_buffer_get_info` reports the count, the usage and the
    /// stride and nothing about ownership. Inside a game there is one, and
    /// `cna_game_get_graphics_device` names it -- which is what makes adopting
    /// a buffer from a bare handle possible at all rather than a missing
    /// member with a reason.
    static func currentDevice(
        _ runtime: RuntimeState
    ) -> Microsoft.Xna.Framework.Graphics.GraphicsDevice? {
        var device: UInt64 = 0
        guard runtime.functions.gameGetGraphicsDevice(
            runtime.gameHandle, &device) == 0, device != 0 else { return nil }
        return try? Microsoft.Xna.Framework.Graphics.GraphicsDevice(
            borrowedHandle: device, runtime: runtime)
    }

    static func adoptVertexBuffer(
        _ handle: UInt64, _ runtime: RuntimeState
    ) -> Microsoft.Xna.Framework.Graphics.VertexBuffer? {
        guard let device = currentDevice(runtime) else { return nil }
        var info = CNASwift_VertexBufferInfo()
        info.struct_size = UInt32(MemoryLayout<CNASwift_VertexBufferInfo>.size)
        info.struct_version = 1
        guard runtime.functions.vertexBufferGetInfo(handle, &info) == 0
        else { return nil }

        var elements = [CNASwift_VertexElement](
            repeating: CNASwift_VertexElement(),
            count: max(Int(info.vertex_element_count), 1))
        var written: UInt64 = 0
        guard elements.withUnsafeMutableBufferPointer({ buffer in
            runtime.functions.vertexBufferCopyDeclarationElements(
                handle, buffer.baseAddress, UInt64(buffer.count), &written)
        }) == 0 else { return nil }

        let projected = elements.prefix(Int(written)).map { element in
            Microsoft.Xna.Framework.Graphics.VertexElement(
                element.offset,
                Microsoft.Xna.Framework.Graphics.VertexElementFormat(
                    rawValue: Int32(element.format)) ?? .Single,
                Microsoft.Xna.Framework.Graphics.VertexElementUsage(
                    rawValue: Int32(element.usage)) ?? .Position,
                element.usage_index)
        }
        guard let declaration = try? Microsoft.Xna.Framework.Graphics
            .VertexDeclaration(vertexStride: info.vertex_stride,
                               elements: Array(projected))
        else { return nil }
        return Microsoft.Xna.Framework.Graphics.VertexBuffer(
            handle: handle, runtime: runtime, device: device,
            typeName: "VertexBuffer", declaration: declaration,
            vertexCount: info.vertex_count,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage(
                rawValue: Int32(info.buffer_usage)))
    }

    static func adoptIndexBuffer(
        _ handle: UInt64, _ runtime: RuntimeState
    ) -> Microsoft.Xna.Framework.Graphics.IndexBuffer? {
        guard let device = currentDevice(runtime) else { return nil }
        var info = CNASwift_IndexBufferInfo()
        info.struct_size = UInt32(MemoryLayout<CNASwift_IndexBufferInfo>.size)
        info.struct_version = 1
        guard runtime.functions.indexBufferGetInfo(handle, &info) == 0
        else { return nil }
        return Microsoft.Xna.Framework.Graphics.IndexBuffer(
            handle: handle, runtime: runtime, device: device,
            typeName: "IndexBuffer", indexCount: info.index_count,
            elementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize(
                rawValue: Int32(info.index_element_size)) ?? .SixteenBits,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage(
                rawValue: Int32(info.buffer_usage)))
    }

    static func matrix(
        from native: CNASwift_Matrix
    ) -> Microsoft.Xna.Framework.Matrix {
        Microsoft.Xna.Framework.Matrix(
            native.m11, native.m12, native.m13, native.m14,
            native.m21, native.m22, native.m23, native.m24,
            native.m31, native.m32, native.m33, native.m34,
            native.m41, native.m42, native.m43, native.m44)
    }

    static func native(
        _ value: Microsoft.Xna.Framework.Matrix
    ) -> CNASwift_Matrix {
        var native = CNASwift_Matrix()
        native.m11 = value.M11; native.m12 = value.M12
        native.m13 = value.M13; native.m14 = value.M14
        native.m21 = value.M21; native.m22 = value.M22
        native.m23 = value.M23; native.m24 = value.M24
        native.m31 = value.M31; native.m32 = value.M32
        native.m33 = value.M33; native.m34 = value.M34
        native.m41 = value.M41; native.m42 = value.M42
        native.m43 = value.M43; native.m44 = value.M44
        return native
    }
}
