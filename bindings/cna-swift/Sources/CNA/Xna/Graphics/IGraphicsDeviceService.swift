// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // Pinned in Microsoft.Xna.Framework.Graphics.dll as a public abstract
    // interface with no base interface and exactly five public identities: the
    // get-only `GraphicsDevice` property and the `DeviceCreated`,
    // `DeviceDisposing`, `DeviceReset` and `DeviceResetting` events. The IL
    // declares `add_`/`remove_` accessor pairs and no `raise_` accessor at all;
    // those accessors are the CLR encoding of an event, not XNA identities, and
    // are not projected.
    //
    // `GraphicsDevice` is the reason this type could not be declared before.
    // An abstract accessor has no body, so both its fallibility and its return
    // nullability are the ones a caller can actually be handed, and this
    // interface has exactly one registered implementor:
    //
    //     GraphicsDeviceManager::get_GraphicsDevice
    //       IL_0000:  ldarg.0
    //       IL_0001:  ldfld  GraphicsDevice GraphicsDeviceManager::device
    //       IL_0006:  ret
    //
    // A bare field read, so the accessor is infallible; and `device` is never
    // assigned by `GraphicsDeviceManager..ctor` and is explicitly `ldnull`-
    // stored by both `Dispose` (IL_00b0) and `CreateDevice` (IL_0014), so the
    // reference is nullable. XNA's own `IGraphicsDeviceManager.BeginDraw` and
    // `.EndDraw` guard it with `brfalse` rather than treating null as an error.
    // The requirement is therefore Optional *and* non-throwing: a service that
    // has not created its device yet answers `nil`, which is a normal result
    // and not a failure.
    //
    // Declaring the protocol claims no device capability. Nothing conforms to
    // it: `GraphicsDeviceManager` remains an untouched runtime partial whose
    // own reader cannot yet distinguish "no device yet" from a native failure.
    public protocol IGraphicsDeviceService {
        var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { get }

        var DeviceCreated: CNAEvent<CNAEventArgs> { get }

        var DeviceDisposing: CNAEvent<CNAEventArgs> { get }

        var DeviceReset: CNAEvent<CNAEventArgs> { get }

        var DeviceResetting: CNAEvent<CNAEventArgs> { get }
    }
}
