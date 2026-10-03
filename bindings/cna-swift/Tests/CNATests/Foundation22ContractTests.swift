// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 22 accessor-projection batch,
// transcribed from the registered, hash-matched Microsoft.Xna.Framework.dll IL.
extension PureValueTests {
    // AudioEmitter stores an XACT_EMITTER_DATA structure in X3DAudio's
    // left-handed space, exactly as AudioListener does, and every Vector3
    // accessor passes through UnsafeNativeStructures.FlipHandedness ==
    // Vector3(v.X, v.Y, -v.Z).
    func testAudioEmitterXnaContract() throws {
        typealias F = Microsoft.Xna.Framework
        let emitter = F.Audio.AudioEmitter()

        // `_Position` and `_Velocity` are seeded with Vector3.Zero WITHOUT the
        // flip while the getters flip on the way out, so the default Z is
        // negative zero -- equal to zero, different bit pattern.
        XCTAssertEqual(emitter.Position.X, 0)
        XCTAssertEqual(emitter.Position.Y, 0)
        XCTAssertEqual(emitter.Position.Z, 0)
        XCTAssertEqual(emitter.Position.Z.sign, .minus)
        XCTAssertEqual(emitter.Position.Z.bitPattern, Float(-0.0).bitPattern)
        XCTAssertEqual(emitter.Velocity.Z.bitPattern, Float(-0.0).bitPattern)

        // `_Forward` and `_Up` are seeded *through* the flip and read back as
        // exactly the Vector3 constants.
        XCTAssertEqual(emitter.Forward.X, 0)
        XCTAssertEqual(emitter.Forward.Y, 0)
        XCTAssertEqual(emitter.Forward.Z.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(emitter.Up.X, 0)
        XCTAssertEqual(emitter.Up.Y, 1)
        XCTAssertEqual(emitter.Up.Z.bitPattern, Float(0).bitPattern)

        // `ldc.r4 1` into `_DopplerScale`; the getter does not flip anything.
        XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(1).bitPattern)

        // The flip is a bitwise involution, so a written value round-trips
        // unchanged, negative zero and non-finite components included.
        emitter.Position = F.Vector3(1, -2, 3)
        XCTAssertEqual(emitter.Position.X, 1)
        XCTAssertEqual(emitter.Position.Y, -2)
        XCTAssertEqual(emitter.Position.Z, 3)
        emitter.Velocity = F.Vector3(0, 0, -0.0)
        XCTAssertEqual(emitter.Velocity.Z.bitPattern, Float(-0.0).bitPattern)
        emitter.Forward = F.Vector3(.nan, .infinity, -.infinity)
        XCTAssertTrue(emitter.Forward.X.isNaN)
        XCTAssertEqual(emitter.Forward.Y, .infinity)
        XCTAssertEqual(emitter.Forward.Z, -.infinity)
        emitter.Up = F.Vector3(7, 8, 9)
        XCTAssertEqual(emitter.Up.Z, 9)
    }

    // set_DopplerScale is the whole reason this type waited for the general
    // accessor rule:
    //
    //     IL_0000:  ldarg.1
    //     IL_0001:  ldc.r4     0.0
    //     IL_0006:  bge.un.s   IL_0018
    //     ...       newobj ArgumentOutOfRangeException("value", ...); throw
    //     IL_0018:  stfld      _DopplerScale
    //
    // `bge.un.s` is the UNORDERED form, so the branch past the throw is taken
    // when the value is >= 0 *or* when the comparison is unordered. NaN is
    // therefore accepted and stored; only an ordered `value < 0` throws.
    func testAudioEmitterDopplerScaleBoundaryXnaContract() throws {
        typealias F = Microsoft.Xna.Framework
        let emitter = F.Audio.AudioEmitter()

        // Accepted: +0, -0, positive finite, +Infinity, and every NaN.
        try emitter.SetDopplerScale(0)
        XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(0).bitPattern)

        try emitter.SetDopplerScale(-0.0)
        XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(-0.0).bitPattern)
        XCTAssertEqual(emitter.DopplerScale.sign, .minus)
        XCTAssertEqual(emitter.DopplerScale, 0)

        try emitter.SetDopplerScale(Float.leastNonzeroMagnitude)
        XCTAssertEqual(
            emitter.DopplerScale.bitPattern, Float.leastNonzeroMagnitude.bitPattern)

        try emitter.SetDopplerScale(2.5)
        XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(2.5).bitPattern)

        try emitter.SetDopplerScale(.greatestFiniteMagnitude)
        XCTAssertEqual(
            emitter.DopplerScale.bitPattern, Float.greatestFiniteMagnitude.bitPattern)

        try emitter.SetDopplerScale(.infinity)
        XCTAssertEqual(emitter.DopplerScale, .infinity)

        // Every NaN, whatever its sign bit or payload. The bit patterns are
        // built explicitly rather than with `-Float.nan`, because negating a
        // NaN literal is a Swift/LLVM constant-folding detail -- the optimizer
        // does not have to preserve the sign bit -- and none of that is XNA
        // behaviour. XNA's own behaviour is that the comparison is unordered,
        // so the value is stored verbatim.
        for pattern: UInt32 in [
            0x7FC0_0000,   // quiet NaN, sign 0
            0xFFC0_0000,   // quiet NaN, sign 1
            0x7F80_0001,   // signaling NaN, smallest payload
            0xFF92_3456,   // negative NaN with a payload
        ] {
            let value = Float(bitPattern: pattern)
            XCTAssertTrue(value.isNaN)
            try emitter.SetDopplerScale(value)
            XCTAssertTrue(emitter.DopplerScale.isNaN)
            XCTAssertEqual(emitter.DopplerScale.bitPattern, pattern)
        }

        // Rejected: every ordered negative, down to the smallest subnormal.
        // The store is not performed, so the previous value survives.
        try emitter.SetDopplerScale(3)
        for rejected in [
            -Float.leastNonzeroMagnitude,
            -Float.leastNormalMagnitude,
            -0.5,
            -1,
            -Float.greatestFiniteMagnitude,
            -Float.infinity,
        ] {
            // ArgumentOutOfRangeException("value", InvalidEmitterDopplerScale)
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "The doppler scale of an audio emitter must be greater "
                    + "than or equal to zero.", paramName: "value"),
                paramName: "value",
                hResult: Int32(bitPattern: 0x8013_1502)
            ) {
                try emitter.SetDopplerScale(rejected)
            }
            XCTAssertEqual(emitter.DopplerScale.bitPattern, Float(3).bitPattern)
        }
    }
}
