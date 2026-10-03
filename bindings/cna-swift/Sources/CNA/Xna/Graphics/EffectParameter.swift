// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.EffectParameter` projection.
    ///
    /// Fifty-one members, and they are fifty-one because XNA writes an overload
    /// per type rather than a generic: twenty `SetValue`/`SetValueTranspose`,
    /// twenty-three `GetValue*`, eight properties. CNA does **not** repeat that
    /// shape — it publishes one *tagged* pair,
    ///
    /// ```text
    /// cna_effect_parameter_get_value (parameter, CNA_EffectValueType, void* out)
    /// cna_effect_parameter_set_value (parameter, CNA_EffectValueType, const void*)
    /// cna_effect_parameter_get_values(parameter, type, requested, dst, cap, &n)
    /// cna_effect_parameter_set_values(parameter, type, values, count)
    /// ```
    ///
    /// over nine value types, plus a texture pair over four overload
    /// identities. So the fifty-one members are fifty-one *names* over six
    /// routes, and every one of them is one line naming its tag. That is what
    /// makes the count tractable, and it is also where a defect would hide: a
    /// member naming the wrong tag would compile, run, and quietly read or
    /// write the wrong storage. Each tag is therefore asserted individually.
    ///
    /// **`SetValueTranspose` and `GetValueMatrixTranspose` are not a second
    /// storage location.** They are `MATRIX_TRANSPOSE` against the same
    /// parameter, which is CNA's model and XNA's: `SetValueTranspose` writes
    /// the transpose of what it is given into the same register file.
    public final class EffectParameter {
        internal let box: EffectHandleBox
        private let info: CNASwift_EffectParameterInfo
        private var annotations: EffectAnnotationCollection?
        private var elements: EffectParameterCollection?
        private var structureMembers: EffectParameterCollection?

        internal init(handle: UInt64, runtime: RuntimeState) {
            box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectParameterDestroy)
            var read = CNASwift_EffectParameterInfo()
            read.struct_size = UInt32(MemoryLayout<CNASwift_EffectParameterInfo>.size)
            read.struct_version = 1
            if runtime.functions.effectParameterGetInfo(handle, &read) != 0 {
                read.row_count = 0
                read.column_count = 0
            }
            info = read
        }

        // ------------------------------------------------------------------
        // The eight properties. All eight are infallible field reads in XNA.

        /// `EffectParameter.RowCount`.
        public var RowCount: Int32 { info.row_count }
        /// `EffectParameter.ColumnCount`.
        public var ColumnCount: Int32 { info.column_count }

        /// `EffectParameter.ParameterClass`.
        public var ParameterClass: EffectParameterClass {
            EffectParameterClass(rawValue: Int32(bitPattern: info.parameter_class))
                ?? .Scalar
        }

        /// `EffectParameter.ParameterType`.
        public var ParameterType: EffectParameterType {
            EffectParameterType(rawValue: Int32(bitPattern: info.parameter_type))
                ?? .Void
        }

        /// `EffectParameter.Name`.
        public var Name: String { (try? text(name: true)) ?? "" }
        /// `EffectParameter.Semantic`.
        ///
        /// **Optional**, and proven: `PROVEN_NULLABLE_SUCCESS`. A parameter
        /// the author gave no semantic reports nil, not the empty string —
        /// CNA's zero byte count is that state. `Name` stays non-Optional
        /// because its verdict is `UNKNOWN_REFERENCE_NULLABILITY` and the
        /// deferral rule keeps the non-Optional shape until nullability is
        /// proven.
        public var Semantic: String? {
            guard let text = try? text(name: false), !text.isEmpty else { return nil }
            return text
        }

        /// `EffectParameter.Annotations`.
        public var Annotations: EffectAnnotationCollection {
            if let annotations { return annotations }
            let built = Microsoft.Xna.Framework.Graphics.annotationCollection(
                box: box, route: box.runtime.functions.effectParameterGetAnnotations)
            annotations = built
            return built
        }

        /// `EffectParameter.Elements` — an array parameter's members, empty for
        /// a scalar one.
        public var Elements: EffectParameterCollection {
            if let elements { return elements }
            let built = parameterCollection(
                box.runtime.functions.effectParameterGetElements)
            elements = built
            return built
        }

        /// `EffectParameter.StructureMembers` — a struct parameter's fields.
        public var StructureMembers: EffectParameterCollection {
            if let structureMembers { return structureMembers }
            let built = parameterCollection(
                box.runtime.functions.effectParameterGetStructureMembers)
            structureMembers = built
            return built
        }

        private func parameterCollection(
            _ route: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
        ) -> EffectParameterCollection {
            var handle: UInt64 = 0
            guard let owner = try? box.validated("EffectParameter collection"),
                  route(owner, &handle) == 0 else {
                return EffectParameterCollection(handle: 0, runtime: box.runtime)
            }
            return EffectParameterCollection(handle: handle, runtime: box.runtime)
        }

        private func text(name: Bool) throws -> String {
            let handle = try box.validated("EffectParameter.Name")
            let functions = box.runtime.functions
            return try Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: name ? "cna_effect_parameter_copy_name"
                                : "cna_effect_parameter_copy_semantic",
                byteCount: name ? functions.effectParameterGetNameByteCount
                                : functions.effectParameterGetSemanticByteCount,
                copy: name ? functions.effectParameterCopyName
                           : functions.effectParameterCopySemantic)
        }

        // ------------------------------------------------------------------
        // The tagged value model, written once.

        /// `CNA_EffectValueType`, as named constants rather than literals at
        /// fifty call sites. A wrong tag is the defect this type is most
        /// exposed to, so the tags have names and the names are asserted.
        internal enum ValueTag: UInt32 {
            case boolean = 0, int32 = 1, single = 2, matrix = 3
            case matrixTranspose = 4, quaternion = 5
            case vector2 = 6, vector3 = 7, vector4 = 8
        }

        /// `CNA_EffectTextureType`. `base` is **setter-only**: CNA's header says
        /// "no corresponding native getter exists", and XNA agrees — its
        /// `GetValue` overloads are `Texture2D`, `Texture3D` and `TextureCube`,
        /// never plain `Texture`.
        internal enum TextureTag: UInt32 {
            case base = 0, twoDimensional = 1, threeDimensional = 2, cube = 3
        }

        private func readScalar<T>(_ tag: ValueTag, _ zero: T) throws -> T {
            let handle = try box.validated("EffectParameter.GetValue")
            var value = zero
            try box.runtime.functions.check(
                withUnsafeMutablePointer(to: &value) {
                    box.runtime.functions.effectParameterGetValue(
                        handle, tag.rawValue, UnsafeMutableRawPointer($0))
                },
                operation: "cna_effect_parameter_get_value")
            return value
        }

        private func writeScalar<T>(_ tag: ValueTag, _ value: T) throws {
            let handle = try box.validated("EffectParameter.SetValue")
            var copy = value
            try box.runtime.functions.check(
                withUnsafePointer(to: &copy) {
                    box.runtime.functions.effectParameterSetValue(
                        handle, tag.rawValue, UnsafeRawPointer($0))
                },
                operation: "cna_effect_parameter_set_value")
        }

        /// The array reader. `count` is the caller's request, and XNA's own
        /// `GetValue*Array(int count)` allocates exactly that many and asks the
        /// effect to fill them — a shorter native answer leaves the tail at its
        /// default, which is what `new T[count]` starts as.
        private func readArray<T>(_ tag: ValueTag, count: Int32, _ zero: T) throws -> [T] {
            guard count >= 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "count")
            }
            guard count > 0 else { return [] }
            let handle = try box.validated("EffectParameter.GetValueArray")
            var values = [T](repeating: zero, count: Int(count))
            var written: UInt64 = 0
            try box.runtime.functions.check(
                values.withUnsafeMutableBufferPointer { buffer in
                    box.runtime.functions.effectParameterGetValues(
                        handle, tag.rawValue, UInt64(count),
                        UnsafeMutableRawPointer(buffer.baseAddress),
                        UInt64(count), &written)
                },
                operation: "cna_effect_parameter_get_values")
            return values
        }

        private func writeArray<T>(_ tag: ValueTag, _ values: [T]) throws {
            let handle = try box.validated("EffectParameter.SetValue")
            try box.runtime.functions.check(
                values.withUnsafeBufferPointer { buffer in
                    box.runtime.functions.effectParameterSetValues(
                        handle, tag.rawValue,
                        UnsafeRawPointer(buffer.baseAddress), UInt64(buffer.count))
                },
                operation: "cna_effect_parameter_set_values")
        }

        // ------------------------------------------------------------------
        // SetValue: eighteen overloads plus the two transposes.

        /// `SetValue(Boolean value)`.
        public func SetValue(_ value: Bool) throws {
            try writeScalar(.boolean, UInt8(value ? 1 : 0))
        }
        /// `SetValue(Boolean[] value)`.
        public func SetValue(_ value: [Bool]) throws {
            try writeArray(.boolean, value.map { UInt8($0 ? 1 : 0) })
        }
        /// `SetValue(Int32 value)`.
        public func SetValue(_ value: Int32) throws { try writeScalar(.int32, value) }
        /// `SetValue(Int32[] value)`.
        public func SetValue(_ value: [Int32]) throws { try writeArray(.int32, value) }
        /// `SetValue(Single value)`.
        public func SetValue(_ value: Float) throws { try writeScalar(.single, value) }
        /// `SetValue(Single[] value)`.
        public func SetValue(_ value: [Float]) throws { try writeArray(.single, value) }

        /// `SetValue(Vector2 value)`.
        public func SetValue(_ value: Microsoft.Xna.Framework.Vector2) throws {
            try writeScalar(.vector2, CNASwift_Vector2(x: value.X, y: value.Y))
        }
        /// `SetValue(Vector2[] value)`.
        public func SetValue(_ value: [Microsoft.Xna.Framework.Vector2]) throws {
            try writeArray(.vector2, value.map { CNASwift_Vector2(x: $0.X, y: $0.Y) })
        }
        /// `SetValue(Vector3 value)`.
        public func SetValue(_ value: Microsoft.Xna.Framework.Vector3) throws {
            try writeScalar(.vector3, CNASwift_Vector3(x: value.X, y: value.Y, z: value.Z))
        }
        /// `SetValue(Vector3[] value)`.
        public func SetValue(_ value: [Microsoft.Xna.Framework.Vector3]) throws {
            try writeArray(.vector3, value.map {
                CNASwift_Vector3(x: $0.X, y: $0.Y, z: $0.Z)
            })
        }
        /// `SetValue(Vector4 value)`.
        public func SetValue(_ value: Microsoft.Xna.Framework.Vector4) throws {
            try writeScalar(.vector4, CNASwift_Vector4(
                x: value.X, y: value.Y, z: value.Z, w: value.W))
        }
        /// `SetValue(Vector4[] value)`.
        public func SetValue(_ value: [Microsoft.Xna.Framework.Vector4]) throws {
            try writeArray(.vector4, value.map {
                CNASwift_Vector4(x: $0.X, y: $0.Y, z: $0.Z, w: $0.W)
            })
        }
        /// `SetValue(Quaternion value)`.
        public func SetValue(_ value: Microsoft.Xna.Framework.Quaternion) throws {
            try writeScalar(.quaternion, CNASwift_Quaternion(
                x: value.X, y: value.Y, z: value.Z, w: value.W))
        }
        /// `SetValue(Quaternion[] value)`.
        public func SetValue(_ value: [Microsoft.Xna.Framework.Quaternion]) throws {
            try writeArray(.quaternion, value.map {
                CNASwift_Quaternion(x: $0.X, y: $0.Y, z: $0.Z, w: $0.W)
            })
        }
        /// `SetValue(Matrix value)`.
        public func SetValue(_ value: Microsoft.Xna.Framework.Matrix) throws {
            try writeScalar(.matrix,
                            Microsoft.Xna.Framework.Graphics.nativeMatrix(from: value))
        }
        /// `SetValue(Matrix[] value)`.
        public func SetValue(_ value: [Microsoft.Xna.Framework.Matrix]) throws {
            try writeArray(.matrix, value.map {
                Microsoft.Xna.Framework.Graphics.nativeMatrix(from: $0)
            })
        }

        /// `SetValueTranspose(Matrix value)` — the same parameter, the
        /// transposed tag.
        public func SetValueTranspose(_ value: Microsoft.Xna.Framework.Matrix) throws {
            try writeScalar(.matrixTranspose,
                            Microsoft.Xna.Framework.Graphics.nativeMatrix(from: value))
        }
        /// `SetValueTranspose(Matrix[] value)`.
        public func SetValueTranspose(_ value: [Microsoft.Xna.Framework.Matrix]) throws {
            try writeArray(.matrixTranspose, value.map {
                Microsoft.Xna.Framework.Graphics.nativeMatrix(from: $0)
            })
        }

        /// `SetValue(String value)`.
        public func SetValue(_ value: String) throws {
            let handle = try box.validated("EffectParameter.SetValue")
            var bytes = Array(value.utf8)
            try box.runtime.functions.check(
                bytes.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                    var view = CNASwift_StringView()
                    view.byte_length = UInt64(buffer.count)
                    guard let base = buffer.baseAddress else {
                        return box.runtime.functions.effectParameterSetValueString(
                            handle, view)
                    }
                    return base.withMemoryRebound(
                        to: CChar.self, capacity: buffer.count) {
                        view.data = UnsafePointer($0)
                        return box.runtime.functions.effectParameterSetValueString(
                            handle, view)
                    }
                },
                operation: "cna_effect_parameter_set_value_string")
        }

        /// `SetValue(Texture value)` — the **base** overload, which is the one
        /// with no getter on either side.
        public func SetValue(_ value: Texture) throws {
            try setTexture(.base, value)
        }

        private func setTexture(_ tag: TextureTag, _ value: Texture?) throws {
            let handle = try box.validated("EffectParameter.SetValue")
            let texture = try value.map {
                try $0.validatedHandle("EffectParameter.SetValue")
            } ?? 0
            try box.runtime.functions.check(
                box.runtime.functions.effectParameterSetValueTexture(
                    handle, tag.rawValue, texture),
                operation: "cna_effect_parameter_set_value_texture")
        }

        // ------------------------------------------------------------------
        // GetValue: twenty-three readers.

        /// `GetValueBoolean()`.
        public func GetValueBoolean() throws -> Bool {
            try readScalar(.boolean, UInt8(0)) != 0
        }
        /// `GetValueBooleanArray(Int32 count)`.
        public func GetValueBooleanArray(_ count: Int32) throws -> [Bool] {
            try readArray(.boolean, count: count, UInt8(0)).map { $0 != 0 }
        }
        /// `GetValueInt32()`.
        public func GetValueInt32() throws -> Int32 { try readScalar(.int32, Int32(0)) }
        /// `GetValueInt32Array(Int32 count)`.
        public func GetValueInt32Array(_ count: Int32) throws -> [Int32] {
            try readArray(.int32, count: count, Int32(0))
        }
        /// `GetValueSingle()`.
        public func GetValueSingle() throws -> Float { try readScalar(.single, Float(0)) }
        /// `GetValueSingleArray(Int32 count)`.
        public func GetValueSingleArray(_ count: Int32) throws -> [Float] {
            try readArray(.single, count: count, Float(0))
        }

        /// `GetValueVector2()`.
        public func GetValueVector2() throws -> Microsoft.Xna.Framework.Vector2 {
            let value = try readScalar(.vector2, CNASwift_Vector2())
            return Microsoft.Xna.Framework.Vector2(value.x, value.y)
        }
        /// `GetValueVector2Array(Int32 count)`.
        public func GetValueVector2Array(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Vector2] {
            try readArray(.vector2, count: count, CNASwift_Vector2())
                .map { Microsoft.Xna.Framework.Vector2($0.x, $0.y) }
        }
        /// `GetValueVector3()`.
        public func GetValueVector3() throws -> Microsoft.Xna.Framework.Vector3 {
            let value = try readScalar(.vector3, CNASwift_Vector3())
            return Microsoft.Xna.Framework.Vector3(value.x, value.y, value.z)
        }
        /// `GetValueVector3Array(Int32 count)`.
        public func GetValueVector3Array(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Vector3] {
            try readArray(.vector3, count: count, CNASwift_Vector3())
                .map { Microsoft.Xna.Framework.Vector3($0.x, $0.y, $0.z) }
        }
        /// `GetValueVector4()`.
        public func GetValueVector4() throws -> Microsoft.Xna.Framework.Vector4 {
            let value = try readScalar(.vector4, CNASwift_Vector4())
            return Microsoft.Xna.Framework.Vector4(value.x, value.y, value.z, value.w)
        }
        /// `GetValueVector4Array(Int32 count)`.
        public func GetValueVector4Array(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Vector4] {
            try readArray(.vector4, count: count, CNASwift_Vector4())
                .map { Microsoft.Xna.Framework.Vector4($0.x, $0.y, $0.z, $0.w) }
        }
        /// `GetValueQuaternion()`.
        public func GetValueQuaternion() throws -> Microsoft.Xna.Framework.Quaternion {
            let value = try readScalar(.quaternion, CNASwift_Quaternion())
            return Microsoft.Xna.Framework.Quaternion(value.x, value.y, value.z, value.w)
        }
        /// `GetValueQuaternionArray(Int32 count)`.
        public func GetValueQuaternionArray(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Quaternion] {
            try readArray(.quaternion, count: count, CNASwift_Quaternion())
                .map { Microsoft.Xna.Framework.Quaternion($0.x, $0.y, $0.z, $0.w) }
        }

        /// `GetValueMatrix()`.
        public func GetValueMatrix() throws -> Microsoft.Xna.Framework.Matrix {
            Microsoft.Xna.Framework.Graphics.matrix(
                from: try readScalar(.matrix, CNASwift_Matrix()))
        }
        /// `GetValueMatrixArray(Int32 count)`.
        public func GetValueMatrixArray(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Matrix] {
            try readArray(.matrix, count: count, CNASwift_Matrix())
                .map { Microsoft.Xna.Framework.Graphics.matrix(from: $0) }
        }
        /// `GetValueMatrixTranspose()` — the same parameter through the
        /// transposed tag, not a second storage location.
        public func GetValueMatrixTranspose() throws -> Microsoft.Xna.Framework.Matrix {
            Microsoft.Xna.Framework.Graphics.matrix(
                from: try readScalar(.matrixTranspose, CNASwift_Matrix()))
        }
        /// `GetValueMatrixTransposeArray(Int32 count)`.
        public func GetValueMatrixTransposeArray(_ count: Int32) throws
            -> [Microsoft.Xna.Framework.Matrix] {
            try readArray(.matrixTranspose, count: count, CNASwift_Matrix())
                .map { Microsoft.Xna.Framework.Graphics.matrix(from: $0) }
        }

        /// `GetValueString()`.
        public func GetValueString() throws -> String {
            let handle = try box.validated("EffectParameter.GetValueString")
            let functions = box.runtime.functions
            return try Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: "cna_effect_parameter_copy_value_string",
                byteCount: functions.effectParameterGetValueStringByteCount,
                copy: functions.effectParameterCopyValueString)
        }

        /// `GetValueTexture2D()`.
        ///
        /// **Optional**, and that is XNA's own: the getter reads a field that a
        /// parameter with no texture assigned leaves null. CNA answers
        /// `CNA_INVALID_HANDLE` for the same state.
        ///
        /// The object is resolved through the runtime's registry, because a
        /// handle is not an object and this ABI publishes no route back — the
        /// same rule `TextureCollection` reproduces. A texture set through this
        /// parameter is found; one CNA owns is not, and answers nil rather than
        /// a wrapper this binding does not own.
        public func GetValueTexture2D() throws -> Texture2D? {
            try texture(.twoDimensional) as? Texture2D
        }
        /// `GetValueTexture3D()`.
        public func GetValueTexture3D() throws -> Texture3D? {
            try texture(.threeDimensional) as? Texture3D
        }
        /// `GetValueTextureCube()`.
        public func GetValueTextureCube() throws -> TextureCube? {
            try texture(.cube) as? TextureCube
        }

        /// What this parameter was last given through the matching setter.
        ///
        /// Cached per texture identity for the reason every other object
        /// getter in this binding caches: `cna_effect_parameter_get_value_texture`
        /// answers a handle, and a handle cannot be turned back into the object
        /// a caller passed in.
        private var textures: [UInt32: Texture] = [:]

        private func texture(_ tag: TextureTag) throws -> Texture? {
            let handle = try box.validated("EffectParameter.GetValueTexture")
            var native: UInt64 = 0
            try box.runtime.functions.check(
                box.runtime.functions.effectParameterGetValueTexture(
                    handle, tag.rawValue, &native),
                operation: "cna_effect_parameter_get_value_texture")
            guard native != 0 else {
                textures[tag.rawValue] = nil
                return nil
            }
            guard let cached = textures[tag.rawValue],
                  (try? cached.validatedHandle("EffectParameter")) == native else {
                return nil
            }
            return cached
        }

        /// The typed texture setters XNA does not declare separately: its
        /// `SetValue(Texture)` is the only texture setter, and the *getter*
        /// side is what distinguishes 2D from cube from volume. The tag is
        /// chosen from the value's own type so the matching getter answers.
        internal func setTypedTexture(_ value: Texture) throws {
            let tag: TextureTag
            if value is TextureCube { tag = .cube }
            else if value is Texture3D { tag = .threeDimensional }
            else if value is Texture2D { tag = .twoDimensional }
            else { tag = .base }
            try setTexture(tag, value)
            textures[tag.rawValue] = value
        }
    }

    /// `Microsoft.Xna.Framework.Graphics.EffectParameterCollection`.
    ///
    /// Five members: the four every effect collection has, plus
    /// `GetParameterBySemantic`, which is the same managed scan against
    /// `Semantic` instead of `Name`.
    public final class EffectParameterCollection {
        private let cache: EffectElementCache<EffectParameter>

        internal init(handle: UInt64, runtime: RuntimeState) {
            let box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectParameterCollectionDestroy)
            cache = EffectElementCache(
                box: box,
                getCount: runtime.functions.effectParameterCollectionGetCount,
                getAt: runtime.functions.effectParameterCollectionGetAt,
                wrap: { EffectParameter(handle: $0, runtime: $1) })
        }

        /// `EffectParameterCollection.Count`.
        public var Count: Int32 { cache.count }

        /// `EffectParameterCollection.Item[Int32]` — a `subscript`, and null
        /// rather than a throw for an out-of-range index.
        public subscript(index: Int32) -> EffectParameter? {
            cache.element(at: index)
        }

        /// `EffectParameterCollection.Item[String]`.
        public subscript(name: String) -> EffectParameter? {
            cache.element(named: name) { $0.Name }
        }

        /// `EffectParameterCollection.GetParameterBySemantic(String semantic)`
        /// — the same managed scan against `Semantic` instead of `Name`.
        public func GetParameterBySemantic(_ semantic: String) -> EffectParameter? {
            cache.element(named: semantic) { $0.Semantic }
        }

        /// `EffectParameterCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<EffectParameter> {
            let elements = cache.all()
            return CNAEnumerator(expectedVersion: 0) { index, _ in
                index < elements.count ? elements[index] : nil
            }
        }
    }
}
