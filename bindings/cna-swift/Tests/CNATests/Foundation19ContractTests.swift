// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 19 event batch, transcribed
// from the registered, hash-matched Microsoft.Xna.Framework.Game.dll and
// Microsoft.Xna.Framework.Graphics.dll IL.
extension PureValueTests {
    // IUpdateable is pinned as a public abstract interface with no base
    // interface and exactly five public identities: get_Enabled,
    // get_UpdateOrder, Update, and the EnabledChanged and UpdateOrderChanged
    // events. Both properties are get-only; there is no setter in the metadata.
    func testIUpdateableXnaContract() throws {
        let component = ContractUpdateable()
        let updateable: Microsoft.Xna.Framework.IUpdateable = component

        XCTAssertTrue(updateable.Enabled)
        XCTAssertEqual(updateable.UpdateOrder, 0)

        // Update takes exactly one GameTime and returns void.
        let gameTime = Microsoft.Xna.Framework.GameTime()
        try updateable.Update(gameTime)
        XCTAssertEqual(component.updates, 1)

        // XNA raises the change events from the property setters, and the IL
        // passes `this` as sender and the shared EventArgs.Empty as args.
        var senders: [Bool] = []
        var argsWereEmpty: [Bool] = []
        updateable.EnabledChanged.Add { sender, args in
            senders.append((sender as? ContractUpdateable) === component)
            argsWereEmpty.append(args === CNAEventArgs.Empty)
        }
        try component.setEnabled(false)
        XCTAssertFalse(updateable.Enabled)
        XCTAssertEqual(senders, [true])
        XCTAssertEqual(argsWereEmpty, [true])

        // GameComponent.set_Enabled compares before storing and returns
        // without raising when the value is unchanged: `beq.s` to `ret`.
        try component.setEnabled(false)
        XCTAssertEqual(senders.count, 1)

        var orderRaises = 0
        updateable.UpdateOrderChanged.Add { _, _ in orderRaises += 1 }
        try component.setUpdateOrder(3)
        XCTAssertEqual(updateable.UpdateOrder, 3)
        XCTAssertEqual(orderRaises, 1)
        try component.setUpdateOrder(3)
        XCTAssertEqual(orderRaises, 1)
    }

    // IDrawable is pinned identically shaped and, importantly, does **not**
    // extend IUpdateable: the two interfaces are independent in the metadata,
    // and DrawableGameComponent is what implements both.
    func testIDrawableXnaContract() throws {
        let component = ContractDrawable()
        let drawable: Microsoft.Xna.Framework.IDrawable = component

        XCTAssertTrue(drawable.Visible)
        XCTAssertEqual(drawable.DrawOrder, 0)

        try drawable.Draw(Microsoft.Xna.Framework.GameTime())
        XCTAssertEqual(component.draws, 1)

        var visibleRaises = 0
        var orderRaises = 0
        drawable.VisibleChanged.Add { _, _ in visibleRaises += 1 }
        drawable.DrawOrderChanged.Add { _, _ in orderRaises += 1 }
        try component.setVisible(false)
        try component.setDrawOrder(-2)
        XCTAssertFalse(drawable.Visible)
        XCTAssertEqual(drawable.DrawOrder, -2)
        XCTAssertEqual(visibleRaises, 1)
        XCTAssertEqual(orderRaises, 1)

        // An IDrawable conformer is not an IUpdateable: no inherited interface.
        XCTAssertFalse((drawable as Any) is Microsoft.Xna.Framework.IUpdateable)
    }

    // GameComponentCollectionEventArgs is pinned as a public, non-sealed class
    // extending System.EventArgs, with a public .ctor(IGameComponent) that
    // chains System.EventArgs::.ctor() and stores its argument with a single
    // stfld, and a getter that returns that field with a single ldfld. No
    // validation, no copying.
    func testGameComponentCollectionEventArgsXnaContract() {
        typealias Args = Microsoft.Xna.Framework.GameComponentCollectionEventArgs
        let component = ContractComponent()
        let args = Args(gameComponent: component)

        XCTAssertTrue(args.GameComponent as? ContractComponent === component)
        // Reference semantics: two argument objects over the same component are
        // distinct objects, and each raise carries the object it was given.
        let second = Args(gameComponent: component)
        XCTAssertFalse(args === second)
        XCTAssertTrue(second.GameComponent as? ContractComponent === component)

        // The declared base really is System.EventArgs.
        XCTAssertTrue((args as Any) is CNAEventArgs)
    }

    // ResourceCreatedEventArgs is pinned as a public **sealed** class extending
    // System.EventArgs whose only constructor is `assembly`: .ctor(object).
    // The single public identity is the get-only Resource, returning the stored
    // object field verbatim.
    func testResourceCreatedEventArgsXnaContract() {
        typealias Args = Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs
        let resource = ContractComponent()
        let args = Args(resource: resource)

        XCTAssertTrue(args.Resource as? ContractComponent === resource)
        XCTAssertTrue((args as Any) is CNAEventArgs)

        // System.Object maps to Any?, so a null resource is preserved as nil
        // rather than substituted.
        XCTAssertNil(Args(resource: nil).Resource)

        // A value resource is stored verbatim too; the IL boxes and returns it.
        XCTAssertEqual(Args(resource: Int32(7)).Resource as? Int32, 7)
    }

    // ResourceDestroyedEventArgs is pinned as a public **sealed** class
    // extending System.EventArgs with a single `assembly`
    // .ctor(string name, object tag). Both public identities are get-only and
    // return their stored fields verbatim.
    func testResourceDestroyedEventArgsXnaContract() {
        typealias Args = Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs
        let tag = ContractComponent()
        let args = Args(name: "surface", tag: tag)

        XCTAssertEqual(args.Name, "surface")
        XCTAssertTrue(args.Tag as? ContractComponent === tag)
        XCTAssertTrue((args as Any) is CNAEventArgs)

        // Neither field is validated or defaulted: an empty name and a null tag
        // are stored and returned exactly as supplied.
        let empty = Args(name: "", tag: nil)
        XCTAssertEqual(empty.Name, "")
        XCTAssertNil(empty.Tag)
    }

    // The pinned IL never constructs an event-argument object for an
    // EventHandler<EventArgs> raise. All 46 raise sites across the registered
    // Game.dll and Graphics.dll execute `ldsfld System.EventArgs::Empty`, and
    // none executes `newobj System.EventArgs::.ctor`, so every such handler
    // observes one and the same object.
    func testEventArgsEmptyIsOneSharedObjectXnaContract() {
        let first = CNAEventArgs.Empty
        let second = CNAEventArgs.Empty
        XCTAssertTrue(first === second)
        XCTAssertFalse(first === CNAEventArgs())
    }
}

// External conformers, defined outside the framework types exactly as a user
// type would be: each owns private CNAEventSource instances and publishes only
// the CNAEvent views.
private final class ContractUpdateable: Microsoft.Xna.Framework.IUpdateable {
    private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
    private let updateOrderChangedSource = CNAEventSource<CNAEventArgs>()
    private var enabled = true
    private var updateOrder: Int32 = 0
    private(set) var updates = 0

    var Enabled: Bool { enabled }

    var UpdateOrder: Int32 { updateOrder }

    var EnabledChanged: CNAEvent<CNAEventArgs> { enabledChangedSource.Event }

    var UpdateOrderChanged: CNAEvent<CNAEventArgs> { updateOrderChangedSource.Event }

    func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        updates += 1
    }

    // GameComponent.set_Enabled: compare, store, then raise with (this, Empty).
    func setEnabled(_ value: Bool) throws {
        guard enabled != value else { return }
        enabled = value
        try enabledChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }

    func setUpdateOrder(_ value: Int32) throws {
        guard updateOrder != value else { return }
        updateOrder = value
        try updateOrderChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }
}

private final class ContractDrawable: Microsoft.Xna.Framework.IDrawable {
    private let visibleChangedSource = CNAEventSource<CNAEventArgs>()
    private let drawOrderChangedSource = CNAEventSource<CNAEventArgs>()
    private var visible = true
    private var drawOrder: Int32 = 0
    private(set) var draws = 0

    var Visible: Bool { visible }

    var DrawOrder: Int32 { drawOrder }

    var VisibleChanged: CNAEvent<CNAEventArgs> { visibleChangedSource.Event }

    var DrawOrderChanged: CNAEvent<CNAEventArgs> { drawOrderChangedSource.Event }

    func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        draws += 1
    }

    func setVisible(_ value: Bool) throws {
        guard visible != value else { return }
        visible = value
        try visibleChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }

    func setDrawOrder(_ value: Int32) throws {
        guard drawOrder != value else { return }
        drawOrder = value
        try drawOrderChangedSource.Raise(self, args: CNAEventArgs.Empty)
    }
}

private final class ContractComponent: Microsoft.Xna.Framework.IGameComponent {
    func Initialize() throws {}
}
