// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelMeshPart`.
    ///
    /// Eight properties and no methods. Seven are get-only in the CLR even
    /// though CNA publishes a setter for each: `cna_model_mesh_part_set_*`
    /// exists for `NumVertices`, `PrimitiveCount`, `StartIndex` and
    /// `VertexOffset`, and none of them is bound here, because the Foundation
    /// 67 rule is that a route with no consuming member is not bound. XNA
    /// makes those four read-only and so does this.
    public final class ModelMeshPart {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private var storedTag: Any?

        internal init(handle: UInt64, runtime: RuntimeState, owned: Bool) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelMeshPart",
                ownership: owned ? .owned : .borrowed, runtime: runtime,
                destroy: runtime.functions.modelMeshPartDestroy)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelMeshPart.NumVertices`.
        public var NumVertices: Int32 { readInt32 {
            self.runtime.functions.modelMeshPartGetNumVertices(
                self.nativeHandle, $0) } }

        /// `ModelMeshPart.PrimitiveCount`.
        public var PrimitiveCount: Int32 { readInt32 {
            self.runtime.functions.modelMeshPartGetPrimitiveCount(
                self.nativeHandle, $0) } }

        /// `ModelMeshPart.StartIndex`.
        public var StartIndex: Int32 { readInt32 {
            self.runtime.functions.modelMeshPartGetStartIndex(
                self.nativeHandle, $0) } }

        /// `ModelMeshPart.VertexOffset`.
        public var VertexOffset: Int32 { readInt32 {
            self.runtime.functions.modelMeshPartGetVertexOffset(
                self.nativeHandle, $0) } }

        /// `ModelMeshPart.VertexBuffer`.
        public var VertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer? {
            var has: UInt8 = 0
            var produced: UInt64 = 0
            guard runtime.functions.modelMeshPartGetVertexBuffer(
                nativeHandle, &has, &produced) == 0, has != 0 else { return nil }
            return ModelSupport.adoptVertexBuffer(produced, runtime)
        }

        /// `ModelMeshPart.IndexBuffer`.
        public var IndexBuffer: Microsoft.Xna.Framework.Graphics.IndexBuffer? {
            var has: UInt8 = 0
            var produced: UInt64 = 0
            guard runtime.functions.modelMeshPartGetIndexBuffer(
                nativeHandle, &has, &produced) == 0, has != 0 else { return nil }
            return ModelSupport.adoptIndexBuffer(produced, runtime)
        }

        /// `ModelMeshPart.Effect`, settable.
        public var Effect: Microsoft.Xna.Framework.Graphics.Effect? {
            get {
                var has: UInt8 = 0
                var produced: UInt64 = 0
                guard runtime.functions.modelMeshPartGetEffect(
                    nativeHandle, &has, &produced) == 0, has != 0
                else { return nil }
                return Microsoft.Xna.Framework.Graphics.Effect(
                    handle: produced, runtime: runtime, device: nil,
                    typeName: "Effect")
            }
            set {
                let result = runtime.functions.modelMeshPartSetEffect(
                    nativeHandle, (try? newValue?.validatedHandle(
                        "cna_model_mesh_part_set_effect")) ?? 0)
                if result != 0 {
                    lastPushFailure = CNAError.nativeFailure(
                        operation: "cna_model_mesh_part_set_effect",
                        result: result,
                        message: "the part kept the effect the caller set and "
                            + "the runtime did not take it")
                }
            }
        }

        /// `ModelMeshPart.Tag`, settable.
        ///
        /// **Held on the Swift side.** The CLR property is `Object` and
        /// `CNA_ModelMeshPartTag` is a `uint64_t` token; an arbitrary Swift
        /// object and an opaque integer cannot round-trip through each other
        /// without a side table this projection has no reason to keep. Storing
        /// it here reproduces the whole observable contract -- set it, read it
        /// back -- and the tag routes stay unbound under the Foundation 67
        /// rule. When `ContentManager.Load<Model>` is wired, a tag the pipeline
        /// set natively will need reconciling with this.
        public var Tag: Any? {
            get { storedTag }
            set { storedTag = newValue }
        }

        /// The failure a setter had no way to report, kept rather than thrown.
        internal private(set) var lastPushFailure: Error?

        private func readInt32(
            _ read: (UnsafeMutablePointer<Int32>?) -> UInt32
        ) -> Int32 {
            var value: Int32 = 0
            guard read(&value) == 0 else { return 0 }
            return value
        }
    }
}
