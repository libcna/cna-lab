// SPDX-License-Identifier: MIT

import Foundation

internal enum GenerationSequence {
    private static let lock = NSLock()
    private static var nextValue: UInt64 = 1

    static func next() -> UInt64 {
        lock.lock()
        defer { lock.unlock() }
        let value = nextValue
        nextValue &+= 1
        if nextValue == 0 { nextValue = 1 }
        return value
    }
}
