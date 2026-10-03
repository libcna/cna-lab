// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A manager that records which virtual members were reached.
private final class RecordingManager: F.GraphicsDeviceManager {
    var disposeCalls: [Bool] = []
    var deviceCreatedSenders: [String] = []

    override func Dispose(_ disposing: Bool) throws {
        disposeCalls.append(disposing)
        try super.Dispose(disposing)
    }

    override func OnDeviceCreated(_ sender: Any?, args: CNAEventArgs) throws {
        deviceCreatedSenders.append(sender is String ? "string" : "other")
        try super.OnDeviceCreated(sender, args: args)
    }
}

/// Foundation 52: the manager's four protected raisers, its `Disposed` event,
/// and `Dispose(Bool)`.
final class Foundation52ManagerDisposalTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    /// The raisers forward the **caller's** sender — `ldarg.1`, not `ldarg.0`.
    /// `Game.OnActivated` does the opposite with the same signature, so this
    /// is asserted rather than assumed from the shape.
    func testTheRaisersForwardTheCallersSender() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        var seen: [String] = []
        _ = manager.DeviceCreated.Add { sender, _ in
            seen.append(sender is String ? "the caller's" : "something else")
        }
        _ = manager.DeviceReset.Add { sender, _ in
            seen.append(sender is String ? "the caller's" : "something else")
        }
        try manager.OnDeviceCreated("a sender that is not the manager",
                                    args: CNAEventArgs.Empty)
        try manager.OnDeviceReset("a sender that is not the manager",
                                  args: CNAEventArgs.Empty)
        XCTAssertEqual(seen, ["the caller's", "the caller's"])
    }

    /// Each raiser reaches its own event and no other.
    func testEachRaiserReachesItsOwnEvent() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        var log: [String] = []
        _ = manager.DeviceCreated.Add { _, _ in log.append("created") }
        _ = manager.DeviceDisposing.Add { _, _ in log.append("disposing") }
        _ = manager.DeviceReset.Add { _, _ in log.append("reset") }
        _ = manager.DeviceResetting.Add { _, _ in log.append("resetting") }
        try manager.OnDeviceResetting(manager, args: CNAEventArgs.Empty)
        try manager.OnDeviceReset(manager, args: CNAEventArgs.Empty)
        try manager.OnDeviceDisposing(manager, args: CNAEventArgs.Empty)
        try manager.OnDeviceCreated(manager, args: CNAEventArgs.Empty)
        XCTAssertEqual(log, ["resetting", "reset", "disposing", "created"])
    }

    /// The native device events go **through** the virtual raisers, so a
    /// subclass that overrides one sees them. XNA's own device path calls
    /// `OnDeviceCreated` rather than touching the delegate field, and a
    /// projection that raised the event directly would silently skip every
    /// override.
    func testTheNativeEventsGoThroughTheRaisers() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try RecordingManager(game: game)
        manager.nativeManagerEventFired(F.GraphicsDeviceManager.eventDeviceCreated)
        XCTAssertEqual(
            manager.deviceCreatedSenders.count, 1,
            "the override ran, so the native event reached the virtual raiser")
        XCTAssertEqual(manager.deviceCreatedSenders, ["other"],
                       "and the native dispatch passes the manager itself")
        try manager.Dispose()
    }

    /// `Dispose()` is `Dispose(true)`, which a subclass sees.
    func testDisposeForwardsToTheVirtualOverload() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try RecordingManager(game: game)
        try manager.Dispose()
        XCTAssertEqual(manager.disposeCalls, [true])
    }

    /// `Dispose(false)` returns immediately: no service is removed, nothing is
    /// released, and `Disposed` does not fire.
    func testDisposeFalseDoesNothing() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        var raises = 0
        _ = manager.Disposed.Add { _, _ in raises += 1 }
        try manager.Dispose(false)
        XCTAssertEqual(raises, 0)
        XCTAssertNotNil(
            game.Services.GetService(G.IGraphicsDeviceService.self),
            "the service is still registered")
        try manager.Dispose()
    }

    /// `Dispose(true)` removes `IGraphicsDeviceService` — and **not**
    /// `IGraphicsDeviceManager`, which XNA leaves registered — then raises
    /// `Disposed` last.
    func testDisposeRemovesOneServiceAndRaisesDisposed() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        var serviceWhenRaised: String = "not raised"
        _ = manager.Disposed.Add { _, _ in
            serviceWhenRaised = game.Services.GetService(
                G.IGraphicsDeviceService.self) == nil ? "already removed" : "still there"
        }
        try manager.Dispose()
        XCTAssertNil(game.Services.GetService(G.IGraphicsDeviceService.self))
        XCTAssertNotNil(
            game.Services.GetService(F.IGraphicsDeviceManager.self),
            "XNA removes only the device service, not the manager service")
        XCTAssertEqual(serviceWhenRaised, "already removed",
                       "Disposed is raised after the removal, not before")
    }

    /// A second `Dispose` finds no registered service and takes the other
    /// branch of `GetService(...) == this` — the branch that exists so one
    /// manager cannot unregister another's service.
    func testASecondDisposeTakesTheOtherBranch() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        var raises = 0
        _ = manager.Disposed.Add { _, _ in raises += 1 }
        try manager.Dispose()
        XCTAssertEqual(raises, 1)
        try manager.Dispose()
        XCTAssertEqual(raises, 2, "the raise is unconditional; the removal is not")
        XCTAssertNil(game.Services.GetService(G.IGraphicsDeviceService.self))
    }
}
