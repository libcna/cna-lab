// SPDX-License-Identifier: MIT

import Foundation

#if os(Linux)
import Glibc
#endif

internal final class OwnerThread {
    let swiftThread: Thread

    #if os(Linux)
    private let nativeThread: pthread_t
    #endif

    init() {
        swiftThread = Thread.current
        #if os(Linux)
        nativeThread = pthread_self()
        #endif
    }

    var isCurrent: Bool {
        #if os(Linux)
        return pthread_equal(nativeThread, pthread_self()) != 0
        #else
        return swiftThread === Thread.current
        #endif
    }

    func validate(_ operation: String) throws {
        guard isCurrent else { throw CNAError.ownerThreadViolation(operation) }
    }
}
