// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelMeshPartCollection`.
    ///
    /// The plainest of the four: `ReadOnlyCollection<ModelMeshPart>` plus a
    /// struct enumerator. No name lookup, because a mesh part has no name to
    /// look one up by.
    public final class ModelMeshPartCollection: CNAReadOnlyCollection<Microsoft.Xna.Framework.Graphics.ModelMeshPart> {

        private let storage: NativeHandleStorage

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelMeshPartCollection",
                ownership: .owned, runtime: runtime,
                destroy: runtime.functions.modelMeshPartCollectionDestroy)
            let list = CNAList<ModelMeshPart>()
            var count: UInt64 = 0
            if runtime.functions.modelMeshPartCollectionGetCount(
                handle, &count) == 0 {
                for index in 0..<count {
                    var produced: UInt64 = 0
                    guard runtime.functions.modelMeshPartCollectionGetAt(
                        handle, index, &produced) == 0 else { continue }
                    list.Add(ModelMeshPart(handle: produced, runtime: runtime,
                                           owned: false))
                }
            }
            super.init(list: list)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelMeshPartCollection.GetEnumerator()`.
        public func GetEnumerator() -> Enumerator { Enumerator(collection: self) }

        public struct Enumerator {
            private let collection: ModelMeshPartCollection
            private var position: Int32

            internal init(collection: ModelMeshPartCollection) {
                self.collection = collection
                position = -1
            }

            /// `Current` is INFALLIBLE in the CLR, so it cannot become a
            /// Swift reader that refuses. Read before the first `MoveNext` or
            /// after the last, it has nothing to return and traps -- the rule
            /// `GraphicsAdapter.DefaultAdapter` and `Game.Content` already
            /// follow. A first draft made it `get throws`, which the accessor
            /// gate named on all four enumerators at once.
            public var Current: Microsoft.Xna.Framework.Graphics.ModelMeshPart {
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
