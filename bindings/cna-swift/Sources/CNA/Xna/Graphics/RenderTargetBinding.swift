// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.RenderTargetBinding` projection.
    ///
    /// Two fields and two getters, and both constructors do exactly one thing
    /// before storing them:
    ///
    /// ```text
    /// .ctor(RenderTargetCube renderTarget, CubeMapFace cubeMapFace)
    ///     if (renderTarget == null)
    ///         throw new ArgumentNullException("renderTarget", NullNotAllowed);
    ///     _renderTarget = renderTarget;  _cubeMapFace = cubeMapFace;
    ///
    /// .ctor(RenderTarget2D renderTarget)
    ///     if (renderTarget == null)
    ///         throw new ArgumentNullException("renderTarget", NullNotAllowed);
    ///     _renderTarget = renderTarget;  _cubeMapFace = 0;    // PositiveX
    /// ```
    ///
    /// `_renderTarget` is typed **`Texture`**, the common base of the two, and
    /// `RenderTarget` hands that back — so the property does not say which kind
    /// it holds and a consumer must ask. That is XNA's own shape and it is kept:
    /// narrowing the property to a union type this projection invented would be
    /// a different API.
    ///
    /// Both parameters are non-Optional Swift class parameters, so neither
    /// null branch is reachable through the projected surface; both are
    /// recorded in `recorded-message-absences.json` under the rule that an
    /// unreachable branch is recorded rather than written as code that cannot
    /// run.
    ///
    /// A `struct`, because XNA's is a `ValueType` — and the whole reason the
    /// managed cache behind `GetRenderTargets` must copy rather than share.
    public struct RenderTargetBinding {
        private let storedRenderTarget: Texture
        private let storedCubeMapFace: CubeMapFace

        /// `RenderTargetBinding(RenderTargetCube renderTarget, CubeMapFace cubeMapFace)`.
        public init(_ renderTarget: RenderTargetCube, _ cubeMapFace: CubeMapFace) {
            storedRenderTarget = renderTarget
            storedCubeMapFace = cubeMapFace
        }

        /// `RenderTargetBinding(RenderTarget2D renderTarget)`.
        ///
        /// The face is stored as `PositiveX`, which is `ldc.i4.0` in the IL —
        /// not "unset". A 2D target has no face, and zero is the value the
        /// field carries when it means nothing; CNA's own header says the same
        /// of `CNA_RenderTargetBinding::cube_map_face`, and refuses any other
        /// value for a 2D target (`build-probe/f65_rtcube.c`).
        public init(_ renderTarget: RenderTarget2D) {
            storedRenderTarget = renderTarget
            storedCubeMapFace = .PositiveX
        }

        /// `RenderTargetBinding.RenderTarget`.
        public var RenderTarget: Texture { storedRenderTarget }

        /// `RenderTargetBinding.CubeMapFace`.
        public var CubeMapFace: Microsoft.Xna.Framework.Graphics.CubeMapFace {
            storedCubeMapFace
        }

        /// `public static implicit operator RenderTargetBinding(RenderTarget2D)`.
        ///
        /// Ten bytes of IL that call the one-argument constructor. Swift has no
        /// implicit user conversion, so this is the named static the projection
        /// rules give an `op_Implicit`.
        public static func op_Implicit(_ renderTarget: RenderTarget2D)
            -> RenderTargetBinding {
            RenderTargetBinding(renderTarget)
        }
    }
}
