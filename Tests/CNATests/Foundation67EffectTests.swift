// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class EffectProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((EffectProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (EffectProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant("no device")
            }
            try body?(self, device)
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

/// Foundation 67: the `Effect` core — nine types and ninety-six members.
///
/// The effect this host can build is CNA's `create_empty`, which the header
/// calls "the minimal concrete adapter for the native abstract Effect base
/// class": **no parameters, one technique, one pass**. So the parameter
/// surface's fifty-one members have no parameter to read on this artifact, and
/// what is asserted here is the structure, the identity rules, the collection
/// semantics that differ from every other collection in this binding, and the
/// one thing the whole milestone existed for — that applying an effect makes a
/// draw legal.
final class Foundation67EffectTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (EffectProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> EffectProbeGame {
        let game = try EffectProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    // ------------------------------------------------------------------
    // The structure.

    /// An effect has the shape CNA's empty adapter has, and it is a
    /// `GraphicsResource` like every other native object here.
    func testTheEmptyEffectHasTheShapeCnaBuilds() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            let asResource: G.GraphicsResource = effect
            game.observations["is a resource"] = "\(asResource === effect)"
            game.observations["device"] = "\(effect.GraphicsDevice === device)"
            game.observations["parameters"] = "\(effect.Parameters?.Count ?? -1)"
            game.observations["techniques"] = "\(effect.Techniques?.Count ?? -1)"
            game.observations["passes"] =
                "\(effect.Techniques?[Int32(0)]?.Passes.Count ?? -1)"
            game.observations["current"] = "\(effect.CurrentTechnique != nil)"
            try effect.Dispose()
            game.observations["disposed"] = "\(effect.IsDisposed)"
        }
        XCTAssertEqual(game.observations["is a resource"], "true")
        XCTAssertEqual(game.observations["device"], "true")
        XCTAssertEqual(game.observations["parameters"], "0")
        XCTAssertEqual(game.observations["techniques"], "1")
        XCTAssertEqual(game.observations["passes"], "1")
        XCTAssertEqual(game.observations["current"], "true")
        XCTAssertEqual(game.observations["disposed"], "true")
    }

    /// **The collections answer the same object every time**, which is what
    /// XNA's `List<T>` field gives and what a fresh CNA view would not.
    ///
    /// `cna_effect_get_techniques` hands back a *new owned collection view* on
    /// every call and `get_at` a new element view, so without the cache
    /// `effect.Techniques !== effect.Techniques` and
    /// `technique.Passes[0] !== technique.Passes[0]`. Every `===` below would
    /// be false.
    func testTheObjectGraphIsStableByIdentity() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            game.observations["techniques"] =
                "\(effect.Techniques === effect.Techniques)"
            game.observations["parameters"] =
                "\(effect.Parameters === effect.Parameters)"
            guard let technique = effect.Techniques?[Int32(0)] else {
                throw CNAError.producerInvariant("no technique")
            }
            game.observations["technique"] =
                "\(effect.Techniques?[Int32(0)] === technique)"
            game.observations["passes"] =
                "\(technique.Passes === technique.Passes)"
            game.observations["pass"] =
                "\(technique.Passes[Int32(0)] === technique.Passes[Int32(0)])"
            game.observations["annotations"] =
                "\(technique.Annotations === technique.Annotations)"
            // CurrentTechnique resolves back through Techniques, so it is the
            // same object the collection hands out and not a third view.
            game.observations["current is the same"] =
                "\(effect.CurrentTechnique === technique)"
            try effect.Dispose()
        }
        for key in ["techniques", "parameters", "technique", "passes", "pass",
                    "annotations", "current is the same"] {
            XCTAssertEqual(game.observations[key], "true", key)
        }
    }

    /// **An out-of-range index is nil, not a throw** — and that is the
    /// opposite of every other collection in this binding.
    ///
    /// `EffectPassCollection.get_Item(int)` is
    /// `if (index < 0 || index >= Count) { ldnull; ret }`, pinned
    /// `IL_NO_FAILURE_PATH` and `PROVEN_NULLABLE_SUCCESS`, where
    /// `SamplerStateCollection` and `TextureCollection` both raise
    /// `ArgumentOutOfRangeException("index")`. The difference is in the IL, so
    /// it is reproduced rather than smoothed over.
    func testAnOutOfRangeIndexIsNilRatherThanAThrow() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let techniques = effect.Techniques else {
                throw CNAError.producerInvariant("no techniques")
            }
            game.observations["count"] = "\(techniques.Count)"
            game.observations["zero"] = "\(techniques[Int32(0)] != nil)"
            game.observations["one"] = "\(techniques[Int32(1)] == nil)"
            game.observations["negative"] = "\(techniques[Int32(-1)] == nil)"
            game.observations["far"] = "\(techniques[Int32(9999)] == nil)"
            // The same for a name that matches nothing.
            game.observations["absent name"] = "\(techniques["no such technique"] == nil)"
            // An empty parameter collection answers nil for everything.
            game.observations["no parameters"] =
                "\(effect.Parameters?[Int32(0)] == nil)"
            game.observations["no semantic"] =
                "\(effect.Parameters?.GetParameterBySemantic("WORLD") == nil)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["count"], "1")
        XCTAssertEqual(game.observations["zero"], "true")
        XCTAssertEqual(game.observations["one"], "true")
        XCTAssertEqual(game.observations["negative"], "true")
        XCTAssertEqual(game.observations["far"], "true")
        XCTAssertEqual(game.observations["absent name"], "true")
        XCTAssertEqual(game.observations["no parameters"], "true")
        XCTAssertEqual(game.observations["no semantic"], "true")
    }

    /// A name lookup and an index lookup answer the **same object**, because
    /// the name scan resolves through the index cache exactly as XNA's scan
    /// over its own `List<T>` does.
    func testANameLookupAnswersTheIndexedObject() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let techniques = effect.Techniques,
                  let byIndex = techniques[Int32(0)] else {
                throw CNAError.producerInvariant("no technique")
            }
            let name = byIndex.Name
            game.observations["name"] = name
            game.observations["same object"] = "\(techniques[name] === byIndex)"
            game.observations["pass name"] = byIndex.Passes[Int32(0)]?.Name ?? "<none>"
            game.observations["semantic"] =
                "\(effect.Parameters?.GetParameterBySemantic("") == nil)"
            try effect.Dispose()
        }
        // The EXACT names CNA's empty adapter carries, measured in
        // `build-probe/f67_effect.c`: seven bytes and two. Asserting the values
        // rather than "not empty" is what makes the two-call string protocol
        // falsifiable -- `effect-string-ignores-the-written-length` survived
        // while this test only checked for a non-empty string, because a name
        // read back with its buffer padding still matches itself.
        XCTAssertEqual(game.observations["name"], "Default")
        XCTAssertEqual(game.observations["pass name"], "P0")
        XCTAssertEqual(game.observations["same object"], "true")
    }

    /// `GetEnumerator` walks every element, in order, and hands back the same
    /// objects the indexer does.
    func testTheEnumeratorWalksTheSameObjects() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let techniques = effect.Techniques else {
                throw CNAError.producerInvariant("no techniques")
            }
            var walked: [G.EffectTechnique] = []
            let enumerator = techniques.GetEnumerator()
            while let next = try enumerator.Next() { walked.append(next) }
            game.observations["walked"] = "\(walked.count)"
            game.observations["identity"] =
                "\(walked.first === techniques[Int32(0)])"

            // And the empty collection walks zero times rather than failing.
            var parameterCount = 0
            if let parameters = effect.Parameters {
                let parameterEnumerator = parameters.GetEnumerator()
                while try parameterEnumerator.Next() != nil { parameterCount += 1 }
            }
            game.observations["parameters walked"] = "\(parameterCount)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["walked"], "1")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["parameters walked"], "0")
    }

    /// `Clone` produces an independent effect, not the same object.
    func testCloneIsIndependent() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            let clone = try effect.Clone()
            game.observations["distinct"] = "\(clone !== effect)"
            game.observations["same shape"] =
                "\(clone.Techniques?.Count == effect.Techniques?.Count)"
            game.observations["distinct techniques"] =
                "\(clone.Techniques?[Int32(0)] !== effect.Techniques?[Int32(0)])"
            game.observations["clone device"] = "\(clone.GraphicsDevice === device)"
            try clone.Dispose()
            game.observations["source alive"] = "\(!effect.IsDisposed)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["distinct"], "true")
        XCTAssertEqual(game.observations["same shape"], "true")
        XCTAssertEqual(game.observations["distinct techniques"], "true")
        XCTAssertEqual(game.observations["clone device"], "true")
        XCTAssertEqual(game.observations["source alive"], "true")
    }

    // ------------------------------------------------------------------
    // What the milestone was for.

    /// **Applying a pass succeeds**, which is the half of Foundation 63's
    /// claim this milestone can assert through the projected surface.
    ///
    /// The other half — that the same draw which answered *"no effect has been
    /// applied"* returns 0 afterwards — is measured in
    /// `build-probe/f67_effect.c` and recorded in the evidence, because the
    /// draw members are not projected yet and
    /// `cna_graphics_device_draw_primitives` therefore has no consuming member.
    /// Binding a route for a test alone is what `docs/native-abi.md` forbids,
    /// and it is the same reason Foundation 63 read its own refusal from a
    /// probe rather than from a test.
    func testApplyingAPassSucceeds() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()
            try pass.Apply()
            game.observations["applied twice"] = "yes"

            // And through the whole-effect route, which is what `Effect.apply`
            // reaches.
            try effect.apply()
            game.observations["effect applied"] = "yes"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["applied twice"], "yes")
        XCTAssertEqual(game.observations["effect applied"], "yes")
    }

    /// A pass whose effect has been disposed refuses rather than reaching a
    /// dead handle.
    func testAPassOfADisposedEffectRefuses() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try effect.Dispose()
            // `Helpers.CheckDisposed(effect, effect.pComPtr)` is Apply's FIRST
            // instruction, and the object it names is the effect rather than
            // the pass.
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'Effect'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try pass.Apply() }
            game.observations["after disposal"] = "refused"
        }
        XCTAssertEqual(game.observations["after disposal"], "refused")
    }

    /// `OnApply` is empty in XNA — `nop; ret` — and exists so a derived effect
    /// can push state before a pass applies.
    ///
    /// **It is `EffectPass.Apply` that calls it**, not any whole-effect entry
    /// point: `IL_003b: callvirt Effect::OnApply()` sits inside the pass. The
    /// first draft of this projection called it from `Effect.apply` instead,
    /// which would have fired the derived hook twice for one application.
    func testOnApplyIsCalledByThePass() throws {
        try requireNative()

        final class CountingEffect: G.Effect {
            var applied = 0
            override func OnApply() throws { applied += 1 }
        }

        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            // A derived effect over its own native handle. Swift does not
            // inherit a convenience initializer across a module boundary when
            // the designated one is internal, so the subclass is built through
            // the designated initializer directly -- which is what the
            // convenience one does anyway.
            var cloneHandle: UInt64 = 0
            try device.runtimeState.functions.check(
                device.runtimeState.functions.effectClone(
                    try effect.validatedHandle("clone"), &cloneHandle),
                operation: "cna_effect_clone")
            let derived = CountingEffect(
                handle: cloneHandle, runtime: device.runtimeState,
                device: device, typeName: "Effect")

            guard let pass = derived.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()
            try pass.Apply()
            game.observations["by the pass"] = "\(derived.applied)"

            // The whole-effect route does NOT call it.
            try derived.apply()
            game.observations["after effect apply"] = "\(derived.applied)"

            try derived.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["by the pass"], "2")
        XCTAssertEqual(game.observations["after effect apply"], "2")
    }

    /// A pass whose technique is **not** the effect's `CurrentTechnique` is
    /// refused — `InvalidOperationException(NotCurrentTechnique)`, which sits
    /// between the disposal check and `OnApply`.
    ///
    /// The empty effect has one technique, and it is the current one, so the
    /// reachable half of this is the accepting branch. The current technique
    /// cannot be cleared to reach the refusal: XNA's setter throws
    /// `ArgumentNullException("value", NullNotAllowed)` for null, and so does
    /// this one (CNA 0.21 accepted a null technique; 0.35 refuses it too).
    func testAPassMustBelongToTheCurrentTechnique() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let technique = effect.Techniques?[Int32(0)],
                  let pass = technique.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            game.observations["is current"] =
                "\(effect.CurrentTechnique === technique)"
            try pass.Apply()
            game.observations["accepted"] = "yes"

            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    G.GraphicsDevice.nullNotAllowedMessage, paramName: "value"),
                paramName: "value",
                hResult: Int32(bitPattern: 0x8000_4003)
            ) { try effect.SetCurrentTechnique(nil) }
            game.observations["still current"] =
                "\(effect.CurrentTechnique === technique)"

            try effect.SetCurrentTechnique(technique)
            try pass.Apply()
            game.observations["accepted again"] = "yes"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["is current"], "true")
        XCTAssertEqual(game.observations["accepted"], "yes")
        XCTAssertEqual(game.observations["still current"], "true")
        XCTAssertEqual(game.observations["accepted again"], "yes")
    }

    /// The byte-array constructor's two **managed** tests, in order.
    ///
    /// ```text
    /// if (effectCode == null || effectCode.Length == 0)
    ///     throw new ArgumentNullException("effectCode", NullNotAllowed);
    /// if (effectCode.Length % 4 != 0)
    ///     throw new ArgumentException(
    ///         Format(ArrayMultipleFour, "effectCode"), "effectCode");
    /// ```
    ///
    /// Everything after them reads the compiled effect's own header, which
    /// this binding does not parse — `MustUserShaderCode` and the two shader
    /// model refusals are recorded as native-owned. So a well-formed-length
    /// array of nonsense reaches CNA and fails there, on the runtime channel,
    /// which is the honest place for a verdict about bytecode.
    func testTheEffectCodeConstructorChecksTheArrayFirst() throws {
        try requireNative()
        let game = try run { game, device in
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "effectCode"),
                paramName: "effectCode",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { _ = try G.Effect(graphicsDevice: device, effectCode: []) }

            for length in [1, 2, 3, 5, 7] {
                assertProjected(
                    CNAArgumentException.self,
                    message: composedArgumentMessage(
                        "The array effectCode must have a length that is a "
                        + "multiple of four.",
                        paramName: "effectCode"),
                    paramName: "effectCode",
                    hResult: CNAArgumentException.corArgumentHResult
                ) {
                    _ = try G.Effect(
                        graphicsDevice: device,
                        effectCode: [UInt8](repeating: 0, count: length))
                }
            }

            // A multiple of four gets past both managed tests and reaches CNA,
            // which refuses the content on the runtime channel.
            var reached = "accepted"
            do {
                _ = try G.Effect(graphicsDevice: device,
                                 effectCode: [UInt8](repeating: 0, count: 8))
            } catch is CNAError {
                reached = "native refusal"
            } catch {
                reached = "\(type(of: error))"
            }
            game.observations["four byte multiple"] = reached
        }
        XCTAssertEqual(game.observations["four byte multiple"], "native refusal")
    }

    /// A disposed effect refuses on the CLR channel, like every other
    /// `GraphicsResource`.
    func testADisposedEffectIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            try effect.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'Effect'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { _ = try effect.Clone() }
            // The infallible getters answer an empty collection rather than
            // throwing, because XNA's are field reads with no failure path.
            game.observations["parameters"] = "\(effect.Parameters?.Count ?? -1)"
            game.observations["techniques"] = "\(effect.Techniques?.Count ?? -1)"
        }
    }
}
