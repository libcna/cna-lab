// SPDX-License-Identifier: MIT

// Managed scalar helpers ported instruction-for-instruction from the pinned
// Microsoft XNA 4.0 Windows runtime's PackUtils and HalfUtils IL.

@inline(__always) internal func xnaPackedClampAndRound(
    _ value: Float,
    min: Float,
    max: Float
) -> Double {
    if value.isNaN { return 0 }
    if value.isInfinite { return value.sign == .minus ? Double(min) : Double(max) }
    if value < min { return Double(min) }
    if value > max { return Double(max) }
    return Double(value).rounded(.toNearestOrEven)
}

@inline(__always) internal func xnaPackUnsigned(_ bitmask: Float, _ value: Float) -> UInt32 {
    UInt32(xnaPackedClampAndRound(value, min: 0, max: bitmask))
}

@inline(__always) internal func xnaPackSigned(_ bitmask: UInt32, _ value: Float) -> UInt32 {
    let maximum = Float(bitmask >> 1)
    let minimum = -maximum - 1
    let rounded = Int32(xnaPackedClampAndRound(value, min: minimum, max: maximum))
    return UInt32(bitPattern: rounded) & bitmask
}

@inline(__always) internal func xnaPackUNorm(_ bitmask: Float, _ value: Float) -> UInt32 {
    let scaled = value * bitmask
    return UInt32(xnaPackedClampAndRound(scaled, min: 0, max: bitmask))
}

@inline(__always) internal func xnaUnpackUNorm(_ bitmask: UInt32, _ value: UInt32) -> Float {
    Float(value & bitmask) / Float(bitmask)
}

@inline(__always) internal func xnaPackSNorm(_ bitmask: UInt32, _ value: Float) -> UInt32 {
    let maximum = Float(bitmask >> 1)
    let scaled = value * maximum
    let rounded = Int32(xnaPackedClampAndRound(scaled, min: -maximum, max: maximum))
    return UInt32(bitPattern: rounded) & bitmask
}

@inline(__always) internal func xnaUnpackSNorm(_ bitmask: UInt32, _ input: UInt32) -> Float {
    let sign = (bitmask &+ 1) >> 1
    var value = input
    if value & sign != 0 {
        if value & bitmask == sign { return -1 }
        value |= ~bitmask
    } else {
        value &= bitmask
    }
    return Float(Int32(bitPattern: value)) / Float(bitmask >> 1)
}

@inline(__always) internal func xnaPackHalf(_ value: Float) -> UInt16 {
    let bits = value.bitPattern
    let sign = (bits & 0x8000_0000) >> 16
    var magnitude = bits & 0x7FFF_FFFF

    if magnitude > 0x47FF_EFFF {
        return UInt16(sign | 0x7FFF)
    }
    if magnitude < 0x3880_0000 {
        let significand = (magnitude & 0x007F_FFFF) | 0x0080_0000
        let shift = Int(113 - (magnitude >> 23))
        magnitude = shift <= 31 ? significand >> shift : 0
        let rounded = (magnitude &+ 4095 &+ ((magnitude >> 13) & 1)) >> 13
        return UInt16(sign | rounded)
    }

    let rounded = (magnitude &- 0x3800_0000) &+ 4095 &+ ((magnitude >> 13) & 1)
    return UInt16(sign | (rounded >> 13))
}

@inline(__always) internal func xnaUnpackHalf(_ value: UInt16) -> Float {
    let raw = UInt32(value)
    let sign = (raw & 0x8000) << 16
    let fraction = raw & 0x03FF
    let result: UInt32

    if raw & 0x7C00 == 0 {
        if fraction == 0 {
            result = sign
        } else {
            var exponent: Int32 = -14
            var significand = fraction
            while significand & 0x0400 == 0 {
                exponent -= 1
                significand <<= 1
            }
            significand &= ~UInt32(0x0400)
            result = sign | (UInt32(exponent + 127) << 23) | (significand << 13)
        }
    } else {
        let exponent = Int32((raw >> 10) & 0x1F) - 15 + 127
        result = sign | (UInt32(exponent) << 23) | (fraction << 13)
    }
    return Float(bitPattern: result)
}

internal func xnaPackedHex<T>(_ value: T, width: Int) -> String
where T: FixedWidthInteger & UnsignedInteger {
    let digits = String(value, radix: 16, uppercase: true)
    return String(repeating: "0", count: Swift.max(0, width - digits.count)) + digits
}

@inline(__always) internal func xnaPackedHash(_ value: UInt8) -> Int32 { Int32(value) }
@inline(__always) internal func xnaPackedHash(_ value: UInt16) -> Int32 { Int32(value) }
@inline(__always) internal func xnaPackedHash(_ value: UInt32) -> Int32 { Int32(bitPattern: value) }
@inline(__always) internal func xnaPackedHash(_ value: UInt64) -> Int32 {
    Int32(bitPattern: UInt32(truncatingIfNeeded: value) ^ UInt32(truncatingIfNeeded: value >> 32))
}
