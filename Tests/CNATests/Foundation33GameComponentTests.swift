// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the managed component engine, transcribed
// from the CIL of the hash-registered `Microsoft.Xna.Framework.Game.dll`
// (SHA-256 b5dffdd8…a1f0): `Game..ctor`, `Game::GameComponentAdded`,
// `Game::GameComponentRemoved`, `Game::UpdateableUpdateOrderChanged`,
// `Game::DrawableDrawOrderChanged`, `Game::Initialize`, `Game::Update`,
// `Game::Draw` and the two order comparers.
//
// These construct a real `Game`, which loads the native library, so the file
// is gated on `CNA_NATIVE_LIBRARY` and is deliberately NOT counted in the pure
// managed behaviour corpus -- the same convention `NativeLifecycleTests`
// follows. The behaviour under test is entirely managed; only the constructor
// needs the host.

/// A component that is updateable, drawable and orderable, and that raises the
/// two order-changed events XNA's engine listens for.
private final class ProbeComponent:
    Microsoft.Xna.Framework.IGameComponent,
    Microsoft.Xna.Framework.IUpdateable,
    Microsoft.Xna.Framework.IDrawable
{
    let name: String
    private(set) var initializeCount = 0
    private(set) var updateCount = 0
    private(set) var drawCount = 0

    /// A shared log so a test can see the ORDER in which components ran, which
    /// is the whole point of the sorted lists.
    let log: Log
    final class Log { var entries: [String] = [] }

    var Enabled = true
    var Visible = true

    private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
    private let updateOrderChangedSource = CNAEventSource<CNAEventArgs>()
    private let visibleChangedSource = CNAEventSource<CNAEventArgs>()
    private let drawOrderChangedSource = CNAEventSource<CNAEventArgs>()

    private var updateOrder: Int32
    private var drawOrder: Int32

    init(_ name: String, log: Log, updateOrder: Int32 = 0, drawOrder: Int32 = 0) {
        self.name = name
        self.log = log
        self.updateOrder = updateOrder
        self.drawOrder = drawOrder
    }

    var UpdateOrder: Int32 { updateOrder }
    var DrawOrder: Int32 { drawOrder }
    var EnabledChanged: CNAEvent<CNAEventArgs> { enabledChangedSource.Event }
    var UpdateOrderChanged: CNAEvent<CNAEventArgs> { updateOrderChangedSource.Event }
    var VisibleChanged: CNAEvent<CNAEventArgs> { visibleChangedSource.Event }
    var DrawOrderChanged: CNAEvent<CNAEventArgs> { drawOrderChangedSource.Event }

    /// A conformer decides how the order changes and raises the event, which
    /// is exactly what the pinned interface says: both properties are get-only
    /// and the event is the conformer's responsibility.
    func setUpdateOrder(_ value: Int32) throws {
        updateOrder = value
        try updateOrderChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }

    func setDrawOrder(_ value: Int32) throws {
        drawOrder = value
        try drawOrderChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }

    func Initialize() throws {
        initializeCount += 1
        log.entries.append("init:\(name)")
    }

    func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        updateCount += 1
        log.entries.append("update:\(name)")
    }

    func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        drawCount += 1
        log.entries.append("draw:\(name)")
    }
}

/// A component that is only an `IGameComponent`: it must be initialized and
/// must never be updated or drawn.
private final class BareComponent: Microsoft.Xna.Framework.IGameComponent {
    private(set) var initializeCount = 0
    func Initialize() throws { initializeCount += 1 }
}

final class GameComponentEngineTests: XCTestCase {
    private var nativeConfigured: Bool {
        guard let value = ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"]
        else { return false }
        return value.hasPrefix("/") && FileManager.default.fileExists(atPath: value)
    }

    private func makeGame() throws -> Microsoft.Xna.Framework.Game {
        if !nativeConfigured {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
        return try Microsoft.Xna.Framework.Game()
    }

    private func time() -> Microsoft.Xna.Framework.GameTime {
        Microsoft.Xna.Framework.GameTime()
    }

    // ------------------------------------------------------------------
    // The two stable objects the constructor allocates.
    // ------------------------------------------------------------------

    // `get_Components` and `get_LaunchParameters` are bare field reads of the
    // single instance `Game..ctor` allocates, so each read is the SAME object.
    // Returning a fresh one would silently discard every component a consumer
    // had added.
    func testComponentsAndLaunchParametersAreStableObjects() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        XCTAssertTrue(game.Components === game.Components)
        XCTAssertTrue(game.LaunchParameters === game.LaunchParameters)
        XCTAssertEqual(game.Components.Count, 0)
    }

    // ------------------------------------------------------------------
    // notYetInitialized, and the inRun branch.
    // ------------------------------------------------------------------

    // `GameComponentAdded`: `inRun ? component.Initialize() : notYetInitialized.Add(c)`.
    // Before the run, adding queues; the base `Initialize` drains the queue.
    func testComponentsAddedBeforeTheRunAreInitializedByInitialize() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let first = ProbeComponent("a", log: log)
        let second = ProbeComponent("b", log: log)

        try game.Components.Add(first)
        try game.Components.Add(second)
        XCTAssertEqual(first.initializeCount, 0, "adding must not initialize yet")
        XCTAssertEqual(second.initializeCount, 0)

        try game.Initialize()
        XCTAssertEqual(first.initializeCount, 1)
        XCTAssertEqual(second.initializeCount, 1)
        XCTAssertEqual(log.entries, ["init:a", "init:b"])

        // The queue is drained, so a second Initialize initializes nothing.
        try game.Initialize()
        XCTAssertEqual(first.initializeCount, 1)
    }

    // Once `BeginRun` has set `inRun`, a newly added component is initialized
    // immediately instead of being queued.
    func testComponentsAddedDuringTheRunAreInitializedImmediately() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        try game.BeginRun()

        let component = ProbeComponent("a", log: log)
        try game.Components.Add(component)
        XCTAssertEqual(component.initializeCount, 1)

        // ... and it is not queued, so Initialize does not run it again.
        try game.Initialize()
        XCTAssertEqual(component.initializeCount, 1)

        try game.EndRun()
        let afterRun = ProbeComponent("b", log: log)
        try game.Components.Add(afterRun)
        XCTAssertEqual(afterRun.initializeCount, 0, "EndRun clears inRun")
    }

    // A component that is only an IGameComponent is initialized and never
    // updated or drawn: `GameComponentAdded` type-tests separately for each
    // interface.
    func testABareComponentIsInitializedButNeverUpdatedOrDrawn() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let bare = BareComponent()
        try game.Components.Add(bare)
        try game.Initialize()
        XCTAssertEqual(bare.initializeCount, 1)
        try game.Update(time())
        try game.Draw(time())
        XCTAssertEqual(bare.initializeCount, 1)
    }

    // ------------------------------------------------------------------
    // Ordering.
    // ------------------------------------------------------------------

    // The sorted insertion runs the lowest UpdateOrder first regardless of the
    // order components were added in.
    func testUpdateOrderDecidesTheUpdateSequence() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        try game.Components.Add(ProbeComponent("third", log: log, updateOrder: 30))
        try game.Components.Add(ProbeComponent("first", log: log, updateOrder: 10))
        try game.Components.Add(ProbeComponent("second", log: log, updateOrder: 20))

        try game.Update(time())
        XCTAssertEqual(log.entries,
                       ["update:first", "update:second", "update:third"])
    }

    // `UpdateOrderComparer.Compare` returns 0 only when `x.Equals(y)`, never
    // for two distinct components with the same order, and the insertion then
    // scans past every equal-ordered element. So equal orders keep INSERTION
    // order -- a comparer that merely compared the number would not have.
    func testEqualUpdateOrdersKeepInsertionOrder() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        for name in ["a", "b", "c", "d"] {
            try game.Components.Add(ProbeComponent(name, log: log, updateOrder: 5))
        }
        try game.Update(time())
        XCTAssertEqual(log.entries,
                       ["update:a", "update:b", "update:c", "update:d"])
    }

    func testDrawOrderDecidesTheDrawSequence() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        try game.Components.Add(ProbeComponent("back", log: log, drawOrder: 1))
        try game.Components.Add(ProbeComponent("front", log: log, drawOrder: 9))
        try game.Components.Add(ProbeComponent("middle", log: log, drawOrder: 5))

        try game.Draw(time())
        XCTAssertEqual(log.entries,
                       ["draw:back", "draw:middle", "draw:front"])
    }

    // `UpdateableUpdateOrderChanged` removes and re-inserts the component at
    // the position its NEW order gives. Without the subscription the list
    // would keep the stale position and the sequence would be wrong.
    func testChangingUpdateOrderReordersTheSequence() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let low = ProbeComponent("low", log: log, updateOrder: 1)
        let high = ProbeComponent("high", log: log, updateOrder: 9)
        try game.Components.Add(low)
        try game.Components.Add(high)

        try game.Update(time())
        XCTAssertEqual(log.entries, ["update:low", "update:high"])

        log.entries = []
        try low.setUpdateOrder(20)
        try game.Update(time())
        XCTAssertEqual(log.entries, ["update:high", "update:low"])
    }

    func testChangingDrawOrderReordersTheSequence() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let back = ProbeComponent("back", log: log, drawOrder: 1)
        let front = ProbeComponent("front", log: log, drawOrder: 9)
        try game.Components.Add(back)
        try game.Components.Add(front)

        log.entries = []
        try front.setDrawOrder(0)
        try game.Draw(time())
        XCTAssertEqual(log.entries, ["draw:front", "draw:back"])
    }

    // ------------------------------------------------------------------
    // Enabled and Visible.
    // ------------------------------------------------------------------

    // `Update` reads `Enabled` per component as it reaches it, so a disabled
    // component is skipped but stays in the list and resumes when re-enabled.
    func testDisabledComponentsAreSkippedButNotForgotten() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = ProbeComponent("a", log: log)
        try game.Components.Add(component)

        component.Enabled = false
        try game.Update(time())
        XCTAssertEqual(component.updateCount, 0)

        component.Enabled = true
        try game.Update(time())
        XCTAssertEqual(component.updateCount, 1)
    }

    func testInvisibleComponentsAreSkippedButNotForgotten() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = ProbeComponent("a", log: log)
        try game.Components.Add(component)

        component.Visible = false
        try game.Draw(time())
        XCTAssertEqual(component.drawCount, 0)

        component.Visible = true
        try game.Draw(time())
        XCTAssertEqual(component.drawCount, 1)
    }

    // ------------------------------------------------------------------
    // Removal.
    // ------------------------------------------------------------------

    // `GameComponentRemoved` takes the component out of both sorted lists and
    // unsubscribes from its order-changed event, so a later order change must
    // not put it back.
    func testRemovingAComponentStopsUpdatesAndDrawsAndUnsubscribes() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = ProbeComponent("a", log: log)
        try game.Components.Add(component)
        try game.Update(time())
        XCTAssertEqual(component.updateCount, 1)

        try game.Components.Remove(component)
        try game.Update(time())
        try game.Draw(time())
        XCTAssertEqual(component.updateCount, 1)
        XCTAssertEqual(component.drawCount, 0)

        // The handler is gone, so raising the event changes nothing.
        try component.setUpdateOrder(99)
        try game.Update(time())
        XCTAssertEqual(component.updateCount, 1,
                       "a removed component must not be resurrected by its own "
                       + "order-changed event")
    }

    // A component removed before the run is taken off the not-yet-initialized
    // queue too, so `Initialize` does not initialize it.
    func testRemovingBeforeTheRunAlsoUnqueuesIt() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = ProbeComponent("a", log: log)
        try game.Components.Add(component)
        try game.Components.Remove(component)
        try game.Initialize()
        XCTAssertEqual(component.initializeCount, 0)
    }

    // ------------------------------------------------------------------
    // Mutation during traversal.
    // ------------------------------------------------------------------

    // `Update` copies the list before walking it, so a component added during
    // the pass is not updated until the NEXT one.
    func testAComponentAddedDuringUpdateRunsOnlyFromTheNextPass() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let late = ProbeComponent("late", log: log, updateOrder: -100)
        let adder = AddingComponent(name: "adder", log: log, game: game,
                                    component: late)
        try game.Components.Add(adder)

        try game.Update(time())
        XCTAssertEqual(log.entries, ["update:adder"],
                       "the copy is what makes the current pass fixed")

        log.entries = []
        try game.Update(time())
        // `late` sorts first, so the next pass proves it was really inserted.
        XCTAssertEqual(log.entries, ["update:late", "update:adder"])
    }

    // ... and one removed during the pass is still reached, for the same
    // reason: the walk is over the copy.
    func testAComponentRemovedDuringUpdateIsStillReachedInThatPass() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let victim = ProbeComponent("victim", log: log, updateOrder: 10)
        let remover = RemovingComponent(name: "remover", log: log, game: game,
                                        component: victim, updateOrder: 1)
        try game.Components.Add(remover)
        try game.Components.Add(victim)

        try game.Update(time())
        XCTAssertEqual(log.entries, ["update:remover", "update:victim"])

        log.entries = []
        try game.Update(time())
        XCTAssertEqual(log.entries, ["update:remover"])
    }
}

/// A component whose `Update` adds another component to the game.
private final class AddingComponent:
    Microsoft.Xna.Framework.IGameComponent, Microsoft.Xna.Framework.IUpdateable
{
    private let name: String
    private let log: ProbeComponent.Log
    private weak var game: Microsoft.Xna.Framework.Game?
    private let component: any Microsoft.Xna.Framework.IGameComponent
    private var added = false
    private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
    private let updateOrderChangedSource = CNAEventSource<CNAEventArgs>()

    init(name: String, log: ProbeComponent.Log,
         game: Microsoft.Xna.Framework.Game,
         component: any Microsoft.Xna.Framework.IGameComponent) {
        self.name = name
        self.log = log
        self.game = game
        self.component = component
    }

    var Enabled: Bool { true }
    var UpdateOrder: Int32 { 0 }
    var EnabledChanged: CNAEvent<CNAEventArgs> { enabledChangedSource.Event }
    var UpdateOrderChanged: CNAEvent<CNAEventArgs> { updateOrderChangedSource.Event }
    func Initialize() throws {}

    func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        log.entries.append("update:\(name)")
        guard !added, let game else { return }
        added = true
        try game.Components.Add(component)
    }
}

/// A component whose `Update` removes another component from the game.
private final class RemovingComponent:
    Microsoft.Xna.Framework.IGameComponent, Microsoft.Xna.Framework.IUpdateable
{
    private let name: String
    private let log: ProbeComponent.Log
    private weak var game: Microsoft.Xna.Framework.Game?
    private let component: any Microsoft.Xna.Framework.IGameComponent
    private var removed = false
    private let order: Int32
    private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
    private let updateOrderChangedSource = CNAEventSource<CNAEventArgs>()

    init(name: String, log: ProbeComponent.Log,
         game: Microsoft.Xna.Framework.Game,
         component: any Microsoft.Xna.Framework.IGameComponent,
         updateOrder: Int32) {
        self.name = name
        self.log = log
        self.game = game
        self.component = component
        self.order = updateOrder
    }

    var Enabled: Bool { true }
    var UpdateOrder: Int32 { order }
    var EnabledChanged: CNAEvent<CNAEventArgs> { enabledChangedSource.Event }
    var UpdateOrderChanged: CNAEvent<CNAEventArgs> { updateOrderChangedSource.Event }
    func Initialize() throws {}

    func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        log.entries.append("update:\(name)")
        guard !removed, let game else { return }
        removed = true
        try game.Components.Remove(component)
    }
}

// ----------------------------------------------------------------------
// `Microsoft.Xna.Framework.GameComponent` itself, transcribed from the same
// assembly. It is the payoff of the engine above: an ordinary XNA component
// added to `Game.Components` and driven by it.
// ----------------------------------------------------------------------

private final class CountingComponent: Microsoft.Xna.Framework.GameComponent {
    private(set) var initializeCount = 0
    private(set) var updateCount = 0
    let name: String
    let log: ProbeComponent.Log

    init(_ name: String, log: ProbeComponent.Log,
         game: Microsoft.Xna.Framework.Game) {
        self.name = name
        self.log = log
        super.init(game: game)
    }

    override func Initialize() throws {
        initializeCount += 1
        log.entries.append("init:\(name)")
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        updateCount += 1
        log.entries.append("update:\(name)")
    }
}

extension GameComponentEngineTests {
    // `.ctor` sets `enabled = true` before the base call and leaves
    // `updateOrder` at its zero value, and `get_Game` is a bare field read.
    func testGameComponentDefaults() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let component = Microsoft.Xna.Framework.GameComponent(game: game)
        XCTAssertTrue(component.Enabled, "a component starts enabled")
        XCTAssertEqual(component.UpdateOrder, 0)
        XCTAssertTrue(component.Game === game)
    }

    // Both setters compare first: assigning the same value raises nothing, so
    // a no-op assignment does not make the game re-sort its list.
    func testGameComponentSettersRaiseOnlyOnARealChange() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let component = Microsoft.Xna.Framework.GameComponent(game: game)
        var enabledRaises = 0
        var orderRaises = 0
        _ = component.EnabledChanged.Add { _, _ in enabledRaises += 1 }
        _ = component.UpdateOrderChanged.Add { _, _ in orderRaises += 1 }

        component.Enabled = true
        component.UpdateOrder = 0
        XCTAssertEqual(enabledRaises, 0, "assigning the same value raises nothing")
        XCTAssertEqual(orderRaises, 0)

        component.Enabled = false
        component.UpdateOrder = 7
        XCTAssertEqual(enabledRaises, 1)
        XCTAssertEqual(orderRaises, 1)

        component.Enabled = false
        XCTAssertEqual(enabledRaises, 1)
    }

    // `OnEnabledChanged` ignores the sender it is handed and raises with
    // `this`. The parameter is part of the signature and not of the behaviour.
    func testOnChangedHandlersRaiseWithTheComponentAsSender() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let component = Microsoft.Xna.Framework.GameComponent(game: game)
        var observed: AnyObject?
        _ = component.EnabledChanged.Add { sender, _ in
            observed = sender as AnyObject
        }
        try component.OnEnabledChanged("a different sender",
                                       args: CNAEventArgs.Empty)
        XCTAssertTrue(observed === component)
    }

    // The whole point: a GameComponent added to Components is driven by the
    // engine, in UpdateOrder sequence and only while Enabled.
    func testGameComponentsAreDrivenByTheEngine() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let second = CountingComponent("second", log: log, game: game)
        second.UpdateOrder = 20
        let first = CountingComponent("first", log: log, game: game)
        first.UpdateOrder = 10

        try game.Components.Add(second)
        try game.Components.Add(first)
        try game.Initialize()
        XCTAssertEqual(log.entries, ["init:second", "init:first"])

        log.entries = []
        try game.Update(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(log.entries, ["update:first", "update:second"])

        // Changing the order through the real property raises the real event,
        // which is what the engine listens to.
        log.entries = []
        first.UpdateOrder = 30
        try game.Update(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(log.entries, ["update:second", "update:first"])

        log.entries = []
        second.Enabled = false
        try game.Update(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(log.entries, ["update:first"])
    }

    // `Dispose(true)` removes the component from `Game.Components` -- which
    // drives `GameComponentRemoved` and stops the updates -- and raises
    // `Disposed` AFTER the removal, so a handler sees a component the game has
    // already let go of.
    func testDisposeRemovesFromComponentsAndThenRaisesDisposed() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = CountingComponent("a", log: log, game: game)
        try game.Components.Add(component)
        try game.Update(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(component.updateCount, 1)

        var countAtRaise: Int32 = -1
        _ = component.Disposed.Add { _, _ in
            countAtRaise = game.Components.Count
        }
        try component.Dispose()
        XCTAssertEqual(game.Components.Count, 0)
        XCTAssertEqual(countAtRaise, 0,
                       "Disposed is raised after the removal, not before")

        try game.Update(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(component.updateCount, 1, "a disposed component stops")
    }

    // `Dispose(false)` is the finalizer path and does nothing at all.
    func testDisposeFalseDoesNothing() throws {
        let game = try makeGame()
        defer { try? game.Dispose() }
        let log = ProbeComponent.Log()
        let component = CountingComponent("a", log: log, game: game)
        try game.Components.Add(component)
        var raised = false
        _ = component.Disposed.Add { _, _ in raised = true }

        try component.Dispose(false)
        XCTAssertEqual(game.Components.Count, 1, "the finalizer path removes nothing")
        XCTAssertFalse(raised)
    }
}
