// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the Foundation 17 batch. These
// are Swift/CLR mapping facts, not XNA runtime observations, so they are
// deliberately kept out of the pure XNA-derived behaviour corpus.
final class Foundation17ProjectionTests: XCTestCase {
    typealias Gesture = Microsoft.Xna.Framework.Input.Touch.GestureType
    typealias TouchState = Microsoft.Xna.Framework.Input.Touch.TouchLocationState
    typealias StopOptions = Microsoft.Xna.Framework.Audio.AudioStopOptions
    typealias Soundtrack = Microsoft.Xna.Framework.Media.VideoSoundtrackType

    func testFlagsEnumProjectsAsOptionSetAndOthersAsInt32RawEnums() {
        // GestureType is the only Foundation 17 enum carrying
        // System.FlagsAttribute in its pinned binary, so it is the only one
        // that maps to an OptionSet struct rather than a Swift enum.
        let combined: Gesture = [.Tap, .Flick]
        XCTAssertEqual(combined.rawValue, 1 | 128)
        let rawValue: Int32 = combined.rawValue
        XCTAssertTrue(type(of: rawValue) == Int32.self)

        // An OptionSet accepts any raw word, including one with no declared
        // literal. This is exactly the behaviour a non-flags enum must not
        // have.
        XCTAssertEqual(Gesture(rawValue: 0x7FFF_FFFF).rawValue, 0x7FFF_FFFF)

        // The three non-flags enums reject every undeclared raw value.
        XCTAssertNil(TouchState(rawValue: 4))
        XCTAssertNil(StopOptions(rawValue: 2))
        XCTAssertNil(Soundtrack(rawValue: 3))

        // Each carries the reviewed Int32 raw type.
        let touchRaw: Int32 = TouchState.Moved.rawValue
        let stopRaw: Int32 = StopOptions.Immediate.rawValue
        let soundtrackRaw: Int32 = Soundtrack.MusicAndDialog.rawValue
        XCTAssertEqual(touchRaw, 3)
        XCTAssertEqual(stopRaw, 1)
        XCTAssertEqual(soundtrackRaw, 2)

        // Enum and OptionSet alike are value types.
        var copy = TouchState.Pressed
        let original = copy
        copy = .Released
        XCTAssertEqual(original, .Pressed)
        XCTAssertEqual(copy, .Released)
    }

    func testTouchPanelCapabilitiesExposesNoPublicConstructionRoute() {
        typealias Capabilities =
            Microsoft.Xna.Framework.Input.Touch.TouchPanelCapabilities

        // The pinned struct declares no constructor at all: its two auto
        // properties have private setters and its only producer is an
        // `assembly` static. The Swift projection therefore keeps its
        // initializer internal, and both stored properties are private, so
        // Swift's implicit memberwise initializer is not public either. The
        // compiler Symbol Graph is the authority and contains no public init.
        let capabilities = Capabilities(isConnected: true, maximumTouchCount: 2)
        XCTAssertTrue(capabilities.IsConnected)
        XCTAssertEqual(capabilities.MaximumTouchCount, 2)

        let isConnected = capabilities.IsConnected
        let maximumTouchCount = capabilities.MaximumTouchCount
        XCTAssertTrue(type(of: isConnected) == Bool.self)
        XCTAssertTrue(type(of: maximumTouchCount) == Int32.self)

        // The pinned type declares no equality identity, so none is implied.
        XCTAssertFalse(capabilities is any Equatable)
    }

    func testInterfacesProjectAsProtocolsWithNoSuppliedConformer() {
        // A CLR interface maps to a Swift protocol with one requirement per
        // declared member and no invented conformance. Both protocols below
        // are declared with no XNA conformer, exactly as IEffectFog and
        // IEffectMatrices were in Foundation 14.
        //
        // Conforming here proves the requirement set is usable and complete;
        // these are test types, not XNA surface.
        final class Component: Microsoft.Xna.Framework.IGameComponent {
            var initialized = false
            func Initialize() throws { initialized = true }
        }
        let component = Component()
        XCTAssertNoThrow(try component.Initialize())
        XCTAssertTrue(component.initialized)

        final class Manager: Microsoft.Xna.Framework.IGraphicsDeviceManager {
            var created = false
            var ended = false
            func CreateDevice() throws { created = true }
            func BeginDraw() throws -> Bool { true }
            func EndDraw() throws { ended = true }
        }
        let manager = Manager()
        XCTAssertNoThrow(try manager.CreateDevice())
        XCTAssertTrue(manager.created)
        XCTAssertEqual(try manager.BeginDraw(), true)
        XCTAssertNoThrow(try manager.EndDraw())
        XCTAssertTrue(manager.ended)

        // `throws` is the established language projection for an XNA
        // runtime/failure path and adds no XNA member. A conformer may throw,
        // and the error propagates unchanged.
        struct Failure: Error {}
        final class Failing: Microsoft.Xna.Framework.IGraphicsDeviceManager {
            func CreateDevice() throws { throw Failure() }
            func BeginDraw() throws -> Bool { throw Failure() }
            func EndDraw() throws { throw Failure() }
        }
        let failing = Failing()
        XCTAssertThrowsError(try failing.CreateDevice()) {
            XCTAssertTrue($0 is Failure)
        }
        XCTAssertThrowsError(try failing.BeginDraw())
        XCTAssertThrowsError(try failing.EndDraw())

        // Foundation 40 supplied the conformer XNA has:
        // `GraphicsDeviceManager..ctor` registers itself into `Game.Services`
        // under this very interface, so a projection where nothing conformed
        // could not have answered `Game.GraphicsDevice`.
        XCTAssertTrue(
            (Microsoft.Xna.Framework.GraphicsDeviceManager.self as Any)
                is Microsoft.Xna.Framework.IGraphicsDeviceManager.Type)
    }
}
