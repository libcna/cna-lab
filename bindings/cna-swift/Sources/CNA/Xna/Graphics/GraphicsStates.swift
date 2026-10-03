// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The shared behaviour of XNA's four state objects.
    ///
    /// This is an **internal** protocol, not a public base class. XNA derives
    /// `BlendState`, `DepthStencilState`, `RasterizerState` and `SamplerState`
    /// directly from `GraphicsResource` with no intermediate type, and the
    /// projection reproduces the metadata shape exactly: inserting a shared
    /// public base would be an invented XNA type. An internal Swift protocol
    /// carries the shared code without appearing in the public surface.
    ///
    /// All four own no native object: a caller allocates one before any device
    /// exists, sets properties on it, and hands it to a device. CNA models the
    /// same settings as POD descriptors with no handle, which is why
    /// `GraphicsResource` admits a resource with no storage.
    ///
    /// The behaviour they share is `ThrowIfBound`, which every setter calls:
    /// **a state object becomes read-only the first time it is bound to a
    /// device**, and a later write raises `InvalidOperationException` with the
    /// assembly's own `BoundStateObject` message formatted with the class's
    /// simple name.
    internal protocol GraphicsStateObject: GraphicsResource {
        /// `isBound`. False until a device takes this object.
        var isBound: Bool { get set }

        /// `GraphicsResource._parent` as `Apply` uses it: the device this
        /// state was last attached to, which decides whether an `Apply`
        /// rebinds or merely re-pushes.
        var attachedDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { get set }

        /// The name `ThrowIfBound` formats into the message.
        ///
        /// XNA loads it from `ldtoken <the declaring class>` — the **static**
        /// type, not `GetType()` — so a user subclass of `BlendState` still
        /// reports `BlendState`. A dynamic lookup would be a divergence, so
        /// each class states its own name.
        static var boundStateTypeName: String { get }
    }

    /// The `Microsoft.Xna.Framework.Graphics.BlendState` projection.
    open class BlendState: GraphicsResource, GraphicsStateObject {
        internal var isBound = false
        internal weak var attachedDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice?
        internal static let boundStateTypeName = "BlendState"

        /// `Dispose(Boolean)`. XNA declares its own override on each state
        /// class; the body is `base.Dispose(disposing)` in both branches of
        /// the compiler-emitted try/finally, so it adds no behaviour of its
        /// own — but it is part of the type's metadata shape.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        private var colorSourceBlend: Blend = .One
        private var colorDestinationBlend: Blend = .Zero
        private var colorBlendFunction: BlendFunction = .Add
        private var alphaSourceBlend: Blend = .One
        private var alphaDestinationBlend: Blend = .Zero
        private var alphaBlendFunction: BlendFunction = .Add
        private var colorWriteChannels: ColorWriteChannels = .All
        private var colorWriteChannels1: ColorWriteChannels = .All
        private var colorWriteChannels2: ColorWriteChannels = .All
        private var colorWriteChannels3: ColorWriteChannels = .All
        private var blendFactor: Microsoft.Xna.Framework.Color = .White
        private var multiSampleMask: Int32 = -1

        /// `BlendState..ctor()` — `SetDefaults()` then `isBound = false`.
        ///
        /// `SetDefaults` writes One/Zero/Add on both channels, `All` on all
        /// four write masks, `Color.White` and `-1`; every one of those writes
        /// goes through `ThrowIfBound`, which cannot fire on a fresh object.
        public init() { super.init(storage: nil, device: nil) }

        /// The private `.ctor(Blend sourceBlend, Blend destinationBlend, String name)`
        /// the static presets use. It defaults everything first and then
        /// overwrites **both** the colour and the alpha source and destination.
        ///
        /// Its last act is `isBound = true`, so **every static preset is born
        /// bound and is permanently read-only** — mutating `BlendState.Opaque`
        /// raises `InvalidOperationException`, which is how XNA stops a
        /// consumer corrupting a process-wide shared object.
        private convenience init(source: Blend, destination: Blend, name: String) {
            self.init()
            colorSourceBlend = source
            colorDestinationBlend = destination
            alphaSourceBlend = source
            alphaDestinationBlend = destination
            Name = name
            isBound = true
        }

        public var ColorSourceBlend: Blend { colorSourceBlend }
        public func SetColorSourceBlend(_ value: Blend) throws {
            try throwIfBound(); colorSourceBlend = value
        }
        public var ColorDestinationBlend: Blend { colorDestinationBlend }
        public func SetColorDestinationBlend(_ value: Blend) throws {
            try throwIfBound(); colorDestinationBlend = value
        }
        public var ColorBlendFunction: BlendFunction { colorBlendFunction }
        public func SetColorBlendFunction(_ value: BlendFunction) throws {
            try throwIfBound(); colorBlendFunction = value
        }
        public var AlphaSourceBlend: Blend { alphaSourceBlend }
        public func SetAlphaSourceBlend(_ value: Blend) throws {
            try throwIfBound(); alphaSourceBlend = value
        }
        public var AlphaDestinationBlend: Blend { alphaDestinationBlend }
        public func SetAlphaDestinationBlend(_ value: Blend) throws {
            try throwIfBound(); alphaDestinationBlend = value
        }
        public var AlphaBlendFunction: BlendFunction { alphaBlendFunction }
        public func SetAlphaBlendFunction(_ value: BlendFunction) throws {
            try throwIfBound(); alphaBlendFunction = value
        }
        public var ColorWriteChannels: ColorWriteChannels { colorWriteChannels }
        public func SetColorWriteChannels(_ value: ColorWriteChannels) throws {
            try throwIfBound(); colorWriteChannels = value
        }
        public var ColorWriteChannels1: ColorWriteChannels { colorWriteChannels1 }
        public func SetColorWriteChannels1(_ value: ColorWriteChannels) throws {
            try throwIfBound(); colorWriteChannels1 = value
        }
        public var ColorWriteChannels2: ColorWriteChannels { colorWriteChannels2 }
        public func SetColorWriteChannels2(_ value: ColorWriteChannels) throws {
            try throwIfBound(); colorWriteChannels2 = value
        }
        public var ColorWriteChannels3: ColorWriteChannels { colorWriteChannels3 }
        public func SetColorWriteChannels3(_ value: ColorWriteChannels) throws {
            try throwIfBound(); colorWriteChannels3 = value
        }
        public var BlendFactor: Microsoft.Xna.Framework.Color { blendFactor }
        public func SetBlendFactor(_ value: Microsoft.Xna.Framework.Color) throws {
            try throwIfBound(); blendFactor = value
        }
        public var MultiSampleMask: Int32 { multiSampleMask }
        public func SetMultiSampleMask(_ value: Int32) throws {
            try throwIfBound(); multiSampleMask = value
        }

        /// `BlendState.Opaque` — source `One`, destination `Zero`.
        public static let Opaque = BlendState(
            source: .One, destination: .Zero, name: "BlendState.Opaque")
        /// `BlendState.AlphaBlend` — source `One`, destination `InverseSourceAlpha`.
        public static let AlphaBlend = BlendState(
            source: .One, destination: .InverseSourceAlpha, name: "BlendState.AlphaBlend")
        /// `BlendState.Additive` — source `SourceAlpha`, destination `One`.
        public static let Additive = BlendState(
            source: .SourceAlpha, destination: .One, name: "BlendState.Additive")
        /// `BlendState.NonPremultiplied` — source `SourceAlpha`, destination
        /// `InverseSourceAlpha`.
        public static let NonPremultiplied = BlendState(
            source: .SourceAlpha, destination: .InverseSourceAlpha,
            name: "BlendState.NonPremultiplied")
    }

    /// The `Microsoft.Xna.Framework.Graphics.DepthStencilState` projection.
    open class DepthStencilState: GraphicsResource, GraphicsStateObject {
        internal var isBound = false
        internal weak var attachedDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice?
        internal static let boundStateTypeName = "DepthStencilState"

        /// `Dispose(Boolean)`. XNA declares its own override on each state
        /// class; the body is `base.Dispose(disposing)` in both branches of
        /// the compiler-emitted try/finally, so it adds no behaviour of its
        /// own — but it is part of the type's metadata shape.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        private var depthBufferEnable = true
        private var depthBufferWriteEnable = true
        private var depthBufferFunction: CompareFunction = .LessEqual
        private var stencilEnable = false
        private var stencilFunction: CompareFunction = .Always
        private var stencilPass: StencilOperation = .Keep
        private var stencilFail: StencilOperation = .Keep
        private var stencilDepthBufferFail: StencilOperation = .Keep
        private var twoSidedStencilMode = false
        private var counterClockwiseStencilFunction: CompareFunction = .Always
        private var counterClockwiseStencilPass: StencilOperation = .Keep
        private var counterClockwiseStencilFail: StencilOperation = .Keep
        private var counterClockwiseStencilDepthBufferFail: StencilOperation = .Keep
        private var stencilMask: Int32 = -1
        private var stencilWriteMask: Int32 = -1
        private var referenceStencil: Int32 = 0

        /// `DepthStencilState..ctor()`. `SetDefaults` writes depth testing and
        /// writing on, `CompareFunction.LessEqual`, stencil off with `Always`
        /// and `Keep` throughout, and `-1` masks.
        public init() { super.init(storage: nil, device: nil) }

        /// The private `.ctor(bool depthBufferEnable, bool depthBufferWriteEnable, String name)`,
        /// which also ends with `isBound = true`: the presets are read-only.
        private convenience init(depthEnable: Bool, depthWrite: Bool, name: String) {
            self.init()
            depthBufferEnable = depthEnable
            depthBufferWriteEnable = depthWrite
            Name = name
            isBound = true
        }

        public var DepthBufferEnable: Bool { depthBufferEnable }
        public func SetDepthBufferEnable(_ value: Bool) throws {
            try throwIfBound(); depthBufferEnable = value
        }
        public var DepthBufferWriteEnable: Bool { depthBufferWriteEnable }
        public func SetDepthBufferWriteEnable(_ value: Bool) throws {
            try throwIfBound(); depthBufferWriteEnable = value
        }
        public var DepthBufferFunction: CompareFunction { depthBufferFunction }
        public func SetDepthBufferFunction(_ value: CompareFunction) throws {
            try throwIfBound(); depthBufferFunction = value
        }
        public var StencilEnable: Bool { stencilEnable }
        public func SetStencilEnable(_ value: Bool) throws {
            try throwIfBound(); stencilEnable = value
        }
        public var StencilFunction: CompareFunction { stencilFunction }
        public func SetStencilFunction(_ value: CompareFunction) throws {
            try throwIfBound(); stencilFunction = value
        }
        public var StencilPass: StencilOperation { stencilPass }
        public func SetStencilPass(_ value: StencilOperation) throws {
            try throwIfBound(); stencilPass = value
        }
        public var StencilFail: StencilOperation { stencilFail }
        public func SetStencilFail(_ value: StencilOperation) throws {
            try throwIfBound(); stencilFail = value
        }
        public var StencilDepthBufferFail: StencilOperation { stencilDepthBufferFail }
        public func SetStencilDepthBufferFail(_ value: StencilOperation) throws {
            try throwIfBound(); stencilDepthBufferFail = value
        }
        public var TwoSidedStencilMode: Bool { twoSidedStencilMode }
        public func SetTwoSidedStencilMode(_ value: Bool) throws {
            try throwIfBound(); twoSidedStencilMode = value
        }
        public var CounterClockwiseStencilFunction: CompareFunction {
            counterClockwiseStencilFunction
        }
        public func SetCounterClockwiseStencilFunction(_ value: CompareFunction) throws {
            try throwIfBound(); counterClockwiseStencilFunction = value
        }
        public var CounterClockwiseStencilPass: StencilOperation {
            counterClockwiseStencilPass
        }
        public func SetCounterClockwiseStencilPass(_ value: StencilOperation) throws {
            try throwIfBound(); counterClockwiseStencilPass = value
        }
        public var CounterClockwiseStencilFail: StencilOperation {
            counterClockwiseStencilFail
        }
        public func SetCounterClockwiseStencilFail(_ value: StencilOperation) throws {
            try throwIfBound(); counterClockwiseStencilFail = value
        }
        public var CounterClockwiseStencilDepthBufferFail: StencilOperation {
            counterClockwiseStencilDepthBufferFail
        }
        public func SetCounterClockwiseStencilDepthBufferFail(
            _ value: StencilOperation
        ) throws {
            try throwIfBound(); counterClockwiseStencilDepthBufferFail = value
        }
        public var StencilMask: Int32 { stencilMask }
        public func SetStencilMask(_ value: Int32) throws {
            try throwIfBound(); stencilMask = value
        }
        public var StencilWriteMask: Int32 { stencilWriteMask }
        public func SetStencilWriteMask(_ value: Int32) throws {
            try throwIfBound(); stencilWriteMask = value
        }
        public var ReferenceStencil: Int32 { referenceStencil }
        public func SetReferenceStencil(_ value: Int32) throws {
            try throwIfBound(); referenceStencil = value
        }

        /// `DepthStencilState.Default` — depth testing and writing on.
        public static let Default = DepthStencilState(
            depthEnable: true, depthWrite: true, name: "DepthStencilState.Default")
        /// `DepthStencilState.DepthRead` — testing on, writing off.
        public static let DepthRead = DepthStencilState(
            depthEnable: true, depthWrite: false, name: "DepthStencilState.DepthRead")
        /// `DepthStencilState.None` — both off.
        public static let None = DepthStencilState(
            depthEnable: false, depthWrite: false, name: "DepthStencilState.None")
    }

    /// The `Microsoft.Xna.Framework.Graphics.RasterizerState` projection.
    open class RasterizerState: GraphicsResource, GraphicsStateObject {
        internal var isBound = false
        internal weak var attachedDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice?
        internal static let boundStateTypeName = "RasterizerState"

        /// `Dispose(Boolean)`. XNA declares its own override on each state
        /// class; the body is `base.Dispose(disposing)` in both branches of
        /// the compiler-emitted try/finally, so it adds no behaviour of its
        /// own — but it is part of the type's metadata shape.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        private var cullMode: CullMode = .CullCounterClockwiseFace
        private var fillMode: FillMode = .Solid
        private var scissorTestEnable = false
        private var multiSampleAntiAlias = true
        private var depthBias: Float = 0
        private var slopeScaleDepthBias: Float = 0

        /// `RasterizerState..ctor()`. `SetDefaults` writes
        /// `CullCounterClockwiseFace`, `Solid`, scissor off, multisample AA
        /// **on**, and zero biases.
        public init() { super.init(storage: nil, device: nil) }

        /// The private `.ctor(CullMode cullMode, String name)`, which also ends
        /// with `isBound = true`: the presets are read-only.
        private convenience init(cull: CullMode, name: String) {
            self.init()
            cullMode = cull
            Name = name
            isBound = true
        }

        public var CullMode: CullMode { cullMode }
        public func SetCullMode(_ value: CullMode) throws {
            try throwIfBound(); cullMode = value
        }
        public var FillMode: FillMode { fillMode }
        public func SetFillMode(_ value: FillMode) throws {
            try throwIfBound(); fillMode = value
        }
        public var ScissorTestEnable: Bool { scissorTestEnable }
        public func SetScissorTestEnable(_ value: Bool) throws {
            try throwIfBound(); scissorTestEnable = value
        }
        public var MultiSampleAntiAlias: Bool { multiSampleAntiAlias }
        public func SetMultiSampleAntiAlias(_ value: Bool) throws {
            try throwIfBound(); multiSampleAntiAlias = value
        }
        public var DepthBias: Float { depthBias }
        public func SetDepthBias(_ value: Float) throws {
            try throwIfBound(); depthBias = value
        }
        public var SlopeScaleDepthBias: Float { slopeScaleDepthBias }
        public func SetSlopeScaleDepthBias(_ value: Float) throws {
            try throwIfBound(); slopeScaleDepthBias = value
        }

        /// `RasterizerState.CullNone`.
        public static let CullNone = RasterizerState(
            cull: .None, name: "RasterizerState.CullNone")
        /// `RasterizerState.CullClockwise`.
        public static let CullClockwise = RasterizerState(
            cull: .CullClockwiseFace, name: "RasterizerState.CullClockwise")
        /// `RasterizerState.CullCounterClockwise`.
        public static let CullCounterClockwise = RasterizerState(
            cull: .CullCounterClockwiseFace,
            name: "RasterizerState.CullCounterClockwise")
    }

    /// The `Microsoft.Xna.Framework.Graphics.SamplerState` projection.
    open class SamplerState: GraphicsResource, GraphicsStateObject {
        internal var isBound = false
        internal weak var attachedDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice?
        internal static let boundStateTypeName = "SamplerState"

        /// `Dispose(Boolean)`. XNA declares its own override on each state
        /// class; the body is `base.Dispose(disposing)` in both branches of
        /// the compiler-emitted try/finally, so it adds no behaviour of its
        /// own — but it is part of the type's metadata shape.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        private var filter: TextureFilter = .Linear
        private var addressU: TextureAddressMode = .Wrap
        private var addressV: TextureAddressMode = .Wrap
        private var addressW: TextureAddressMode = .Wrap
        private var maxAnisotropy: Int32 = 4
        private var maxMipLevel: Int32 = 0
        private var mipMapLevelOfDetailBias: Float = 0

        /// `SamplerState..ctor()`. `SetDefaults` writes `Linear`, `Wrap` on all
        /// three axes, anisotropy 4, mip level 0 and a zero LOD bias.
        public init() { super.init(storage: nil, device: nil) }

        /// The private `.ctor(TextureFilter filter, TextureAddressMode addressMode, String name)`,
        /// which sets the one filter and the SAME address mode on all three
        /// axes, and ends with `isBound = true`: the presets are read-only.
        private convenience init(
            filter newFilter: TextureFilter,
            address: TextureAddressMode,
            name: String
        ) {
            self.init()
            filter = newFilter
            addressU = address
            addressV = address
            addressW = address
            Name = name
            isBound = true
        }

        public var Filter: TextureFilter { filter }
        public func SetFilter(_ value: TextureFilter) throws {
            try throwIfBound(); filter = value
        }
        public var AddressU: TextureAddressMode { addressU }
        public func SetAddressU(_ value: TextureAddressMode) throws {
            try throwIfBound(); addressU = value
        }
        public var AddressV: TextureAddressMode { addressV }
        public func SetAddressV(_ value: TextureAddressMode) throws {
            try throwIfBound(); addressV = value
        }
        public var AddressW: TextureAddressMode { addressW }
        public func SetAddressW(_ value: TextureAddressMode) throws {
            try throwIfBound(); addressW = value
        }
        public var MaxAnisotropy: Int32 { maxAnisotropy }
        public func SetMaxAnisotropy(_ value: Int32) throws {
            try throwIfBound(); maxAnisotropy = value
        }
        public var MaxMipLevel: Int32 { maxMipLevel }
        public func SetMaxMipLevel(_ value: Int32) throws {
            try throwIfBound(); maxMipLevel = value
        }
        public var MipMapLevelOfDetailBias: Float { mipMapLevelOfDetailBias }
        public func SetMipMapLevelOfDetailBias(_ value: Float) throws {
            try throwIfBound(); mipMapLevelOfDetailBias = value
        }

        /// `SamplerState.PointWrap`.
        public static let PointWrap = SamplerState(
            filter: .Point, address: .Wrap, name: "SamplerState.PointWrap")
        /// `SamplerState.PointClamp`.
        public static let PointClamp = SamplerState(
            filter: .Point, address: .Clamp, name: "SamplerState.PointClamp")
        /// `SamplerState.LinearWrap`.
        public static let LinearWrap = SamplerState(
            filter: .Linear, address: .Wrap, name: "SamplerState.LinearWrap")
        /// `SamplerState.LinearClamp`.
        public static let LinearClamp = SamplerState(
            filter: .Linear, address: .Clamp, name: "SamplerState.LinearClamp")
        /// `SamplerState.AnisotropicWrap`.
        public static let AnisotropicWrap = SamplerState(
            filter: .Anisotropic, address: .Wrap, name: "SamplerState.AnisotropicWrap")
        /// `SamplerState.AnisotropicClamp`.
        public static let AnisotropicClamp = SamplerState(
            filter: .Anisotropic, address: .Clamp, name: "SamplerState.AnisotropicClamp")
    }
}

/// The exact `BoundStateObject` template, read out of
/// `Microsoft.Xna.Framework.dll`'s own resource table rather than transcribed.
internal let boundStateObjectFormat =
    "Cannot change read-only {0}. State objects become read-only the "
    + "first time they are bound to a GraphicsDevice. To change "
    + "property values, create a new {0} instance."

extension Microsoft.Xna.Framework.Graphics.GraphicsStateObject {
    /// `ThrowIfBound()`.
    ///
    /// `String.Format(CurrentCulture, BoundStateObject, Type.Name)` — the
    /// **simple** name, not the namespace-qualified one, and the template
    /// substitutes it twice.
    internal func throwIfBound() throws {
        guard isBound else { return }
        throw CNAInvalidOperationException(
            message: boundStateObjectFormat.replacingOccurrences(
                of: "{0}", with: Self.boundStateTypeName))
    }

    /// Marks the object read-only, which is what binding it to a device does.
    /// Internal: a caller cannot bind a state object except by giving it to a
    /// device.
    internal func markBound() { isBound = true }

    /// The shared prologue of `SamplerState.Apply(device, samplerIndex)` and
    /// the three `Apply(device)` methods, which are identical:
    ///
    ///     if (isDisposed)
    ///         throw new ObjectDisposedException(typeof(<declaring>).Name);
    ///     if (_parent != device) { _parent = device; isBound = true; }
    ///     ...push the state...
    ///
    /// The disposal message names the **declaring** class through `ldtoken`,
    /// not `GetType()`, so it uses the same static name `ThrowIfBound` does.
    /// The push itself is per-class and is the caller's business.
    internal func attach(to device: Microsoft.Xna.Framework.Graphics.GraphicsDevice) throws {
        guard !IsDisposed else {
            throw CNAObjectDisposedException(objectName: Self.boundStateTypeName)
        }
        // `_parent != device` in the IL. A state object reaching a second
        // device rebinds and stays bound; reaching the same one again pushes
        // without touching `isBound`.
        if !isAttached(to: device) {
            attachedDevice = device
            isBound = true
        }
    }

    /// XNA compares `GraphicsResource._parent`, which the projection models as
    /// `GraphicsResource.GraphicsDevice`. That property is set at construction
    /// and a state object is constructed without one, so the attachment is
    /// tracked separately rather than by mutating a resource field the rest of
    /// the hierarchy treats as immutable.
    internal func isAttached(
        to device: Microsoft.Xna.Framework.Graphics.GraphicsDevice
    ) -> Bool {
        attachedDevice === device
    }
}
