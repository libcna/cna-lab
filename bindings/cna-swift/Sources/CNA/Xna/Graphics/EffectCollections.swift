// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// `Microsoft.Xna.Framework.Graphics.EffectAnnotationCollection`.
    ///
    /// `sealed`, `IEnumerable<EffectAnnotation>`, four members. Everything it
    /// does lives in `EffectElementCache`; the four collections differ only in
    /// their element type, their two routes and — for `Item[String]` — which
    /// property the managed scan compares.
    public final class EffectAnnotationCollection {
        private let cache: EffectElementCache<EffectAnnotation>

        internal init(handle: UInt64, runtime: RuntimeState) {
            let box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectAnnotationCollectionDestroy)
            cache = EffectElementCache(
                box: box,
                getCount: runtime.functions.effectAnnotationCollectionGetCount,
                getAt: runtime.functions.effectAnnotationCollectionGetAt,
                wrap: { EffectAnnotation(handle: $0, runtime: $1) })
        }

        /// `EffectAnnotationCollection.Count`.
        public var Count: Int32 { cache.count }

        /// `EffectAnnotationCollection.Item[Int32]` — a Swift `subscript`, because the
        /// pinned verdict is `IL_NO_FAILURE_PATH`: the getter guards the index
        /// and returns **null** rather than raising.
        public subscript(index: Int32) -> EffectAnnotation? {
            cache.element(at: index)
        }

        /// `EffectAnnotationCollection.Item[String]` — nil when no annotation
        /// carries that name, which is the `return null` the getter ends in.
        public subscript(name: String) -> EffectAnnotation? {
            cache.element(named: name) { $0.Name }
        }

        /// `EffectAnnotationCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<EffectAnnotation> {
            let elements = cache.all()
            return CNAEnumerator(expectedVersion: 0) { index, _ in
                index < elements.count ? elements[index] : nil
            }
        }
    }

    /// `Microsoft.Xna.Framework.Graphics.EffectPass`.
    ///
    /// Three members, and `Apply` is the one that matters: it is what makes a
    /// draw legal. `build-probe/f67_effect.c` measures
    /// `cna_effect_pass_apply -> 0` followed by
    /// `cna_graphics_device_draw_primitives -> 0`, where the same draw without
    /// any apply is refused with *"no effect has been applied"*.
    public final class EffectPass {
        private let box: EffectHandleBox
        private var annotations: EffectAnnotationCollection?

        /// `EffectPass._technique`, the field `Apply` compares against the
        /// effect's `CurrentTechnique`. Weak, because the technique owns the
        /// collection that owns this pass and a strong link would be a cycle.
        internal weak var technique: EffectTechnique?

        internal init(
            handle: UInt64, runtime: RuntimeState, technique: EffectTechnique?
        ) {
            box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectPassDestroy)
            self.technique = technique
        }

        /// `EffectPass.Name`.
        public var Name: String {
            guard let handle = try? box.validated("EffectPass.Name") else { return "" }
            let functions = box.runtime.functions
            return (try? Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: "cna_effect_pass_copy_name",
                byteCount: functions.effectPassGetNameByteCount,
                copy: functions.effectPassCopyName)) ?? ""
        }

        /// `EffectPass.Annotations`.
        ///
        /// Built once and cached, so `pass.Annotations === pass.Annotations`.
        /// XNA's getter reads a field the constructor filled; CNA's route hands
        /// back a fresh owned collection view on every call, and returning two
        /// objects for one collection is exactly what the cache prevents.
        public var Annotations: EffectAnnotationCollection {
            if let annotations { return annotations }
            let built = Microsoft.Xna.Framework.Graphics.annotationCollection(
                box: box, route: box.runtime.functions.effectPassGetAnnotations)
            annotations = built
            return built
        }

        /// `EffectPass.Apply()`.
        ///
        /// ```text
        /// Helpers.CheckDisposed(_technique._parent, effect.pComPtr);
        /// if (effect._currentTechnique != _technique)
        ///     throw new InvalidOperationException(NotCurrentTechnique);
        /// effect.OnApply();
        /// ... the native apply
        /// ```
        ///
        /// **Three things the IL says that a reasonable design would not.**
        /// The disposal check is on the *effect*, not on the pass — a pass
        /// whose effect is gone raises `ObjectDisposedException` naming
        /// `Effect`. The pass refuses unless its technique is the effect's
        /// `CurrentTechnique`, so a pass reached through a non-current
        /// technique is not applicable. And **`OnApply` is called from here**,
        /// not from any whole-effect entry point — the derived hook fires once
        /// per pass application.
        ///
        /// The first draft of this projection had none of the three: it
        /// validated its own handle, applied unconditionally, and called
        /// `OnApply` from the effect. A test asserting that a disposed effect's
        /// pass refuses is what caught it.
        public func Apply() throws {
            guard let technique, let effect = technique.owner else {
                throw CNAError.producerInvariant(
                    "EffectPass.Apply on a pass whose effect is gone")
            }
            _ = try effect.validatedHandle("EffectPass.Apply")
            guard effect.CurrentTechnique === technique else {
                throw CNAInvalidOperationException(
                    message: EffectPass.notCurrentTechniqueMessage)
            }
            try effect.OnApply()
            try box.runtime.functions.check(
                box.runtime.functions.effectPassApply(
                    try box.validated("EffectPass.Apply")),
                operation: "cna_effect_pass_apply")
        }

        /// `FrameworkResources.NotCurrentTechnique`.
        internal static let notCurrentTechniqueMessage =
            "Cannot Apply an EffectPass that is not from the CurrentTechnique."
    }

    /// `Microsoft.Xna.Framework.Graphics.EffectPassCollection`.
    public final class EffectPassCollection {
        private let cache: EffectElementCache<EffectPass>

        internal init(
            handle: UInt64, runtime: RuntimeState, technique: EffectTechnique?
        ) {
            let box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectPassCollectionDestroy)
            cache = EffectElementCache(
                box: box,
                getCount: runtime.functions.effectPassCollectionGetCount,
                getAt: runtime.functions.effectPassCollectionGetAt,
                wrap: { [weak technique] in
                    EffectPass(handle: $0, runtime: $1, technique: technique)
                })
        }

        /// `EffectPassCollection.Count`.
        public var Count: Int32 { cache.count }

        /// `EffectPassCollection.Item[Int32]` — a Swift `subscript`, because the
        /// pinned verdict is `IL_NO_FAILURE_PATH`: the getter guards the index
        /// and returns **null** rather than raising.
        public subscript(index: Int32) -> EffectPass? {
            cache.element(at: index)
        }

        /// `EffectPassCollection.Item[String]`.
        public subscript(name: String) -> EffectPass? {
            cache.element(named: name) { $0.Name }
        }

        /// `EffectPassCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<EffectPass> {
            let elements = cache.all()
            return CNAEnumerator(expectedVersion: 0) { index, _ in
                index < elements.count ? elements[index] : nil
            }
        }
    }

    /// `Microsoft.Xna.Framework.Graphics.EffectTechnique`.
    public final class EffectTechnique {
        internal let box: EffectHandleBox
        private var annotations: EffectAnnotationCollection?
        private var passes: EffectPassCollection?

        /// `EffectTechnique._parent`. Weak: the effect owns the collection that
        /// owns this technique.
        internal weak var owner: Effect?

        internal init(handle: UInt64, runtime: RuntimeState, owner: Effect?) {
            box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectTechniqueDestroy)
            self.owner = owner
        }

        /// `EffectTechnique.Name`.
        public var Name: String {
            guard let handle = try? box.validated("EffectTechnique.Name") else {
                return ""
            }
            let functions = box.runtime.functions
            return (try? Microsoft.Xna.Framework.Graphics.effectString(
                runtime: box.runtime, handle: handle,
                operation: "cna_effect_technique_copy_name",
                byteCount: functions.effectTechniqueGetNameByteCount,
                copy: functions.effectTechniqueCopyName)) ?? ""
        }

        /// `EffectTechnique.Annotations`.
        public var Annotations: EffectAnnotationCollection {
            if let annotations { return annotations }
            let built = Microsoft.Xna.Framework.Graphics.annotationCollection(
                box: box, route: box.runtime.functions.effectTechniqueGetAnnotations)
            annotations = built
            return built
        }

        /// `EffectTechnique.Passes`.
        ///
        /// Cached for the reason `Annotations` is: XNA reads a field, CNA
        /// answers a fresh owned view, and `technique.Passes[0] ===
        /// technique.Passes[0]` must hold.
        public var Passes: EffectPassCollection {
            if let passes { return passes }
            var handle: UInt64 = 0
            let built: EffectPassCollection
            if let native = try? box.validated("EffectTechnique.Passes"),
               box.runtime.functions.effectTechniqueGetPasses(native, &handle) == 0 {
                built = EffectPassCollection(
                    handle: handle, runtime: box.runtime, technique: self)
            } else {
                built = EffectPassCollection(
                    handle: 0, runtime: box.runtime, technique: self)
            }
            passes = built
            return built
        }
    }

    /// `Microsoft.Xna.Framework.Graphics.EffectTechniqueCollection`.
    public final class EffectTechniqueCollection {
        private let cache: EffectElementCache<EffectTechnique>

        internal init(handle: UInt64, runtime: RuntimeState, owner: Effect?) {
            let box = EffectHandleBox(
                handle: handle, runtime: runtime,
                destroy: runtime.functions.effectTechniqueCollectionDestroy)
            cache = EffectElementCache(
                box: box,
                getCount: runtime.functions.effectTechniqueCollectionGetCount,
                getAt: runtime.functions.effectTechniqueCollectionGetAt,
                wrap: { [weak owner] in
                    EffectTechnique(handle: $0, runtime: $1, owner: owner)
                })
        }

        /// `EffectTechniqueCollection.Count`.
        public var Count: Int32 { cache.count }

        /// `EffectTechniqueCollection.Item[Int32]` — a Swift `subscript`, because the
        /// pinned verdict is `IL_NO_FAILURE_PATH`: the getter guards the index
        /// and returns **null** rather than raising.
        public subscript(index: Int32) -> EffectTechnique? {
            cache.element(at: index)
        }

        /// `EffectTechniqueCollection.Item[String]`.
        public subscript(name: String) -> EffectTechnique? {
            cache.element(named: name) { $0.Name }
        }

        /// `EffectTechniqueCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<EffectTechnique> {
            let elements = cache.all()
            return CNAEnumerator(expectedVersion: 0) { index, _ in
                index < elements.count ? elements[index] : nil
            }
        }
    }

    /// The three `Annotations` getters' shared body: one route, one
    /// collection, and an empty one when the handle no longer answers — which
    /// is what an infallible getter can honestly say.
    internal static func annotationCollection(
        box: EffectHandleBox,
        route: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32
    ) -> EffectAnnotationCollection {
        var handle: UInt64 = 0
        guard let owner = try? box.validated("Annotations"),
              route(owner, &handle) == 0 else {
            return EffectAnnotationCollection(handle: 0, runtime: box.runtime)
        }
        return EffectAnnotationCollection(handle: handle, runtime: box.runtime)
    }
}
