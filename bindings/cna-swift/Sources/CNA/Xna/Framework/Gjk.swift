// SPDX-License-Identifier: MIT

// Private XNA 4.0 GJK simplex implementation used only by BoundingFrustum.
// It intentionally has no public identity in the Microsoft namespace.
internal final class XnaGjk {
    private static let bitsToIndices = [
        0, 1, 2, 17, 3, 25, 26, 209, 4, 33, 34, 273, 35, 281, 282, 2257,
    ]

    private var closestPoint = Microsoft.Xna.Framework.Vector3.Zero
    private var y = Array(repeating: Microsoft.Xna.Framework.Vector3.Zero, count: 4)
    private var yLengthSquared = Array(repeating: Float(0), count: 4)
    private var edges = Array(
        repeating: Array(repeating: Microsoft.Xna.Framework.Vector3.Zero, count: 4), count: 4)
    private var edgeLengthSquared = Array(repeating: Array(repeating: Float(0), count: 4), count: 4)
    private var determinants = Array(repeating: Array(repeating: Float(0), count: 4), count: 16)
    private var simplexBits = 0
    private var maxLengthSquared = Float(0)

    internal var FullSimplex: Bool { simplexBits == 15 }
    internal var MaxLengthSquared: Float { maxLengthSquared }
    internal var ClosestPoint: Microsoft.Xna.Framework.Vector3 { closestPoint }

    internal func Reset() {
        simplexBits = 0
        maxLengthSquared = 0
    }

    @discardableResult
    internal func AddSupportPoint(_ newPoint: Microsoft.Xna.Framework.Vector3) -> Bool {
        let newIndex = (Self.bitsToIndices[simplexBits ^ 15] & 7) - 1
        y[newIndex] = newPoint
        yLengthSquared[newIndex] = newPoint.LengthSquared()

        var indices = Self.bitsToIndices[simplexBits]
        while indices != 0 {
            let index = (indices & 7) - 1
            let edge = y[index] - newPoint
            edges[index][newIndex] = edge
            edges[newIndex][index] = -edge
            let length = edge.LengthSquared()
            edgeLengthSquared[newIndex][index] = length
            edgeLengthSquared[index][newIndex] = length
            indices >>= 3
        }

        updateDeterminant(newIndex)
        return updateSimplex(newIndex)
    }

    @inline(__always)
    private func dot(_ a: Microsoft.Xna.Framework.Vector3, _ b: Microsoft.Xna.Framework.Vector3) -> Float {
        (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z)
    }

    private func updateDeterminant(_ newIndex: Int) {
        let newBit = 1 << newIndex
        determinants[newBit][newIndex] = 1
        let allIndices = Self.bitsToIndices[simplexBits]
        var remainingIndices = allIndices
        var priorCount = 0

        while remainingIndices != 0 {
            let index = (remainingIndices & 7) - 1
            let indexBit = 1 << index
            let pairBits = indexBit | newBit
            determinants[pairBits][index] = dot(edges[newIndex][index], y[newIndex])
            determinants[pairBits][newIndex] = dot(edges[index][newIndex], y[index])

            var earlierIndices = allIndices
            if priorCount > 0 {
                for _ in 0..<priorCount {
                    let earlierIndex = (earlierIndices & 7) - 1
                    let earlierBit = 1 << earlierIndex
                    let tripleBits = pairBits | earlierBit

                    var edgeIndex = edgeLengthSquared[index][earlierIndex] <
                        edgeLengthSquared[newIndex][earlierIndex] ? index : newIndex
                    determinants[tripleBits][earlierIndex] =
                        (determinants[pairBits][index] * dot(edges[edgeIndex][earlierIndex], y[index])) +
                        (determinants[pairBits][newIndex] * dot(edges[edgeIndex][earlierIndex], y[newIndex]))

                    edgeIndex = edgeLengthSquared[earlierIndex][index] <
                        edgeLengthSquared[newIndex][index] ? earlierIndex : newIndex
                    determinants[tripleBits][index] =
                        (determinants[earlierBit | newBit][earlierIndex] *
                            dot(edges[edgeIndex][index], y[earlierIndex])) +
                        (determinants[earlierBit | newBit][newIndex] *
                            dot(edges[edgeIndex][index], y[newIndex]))

                    edgeIndex = edgeLengthSquared[index][newIndex] <
                        edgeLengthSquared[earlierIndex][newIndex] ? index : earlierIndex
                    determinants[tripleBits][newIndex] =
                        (determinants[indexBit | earlierBit][earlierIndex] *
                            dot(edges[edgeIndex][newIndex], y[earlierIndex])) +
                        (determinants[indexBit | earlierBit][index] *
                            dot(edges[edgeIndex][newIndex], y[index]))
                    earlierIndices >>= 3
                }
            }
            remainingIndices >>= 3
            priorCount += 1
        }

        if (simplexBits | newBit) != 15 { return }

        var selected = !(edgeLengthSquared[1][0] < edgeLengthSquared[2][0])
            ? (edgeLengthSquared[2][0] < edgeLengthSquared[3][0] ? 2 : 3)
            : (edgeLengthSquared[1][0] < edgeLengthSquared[3][0] ? 1 : 3)
        determinants[15][0] =
            (determinants[14][1] * dot(edges[selected][0], y[1])) +
            (determinants[14][2] * dot(edges[selected][0], y[2])) +
            (determinants[14][3] * dot(edges[selected][0], y[3]))

        selected = !(edgeLengthSquared[0][1] < edgeLengthSquared[2][1])
            ? (edgeLengthSquared[2][1] < edgeLengthSquared[3][1] ? 2 : 3)
            : (!(edgeLengthSquared[0][1] < edgeLengthSquared[3][1]) ? 3 : 0)
        determinants[15][1] =
            (determinants[13][0] * dot(edges[selected][1], y[0])) +
            (determinants[13][2] * dot(edges[selected][1], y[2])) +
            (determinants[13][3] * dot(edges[selected][1], y[3]))

        selected = !(edgeLengthSquared[0][2] < edgeLengthSquared[1][2])
            ? (edgeLengthSquared[1][2] < edgeLengthSquared[3][2] ? 1 : 3)
            : (!(edgeLengthSquared[0][2] < edgeLengthSquared[3][2]) ? 3 : 0)
        determinants[15][2] =
            (determinants[11][0] * dot(edges[selected][2], y[0])) +
            (determinants[11][1] * dot(edges[selected][2], y[1])) +
            (determinants[11][3] * dot(edges[selected][2], y[3]))

        selected = !(edgeLengthSquared[0][3] < edgeLengthSquared[1][3])
            ? (edgeLengthSquared[1][3] < edgeLengthSquared[2][3] ? 1 : 2)
            : (!(edgeLengthSquared[0][3] < edgeLengthSquared[2][3]) ? 2 : 0)
        determinants[15][3] =
            (determinants[7][0] * dot(edges[selected][3], y[0])) +
            (determinants[7][1] * dot(edges[selected][3], y[1])) +
            (determinants[7][2] * dot(edges[selected][3], y[2]))
    }

    private func updateSimplex(_ newIndex: Int) -> Bool {
        let allBits = simplexBits | (1 << newIndex)
        let newBit = 1 << newIndex
        if simplexBits > 0 {
            for bits in stride(from: simplexBits, through: 1, by: -1) {
                if (bits & allBits) == bits && satisfiesRule(bits | newBit, allBits) {
                    simplexBits = bits | newBit
                    closestPoint = computeClosestPoint()
                    return true
                }
            }
        }
        if !satisfiesRule(newBit, allBits) { return false }
        simplexBits = newBit
        closestPoint = y[newIndex]
        maxLengthSquared = yLengthSquared[newIndex]
        return true
    }

    private func computeClosestPoint() -> Microsoft.Xna.Framework.Vector3 {
        var determinantSum: Float = 0
        var result = Microsoft.Xna.Framework.Vector3.Zero
        maxLengthSquared = 0
        var indices = Self.bitsToIndices[simplexBits]
        while indices != 0 {
            let index = (indices & 7) - 1
            let determinant = determinants[simplexBits][index]
            determinantSum += determinant
            result = result + y[index] * determinant
            maxLengthSquared = Microsoft.Xna.Framework.MathHelper.Max(
                maxLengthSquared, value2: yLengthSquared[index])
            indices >>= 3
        }
        return result / determinantSum
    }

    private func satisfiesRule(_ candidateBits: Int, _ allBits: Int) -> Bool {
        var indices = Self.bitsToIndices[allBits]
        while indices != 0 {
            let index = (indices & 7) - 1
            let bit = 1 << index
            if (bit & candidateBits) != 0 {
                if determinants[candidateBits][index] <= 0 { return false }
            } else if determinants[candidateBits | bit][index] > 0 {
                return false
            }
            indices >>= 3
        }
        return true
    }
}
