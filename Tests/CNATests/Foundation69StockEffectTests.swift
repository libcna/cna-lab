// SPDX-License-Identifier: MIT

import CNAShim
import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class StockEffectProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((StockEffectProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (StockEffectProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 69: the five stock effects, `DirectionalLight`, `IEffectLights`
/// and `EffectMaterial` — eight types and a hundred and twenty-nine members.
///
/// Almost everything here is assertable without a renderer, because almost
/// everything here is managed. XNA's stock effects cache their state in fields
/// and push it at `OnApply`; the two rules that decide which half a property
/// lives in, the enable/disable discipline of a directional light, the exact
/// ten vectors of default lighting and the two effects that refuse to turn
/// lighting off are all managed behaviour, and all of it is read out of the IL.
final class Foundation69StockEffectTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (StockEffectProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> StockEffectProbeGame {
        let game = try StockEffectProbeGame(body)
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
    // DirectionalLight, which is entirely managed.

    /// The parameterless clone source runs the three setters, and only
    /// `Direction` reaches a parameter: `enabled` is still false, so
    /// `DiffuseColor` and `SpecularColor` cache without writing.
    ///
    /// ```text
    /// IL_0054: this.Direction     = Vector3.Down
    /// IL_005f: this.DiffuseColor  = Vector3.One
    /// IL_006a: this.SpecularColor = Vector3.Zero
    /// ```
    func testAFreshLightCachesTheThreeDefaultsAndWritesOnlyTheDirection() throws {
        let light = try G.DirectionalLight(
            directionParameter: nil, diffuseColorParameter: nil,
            specularColorParameter: nil, cloneSource: nil)
        XCTAssertFalse(light.Enabled)
        XCTAssertTrue(light.Direction == F.Vector3.Down)
        XCTAssertTrue(light.DiffuseColor == F.Vector3.One)
        XCTAssertTrue(light.SpecularColor == F.Vector3.Zero)
    }

    /// The clone constructor copies the four cached values FIELD TO FIELD, so
    /// no setter runs and no parameter is written. A light cloned while
    /// enabled stays enabled without re-pushing anything.
    func testACloneCopiesTheCachesWithoutRunningASetter() throws {
        let game = try run { game, device in
            let host = try G.AlphaTestEffect(device: device)
            let parameter = try XCTUnwrap(host.Parameters?["DiffuseColor"])

            let source = try G.DirectionalLight(
                directionParameter: nil, diffuseColorParameter: nil,
                specularColorParameter: nil, cloneSource: nil)
            try source.SetEnabled(true)
            try source.SetDiffuseColor(F.Vector3(0.25, 0.5, 0.75))

            // The clone gets a REAL parameter. If the constructor assigned
            // through the setters instead of copying the fields, enabling it
            // would push a colour into this parameter -- so the parameter
            // being untouched is what "field to field" means, and asserting
            // only the copied values would not notice the difference.
            try parameter.SetValue(F.Vector3(9, 9, 9))
            let clone = try G.DirectionalLight(
                directionParameter: nil, diffuseColorParameter: parameter,
                specularColorParameter: nil, cloneSource: source)
            game.observations["untouched"] = "\(try parameter.GetValueVector3())"
            game.observations["enabled"] = "\(clone.Enabled)"
            game.observations["direction"] = "\(clone.Direction == source.Direction)"
            game.observations["diffuse"] =
                "\(clone.DiffuseColor == F.Vector3(0.25, 0.5, 0.75))"
            game.observations["specular"] =
                "\(clone.SpecularColor == source.SpecularColor)"
            try host.Dispose()
        }
        XCTAssertEqual(game.observations["untouched"], "\(F.Vector3(9, 9, 9))")
        XCTAssertEqual(game.observations["enabled"], "true")
        XCTAssertEqual(game.observations["direction"], "true")
        XCTAssertEqual(game.observations["diffuse"], "true")
        XCTAssertEqual(game.observations["specular"], "true")
    }

    /// **Disabling a light does not clear what it remembers.** `set_Enabled`
    /// writes `Vector3.Zero` to the two colour PARAMETERS, and leaves
    /// `cachedDiffuseColor` and `cachedSpecularColor` alone — so the getters,
    /// which read the caches, answer exactly what they answered before.
    func testDisablingALightLeavesItsRememberedColorsAlone() throws {
        let light = try G.DirectionalLight(
            directionParameter: nil, diffuseColorParameter: nil,
            specularColorParameter: nil, cloneSource: nil)
        try light.SetEnabled(true)
        try light.SetDiffuseColor(F.Vector3(0.1, 0.2, 0.3))
        try light.SetSpecularColor(F.Vector3(0.4, 0.5, 0.6))
        try light.SetEnabled(false)
        XCTAssertTrue(light.DiffuseColor == F.Vector3(0.1, 0.2, 0.3))
        XCTAssertTrue(light.SpecularColor == F.Vector3(0.4, 0.5, 0.6))
    }

    /// `set_Enabled` opens with `if (this.enabled == value) return;` — setting
    /// it to what it already is does nothing at all, which is observable
    /// through the parameters it would otherwise have written.
    func testSettingEnabledToItsCurrentValueIsANoOp() throws {
        let game = try run { game, device in
            let host = try G.AlphaTestEffect(device: device)
            let diffuse = try XCTUnwrap(host.Parameters?["DiffuseColor"])
            try diffuse.SetValue(F.Vector3(9, 9, 9))
            let light = try G.DirectionalLight(
                directionParameter: nil, diffuseColorParameter: diffuse,
                specularColorParameter: nil, cloneSource: nil)
            // Still disabled, and set to the value it already has: the
            // parameter must be untouched.
            try light.SetEnabled(false)
            game.observations["after no-op"] = "\(try diffuse.GetValueVector3())"
            // A real transition writes Zero, because the light is disabled
            // on the way in and enabled on the way out writes the cache.
            try light.SetEnabled(true)
            game.observations["after enable"] = "\(try diffuse.GetValueVector3())"
            try host.Dispose()
        }
        XCTAssertEqual(game.observations["after no-op"], "\(F.Vector3(9, 9, 9))")
        XCTAssertEqual(game.observations["after enable"], "\(F.Vector3.One)")
    }

    /// The parameter half, measured against a real `EffectParameter`.
    ///
    /// `AlphaTestEffect` is what makes this testable: it publishes six shader
    /// parameters where `BasicEffect` publishes none, so a standalone light
    /// can be built over one of them and every push observed.
    func testTheParameterWritesFollowTheEnabledFlag() throws {
        let game = try run { game, device in
            let host = try G.AlphaTestEffect(device: device)
            let diffuse = try XCTUnwrap(host.Parameters?["DiffuseColor"])
            let light = try G.DirectionalLight(
                directionParameter: nil, diffuseColorParameter: diffuse,
                specularColorParameter: nil, cloneSource: nil)

            // Disabled: the value is cached and the parameter is NOT written.
            try diffuse.SetValue(F.Vector3(7, 7, 7))
            try light.SetDiffuseColor(F.Vector3(0.25, 0.5, 0.75))
            game.observations["disabled write"] = "\(try diffuse.GetValueVector3())"
            game.observations["disabled cache"] = "\(light.DiffuseColor)"

            // Enabling pushes the remembered value.
            try light.SetEnabled(true)
            game.observations["enabled push"] = "\(try diffuse.GetValueVector3())"

            // Enabled: the write goes straight through.
            try light.SetDiffuseColor(F.Vector3(0.125, 0.25, 0.5))
            game.observations["enabled write"] = "\(try diffuse.GetValueVector3())"

            // Disabling writes Zero to the parameter, not to the cache.
            try light.SetEnabled(false)
            game.observations["disabled zero"] = "\(try diffuse.GetValueVector3())"
            game.observations["cache survives"] = "\(light.DiffuseColor)"
            try host.Dispose()
        }
        XCTAssertEqual(game.observations["disabled write"], "\(F.Vector3(7, 7, 7))")
        XCTAssertEqual(game.observations["disabled cache"], "\(F.Vector3(0.25, 0.5, 0.75))")
        XCTAssertEqual(game.observations["enabled push"], "\(F.Vector3(0.25, 0.5, 0.75))")
        XCTAssertEqual(game.observations["enabled write"], "\(F.Vector3(0.125, 0.25, 0.5))")
        XCTAssertEqual(game.observations["disabled zero"], "\(F.Vector3.Zero)")
        XCTAssertEqual(game.observations["cache survives"], "\(F.Vector3(0.125, 0.25, 0.5))")
    }

    /// `set_Direction` is the one setter that does not consult `enabled`: it
    /// writes its parameter unconditionally.
    func testTheDirectionParameterIsWrittenEvenWhileDisabled() throws {
        let game = try run { game, device in
            let host = try G.AlphaTestEffect(device: device)
            let parameter = try XCTUnwrap(host.Parameters?["DiffuseColor"])
            let light = try G.DirectionalLight(
                directionParameter: parameter, diffuseColorParameter: nil,
                specularColorParameter: nil, cloneSource: nil)
            try light.SetDirection(F.Vector3(0.5, 0.25, 0.125))
            game.observations["written"] = "\(try parameter.GetValueVector3())"
            game.observations["enabled"] = "\(light.Enabled)"
            try host.Dispose()
        }
        XCTAssertEqual(game.observations["enabled"], "false")
        XCTAssertEqual(game.observations["written"], "\(F.Vector3(0.5, 0.25, 0.125))")
    }

    // ------------------------------------------------------------------
    // The five effects.

    /// Every stock effect is an `Effect`, is a `GraphicsResource`, and answers
    /// the device that made it.
    func testTheFiveStockEffectsAreEffectsOfTheirDevice() throws {
        let game = try run { game, device in
            let effects: [(String, G.Effect)] = [
                ("basic", try G.BasicEffect(device: device)),
                ("alphaTest", try G.AlphaTestEffect(device: device)),
                ("dualTexture", try G.DualTextureEffect(device: device)),
                ("environment", try G.EnvironmentMapEffect(device: device)),
                ("skinned", try G.SkinnedEffect(device: device)),
            ]
            // The array's element type is the assertion: a type that did not
            // derive from `Effect` could not be put in it. What varies at run
            // time -- and what is recorded -- is the device each one answers.
            for (name, effect) in effects {
                game.observations[name] = "\(effect.GraphicsDevice === device)"
                try effect.Dispose()
            }
        }
        for name in ["basic", "alphaTest", "dualTexture", "environment", "skinned"] {
            XCTAssertEqual(game.observations[name], "true", name)
        }
    }

    /// `BasicEffect(GraphicsDevice)`'s own IL, every value of it.
    ///
    /// ```text
    /// world = view = projection = Matrix.Identity
    /// diffuseColor = Vector3.One      emissiveColor = Vector3.Zero
    /// ambientLightColor = Vector3.Zero
    /// alpha = 1                        fogEnd = 1
    /// DirectionalLight0.Enabled = true
    /// SpecularColor = Vector3.One      SpecularPower = 16
    /// ```
    func testBasicEffectsConstructorEstablishesXnasDefaults() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            game.observations["world"] = "\(effect.World == F.Matrix.Identity)"
            game.observations["view"] = "\(effect.View == F.Matrix.Identity)"
            game.observations["projection"] = "\(effect.Projection == F.Matrix.Identity)"
            game.observations["diffuse"] = "\(effect.DiffuseColor == F.Vector3.One)"
            game.observations["emissive"] = "\(effect.EmissiveColor == F.Vector3.Zero)"
            game.observations["ambient"] = "\(effect.AmbientLightColor == F.Vector3.Zero)"
            game.observations["alpha"] = "\(effect.Alpha)"
            game.observations["fogEnd"] = "\(effect.FogEnd)"
            game.observations["fogStart"] = "\(effect.FogStart)"
            game.observations["lighting"] = "\(effect.LightingEnabled)"
            game.observations["light0"] = "\(effect.DirectionalLight0?.Enabled ?? false)"
            game.observations["light1"] = "\(effect.DirectionalLight1?.Enabled ?? true)"
            game.observations["light2"] = "\(effect.DirectionalLight2?.Enabled ?? true)"
            game.observations["specular"] = "\(try effect.SpecularColor == F.Vector3.One)"
            game.observations["power"] = "\(try effect.SpecularPower)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["world"], "true")
        XCTAssertEqual(game.observations["view"], "true")
        XCTAssertEqual(game.observations["projection"], "true")
        XCTAssertEqual(game.observations["diffuse"], "true")
        XCTAssertEqual(game.observations["emissive"], "true")
        XCTAssertEqual(game.observations["ambient"], "true")
        XCTAssertEqual(game.observations["alpha"], "1.0")
        XCTAssertEqual(game.observations["fogEnd"], "1.0")
        XCTAssertEqual(game.observations["fogStart"], "0.0")
        XCTAssertEqual(game.observations["lighting"], "false")
        XCTAssertEqual(game.observations["light0"], "true")
        XCTAssertEqual(game.observations["light1"], "false")
        XCTAssertEqual(game.observations["light2"], "false")
        XCTAssertEqual(game.observations["specular"], "true")
        XCTAssertEqual(game.observations["power"], "16.0")
    }

    /// The other four constructors' defaults, which live on the NATIVE side
    /// because the properties that read them are parameter-backed.
    ///
    /// XNA sets each of these in its own constructor —
    /// `EnvironmentMapAmount = 1`, `FresnelFactor = 1`,
    /// `EnvironmentMapSpecular = Zero`, `AlphaFunction = Greater` (`ldc.i4.6`),
    /// `ReferenceAlpha = 0`, `SpecularPower = 16`, `SpecularColor = One`,
    /// `WeightsPerVertex = 4`, and seventy-two identity bones. A
    /// parameter-backed property reads the device, so what a caller sees is
    /// whatever the native constructor established: this asserts that it is
    /// what XNA's establishes, which `build-probe/f69_defaults.c` measured
    /// before it was relied on.
    func testTheParameterBackedDefaultsAreTheOnesXnasConstructorsSet() throws {
        let game = try run { game, device in
            let environment = try G.EnvironmentMapEffect(device: device)
            game.observations["amount"] = "\(try environment.EnvironmentMapAmount)"
            game.observations["fresnel"] = "\(try environment.FresnelFactor)"
            game.observations["envSpecular"] =
                "\(try environment.EnvironmentMapSpecular == F.Vector3.Zero)"

            let alphaTest = try G.AlphaTestEffect(device: device)
            game.observations["function"] = "\(alphaTest.AlphaFunction)"
            game.observations["reference"] = "\(alphaTest.ReferenceAlpha)"

            let skinned = try G.SkinnedEffect(device: device)
            game.observations["power"] = "\(try skinned.SpecularPower)"
            game.observations["specular"] =
                "\(try skinned.SpecularColor == F.Vector3.One)"
            game.observations["weights"] = "\(skinned.WeightsPerVertex)"
            let bones = try skinned.GetBoneTransforms(G.SkinnedEffect.MaxBones)
            game.observations["bones"] = "\(bones.count)"
            game.observations["identity"] =
                "\(bones.allSatisfy { $0 == F.Matrix.Identity })"

            try environment.Dispose()
            try alphaTest.Dispose()
            try skinned.Dispose()
        }
        XCTAssertEqual(game.observations["amount"], "1.0")
        XCTAssertEqual(game.observations["fresnel"], "1.0")
        XCTAssertEqual(game.observations["envSpecular"], "true")
        XCTAssertEqual(game.observations["function"], "Greater")
        XCTAssertEqual(game.observations["reference"], "0")
        XCTAssertEqual(game.observations["power"], "16.0")
        XCTAssertEqual(game.observations["specular"], "true")
        XCTAssertEqual(game.observations["weights"], "4")
        XCTAssertEqual(game.observations["bones"], "72")
        XCTAssertEqual(game.observations["identity"], "true")
    }

    /// The three lights are held, not fetched: `effect.DirectionalLight0 ===
    /// effect.DirectionalLight0`.
    ///
    /// CNA hands back a fresh owned view handle on every
    /// `get_directional_light`, so without the cache this is false and a
    /// caller who kept a light would be writing to an object the effect no
    /// longer consults.
    func testAnEffectsLightsAreTheSameObjectsEveryTime() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            game.observations["same0"] =
                "\(effect.DirectionalLight0 === effect.DirectionalLight0)"
            game.observations["distinct"] =
                "\(effect.DirectionalLight0 !== effect.DirectionalLight1)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["same0"], "true")
        XCTAssertEqual(game.observations["distinct"], "true")
    }

    /// `EffectHelpers.EnableDefaultLighting`, all ten vectors of it, read out
    /// of the IL rather than out of CNA — which produces the same ten.
    func testEnableDefaultLightingReproducesXnasTable() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            try effect.EnableDefaultLighting()
            game.observations["lighting"] = "\(effect.LightingEnabled)"
            game.observations["ambient"] = "\(effect.AmbientLightColor)"
            for (index, light) in [
                effect.DirectionalLight0, effect.DirectionalLight1,
                effect.DirectionalLight2,
            ].enumerated() {
                let light = try XCTUnwrap(light)
                game.observations["dir\(index)"] = "\(light.Direction)"
                game.observations["diff\(index)"] = "\(light.DiffuseColor)"
                game.observations["spec\(index)"] = "\(light.SpecularColor)"
                game.observations["on\(index)"] = "\(light.Enabled)"
            }
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["lighting"], "true")
        XCTAssertEqual(game.observations["ambient"],
                       "\(F.Vector3(0.05333332, 0.09882354, 0.1819608))")
        XCTAssertEqual(game.observations["dir0"],
                       "\(F.Vector3(-0.5265408, -0.5735765, -0.6275069))")
        XCTAssertEqual(game.observations["diff0"],
                       "\(F.Vector3(1, 0.9607844, 0.8078432))")
        XCTAssertEqual(game.observations["spec0"],
                       "\(F.Vector3(1, 0.9607844, 0.8078432))")
        XCTAssertEqual(game.observations["dir1"],
                       "\(F.Vector3(0.7198464, 0.3420201, 0.6040227))")
        XCTAssertEqual(game.observations["diff1"],
                       "\(F.Vector3(0.9647059, 0.7607844, 0.4078432))")
        XCTAssertEqual(game.observations["spec1"], "\(F.Vector3.Zero)")
        XCTAssertEqual(game.observations["dir2"],
                       "\(F.Vector3(0.4545195, -0.7660444, 0.4545195))")
        XCTAssertEqual(game.observations["diff2"],
                       "\(F.Vector3(0.3231373, 0.3607844, 0.3937255))")
        XCTAssertEqual(game.observations["spec2"],
                       "\(F.Vector3(0.3231373, 0.3607844, 0.3937255))")
        for index in 0..<3 {
            XCTAssertEqual(game.observations["on\(index)"], "true")
        }
    }

    /// The two effects whose lighting cannot be turned off.
    ///
    /// The getter is `ldc.i4.1; ret` and the setter refuses only `false`, with
    /// the effect's own type name formatted into `CantDisableLighting`.
    func testTheTwoAlwaysLitEffectsRefuseToDisableLighting() throws {
        let game = try run { game, device in
            let environment = try G.EnvironmentMapEffect(device: device)
            let skinned = try G.SkinnedEffect(device: device)
            game.observations["environment on"] = "\(environment.LightingEnabled)"
            game.observations["skinned on"] = "\(skinned.LightingEnabled)"
            // `true` returns silently.
            try environment.SetLightingEnabled(true)
            try skinned.SetLightingEnabled(true)
            game.observations["true accepted"] = "yes"
            for (name, effect) in [
                ("environment", environment as any Microsoft.Xna.Framework.Graphics.IEffectLights),
                ("skinned", skinned as any Microsoft.Xna.Framework.Graphics.IEffectLights),
            ] {
                do {
                    try effect.SetLightingEnabled(false)
                    game.observations[name] = "accepted"
                } catch let error as CNANotSupportedException {
                    game.observations[name] = error.Message
                }
            }
            try environment.Dispose()
            try skinned.Dispose()
        }
        XCTAssertEqual(game.observations["environment on"], "true")
        XCTAssertEqual(game.observations["skinned on"], "true")
        XCTAssertEqual(game.observations["true accepted"], "yes")
        XCTAssertEqual(game.observations["environment"],
                       "EnvironmentMapEffect does not support setting LightingEnabled to false.")
        XCTAssertEqual(game.observations["skinned"],
                       "SkinnedEffect does not support setting LightingEnabled to false.")
    }

    /// `BasicEffect`'s own `LightingEnabled` is a plain settable property, and
    /// its `SetLightingEnabled` — the witness Swift's conformance rules force
    /// — writes the same state.
    func testBasicEffectsLightingEnabledIsAPlainSettableProperty() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            effect.LightingEnabled = true
            game.observations["property"] = "\(effect.LightingEnabled)"
            let lights: any Microsoft.Xna.Framework.Graphics.IEffectLights = effect
            try lights.SetLightingEnabled(false)
            game.observations["witness"] = "\(effect.LightingEnabled)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["property"], "true")
        XCTAssertEqual(game.observations["witness"], "false")
    }

    // ------------------------------------------------------------------
    // SkinnedEffect's bones and weights.

    /// `SetBoneTransforms`' two refusals, which are two DIFFERENT exceptions
    /// on adjacent tests of the same argument.
    func testSetBoneTransformsRefusesEmptyAndOversizedArraysDifferently() throws {
        let game = try run { game, device in
            let effect = try G.SkinnedEffect(device: device)
            do {
                try effect.SetBoneTransforms([])
                game.observations["empty"] = "accepted"
            } catch let error as CNAArgumentNullException {
                game.observations["empty"] = "\(error.ParamName ?? "-")|\(error.Message)"
            }
            do {
                try effect.SetBoneTransforms(
                    Array(repeating: F.Matrix.Identity, count: 73))
                game.observations["too many"] = "accepted"
            } catch let error as CNAArgumentException {
                game.observations["too many"] =
                    "\(error.ParamName ?? "nil")|\(error.Message)"
            }
            // Seventy-two is the largest accepted, and one is the smallest.
            try effect.SetBoneTransforms(
                Array(repeating: F.Matrix.Identity, count: 72))
            try effect.SetBoneTransforms([F.Matrix.Identity])
            game.observations["bounds"] = "accepted"
            try effect.Dispose()
        }
        // `Message` carries .NET's own `\r\nParameter name: …` suffix, which
        // `ArgumentException` appends to whatever message it was given. That
        // is BCL behaviour, not XNA's, and the projection reproduces it.
        XCTAssertEqual(
            game.observations["empty"],
            "boneTransforms|This method does not accept null for this parameter."
            + "\r\nParameter name: boneTransforms")
        // The oversized refusal carries NO ParamName -- it is the
        // one-argument ArgumentException constructor.
        // No `\r\nParameter name:` suffix here, because there is no parameter
        // name: this really is the one-argument `ArgumentException(string)`,
        // and the absence of the suffix is what proves it.
        XCTAssertEqual(
            game.observations["too many"],
            "nil|SkinnedEffect supports a maximum of 72 bones.")
        XCTAssertEqual(game.observations["bounds"], "accepted")
    }

    /// `GetBoneTransforms`' two refusals are the same exception type with and
    /// without a message, and `MaxBones` is the bound both share.
    func testGetBoneTransformsRefusesNonPositiveAndOversizedCounts() throws {
        let game = try run { game, device in
            let effect = try G.SkinnedEffect(device: device)
            do {
                _ = try effect.GetBoneTransforms(0)
                game.observations["zero"] = "accepted"
            } catch let error as CNAArgumentOutOfRangeException {
                game.observations["zero"] = "\(error.ParamName ?? "-")|\(error.Message)"
            }
            do {
                _ = try effect.GetBoneTransforms(G.SkinnedEffect.MaxBones + 1)
                game.observations["too many"] = "accepted"
            } catch let error as CNAArgumentOutOfRangeException {
                game.observations["too many"] =
                    "\(error.ParamName ?? "-")|\(error.Message)"
            }
            try effect.Dispose()
        }
        XCTAssertEqual(G.SkinnedEffect.MaxBones, 72)
        // `ArgumentOutOfRangeException(string paramName)` supplies the BCL's
        // own default message -- XNA passes no message at all here, and what
        // a caller sees is the framework's.
        XCTAssertEqual(
            game.observations["zero"],
            "count|Specified argument was out of the range of valid values."
            + "\r\nParameter name: count")
        XCTAssertTrue(
            game.observations["too many"]?.hasPrefix(
                "count|SkinnedEffect supports a maximum of 72 bones.") == true,
            "\(game.observations["too many"] ?? "-")")
    }

    /// A round trip through the bone transforms, and the `M44` the reader
    /// restores.
    ///
    /// XNA packs a bone as three rows, so the fourth column comes back from
    /// the parameter as whatever the packed form left there and the reader's
    /// loop writes `M44 = 1`. This asserts the restoration directly: a matrix
    /// written with `M44 = 0` reads back with `M44 = 1`.
    func testGetBoneTransformsRestoresTheFourthDiagonal() throws {
        let game = try run { game, device in
            let effect = try G.SkinnedEffect(device: device)
            var bone = F.Matrix.Identity
            bone.M41 = 3
            bone.M44 = 0
            try effect.SetBoneTransforms([bone, bone])
            let read = try effect.GetBoneTransforms(2)
            game.observations["count"] = "\(read.count)"
            game.observations["m44"] = "\(read.first?.M44 ?? -1)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["count"], "2")
        XCTAssertEqual(game.observations["m44"], "1.0")
    }

    /// `set_WeightsPerVertex` accepts 1, 2 and 4 and refuses everything else,
    /// before writing anything.
    func testWeightsPerVertexAcceptsOnlyOneTwoAndFour() throws {
        let game = try run { game, device in
            let effect = try G.SkinnedEffect(device: device)
            game.observations["default"] = "\(effect.WeightsPerVertex)"
            for value: Int32 in [1, 2, 4] {
                try effect.SetWeightsPerVertex(value)
            }
            game.observations["accepted"] = "\(effect.WeightsPerVertex)"
            do {
                try effect.SetWeightsPerVertex(3)
                game.observations["three"] = "accepted"
            } catch let error as CNAArgumentOutOfRangeException {
                game.observations["three"] = "\(error.ParamName ?? "-")|\(error.Message)"
            }
            // The throw is BEFORE the field write, so the value is unchanged.
            game.observations["unchanged"] = "\(effect.WeightsPerVertex)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["default"], "4")
        XCTAssertEqual(game.observations["accepted"], "4")
        XCTAssertTrue(
            game.observations["three"]?.hasPrefix(
                "value|SkinnedEffect.WeightsPerVertex must be 1, 2, or 4.") == true,
            "\(game.observations["three"] ?? "-")")
        XCTAssertEqual(game.observations["unchanged"], "4")
    }

    // ------------------------------------------------------------------
    // The two halves of the state, and what OnApply does with them.

    /// A field-backed property answers its new value immediately and reaches
    /// the device only at `OnApply`; a parameter-backed one reaches the device
    /// at once. That is the whole rule, and it is observable because the
    /// parameter-backed reader is a native read.
    func testTheParameterBackedStateReachesTheDeviceWithoutAnApply() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            try effect.SetSpecularPower(24)
            game.observations["immediate"] = "\(try effect.SpecularPower)"
            effect.Alpha = 0.5
            game.observations["cached"] = "\(effect.Alpha)"
            try effect.OnApply()
            game.observations["after apply"] = "\(effect.Alpha)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["immediate"], "24.0")
        XCTAssertEqual(game.observations["cached"], "0.5")
        XCTAssertEqual(game.observations["after apply"], "0.5")
    }

    /// **`OnApply` really does reach the device**, which nothing else here
    /// observes: every field-backed property answers its own cache, so a
    /// setter that forgot to mark itself dirty would look correct from the
    /// public surface and never reach the shader.
    ///
    /// This reads the native values back through the routes `OnApply` writes.
    /// It is the only test in the milestone that goes behind the projection,
    /// and it is the one that makes the whole deferred-write architecture an
    /// assertion rather than a design note.
    func testOnApplyPushesTheDirtyValuesToTheDevice() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            let functions = effect.nativeStorage.runtime.functions
            let handle = try effect.validatedHandle("test")

            effect.Alpha = 0.375
            effect.DiffuseColor = F.Vector3(0.125, 0.25, 0.5)
            effect.TextureEnabled = true

            // Before the apply the device still holds the constructor's values.
            var alpha: Float = 0
            _ = functions.basicEffectGetAlpha(handle, &alpha)
            game.observations["before"] = "\(alpha)"

            try effect.OnApply()

            alpha = 0
            _ = functions.basicEffectGetAlpha(handle, &alpha)
            game.observations["after"] = "\(alpha)"
            var diffuse = CNASwift_Vector3()
            _ = functions.basicEffectGetDiffuseColor(handle, &diffuse)
            game.observations["diffuse"] = "\(diffuse.x) \(diffuse.y) \(diffuse.z)"
            var textured: UInt8 = 0
            _ = functions.basicEffectGetTextureEnabled(handle, &textured)
            game.observations["textured"] = "\(textured)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["before"], "1.0")
        XCTAssertEqual(game.observations["after"], "0.375")
        XCTAssertEqual(game.observations["diffuse"], "0.125 0.25 0.5")
        XCTAssertEqual(game.observations["textured"], "1")
    }

    /// `OnApply` on an untouched effect pushes nothing and still succeeds, and
    /// a second `OnApply` after a change pushes only that change: the dirty
    /// mask is cleared by the first.
    func testOnApplyIsIdempotentAndClearsWhatItPushed() throws {
        let game = try run { game, device in
            let effect = try G.AlphaTestEffect(device: device)
            try effect.OnApply()
            effect.ReferenceAlpha = 128
            try effect.OnApply()
            try effect.OnApply()
            game.observations["reference"] = "\(effect.ReferenceAlpha)"
            game.observations["survived"] = "yes"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["reference"], "128")
        XCTAssertEqual(game.observations["survived"], "yes")
    }

    /// A texture assigned to an effect comes back as the same object, and
    /// clearing it answers nil.
    func testAnAssignedTextureComesBackAsTheSameObject() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            let texture = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            game.observations["before"] = "\(try effect.Texture == nil)"
            try effect.SetTexture(texture)
            game.observations["identity"] = "\(try effect.Texture === texture)"
            try effect.SetTexture(nil)
            game.observations["after"] = "\(try effect.Texture == nil)"
            try texture.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["before"], "true")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["after"], "true")
    }

    /// `DualTextureEffect`'s two properties are two LAYERS of one CNA route,
    /// and they must not be the same layer.
    func testTheTwoDualTextureLayersAreIndependent() throws {
        let game = try run { game, device in
            let effect = try G.DualTextureEffect(device: device)
            let first = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            let second = try G.Texture2D(graphicsDevice: device, width: 8, height: 8)
            try effect.SetTexture(first)
            game.observations["second still empty"] = "\(try effect.Texture2 == nil)"
            try effect.SetTexture2(second)
            game.observations["first"] = "\(try effect.Texture === first)"
            game.observations["second"] = "\(try effect.Texture2 === second)"
            try effect.SetTexture(nil)
            game.observations["second survives"] = "\(try effect.Texture2 === second)"

            // CNA RETAINS an assigned texture, and refuses to dispose one that
            // an effect still points at. XNA has no such rule -- its
            // `EffectParameter` stores a raw pointer and disposing the texture
            // is the caller's problem -- so this is a native-owned refusal
            // recorded, not a managed test reproduced. It is asserted because
            // a consumer meets it: the texture cleared from layer zero
            // disposes, the one still on layer one does not.
            try first.Dispose()
            do {
                try second.Dispose()
                game.observations["retained"] = "accepted"
            } catch {
                game.observations["retained"] = "refused"
            }
            try effect.SetTexture2(nil)
            try second.Dispose()
            game.observations["released"] = "accepted"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["second still empty"], "true")
        XCTAssertEqual(game.observations["first"], "true")
        XCTAssertEqual(game.observations["second"], "true")
        XCTAssertEqual(game.observations["second survives"], "true")
        XCTAssertEqual(game.observations["retained"], "refused")
        XCTAssertEqual(game.observations["released"], "accepted")
    }

    // ------------------------------------------------------------------
    // Cloning, and EffectMaterial.

    /// `Clone` returns the derived type, a distinct object, with the cached
    /// state copied.
    func testCloningAStockEffectCopiesItsCachedState() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            effect.Alpha = 0.25
            effect.FogEnabled = true
            effect.World = F.Matrix.CreateTranslation(1, yPosition: 2, zPosition: 3)
            let clone = try XCTUnwrap(try effect.Clone() as? G.BasicEffect)
            game.observations["distinct"] = "\(clone !== effect)"
            game.observations["alpha"] = "\(clone.Alpha)"
            game.observations["fog"] = "\(clone.FogEnabled)"
            game.observations["world"] = "\(clone.World == effect.World)"
            game.observations["lights"] =
                "\(clone.DirectionalLight0 !== effect.DirectionalLight0)"
            try clone.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["distinct"], "true")
        XCTAssertEqual(game.observations["alpha"], "0.25")
        XCTAssertEqual(game.observations["fog"], "true")
        XCTAssertEqual(game.observations["world"], "true")
        // A clone is a different native effect, so it has different lights.
        XCTAssertEqual(game.observations["lights"], "true")
    }

    /// `EffectMaterial(Effect cloneSource)` adds no member and no state: it
    /// adds a type, over any effect at all.
    func testEffectMaterialIsAnEffectOverAnyCloneSource() throws {
        let game = try run { game, device in
            let source = try G.AlphaTestEffect(device: device)
            // Typed as `Effect`, which only compiles because it derives from
            // one -- the CLR base the contract records.
            let material: G.Effect = try G.EffectMaterial(cloneSource: source)
            game.observations["distinct"] = "\(material !== source)"
            game.observations["device"] = "\(material.GraphicsDevice === device)"
            try material.Dispose()
            try source.Dispose()
        }
        XCTAssertEqual(game.observations["distinct"], "true")
        XCTAssertEqual(game.observations["device"], "true")
    }

    /// The two effects with no lighting model do not conform to
    /// `IEffectLights`, which is what CNA enforces on its own side by refusing
    /// every lighting route on them.
    func testTheUnlitEffectsDoNotConformToIEffectLights() throws {
        let game = try run { game, device in
            let alphaTest = try G.AlphaTestEffect(device: device)
            let dualTexture = try G.DualTextureEffect(device: device)
            game.observations["alphaTest"] = "\(alphaTest is any Microsoft.Xna.Framework.Graphics.IEffectLights)"
            game.observations["dualTexture"] = "\(dualTexture is any Microsoft.Xna.Framework.Graphics.IEffectLights)"
            // The positive control is a compile-time one: this only type-checks
            // because `BasicEffect` declares the conformance, and a runtime
            // `is` beside it would be a tautology.
            let lit: any Microsoft.Xna.Framework.Graphics.IEffectLights =
                try G.BasicEffect(device: device)
            game.observations["basic"] = "\(lit.LightingEnabled == false)"
            try alphaTest.Dispose()
            try dualTexture.Dispose()
        }
        XCTAssertEqual(game.observations["alphaTest"], "false")
        XCTAssertEqual(game.observations["dualTexture"], "false")
        XCTAssertEqual(game.observations["basic"], "true")
    }

    /// A disposed effect refuses every member that reaches the device.
    func testADisposedStockEffectRefusesItsNativeMembers() throws {
        let game = try run { game, device in
            let effect = try G.BasicEffect(device: device)
            try effect.Dispose()
            do {
                _ = try effect.SpecularPower
                game.observations["getter"] = "accepted"
            } catch {
                game.observations["getter"] = "refused"
            }
            do {
                try effect.OnApply()
                game.observations["apply"] = "accepted"
            } catch {
                game.observations["apply"] = "refused"
            }
        }
        XCTAssertEqual(game.observations["getter"], "refused")
        XCTAssertEqual(game.observations["apply"], "refused")
    }
}
