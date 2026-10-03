// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A drawable component that records every lifecycle call it receives.
private final class RecordingDrawable: F.DrawableGameComponent {
    var loadContentCalls = 0
    var unloadContentCalls = 0
    var draws = 0
    var visibleChanges = 0
    var drawOrderChanges = 0

    override init(game: F.Game) {
        super.init(game: game)
        _ = VisibleChanged.Add { [weak self] _, _ in self?.visibleChanges += 1 }
        _ = DrawOrderChanged.Add { [weak self] _, _ in self?.drawOrderChanges += 1 }
    }

    override func LoadContent() throws { loadContentCalls += 1 }
    override func UnloadContent() throws { unloadContentCalls += 1 }
    override func Draw(_ gameTime: F.GameTime) throws { draws += 1 }
}

/// A game whose body runs once inside `LoadContent`, where a device exists.
private final class ServiceProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((ServiceProbeGame) throws -> Void)?

    init(withManager: Bool, _ body: @escaping (ServiceProbeGame) throws -> Void) throws {
        try super.init()
        self.body = body
        if withManager { manager = try F.GraphicsDeviceManager(game: self) }
    }

    override func LoadContent() throws {
        do { try body?(self) } catch { failure = error }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

final class Foundation40GraphicsDeviceServiceTests: XCTestCase {
    private var nativeConfigured: Bool {
        ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil
    }

    private func requireNative() throws {
        if !nativeConfigured {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    @discardableResult
    private func run(
        withManager: Bool = true,
        _ body: @escaping (ServiceProbeGame) throws -> Void
    ) throws -> ServiceProbeGame {
        let game = try ServiceProbeGame(withManager: withManager, body)
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
    // The producer.
    // ------------------------------------------------------------------

    /// `GraphicsDeviceManager..ctor` registers itself under **both** service
    /// types, which is what makes `Game.GraphicsDevice` and
    /// `DrawableGameComponent` work at all.
    func testTheManagerRegistersItselfUnderBothServiceTypes() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        XCTAssertNil(game.Services.GetService(G.IGraphicsDeviceService.self))
        XCTAssertNil(game.Services.GetService(F.IGraphicsDeviceManager.self))

        let manager = try F.GraphicsDeviceManager(game: game)
        XCTAssertTrue(
            game.Services.GetService(G.IGraphicsDeviceService.self) as AnyObject? === manager)
        XCTAssertTrue(
            game.Services.GetService(F.IGraphicsDeviceManager.self) as AnyObject? === manager)
        try manager.Dispose()
    }

    /// A second manager is refused with XNA's own message, and refused
    /// **before** any native object is created.
    func testASecondManagerIsRefusedWithTheAssemblysOwnMessage() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let first = try F.GraphicsDeviceManager(game: game)
        assertProjected(
            CNAArgumentException.self,
            message: "A graphics device manager is already registered.  The "
                + "graphics device manager cannot be changed once it is set.",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            _ = try F.GraphicsDeviceManager(game: game)
        }
        // The registration still names the first manager.
        XCTAssertTrue(
            game.Services.GetService(F.IGraphicsDeviceManager.self) as AnyObject? === first)
        try first.Dispose()
    }

    /// The four device events are real CNA subscriptions, released on
    /// disposal.
    func testTheManagerSubscribesToTheFourDeviceEventsAndReleasesThem() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        XCTAssertEqual(
            manager.managerEventRegistrationCountForTests, 4,
            "one registration per IGraphicsDeviceService event")
        try manager.Dispose()
        XCTAssertEqual(manager.managerEventRegistrationCountForTests, 0)
    }

    // ------------------------------------------------------------------
    // Game.GraphicsDevice through the container.
    // ------------------------------------------------------------------

    /// With no manager there is no service, and XNA's own message says so.
    func testGameGraphicsDeviceRaisesWithoutAService() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        assertProjected(
            CNAInvalidOperationException.self,
            message: "This property requires a graphics device service in the "
                + "game service container.",
            hResult: Int32(bitPattern: 0x8013_1509)
        ) {
            _ = try game.GraphicsDevice
        }
    }

    /// With a manager the device comes from the service, and it is the
    /// service's own device.
    func testGameGraphicsDeviceComesFromTheRegisteredService() throws {
        try requireNative()
        let game = try run { game in
            let fromGame = try game.GraphicsDevice
            let fromService = game.manager?.GraphicsDevice
            game.observations["gameHasDevice"] = String(fromGame != nil)
            game.observations["serviceHasDevice"] = String(fromService != nil)
        }
        XCTAssertEqual(game.observations["gameHasDevice"], "true")
        XCTAssertEqual(game.observations["serviceHasDevice"], "true")
    }

    /// Outside a lifecycle callback the service has no device to hand out,
    /// which is the state XNA's field is in before the loop creates one — and
    /// the getter reports it as nil rather than by throwing.
    func testTheServiceReportsNoDeviceOutsideACallback() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        XCTAssertNil(
            manager.GraphicsDevice,
            "a callback-scoped device cannot be handed out from outside one")
        XCTAssertNil(try game.GraphicsDevice, "and Game reports the same nil")
        try manager.Dispose()
        XCTAssertNil(manager.GraphicsDevice, "nor after disposal")
    }

    // ------------------------------------------------------------------
    // DrawableGameComponent.
    // ------------------------------------------------------------------

    /// With no service, `Initialize` raises XNA's own — and **different** —
    /// message.
    func testADrawableWithoutAServiceRaisesItsOwnMessage() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let component = RecordingDrawable(game: game)
        assertProjected(
            CNAInvalidOperationException.self,
            message: "Drawable components require a graphics device service in "
                + "the game service container.",
            hResult: Int32(bitPattern: 0x8013_1509)
        ) {
            try component.Initialize()
        }
        XCTAssertEqual(component.loadContentCalls, 0)
        XCTAssertNotEqual(
            F.DrawableGameComponent.missingGraphicsDeviceServiceMessage,
            F.Game.noGraphicsDeviceServiceMessage,
            "the two resources are different strings and are not interchangeable")
    }

    /// With a service **and** a device, `Initialize` loads content once.
    func testInitializeLoadsContentWhenTheServiceHasADevice() throws {
        try requireNative()
        let game = try run { game in
            let component = RecordingDrawable(game: game)
            try component.Initialize()
            game.observations["loadsAfterInitialize"] = String(component.loadContentCalls)
            // A second Initialize resolves nothing again and loads nothing
            // again: the guard is on the resolution, not on the reload.
            try component.Initialize()
            game.observations["loadsAfterSecond"] = String(component.loadContentCalls)
            game.observations["deviceThroughComponent"] =
                String((try component.GraphicsDevice) != nil)
            try component.Dispose()
            game.observations["unloadsAfterDispose"] = String(component.unloadContentCalls)
        }
        XCTAssertEqual(game.observations["loadsAfterInitialize"], "1")
        XCTAssertEqual(game.observations["loadsAfterSecond"], "1")
        XCTAssertEqual(game.observations["deviceThroughComponent"], "true")
        XCTAssertEqual(game.observations["unloadsAfterDispose"], "1")
    }

    /// The `DeviceCreated` subscription stays live after `Initialize`, which
    /// is what reloads content on a device reset. Guarding the reload with the
    /// one-time flag would fix startup and break every subsequent reset.
    func testTheDeviceCreatedSubscriptionSurvivesInitialize() throws {
        try requireNative()
        let game = try run { game in
            let component = RecordingDrawable(game: game)
            try component.Initialize()
            let before = component.loadContentCalls
            // Raising the service's own event is what a device reset does.
            game.manager?.nativeManagerEventFired(
                F.GraphicsDeviceManager.eventDeviceCreated)
            game.observations["reloaded"] = String(component.loadContentCalls - before)
            game.manager?.nativeManagerEventFired(
                F.GraphicsDeviceManager.eventDeviceDisposing)
            game.observations["unloaded"] = String(component.unloadContentCalls)
            try component.Dispose()
        }
        XCTAssertEqual(
            game.observations["reloaded"], "1",
            "DeviceCreated must still reload content after Initialize has run")
        XCTAssertEqual(
            game.observations["unloaded"], "1",
            "DeviceDisposing unloads it")
    }

    /// `Visible` and `DrawOrder` compare before they store: writing the same
    /// value raises nothing.
    func testVisibleAndDrawOrderRaiseOnlyOnAnActualChange() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let component = RecordingDrawable(game: game)
        XCTAssertTrue(component.Visible)
        XCTAssertEqual(component.DrawOrder, 0)

        component.Visible = true
        XCTAssertEqual(component.visibleChanges, 0, "same value, no event")
        component.Visible = false
        XCTAssertEqual(component.visibleChanges, 1)
        component.Visible = false
        XCTAssertEqual(component.visibleChanges, 1)

        component.DrawOrder = 0
        XCTAssertEqual(component.drawOrderChanges, 0)
        component.DrawOrder = 7
        XCTAssertEqual(component.drawOrderChanges, 1)
        XCTAssertEqual(component.DrawOrder, 7)
    }

    /// A drawable is an `IDrawable` and a `GameComponent`, so `Game.Components`
    /// takes it and the engine drives it.
    func testADrawableIsBothADrawableAndAComponent() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let component = RecordingDrawable(game: game)
        XCTAssertTrue((component as Any) is F.IDrawable)
        XCTAssertTrue((component as Any) is F.IGameComponent)
        XCTAssertTrue((component as Any) is F.IUpdateable)
        XCTAssertTrue((component as Any) is F.GameComponent)
        try game.Components.Add(component)
        XCTAssertEqual(game.Components.Count, 1)
    }
}
