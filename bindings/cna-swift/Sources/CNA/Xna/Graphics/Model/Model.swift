// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.Model`.
    ///
    /// **No public constructor, as the CLR type has none.** In XNA a model
    /// arrives from `ContentManager.Load<Model>`, which needs an `.xnb` this
    /// repository cannot yet produce. That is a limit on how a CONSUMER
    /// obtains one, not on whether the type can be projected: CNA publishes
    /// `cna_model_create_default`, `cna_model_create` and creates for every
    /// bone, mesh and part, so the surface is reachable and testable through
    /// the internal adoption this file provides.
    public final class Model {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private var storedTag: Any?

        internal init(handle: UInt64, runtime: RuntimeState, owned: Bool) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "Model",
                ownership: owned ? .owned : .borrowed, runtime: runtime,
                destroy: runtime.functions.modelDestroy)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `Model.Root`.
        public var Root: ModelBone? {
            var has: UInt8 = 0
            var produced: UInt64 = 0
            guard runtime.functions.modelGetRoot(
                nativeHandle, &has, &produced) == 0, has != 0 else { return nil }
            return ModelBone(handle: produced, runtime: runtime, owned: false)
        }

        /// `Model.Bones`.
        public var Bones: ModelBoneCollection? {
            var produced: UInt64 = 0
            guard runtime.functions.modelGetBones(nativeHandle, &produced) == 0
            else { return nil }
            return ModelBoneCollection(handle: produced, runtime: runtime)
        }

        /// `Model.Meshes`.
        public var Meshes: ModelMeshCollection? {
            var produced: UInt64 = 0
            guard runtime.functions.modelGetMeshes(nativeHandle, &produced) == 0
            else { return nil }
            return ModelMeshCollection(handle: produced, runtime: runtime)
        }

        /// `Model.Tag`, held Swift-side for the reason `ModelMeshPart.Tag`
        /// records.
        public var Tag: Any? {
            get { storedTag }
            set { storedTag = newValue }
        }

        /// `Model.CopyBoneTransformsTo(Matrix[] destinationBoneTransforms)`.
        ///
        /// The array is `inout` rather than returned, because the CLR method
        /// WRITES INTO the caller's array and raises when it is too small.
        /// Returning a fresh array would read more conveniently and would be a
        /// different method.
        public func CopyBoneTransformsTo(
            _ destinationBoneTransforms: inout [Microsoft.Xna.Framework.Matrix]
        ) throws {
            try copy(&destinationBoneTransforms, absolute: false)
        }

        /// `Model.CopyAbsoluteBoneTransformsTo(Matrix[] destinationBoneTransforms)`.
        public func CopyAbsoluteBoneTransformsTo(
            _ destinationBoneTransforms: inout [Microsoft.Xna.Framework.Matrix]
        ) throws {
            try copy(&destinationBoneTransforms, absolute: true)
        }

        /// `Model.CopyBoneTransformsFrom(Matrix[] sourceBoneTransforms)`.
        public func CopyBoneTransformsFrom(
            _ sourceBoneTransforms: [Microsoft.Xna.Framework.Matrix]
        ) throws {
            var native = sourceBoneTransforms.map { ModelSupport.native($0) }
            try runtime.functions.check(
                native.withUnsafeMutableBufferPointer { buffer in
                    runtime.functions.modelSetBoneTransforms(
                        nativeHandle, buffer.baseAddress, UInt64(buffer.count))
                },
                operation: "cna_model_set_bone_transforms")
        }

        /// `Model.Draw(Matrix world, Matrix view, Matrix projection)`.
        public func Draw(
            _ world: Microsoft.Xna.Framework.Matrix,
            view: Microsoft.Xna.Framework.Matrix,
            projection: Microsoft.Xna.Framework.Matrix
        ) throws {
            try refuseUndrawableParts()
            try runtime.functions.check(
                runtime.functions.modelDraw(
                    nativeHandle, ModelSupport.native(world),
                    ModelSupport.native(view), ModelSupport.native(projection)),
                operation: "cna_model_draw")
        }

        /// XNA's `Draw` raises before it draws anything: once for a part with
        /// no effect, and once for an effect that cannot take the three
        /// matrices it was handed. Both messages are the assembly's own.
        private func refuseUndrawableParts() throws {
            guard let meshes = Meshes else { return }
            for meshIndex in 0..<meshes.Count {
                guard let mesh = try? meshes.Items.Item(meshIndex) else { continue }
                let parts = mesh.MeshParts
                for partIndex in 0..<parts.Count {
                    guard let part = try? parts.Items.Item(partIndex) else { continue }
                    guard let effect = part.Effect else {
                        throw CNAInvalidOperationException(
                            message: ModelSupport.modelHasNoEffect)
                    }
                    guard effect is Microsoft.Xna.Framework.Graphics.IEffectMatrices
                    else {
                        throw CNAInvalidOperationException(
                            message: ModelSupport.modelHasNoIEffectMatrices)
                    }
                }
            }
        }

        private func copy(
            _ destination: inout [Microsoft.Xna.Framework.Matrix],
            absolute: Bool
        ) throws {
            var native = [CNASwift_Matrix](
                repeating: CNASwift_Matrix(),
                count: max(destination.count, 1))
            var written: UInt64 = 0
            try runtime.functions.check(
                native.withUnsafeMutableBufferPointer { buffer in
                    absolute
                        ? runtime.functions.modelCopyAbsoluteBoneTransforms(
                            nativeHandle, buffer.baseAddress,
                            UInt64(destination.count), &written)
                        : runtime.functions.modelCopyBoneTransforms(
                            nativeHandle, buffer.baseAddress,
                            UInt64(destination.count), &written)
                },
                operation: absolute
                    ? "cna_model_copy_absolute_bone_transforms"
                    : "cna_model_copy_bone_transforms")
            for index in 0..<Int(written) where index < destination.count {
                destination[index] = ModelSupport.matrix(from: native[index])
            }
        }
    }
}
