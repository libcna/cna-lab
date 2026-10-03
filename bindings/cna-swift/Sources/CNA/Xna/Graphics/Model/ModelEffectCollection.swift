// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.ModelEffectCollection`.
    ///
    /// `ReadOnlyCollection<Effect>` plus a struct enumerator. CNA publishes
    /// `add` and `remove` for this collection and neither is bound: the CLR
    /// type is read-only and the Foundation 67 rule is that a route with no
    /// consuming member is not bound.
    public final class ModelEffectCollection: CNAReadOnlyCollection<Microsoft.Xna.Framework.Graphics.Effect> {

        private let storage: NativeHandleStorage

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.storage = NativeHandleStorage(
                handle: handle, typeName: "ModelEffectCollection",
                ownership: .owned, runtime: runtime,
                destroy: runtime.functions.modelEffectCollectionDestroy)
            let list = CNAList<Effect>()
            var count: UInt64 = 0
            if runtime.functions.modelEffectCollectionGetCount(
                handle, &count) == 0 {
                for index in 0..<count {
                    var produced: UInt64 = 0
                    guard runtime.functions.modelEffectCollectionGetAt(
                        handle, index, &produced) == 0 else { continue }
                    list.Add(Effect(handle: produced, runtime: runtime,
                                    device: nil, typeName: "Effect"))
                }
            }
            super.init(list: list)
        }

        internal var nativeHandle: UInt64 { storage.handle }

        /// `ModelEffectCollection.GetEnumerator()`.
        public func GetEnumerator() -> Enumerator { Enumerator(collection: self) }

        public struct Enumerator {
            private let collection: ModelEffectCollection
            private var position: Int32

            internal init(collection: ModelEffectCollection) {
                self.collection = collection
                position = -1
            }

            /// `Current` is INFALLIBLE in the CLR, so it cannot become a
            /// Swift reader that refuses. Read before the first `MoveNext` or
            /// after the last, it has nothing to return and traps -- the rule
            /// `GraphicsAdapter.DefaultAdapter` and `Game.Content` already
            /// follow. A first draft made it `get throws`, which the accessor
            /// gate named on all four enumerators at once.
            public var Current: Microsoft.Xna.Framework.Graphics.Effect {
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
