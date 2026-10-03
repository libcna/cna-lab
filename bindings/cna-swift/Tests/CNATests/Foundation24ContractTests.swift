// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for `IGraphicsDeviceService`, transcribed from the
// registered, hash-matched Microsoft.Xna.Framework.Graphics.dll and
// Microsoft.Xna.Framework.Game.dll IL.
//
// An interface declares no bodies, so what is observable about it is the
// contract every registered implementation must satisfy. This interface has
// exactly one registered implementor, and its whole shape follows from that
// implementor's IL.
extension PureValueTests {
    // `GraphicsDeviceManager::get_GraphicsDevice` is
    // `ldarg.0; ldfld device; ret` -- a bare field read with no branch, no call
    // and no throw -- so the requirement is infallible. `device` is never
    // assigned by `GraphicsDeviceManager..ctor`, and both `Dispose` (IL_00b0)
    // and `CreateDevice` (IL_0014) explicitly store `ldnull` into it, so the
    // reference is nullable. A service that has not created its device answers
    // nil, and XNA's own `BeginDraw`/`EndDraw` guard it with `brfalse` rather
    // than treating that as an error.
    func testGraphicsDeviceServiceRequirementIsNullableAndInfallible() {
        typealias Service = Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService

        final class BeforeDeviceCreation: Service {
            private let created = CNAEventSource<CNAEventArgs>()
            private let disposing = CNAEventSource<CNAEventArgs>()
            private let reset = CNAEventSource<CNAEventArgs>()
            private let resetting = CNAEventSource<CNAEventArgs>()
            var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { nil }
            var DeviceCreated: CNAEvent<CNAEventArgs> { created.Event }
            var DeviceDisposing: CNAEvent<CNAEventArgs> { disposing.Event }
            var DeviceReset: CNAEvent<CNAEventArgs> { reset.Event }
            var DeviceResetting: CNAEvent<CNAEventArgs> { resetting.Event }
        }

        // No `try` and no `do`/`catch`: the absence of a device is a value.
        let service: any Service = BeforeDeviceCreation()
        XCTAssertNil(service.GraphicsDevice)
    }

    // The four events are all `EventHandler<EventArgs>` in the pinned metadata,
    // declared with `add_`/`remove_` accessor pairs and no `raise_` accessor.
    // A conformer owns the raise capability privately and publishes only the
    // consumer view, which is what the CLR encoding means.
    func testGraphicsDeviceServiceRaisesItsFourEvents() throws {
        typealias Service = Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService

        final class Recording: Service {
            let created = CNAEventSource<CNAEventArgs>()
            let disposing = CNAEventSource<CNAEventArgs>()
            let reset = CNAEventSource<CNAEventArgs>()
            let resetting = CNAEventSource<CNAEventArgs>()
            var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { nil }
            var DeviceCreated: CNAEvent<CNAEventArgs> { created.Event }
            var DeviceDisposing: CNAEvent<CNAEventArgs> { disposing.Event }
            var DeviceReset: CNAEvent<CNAEventArgs> { reset.Event }
            var DeviceResetting: CNAEvent<CNAEventArgs> { resetting.Event }
        }

        let service = Recording()
        var order: [String] = []
        _ = service.DeviceCreated.Add { _, _ in order.append("created") }
        _ = service.DeviceResetting.Add { _, _ in order.append("resetting") }
        _ = service.DeviceReset.Add { _, _ in order.append("reset") }
        _ = service.DeviceDisposing.Add { _, _ in order.append("disposing") }

        try service.created.Raise(service, args: CNAEventArgs.Empty)
        try service.resetting.Raise(service, args: CNAEventArgs.Empty)
        try service.reset.Raise(service, args: CNAEventArgs.Empty)
        try service.disposing.Raise(service, args: CNAEventArgs.Empty)
        XCTAssertEqual(order, ["created", "resetting", "reset", "disposing"])
    }
}
