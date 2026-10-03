// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelMesh`.
    public final class ModelMesh {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private var storedTag: Any?

        internal init(handle: UInt64, runtime: RuntimeState, owned: Bool) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelMesh",
                ownership: owned ? .owned : .borrowed, runtime: runtime,
                destroy: runtime.functions.modelMeshDestroy)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelMesh.Name`.
        public var Name: String {
            (try? ModelSupport.copyString(
                runtime.functions, nativeHandle,
                { h, out in self.runtime.functions.modelMeshGetNameByteCount(h, out) },
                { h, dst, cap, out in
                    self.runtime.functions.modelMeshCopyName(h, dst, cap, out)
                },
                "cna_model_mesh_get_name_byte_count",
                "cna_model_mesh_copy_name")) ?? ""
        }

        /// `ModelMesh.ParentBone`.
        ///
        /// **Not Optional.** Whether XNA can return null here is not proven
        /// from the IL, and the return-nullability rule sends an unproven
        /// reference to a non-Optional -- a projection that offered nil would
        /// be claiming a possibility the metadata does not establish. An
        /// infallible getter with nothing to hand back therefore traps.
        public var ParentBone: ModelBone {
            var has: UInt8 = 0
            var produced: UInt64 = 0
            guard runtime.functions.modelMeshGetParentBone(
                      nativeHandle, &has, &produced) == 0, has != 0 else {
                preconditionFailure(
                    "the mesh has no parent bone; XNA's metadata does not "
                    + "establish that this can happen and CNA reported it")
            }
            return ModelBone(handle: produced, runtime: runtime, owned: false)
        }

        /// `ModelMesh.BoundingSphere`.
        ///
        /// The one place the family hands back a shape rather than a scalar or
        /// a handle, which is why `CNASwift_BoundingSphere` is mirrored at all.
        public var BoundingSphere: Microsoft.Xna.Framework.BoundingSphere {
            var native = CNASwift_BoundingSphere()
            guard runtime.functions.modelMeshGetBoundingSphere(
                      nativeHandle, &native) == 0,
                  let sphere = try? Microsoft.Xna.Framework.BoundingSphere(
                      Microsoft.Xna.Framework.Vector3(
                          native.center.x, native.center.y, native.center.z),
                      native.radius)
            else { return ModelMesh.emptySphere }
            return sphere
        }

        /// What an infallible getter answers when the route fails or the
        /// runtime hands back a radius `BoundingSphere` itself refuses.
        /// A zero sphere is the one value that initializer cannot reject, so
        /// building it here cannot fail either.
        private static let emptySphere =
            (try? Microsoft.Xna.Framework.BoundingSphere(
                Microsoft.Xna.Framework.Vector3.Zero, 0))!

        /// `ModelMesh.MeshParts`.
        /// **Not Optional:** XNA cannot normally return null here, so an
        /// Optional would offer a nil a caller can never meet.
        public var MeshParts: ModelMeshPartCollection {
            var produced: UInt64 = 0
            guard runtime.functions.modelMeshGetMeshParts(nativeHandle, &produced) == 0 else {
                preconditionFailure("cna_model_mesh_get_mesh_parts refused a live mesh")
            }
            return ModelMeshPartCollection(handle: produced, runtime: runtime)
        }

        /// `ModelMesh.Effects`.
        /// **Not Optional:** XNA cannot normally return null here, so an
        /// Optional would offer a nil a caller can never meet.
        public var Effects: ModelEffectCollection {
            var produced: UInt64 = 0
            guard runtime.functions.modelMeshGetEffects(nativeHandle, &produced) == 0 else {
                preconditionFailure("cna_model_mesh_get_effects refused a live mesh")
            }
            return ModelEffectCollection(handle: produced, runtime: runtime)
        }

        /// `ModelMesh.Tag`, held Swift-side for the reason `ModelMeshPart.Tag`
        /// records: the CLR property is `Object` and the native tag is an
        /// opaque `uint64_t`.
        public var Tag: Any? {
            get { storedTag }
            set { storedTag = newValue }
        }

        /// `ModelMesh.Draw()`.
        ///
        /// XNA walks the parts first and raises `InvalidOperationException`
        /// for the first one carrying no effect. The message-coverage gate is
        /// what asked for this: forwarding straight to `cna_model_mesh_draw`
        /// left a message XNA raises neither reproduced nor recorded.
        public func Draw() throws {
            let parts = MeshParts
            for index in 0..<parts.Count where
                (try? parts.Items.Item(index))?.Effect == nil {
                throw CNAInvalidOperationException(
                    message: ModelSupport.modelHasNoEffect)
            }
            try runtime.functions.check(
                runtime.functions.modelMeshDraw(nativeHandle),
                operation: "cna_model_mesh_draw")
        }
    }
}
