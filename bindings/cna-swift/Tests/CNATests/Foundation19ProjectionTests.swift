// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the Foundation 19 CLR event
// architecture. Multicast-delegate dispatch is a BCL rule rather than something
// transcribed from XNA IL, and token-based removal is an outright Swift
// projection, so none of this is counted as pure XNA-derived behaviour.
final class Foundation19EventProjectionTests: XCTestCase {
    typealias Args = CNAEventArgs
    typealias Source = CNAEventSource<CNAEventArgs>

    // MARK: - Dispatch

    func testRaiseWithZeroHandlersIsANoOp() throws {
        let source = Source()
        // A CLR raise site null-checks the delegate field and returns; there is
        // no error and nothing to observe.
        try source.Raise(self, args: Args.Empty)
        XCTAssertNotNil(source.Event)
    }

    func testSingleHandlerReceivesSenderAndArgs() throws {
        let source = Source()
        let sender = NSObject()
        let args = Args()
        var seen: [(Any?, Args)] = []
        source.Event.Add { received, receivedArgs in
            seen.append((received, receivedArgs))
        }

        try source.Raise(sender, args: args)

        XCTAssertEqual(seen.count, 1)
        // Sender identity is passed through untouched, exactly as
        // EventHandler<T>.Invoke(object, T) does.
        XCTAssertTrue(seen[0].0 as? NSObject === sender)
        // The args object is the very object supplied, not a copy: CNAEventArgs
        // is a class precisely so this identity survives.
        XCTAssertTrue(seen[0].1 === args)
    }

    func testHandlersRunInRegistrationOrder() throws {
        let source = Source()
        var order: [Int] = []
        for index in 0..<5 {
            source.Event.Add { _, _ in order.append(index) }
        }

        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(order, [0, 1, 2, 3, 4])
    }

    func testNilSenderIsPreserved() throws {
        let source = Source()
        var senderWasNil = false
        source.Event.Add { sender, _ in senderWasNil = sender == nil }

        try source.Raise(nil, args: Args.Empty)

        XCTAssertTrue(senderWasNil)
    }

    // MARK: - Duplicate registration and token identity

    func testAddingTheSameClosureTwiceCreatesTwoIndependentRegistrations() throws {
        let source = Source()
        var calls = 0
        let handler: (Any?, Args) throws -> Void = { _, _ in calls += 1 }

        let first = source.Event.Add(handler)
        let second = source.Event.Add(handler)

        // This is the documented divergence from CLR: Delegate.Combine would
        // also produce two entries, but Delegate.Remove matches on delegate
        // identity and would remove one of *either*. Here each registration has
        // its own token and is removed on its own terms.
        XCTAssertFalse(first === second)
        try source.Raise(nil, args: Args.Empty)
        XCTAssertEqual(calls, 2)
    }

    func testRemovingTheFirstDuplicateLeavesTheSecond() throws {
        let source = Source()
        var order: [String] = []
        let first = source.Event.Add { _, _ in order.append("first") }
        source.Event.Add { _, _ in order.append("second") }

        source.Event.Remove(first)
        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(order, ["second"])
    }

    func testRemovingTheSecondDuplicateLeavesTheFirst() throws {
        let source = Source()
        var order: [String] = []
        source.Event.Add { _, _ in order.append("first") }
        let second = source.Event.Add { _, _ in order.append("second") }

        source.Event.Remove(second)
        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(order, ["first"])
    }

    func testRemovingATokenTwiceIsHarmless() throws {
        let source = Source()
        var calls = 0
        let token = source.Event.Add { _, _ in calls += 1 }
        source.Event.Add { _, _ in calls += 1 }

        source.Event.Remove(token)
        source.Event.Remove(token)
        source.Event.Remove(token)
        try source.Raise(nil, args: Args.Empty)

        // The second and third removals must not consume the surviving
        // registration.
        XCTAssertEqual(calls, 1)
    }

    func testRemovingAForeignTokenCorruptsNeitherEvent() throws {
        let first = Source()
        let second = Source()
        var firstCalls = 0
        var secondCalls = 0
        let firstToken = first.Event.Add { _, _ in firstCalls += 1 }
        second.Event.Add { _, _ in secondCalls += 1 }

        // A token belonging to another event matches nothing here.
        second.Event.Remove(firstToken)

        try first.Raise(nil, args: Args.Empty)
        try second.Raise(nil, args: Args.Empty)
        XCTAssertEqual(firstCalls, 1)
        XCTAssertEqual(secondCalls, 1)
    }

    func testTokenDeinitDoesNotUnsubscribe() throws {
        let source = Source()
        var calls = 0
        do {
            // The only reference to the token goes out of scope here. A CLR
            // event keeps its delegates until they are explicitly removed, so
            // the registration must survive.
            let token = source.Event.Add { _, _ in calls += 1 }
            XCTAssertNotNil(token)
        }

        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(calls, 1)
    }

    func testStorageRetainsTheHandlerStrongly() throws {
        let source = Source()
        var observed: String?
        do {
            let captured = Captured(name: "retained")
            source.Event.Add { _, _ in observed = captured.name }
        }

        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(observed, "retained")
    }

    // MARK: - Mutation during dispatch

    func testSubscribingDuringDispatchAffectsOnlyLaterRaises() throws {
        let source = Source()
        var calls: [String] = []
        source.Event.Add { [weak source] _, _ in
            calls.append("outer")
            source?.Event.Add { _, _ in calls.append("added") }
        }

        try source.Raise(nil, args: Args.Empty)
        // Dispatch walks a snapshot, so the handler added mid-dispatch does not
        // run in this raise.
        XCTAssertEqual(calls, ["outer"])

        calls.removeAll()
        try source.Raise(nil, args: Args.Empty)
        // The second raise takes a fresh snapshot, which now includes it. The
        // first handler adds another as it runs, so exactly one "added" fires.
        XCTAssertEqual(calls, ["outer", "added"])
    }

    func testRemovalDuringDispatchAffectsOnlyLaterRaises() throws {
        let source = Source()
        var calls: [String] = []
        var laterToken: CNAEventSubscription?
        source.Event.Add { [weak source] _, _ in
            calls.append("first")
            if let laterToken { source?.Event.Remove(laterToken) }
        }
        laterToken = source.Event.Add { _, _ in calls.append("second") }

        try source.Raise(nil, args: Args.Empty)
        // "second" was already in the snapshot when the raise began, so it
        // still runs even though the first handler removed it.
        XCTAssertEqual(calls, ["first", "second"])

        calls.removeAll()
        try source.Raise(nil, args: Args.Empty)
        XCTAssertEqual(calls, ["first"])
    }

    func testSelfRemovalDuringDispatchDoesNotDisturbTheCurrentSnapshot() throws {
        let source = Source()
        var calls: [String] = []
        var ownToken: CNAEventSubscription?
        ownToken = source.Event.Add { [weak source] _, _ in
            calls.append("self-removing")
            if let ownToken { source?.Event.Remove(ownToken) }
        }
        source.Event.Add { _, _ in calls.append("trailing") }

        try source.Raise(nil, args: Args.Empty)
        XCTAssertEqual(calls, ["self-removing", "trailing"])

        calls.removeAll()
        try source.Raise(nil, args: Args.Empty)
        XCTAssertEqual(calls, ["trailing"])
    }

    // MARK: - Exception propagation

    func testAThrowingFirstHandlerStopsDispatchAndPropagates() {
        let source = Source()
        var calls: [String] = []
        source.Event.Add { _, _ in
            calls.append("first")
            throw CNAError.producerInvariant("handler failed")
        }
        source.Event.Add { _, _ in calls.append("second") }

        XCTAssertThrowsError(try source.Raise(nil, args: Args.Empty)) { error in
            // The handler's own error reaches the raiser unchanged. Nothing is
            // swallowed and nothing is wrapped.
            XCTAssertEqual(error as? CNAError, CNAError.producerInvariant("handler failed"))
        }
        XCTAssertEqual(calls, ["first"])
    }

    func testAThrowingMiddleHandlerStopsTheHandlersAfterIt() {
        let source = Source()
        var calls: [String] = []
        source.Event.Add { _, _ in calls.append("first") }
        source.Event.Add { _, _ in
            calls.append("second")
            throw CNAError.callbackOutsideGameLifecycle
        }
        source.Event.Add { _, _ in calls.append("third") }

        XCTAssertThrowsError(try source.Raise(nil, args: Args.Empty)) { error in
            XCTAssertEqual(error as? CNAError, CNAError.callbackOutsideGameLifecycle)
        }
        XCTAssertEqual(calls, ["first", "second"])
    }

    func testOnlyTheFirstErrorPropagates() {
        let source = Source()
        source.Event.Add { _, _ in throw CNAError.producerInvariant("first") }
        source.Event.Add { _, _ in throw CNAError.producerInvariant("second") }

        XCTAssertThrowsError(try source.Raise(nil, args: Args.Empty)) { error in
            XCTAssertEqual(error as? CNAError, CNAError.producerInvariant("first"))
        }
    }

    func testTheRegistrationListSurvivesAThrownDispatch() throws {
        let source = Source()
        var calls: [String] = []
        var shouldThrow = true
        source.Event.Add { _, _ in
            calls.append("first")
            if shouldThrow { throw CNAError.producerInvariant("once") }
        }
        source.Event.Add { _, _ in calls.append("second") }

        XCTAssertThrowsError(try source.Raise(nil, args: Args.Empty))
        XCTAssertEqual(calls, ["first"])

        // A throwing dispatch removes nothing; the next raise sees both.
        calls.removeAll()
        shouldThrow = false
        try source.Raise(nil, args: Args.Empty)
        XCTAssertEqual(calls, ["first", "second"])
    }

    func testANonThrowingClosureIsUsableAsAHandler() throws {
        let source = Source()
        var calls = 0
        // Declared without `throws`: Swift function subtyping accepts it where
        // the throwing handler type is expected, so a consumer never has to
        // write `throws` it does not need.
        let handler: (Any?, Args) -> Void = { _, _ in calls += 1 }
        source.Event.Add(handler)

        try source.Raise(nil, args: Args.Empty)

        XCTAssertEqual(calls, 1)
    }

    // MARK: - Capability boundary

    func testTheConsumerViewIsStableAndCarriesNoRaiseCapability() {
        let source = Source()

        // The published view is one object, so an event property backed by a
        // source has stable identity like a CLR event field.
        XCTAssertTrue(source.Event === source.Event)

        // A consumer holding only the view cannot reach the source. The two
        // types are related by composition, never inheritance, so neither has a
        // superclass at all and no downcast can recover Raise. This is what
        // replaces CLR's "only the declaring type may raise".
        //
        // The compiler enforces it before this test even runs: writing
        // `view is CNAEventSource<Args>` is rejected as a cast "to unrelated
        // type ... always fails", which is a stronger guarantee than any
        // runtime assertion. What is checked here is the shape that makes that
        // true.
        let view: CNAEvent<Args> = source.Event
        XCTAssertNil(Mirror(reflecting: view).superclassMirror)
        XCTAssertNil(Mirror(reflecting: source).superclassMirror)
        XCTAssertTrue(type(of: view) == CNAEvent<Args>.self)
    }

    func testDistinctSourcesDoNotShareStorage() throws {
        let first = Source()
        let second = Source()
        var firstCalls = 0
        first.Event.Add { _, _ in firstCalls += 1 }

        try second.Raise(nil, args: Args.Empty)

        XCTAssertEqual(firstCalls, 0)
    }

    // MARK: - CNAEventArgs

    func testEventArgsIsAReferenceTypeWithSharedEmpty() {
        // Empty is one shared instance, so every raise that uses it hands
        // handlers the same object, as the pinned XNA IL does.
        XCTAssertTrue(CNAEventArgs.Empty === CNAEventArgs.Empty)
        // A fresh instance is a different object: the type has CLR reference
        // semantics, not the value semantics the earlier support struct had.
        XCTAssertFalse(CNAEventArgs() === CNAEventArgs.Empty)
        XCTAssertFalse(CNAEventArgs() === CNAEventArgs())
    }

    func testEventArgsIsOpenAndSubclassable() {
        // The whole reason for the struct-to-class migration: the CLR
        // event-argument hierarchy must be expressible.
        let derived = ExternalArgs(marker: 7)
        let base: CNAEventArgs = derived
        XCTAssertTrue(base === derived)
        XCTAssertTrue((base as? ExternalArgs) === derived)
        XCTAssertEqual(derived.marker, 7)

        // The subclass really does sit on CNAEventArgs, and CNAEventArgs itself
        // is the root of the hierarchy.
        let superclass = Mirror(reflecting: derived).superclassMirror
        XCTAssertTrue(superclass?.subjectType == CNAEventArgs.self)
        XCTAssertNil(Mirror(reflecting: CNAEventArgs.Empty).superclassMirror)
    }

    func testDerivedArgsSurviveDispatchWithTheirDynamicType() throws {
        let source = Source()
        var observed: CNAEventArgs?
        source.Event.Add { _, args in observed = args }
        let derived = ExternalArgs(marker: 11)

        try source.Raise(nil, args: derived)

        XCTAssertTrue(observed === derived)
        XCTAssertEqual((observed as? ExternalArgs)?.marker, 11)
    }

    func testEventArgsDoesNotClaimSendable() {
        // An open class whose subclasses may add mutable state cannot honestly
        // promise cross-actor safety, so the conformance is deliberately absent
        // rather than asserted with @unchecked.
        //
        // Sendable is a marker protocol, so there is no runtime cast to test.
        // Overload resolution is the observable proof instead: the constrained
        // overload is selected only when the argument really does conform.
        XCTAssertFalse(claimsSendable(CNAEventArgs.Empty))
        XCTAssertFalse(claimsSendable(ExternalArgs(marker: 1)))
        // The control: a type that does conform selects the other overload, so
        // a false result means "does not conform" rather than "never matches".
        XCTAssertTrue(claimsSendable(42))
        XCTAssertTrue(claimsSendable("qualified"))
    }

    // MARK: - Typed events

    func testATypedEventCarriesItsOwnArgumentType() throws {
        typealias Component = Microsoft.Xna.Framework.IGameComponent
        typealias ComponentArgs = Microsoft.Xna.Framework.GameComponentCollectionEventArgs
        let source = CNAEventSource<ComponentArgs>()
        var observed: Component?
        source.Event.Add { _, args in observed = args.GameComponent }
        let component = ProbeComponent()

        try source.Raise(self, args: ComponentArgs(gameComponent: component))

        XCTAssertTrue(observed as? ProbeComponent === component)
    }
}

// A user type deriving from the support base, proving the hierarchy is open.
private final class ExternalArgs: CNAEventArgs {
    let marker: Int

    init(marker: Int) {
        self.marker = marker
        super.init()
    }
}

// Overload resolution prefers the constrained generic when the argument
// conforms, so the return value reports conformance to a marker protocol that
// cannot be interrogated with `is`.
private func claimsSendable<T: Sendable>(_ value: T) -> Bool { true }

private func claimsSendable<T>(_ value: T) -> Bool { false }

private final class Captured {
    let name: String
    init(name: String) { self.name = name }
}

private final class ProbeComponent: Microsoft.Xna.Framework.IGameComponent {
    func Initialize() throws {}
}
