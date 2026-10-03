// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Quaternion {
        public var X: Float
        public var Y: Float
        public var Z: Float
        public var W: Float

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) { X = x; Y = y; Z = z; W = w }
        public init(_ vectorPart: Vector3, _ scalarPart: Float) {
            X = vectorPart.X; Y = vectorPart.Y; Z = vectorPart.Z; W = scalarPart
        }

        public static var Identity: Quaternion { Quaternion(0, 0, 0, 1) }

        public func LengthSquared() -> Float { X * X + Y * Y + Z * Z + W * W }
        public func Length() -> Float { xnaSqrt(X * X + Y * Y + Z * Z + W * W) }
        public mutating func Normalize() {
            let factor: Float = 1 / xnaSqrt(LengthSquared())
            X *= factor; Y *= factor; Z *= factor; W *= factor
        }
        public static func Normalize(_ quaternion: Quaternion) -> Quaternion {
            var quaternion = quaternion; quaternion.Normalize(); return quaternion
        }
        public static func Normalize(_ quaternion: inout Quaternion, result: inout Quaternion) { result = Normalize(quaternion) }

        public mutating func Conjugate() { X = -X; Y = -Y; Z = -Z }
        public static func Conjugate(_ value: Quaternion) -> Quaternion { Quaternion(-value.X, -value.Y, -value.Z, value.W) }
        public static func Conjugate(_ value: inout Quaternion, result: inout Quaternion) { result = Conjugate(value) }
        public static func Inverse(_ quaternion: Quaternion) -> Quaternion {
            let inverseLengthSquared: Float = 1 / quaternion.LengthSquared()
            return Quaternion(-quaternion.X * inverseLengthSquared,
                              -quaternion.Y * inverseLengthSquared,
                              -quaternion.Z * inverseLengthSquared,
                              quaternion.W * inverseLengthSquared)
        }
        public static func Inverse(_ quaternion: inout Quaternion, result: inout Quaternion) { result = Inverse(quaternion) }

        public static func CreateFromAxisAngle(_ axis: Vector3, angle: Float) -> Quaternion {
            let halfAngle = angle * 0.5
            let sin = xnaSin(halfAngle); let cos = xnaCos(halfAngle)
            return Quaternion(axis.X * sin, axis.Y * sin, axis.Z * sin, cos)
        }
        public static func CreateFromAxisAngle(_ axis: inout Vector3, angle: Float, result: inout Quaternion) {
            result = CreateFromAxisAngle(axis, angle: angle)
        }
        public static func CreateFromYawPitchRoll(_ yaw: Float, pitch: Float, roll: Float) -> Quaternion {
            let halfRoll = roll * 0.5; let sinRoll = xnaSin(halfRoll); let cosRoll = xnaCos(halfRoll)
            let halfPitch = pitch * 0.5; let sinPitch = xnaSin(halfPitch); let cosPitch = xnaCos(halfPitch)
            let halfYaw = yaw * 0.5; let sinYaw = xnaSin(halfYaw); let cosYaw = xnaCos(halfYaw)
            return Quaternion(
                cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll,
                sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll,
                cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll,
                cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll)
        }
        public static func CreateFromYawPitchRoll(_ yaw: Float, pitch: Float, roll: Float, result: inout Quaternion) {
            result = CreateFromYawPitchRoll(yaw, pitch: pitch, roll: roll)
        }
        public static func CreateFromRotationMatrix(_ matrix: Matrix) -> Quaternion {
            let trace = matrix.M11 + matrix.M22 + matrix.M33
            if trace > 0 {
                let root = xnaSqrt(trace + 1); let factor: Float = 0.5 / root
                return Quaternion((matrix.M23 - matrix.M32) * factor,
                                  (matrix.M31 - matrix.M13) * factor,
                                  (matrix.M12 - matrix.M21) * factor,
                                  root * 0.5)
            }
            if matrix.M11 >= matrix.M22 && matrix.M11 >= matrix.M33 {
                let root = xnaSqrt(1 + matrix.M11 - matrix.M22 - matrix.M33); let factor: Float = 0.5 / root
                return Quaternion(0.5 * root,
                                  (matrix.M12 + matrix.M21) * factor,
                                  (matrix.M13 + matrix.M31) * factor,
                                  (matrix.M23 - matrix.M32) * factor)
            }
            if matrix.M22 > matrix.M33 {
                let root = xnaSqrt(1 + matrix.M22 - matrix.M11 - matrix.M33); let factor: Float = 0.5 / root
                return Quaternion((matrix.M21 + matrix.M12) * factor,
                                  0.5 * root,
                                  (matrix.M32 + matrix.M23) * factor,
                                  (matrix.M31 - matrix.M13) * factor)
            }
            let root = xnaSqrt(1 + matrix.M33 - matrix.M11 - matrix.M22); let factor: Float = 0.5 / root
            return Quaternion((matrix.M31 + matrix.M13) * factor,
                              (matrix.M32 + matrix.M23) * factor,
                              0.5 * root,
                              (matrix.M12 - matrix.M21) * factor)
        }
        public static func CreateFromRotationMatrix(_ matrix: inout Matrix, result: inout Quaternion) {
            result = CreateFromRotationMatrix(matrix)
        }

        public static func Dot(_ quaternion1: Quaternion, quaternion2: Quaternion) -> Float {
            quaternion1.X * quaternion2.X + quaternion1.Y * quaternion2.Y +
                quaternion1.Z * quaternion2.Z + quaternion1.W * quaternion2.W
        }
        public static func Dot(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, result: inout Float) {
            result = Dot(quaternion1, quaternion2: quaternion2)
        }
        public static func Slerp(_ quaternion1: Quaternion, quaternion2: Quaternion, amount: Float) -> Quaternion {
            var cosOmega = Dot(quaternion1, quaternion2: quaternion2)
            var flip = false
            if cosOmega < 0 { flip = true; cosOmega = -cosOmega }
            let weight1: Float
            let weight2: Float
            if cosOmega > 0.999999 {
                weight1 = 1 - amount
                weight2 = flip ? -amount : amount
            } else {
                let omega = xnaAcos(cosOmega)
                let inverseSinOmega = xnaSinReciprocal(omega)
                weight1 = xnaSin((1 - amount) * omega) * inverseSinOmega
                weight2 = (flip ? -xnaSin(amount * omega) : xnaSin(amount * omega)) * inverseSinOmega
            }
            return Quaternion(weight1 * quaternion1.X + weight2 * quaternion2.X,
                              weight1 * quaternion1.Y + weight2 * quaternion2.Y,
                              weight1 * quaternion1.Z + weight2 * quaternion2.Z,
                              weight1 * quaternion1.W + weight2 * quaternion2.W)
        }
        public static func Slerp(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, amount: Float, result: inout Quaternion) {
            result = Slerp(quaternion1, quaternion2: quaternion2, amount: amount)
        }
        public static func Lerp(_ quaternion1: Quaternion, quaternion2: Quaternion, amount: Float) -> Quaternion {
            let inverse = 1 - amount
            let result: Quaternion
            if Dot(quaternion1, quaternion2: quaternion2) >= 0 {
                result = Quaternion(inverse * quaternion1.X + amount * quaternion2.X,
                                    inverse * quaternion1.Y + amount * quaternion2.Y,
                                    inverse * quaternion1.Z + amount * quaternion2.Z,
                                    inverse * quaternion1.W + amount * quaternion2.W)
            } else {
                result = Quaternion(inverse * quaternion1.X - amount * quaternion2.X,
                                    inverse * quaternion1.Y - amount * quaternion2.Y,
                                    inverse * quaternion1.Z - amount * quaternion2.Z,
                                    inverse * quaternion1.W - amount * quaternion2.W)
            }
            return Normalize(result)
        }
        public static func Lerp(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, amount: Float, result: inout Quaternion) {
            result = Lerp(quaternion1, quaternion2: quaternion2, amount: amount)
        }
        public static func Concatenate(_ value1: Quaternion, value2: Quaternion) -> Quaternion { value2 * value1 }
        public static func Concatenate(_ value1: inout Quaternion, value2: inout Quaternion, result: inout Quaternion) { result = Concatenate(value1, value2: value2) }

        public static func Negate(_ quaternion: Quaternion) -> Quaternion { -quaternion }
        public static func Negate(_ quaternion: inout Quaternion, result: inout Quaternion) { result = -quaternion }
        public static func Add(_ quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion { quaternion1 + quaternion2 }
        public static func Add(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, result: inout Quaternion) { result = quaternion1 + quaternion2 }
        public static func Subtract(_ quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion { quaternion1 - quaternion2 }
        public static func Subtract(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, result: inout Quaternion) { result = quaternion1 - quaternion2 }
        public static func Multiply(_ quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion { quaternion1 * quaternion2 }
        public static func Multiply(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, result: inout Quaternion) { result = quaternion1 * quaternion2 }
        public static func Multiply(_ quaternion1: Quaternion, scaleFactor: Float) -> Quaternion { quaternion1 * scaleFactor }
        public static func Multiply(_ quaternion1: inout Quaternion, scaleFactor: Float, result: inout Quaternion) { result = quaternion1 * scaleFactor }
        public static func Divide(_ quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion { quaternion1 / quaternion2 }
        public static func Divide(_ quaternion1: inout Quaternion, quaternion2: inout Quaternion, result: inout Quaternion) { result = quaternion1 / quaternion2 }

        public func Equals(_ other: Quaternion) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool { guard let other = obj as? Quaternion else { return false }; return self == other }
        public func GetHashCode() -> Int32 { xnaFloatHash(X) &+ xnaFloatHash(Y) &+ xnaFloatHash(Z) &+ xnaFloatHash(W) }
        public func ToString() -> String { "{X:\(xnaFloatString(X)) Y:\(xnaFloatString(Y)) Z:\(xnaFloatString(Z)) W:\(xnaFloatString(W))}" }

        public static prefix func - (quaternion: Quaternion) -> Quaternion { Quaternion(-quaternion.X, -quaternion.Y, -quaternion.Z, -quaternion.W) }
        public static func + (lhs: Quaternion, rhs: Quaternion) -> Quaternion { Quaternion(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z, lhs.W + rhs.W) }
        public static func - (lhs: Quaternion, rhs: Quaternion) -> Quaternion { Quaternion(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z, lhs.W - rhs.W) }
        public static func * (quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion {
            let crossX = quaternion1.Y * quaternion2.Z - quaternion1.Z * quaternion2.Y
            let crossY = quaternion1.Z * quaternion2.X - quaternion1.X * quaternion2.Z
            let crossZ = quaternion1.X * quaternion2.Y - quaternion1.Y * quaternion2.X
            let dot = quaternion1.X * quaternion2.X + quaternion1.Y * quaternion2.Y + quaternion1.Z * quaternion2.Z
            return Quaternion(quaternion1.X * quaternion2.W + quaternion2.X * quaternion1.W + crossX,
                              quaternion1.Y * quaternion2.W + quaternion2.Y * quaternion1.W + crossY,
                              quaternion1.Z * quaternion2.W + quaternion2.Z * quaternion1.W + crossZ,
                              quaternion1.W * quaternion2.W - dot)
        }
        public static func * (quaternion1: Quaternion, scaleFactor: Float) -> Quaternion {
            Quaternion(quaternion1.X * scaleFactor, quaternion1.Y * scaleFactor,
                       quaternion1.Z * scaleFactor, quaternion1.W * scaleFactor)
        }
        public static func / (quaternion1: Quaternion, quaternion2: Quaternion) -> Quaternion { quaternion1 * Inverse(quaternion2) }
        public static func == (lhs: Quaternion, rhs: Quaternion) -> Bool {
            lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z && lhs.W == rhs.W
        }
        public static func != (lhs: Quaternion, rhs: Quaternion) -> Bool { !(lhs == rhs) }
    }
}
