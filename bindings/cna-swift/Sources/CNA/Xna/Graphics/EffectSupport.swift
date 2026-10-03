// SPDX-License-Identifier: MIT

import CNAShim

/// One owned native effect handle, released exactly once.
///
/// The effect object graph is entirely handle-based and every node is owned:
/// `cna_effect_get_techniques` hands back an owned collection view, its
/// `get_at` hands back an owned element view, and each must be destroyed
/// through its own route. `NativeHandleStorage` cannot serve here because these
/// are not `GraphicsResource`s — they carry no `Name`, no `Tag`, no `Disposing`
/// event and no public `Dispose` — so this is the smaller thing that is
/// actually needed: a handle, its destroy route, and release-once.
internal final class EffectHandleBox {
    internal private(set) var handle: UInt64
    private let destroy: NativeDestroyRoute
    internal let runtime: RuntimeState

    init(handle: UInt64, runtime: RuntimeState, destroy: @escaping NativeDestroyRoute) {
        self.handle = handle
        self.runtime = runtime
        self.destroy = destroy
    }

    /// Releases the native handle. Idempotent, because a Swift object graph
    /// does not promise an order: a collection may outlive the element it
    /// handed out, or the other way round.
    func release() {
        guard handle != 0 else { return }
        _ = destroy(handle)
        handle = 0
    }

    /// The handle, or a refusal naming the operation, so every caller reports
    /// a released handle the same way.
    func validated(_ operation: String) throws -> UInt64 {
        guard handle != 0 else {
            throw CNAError.producerInvariant(
                "\(operation) on a released effect handle")
        }
        return handle
    }

    deinit { release() }
}

extension Microsoft.Xna.Framework.Graphics {
    /// The four effect collections' whole behaviour, once.
    ///
    /// `EffectParameterCollection`, `EffectTechniqueCollection`,
    /// `EffectPassCollection` and `EffectAnnotationCollection` are the same four
    /// members over four element types — `Count`, `Item[Int32]`, `Item[String]`
    /// and `GetEnumerator` — and CNA gives each the same routes. Writing them
    /// four times is writing the same defect four times, which is what
    /// `TextureCollection` learnt from `SamplerStateCollection`.
    ///
    /// **The element objects are cached by index.** CNA's `get_at` hands back a
    /// *fresh owned view handle* on every call, so asking twice would give two
    /// Swift objects for one native element and `c[0] === c[0]` would be false.
    /// XNA's collections hold a `List<T>` built once and `Item` is an index into
    /// it, so identity holds there; the cache is what reproduces that.
    ///
    /// **The name lookup is a managed scan, and that is XNA's own.**
    /// Every one of the four `Item[String]` getters is a `for` over `_items`
    /// comparing `Name`, ending in `return null`. CNA does publish
    /// `..._collection_find`, but it answers a *second* owned view of the
    /// element rather than the one `get_at` gave, so using it would break the
    /// identity the index path establishes — `parameters["X"] === parameters[0]`
    /// would be false where XNA has it true. Those five `find` routes therefore
    /// have no consuming member and are deliberately **not bound**, which is
    /// what `docs/native-abi.md` requires.
    internal final class EffectElementCache<Element: AnyObject> {
        private let box: EffectHandleBox
        private let getCount: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
        private let getAt: (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
        private let wrap: (UInt64, RuntimeState) -> Element
        private var cached: [Int: Element] = [:]

        init(
            box: EffectHandleBox,
            getCount: @escaping (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
            getAt: @escaping (UInt64, UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
            wrap: @escaping (UInt64, RuntimeState) -> Element
        ) {
            self.box = box
            self.getCount = getCount
            self.getAt = getAt
            self.wrap = wrap
        }

        var runtime: RuntimeState { box.runtime }

        /// `Count` — `_items.Count`, a field read in XNA and a route here.
        ///
        /// XNA's getter has no failure path, so this must not throw. A handle
        /// that no longer answers reports zero, which is what an emptied
        /// collection looks like and the only honest answer an infallible
        /// getter can give.
        var count: Int32 {
            var value: UInt64 = 0
            guard box.handle != 0, getCount(box.handle, &value) == 0,
                  value <= UInt64(Int32.max) else { return 0 }
            return Int32(value)
        }

        /// `Item[Int32]`, and it does **not** throw:
        ///
        /// ```text
        /// if (index < 0 || index >= pPass.Count) { ldnull; ret }
        /// return pPass[index];
        /// ```
        ///
        /// An out-of-range index answers **null**, and the pinned verdict is
        /// `IL_NO_FAILURE_PATH` with `PROVEN_NULLABLE_SUCCESS`. That is the
        /// opposite of `SamplerStateCollection` and `TextureCollection`, whose
        /// indexers raise `ArgumentOutOfRangeException("index")` — the four
        /// effect collections guard first and return null instead, and the
        /// difference is in the IL rather than in a convention.
        func element(at index: Int32) -> Element? {
            guard index >= 0, index < count else { return nil }
            if let existing = cached[Int(index)] { return existing }
            guard box.handle != 0 else { return nil }
            var element: UInt64 = 0
            guard getAt(box.handle, UInt64(index), &element) == 0, element != 0 else {
                return nil
            }
            let wrapped = wrap(element, box.runtime)
            cached[Int(index)] = wrapped
            return wrapped
        }

        /// `Item[String]`, as the managed scan XNA performs: the first element
        /// whose key matches, or **nil**, which is the `return null` every one
        /// of the four getters ends in.
        func element(named name: String, key: (Element) -> String?) -> Element? {
            for index in 0..<count {
                guard let candidate = element(at: index) else { continue }
                if key(candidate) == name { return candidate }
            }
            return nil
        }

        /// Every element, in order, for `GetEnumerator`.
        func all() -> [Element] {
            (0..<count).compactMap { element(at: $0) }
        }
    }

    /// Reads a UTF-8 string CNA reports in two calls — a byte count, then a
    /// copy into a buffer of exactly that size.
    ///
    /// Every name, semantic and string value in the effect family arrives this
    /// way, so the two-call protocol and its capacity contract are written
    /// once. An empty string is a real answer and not an error: a parameter
    /// with no semantic reports zero bytes.
    internal static func effectString(
        runtime: RuntimeState,
        handle: UInt64,
        operation: String,
        byteCount: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
        copy: (UInt64, UnsafeMutablePointer<CChar>?, UInt64,
               UnsafeMutablePointer<UInt64>?) -> UInt32
    ) throws -> String {
        var bytes: UInt64 = 0
        try runtime.functions.check(
            byteCount(handle, &bytes), operation: "\(operation) byte count")
        guard bytes > 0 else { return "" }
        guard bytes <= UInt64(Int32.max) else {
            throw CNAError.nativeFailure(
                operation: operation, result: 10,
                message: "string length exceeds the XNA Int32 range")
        }
        var buffer = [CChar](repeating: 0, count: Int(bytes))
        var written: UInt64 = 0
        try runtime.functions.check(
            buffer.withUnsafeMutableBufferPointer {
                copy(handle, $0.baseAddress, bytes, &written)
            },
            operation: operation)
        let used = buffer.prefix(Int(min(written, bytes))).map { UInt8(bitPattern: $0) }
        guard let text = String(bytes: used, encoding: .utf8) else {
            throw CNAError.nativeFailure(
                operation: operation, result: 11,
                message: "native text is not valid UTF-8")
        }
        return text
    }
}
