// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned XNA 4.0 Windows metadata declares a public sealed sequential
    // value type extending System.ValueType with no declared interface. It
    // stores four `assembly` int32-wide fields — `_offset`, `_format`,
    // `_usage`, `_usageIndex` — behind four public read/write properties whose
    // accessors are plain field loads and stores. The constructor assigns all
    // four arguments verbatim and validates nothing, so no argument check,
    // clamp, or normalization is invented here.
    public struct VertexElement {
        public var Offset: Int32
        public var VertexElementFormat: Microsoft.Xna.Framework.Graphics.VertexElementFormat
        public var VertexElementUsage: Microsoft.Xna.Framework.Graphics.VertexElementUsage
        public var UsageIndex: Int32

        public init(
            _ offset: Int32,
            _ elementFormat: Microsoft.Xna.Framework.Graphics.VertexElementFormat,
            _ elementUsage: Microsoft.Xna.Framework.Graphics.VertexElementUsage,
            _ usageIndex: Int32
        ) {
            Offset = offset
            VertexElementFormat = elementFormat
            VertexElementUsage = elementUsage
            UsageIndex = usageIndex
        }

        // Equals(object) returns false for null, false when the runtime types
        // differ, and otherwise defers to op_Equality on the unboxed value.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? VertexElement else { return false }
            return self == other
        }

        // GetHashCode boxes the value and calls the internal
        // Microsoft.Xna.Framework.Helpers.SmartGetHashCode, which pins the box,
        // XORs Marshal.SizeOf(obj) / 4 consecutive int32 words, and substitutes
        // 0x7FFFFFFF when the accumulated XOR is zero. This sequential layout
        // is exactly four int32 words, and XOR is order independent.
        public func GetHashCode() -> Int32 {
            let hash = UInt32(bitPattern: Offset) ^
                UInt32(bitPattern: VertexElementFormat.rawValue) ^
                UInt32(bitPattern: VertexElementUsage.rawValue) ^
                UInt32(bitPattern: UsageIndex)
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        // string.Format(CultureInfo.CurrentCulture,
        //   "{{Offset:{0} Format:{1} Usage:{2} UsageIndex:{3}}}",
        //   Offset, VertexElementFormat, VertexElementUsage, UsageIndex) — the
        // doubled braces are composite-format escapes, so one brace pair is
        // emitted, and the two boxed enum arguments render through
        // Enum.ToString().
        public func ToString() -> String {
            "{Offset:\(Offset) Format:\(Self.formatName(VertexElementFormat))" +
                " Usage:\(Self.usageName(VertexElementUsage))" +
                " UsageIndex:\(UsageIndex)}"
        }

        // op_Equality compares _offset, then _usageIndex, then _usage, then
        // _format, and any difference short-circuits to false.
        public static func == (lhs: VertexElement, rhs: VertexElement) -> Bool {
            lhs.Offset == rhs.Offset &&
                lhs.UsageIndex == rhs.UsageIndex &&
                lhs.VertexElementUsage == rhs.VertexElementUsage &&
                lhs.VertexElementFormat == rhs.VertexElementFormat
        }

        // op_Inequality is exactly the negation of op_Equality.
        public static func != (lhs: VertexElement, rhs: VertexElement) -> Bool {
            !(lhs == rhs)
        }

        // Boxed Enum.ToString() renders the declared CLR literal name.
        // VertexElementFormat and VertexElementUsage deliberately expose no
        // public string surface, so both tables stay private to this type.
        private static func formatName(
            _ value: Microsoft.Xna.Framework.Graphics.VertexElementFormat
        ) -> String {
            switch value {
            case .Single: return "Single"
            case .Vector2: return "Vector2"
            case .Vector3: return "Vector3"
            case .Vector4: return "Vector4"
            case .Color: return "Color"
            case .Byte4: return "Byte4"
            case .Short2: return "Short2"
            case .Short4: return "Short4"
            case .NormalizedShort2: return "NormalizedShort2"
            case .NormalizedShort4: return "NormalizedShort4"
            case .HalfVector2: return "HalfVector2"
            case .HalfVector4: return "HalfVector4"
            }
        }

        private static func usageName(
            _ value: Microsoft.Xna.Framework.Graphics.VertexElementUsage
        ) -> String {
            switch value {
            case .Position: return "Position"
            case .Color: return "Color"
            case .TextureCoordinate: return "TextureCoordinate"
            case .Normal: return "Normal"
            case .Binormal: return "Binormal"
            case .Tangent: return "Tangent"
            case .BlendIndices: return "BlendIndices"
            case .BlendWeight: return "BlendWeight"
            case .Depth: return "Depth"
            case .Fog: return "Fog"
            case .PointSize: return "PointSize"
            case .Sample: return "Sample"
            case .TessellateFactor: return "TessellateFactor"
            }
        }
    }
}
