// SPDX-License-Identifier: MIT

import CNAShim

/// The three `SpriteBatch` begin/end messages, read out of the embedded string
/// table of the registered `Microsoft.Xna.Framework.dll`. Each says "called
/// **successfully**": XNA's rule is about a begin that completed, which is why
/// the projected flag is set after the native call rather than before it.
internal let endMustBeCalledBeforeBeginMessage =
    "Begin cannot be called again until End has been successfully called."

internal let beginMustBeCalledBeforeEndMessage =
    "Begin must be called successfully before End can be called."

internal let beginMustBeCalledBeforeDrawMessage =
    "Begin must be called successfully before a Draw can be called."

extension Microsoft.Xna.Framework.Graphics {
    public enum SpriteSortMode: UInt32 {
        case Deferred = 0
        case Immediate = 1
        case Texture = 2
        case BackToFront = 3
        case FrontToBack = 4
    }

    public struct SpriteEffects: OptionSet {
        public let rawValue: UInt32
        public init(rawValue: UInt32) { self.rawValue = rawValue }

        public static let None = SpriteEffects([])
        public static let FlipHorizontally = SpriteEffects(rawValue: 1)
        public static let FlipVertically = SpriteEffects(rawValue: 2)
    }

    /// The `Microsoft.Xna.Framework.Graphics.SpriteBatch` projection.
    ///
    /// `open`, not `final`: XNA leaves it derivable, and its own
    /// `Dispose(bool)` override is the extension point a subclass would use.
    /// Its base is `GraphicsResource`, which is where the handle, the
    /// disposal, `IsDisposed`, `Name`, `Tag`, `GraphicsDevice` and `Disposing`
    /// now live.
    open class SpriteBatch: GraphicsResource {
        public init(graphicsDevice: GraphicsDevice) throws {
            let deviceHandle = try graphicsDevice.validatedHandle("SpriteBatch.init")
            let runtime = graphicsDevice.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.spriteBatchCreate(deviceHandle, &handle),
                operation: "cna_sprite_batch_create"
            )
            super.init(
                storage: NativeHandleStorage(
                    handle: handle,
                    typeName: "SpriteBatch",
                    ownership: .owned,
                    runtime: runtime,
                    destroy: runtime.functions.spriteBatchDestroy
                ),
                device: graphicsDevice
            )
            runtime.register(self)
        }

        /// `SpriteBatch.inBeginEndPair`, the managed flag the begin/end rule
        /// is written in terms of.
        ///
        /// CNA enforces the same sequence natively —
        /// `cna_sprite_batch_begin` answers `CNA_RESULT_INVALID_STATE` when an
        /// interval is already open, and `_end` and `_submit_many` do the same
        /// outside one — so this flag is not what *prevents* a bad sequence.
        /// It is what makes the failure XNA's: a caller who ends without
        /// beginning has broken an XNA rule and must see
        /// `InvalidOperationException` with XNA's message, not a
        /// `CNAError.nativeFailure` carrying a CNA result code. That is the
        /// error-channel line this binding draws — what caused the failure,
        /// not which layer noticed it.
        private var inBeginEndPair = false

        /// `SpriteBatch.Begin()`, which forwards to
        /// `Begin(SpriteSortMode.Deferred, null, null, null, null, null,
        /// Matrix.Identity)`.
        ///
        /// ```text
        /// if (inBeginEndPair)
        ///     throw new InvalidOperationException(EndMustBeCalledBeforeBegin);
        /// … store the six state fields …
        /// if (sortMode == Immediate) {
        ///     if (_parent.spriteBeginCount > 0)
        ///         throw new InvalidOperationException(CannotNextSpriteBeginImmediate);
        ///     SetRenderState(); _parent.spriteImmediateBeginCount++;
        /// }
        /// inBeginEndPair = true; _parent.spriteBeginCount++;
        /// ```
        ///
        /// The `Immediate` branch is unreachable through this projection: the
        /// parameterless overload is the only one implemented and it passes
        /// `Deferred`. `CannotNextSpriteBeginImmediate` and the two device
        /// counters that serve it are therefore not projected — recorded
        /// here, not written as code that cannot run.
        ///
        /// The flag is set only after the native begin succeeds, which is
        /// where XNA sets it too: everything that can fail in XNA's body
        /// happens before the assignment.
        /// The five fields `Begin` stores and `SetRenderState` reads. All four
        /// state fields are Optional because XNA's are null when the caller
        /// did not pass one, and "null" is not the same as "the default": the
        /// default is chosen later, in `SetRenderState`.
        private var spriteSortMode: SpriteSortMode = .Deferred
        private var blendState: BlendState?
        private var samplerState: SamplerState?
        private var depthStencilState: DepthStencilState?
        private var rasterizerState: RasterizerState?

        /// `SpriteBatch.spriteEffect` and `transformMatrix`, the two fields the
        /// seven-argument `Begin` stores that the five-argument one leaves at
        /// their defaults.
        private var spriteEffect: Effect?
        private var transformMatrix: Microsoft.Xna.Framework.Matrix = .Identity

        /// `SpriteBatch.Begin()`, which is
        /// `ldc.i4.0; ldnull x5; Matrix.Identity` -- Deferred and five nulls.
        public func Begin() throws {
            try Begin(.Deferred, blendState: nil, samplerState: nil,
                      depthStencilState: nil, rasterizerState: nil)
        }

        /// `Begin(SpriteSortMode sortMode, BlendState blendState)`, which
        /// forwards with three more nulls.
        public func Begin(
            _ sortMode: SpriteSortMode,
            blendState: BlendState?
        ) throws {
            try Begin(sortMode, blendState: blendState, samplerState: nil,
                      depthStencilState: nil, rasterizerState: nil)
        }

        /// `Begin(SpriteSortMode, BlendState, SamplerState, DepthStencilState,
        /// RasterizerState)`.
        ///
        /// The pair check comes first because XNA's does: `Begin` has no
        /// `Helpers.CheckDisposed` at all, and its very first instruction is
        /// `ldfld inBeginEndPair`. The order is observable --
        /// `Begin(); Dispose(); Begin()` raises XNA's rule violation, not a
        /// disposal error.
        ///
        /// XNA then stores the six state fields and, **only for `Immediate`**,
        /// calls `SetRenderState`. For every other sort mode the states are
        /// applied at `End`. That timing is reproduced rather than simplified:
        /// between `Begin` and `End`, a Deferred batch has not touched the
        /// device, and `GraphicsDevice.BlendState` still reads what it read
        /// before.
        ///
        /// CNA has to be told at begin time regardless, because
        /// `cna_sprite_batch_begin` **overwrites** the device's blend state
        /// with its own default: `build-probe/f54_beginstate.c` sets an
        /// unmistakable custom state, calls the plain begin, and reads a
        /// different one back. So the resolved descriptors cross the boundary
        /// through `cna_sprite_batch_begin_with_states`, and the *managed*
        /// device state moves at XNA's own moment. The two stay consistent
        /// because the managed device state is a cache on `RuntimeState`,
        /// which is what `GraphicsDevice.BlendState` reads.
        public func Begin(
            _ sortMode: SpriteSortMode,
            blendState: BlendState?,
            samplerState: SamplerState?,
            depthStencilState: DepthStencilState?,
            rasterizerState: RasterizerState?
        ) throws {
            try beginCore(sortMode, blendState, samplerState, depthStencilState,
                          rasterizerState, nil, .Identity)
        }

        /// `Begin(SpriteSortMode, BlendState, SamplerState, DepthStencilState,
        /// RasterizerState, Effect)`.
        ///
        /// XNA's body is twenty-one bytes: it loads its five arguments, pushes
        /// `Matrix.Identity`, and calls the seven-argument overload. **The
        /// identity matrix is XNA's default, not this binding's**, which is
        /// why the five-argument overload above passes it too.
        public func Begin(
            _ sortMode: SpriteSortMode,
            blendState: BlendState?,
            samplerState: SamplerState?,
            depthStencilState: DepthStencilState?,
            rasterizerState: RasterizerState?,
            effect: Effect
        ) throws {
            try beginCore(sortMode, blendState, samplerState, depthStencilState,
                          rasterizerState, effect, .Identity)
        }

        /// `Begin(..., Effect, Matrix)` — the one the other four forward to.
        ///
        /// The `effect` parameter is **not Optional**, because the registered
        /// CIL does not prove that XNA accepts null there and the projection is
        /// deliberately non-Optional until it does. The nulls the other
        /// overloads pass are internal.
        public func Begin(
            _ sortMode: SpriteSortMode,
            blendState: BlendState?,
            samplerState: SamplerState?,
            depthStencilState: DepthStencilState?,
            rasterizerState: RasterizerState?,
            effect: Effect,
            transformMatrix: Microsoft.Xna.Framework.Matrix
        ) throws {
            try beginCore(sortMode, blendState, samplerState, depthStencilState,
                          rasterizerState, effect, transformMatrix)
        }

        private func beginCore(
            _ sortMode: SpriteSortMode,
            _ blendState: BlendState?,
            _ samplerState: SamplerState?,
            _ depthStencilState: DepthStencilState?,
            _ rasterizerState: RasterizerState?,
            _ effect: Effect?,
            _ transformMatrix: Microsoft.Xna.Framework.Matrix
        ) throws {
            guard !inBeginEndPair else {
                throw CNAInvalidOperationException(
                    message: endMustBeCalledBeforeBeginMessage)
            }
            let handle = try validatedHandle("SpriteBatch.Begin")
            self.spriteSortMode = sortMode
            self.blendState = blendState
            self.samplerState = samplerState
            self.depthStencilState = depthStencilState
            self.rasterizerState = rasterizerState
            self.spriteEffect = effect
            self.transformMatrix = transformMatrix

            var blend = resolvedBlendState.nativeDescriptor()
            var sampler = resolvedSamplerState.nativeDescriptor()
            var depth = resolvedDepthStencilState.nativeDescriptor()
            var raster = resolvedRasterizerState.nativeDescriptor()
            // The effect-carrying route is taken only when there is something
            // for it to carry. CNA reads `CNA_INVALID_HANDLE` as "the default
            // sprite effect", so passing it would be equivalent -- but the
            // stateful route is the one Foundation 54's probe measured, and a
            // batch with neither an effect nor a transform keeps using it.
            if let effect, let effectHandle = try? effect.validatedHandle(
                "SpriteBatch.Begin(effect:)") {
                var matrix = Microsoft.Xna.Framework.Graphics.nativeMatrix(
                    from: transformMatrix)
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.spriteBatchBeginWithEffect(
                        handle, sortMode.rawValue, &blend, &sampler, &depth,
                        &raster, effectHandle, &matrix),
                    operation: "cna_sprite_batch_begin_with_effect"
                )
            } else {
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.spriteBatchBeginWithStates(
                        handle, sortMode.rawValue, &blend, &sampler, &depth, &raster),
                    operation: "cna_sprite_batch_begin_with_states"
                )
            }
            if sortMode == .Immediate {
                try setRenderState()
            }
            inBeginEndPair = true
        }

        /// The four `??` defaults, each an `ldsfld` of the preset in
        /// `SetRenderState`'s null branch: `BlendState.AlphaBlend`,
        /// `SamplerState.LinearClamp`, `DepthStencilState.None` and
        /// `RasterizerState.CullCounterClockwise`.
        private var resolvedBlendState: BlendState { blendState ?? .AlphaBlend }
        private var resolvedSamplerState: SamplerState { samplerState ?? .LinearClamp }
        private var resolvedDepthStencilState: DepthStencilState {
            depthStencilState ?? .None
        }
        private var resolvedRasterizerState: RasterizerState {
            rasterizerState ?? .CullCounterClockwise
        }

        /// `SpriteBatch.SetRenderState()`.
        ///
        /// ```text
        /// _parent.BlendState        = blendState        ?? BlendState.AlphaBlend
        /// _parent.DepthStencilState = depthStencilState ?? DepthStencilState.None
        /// _parent.RasterizerState   = rasterizerState   ?? RasterizerState.CullCounterClockwise
        /// _parent.SamplerStates[0]  = samplerState      ?? SamplerState.LinearClamp
        /// ```
        ///
        /// Through the projected device members rather than around them, so
        /// the state objects are bound and the device's cache moves exactly as
        /// if a caller had assigned them -- which is why a custom state passed
        /// to `Begin` is read-only afterwards, in XNA and here.
        ///
        /// The device is `GraphicsResource._parent`, Optional because no XNA
        /// constructor assigns it; a batch with no device applies nothing,
        /// which is the projection of a null `_parent` and not a new failure.
        ///
        /// The stored facade is **not** the one used. XNA's `_parent` is the
        /// device and lives as long as the batch; here a `GraphicsDevice` is a
        /// per-callback capability token, measured in
        /// `build-probe/f42b_identity.c`, so a batch created in `LoadContent`
        /// holds a token that is stale by the time `End` runs in `Draw`. The
        /// first version of this applied state through the stored facade and
        /// the sixty-frame canary failed with "requires an active Game
        /// lifecycle callback" -- correctly. A current token is borrowed
        /// instead: there is one device per game, so it is the same device
        /// XNA's `_parent` would have been.
        private func setRenderState() throws {
            guard GraphicsDevice != nil else { return }
            let device = try Microsoft.Xna.Framework.Graphics.GraphicsDevice
                .borrow(from: nativeStorage.runtime)
            try device.SetBlendState(resolvedBlendState)
            try device.SetDepthStencilState(resolvedDepthStencilState)
            try device.SetRasterizerState(resolvedRasterizerState)
            try device.SamplerStates?.SetItem(0, resolvedSamplerState)
        }

        public func Draw(
            _ texture: Texture2D,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try Draw(
                texture,
                position: position,
                sourceRectangle: nil,
                color: color,
                rotation: 0,
                origin: .Zero,
                scale: 1,
                effects: .None,
                layerDepth: 0
            )
        }

        /// `Draw(Texture2D, Vector2, Nullable<Rectangle>, Color)`.
        ///
        /// Every `Draw` overload builds a `Vector4` and calls `InternalDraw`
        /// with a `scaleDestination` flag:
        ///
        /// ```text
        /// position family:    (X, Y, scaleX, scaleY)   scaleDestination = 1
        /// destination family: (X, Y, Width,  Height)   scaleDestination = 0
        /// ```
        ///
        /// That flag is the whole difference between the two families, and CNA
        /// splits its commands along the same line: `CNA_SpriteScaledCommand`
        /// carries position and scale, `CNA_SpriteCommand` carries a
        /// destination rectangle. So each family maps onto its own command
        /// with no arithmetic invented in between — no dividing a destination
        /// rectangle by a source size to fabricate a scale.
        ///
        /// This overload's own literals are `ldc.r4 1` twice for the scale,
        /// `ldc.r4 0.0` for the rotation and depth, `vector2Zero` for the
        /// origin and `ldc.i4.0` for the effects.
        public func Draw(
            _ texture: Texture2D,
            position: Microsoft.Xna.Framework.Vector2,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try Draw(
                texture, position: position, sourceRectangle: sourceRectangle,
                color: color, rotation: 0, origin: .Zero,
                scale: Microsoft.Xna.Framework.Vector2(1, 1),
                effects: .None, layerDepth: 0)
        }

        /// `Draw(Texture2D, Vector2, Nullable<Rectangle>, Color, Single,
        /// Vector2, Vector2, SpriteEffects, Single)` — the per-axis scale.
        public func Draw(
            _ texture: Texture2D,
            position: Microsoft.Xna.Framework.Vector2,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try submitScaled(
                texture, position: position, sourceRectangle: sourceRectangle,
                color: color, rotation: rotation, origin: origin, scale: scale,
                effects: effects, layerDepth: layerDepth)
        }

        /// `Draw(Texture2D, Rectangle, Color)`.
        public func Draw(
            _ texture: Texture2D,
            destinationRectangle: Microsoft.Xna.Framework.Rectangle,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try submitDestination(
                texture, destinationRectangle: destinationRectangle,
                sourceRectangle: nil, color: color, rotation: 0,
                origin: .Zero, effects: .None, layerDepth: 0)
        }

        /// `Draw(Texture2D, Rectangle, Nullable<Rectangle>, Color)`.
        public func Draw(
            _ texture: Texture2D,
            destinationRectangle: Microsoft.Xna.Framework.Rectangle,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try submitDestination(
                texture, destinationRectangle: destinationRectangle,
                sourceRectangle: sourceRectangle, color: color, rotation: 0,
                origin: .Zero, effects: .None, layerDepth: 0)
        }

        /// `Draw(Texture2D, Rectangle, Nullable<Rectangle>, Color, Single,
        /// Vector2, SpriteEffects, Single)`.
        ///
        /// The destination family has **no scale parameter at all** — the
        /// rectangle is the size — which is why it is eight parameters where
        /// the position family is nine.
        public func Draw(
            _ texture: Texture2D,
            destinationRectangle: Microsoft.Xna.Framework.Rectangle,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try submitDestination(
                texture, destinationRectangle: destinationRectangle,
                sourceRectangle: sourceRectangle, color: color,
                rotation: rotation, origin: origin, effects: effects,
                layerDepth: layerDepth)
        }

        /// `Draw(Texture2D, Vector2, Nullable<Rectangle>, Color, Single,
        /// Vector2, Single, SpriteEffects, Single)` — the uniform scale, which
        /// XNA writes into both components of the `Vector4`.
        public func Draw(
            _ texture: Texture2D,
            position: Microsoft.Xna.Framework.Vector2,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Float,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try submitScaled(
                texture, position: position, sourceRectangle: sourceRectangle,
                color: color, rotation: rotation, origin: origin,
                scale: Microsoft.Xna.Framework.Vector2(scale, scale),
                effects: effects, layerDepth: layerDepth)
        }

        private func submitScaled(
            _ texture: Texture2D,
            position: Microsoft.Xna.Framework.Vector2,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            // `SpriteBatch.InternalDraw`, which every public `Draw` overload
            // funnels through, opens with two checks and nothing else:
            //
            //     if (texture == null)
            //         throw new ArgumentNullException("texture", NullNotAllowed);
            //     if (!inBeginEndPair)
            //         throw new InvalidOperationException(BeginMustBeCalledBeforeDraw);
            //
            // The first is unreachable here — `texture` is a non-Optional
            // class parameter — and is recorded rather than written. The
            // second is therefore the first reachable check, and it comes
            // before the handles are resolved: XNA has no `CheckDisposed`
            // anywhere in this path, so a disposed batch outside a pair
            // reports the rule it broke, not its disposal.
            guard inBeginEndPair else {
                throw CNAInvalidOperationException(
                    message: beginMustBeCalledBeforeDrawMessage)
            }
            let batchHandle = try validatedHandle("SpriteBatch.Draw")
            let textureHandle = try texture.validatedHandle("SpriteBatch.Draw texture")
            guard texture.runtimeState === nativeStorage.runtime else {
                throw CNAError.staleRuntimeGeneration(expected: nativeStorage.generation, actual: texture.runtimeState.generation)
            }
            var command = CNASwift_SpriteScaledCommand()
            command.struct_size = UInt32(MemoryLayout<CNASwift_SpriteScaledCommand>.size)
            command.struct_version = 1
            command.texture = textureHandle
            command.position = CNASwift_Vector2(x: position.X, y: position.Y)
            if let sourceRectangle {
                command.source = CNASwift_Rectangle(
                    x: sourceRectangle.X,
                    y: sourceRectangle.Y,
                    width: sourceRectangle.Width,
                    height: sourceRectangle.Height
                )
            }
            command.color = color.native
            command.rotation = rotation
            command.origin = CNASwift_Vector2(x: origin.X, y: origin.Y)
            command.scale = CNASwift_Vector2(x: scale.X, y: scale.Y)
            command.effects = effects.rawValue
            command.layer_depth = layerDepth
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.spriteBatchSubmitScaled(batchHandle, &command, 1),
                operation: "cna_sprite_batch_submit_scaled_many"
            )
        }

        /// The `scaleDestination = 0` family, on CNA's own destination-rectangle
        /// command. The same two checks in the same order as the scaled funnel.
        private func submitDestination(
            _ texture: Texture2D,
            destinationRectangle: Microsoft.Xna.Framework.Rectangle,
            sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            guard inBeginEndPair else {
                throw CNAInvalidOperationException(
                    message: beginMustBeCalledBeforeDrawMessage)
            }
            let batchHandle = try validatedHandle("SpriteBatch.Draw")
            let textureHandle = try texture.validatedHandle("SpriteBatch.Draw texture")
            guard texture.runtimeState === nativeStorage.runtime else {
                throw CNAError.staleRuntimeGeneration(
                    expected: nativeStorage.generation,
                    actual: texture.runtimeState.generation)
            }
            var command = CNASwift_SpriteCommand()
            command.struct_size = UInt32(MemoryLayout<CNASwift_SpriteCommand>.size)
            command.struct_version = 1
            command.texture = textureHandle
            command.destination = CNASwift_Rectangle(
                x: destinationRectangle.X, y: destinationRectangle.Y,
                width: destinationRectangle.Width, height: destinationRectangle.Height)
            // A zero-by-zero source is CNA's own spelling of "the whole
            // texture", which is what an absent Nullable<Rectangle> means to
            // the canonical call -- its header says so. An XNA caller who
            // passes a *present* rectangle of zero size means something else,
            // and CNA cannot be told the difference through this command; that
            // divergence is recorded rather than papered over.
            if let sourceRectangle {
                command.source = CNASwift_Rectangle(
                    x: sourceRectangle.X, y: sourceRectangle.Y,
                    width: sourceRectangle.Width, height: sourceRectangle.Height)
            }
            command.color = color.native
            command.rotation = rotation
            command.origin = CNASwift_Vector2(x: origin.X, y: origin.Y)
            command.effects = effects.rawValue
            command.layer_depth = layerDepth
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.spriteBatchSubmit(batchHandle, &command, 1),
                operation: "cna_sprite_batch_submit_many"
            )
        }

        /// `SpriteBatch.End()`.
        ///
        /// ```text
        /// if (!inBeginEndPair)
        ///     throw new InvalidOperationException(BeginMustBeCalledBeforeEnd);
        /// if (spriteSortMode != Immediate) SetRenderState();
        /// else _parent.spriteImmediateBeginCount--;
        /// if (spriteQueueCount > 0) Flush();
        /// inBeginEndPair = false;
        /// _parent.spriteBeginCount--;
        /// ```
        ///
        /// XNA clears the flag *after* the flush, so a failing flush leaves
        /// the pair open; the native end is this projection's flush, and the
        /// flag is cleared after it succeeds for the same reason.
        public func End() throws {
            // Same order as `Begin`, and for the same reason: `End`'s first
            // instruction is the flag test, not a disposal check.
            guard inBeginEndPair else {
                throw CNAInvalidOperationException(
                    message: beginMustBeCalledBeforeEndMessage)
            }
            let handle = try validatedHandle("SpriteBatch.End")
            // `if (spriteSortMode != Immediate) SetRenderState();` -- the
            // states a Deferred batch was given reach the device here, not at
            // Begin. An Immediate batch applied them already and decrements
            // the device's immediate counter instead, which this projection
            // does not keep: that counter's only reader is the
            // `CannotNextSpriteBeginImmediate` guard, on a branch no projected
            // overload can reach.
            if spriteSortMode != .Immediate {
                try setRenderState()
            }
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.spriteBatchEnd(handle),
                operation: "cna_sprite_batch_end"
            )
            inBeginEndPair = false
        }

        /// `protected override void Dispose(bool)`.
        ///
        /// ```text
        /// try {
        ///     if (disposing && !IsDisposed) {
        ///         if (spriteEffect != null) spriteEffect.Dispose();
        ///         DisposePlatformData();      // vertexBuffer?.Dispose();
        ///                                     // indexBuffer?.Dispose();
        ///     }
        /// } finally { base.Dispose(disposing); }
        /// ```
        ///
        /// All three objects it disposes are XNA's own internals — the sprite
        /// shader it compiles at construction, and the `DynamicVertexBuffer`
        /// and `DynamicIndexBuffer` it batches through. None is reachable from
        /// the public surface, and here all three are inside CNA's sprite
        /// batch, which `cna_sprite_batch_destroy` releases with it. The
        /// disposing-only guard around them therefore has nothing to guard,
        /// and what is left is the `finally`: the base call, with the flag
        /// unchanged, which is where the disposal actually happens.
        ///
        /// The override exists because XNA declares it. It is `open` for the
        /// same reason the base is: `protected virtual` maps to `open`, and a
        /// consumer's subclass overriding it must reach this link of the chain.
        // MARK: - DrawString

        /// `DrawString(SpriteFont, String, Vector2, Color)`.
        public func DrawString(
            _ spriteFont: SpriteFont,
            text: String,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try DrawString(
                spriteFont, text: text, position: position, color: color,
                rotation: 0, origin: .Zero,
                scale: Microsoft.Xna.Framework.Vector2(1, 1),
                effects: .None, layerDepth: 0)
        }

        /// `DrawString(SpriteFont, String, Vector2, Color, Single, Vector2,
        /// Single, SpriteEffects, Single)` — the uniform-scale overload, whose
        /// IL widens the single scale into `new Vector2(scale, scale)` before
        /// calling the same `SpriteFont.InternalDraw`.
        public func DrawString(
            _ spriteFont: SpriteFont,
            text: String,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Float,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try DrawString(
                spriteFont, text: text, position: position, color: color,
                rotation: rotation, origin: origin,
                scale: Microsoft.Xna.Framework.Vector2(scale, scale),
                effects: effects, layerDepth: layerDepth)
        }

        /// `DrawString(SpriteFont, String, Vector2, Color, Single, Vector2,
        /// Vector2, SpriteEffects, Single)` — the per-axis scale, and the one
        /// the other two funnel into.
        ///
        /// **The begin/end check is conditional on the text, and that is not a
        /// simplification.** XNA does not test the pair here at all:
        /// `DrawString` checks its two arguments for null and calls
        /// `SpriteFont.InternalDraw`, which loops over the characters and
        /// calls `SpriteBatch.InternalDraw` once per glyph — and it is
        /// *there* that `BeginMustBeCalledBeforeDraw` is raised. An empty
        /// string therefore draws no glyph, raises nothing, and succeeds
        /// outside a begin/end pair.
        ///
        /// CNA's `cna_sprite_batch_draw_string` answers `INVALID_STATE`
        /// outside an interval whatever the text, so forwarding an empty
        /// string would refuse a call XNA accepts. The managed test reproduces
        /// XNA's condition exactly: the pair is required only when the text
        /// would produce a glyph.
        ///
        /// A string of nothing but carriage returns and newlines is the
        /// interesting case, and it goes the other way: XNA's loop *does*
        /// iterate, but `'\r'` is skipped and `'\n'` only advances the line,
        /// so neither reaches `InternalDraw` and neither raises. The
        /// condition is "produces a glyph", not "is non-empty".
        public func DrawString(
            _ spriteFont: SpriteFont,
            text: String,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try drawText(spriteFont, units: Array(text.utf16),
                         position: position, color: color,
                         rotation: rotation, origin: origin, scale: scale,
                         effects: effects, layerDepth: layerDepth)
        }

        /// `DrawString(SpriteFont, StringBuilder, …)`, all three overloads.
        ///
        /// XNA declares three, each one line over a `StringProxy`; they are
        /// three here for the same reason and share the same body. The
        /// buffer's code units are read where the `String` overloads read
        /// theirs, so a caller holding a builder is not made to materialise
        /// a string first.
        public func DrawString(
            _ spriteFont: SpriteFont,
            text: CNAStringBuilder,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color
        ) throws {
            try DrawString(
                spriteFont, text: text, position: position, color: color,
                rotation: 0, origin: .Zero,
                scale: Microsoft.Xna.Framework.Vector2(1, 1),
                effects: .None, layerDepth: 0)
        }

        public func DrawString(
            _ spriteFont: SpriteFont,
            text: CNAStringBuilder,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Float,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try DrawString(
                spriteFont, text: text, position: position, color: color,
                rotation: rotation, origin: origin,
                scale: Microsoft.Xna.Framework.Vector2(scale, scale),
                effects: effects, layerDepth: layerDepth)
        }

        public func DrawString(
            _ spriteFont: SpriteFont,
            text: CNAStringBuilder,
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            try drawText(spriteFont, units: text.codeUnits,
                         position: position, color: color,
                         rotation: rotation, origin: origin, scale: scale,
                         effects: effects, layerDepth: layerDepth)
        }

        /// What both families funnel into, over the code units each one
        /// already holds.
        private func drawText(
            _ spriteFont: SpriteFont,
            units: [UInt16],
            position: Microsoft.Xna.Framework.Vector2,
            color: Microsoft.Xna.Framework.Color,
            rotation: Float,
            origin: Microsoft.Xna.Framework.Vector2,
            scale: Microsoft.Xna.Framework.Vector2,
            effects: SpriteEffects,
            layerDepth: Float
        ) throws {
            let drawsAGlyph = units.contains { $0 != 13 && $0 != 10 }
            guard drawsAGlyph else { return }
            guard inBeginEndPair else {
                throw CNAInvalidOperationException(
                    message: beginMustBeCalledBeforeDrawMessage)
            }
            // Every character has to be in the font before anything is
            // submitted, because XNA's loop raises from the glyph it reaches
            // and CNA's route would refuse the whole string with a message of
            // its own. Measuring is the cheapest way to ask, and it asks the
            // same `GetIndexForCharacter` a draw would.
            _ = try spriteFont.measure(units)

            let batchHandle = try validatedHandle("SpriteBatch.DrawString")
            let fontHandle = try spriteFont.validatedHandleForDraw()
            guard spriteFont.box.runtime === nativeStorage.runtime else {
                throw CNAError.staleRuntimeGeneration(
                    expected: nativeStorage.generation,
                    actual: spriteFont.box.runtime.generation)
            }
            var utf8 = Array(String(decoding: units, as: UTF16.self).utf8)
            try utf8.withUnsafeMutableBufferPointer { buffer in
                var command = CNASwift_SpriteTextCommand()
                command.struct_size =
                    UInt32(MemoryLayout<CNASwift_SpriteTextCommand>.size)
                command.struct_version = 1
                command.sprite_font = fontHandle
                command.text = CNASwift_StringView(
                    data: UnsafeRawPointer(buffer.baseAddress)?
                        .assumingMemoryBound(to: CChar.self),
                    byte_length: UInt64(buffer.count))
                command.position = CNASwift_Vector2(x: position.X, y: position.Y)
                command.color = color.native
                command.rotation = rotation
                command.origin = CNASwift_Vector2(x: origin.X, y: origin.Y)
                command.scale = CNASwift_Vector2(x: scale.X, y: scale.Y)
                command.effects = effects.rawValue
                command.layer_depth = layerDepth
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.spriteBatchDrawString(
                        batchHandle, &command),
                    operation: "cna_sprite_batch_draw_string")
            }
        }

        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
