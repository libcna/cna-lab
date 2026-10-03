// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the PresentationParameters
// class mapping and for the general `System.IntPtr -> Int` BCL rule it is the
// first XNA public signature to exercise. These are Swift/CLR mapping facts,
// not XNA runtime observations, so they are deliberately kept out of the pure
// XNA-derived behaviour corpus.
final class PresentationParametersProjectionTests: XCTestCase {
    typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

    func testPublicConstructionAndOpenDerivationAreTheMappedCapabilities() {
        // The pinned CLR type is public, non-sealed, and declares a public
        // parameterless constructor, so it is externally constructible and
        // externally derivable. The Swift projection is `open` with a public
        // `init()`; the compiler Symbol Graph is the authority for both.
        let parameters = Parameters()
        XCTAssertTrue(parameters.IsFullScreen)

        // Deriving from the projection is possible and the inherited state
        // behaves exactly as on the base type. CLR members are all
        // `hidebysig` without `virtual`, so nothing here is overridden.
        final class Derived: Parameters {}
        let derived = Derived()
        XCTAssertTrue(derived.IsFullScreen)
        derived.BackBufferWidth = 64
        XCTAssertEqual(derived.BackBufferWidth, 64)
        XCTAssertEqual(derived.Bounds.Width, 64)

        // `Clone()` constructs `PresentationParameters` through `newobj` on
        // the base constructor rather than the dynamic type, so a derived
        // instance clones to a base instance. This is pinned IL behaviour
        // preserved through the Swift mapping, not a Swift limitation.
        let clone = derived.Clone()
        XCTAssertEqual(clone.BackBufferWidth, 64)
        XCTAssertFalse(clone is Derived)
        XCTAssertTrue(type(of: clone) == Parameters.self)
    }

    func testClrClassMapsToSwiftReferenceIdentitySemantics() {
        // A CLR class is a reference type: assignment aliases one instance,
        // and two separately constructed instances stay distinct. The pinned
        // type declares no equality identity, so no value comparison is
        // available or implied.
        let first = Parameters()
        let alias = first
        let second = Parameters()

        alias.BackBufferWidth = 512
        XCTAssertEqual(first.BackBufferWidth, 512)
        XCTAssertEqual(second.BackBufferWidth, 0)
        XCTAssertTrue(first === alias)
        XCTAssertFalse(first === second)

        // The clone is a fresh reference, never an alias.
        let clone = first.Clone()
        XCTAssertFalse(first === clone)
        XCTAssertEqual(clone.BackBufferWidth, 512)
    }

    func testIntPtrProjectsToSwiftIntAndNotToAPointerOrFixedWidthInteger() {
        // General BCL rule: `System.IntPtr` maps to Swift `Int` — the opaque
        // pointer-width signed numeric value of the CLR IntPtr. It is not a
        // Swift pointer, not a dereferenceable address, not a CNA native
        // handle, and carries no proof that any window exists.
        let parameters = Parameters()

        // The static type is exactly `Int`. `UnsafeRawPointer`,
        // `UnsafeMutableRawPointer`, `OpaquePointer`, and any CNA handle
        // wrapper are all excluded by this identity, and the API verifier
        // measures the same fact on the compiler Symbol Graph.
        XCTAssertTrue(type(of: parameters.DeviceWindowHandle) == Int.self)

        // `Int` is pointer-width on the qualified host, which is what makes it
        // the faithful IntPtr projection rather than a fixed-width choice.
        XCTAssertEqual(MemoryLayout<Int>.size, MemoryLayout<UnsafeRawPointer>.size)
        XCTAssertEqual(Int.bitWidth, MemoryLayout<UnsafeRawPointer>.size * 8)

        // IntPtr is signed. A `UInt` projection would make this value
        // unrepresentable; `Int` round-trips it.
        parameters.DeviceWindowHandle = -1
        XCTAssertEqual(parameters.DeviceWindowHandle, -1)
        XCTAssertLessThan(parameters.DeviceWindowHandle, 0)

        // A fixed-width `Int64` projection would be wrong for the same reason
        // `Int32` would: the mapped Swift type must follow the host pointer
        // width, not a constant. On this qualified 64-bit host the two happen
        // to agree in size, so the identity check above — not the width — is
        // what distinguishes them.
        XCTAssertFalse(type(of: parameters.DeviceWindowHandle) == Int64.self)
        XCTAssertFalse(type(of: parameters.DeviceWindowHandle) == UInt.self)

        // Storing the value does not make it usable as an address. The
        // projection never converts, dereferences, or resolves it, and no
        // unsafe pointer manipulation is required to hold an XNA IntPtr.
        parameters.DeviceWindowHandle = 0x1_0000
        XCTAssertEqual(parameters.DeviceWindowHandle, 0x1_0000)
        parameters.DeviceWindowHandle = 0
        XCTAssertEqual(parameters.DeviceWindowHandle, 0)
    }

    func testMappedPropertyTypesAreTheDeclaredSwiftProjections() {
        // Each property's static Swift type is the mapped projection of its
        // pinned CLR type. `System.Int32 -> Int32` and `System.Boolean -> Bool`
        // are the established scalar rules; the four enum dependencies and
        // Rectangle are the XNA types completed in Foundation 10-13.
        let parameters = Parameters()
        XCTAssertTrue(type(of: parameters.BackBufferWidth) == Int32.self)
        XCTAssertTrue(type(of: parameters.BackBufferHeight) == Int32.self)
        XCTAssertTrue(type(of: parameters.MultiSampleCount) == Int32.self)
        XCTAssertTrue(type(of: parameters.IsFullScreen) == Bool.self)
        XCTAssertTrue(
            type(of: parameters.BackBufferFormat)
                == Microsoft.Xna.Framework.Graphics.SurfaceFormat.self)
        XCTAssertTrue(
            type(of: parameters.DepthStencilFormat)
                == Microsoft.Xna.Framework.Graphics.DepthFormat.self)
        XCTAssertTrue(
            type(of: parameters.PresentationInterval)
                == Microsoft.Xna.Framework.Graphics.PresentInterval.self)
        XCTAssertTrue(
            type(of: parameters.RenderTargetUsage)
                == Microsoft.Xna.Framework.Graphics.RenderTargetUsage.self)
        XCTAssertTrue(
            type(of: parameters.DisplayOrientation)
                == Microsoft.Xna.Framework.DisplayOrientation.self)
        XCTAssertTrue(
            type(of: parameters.Bounds) == Microsoft.Xna.Framework.Rectangle.self)
        XCTAssertTrue(type(of: parameters.Clone()) == Parameters.self)

        // `Int32` is deliberately not widened to Swift's native `Int` for the
        // dimension properties; only the IntPtr projection uses `Int`.
        XCTAssertFalse(type(of: parameters.BackBufferWidth) == Int.self)
    }

    func testInternalSettingsStorageStaysOutOfThePublicSurface() {
        // The pinned nested `Settings` value struct and the `settings` field
        // are both `assembly`, so both map to `internal` and neither appears
        // in the public Symbol Graph. The transcription is exercised here from
        // inside the module only.
        let parameters = Parameters()
        XCTAssertEqual(parameters.settings.BackBufferWidth, 0)
        XCTAssertEqual(parameters.settings.IsFullScreen, 1)

        // It is a value struct, so reading it yields an independent copy and
        // `Clone`'s single wholesale assignment is a full deep copy of all ten
        // fields.
        var snapshot = parameters.settings
        snapshot.BackBufferWidth = 777
        XCTAssertEqual(parameters.settings.BackBufferWidth, 0)
        XCTAssertEqual(snapshot.BackBufferWidth, 777)

        // The mapped 0/1 IsFullScreen word is the only field whose storage
        // type differs from its public projection.
        //
        // Swift 6.0.3 SILGen crashes on `type(of:)` applied to a stored
        // property of an internal value struct reached through a class
        // property when that expression sits inside an XCTAssert autoclosure
        // (SILGenLValue.cpp `emitAddressOfLValue`: "resolving lvalue did not
        // give an address"). Binding the value to a local first is the
        // documented workaround and does not weaken the identity check.
        let storedIsFullScreen = parameters.settings.IsFullScreen
        let storedHandle = parameters.settings.DeviceWindowHandle
        XCTAssertTrue(type(of: storedIsFullScreen) == Int32.self)
        XCTAssertTrue(type(of: storedHandle) == Int.self)
    }
}
