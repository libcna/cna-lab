// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The internal `Microsoft.Xna.Framework.Graphics.ProfileCapabilities`.
    ///
    /// Not an XNA public type and not projected as one: it is the per-profile
    /// limit table nine of this binding's messages compare against, and it is
    /// `internal` for exactly the reason XNA's is `private`.
    ///
    /// **Every value below is extracted, not transcribed.**
    /// `tools/api_compat/profile_capabilities.py` walks the class constructor
    /// of the hash-registered `Microsoft.Xna.Framework.Graphics.dll`, reads each
    /// `ldc`/`stfld` pair and each `List<T>.Add`, and writes
    /// `reference/xna40-profile-capabilities.json`; `--check` re-extracts and
    /// fails if the pinned file has drifted, and
    /// `Foundation62ProfileCapabilityTests` compares this table against that
    /// file field for field. A table of thirty-two constants copied by hand is
    /// exactly the thing that is wrong in one place and never noticed.
    internal struct ProfileCapabilities {
        internal let profile: GraphicsProfile
        internal let vertexShaderVersion: Int32
        internal let pixelShaderVersion: Int32
        internal let maxPrimitiveCount: Int32
        internal let maxVertexStreams: Int32
        internal let maxStreamStride: Int32
        internal let maxVertexBufferSize: Int32
        internal let maxIndexBufferSize: Int32
        internal let maxTextureSize: Int32
        internal let maxCubeSize: Int32
        internal let maxVolumeExtent: Int32
        internal let maxTextureAspectRatio: Int32
        internal let maxSamplers: Int32
        internal let maxVertexSamplers: Int32
        internal let maxRenderTargets: Int32
        internal let occlusionQuery: Bool
        internal let getBackBufferData: Bool
        internal let separateAlphaBlend: Bool
        internal let destBlendSrcAlphaSat: Bool
        internal let minMaxSrcDestBlend: Bool
        internal let indexElementSize32: Bool
        internal let nonPow2Unconditional: Bool
        internal let nonPow2Cube: Bool
        internal let nonPow2Volume: Bool
        internal let validTextureFormats: [SurfaceFormat]
        internal let validCubeFormats: [SurfaceFormat]
        internal let validVolumeFormats: [SurfaceFormat]
        internal let validVertexTextureFormats: [SurfaceFormat]
        internal let invalidFilterFormats: [SurfaceFormat]
        internal let invalidBlendFormats: [SurfaceFormat]
        internal let validDepthFormats: [DepthFormat]
        internal let validVertexFormats: [VertexElementFormat]

        // ------------------------------------------------------------------
        // The messages, and the one method that raises them.

        /// `FrameworkResources.ProfileTooBig`.
        internal static let profileTooBig =
            "XNA Framework {0} profile supports a maximum {1} size of {2}."
        /// `FrameworkResources.ProfileFormatNotSupported`.
        internal static let profileFormatNotSupported =
            "XNA Framework {0} profile does not support {1} format {2}."
        /// `FrameworkResources.ProfileAspectRatio`.
        internal static let profileAspectRatio =
            "XNA Framework {0} profile supports a maximum {1} aspect ratio of {2}."
        /// `FrameworkResources.ProfileNotPowerOfTwoMipped`.
        internal static let profileNotPowerOfTwoMipped =
            "XNA Framework {0} profile requires mipmapped {1} sizes to be powers "
            + "of two. To use a non power of two {1}, remove the mipmaps."
        /// `FrameworkResources.ProfileNotPowerOfTwoDXT`.
        internal static let profileNotPowerOfTwoDXT =
            "XNA Framework {0} profile requires DXT compressed {1} sizes to be "
            + "powers of two. To use a non power of two {1}, disable DXT compression."
        /// `FrameworkResources.ProfileNoIndexElementSize32`.
        internal static let profileNoIndexElementSize32 =
            "XNA Framework {0} profile does not support 32 bit indices. Use "
            + "IndexElementSize.SixteenBits or a type that has a size of two bytes."
        /// `FrameworkResources.DxtNotMultipleOfFour`.
        internal static let dxtNotMultipleOfFour =
            "DXT compressed texture sizes must be multiples of four."
        /// `FrameworkResources.ProfileMaxVertexStreams`.
        internal static let profileMaxVertexStreams =
            "XNA Framework {0} profile supports a maximum of {1} simultaneous "
            + "vertex buffers."
        /// `FrameworkResources.ProfileVertexTextureFormatNotSupported`.
        internal static let profileVertexTextureFormatNotSupported =
            "XNA Framework {0} profile does not support vertex texture format {1}."
        /// `FrameworkResources.ProfileMaxRenderTargets`.
        internal static let profileMaxRenderTargets =
            "XNA Framework {0} profile supports a maximum of {1} simultaneous "
            + "rendertargets."
        /// `FrameworkResources.ProfileMaxPrimitiveCount`.
        internal static let profileMaxPrimitiveCount =
            "XNA Framework {0} profile supports a maximum of {1} primitives per "
            + "draw call."

        /// `ProfileCapabilities.ThrowNotSupportedException(String, params)`.
        ///
        /// All three overloads build the same argument array with the **profile
        /// itself as `{0}`** and the caller's arguments after it, format it
        /// against the current culture, and throw `NotSupportedException`. A
        /// message that named only the limit and not the profile would be a
        /// different message.
        internal func throwNotSupported(
            _ template: String, _ arguments: String...
        ) throws -> Never {
            var message = template.replacingOccurrences(
                of: "{0}", with: "\(profile)")
            for (index, argument) in arguments.enumerated() {
                message = message.replacingOccurrences(
                    of: "{\(index + 1)}", with: argument)
            }
            throw CNANotSupportedException(message: message)
        }

        // ------------------------------------------------------------------
        // The three validations nine deferred messages were waiting for.

        /// `Texture2D.ValidateCreationParameters(ProfileCapabilities, Int32,
        /// Int32, SurfaceFormat, Boolean)`, after the two dimension checks the
        /// constructor already performs.
        ///
        /// ```text
        /// bool compressed = CheckCompressedTexture(format);
        /// if (!ValidTextureFormats.Contains(format))
        ///     Throw(ProfileFormatNotSupported, "Texture2D", format);
        /// if (width > MaxTextureSize || height > MaxTextureSize)
        ///     Throw(ProfileTooBig, "Texture2D", MaxTextureSize);
        /// if ((Max(w,h) + Min(w,h) - 1) / Min(w,h) > MaxTextureAspectRatio)
        ///     Throw(ProfileAspectRatio, "Texture2D", MaxTextureAspectRatio);
        /// if (!NonPow2Unconditional && !(IsPowerOfTwo(w) && IsPowerOfTwo(h))) {
        ///     if (mipMap)    Throw(ProfileNotPowerOfTwoMipped, "Texture2D");
        ///     if (compressed) Throw(ProfileNotPowerOfTwoDXT,    "Texture2D");
        /// }
        /// if (compressed && ((w & 3) != 0 || (h & 3) != 0))
        ///     throw new ArgumentException(DxtNotMultipleOfFour);
        /// ```
        ///
        /// The aspect-ratio arithmetic is XNA's own and is transcribed
        /// literally: it is a **ceiling** division of the longer side by the
        /// shorter, so a 2048x1 texture has ratio 2048 and passes Reach's limit
        /// of exactly 2048.
        internal func validateTextureCreation(
            width: Int32, height: Int32,
            format: SurfaceFormat, mipMap: Bool
        ) throws {
            let compressed = ProfileCapabilities.isCompressed(format)
            guard validTextureFormats.contains(format) else {
                try throwNotSupported(
                    ProfileCapabilities.profileFormatNotSupported,
                    "Texture2D", "\(format)")
            }
            guard width <= maxTextureSize, height <= maxTextureSize else {
                try throwNotSupported(
                    ProfileCapabilities.profileTooBig,
                    "Texture2D", "\(maxTextureSize)")
            }
            let longer = max(width, height), shorter = min(width, height)
            guard (longer + shorter - 1) / shorter <= maxTextureAspectRatio else {
                try throwNotSupported(
                    ProfileCapabilities.profileAspectRatio,
                    "Texture2D", "\(maxTextureAspectRatio)")
            }
            if !nonPow2Unconditional,
               !(ProfileCapabilities.isPowerOfTwo(width)
                 && ProfileCapabilities.isPowerOfTwo(height)) {
                if mipMap {
                    try throwNotSupported(
                        ProfileCapabilities.profileNotPowerOfTwoMipped, "Texture2D")
                }
                if compressed {
                    try throwNotSupported(
                        ProfileCapabilities.profileNotPowerOfTwoDXT, "Texture2D")
                }
            }
            if compressed, width & 3 != 0 || height & 3 != 0 {
                throw CNAArgumentException(
                    message: ProfileCapabilities.dxtNotMultipleOfFour)
            }
        }

        /// `VertexBuffer.CreateBuffer`'s one profile check.
        ///
        /// `size = declaration.VertexStride * vertexCount`, compared **unsigned**
        /// against `MaxVertexBufferSize`.
        internal func validateVertexBufferSize(_ size: Int) throws {
            guard UInt32(truncatingIfNeeded: size)
                <= UInt32(bitPattern: maxVertexBufferSize) else {
                try throwNotSupported(
                    ProfileCapabilities.profileTooBig,
                    "VertexBuffer", "\(maxVertexBufferSize)")
            }
        }

        /// `IndexBuffer.CreateBuffer`'s two profile checks, in the IL's order:
        /// the 32-bit refusal comes **before** the size comparison.
        internal func validateIndexBuffer(elementSizeInBytes: Int, size: Int) throws {
            if elementSizeInBytes == 4, !indexElementSize32 {
                try throwNotSupported(
                    ProfileCapabilities.profileNoIndexElementSize32)
            }
            guard UInt32(truncatingIfNeeded: size)
                <= UInt32(bitPattern: maxIndexBufferSize) else {
                try throwNotSupported(
                    ProfileCapabilities.profileTooBig,
                    "IndexBuffer", "\(maxIndexBufferSize)")
            }
        }

        /// `FrameworkResources.ProfileNotPowerOfTwo`.
        internal static let profileNotPowerOfTwo =
            "XNA Framework {0} profile requires {1} sizes to be powers of two."
        /// `FrameworkResources.ProfileFeatureNotSupported`.
        internal static let profileFeatureNotSupported =
            "XNA Framework {0} profile does not support {1}."

        /// `TextureCube.ValidateCreationParameters`.
        ///
        /// ```text
        /// if (size <= 0) throw new ArgumentOutOfRangeException(
        ///     "size", ResourcesMustBeGreaterThanZeroSize);
        /// bool compressed = CheckCompressedTexture(format);
        /// if (!ValidCubeFormats.Contains(format))
        ///     Throw(ProfileFormatNotSupported, "TextureCube", format);
        /// if (size > MaxCubeSize)  Throw(ProfileTooBig, "TextureCube", MaxCubeSize);
        /// if (!NonPow2Cube && !IsPowerOfTwo(size))
        ///     Throw(ProfileNotPowerOfTwo, "TextureCube");
        /// if (compressed && (size & 3) != 0)
        ///     throw new ArgumentException(DxtNotMultipleOfFour);
        /// ```
        ///
        /// The cube has **no aspect-ratio check** — a cube face is square by
        /// construction — and its power-of-two message is
        /// `ProfileNotPowerOfTwo`, not `Texture2D`'s mipmap-specific one.
        internal func validateCubeCreation(
            size: Int32, format: SurfaceFormat
        ) throws {
            guard size > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "size",
                    message: Microsoft.Xna.Framework.Graphics.Texture2D
                        .resourcesMustBeGreaterThanZeroSizeMessage)
            }
            let compressed = ProfileCapabilities.isCompressed(format)
            guard validCubeFormats.contains(format) else {
                try throwNotSupported(
                    ProfileCapabilities.profileFormatNotSupported,
                    "TextureCube", "\(format)")
            }
            guard size <= maxCubeSize else {
                try throwNotSupported(
                    ProfileCapabilities.profileTooBig, "TextureCube", "\(maxCubeSize)")
            }
            if !nonPow2Cube, !ProfileCapabilities.isPowerOfTwo(size) {
                try throwNotSupported(
                    ProfileCapabilities.profileNotPowerOfTwo, "TextureCube")
            }
            if compressed, size & 3 != 0 {
                throw CNAArgumentException(
                    message: ProfileCapabilities.dxtNotMultipleOfFour)
            }
        }

        /// `Texture3D.ValidateCreationParameters`.
        ///
        /// ```text
        /// if (width  <= 0) throw ArgumentOutOfRangeException("width",  ...);
        /// if (height <= 0) throw ArgumentOutOfRangeException("height", ...);
        /// if (depth  <= 0) throw ArgumentOutOfRangeException("depth",  ...);
        /// if (MaxVolumeExtent == 0)
        ///     Throw(ProfileFeatureNotSupported, "Texture3D");
        /// if (!ValidVolumeFormats.Contains(format))
        ///     Throw(ProfileFormatNotSupported, "Texture3D", format);
        /// if (width > MaxVolumeExtent || height > ... || depth > ...)
        ///     Throw(ProfileTooBig, "Texture3D", MaxVolumeExtent);
        /// if (aspect ratio > MaxTextureAspectRatio)
        ///     Throw(ProfileAspectRatio, "Texture3D", MaxTextureAspectRatio);
        /// if (!NonPow2Volume && !(all three are powers of two))
        ///     Throw(ProfileNotPowerOfTwo, "Texture3D");
        /// ```
        ///
        /// **`MaxVolumeExtent == 0` is the whole story on Reach.** The extracted
        /// table gives Reach a zero extent and an empty `ValidVolumeFormats`, so
        /// the second check fires for every volume texture and no `Texture3D`
        /// can be constructed on this profile at all — which is XNA's rule, not
        /// a limit of this binding, and is why the type is complete and its
        /// constructor always refuses here. CNA agrees independently:
        /// `cna_texture3d_create` answers `CNA_RESULT_NOT_SUPPORTED`
        /// (`build-probe/f64_cube.c`).
        internal func validateVolumeCreation(
            width: Int32, height: Int32, depth: Int32, format: SurfaceFormat
        ) throws {
            for (value, name) in [(width, "width"), (height, "height"), (depth, "depth")] {
                guard value > 0 else {
                    throw CNAArgumentOutOfRangeException(
                        paramName: name,
                        message: Microsoft.Xna.Framework.Graphics.Texture2D
                            .resourcesMustBeGreaterThanZeroSizeMessage)
                }
            }
            guard maxVolumeExtent != 0 else {
                try throwNotSupported(
                    ProfileCapabilities.profileFeatureNotSupported, "Texture3D")
            }
            guard validVolumeFormats.contains(format) else {
                try throwNotSupported(
                    ProfileCapabilities.profileFormatNotSupported,
                    "Texture3D", "\(format)")
            }
            guard width <= maxVolumeExtent, height <= maxVolumeExtent,
                  depth <= maxVolumeExtent else {
                try throwNotSupported(
                    ProfileCapabilities.profileTooBig, "Texture3D", "\(maxVolumeExtent)")
            }
            let (longer, shorter) =
                ProfileCapabilities.volumeAspectExtremes(width, height, depth)
            guard (longer + shorter - 1) / shorter <= maxTextureAspectRatio else {
                try throwNotSupported(
                    ProfileCapabilities.profileAspectRatio,
                    "Texture3D", "\(maxTextureAspectRatio)")
            }
            if !nonPow2Volume,
               !(ProfileCapabilities.isPowerOfTwo(width)
                 && ProfileCapabilities.isPowerOfTwo(height)
                 && ProfileCapabilities.isPowerOfTwo(depth)) {
                try throwNotSupported(
                    ProfileCapabilities.profileNotPowerOfTwo, "Texture3D")
            }
        }

        /// `Max(Max(width, height), depth)` over `Min(Min(width, height),
        /// depth)` -- all THREE extents, not the two `Texture2D` compares.
        ///
        /// **Extracted so it can be tested at all.** The guard it feeds cannot
        /// fire on either profile: every extent is bounded by
        /// `maxVolumeExtent` several instructions earlier -- 256 on HiDef, and
        /// Reach refuses `Texture3D` outright with 0 -- so the ratio is at most
        /// 256 while `maxTextureAspectRatio` is 2048. XNA computes it anyway
        /// and so does this.
        ///
        /// A mutation taking the extremes over width and height only therefore
        /// survived every public route, which is how the unreachability was
        /// found rather than assumed. Same remedy as Foundation 68's
        /// `arrayHolds` and Foundation 65's `sameRenderTargetShape`: when a
        /// comparison cannot be reached through a member, make it a function of
        /// its numbers and test the function.
        internal static func volumeAspectExtremes(
            _ width: Int32, _ height: Int32, _ depth: Int32
        ) -> (longer: Int32, shorter: Int32) {
            (max(max(width, height), depth), min(min(width, height), depth))
        }

        /// `Texture.CheckCompressedTexture`, which answers true for the three
        /// DXT formats and nothing else.
        internal static func isCompressed(_ format: SurfaceFormat) -> Bool {
            format == .Dxt1 || format == .Dxt3 || format == .Dxt5
        }

        /// `Texture.IsPowerOfTwo(UInt32)`.
        internal static func isPowerOfTwo(_ value: Int32) -> Bool {
            let unsigned = UInt32(bitPattern: value)
            return unsigned != 0 && unsigned & (unsigned - 1) == 0
        }

        /// The table for one profile. `Reach` is what the qualified artifact
        /// reports, and `HiDef` is carried because the profile is the device's
        /// and not this host's.
        internal static func table(for profile: GraphicsProfile) -> ProfileCapabilities {
            profile == .Reach ? reach : hidef
        }

        internal static let reach = ProfileCapabilities(
            profile: .Reach,
            vertexShaderVersion: 512,
            pixelShaderVersion: 512,
            maxPrimitiveCount: 65535,
            maxVertexStreams: 16,
            maxStreamStride: 255,
            maxVertexBufferSize: 67108863,
            maxIndexBufferSize: 67108863,
            maxTextureSize: 2048,
            maxCubeSize: 512,
            maxVolumeExtent: 0,
            maxTextureAspectRatio: 2048,
            maxSamplers: 16,
            maxVertexSamplers: 0,
            maxRenderTargets: 1,
            occlusionQuery: false,
            getBackBufferData: false,
            separateAlphaBlend: false,
            destBlendSrcAlphaSat: false,
            minMaxSrcDestBlend: false,
            indexElementSize32: false,
            nonPow2Unconditional: false,
            nonPow2Cube: false,
            nonPow2Volume: false,
            validTextureFormats: [SurfaceFormat(rawValue: 0)!, SurfaceFormat(rawValue: 1)!, SurfaceFormat(rawValue: 2)!, SurfaceFormat(rawValue: 3)!, SurfaceFormat(rawValue: 4)!, SurfaceFormat(rawValue: 5)!, SurfaceFormat(rawValue: 6)!, SurfaceFormat(rawValue: 7)!, SurfaceFormat(rawValue: 8)!],
            validCubeFormats: [SurfaceFormat(rawValue: 0)!, SurfaceFormat(rawValue: 1)!, SurfaceFormat(rawValue: 2)!, SurfaceFormat(rawValue: 3)!, SurfaceFormat(rawValue: 4)!, SurfaceFormat(rawValue: 5)!, SurfaceFormat(rawValue: 6)!],
            validVolumeFormats: [],
            validVertexTextureFormats: [],
            invalidFilterFormats: [],
            invalidBlendFormats: [],
            validDepthFormats: [DepthFormat(rawValue: 1)!, DepthFormat(rawValue: 2)!, DepthFormat(rawValue: 3)!],
            validVertexFormats: [VertexElementFormat(rawValue: 4)!, VertexElementFormat(rawValue: 0)!, VertexElementFormat(rawValue: 1)!, VertexElementFormat(rawValue: 2)!, VertexElementFormat(rawValue: 3)!, VertexElementFormat(rawValue: 5)!, VertexElementFormat(rawValue: 6)!, VertexElementFormat(rawValue: 7)!, VertexElementFormat(rawValue: 8)!, VertexElementFormat(rawValue: 9)!])

        internal static let hidef = ProfileCapabilities(
            profile: .HiDef,
            vertexShaderVersion: 768,
            pixelShaderVersion: 768,
            maxPrimitiveCount: 1048575,
            maxVertexStreams: 16,
            maxStreamStride: 255,
            maxVertexBufferSize: 67108863,
            maxIndexBufferSize: 67108863,
            maxTextureSize: 4096,
            maxCubeSize: 4096,
            maxVolumeExtent: 256,
            maxTextureAspectRatio: 2048,
            maxSamplers: 16,
            maxVertexSamplers: 4,
            maxRenderTargets: 4,
            occlusionQuery: true,
            getBackBufferData: true,
            separateAlphaBlend: true,
            destBlendSrcAlphaSat: true,
            minMaxSrcDestBlend: false,
            indexElementSize32: true,
            nonPow2Unconditional: true,
            nonPow2Cube: true,
            nonPow2Volume: true,
            validTextureFormats: [SurfaceFormat(rawValue: 0)!, SurfaceFormat(rawValue: 1)!, SurfaceFormat(rawValue: 2)!, SurfaceFormat(rawValue: 3)!, SurfaceFormat(rawValue: 4)!, SurfaceFormat(rawValue: 5)!, SurfaceFormat(rawValue: 6)!, SurfaceFormat(rawValue: 7)!, SurfaceFormat(rawValue: 8)!, SurfaceFormat(rawValue: 9)!, SurfaceFormat(rawValue: 10)!, SurfaceFormat(rawValue: 11)!, SurfaceFormat(rawValue: 12)!, SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!, SurfaceFormat(rawValue: 19)!],
            validCubeFormats: [SurfaceFormat(rawValue: 0)!, SurfaceFormat(rawValue: 1)!, SurfaceFormat(rawValue: 2)!, SurfaceFormat(rawValue: 3)!, SurfaceFormat(rawValue: 4)!, SurfaceFormat(rawValue: 5)!, SurfaceFormat(rawValue: 6)!, SurfaceFormat(rawValue: 9)!, SurfaceFormat(rawValue: 10)!, SurfaceFormat(rawValue: 11)!, SurfaceFormat(rawValue: 12)!, SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!, SurfaceFormat(rawValue: 19)!],
            validVolumeFormats: [SurfaceFormat(rawValue: 0)!, SurfaceFormat(rawValue: 1)!, SurfaceFormat(rawValue: 2)!, SurfaceFormat(rawValue: 3)!, SurfaceFormat(rawValue: 9)!, SurfaceFormat(rawValue: 10)!, SurfaceFormat(rawValue: 11)!, SurfaceFormat(rawValue: 12)!, SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!, SurfaceFormat(rawValue: 19)!],
            validVertexTextureFormats: [SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!, SurfaceFormat(rawValue: 19)!],
            invalidFilterFormats: [SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!, SurfaceFormat(rawValue: 19)!],
            invalidBlendFormats: [SurfaceFormat(rawValue: 13)!, SurfaceFormat(rawValue: 14)!, SurfaceFormat(rawValue: 15)!, SurfaceFormat(rawValue: 16)!, SurfaceFormat(rawValue: 17)!, SurfaceFormat(rawValue: 18)!],
            validDepthFormats: [DepthFormat(rawValue: 1)!, DepthFormat(rawValue: 2)!, DepthFormat(rawValue: 3)!],
            validVertexFormats: [VertexElementFormat(rawValue: 4)!, VertexElementFormat(rawValue: 0)!, VertexElementFormat(rawValue: 1)!, VertexElementFormat(rawValue: 2)!, VertexElementFormat(rawValue: 3)!, VertexElementFormat(rawValue: 5)!, VertexElementFormat(rawValue: 6)!, VertexElementFormat(rawValue: 7)!, VertexElementFormat(rawValue: 8)!, VertexElementFormat(rawValue: 9)!, VertexElementFormat(rawValue: 10)!, VertexElementFormat(rawValue: 11)!])
    }
}
