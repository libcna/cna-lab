// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for
// Microsoft.Xna.Framework.Graphics.PresentationParameters, transcribed from
// the pinned, hash-matched Microsoft.Xna.Framework.Graphics.dll IL. Every
// expectation below is a property of that managed code alone; nothing here
// touches a device, adapter, window, or renderer.
extension PureValueTests {
    func testPresentationParametersXnaContractDefaults() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // `.ctor()` is `call System.Object::.ctor(); ldc.i4.1;
        // call set_IsFullScreen(bool)`. It touches exactly one property, so
        // every other field keeps its CLR default and each mapped enum starts
        // at its own pinned zero literal.
        let parameters = Parameters()
        XCTAssertTrue(parameters.IsFullScreen)
        XCTAssertEqual(parameters.BackBufferWidth, 0)
        XCTAssertEqual(parameters.BackBufferHeight, 0)
        XCTAssertEqual(parameters.MultiSampleCount, 0)
        XCTAssertEqual(parameters.BackBufferFormat, .Color)
        XCTAssertEqual(parameters.DepthStencilFormat, .None)
        XCTAssertEqual(parameters.PresentationInterval, .Default)
        XCTAssertEqual(parameters.RenderTargetUsage, .DiscardContents)
        XCTAssertEqual(parameters.DisplayOrientation, .Default)

        // Each zero literal is checked against its own pinned raw value so a
        // renamed or renumbered case cannot silently satisfy the default.
        XCTAssertEqual(parameters.BackBufferFormat.rawValue, 0)
        XCTAssertEqual(parameters.DepthStencilFormat.rawValue, 0)
        XCTAssertEqual(parameters.PresentationInterval.rawValue, 0)
        XCTAssertEqual(parameters.RenderTargetUsage.rawValue, 0)
        XCTAssertEqual(parameters.DisplayOrientation.rawValue, 0)

        // `native int DeviceWindowHandle` starts at IntPtr.Zero. This is
        // descriptor state; the value is never dereferenced or resolved.
        XCTAssertEqual(parameters.DeviceWindowHandle, 0)

        // `get_Bounds` reads the two back-buffer fields directly, so a default
        // instance bounds to the empty rectangle at the origin.
        XCTAssertEqual(parameters.Bounds.X, 0)
        XCTAssertEqual(parameters.Bounds.Y, 0)
        XCTAssertEqual(parameters.Bounds.Width, 0)
        XCTAssertEqual(parameters.Bounds.Height, 0)

        // Two separate constructions observe the same defaults; the pinned
        // constructor holds no shared or static state.
        let second = Parameters()
        XCTAssertTrue(second.IsFullScreen)
        XCTAssertEqual(second.BackBufferWidth, 0)
        XCTAssertEqual(second.DeviceWindowHandle, 0)
    }

    func testPresentationParametersXnaContractMutation() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // Every setter is `ldflda settings; ldarg.1; stfld <field>`. Nothing is
        // validated, clamped, rounded, or rejected, so out-of-range and
        // negative values round-trip verbatim.
        let parameters = Parameters()
        parameters.BackBufferWidth = 1280
        parameters.BackBufferHeight = 720
        parameters.BackBufferFormat = .Bgra4444
        parameters.DepthStencilFormat = .Depth24Stencil8
        parameters.MultiSampleCount = 4
        parameters.DisplayOrientation = [.LandscapeLeft, .Portrait]
        parameters.PresentationInterval = .Two
        parameters.RenderTargetUsage = .PreserveContents
        parameters.IsFullScreen = false

        XCTAssertEqual(parameters.BackBufferWidth, 1280)
        XCTAssertEqual(parameters.BackBufferHeight, 720)
        XCTAssertEqual(parameters.BackBufferFormat, .Bgra4444)
        XCTAssertEqual(parameters.DepthStencilFormat, .Depth24Stencil8)
        XCTAssertEqual(parameters.MultiSampleCount, 4)
        XCTAssertEqual(parameters.DisplayOrientation, [.LandscapeLeft, .Portrait])
        XCTAssertEqual(parameters.PresentationInterval, .Two)
        XCTAssertEqual(parameters.RenderTargetUsage, .PreserveContents)
        XCTAssertFalse(parameters.IsFullScreen)

        // Negative and extreme int32 dimensions are stored, not rejected: the
        // pinned accessors contain no comparison instruction at all.
        parameters.BackBufferWidth = -1
        parameters.BackBufferHeight = Int32.min
        parameters.MultiSampleCount = Int32.max
        XCTAssertEqual(parameters.BackBufferWidth, -1)
        XCTAssertEqual(parameters.BackBufferHeight, Int32.min)
        XCTAssertEqual(parameters.MultiSampleCount, Int32.max)

        // An unnamed DisplayOrientation bit combination is preserved verbatim;
        // the setter stores the raw flags word.
        parameters.DisplayOrientation = Microsoft.Xna.Framework.DisplayOrientation(rawValue: 0x40)
        XCTAssertEqual(parameters.DisplayOrientation.rawValue, 0x40)

        // Setting a property back to its zero literal restores the default
        // observation, proving the accessors are pure field stores.
        parameters.BackBufferWidth = 0
        parameters.BackBufferHeight = 0
        parameters.MultiSampleCount = 0
        parameters.BackBufferFormat = .Color
        parameters.DepthStencilFormat = .None
        parameters.PresentationInterval = .Default
        parameters.RenderTargetUsage = .DiscardContents
        parameters.DisplayOrientation = .Default
        XCTAssertEqual(parameters.BackBufferWidth, 0)
        XCTAssertEqual(parameters.BackBufferFormat, .Color)
        XCTAssertEqual(parameters.DisplayOrientation, .Default)
    }

    func testPresentationParametersXnaContractIsFullScreenNormalization() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // `IsFullScreen` is backed by `int32`, not `bool`.
        // `set_IsFullScreen` stores exactly `ldc.i4.1` or `ldc.i4.0`, and
        // `get_IsFullScreen` normalises through `ldc.i4.0; ceq; ldc.i4.0; ceq`.
        // The public projection is therefore a clean two-state boolean.
        let parameters = Parameters()
        XCTAssertTrue(parameters.IsFullScreen)
        parameters.IsFullScreen = false
        XCTAssertFalse(parameters.IsFullScreen)
        parameters.IsFullScreen = true
        XCTAssertTrue(parameters.IsFullScreen)

        // Repeated identical stores are idempotent and the round trip is
        // stable across many toggles.
        for _ in 0..<4 {
            parameters.IsFullScreen = true
            XCTAssertTrue(parameters.IsFullScreen)
            parameters.IsFullScreen = false
            XCTAssertFalse(parameters.IsFullScreen)
        }

        // The pinned 0/1 storage word is what the getter observes, so any
        // non-zero word would read back as true. The internal transcription is
        // exercised directly to prove the normalisation is real and not an
        // artefact of the Bool projection.
        parameters.settings.IsFullScreen = 42
        XCTAssertTrue(parameters.IsFullScreen)
        parameters.settings.IsFullScreen = -7
        XCTAssertTrue(parameters.IsFullScreen)
        parameters.settings.IsFullScreen = 0
        XCTAssertFalse(parameters.IsFullScreen)

        // A store through the public setter always normalises back to 1 or 0.
        parameters.IsFullScreen = true
        XCTAssertEqual(parameters.settings.IsFullScreen, 1)
        parameters.IsFullScreen = false
        XCTAssertEqual(parameters.settings.IsFullScreen, 0)
    }

    func testPresentationParametersXnaContractBounds() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // `get_Bounds` is exactly
        // `new Rectangle(0, 0, settings.BackBufferWidth,
        //  settings.BackBufferHeight)`.
        // The origin is always (0, 0) and the size always tracks the two
        // back-buffer fields live; there is no cache and no device query.
        let parameters = Parameters()
        parameters.BackBufferWidth = 800
        parameters.BackBufferHeight = 480
        XCTAssertEqual(parameters.Bounds.X, 0)
        XCTAssertEqual(parameters.Bounds.Y, 0)
        XCTAssertEqual(parameters.Bounds.Width, 800)
        XCTAssertEqual(parameters.Bounds.Height, 480)

        parameters.BackBufferWidth = 1920
        XCTAssertEqual(parameters.Bounds.Width, 1920)
        XCTAssertEqual(parameters.Bounds.Height, 480)
        parameters.BackBufferHeight = 1080
        XCTAssertEqual(parameters.Bounds.Height, 1080)

        // Negative dimensions propagate into the rectangle unchanged; the
        // pinned getter performs no clamping.
        parameters.BackBufferWidth = -3
        parameters.BackBufferHeight = -9
        XCTAssertEqual(parameters.Bounds.X, 0)
        XCTAssertEqual(parameters.Bounds.Y, 0)
        XCTAssertEqual(parameters.Bounds.Width, -3)
        XCTAssertEqual(parameters.Bounds.Height, -9)

        // `IsFullScreen`, the orientation, and the window handle are not read
        // by `get_Bounds` and cannot influence it.
        parameters.BackBufferWidth = 640
        parameters.BackBufferHeight = 360
        parameters.IsFullScreen = true
        parameters.DisplayOrientation = .LandscapeRight
        parameters.DeviceWindowHandle = 0x1234
        XCTAssertEqual(parameters.Bounds.Width, 640)
        XCTAssertEqual(parameters.Bounds.Height, 360)
        XCTAssertEqual(parameters.Bounds.X, 0)
        XCTAssertEqual(parameters.Bounds.Y, 0)
    }

    func testPresentationParametersXnaContractDeviceWindowHandle() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // `DeviceWindowHandle` is a `native int` field with a plain load/store
        // accessor pair. It is pure descriptor state: the pinned managed code
        // never dereferences it, never compares it against a real window, and
        // never rejects a value.
        let parameters = Parameters()
        XCTAssertEqual(parameters.DeviceWindowHandle, 0)

        parameters.DeviceWindowHandle = 0x0001_0F42
        XCTAssertEqual(parameters.DeviceWindowHandle, 0x0001_0F42)

        // IntPtr is signed, so a negative value is representable and must
        // round-trip verbatim rather than wrapping through an unsigned domain.
        parameters.DeviceWindowHandle = -1
        XCTAssertEqual(parameters.DeviceWindowHandle, -1)

        // The full pointer-width signed range round-trips.
        parameters.DeviceWindowHandle = Int.min
        XCTAssertEqual(parameters.DeviceWindowHandle, Int.min)
        parameters.DeviceWindowHandle = Int.max
        XCTAssertEqual(parameters.DeviceWindowHandle, Int.max)

        // Zero is IntPtr.Zero and carries no special behaviour beyond being
        // the constructor default.
        parameters.DeviceWindowHandle = 0
        XCTAssertEqual(parameters.DeviceWindowHandle, 0)

        // The handle is independent of every other property.
        parameters.DeviceWindowHandle = 99
        parameters.BackBufferWidth = 320
        parameters.IsFullScreen = false
        XCTAssertEqual(parameters.DeviceWindowHandle, 99)
        XCTAssertEqual(parameters.BackBufferWidth, 320)
        XCTAssertFalse(parameters.IsFullScreen)
    }

    func testPresentationParametersXnaContractClone() {
        typealias Parameters = Microsoft.Xna.Framework.Graphics.PresentationParameters

        // `Clone()` is `newobj .ctor(); ldarg.0; ldfld settings;
        //  stfld settings; ret`. The wholesale value-struct copy overwrites the
        // constructor's `IsFullScreen = true`, so the result is an exact
        // field-by-field copy of the source — including a false IsFullScreen.
        let source = Parameters()
        source.BackBufferWidth = 1024
        source.BackBufferHeight = 768
        source.BackBufferFormat = .Rgba64
        source.DepthStencilFormat = .Depth16
        source.MultiSampleCount = 8
        source.DisplayOrientation = .Portrait
        source.PresentationInterval = .Immediate
        source.RenderTargetUsage = .PlatformContents
        source.DeviceWindowHandle = 0x7FED
        source.IsFullScreen = false

        let clone = source.Clone()
        XCTAssertEqual(clone.BackBufferWidth, 1024)
        XCTAssertEqual(clone.BackBufferHeight, 768)
        XCTAssertEqual(clone.BackBufferFormat, .Rgba64)
        XCTAssertEqual(clone.DepthStencilFormat, .Depth16)
        XCTAssertEqual(clone.MultiSampleCount, 8)
        XCTAssertEqual(clone.DisplayOrientation, .Portrait)
        XCTAssertEqual(clone.PresentationInterval, .Immediate)
        XCTAssertEqual(clone.RenderTargetUsage, .PlatformContents)
        XCTAssertEqual(clone.DeviceWindowHandle, 0x7FED)
        XCTAssertFalse(clone.IsFullScreen)

        // The clone is a distinct instance: mutating either side leaves the
        // other untouched, because the copied `settings` is a value struct.
        clone.BackBufferWidth = 1
        clone.DeviceWindowHandle = 2
        clone.IsFullScreen = true
        XCTAssertEqual(source.BackBufferWidth, 1024)
        XCTAssertEqual(source.DeviceWindowHandle, 0x7FED)
        XCTAssertFalse(source.IsFullScreen)
        source.BackBufferHeight = 3
        XCTAssertEqual(clone.BackBufferHeight, 768)
        // Restore the source height so the checks below read against the
        // originally configured state rather than this independence probe.
        source.BackBufferHeight = 768

        // Cloning a default instance reproduces the constructor defaults, and
        // the true IsFullScreen survives the wholesale copy in that direction
        // too.
        let defaultClone = Parameters().Clone()
        XCTAssertTrue(defaultClone.IsFullScreen)
        XCTAssertEqual(defaultClone.BackBufferWidth, 0)
        XCTAssertEqual(defaultClone.BackBufferFormat, .Color)
        XCTAssertEqual(defaultClone.DeviceWindowHandle, 0)

        // Clone is idempotent across repeated application.
        let twice = source.Clone().Clone()
        XCTAssertEqual(twice.BackBufferWidth, 1024)
        XCTAssertEqual(twice.DeviceWindowHandle, 0x7FED)
        XCTAssertFalse(twice.IsFullScreen)

        // The clone's derived Bounds follows its own copied dimensions.
        XCTAssertEqual(twice.Bounds.Width, 1024)
        XCTAssertEqual(twice.Bounds.Height, 768)
    }
}
