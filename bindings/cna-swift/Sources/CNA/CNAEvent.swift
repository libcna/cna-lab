// SPDX-License-Identifier: MIT

import Foundation

// CLR event support projection.
//
// Every public event in the pinned XNA 4.0 contract has exactly one shape,
// `System.EventHandler<TArgs>`, and each is projected as exactly one get-only
// Swift property preserving its XNA name:
//
//     EnabledChanged: CNAEvent<CNAEventArgs>
//
// `add_`, `remove_` and `raise_` accessors are CLR encoding, not XNA
// identities, and are never exposed. The three support types below live
// outside `Microsoft.Xna.Framework`; they are language-support API and are not
// counted as XNA types or XNA identities.

/// Opaque registration token returned by `CNAEvent.Add`.
///
/// This token is the deliberate Swift projection of the part of CLR delegate
/// identity that Swift closures cannot represent. `Delegate.Remove` matches on
/// delegate identity — target object plus method — and two references to the
/// same C# method on the same instance are equal. Swift closures have no such
/// identity: two closures spelled identically are simply two values, and there
/// is no comparison that would recover the CLR rule. Registration therefore
/// hands back a token, and removal matches on that token.
///
/// The consequences are recorded honestly rather than hidden:
///
/// - each `Add` creates a distinct registration with a distinct token, so
///   adding the same closure twice registers it twice and each registration is
///   removed independently. CLR `Delegate.Remove` instead removes the *last*
///   matching entry of an equal delegate;
/// - the token exposes neither the handler nor any implementation state, so it
///   cannot be used to invoke, inspect or forge a registration;
/// - a token cannot be constructed outside this module;
/// - `deinit` deliberately does **not** unsubscribe. A CLR event keeps its
///   delegates alive until they are explicitly removed, and a token that
///   silently detached a handler when the last reference to it was dropped
///   would make subscription lifetime depend on Swift reference counting
///   rather than on the explicit `Remove` the CLR model uses.
public final class CNAEventSubscription {
    // Identity is the entire contract: two tokens are the same registration
    // exactly when they are the same object. Nothing is stored, so nothing
    // can leak.
    internal init() {}
}

/// Shared invocation state behind one CLR event.
///
/// This is the private storage that a `CNAEventSource` owns and that the
/// `CNAEvent` it publishes reads. It is deliberately internal: it is the only
/// object holding both the mutation and the raise capability, and neither the
/// consumer view nor the token can reach it.
///
/// The lock mirrors the CLR compiler-generated accessors, which combine and
/// remove delegates with `Interlocked.CompareExchange`, and the raise path,
/// which loads the delegate field once into a local before invoking it. Handler
/// invocation happens outside the lock, exactly as a CLR raise invokes the
/// captured invocation list with no lock held.
internal final class CNAEventStorage<TArgs> {
    internal typealias Handler = (Any?, TArgs) throws -> Void

    private let lock = NSLock()
    private var registrations: [(token: CNAEventSubscription, handler: Handler)] = []

    internal init() {}

    internal func add(_ handler: @escaping Handler) -> CNAEventSubscription {
        let token = CNAEventSubscription()
        lock.lock()
        defer { lock.unlock() }
        // Registration order is the invocation order, so new handlers always
        // append. Storage retains the handler strongly, as a CLR event retains
        // its delegates.
        registrations.append((token, handler))
        return token
    }

    internal func remove(_ subscription: CNAEventSubscription) {
        lock.lock()
        defer { lock.unlock() }
        // Exactly the registration created by the `Add` that returned this
        // token. A token that was already removed, or that belongs to another
        // event, simply matches nothing: removal is harmless, never corrupting.
        guard let index = registrations.firstIndex(
            where: { $0.token === subscription }
        ) else { return }
        registrations.remove(at: index)
    }

    internal func raise(_ sender: Any?, _ args: TArgs) throws {
        lock.lock()
        let snapshot = registrations
        lock.unlock()
        // Dispatch walks a snapshot, so an `Add` or `Remove` performed by a
        // handler affects later raises and never the raise in progress.
        for registration in snapshot {
            // A throwing handler propagates its error to the raiser and stops
            // the dispatch. No error is swallowed, and the registration list is
            // left exactly as it was.
            try registration.handler(sender, args)
        }
    }
}

/// The consumer-facing projection of one CLR event.
///
/// A consumer holding a `CNAEvent` can subscribe and unsubscribe and can do
/// nothing else. It cannot raise the event, enumerate the handlers, or reach
/// the storage. The type is `final`, and `CNAEventSource` is not a subclass of
/// it, so there is no downcast that recovers the raise capability — which is
/// what makes this the faithful projection of the CLR rule that only the
/// declaring type may raise an event.
public final class CNAEvent<TArgs> {
    private let storage: CNAEventStorage<TArgs>

    internal init(storage: CNAEventStorage<TArgs>) {
        self.storage = storage
    }

    /// Registers `handler` and returns the token that removes it.
    ///
    /// The handler receives the CLR `EventHandler<TArgs>` arguments: the
    /// `sender` the raiser supplied, and the event arguments. Handlers may
    /// throw; a non-throwing Swift closure is usable here unchanged, because
    /// Swift accepts a non-throwing function wherever a throwing one is
    /// expected. `Add` itself does not throw.
    @discardableResult
    public func Add(
        _ handler: @escaping (Any?, TArgs) throws -> Void
    ) -> CNAEventSubscription {
        storage.add(handler)
    }

    /// Removes the single registration created by the `Add` that returned
    /// `subscription`.
    ///
    /// Removing a token twice, or removing a token that belongs to a different
    /// event, is harmless and leaves both events unchanged.
    public func Remove(_ subscription: CNAEventSubscription) {
        storage.remove(subscription)
    }
}

/// The declaring side of one CLR event.
///
/// CLR encapsulation gives the declaring type a private field it can invoke and
/// gives everyone else only `add`/`remove`. Swift has no equivalent of a
/// member that is public to read and private to invoke, so the capability split
/// is modelled as two objects over one private storage:
///
///     private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
///
///     public var EnabledChanged: CNAEvent<CNAEventArgs> {
///         enabledChangedSource.Event
///     }
///
///     // inside the declaring type only:
///     try enabledChangedSource.Raise(self, args: CNAEventArgs.Empty)
///
/// `CNAEventSource` is **composition, never inheritance**: it deliberately does
/// not derive from `CNAEvent`, so a consumer handed a `CNAEvent` can never
/// downcast its way to `Raise`.
///
/// The type is public because external packages must be able to conform to XNA
/// protocols such as `IUpdateable` and `IDrawable` and raise their own events.
/// That publicness is language-support machinery; it is not an XNA identity.
///
/// The same shape is what a future native-backed event will use: a protected
/// runtime type owns the source privately, a native callback calls `Raise`, and
/// consumers still see only the `CNAEvent` view. No such event is implemented
/// here.
public final class CNAEventSource<TArgs> {
    private let storage: CNAEventStorage<TArgs>
    private let event: CNAEvent<TArgs>

    public init() {
        let storage = CNAEventStorage<TArgs>()
        self.storage = storage
        self.event = CNAEvent(storage: storage)
    }

    /// The consumer view to publish. The same object every time, so an event
    /// property backed by a source has stable identity as a CLR event field
    /// does.
    public var Event: CNAEvent<TArgs> {
        event
    }

    /// Invokes every registered handler, in registration order, over a snapshot
    /// of the invocation list.
    ///
    /// If a handler throws, that error propagates to the caller, no later
    /// handler runs, and the registration list is unchanged.
    public func Raise(_ sender: Any?, args: TArgs) throws {
        try storage.raise(sender, args)
    }
}
