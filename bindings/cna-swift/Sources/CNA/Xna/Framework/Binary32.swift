// SPDX-License-Identifier: MIT

import Foundation

// XNA's scalar transcendental operations accept a binary32 input, execute the
// corresponding System.Math operation, and round the result back to Single.
// Keeping that boundary explicit avoids accidental whole-algorithm promotion.
@inline(__always) internal func xnaSqrt(_ value: Float) -> Float {
    Float(Foundation.sqrt(Double(value)))
}

@inline(__always) internal func xnaSin(_ value: Float) -> Float {
    Float(Foundation.sin(Double(value)))
}

@inline(__always) internal func xnaCos(_ value: Float) -> Float {
    Float(Foundation.cos(Double(value)))
}

@inline(__always) internal func xnaTan(_ value: Float) -> Float {
    Float(Foundation.tan(Double(value)))
}

@inline(__always) internal func xnaAcos(_ value: Float) -> Float {
    Float(Foundation.acos(Double(value)))
}

@inline(__always) internal func xnaSinReciprocal(_ value: Float) -> Float {
    Float(1.0 / Foundation.sin(Double(value)))
}

@inline(__always) internal func xnaFloatHash(_ value: Float) -> Int32 {
    if value == 0 { return 0 }
    return Int32(bitPattern: value.bitPattern)
}

// System.Single.ToString() on the pinned .NET Framework runtime is the "G7"
// general format: seven significant digits, trailing zeros removed, and
// scientific notation with a signed two-digit exponent once the decimal
// exponent leaves [-4, 6]. C's "%.7g" selects the same notation, digits, and
// exponent width, but spells the exponent marker in lower case, so the marker
// is normalised to the CLR's upper case. Only the invariant/en-US rendering is
// projected; full CultureInfo behaviour remains outside the value milestones.
internal func xnaFloatString(_ value: Float) -> String {
    if value.isNaN { return "NaN" }
    if value == .infinity { return "Infinity" }
    if value == -.infinity { return "-Infinity" }
    if value == 0 { return "0" }
    let general = String(
        format: "%.7g", locale: Locale(identifier: "en_US_POSIX"), Double(value)
    )
    return general.replacingOccurrences(of: "e", with: "E")
}

internal func xnaValidateTransformArrayRanges(
    sourceCount: Int,
    sourceIndex: Int32,
    destinationCount: Int,
    destinationIndex: Int32,
    length: Int32
) throws {
    let sourceEnd = Int64(sourceIndex) + Int64(length)
    let destinationEnd = Int64(destinationIndex) + Int64(length)
    // `ArgumentException(FrameworkResources.NotEnoughSourceSize)` and
    // `...NotEnoughTargetSize`, both message-only, in exactly this order.
    if Int64(sourceCount) < sourceEnd {
        throw CNAArgumentException(
            message: "Source array must be equal or bigger than requested length.")
    }
    if Int64(destinationCount) < destinationEnd {
        throw CNAArgumentException(
            message: "Target array size must be equal or bigger than source array size.")
    }
    // XNA validates the two LENGTHS and never the indices, so a negative index
    // reaches `ldelema`, which ECMA-335 specifies raises
    // IndexOutOfRangeException. The runtime, not the IL, produces its message.
    if length > 0 && sourceIndex < 0 {
        throw CNAIndexOutOfRangeException()
    }
    if length > 0 && destinationIndex < 0 {
        throw CNAIndexOutOfRangeException()
    }
}
