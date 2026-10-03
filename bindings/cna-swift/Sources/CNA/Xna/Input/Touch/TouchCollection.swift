// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned as `.class public sequential ansi sealed beforefieldinit ...
    // extends [mscorlib]System.ValueType implements IList<TouchLocation>,
    // ICollection<TouchLocation>, IEnumerable<TouchLocation>, IEnumerable`.
    //
    // The pinned storage is eight *inline* TouchLocation fields, `location0`
    // through `location7`, plus `locationCount` and `isConnected`. Eight is a
    // hard capacity: the constructor rejects a longer array outright, and the
    // private `AddTouchLocation` helper silently returns past slot seven. The
    // Swift projection holds the same eight-element bound in an array; the
    // collection is immutable after construction -- every mutating member
    // throws -- so array copy-on-write is observationally identical to the
    // inline CLR fields.
    //
    // The type also has a private *static* `prevLocations` field and an
    // `assembly` `Update` entry point that drive the touch-panel frame state
    // machine. No public member reads either, which is what makes this type
    // purely managed. Completing it claims no touch capability: `TouchPanel`
    // is not implemented, so nothing produces a live collection.
    public struct TouchCollection {
        private var locations: [Microsoft.Xna.Framework.Input.Touch.TouchLocation]
        private let isConnected: Bool

        // The pinned constructor rejects a null array and any array longer
        // than eight, then rebuilds each entry through the seven-field
        // TouchLocation constructor rather than storing the supplied value:
        // when a touch carries a previous location the previous state and
        // position are carried across, and when it does not they are reset to
        // Invalid/+0/+0. `isConnected` is set to true; only the default
        // `TouchCollection` value has it false.
        //
        // Swift arrays are non-optional, so the CLR null check has no Swift
        // counterpart and the ArgumentNullException path is unreachable here.
        public init(_ touches: [Microsoft.Xna.Framework.Input.Touch.TouchLocation]) throws {
            guard touches.count <= 8 else {
                throw CNAArgumentOutOfRangeException(paramName: "touches")
            }
            isConnected = true
            var rebuilt: [Microsoft.Xna.Framework.Input.Touch.TouchLocation] = []
            rebuilt.reserveCapacity(touches.count)
            for touch in touches {
                var previous = Microsoft.Xna.Framework.Input.Touch.TouchLocation(
                    id: 0, state: .Invalid, x: 0, y: 0,
                    prevState: .Invalid, prevX: 0, prevY: 0)
                let hasPrevious = touch.TryGetPreviousLocation(&previous)
                rebuilt.append(Microsoft.Xna.Framework.Input.Touch.TouchLocation(
                    id: touch.Id,
                    state: touch.State,
                    x: touch.Position.X,
                    y: touch.Position.Y,
                    prevState: hasPrevious ? previous.State : .Invalid,
                    prevX: hasPrevious ? previous.Position.X : 0,
                    prevY: hasPrevious ? previous.Position.Y : 0))
            }
            locations = rebuilt
        }

        // The default CLR value of a struct is all-zero: no locations and
        // `isConnected` false. Internal construction infrastructure, not a
        // public XNA identity -- the pinned type declares only the array
        // constructor.
        internal init() {
            locations = []
            isConnected = false
        }

        public var IsConnected: Bool { isConnected }

        public var Count: Int32 { Int32(locations.count) }

        // Constant `ldc.i4.1`: the collection is always read-only, which is
        // why every mutating member below throws.
        public var IsReadOnly: Bool { true }

        // `get_Item` validates `index < 0 || index >= Count` and throws
        // ArgumentOutOfRangeException("index"); the switch that follows can
        // only be reached with a valid index.
        public func Item(_ index: Int32) throws -> Microsoft.Xna.Framework.Input.Touch.TouchLocation {
            guard index >= 0 && index < Count else {
                throw CNAArgumentOutOfRangeException(paramName: "index")
            }
            return locations[Int(index)]
        }

        // `set_Item` is declared in the metadata, so `Item` is a read/write
        // indexed property, but its entire body is `throw new
        // NotSupportedException()`. The writer is projected as the throwing
        // SetItem method the indexed-property rule requires, and it always
        // throws.
        public func SetItem(
            _ index: Int32,
            _ value: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) throws {
            throw CNANotSupportedException()
        }

        // Scans by `Id` and, on a match, writes that location out and returns
        // true. When no location matches, XNA still writes the out parameter,
        // with `default(TouchLocation)` -- every field zeroed, so `Id` is 0,
        // not -1 -- and returns false.
        public func FindById(
            _ id: Int32,
            touchLocation: inout Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) -> Bool {
            for location in locations where location.Id == id {
                touchLocation = location
                return true
            }
            touchLocation = Microsoft.Xna.Framework.Input.Touch.TouchLocation(
                id: 0, state: .Invalid, x: 0, y: 0,
                prevState: .Invalid, prevX: 0, prevY: 0)
            return false
        }

        // A linear scan comparing with `TouchLocation::op_Equality`, which is
        // the strict comparison that includes both states -- not the typed
        // `Equals`, which ignores them. Returns -1 when absent.
        public func IndexOf(
            _ item: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) -> Int32 {
            for index in 0..<locations.count where locations[index] == item {
                return Int32(index)
            }
            return -1
        }

        public func Contains(
            _ item: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) -> Bool {
            IndexOf(item) >= 0
        }

        // Validates a null array, a negative `arrayIndex`, and that the
        // destination is long enough. The length check is computed in 64-bit
        // in the pinned IL so `arrayIndex + Count` cannot overflow, and it
        // reports the failure on "arrayIndex" rather than on "array".
        public func CopyTo(
            _ array: inout [Microsoft.Xna.Framework.Input.Touch.TouchLocation],
            arrayIndex: Int32
        ) throws {
            guard arrayIndex >= 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "arrayIndex")
            }
            guard Int64(array.count) >= Int64(arrayIndex) + Int64(Count) else {
                throw CNAArgumentOutOfRangeException(paramName: "arrayIndex")
            }
            for index in 0..<locations.count {
                array[Int(arrayIndex) + index] = locations[index]
            }
        }

        // Every mutating member of the pinned IList implementation is exactly
        // `newobj NotSupportedException::.ctor(); throw` with no message and
        // no validation, so none of them inspects its arguments first. The
        // parameterless constructor substitutes Arg_NotSupportedException,
        // which is a different message from the NotSupported_ReadOnlyCollection
        // that Collection<T>'s own read-only guard raises.
        public func Add(
            _ item: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) throws {
            throw CNANotSupportedException()
        }

        public func Insert(
            _ index: Int32,
            item: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) throws {
            throw CNANotSupportedException()
        }

        public func RemoveAt(_ index: Int32) throws {
            throw CNANotSupportedException()
        }

        public func Remove(
            _ item: Microsoft.Xna.Framework.Input.Touch.TouchLocation
        ) throws -> Bool {
            throw CNANotSupportedException()
        }

        public func Clear() throws {
            throw CNANotSupportedException()
        }

        // `GetEnumerator` copies the whole collection into the enumerator by
        // value, so the cursor walks a snapshot. It returns the pinned nested
        // struct rather than the CNAEnumerator support type, because the
        // contract declares that concrete return type.
        public func GetEnumerator() -> Enumerator {
            Enumerator(collection: self)
        }

        // Pinned as a public nested sequential struct implementing
        // IEnumerator<TouchLocation>, IDisposable and IEnumerator, whose only
        // constructor is `assembly`. Its public contract is exactly MoveNext,
        // Dispose and the get-only Current; the `Reset` in the IL is a private
        // explicit interface implementation and is not public contract.
        public struct Enumerator {
            private let collection: Microsoft.Xna.Framework.Input.Touch.TouchCollection
            private var position: Int32

            // The pinned `assembly` constructor: stores the collection by
            // value and starts the cursor before the first element.
            internal init(collection: Microsoft.Xna.Framework.Input.Touch.TouchCollection) {
                self.collection = collection
                position = -1
            }

            // `get_Current` simply forwards to `collection[position]`, so
            // reading it before the first MoveNext, or after the last one,
            // throws the indexer's ArgumentOutOfRangeException rather than
            // returning a default.
            public var Current: Microsoft.Xna.Framework.Input.Touch.TouchLocation {
                get throws {
                    try collection.Item(position)
                }
            }

            // Advances, then clamps to Count when the end is passed, so
            // repeated calls after exhaustion keep returning false without the
            // cursor running away.
            public mutating func MoveNext() -> Bool {
                position += 1
                if position < collection.Count { return true }
                position = collection.Count
                return false
            }

            // The pinned Dispose body is a single `ret`.
            public func Dispose() {}
        }
    }
}
