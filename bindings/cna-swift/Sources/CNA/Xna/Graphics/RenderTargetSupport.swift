// SPDX-License-Identifier: MIT

import CNAShim

/// What a native ContentLost notification is delivered to.
///
/// `RenderTarget2D` derives from `Texture2D` and `RenderTargetCube` from
/// `TextureCube`, so the two cannot share a base class — but they subscribe
/// through the same route, with the same rooting rules and the same disposal
/// ordering. A protocol and one subscription object are what they share
/// instead, so a defect can only exist in one place.
internal protocol NativeContentLostReceiver: AnyObject {
    func nativeContentWasLost()
}

/// The rooted context a native ContentLost subscription carries.
///
/// The native callback is a plain C function pointer with a `void*`; this box
/// is what that pointer addresses. It is retained across the subscription and
/// released when the subscription is removed, so the pointer can never outlive
/// the Swift state it names — and it holds the target **weakly**, so a
/// subscription cannot keep a render target alive.
internal final class RenderTargetContentLostBox {
    weak var target: (any NativeContentLostReceiver)?
    init(_ target: any NativeContentLostReceiver) { self.target = target }
}

internal let renderTargetContentLostCallback: CNASwift_RenderTargetContentLostCallback = {
    _, context in
    guard let context else { return }
    let box = Unmanaged<RenderTargetContentLostBox>.fromOpaque(context).takeUnretainedValue()
    box.target?.nativeContentWasLost()
}

/// One live `cna_render_target_subscribe_content_lost` registration.
///
/// Owns both halves that must be released together — the native registration
/// and the retained Swift box — so neither can be released without the other.
internal final class RenderTargetContentLostSubscription {
    /// The native registration handle, `0` when there is none.
    ///
    /// Internal rather than private so a test can assert that disposal
    /// released it: a subscription that outlived its target would leave a
    /// native callback addressing a released Swift box.
    internal private(set) var registration: UInt64 = 0
    private var box: Unmanaged<RenderTargetContentLostBox>?
    private let runtime: RuntimeState

    init(runtime: RuntimeState) { self.runtime = runtime }

    func subscribe(handle: UInt64, receiver: any NativeContentLostReceiver) throws {
        let retained = Unmanaged.passRetained(RenderTargetContentLostBox(receiver))
        var handleOut: UInt64 = 0
        let result = runtime.functions.renderTargetSubscribeContentLost(
            handle, renderTargetContentLostCallback, retained.toOpaque(), &handleOut)
        guard result == 0 else {
            retained.release()
            try runtime.functions.check(
                result, operation: "cna_render_target_subscribe_content_lost")
            return
        }
        registration = handleOut
        box = retained
    }

    func release() {
        guard registration != 0 else { return }
        _ = runtime.functions.renderTargetUnsubscribeContentLost(registration)
        registration = 0
        box?.release()
        box = nil
    }

    deinit { release() }
}

extension Microsoft.Xna.Framework.Graphics {
    /// What `RenderTargetHelper.IsSameSize` compares, from either kind.
    ///
    /// XNA reads a `RenderTargetHelper` off whichever concrete type it finds:
    ///
    /// ```text
    /// RenderTargetHelper a = (renderTargetA as RenderTarget2D)?.helper
    ///                     ?? (renderTargetA as RenderTargetCube)?.helper;
    /// ... same for b ...
    /// return a.width == b.width && a.height == b.height
    ///     && a.multiSampleCount == b.multiSampleCount
    ///     && a.pixelSize == b.pixelSize;
    /// ```
    ///
    /// Four fields, not two. `pixelSize` is the colour format's byte size, so
    /// two targets of the same dimensions in `Color` and in `Rgba1010102` are
    /// the *same* size to this test — which is why it is the format's width in
    /// bytes and not the format itself.
    internal protocol RenderTargetDescription: AnyObject {
        var renderTargetWidth: Int32 { get }
        var renderTargetHeight: Int32 { get }
        var renderTargetMultiSampleCount: Int32 { get }
        var renderTargetFormat: SurfaceFormat { get }
    }

    /// `RenderTargetHelper.IsSameSize(Texture, Texture)`.
    ///
    /// A `Texture` that is neither render target reaches XNA's `ldnull` branch
    /// and the comparison then dereferences null — a `NullReferenceException`
    /// no caller can reach, because `RenderTargetBinding`'s two constructors
    /// accept only the two render-target types. `false` is the answer here for
    /// the same input, which is the honest projection of an unreachable throw.
    internal static func renderTargetsAreSameSize(
        _ a: Texture, _ b: Texture
    ) -> Bool {
        guard let left = a as? any RenderTargetDescription,
              let right = b as? any RenderTargetDescription else { return false }
        guard let leftPixel = expectedByteSize(of: left.renderTargetFormat),
              let rightPixel = expectedByteSize(of: right.renderTargetFormat) else {
            return false
        }
        return sameRenderTargetShape(
            (left.renderTargetWidth, left.renderTargetHeight,
             left.renderTargetMultiSampleCount, leftPixel),
            (right.renderTargetWidth, right.renderTargetHeight,
             right.renderTargetMultiSampleCount, rightPixel))
    }

    /// The four-way comparison itself, over the four numbers.
    ///
    /// Split out from the `Texture`-taking predicate because **it cannot be
    /// exercised through one on this host.** Only `SurfaceFormat.Color` can be
    /// created here (`build-probe/f55_grants.c`) and the qualified artifact
    /// grants a multisample count of zero, so no two render targets this
    /// binding can construct differ in either of the two fields the extents do
    /// not already separate. A mutation that dropped the pixel-size comparison
    /// therefore survived, and this is what makes it falsifiable: the
    /// arithmetic is the claim, and the arithmetic is testable.
    internal static func sameRenderTargetShape(
        _ left: (width: Int32, height: Int32, samples: Int32, pixelSize: Int32),
        _ right: (width: Int32, height: Int32, samples: Int32, pixelSize: Int32)
    ) -> Bool {
        left.width == right.width
            && left.height == right.height
            && left.samples == right.samples
            && left.pixelSize == right.pixelSize
    }
}

extension Microsoft.Xna.Framework.Graphics.RenderTarget2D:
    Microsoft.Xna.Framework.Graphics.RenderTargetDescription {
    internal var renderTargetWidth: Int32 { Width }
    internal var renderTargetHeight: Int32 { Height }
    internal var renderTargetMultiSampleCount: Int32 { MultiSampleCount }
    internal var renderTargetFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat {
        Format
    }
}

extension Microsoft.Xna.Framework.Graphics.RenderTargetCube:
    Microsoft.Xna.Framework.Graphics.RenderTargetDescription {
    /// A cube's faces are square, so both extents are `Size`.
    internal var renderTargetWidth: Int32 { Size }
    internal var renderTargetHeight: Int32 { Size }
    internal var renderTargetMultiSampleCount: Int32 { MultiSampleCount }
    internal var renderTargetFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat {
        Format
    }
}
