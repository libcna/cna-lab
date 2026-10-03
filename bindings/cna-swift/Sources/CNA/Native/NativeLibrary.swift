// SPDX-License-Identifier: MIT

import Foundation

#if os(Linux)
import Glibc
#endif

internal final class NativeLibrary {
    let handle: UnsafeMutableRawPointer
    let admittedPath: String

    init() throws {
        #if os(Linux)
        let environment = ProcessInfo.processInfo.environment
        let candidates: [String]
        if let override = environment["CNA_NATIVE_LIBRARY"] {
            guard override.hasPrefix("/") else {
                throw CNAError.nativeLibraryLoadFailed(
                    path: override,
                    message: "CNA_NATIVE_LIBRARY must be an absolute path"
                )
            }
            candidates = [override]
        } else {
            candidates = ["libcna_c_api.so"]
        }

        var failures: [String] = []
        for candidate in candidates {
            dlerror()
            if let loaded = dlopen(candidate, Int32(RTLD_NOW | RTLD_LOCAL)) {
                handle = loaded
                admittedPath = candidate
                return
            }
            let message = dlerror().map { String(cString: $0) } ?? "unknown dlopen failure"
            failures.append("\(candidate): \(message)")
        }
        if environment["CNA_NATIVE_LIBRARY"] != nil {
            throw CNAError.nativeLibraryLoadFailed(path: candidates[0], message: failures.joined(separator: "; "))
        }
        throw CNAError.nativeLibraryNotFound(candidates: failures)
        #else
        throw CNAError.unsupportedPlatform("this platform (Foundation 1 qualifies Linux only)")
        #endif
    }

    deinit {
        #if os(Linux)
        dlclose(handle)
        #endif
    }

    func resolve<T>(_ symbol: String, as type: T.Type) throws -> T {
        #if os(Linux)
        dlerror()
        guard let address = dlsym(handle, symbol) else {
            throw CNAError.missingNativeSymbol(symbol)
        }
        return unsafeBitCast(address, to: type)
        #else
        throw CNAError.unsupportedPlatform("this platform")
        #endif
    }
}
