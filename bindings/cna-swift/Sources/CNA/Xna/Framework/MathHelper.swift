// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    /// The XNA 4.0 single-precision scalar math helpers.
    public final class MathHelper {
        private init() {}
        public static let E: Float = 2.718282
        public static let Log2E: Float = 1.442695
        public static let Log10E: Float = 0.4342945
        public static let Pi: Float = 3.141593
        public static let TwoPi: Float = 6.283185
        public static let PiOver2: Float = 1.570796
        public static let PiOver4: Float = 0.7853982

        public static func ToRadians(_ degrees: Float) -> Float {
            degrees * 0.0174532924
        }

        public static func ToDegrees(_ radians: Float) -> Float {
            radians * 57.29578
        }

        public static func Distance(_ value1: Float, value2: Float) -> Float {
            abs(value1 - value2)
        }

        public static func Min(_ value1: Float, value2: Float) -> Float {
            value1 < value2 ? value1 : value2
        }

        public static func Max(_ value1: Float, value2: Float) -> Float {
            value1 > value2 ? value1 : value2
        }

        public static func Clamp(_ value: Float, min: Float, max: Float) -> Float {
            if value > max { return max }
            if value < min { return min }
            return value
        }

        public static func Lerp(_ value1: Float, value2: Float, amount: Float) -> Float {
            value1 + ((value2 - value1) * amount)
        }

        public static func Barycentric(
            _ value1: Float,
            value2: Float,
            value3: Float,
            amount1: Float,
            amount2: Float
        ) -> Float {
            value1 + (value2 - value1) * amount1 + (value3 - value1) * amount2
        }

        public static func SmoothStep(_ value1: Float, value2: Float, amount: Float) -> Float {
            Hermite(value1, tangent1: 0, value2: value2, tangent2: 0, amount: Clamp(amount, min: 0, max: 1))
        }

        public static func CatmullRom(
            _ value1: Float,
            value2: Float,
            value3: Float,
            value4: Float,
            amount: Float
        ) -> Float {
            let amountSquared = amount * amount
            let amountCubed = amountSquared * amount
            return 0.5 * (2 * value2 + (value3 - value1) * amount +
                (2 * value1 - 5 * value2 + 4 * value3 - value4) * amountSquared +
                (3 * value2 - value1 - 3 * value3 + value4) * amountCubed)
        }

        public static func Hermite(
            _ value1: Float,
            tangent1: Float,
            value2: Float,
            tangent2: Float,
            amount: Float
        ) -> Float {
            if amount == 0 { return value1 }
            if amount == 1 { return value2 }
            let squared = amount * amount
            let cubed = squared * amount
            return value1 * ((2 * cubed) - (3 * squared) + 1) +
                value2 * ((-2 * cubed) + (3 * squared)) +
                tangent1 * (cubed - (2 * squared) + amount) +
                tangent2 * (cubed - squared)
        }

        public static func WrapAngle(_ angle: Float) -> Float {
            var result = angle
            if result > -Pi && result <= Pi { return result }
            result = (result + Pi).truncatingRemainder(dividingBy: TwoPi)
            if result <= 0 { result += TwoPi }
            return result - Pi
        }
    }
}
