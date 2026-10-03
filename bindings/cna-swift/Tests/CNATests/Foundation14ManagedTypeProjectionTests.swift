// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Test-only conformers. The pinned interfaces have no XNA conformer yet, so
// these exist purely to prove the Swift requirement sets are exactly the pinned
// member sets and are satisfiable. They are test code and add no public
// surface.
// A class, not a struct: `SetFogColor` projects a CLR setter that mutates a
// class, so the requirement is non-mutating and only a reference type can
// witness it. Every registered XNA implementor of this interface is a class.
private final class FogWitness: Microsoft.Xna.Framework.Graphics.IEffectFog {
    var FogEnabled: Bool = false
    var FogStart: Float = 0
    var FogEnd: Float = 0
    private var fogColor: Microsoft.Xna.Framework.Vector3 = .Zero

    // A non-throwing witness satisfies a `{ get throws }` requirement; the
    // reverse is what the compiler rejects.
    var FogColor: Microsoft.Xna.Framework.Vector3 { fogColor }

    func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws {
        fogColor = value
    }
}

private struct MatricesWitness: Microsoft.Xna.Framework.Graphics.IEffectMatrices {
    var World: Microsoft.Xna.Framework.Matrix = .Identity
    var View: Microsoft.Xna.Framework.Matrix = .Identity
    var Projection: Microsoft.Xna.Framework.Matrix = .Identity
}

final class Foundation14ManagedTypeProjectionTests: XCTestCase {
    func testVertexElementValueSemanticsAndProjection() {
        typealias Element = Microsoft.Xna.Framework.Graphics.VertexElement

        // Swift struct assignment copies; mutating the copy leaves the original
        // untouched. This is Swift value semantics, not an XNA observation.
        let original = Element(0, .Vector3, .Position, 0)
        var copy = original
        copy.Offset = 12
        copy.VertexElementFormat = .Color
        XCTAssertEqual(original.Offset, 0)
        XCTAssertEqual(original.VertexElementFormat, .Vector3)
        XCTAssertEqual(copy.Offset, 12)
        XCTAssertEqual(copy.VertexElementFormat, .Color)
        // The XNA value types deliberately gain no Swift Equatable
        // conformance, so the pinned operator is used directly.
        XCTAssertTrue(original != copy)

        // The two element enums stay ordinary Int32 raw enums inside the
        // struct, so an undefined pattern still has no representation.
        XCTAssertNil(Microsoft.Xna.Framework.Graphics.VertexElementFormat(rawValue: 12))
        XCTAssertNil(Microsoft.Xna.Framework.Graphics.VertexElementUsage(rawValue: 13))

        XCTAssertEqual(String(describing: Element.self), "VertexElement")
    }

    func testEffectInterfaceRequirementSetsAreSatisfiable() throws {
        let fog = FogWitness()
        fog.FogEnabled = true
        fog.FogStart = 1.5
        fog.FogEnd = 40
        try fog.SetFogColor(Microsoft.Xna.Framework.Vector3(1, 0, 0))
        let readFog: Microsoft.Xna.Framework.Graphics.IEffectFog = fog
        XCTAssertTrue(readFog.FogEnabled)
        XCTAssertEqual(readFog.FogStart.bitPattern, Float(1.5).bitPattern)
        XCTAssertEqual(readFog.FogEnd.bitPattern, Float(40).bitPattern)
        XCTAssertTrue(try readFog.FogColor == Microsoft.Xna.Framework.Vector3(1, 0, 0))

        // The writer is the only write path: the reader is get-only, so no
        // unchecked assignment to FogColor exists on the protocol at all.
        try readFog.SetFogColor(Microsoft.Xna.Framework.Vector3(0, 1, 0))
        XCTAssertTrue(try readFog.FogColor == Microsoft.Xna.Framework.Vector3(0, 1, 0))

        var matrices = MatricesWitness()
        matrices.World = Microsoft.Xna.Framework.Matrix.CreateTranslation(
            1, yPosition: 2, zPosition: 3)
        let readMatrices: Microsoft.Xna.Framework.Graphics.IEffectMatrices = matrices
        XCTAssertEqual(readMatrices.World.M41.bitPattern, Float(1).bitPattern)
        XCTAssertTrue(readMatrices.View == Microsoft.Xna.Framework.Matrix.Identity)
        XCTAssertTrue(readMatrices.Projection == Microsoft.Xna.Framework.Matrix.Identity)
    }

    func testSoundStateInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Audio.SoundState
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Playing),
            (1, .Paused),
            (2, .Stopped),
        ]

        XCTAssertEqual(expected.count, 3)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 3))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Playing
        var copy = original
        copy = .Stopped
        XCTAssertEqual(original, .Playing)
        XCTAssertEqual(copy, .Stopped)
    }

    func testAudioChannelsInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Audio.AudioChannels
        let expected: [(rawValue: Int32, value: E)] = [
            (1, .Mono),
            (2, .Stereo),
        ]

        XCTAssertEqual(expected.count, 2)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        // The pinned table has no zero literal, so 0 is undefined here.
        XCTAssertNil(E(rawValue: 0))
        XCTAssertNil(E(rawValue: 3))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Mono
        var copy = original
        copy = .Stereo
        XCTAssertEqual(original, .Mono)
        XCTAssertEqual(copy, .Stereo)
    }
}
