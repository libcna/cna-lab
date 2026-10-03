// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    /// `Microsoft.Xna.Framework.GameComponentCollection`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Game.dll` as a public **sealed**
    /// class whose direct base is
    /// `System.Collections.ObjectModel.Collection<IGameComponent>`. It declares
    /// only a parameterless constructor, four protected overrides and two
    /// events; every usable member — `Count`, `Item`, `Add`, `Clear`,
    /// `Contains`, `CopyTo`, `GetEnumerator`, `IndexOf`, `Insert`, `Remove`,
    /// `RemoveAt` — is inherited.
    ///
    /// That is why the base is a real Swift superclass. `CNACollection` is the
    /// measured projection of the `Collection<T>` shape pinned in
    /// `tools/api_compat/reference/bcl40-selected-shape.json`, and the CLR
    /// generic argument is preserved in the specialization: the base is
    /// `Collection<IGameComponent>`, so the Swift superclass is
    /// `CNACollection<any IGameComponent>` and not `CNACollection<Any>`.
    /// Erasing the element type would have made every inherited signature
    /// wrong.
    ///
    /// CLR `sealed` maps to Swift `final`.
    public final class GameComponentCollection:
        CNACollection<any Microsoft.Xna.Framework.IGameComponent>
    {
        // The declaring type owns its event sources privately and publishes
        // only the consumer views, which is the Foundation 19 architecture: a
        // consumer can subscribe and unsubscribe and can never raise. XNA
        // raises these from `OnComponentAdded` / `OnComponentRemoved`, both
        // `private` in the IL, so there is nothing further to project.
        private let componentAddedSource =
            CNAEventSource<Microsoft.Xna.Framework.GameComponentCollectionEventArgs>()
        private let componentRemovedSource =
            CNAEventSource<Microsoft.Xna.Framework.GameComponentCollectionEventArgs>()

        /// `.ctor()`.
        ///
        /// The IL body is exactly `base..ctor()`, so the collection gets the
        /// fresh `List<IGameComponent>` that `Collection<T>..ctor()` allocates.
        /// XNA declares no wrapping constructor, and neither does this.
        public override init() {
            super.init()
        }

        /// `event EventHandler<GameComponentCollectionEventArgs> ComponentAdded`.
        public var ComponentAdded:
            CNAEvent<Microsoft.Xna.Framework.GameComponentCollectionEventArgs>
        {
            componentAddedSource.Event
        }

        /// `event EventHandler<GameComponentCollectionEventArgs> ComponentRemoved`.
        public var ComponentRemoved:
            CNAEvent<Microsoft.Xna.Framework.GameComponentCollectionEventArgs>
        {
            componentRemovedSource.Event
        }

        /// `protected override void InsertItem(int index, IGameComponent item)`.
        ///
        /// The IL order is exact and each step is observable:
        ///
        /// 1. `base.IndexOf(item)`; if it is not `-1`, throw
        ///    `ArgumentException` with the assembly's own
        ///    `CannotAddSameComponentMultipleTimes` resource string. The
        ///    duplicate check happens **before** the insert, so a rejected
        ///    duplicate leaves the collection untouched and raises no event;
        /// 2. `base.InsertItem(index, item)` — the item is in the collection
        ///    before any handler runs;
        /// 3. `if (item != null)` raise `ComponentAdded`. XNA guards on the
        ///    item, so a null component would be inserted silently and no
        ///    event would fire. Swift's `any IGameComponent` cannot be nil, so
        ///    the guard is unreachable here and the raise is unconditional —
        ///    which is the same observable behaviour for every value this
        ///    signature admits.
        ///
        /// The event argument is a fresh `GameComponentCollectionEventArgs`
        /// carrying the item, and the sender is the collection.
        public override func InsertItem(
            _ index: Int32, item: any Microsoft.Xna.Framework.IGameComponent
        ) throws {
            guard IndexOf(item) == -1 else {
                // `newobj ArgumentException::.ctor(string)` -- the message-only
                // overload, so ParamName is nil.
                throw CNAArgumentException(
                    message: "Cannot add the same game component to a game "
                    + "component collection multiple times.")
            }
            try super.InsertItem(index, item: item)
            try componentAddedSource.Raise(
                self,
                args: Microsoft.Xna.Framework.GameComponentCollectionEventArgs(
                    gameComponent: item))
        }

        /// `protected override void RemoveItem(int index)`.
        ///
        /// The IL reads the item **first**, through `base.get_Item(index)`, so
        /// an out-of-range index fails there and nothing is removed. It then
        /// calls `base.RemoveItem(index)` and only afterwards raises
        /// `ComponentRemoved`, so a handler observes the collection with the
        /// item already gone.
        public override func RemoveItem(_ index: Int32) throws {
            let removed = try Item(index)
            try super.RemoveItem(index)
            try componentRemovedSource.Raise(
                self,
                args: Microsoft.Xna.Framework.GameComponentCollectionEventArgs(
                    gameComponent: removed))
        }

        /// `protected override void SetItem(int index, IGameComponent item)`.
        ///
        /// The IL body is a single `throw` and nothing else: no bounds check,
        /// no store, no event. XNA refuses indexed assignment on this
        /// collection outright, with `NotSupportedException` and the
        /// assembly's own `CannotSetItemsIntoGameComponentCollection` string.
        ///
        /// Because `Collection<T>.set_Item` performs its read-only and bounds
        /// checks *before* reaching the hook, an out-of-range index still
        /// reports the range rather than this refusal — the inherited order is
        /// preserved rather than short-circuited.
        public override func SetItem(
            _ index: Int32, item: any Microsoft.Xna.Framework.IGameComponent
        ) throws {
            throw CNANotSupportedException(
                message: "Cannot set a value using operator[] on "
                + "GameComponentCollection.  Use Add/Remove instead.")
        }

        /// `protected override void ClearItems()`.
        ///
        /// The IL walks **forward** from 0 to `Count`, raising
        /// `ComponentRemoved` for each item, and calls `base.ClearItems()`
        /// only after the loop. Every handler therefore runs while the
        /// collection is still **full**: a handler that reads `Count` during
        /// `Clear` sees the original count, and one that reads `Item(0)` sees
        /// the first component, not the one being reported.
        ///
        /// That ordering is unusual enough to be worth stating plainly — the
        /// obvious implementation, removing each item then announcing it, is
        /// observably different — and it is XNA's, taken from this
        /// assembly's IL rather than from any other binding.
        public override func ClearItems() throws {
            var index: Int32 = 0
            while index < Count {
                try componentRemovedSource.Raise(
                    self,
                    args: Microsoft.Xna.Framework.GameComponentCollectionEventArgs(
                        gameComponent: try Item(index)))
                index += 1
            }
            try super.ClearItems()
        }
    }
}
