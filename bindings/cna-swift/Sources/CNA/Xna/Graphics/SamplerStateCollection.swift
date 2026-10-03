// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.SamplerStateCollection` projection.
    ///
    /// `sealed`, extends `System.Object`, and declares exactly one public
    /// member: the `Item[Int32]` indexer. Both its accessors are recorded
    /// `IL_DIRECT_THROW`, so the projection is a throwing indexed reader plus a
    /// throwing `SetItem` writer method rather than a Swift `subscript`.
    ///
    /// XNA constructs one per device in the device constructor and hands it out
    /// by field read. Here it lives on `RuntimeState`, for the reason recorded
    /// there: CNA's device handle is a per-callback token, so the
    /// `GraphicsDevice` facade cannot own anything that must outlive a
    /// callback.
    public final class SamplerStateCollection {
        /// `pDevice`. Weak: the collection belongs to the game, and a facade
        /// lives only as long as the callback that produced it.
        private weak var device: GraphicsDevice?

        /// `pSamplerList`. Optional elements because `new SamplerState[n]`
        /// starts null and `InitializeDeviceState` is what fills it.
        private var slots: [SamplerState?]

        /// `samplerOffset` — `0` for the pixel collection and `0x101` for the
        /// vertex one, which is `D3DVERTEXTEXTURESAMPLER0`. It is added to the
        /// index before `Apply`, and is not the CNA slot: CNA takes a stage
        /// and a zero-based slot instead, so the offset selects the stage here.
        private let samplerOffset: Int32

        /// `CNA_SHADER_STAGE_PIXEL` / `CNA_SHADER_STAGE_VERTEX`.
        private let stage: UInt32

        /// The assembly-visible `.ctor(GraphicsDevice pParent, int samplerOffset,
        /// int maxSamplers)`.
        internal init(device: GraphicsDevice?, samplerOffset: Int32, stage: UInt32, count: Int) {
            self.device = device
            self.samplerOffset = samplerOffset
            self.stage = stage
            slots = Array(repeating: nil, count: count)
        }

        /// The number of slots. Not an XNA member — `SamplerStateCollection`
        /// declares no `Count` — so it is internal and exists only for the
        /// bounds checks and the tests.
        internal var count: Int { slots.count }

        /// `SamplerStateCollection.get_Item(int index)`.
        ///
        /// `index < 0 || index >= pSamplerList.Length` raises
        /// `ArgumentOutOfRangeException("index")` — the single-argument
        /// constructor, so the message is the substituted
        /// `Arg_ArgumentOutOfRangeException` with the parameter name appended.
        ///
        /// Non-Optional: the recorded return-nullability verdict is
        /// `UNKNOWN_REFERENCE_NULLABILITY`, and the deferral rule keeps the
        /// non-Optional shape until nullability is proven. That is also
        /// behaviourally right on an initialised device, where
        /// `InitializeDeviceState` has filled every slot — but a slot this
        /// binding has never written has no value to answer with, so it
        /// answers `SamplerState.LinearWrap`, which is exactly what XNA's own
        /// initialisation put there.
        public func Item(_ index: Int32) throws -> SamplerState {
            try slots[checkedIndex(index)] ?? SamplerState.LinearWrap
        }

        /// `SamplerStateCollection.set_Item(int index, SamplerState value)`.
        ///
        /// The IL, in order: bounds-check; refuse null with
        /// `ArgumentNullException("value", NullNotAllowed)`; return when the
        /// value is the **same instance** already in the slot;
        /// `value.Apply(pDevice, samplerOffset + index)`; store.
        ///
        /// **The null check is unreachable here.** The writer's parameter type
        /// is the property's type, and `Item`'s recorded return verdict is
        /// `UNKNOWN_REFERENCE_NULLABILITY`, so the deferral rule makes it
        /// non-Optional — and a non-Optional `SamplerState` cannot be nil. The
        /// type system enforces what XNA enforces at run time, exactly as it
        /// does for `VertexElementValidator`'s usage-range check. Writing the
        /// branch as dead code would be worse than saying so; `NullNotAllowed`
        /// is still reproduced, by the three `GraphicsDevice` setters, whose
        /// parameters ARE Optional because their properties are proven
        /// nullable.
        public func SetItem(_ index: Int32, _ value: SamplerState) throws {
            let resolved = try checkedIndex(index)
            // Reference equality, which is what `beq` compares.
            if value === slots[resolved] { return }
            guard let device else {
                throw CNAError.producerInvariant(
                    "the sampler collection has no device to apply to")
            }
            let handle = try device.validatedHandle("SamplerStateCollection.Item")
            try value.attach(to: device)
            var native = value.nativeDescriptor()
            try device.runtimeState.functions.check(
                device.runtimeState.functions.graphicsDeviceSetSamplerState(
                    handle, stage, UInt32(resolved), &native),
                operation: "cna_graphics_device_set_sampler_state")
            slots[resolved] = value
        }

        private func checkedIndex(_ index: Int32) throws -> Int {
            guard index >= 0, Int(index) < slots.count else {
                throw CNAArgumentOutOfRangeException(paramName: "index")
            }
            return Int(index)
        }

        /// Rebinds the collection to the facade of the current callback. The
        /// collection outlives any one facade, so the device it applies
        /// through has to be refreshed rather than captured once.
        internal func rebind(to device: GraphicsDevice) { self.device = device }
    }
}
