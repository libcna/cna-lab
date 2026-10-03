// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelMeshCollection`.
    ///
    /// The bone collection's twin: `ReadOnlyCollection<ModelMesh>` with a name
    /// lookup, a `TryGetValue` and a struct enumerator.
    public final class ModelMeshCollection: CNAReadOnlyCollection<Microsoft.Xna.Framework.Graphics.ModelMesh> {

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelMeshCollection",
                ownership: .owned, runtime: runtime,
                destroy: runtime.functions.modelMeshCollectionDestroy)
            let list = CNAList<ModelMesh>()
            var count: UInt64 = 0
            if runtime.functions.modelMeshCollectionGetCount(handle, &count) == 0 {
                for index in 0..<count {
                    var produced: UInt64 = 0
                    guard runtime.functions.modelMeshCollectionGetAt(
                        handle, index, &produced) == 0 else { continue }
                    list.Add(ModelMesh(handle: produced, runtime: runtime,
                                       owned: false))
                }
            }
            super.init(list: list)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelMeshCollection.Item[String meshName]`.
        public subscript(meshName: String) -> ModelMesh {
            get throws {
                guard let found = lookup(meshName) else {
                    throw CNAKeyNotFoundException()
                }
                return found
            }
        }

        /// `ModelMeshCollection.TryGetValue(String meshName, out ModelMesh value)`.
        ///
        /// The CLR `out` projects as `inout` and the Bool stays the return.
        /// A first draft folded both into an Optional, on the reasoning that
        /// Swift has one and the CLR does not -- the mapping rules say
        /// otherwise, and `Model.CopyBoneTransformsTo` is the same shape: a
        /// method that WRITES INTO the caller's storage is not the same method
        /// as one that hands back a value.
        public func TryGetValue(
            _ meshName: String, value: inout Microsoft.Xna.Framework.Graphics.ModelMesh
        ) -> Bool {
            guard let found = lookup(meshName) else { return false }
            value = found
            return true
        }

        /// `ModelMeshCollection.GetEnumerator()`.
        public func GetEnumerator() -> Enumerator { Enumerator(collection: self) }

        private func lookup(_ name: String) -> ModelMesh? {
            var utf8 = Array(name.utf8)
            var found: UInt8 = 0
            var produced: UInt64 = 0
            let live = nativeHandle
            let result = ModelSupport.withStringView(&utf8) { view in
                self.runtime.functions.modelMeshCollectionFind(
                    live, view, &found, &produced)
            }
            guard result == 0, found != 0 else { return nil }
            return ModelMesh(handle: produced, runtime: runtime, owned: false)
        }

        public struct Enumerator {
            private let collection: ModelMeshCollection
            private var position: Int32

            internal init(collection: ModelMeshCollection) {
                self.collection = collection
                position = -1
            }

            /// `Current` is INFALLIBLE in the CLR, so it cannot become a
            /// Swift reader that refuses. Read before the first `MoveNext` or
            /// after the last, it has nothing to return and traps -- the rule
            /// `GraphicsAdapter.DefaultAdapter` and `Game.Content` already
            /// follow. A first draft made it `get throws`, which the accessor
            /// gate named on all four enumerators at once.
            public var Current: Microsoft.Xna.Framework.Graphics.ModelMesh {
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
