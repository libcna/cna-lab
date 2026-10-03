// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    open class CurveKeyCollection {
        private var storage: [CurveKey] = []
        private var version: UInt64 = 0

        internal var timeRange: Float = 0
        internal var inverseTimeRange: Float = 0
        internal var isCacheAvailable = true

        public init() {}

        public var Count: Int32 { Int32(storage.count) }
        public var IsReadOnly: Bool { false }

        public func IndexOf(_ item: CurveKey?) -> Int32 {
            guard let item else { return -1 }
            for (index, key) in storage.enumerated() where key.Equals(item) {
                return Int32(index)
            }
            return -1
        }

        public func RemoveAt(_ index: Int32) throws {
            let resolved = try checkedIndex(index)
            storage.remove(at: resolved)
            version &+= 1
            isCacheAvailable = false
        }

        public func Add(_ item: CurveKey) {
            var low = 0
            var high = storage.count - 1
            var insertion = 0
            var found: Int?
            while low <= high {
                let middle = low + ((high - low) >> 1)
                let order = storage[middle].xnaPositionCompare(to: item)
                if order == 0 {
                    found = middle
                    break
                }
                if order < 0 {
                    low = middle + 1
                } else {
                    high = middle - 1
                }
            }
            if let found {
                insertion = found
                while insertion < storage.count && item.Position == storage[insertion].Position {
                    insertion += 1
                }
            } else {
                insertion = low
            }
            storage.insert(item, at: insertion)
            version &+= 1
            isCacheAvailable = false
        }

        public func Clear() {
            storage.removeAll(keepingCapacity: true)
            version &+= 1
            timeRange = 0
            inverseTimeRange = 0
            isCacheAvailable = false
        }

        public func Contains(_ item: CurveKey?) -> Bool { IndexOf(item) >= 0 }

        public func CopyTo(_ array: inout [CurveKey], arrayIndex: Int32) throws {
            // CopyTo delegates straight to List<CurveKey>.CopyTo, which
            // delegates to the `internalcall` Array.Copy; neither failure
            // message is in the admitted IL, so the documented classes are
            // raised without one. See CNAList.CopyTo.
            guard arrayIndex >= 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "arrayIndex")
            }
            let start = Int(arrayIndex)
            guard start <= array.count, storage.count <= array.count - start else {
                throw CNAArgumentException()
            }
            for index in storage.indices { array[start + index] = storage[index] }
            isCacheAvailable = false
        }

        public func Remove(_ item: CurveKey?) -> Bool {
            isCacheAvailable = false
            let index = IndexOf(item)
            guard index >= 0 else { return false }
            storage.remove(at: Int(index))
            version &+= 1
            return true
        }

        public func GetEnumerator() -> CNAEnumerator<CurveKey> {
            let expectedVersion = version
            return CNAEnumerator(expectedVersion: expectedVersion) { [self] index, expected in
                // GetEnumerator returns the backing List<CurveKey>'s own
                // enumerator, so this is List<T>.Enumerator's message.
                guard version == expected else {
                    throw CNAInvalidOperationException(
                        message: CNAList<CurveKey>.enumFailedVersionMessage)
                }
                guard index < storage.count else { return nil }
                return storage[index]
            }
        }

        public func Clone() -> CurveKeyCollection {
            let result = CurveKeyCollection()
            result.storage = storage
            result.inverseTimeRange = inverseTimeRange
            result.timeRange = timeRange
            result.isCacheAvailable = true
            return result
        }

        public func Item(_ index: Int32) throws -> CurveKey {
            storage[try checkedIndex(index)]
        }

        public func SetItem(_ index: Int32, _ value: CurveKey) throws {
            let resolved = try checkedIndex(index)
            let oldPosition = storage[resolved].Position
            if oldPosition == value.Position {
                storage[resolved] = value
                version &+= 1
                return
            }
            storage.remove(at: resolved)
            version &+= 1
            Add(value)
        }

        internal func key(at index: Int) -> CurveKey { storage[index] }

        internal func computeCacheValues() {
            timeRange = 0
            inverseTimeRange = 0
            if storage.count > 1 {
                timeRange = storage[storage.count - 1].Position - storage[0].Position
                if timeRange > Float.leastNonzeroMagnitude {
                    inverseTimeRange = 1 / timeRange
                }
            }
            isCacheAvailable = true
        }

        private func checkedIndex(_ index: Int32) throws -> Int {
            // The indexer delegates to List<CurveKey>.get_Item, so this is
            // the no-argument ThrowHelper overload's payload.
            guard index >= 0 && Int(index) < storage.count else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "index",
                    message: CNAList<CurveKey>.indexOutOfRangeMessage)
            }
            return Int(index)
        }
    }
}
