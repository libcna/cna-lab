// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelBone`.
    ///
    /// **No public constructor, exactly as the CLR type has none.** A bone
    /// arrives from a `Model`, and a `Model` arrives from the content
    /// pipeline, so this projection adopts a handle CNA made -- the pattern
    /// `Texture2D.adoptLoaded(handle:runtime:)` established at Foundation 87.
    ///
    /// **Every getter here is infallible, and that is read out of the IL
    /// rather than chosen.** The first draft of this file made them all
    /// `get throws` on the reasoning that a native route can fail, and the
    /// accessor gate named all five: a CLR getter with no failure path must
    /// not become a Swift reader that can refuse. A route failure therefore
    /// answers the same thing an absent value does, because an infallible
    /// getter has no third answer available to it.
    public final class ModelBone {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private var storedTransform: Microsoft.Xna.Framework.Matrix?

        /// Adopts a handle CNA made. `owned` is false for a bone reached
        /// THROUGH another object -- a parent, a child, a collection entry --
        /// because destroying one of those would take it from under the owner
        /// that still holds it.
        internal init(handle: UInt64, runtime: RuntimeState, owned: Bool) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelBone",
                ownership: owned ? .owned : .borrowed, runtime: runtime,
                destroy: runtime.functions.modelBoneDestroy)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// The failure a setter had no way to report, kept rather than thrown.
        internal private(set) var lastPushFailure: Error?

        /// `ModelBone.Name`.
        public var Name: String {
            (try? ModelSupport.copyString(
                runtime.functions, nativeHandle,
                { h, out in self.runtime.functions.modelBoneGetNameByteCount(h, out) },
                { h, dst, cap, out in
                    self.runtime.functions.modelBoneCopyName(h, dst, cap, out)
                },
                "cna_model_bone_get_name_byte_count",
                "cna_model_bone_copy_name")) ?? ""
        }

        /// `ModelBone.Index`.
        public var Index: Int32 {
            var value: Int32 = 0
            guard runtime.functions.modelBoneGetIndex(nativeHandle, &value) == 0 else {
                return 0
            }
            return value
        }

        /// `ModelBone.Transform`, the family's one settable property.
        ///
        /// Settable, infallible on both sides, and crossing the boundary --
        /// which is the third accessor shape this binding recognises: the
        /// value is stored and pushed, and a push that fails is kept in
        /// `lastPushFailure` because the setter has nowhere to report it.
        /// `TouchPanel.DisplayWidth` is the same shape.
        public var Transform: Microsoft.Xna.Framework.Matrix {
            get {
                var native = CNASwift_Matrix()
                guard runtime.functions.modelBoneGetTransform(nativeHandle, &native) == 0
                else { return storedTransform ?? .Identity }
                return ModelSupport.matrix(from: native)
            }
            set {
                storedTransform = newValue
                let result = runtime.functions.modelBoneSetTransform(
                    nativeHandle, ModelSupport.native(newValue))
                if result != 0 {
                    lastPushFailure = CNAError.nativeFailure(
                        operation: "cna_model_bone_set_transform",
                        result: result,
                        message: "the bone kept the value the caller set and "
                            + "the runtime did not take it")
                }
            }
        }

        /// `ModelBone.Parent`.
        ///
        /// Optional because the root bone has none, which CNA reports through
        /// a separate `out_has_parent` rather than a null handle -- the
        /// two-answer shape `Song`'s relation routes use.
        public var Parent: ModelBone? {
            var has: UInt8 = 0
            var produced: UInt64 = 0
            guard runtime.functions.modelBoneGetParent(nativeHandle, &has, &produced) == 0,
                  has != 0 else { return nil }
            return ModelBone(handle: produced, runtime: runtime,
                             owned: false)
        }

        /// `ModelBone.Children`.
        public var Children: ModelBoneCollection? {
            var produced: UInt64 = 0
            guard runtime.functions.modelBoneGetChildren(nativeHandle, &produced) == 0
            else { return nil }
            return ModelBoneCollection(handle: produced, runtime: runtime)
        }
    }
}
