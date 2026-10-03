// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.TextureCollection` projection.
    ///
    /// `sealed`, extends `System.Object`, and declares exactly one public
    /// member: the `Item[Int32]` indexer. Both accessors are recorded
    /// `IL_DIRECT_THROW`, so the projection is a throwing indexed reader plus a
    /// throwing `SetItem` writer method rather than a Swift `subscript` — the
    /// same shape `SamplerStateCollection` has, and for the same recorded
    /// reason.
    ///
    /// **The getter answers from a cache, and that is CNA's own prescription
    /// rather than a workaround.** `cna_graphics_device_get_texture` reports
    /// whether a slot is occupied and, when a C caller owns the texture, which
    /// handle is in it. What it cannot do is hand back an *object*: the header
    /// states, at length, that this ABI publishes no route from a native object
    /// to a handle anywhere, explains why neither way of adding one is worth
    /// what it buys, and names the remedy — *"cache what you bind and answer
    /// from the cache, and use `bound` to tell 'something else owns this slot
    /// now' from 'the slot is empty'."*
    ///
    /// XNA's own getter calls `Texture2D.GetManagedObject` / `TextureCube`'s /
    /// `Texture3D`'s, which is exactly such a native-to-managed map. So the
    /// cache is a reproduction of what XNA does, not an invention. `bound` is
    /// read too, and it is the case the cache cannot cover: a slot filled by
    /// canonical CNA code — a `SpriteBatch` flush — reports occupied with no
    /// handle, and this collection answers `nil` for it, because the object in
    /// that slot is not one any caller of this binding ever held.
    public final class TextureCollection {
        /// `_parent`. Weak, for `SamplerStateCollection`'s reason: the
        /// collection belongs to the game and a facade lives only as long as
        /// the callback that produced it.
        private weak var device: GraphicsDevice?

        /// `_textureOffset` — `0` for the pixel collection and `0x101` for the
        /// vertex one, which is `D3DVERTEXTEXTURESAMPLER0`. It is not a CNA
        /// slot index: CNA takes a stage and a zero-based slot, so the offset
        /// selects the stage here. Its **sign** is what `set_Item` tests to
        /// decide whether the vertex-texture format rule applies.
        private let textureOffset: Int32

        /// `CNA_SHADER_STAGE_PIXEL` / `CNA_SHADER_STAGE_VERTEX`.
        private let stage: UInt32

        /// What was bound through this collection. `nil` is both "empty" and
        /// "filled by something this binding did not do".
        private var slots: [Texture?]

        internal init(
            device: GraphicsDevice?, textureOffset: Int32, stage: UInt32, count: Int
        ) {
            self.device = device
            self.textureOffset = textureOffset
            self.stage = stage
            slots = Array(repeating: nil, count: count)
        }

        /// `_maxTextures`. Not an XNA member — `TextureCollection` declares no
        /// `Count` — so it is internal and exists for the bounds checks and the
        /// tests.
        internal var count: Int { slots.count }

        /// `TextureCollection.get_Item(int index)`.
        ///
        /// ```text
        /// Helpers.CheckDisposed(_parent, _parent->pComPtr);
        /// if (index < 0 || index >= _maxTextures)
        ///     throw new ArgumentOutOfRangeException("index");
        /// GetTexture(_textureOffset + index, &native);
        /// ... QueryInterface for IDirect3DTexture9 / CubeTexture9 /
        ///     VolumeTexture9 and hand back GetManagedObject(...), else null
        /// ```
        ///
        /// **Optional**, and proven so: the pinned verdict is
        /// `PROVEN_NULLABLE_SUCCESS` on `IL_RETURN_VALUE_ANALYSIS`, and the
        /// `ldnull; ret` at `IL_0165` is the empty slot.
        ///
        /// The device is asked as well as the cache, so a slot something else
        /// took over answers `nil` rather than the stale object this collection
        /// last put there.
        public func Item(_ index: Int32) throws -> Texture? {
            let resolved = try checkedIndex(index)
            guard let cached = slots[resolved] else { return nil }
            guard let device, let info = try? slotInfo(device, resolved) else {
                return cached
            }
            guard info.bound != 0 else {
                // The device says the slot is empty, so whatever this
                // collection last bound there is gone -- a destroyed texture
                // unbinds itself from every slot, measured in
                // build-probe/f66_slots.c. The cache is corrected rather than
                // trusted.
                slots[resolved] = nil
                return nil
            }
            guard info.texture != 0,
                  info.texture == (try? cached.validatedHandle("TextureCollection.Item")) else {
                // Occupied by something this binding did not bind: canonical
                // CNA code owns it and no handle names it. There is no object
                // to answer with, and answering the cached one would be a lie.
                return nil
            }
            return cached
        }

        /// `TextureCollection.set_Item(int index, Texture value)`.
        ///
        /// ```text
        /// Helpers.CheckDisposed(_parent, _parent->pComPtr);
        /// if (value != null) {
        ///     Helpers.CheckDisposed(value, value.GetComPtr());
        ///     if (value.isActiveRenderTarget)
        ///         throw new InvalidOperationException(MustResolveRenderTarget);
        ///     if (_textureOffset > 0 && !value.pStateTracker->validVertexTexture)
        ///         Throw(ProfileVertexTextureFormatNotSupported, value.Format);
        /// }
        /// if (index < 0 || index >= _maxTextures)
        ///     throw new ArgumentOutOfRangeException("index");
        /// ... SetTexture(_textureOffset + index, value)
        /// ```
        ///
        /// **The value is checked before the index**, which is the opposite of
        /// what a reader expects and is reproduced anyway: `SetItem(-1, t)` on
        /// an active render target reports `MustResolveRenderTarget`, not the
        /// index. `IL_0088` is the first bounds branch and it comes after both
        /// value tests.
        ///
        /// The vertex-texture rule reads a flag the state tracker cached at
        /// texture creation; the flag is `ValidVertexTextureFormats.Contains`,
        /// so that is what is written.
        ///
        /// **It is reachable on Reach, and it is what Reach reports.** Reach's
        /// `ValidVertexTextureFormats` is empty, so *every* texture fails the
        /// rule — and because the value is tested before the index, a Reach
        /// caller binding into the zero-slot vertex collection is told about
        /// the format, not about the slot. Only a `nil` value skips the value
        /// tests and reaches the bounds check.
        public func SetItem(_ index: Int32, _ value: Texture?) throws {
            if let value {
                _ = try value.validatedHandle("TextureCollection.Item")
                guard !value.isActiveRenderTarget else {
                    throw CNAInvalidOperationException(
                        message: TextureCollection.mustResolveRenderTargetMessage)
                }
                if textureOffset > 0 {
                    let capabilities = device?.profileCapabilities
                        ?? ProfileCapabilities.reach
                    guard capabilities.validVertexTextureFormats.contains(value.Format) else {
                        try capabilities.throwNotSupported(
                            ProfileCapabilities.profileVertexTextureFormatNotSupported,
                            "\(value.Format)")
                    }
                }
            }
            let resolved = try checkedIndex(index)
            guard let device else {
                throw CNAError.producerInvariant(
                    "the texture collection has no device to bind through")
            }
            let deviceHandle = try device.validatedHandle("TextureCollection.Item")
            let textureHandle = try value.map {
                try $0.validatedHandle("TextureCollection.Item")
            } ?? 0
            try device.runtimeState.functions.check(
                device.runtimeState.functions.graphicsDeviceSetTexture(
                    deviceHandle, stage, UInt32(resolved), textureHandle),
                operation: "cna_graphics_device_set_texture")
            slots[resolved] = value
        }

        /// `FrameworkResources.MustResolveRenderTarget`.
        internal static let mustResolveRenderTargetMessage =
            "The render target must not be set on the device when it is used "
            + "as a texture."

        private func checkedIndex(_ index: Int32) throws -> Int {
            guard index >= 0, Int(index) < slots.count else {
                throw CNAArgumentOutOfRangeException(paramName: "index")
            }
            return Int(index)
        }

        private func slotInfo(
            _ device: GraphicsDevice, _ slot: Int
        ) throws -> CNASwift_TextureSlotInfo {
            var info = CNASwift_TextureSlotInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_TextureSlotInfo>.size)
            info.struct_version = 1
            let handle = try device.validatedHandle("TextureCollection.Item")
            try device.runtimeState.functions.check(
                device.runtimeState.functions.graphicsDeviceGetTexture(
                    handle, stage, UInt32(slot), &info),
                operation: "cna_graphics_device_get_texture")
            return info
        }

        /// Whether this collection holds the given texture in any slot.
        ///
        /// `Texture2D.CopyData`, `TextureCube`'s and `Texture3D`'s each scan
        /// both device collections for `this` before a `SetData` and raise
        /// `ResourceInUse`. The scan is `bindings[i] == this` by reference, so
        /// the cache is exactly the right place to answer it.
        internal func holds(_ texture: Texture) -> Bool {
            slots.contains { $0 === texture }
        }

        /// Drops the given texture from every slot it occupies here.
        ///
        /// Not an XNA member. It exists because CNA unbinds a destroyed texture
        /// from every sampler slot on its own — measured, not assumed — so a
        /// disposed texture must leave the managed cache too or `Item` would
        /// keep answering a disposed object the device no longer holds.
        internal func forget(_ texture: Texture) {
            for index in slots.indices where slots[index] === texture {
                slots[index] = nil
            }
        }

        /// Rebinds to the facade of the current callback, for
        /// `SamplerStateCollection`'s reason.
        internal func rebind(to device: GraphicsDevice) { self.device = device }
    }
}
