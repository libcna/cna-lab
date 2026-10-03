// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelBoneCollection`.
    ///
    /// Derives from `CNAReadOnlyCollection<ModelBone>`, which is what the CLR
    /// type does and what the base-mapping gate insists on -- a first draft
    /// was a standalone class reading its elements live through the handle,
    /// and the gate named the missing base immediately.
    ///
    /// The elements are materialised once, at construction. That is not a
    /// compromise: XNA builds this collection from a list when the model is
    /// loaded and never mutates it, so a snapshot IS the contract.
    public final class ModelBoneCollection: CNAReadOnlyCollection<Microsoft.Xna.Framework.Graphics.ModelBone> {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelBoneCollection",
                ownership: .owned, runtime: runtime,
                destroy: runtime.functions.modelBoneCollectionDestroy)
            let list = CNAList<ModelBone>()
            var count: UInt64 = 0
            if runtime.functions.modelBoneCollectionGetCount(handle, &count) == 0 {
                for index in 0..<count {
                    var produced: UInt64 = 0
                    guard runtime.functions.modelBoneCollectionGetAt(
                        handle, index, &produced) == 0 else { continue }
                    list.Add(ModelBone(handle: produced, runtime: runtime,
                                       owned: false))
                }
            }
            super.init(list: list)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelBoneCollection.Item[String boneName]`.
        public subscript(boneName: String) -> ModelBone {
            get throws {
                guard let found = lookup(boneName) else {
                    throw CNAKeyNotFoundException()
                }
                return found
            }
        }

        /// `ModelBoneCollection.TryGetValue(String boneName, out ModelBone value)`.
        ///
        /// The CLR `out` projects as `inout` and the Bool stays the return.
        /// A first draft folded both into an Optional, on the reasoning that
        /// Swift has one and the CLR does not -- the mapping rules say
        /// otherwise, and `Model.CopyBoneTransformsTo` is the same shape: a
        /// method that WRITES INTO the caller's storage is not the same method
        /// as one that hands back a value.
        public func TryGetValue(
            _ boneName: String, value: inout Microsoft.Xna.Framework.Graphics.ModelBone
        ) -> Bool {
            guard let found = lookup(boneName) else { return false }
            value = found
            return true
        }

        /// `ModelBoneCollection.GetEnumerator()`.
        public func GetEnumerator() -> Enumerator {
            Enumerator(collection: self)
        }

        private func lookup(_ name: String) -> ModelBone? {
            var utf8 = Array(name.utf8)
            var found: UInt8 = 0
            var produced: UInt64 = 0
            let live = nativeHandle
            let result = ModelSupport.withStringView(&utf8) { view in
                self.runtime.functions.modelBoneCollectionFind(
                    live, view, &found, &produced)
            }
            guard result == 0, found != 0 else { return nil }
            return ModelBone(handle: produced, runtime: runtime, owned: false)
        }

        /// The pinned public nested struct: `MoveNext`, `Dispose` and a
        /// get-only `Current`. `Reset` in the IL is a private explicit
        /// interface implementation and is not public contract.
        public struct Enumerator {
            private let collection: ModelBoneCollection
            private var position: Int32

            internal init(collection: ModelBoneCollection) {
                self.collection = collection
                position = -1
            }

            /// `Current` is INFALLIBLE in the CLR, so it cannot become a
            /// Swift reader that refuses. Read before the first `MoveNext` or
            /// after the last, it has nothing to return and traps -- the rule
            /// `GraphicsAdapter.DefaultAdapter` and `Game.Content` already
            /// follow. A first draft made it `get throws`, which the accessor
            /// gate named on all four enumerators at once.
            public var Current: Microsoft.Xna.Framework.Graphics.ModelBone {
                guard let value = try? collection.Items.Item(position) else {
                    preconditionFailure(
                        "Current was read outside the enumeration; MoveNext "
                        + "must have returned true first")
                }
                return value
            }

            public mutating func MoveNext() -> Bool {
                position += 1
                if position < collection.Count { return true }
                position = collection.Count
                return false
            }

            public func Dispose() {}
        }
    }
}
