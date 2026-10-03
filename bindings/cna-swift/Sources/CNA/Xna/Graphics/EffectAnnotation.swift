// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.EffectAnnotation` projection.
    ///
    /// `sealed`, fourteen members, and **read-only**: eight `GetValue*`
    /// methods and six properties, with no setter anywhere. An annotation is
    /// metadata the effect author wrote into the shader source, so CNA's own
    /// header calls it "an owned *immutable* effect annotation".
    ///
    /// XNA's own getters are unconditional reads of a native `D3DXHANDLE` —
    /// they do not check the annotation's declared type first, and a
    /// `GetValueSingle` on a string annotation is whatever D3DX returns. CNA is
    /// stricter and refuses a mismatched read, so the two differ in what
    /// happens for a wrong-typed call: XNA answers garbage, this answers a
    /// `CNAError` on the runtime channel. That is the better of the two and it
    /// is not a choice this binding could avoid — there is nothing to
    /// reproduce, because the XNA behaviour is undefined rather than specified.
    public final class EffectAnnotation {
        private let box: EffectHandleBox
        private let info: CNASwift_EffectAnnotationInfo

        internal init(handle: UInt64, runtime: RuntimeState) {
            box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectAnnotationDestroy)
            var read = CNASwift_EffectAnnotationInfo()
            read.struct_size = UInt32(MemoryLayout<CNASwift_EffectAnnotationInfo>.size)
            read.struct_version = 1
            // The description is read ONCE, at construction, because all four
            // properties that expose it are infallible in XNA -- they read
            // fields the constructor filled. A fallible route behind an
            // infallible getter is the mistake `GraphicsDevice.GraphicsProfile`
            // records; this is the same remedy.
            if runtime.functions.effectAnnotationGetInfo(handle, &read) != 0 {
                read.row_count = 0
                read.column_count = 0
            }
            info = read
        }

        /// `EffectAnnotation.RowCount`, a field read.
        public var RowCount: Int32 { info.row_count }
        /// `EffectAnnotation.ColumnCount`, a field read.
        public var ColumnCount: Int32 { info.column_count }

        /// `EffectAnnotation.ParameterClass`.
        public var ParameterClass: EffectParameterClass {
            EffectParameterClass(rawValue: Int32(bitPattern: info.parameter_class))
                ?? .Scalar
        }

        /// `EffectAnnotation.ParameterType`.
        public var ParameterType: EffectParameterType {
            EffectParameterType(rawValue: Int32(bitPattern: info.parameter_type))
                ?? .Void
        }

        /// `EffectAnnotation.Name`.
        ///
        /// Non-Optional: XNA's getter reads a `String` field the constructor
        /// filled from the native description, and an annotation always has a
        /// name. An annotation with no semantic reports the empty string, which
        /// is what CNA's zero byte count means.
        public var Name: String { (try? text(name: true)) ?? "" }

        /// `EffectAnnotation.Semantic`.
        ///
        /// **Optional**, and proven: `PROVEN_NULLABLE_SUCCESS`. `Name` stays
        /// non-Optional on `UNKNOWN_REFERENCE_NULLABILITY` and the deferral
        /// rule.
        public var Semantic: String? {
            guard let text = try? text(name: false), !text.isEmpty else { return nil }
            return text
        }

        private func text(name: Bool) throws -> String {
            let handle = try box.validated("EffectAnnotation.Name")
            let functions = box.runtime.functions
            return try Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: name ? "cna_effect_annotation_copy_name"
                                : "cna_effect_annotation_copy_semantic",
                byteCount: name ? functions.effectAnnotationGetNameByteCount
                                : functions.effectAnnotationGetSemanticByteCount,
                copy: name ? functions.effectAnnotationCopyName
                           : functions.effectAnnotationCopySemantic)
        }

        // ------------------------------------------------------------------
        // The eight readers.

        /// `EffectAnnotation.GetValueBoolean()`.
        public func GetValueBoolean() throws -> Bool {
            var value: UInt8 = 0
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueBoolean(
                    try box.validated("GetValueBoolean"), &value),
                operation: "cna_effect_annotation_get_value_boolean")
            return value != 0
        }

        /// `EffectAnnotation.GetValueInt32()`.
        public func GetValueInt32() throws -> Int32 {
            var value: Int32 = 0
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueInt32(
                    try box.validated("GetValueInt32"), &value),
                operation: "cna_effect_annotation_get_value_int32")
            return value
        }

        /// `EffectAnnotation.GetValueSingle()`.
        public func GetValueSingle() throws -> Float {
            var value: Float = 0
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueSingle(
                    try box.validated("GetValueSingle"), &value),
                operation: "cna_effect_annotation_get_value_single")
            return value
        }

        /// `EffectAnnotation.GetValueVector2()`.
        public func GetValueVector2() throws -> Microsoft.Xna.Framework.Vector2 {
            var value = CNASwift_Vector2()
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueVector2(
                    try box.validated("GetValueVector2"), &value),
                operation: "cna_effect_annotation_get_value_vector2")
            return Microsoft.Xna.Framework.Vector2(value.x, value.y)
        }

        /// `EffectAnnotation.GetValueVector3()`.
        public func GetValueVector3() throws -> Microsoft.Xna.Framework.Vector3 {
            var value = CNASwift_Vector3()
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueVector3(
                    try box.validated("GetValueVector3"), &value),
                operation: "cna_effect_annotation_get_value_vector3")
            return Microsoft.Xna.Framework.Vector3(value.x, value.y, value.z)
        }

        /// `EffectAnnotation.GetValueVector4()`.
        public func GetValueVector4() throws -> Microsoft.Xna.Framework.Vector4 {
            var value = CNASwift_Vector4()
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueVector4(
                    try box.validated("GetValueVector4"), &value),
                operation: "cna_effect_annotation_get_value_vector4")
            return Microsoft.Xna.Framework.Vector4(value.x, value.y, value.z, value.w)
        }

        /// `EffectAnnotation.GetValueMatrix()`.
        public func GetValueMatrix() throws -> Microsoft.Xna.Framework.Matrix {
            var value = CNASwift_Matrix()
            try box.runtime.functions.check(
                box.runtime.functions.effectAnnotationGetValueMatrix(
                    try box.validated("GetValueMatrix"), &value),
                operation: "cna_effect_annotation_get_value_matrix")
            return Microsoft.Xna.Framework.Graphics.matrix(from: value)
        }

        /// `EffectAnnotation.GetValueString()`.
        public func GetValueString() throws -> String {
            let handle = try box.validated("GetValueString")
            let functions = box.runtime.functions
            return try Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: "cna_effect_annotation_copy_value_string",
                byteCount: functions.effectAnnotationGetValueStringByteCount,
                copy: functions.effectAnnotationCopyValueString)
        }
    }

    /// `CNA_Matrix` is row-major `m11..m44`, and so is XNA's `Matrix`. The
    /// conversion is written once rather than at each of the four sites that
    /// needs it.
    internal static func matrix(from value: CNASwift_Matrix)
        -> Microsoft.Xna.Framework.Matrix {
        Microsoft.Xna.Framework.Matrix(
            value.m11, value.m12, value.m13, value.m14,
            value.m21, value.m22, value.m23, value.m24,
            value.m31, value.m32, value.m33, value.m34,
            value.m41, value.m42, value.m43, value.m44)
    }

    internal static func nativeMatrix(from value: Microsoft.Xna.Framework.Matrix)
        -> CNASwift_Matrix {
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
