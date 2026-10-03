// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned XNA 4.0 Windows metadata declares a public, non-sealed class
    // whose only constructor is `assembly` accessible:
    // `.ctor(int32 width, int32 height, valuetype SurfaceFormat format)`.
    // A CLR class with no accessible constructor cannot be derived from
    // outside its own assembly, so this projection is deliberately not `open`;
    // metadata sealed=false keeps it non-`final` so the defining module may
    // still derive from it. The Swift initializer below is the corresponding
    // internal construction path and is implementation infrastructure, not an
    // XNA public member.
    public class DisplayMode {
        private let width: Int32
        private let height: Int32
        private let format: SurfaceFormat

        internal init(width: Int32, height: Int32, format: SurfaceFormat) {
            self.width = width
            self.height = height
            self.format = format
        }

        public var Format: SurfaceFormat { format }

        public var Height: Int32 { height }

        public var Width: Int32 { width }

        // XNA returns +0 when either stored dimension is zero and otherwise
        // divides the two `conv.r4` conversions in the binary32 domain.
        public var AspectRatio: Float {
            height == 0 || width == 0 ? 0 : Float(width) / Float(height)
        }

        // XNA calls the internal Viewport.GetTitleSafeArea(0, 0, width,
        // height), which on the Windows runtime is exactly the unmodified
        // rectangle. No overscan inset and no display query is involved.
        public var TitleSafeArea: Microsoft.Xna.Framework.Rectangle {
            Microsoft.Xna.Framework.Rectangle(0, 0, width, height)
        }

        // string.Format(CultureInfo.CurrentCulture,
        //   "{{Width:{0} Height:{1} Format:{2} AspectRatio:{3}}}",
        //   width, height, Format, AspectRatio) — the doubled braces are
        // composite-format escapes, so a single brace pair is emitted.
        public func ToString() -> String {
            "{Width:\(width) Height:\(height) Format:\(Self.formatName(format))" +
                " AspectRatio:\(xnaFloatString(AspectRatio))}"
        }

        // Boxed `Enum.ToString()` renders the declared CLR literal name.
        // SurfaceFormat deliberately exposes no public string surface, so the
        // table stays private to this type.
        private static func formatName(_ value: SurfaceFormat) -> String {
            switch value {
            case .Color: return "Color"
            case .Bgr565: return "Bgr565"
            case .Bgra5551: return "Bgra5551"
            case .Bgra4444: return "Bgra4444"
            case .Dxt1: return "Dxt1"
            case .Dxt3: return "Dxt3"
            case .Dxt5: return "Dxt5"
            case .NormalizedByte2: return "NormalizedByte2"
            case .NormalizedByte4: return "NormalizedByte4"
            case .Rgba1010102: return "Rgba1010102"
            case .Rg32: return "Rg32"
            case .Rgba64: return "Rgba64"
            case .Alpha8: return "Alpha8"
            case .Single: return "Single"
            case .Vector2: return "Vector2"
            case .Vector4: return "Vector4"
            case .HalfSingle: return "HalfSingle"
            case .HalfVector2: return "HalfVector2"
            case .HalfVector4: return "HalfVector4"
            case .HdrBlendable: return "HdrBlendable"
            }
        }
    }
}
