// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift-language qualification of the general CLR-property-accessor
// projection. None of this is XNA runtime behaviour and none of it is counted
// as such: it measures the shape the Swift compiler actually emitted.
final class Foundation22ProjectionTests: XCTestCase {
    typealias Emitter = Microsoft.Xna.Framework.Audio.AudioEmitter

    // Case C of the rule -- infallible getter, fallible setter. The reader
    // stays ordinary property syntax and the writer is a method, so no
    // unchecked write path exists. A settable property on a class would give a
    // ReferenceWritableKeyPath; DopplerScale gives only a read KeyPath, which
    // is the compiler agreeing there is no setter.
    func testDopplerScaleHasNoWritablePropertyPath() {
        let readable: KeyPath<Emitter, Float> = \Emitter.DopplerScale
        XCTAssertNil(readable as? ReferenceWritableKeyPath<Emitter, Float>)

        let writable: KeyPath<Emitter, Microsoft.Xna.Framework.Vector3> = \Emitter.Position
        XCTAssertNotNil(
            writable as? ReferenceWritableKeyPath<Emitter, Microsoft.Xna.Framework.Vector3>)
    }

    // The writer is the setter accessor of one CLR member, so it carries the
    // property's value and nothing else, and it throws.
    func testDopplerScaleWriterSignature() throws {
        let writer: (Emitter) -> (Float) throws -> Void = { emitter in
            emitter.SetDopplerScale
        }
        let emitter = Emitter()
        try writer(emitter)(4)
        XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(4).bitPattern)
    }

    // AudioEmitter is a CLR class, so the Swift projection is a class: two
    // references observe one instance, and the type is not copied on
    // assignment.
    func testAudioEmitterIsAReferenceType() throws {
        let first = Emitter()
        let second = first
        try second.SetDopplerScale(9)
        second.Position = Microsoft.Xna.Framework.Vector3(1, 2, 3)
        XCTAssertEqual(first.DopplerScale.bitPattern, Float(9).bitPattern)
        XCTAssertEqual(first.Position.X, 1)
        XCTAssertTrue(first === second)
    }

    // Case E of the rule -- a fallible getter and a fallible setter on a
    // protocol requirement. Swift rejects `set` beside a throwing getter
    // outright, so the reader is `{ get throws }` and the writer is a method.
    // A non-throwing witness still satisfies the throwing reader.
    func testEffectFogAccessorProjectionIsUsableThroughTheExistential() throws {
        final class Fog: Microsoft.Xna.Framework.Graphics.IEffectFog {
            var FogEnabled = false
            var FogStart: Float = 0
            var FogEnd: Float = 0
            private var color = Microsoft.Xna.Framework.Vector3.Zero
            var FogColor: Microsoft.Xna.Framework.Vector3 { color }
            func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws {
                color = value
            }
        }
        let fog: any Microsoft.Xna.Framework.Graphics.IEffectFog = Fog()
        try fog.SetFogColor(Microsoft.Xna.Framework.Vector3(3, 4, 5))
        let read = try fog.FogColor
        XCTAssertEqual(read.X, 3)
        XCTAssertEqual(read.Y, 4)
        XCTAssertEqual(read.Z, 5)
    }

    // The three infallible IEffectFog properties keep ordinary Swift property
    // syntax, so the projection is per accessor and not per type: one fallible
    // accessor does not make its siblings throw.
    func testEffectFogInfallibleAccessorsKeepPropertySyntax() {
        final class Fog: Microsoft.Xna.Framework.Graphics.IEffectFog {
            var FogEnabled = false
            var FogStart: Float = 0
            var FogEnd: Float = 0
            var FogColor: Microsoft.Xna.Framework.Vector3 { .Zero }
            func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws {}
        }
        var fog: any Microsoft.Xna.Framework.Graphics.IEffectFog = Fog()
        fog.FogEnabled = true
        fog.FogStart = 2
        fog.FogEnd = 3
        XCTAssertTrue(fog.FogEnabled)
        XCTAssertEqual(fog.FogStart, 2)
        XCTAssertEqual(fog.FogEnd, 3)
    }

    // The established indexed form is unchanged by the generalisation: a
    // fallible read/write CLR indexer is still the Item/SetItem pair, and a
    // read-only infallible one is still a plain Swift subscript.
    func testIndexedAccessorFormsAreUnchanged() throws {
        typealias F = Microsoft.Xna.Framework
        let keys = F.CurveKeyCollection()
        keys.Add(F.CurveKey(position: 0, value: 1))
        XCTAssertEqual(try keys.Item(0).Value, 1)
        try keys.SetItem(0, F.CurveKey(position: 0, value: 7))
        XCTAssertEqual(try keys.Item(0).Value, 7)
        XCTAssertThrowsError(try keys.Item(5))

        // KeyboardState.Item is infallible, so it stays subscript syntax.
        let keyboard = F.Input.KeyboardState([.A])
        XCTAssertEqual(keyboard[.A], .Down)
        XCTAssertEqual(keyboard[.B], .Up)
    }
}
