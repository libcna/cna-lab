// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    open class Curve {
        public var PreLoop: CurveLoopType = .Constant
        public var PostLoop: CurveLoopType = .Constant
        public private(set) var Keys = CurveKeyCollection()

        public var IsConstant: Bool { Keys.Count <= 1 }

        public init() {}

        public func Clone() -> Curve {
            let result = Curve()
            result.PreLoop = PreLoop
            result.PostLoop = PostLoop
            result.Keys = Keys.Clone()
            return result
        }

        public func ComputeTangent(_ keyIndex: Int32, tangentType: CurveTangent) throws {
            try ComputeTangent(
                keyIndex,
                tangentInType: tangentType,
                tangentOutType: tangentType
            )
        }

        public func ComputeTangent(
            _ keyIndex: Int32,
            tangentInType: CurveTangent,
            tangentOutType: CurveTangent
        ) throws {
            guard keyIndex >= 0 && keyIndex < Keys.Count else {
                // `ArgumentOutOfRangeException::.ctor(string paramName)` -- the
                // paramName-only overload, so the message is the substituted
                // Arg_ArgumentOutOfRangeException.
                throw CNAArgumentOutOfRangeException(paramName: "keyIndex")
            }
            computeTangent(
                at: Int(keyIndex),
                tangentInType: tangentInType,
                tangentOutType: tangentOutType
            )
        }

        private func computeTangent(
            at index: Int,
            tangentInType: CurveTangent,
            tangentOutType: CurveTangent
        ) {
            let key = Keys.key(at: index)
            let currentPosition = key.Position
            let currentValue = key.Value
            var previousPosition = currentPosition
            var previousValue = currentValue
            var nextPosition = currentPosition
            var nextValue = currentValue
            if index > 0 {
                previousPosition = Keys.key(at: index - 1).Position
                previousValue = Keys.key(at: index - 1).Value
            }
            if index + 1 < Int(Keys.Count) {
                nextPosition = Keys.key(at: index + 1).Position
                nextValue = Keys.key(at: index + 1).Value
            }

            switch tangentInType {
            case .Smooth:
                let positionSpan = nextPosition - previousPosition
                let valueSpan = nextValue - previousValue
                if abs(valueSpan) < Float.ulpOfOne {
                    key.TangentIn = 0
                } else {
                    key.TangentIn = valueSpan * abs(previousPosition - currentPosition) / positionSpan
                }
            case .Linear:
                key.TangentIn = currentValue - previousValue
            case .Flat:
                key.TangentIn = 0
            }

            switch tangentOutType {
            case .Smooth:
                let positionSpan = nextPosition - previousPosition
                let valueSpan = nextValue - previousValue
                if abs(valueSpan) < Float.ulpOfOne {
                    key.TangentOut = 0
                } else {
                    key.TangentOut = valueSpan * abs(nextPosition - currentPosition) / positionSpan
                }
            case .Linear:
                key.TangentOut = nextValue - currentValue
            case .Flat:
                key.TangentOut = 0
            }
        }

        public func ComputeTangents(_ tangentType: CurveTangent) {
            for index in 0..<Int(Keys.Count) {
                computeTangent(at: index, tangentInType: tangentType, tangentOutType: tangentType)
            }
        }

        public func ComputeTangents(
            _ tangentInType: CurveTangent,
            tangentOutType: CurveTangent
        ) {
            for index in 0..<Int(Keys.Count) {
                computeTangent(
                    at: index,
                    tangentInType: tangentInType,
                    tangentOutType: tangentOutType
                )
            }
        }

        public func Evaluate(_ position: Float) -> Float {
            if Keys.Count == 0 { return 0 }
            if Keys.Count == 1 { return Keys.key(at: 0).Value }

            let first = Keys.key(at: 0)
            let last = Keys.key(at: Int(Keys.Count) - 1)
            var selectedPosition = position
            var valueOffset: Float = 0

            if selectedPosition < first.Position {
                if PreLoop == .Constant { return first.Value }
                if PreLoop == .Linear {
                    return first.Value - first.TangentIn * (first.Position - selectedPosition)
                }
                if !Keys.isCacheAvailable { Keys.computeCacheValues() }
                let cycle = calcCycle(selectedPosition)
                let remainder = selectedPosition - (first.Position + cycle * Keys.timeRange)
                switch PreLoop {
                case .Cycle:
                    selectedPosition = first.Position + remainder
                case .CycleOffset:
                    selectedPosition = first.Position + remainder
                    valueOffset = (last.Value - first.Value) * cycle
                case .Oscillate:
                    selectedPosition = xnaCycleIsOdd(cycle)
                        ? last.Position - remainder
                        : first.Position + remainder
                case .Constant, .Linear:
                    break
                }
            } else if last.Position < selectedPosition {
                if PostLoop == .Constant { return last.Value }
                if PostLoop == .Linear {
                    return last.Value - last.TangentOut * (last.Position - selectedPosition)
                }
                if !Keys.isCacheAvailable { Keys.computeCacheValues() }
                let cycle = calcCycle(selectedPosition)
                let remainder = selectedPosition - (first.Position + cycle * Keys.timeRange)
                switch PostLoop {
                case .Cycle:
                    selectedPosition = first.Position + remainder
                case .CycleOffset:
                    selectedPosition = first.Position + remainder
                    valueOffset = (last.Value - first.Value) * cycle
                case .Oscillate:
                    selectedPosition = xnaCycleIsOdd(cycle)
                        ? last.Position - remainder
                        : first.Position + remainder
                case .Constant, .Linear:
                    break
                }
            }

            let segment = findSegment(selectedPosition)
            return valueOffset + Self.hermite(segment.0, segment.1, segment.2)
        }

        private func calcCycle(_ position: Float) -> Float {
            var cycle = (position - Keys.key(at: 0).Position) * Keys.inverseTimeRange
            if cycle < 0 { cycle -= 1 }
            return Float(xnaUncheckedFloatToInt32(cycle))
        }

        private func findSegment(_ position: Float) -> (CurveKey, CurveKey, Float) {
            var amount = position
            var first = Keys.key(at: 0)
            var second = first
            for index in 1..<Int(Keys.Count) {
                second = Keys.key(at: index)
                if second.Position >= position {
                    let firstPosition = Double(first.Position)
                    let secondPosition = Double(second.Position)
                    let targetPosition = Double(position)
                    let span = secondPosition - firstPosition
                    amount = 0
                    if span > 1e-10 {
                        amount = Float((targetPosition - firstPosition) / span)
                    }
                    break
                }
                first = second
            }
            return (first, second, amount)
        }

        private static func hermite(_ first: CurveKey, _ second: CurveKey, _ amount: Float) -> Float {
            if first.Continuity == .Step {
                return amount < 1 ? first.Value : second.Value
            }
            let squared = amount * amount
            let cubed = squared * amount
            let firstBasis = 2 * cubed - 3 * squared + 1
            let secondBasis = -2 * cubed + 3 * squared
            let outBasis = cubed - 2 * squared + amount
            let inBasis = cubed - squared
            return first.Value * firstBasis
                + second.Value * secondBasis
                + first.TangentOut * outBasis
                + second.TangentIn * inBasis
        }
    }
}

@inline(__always) private func xnaUncheckedFloatToInt32(_ value: Float) -> Int32 {
    if value.isFinite && value >= -2_147_483_648 && value < 2_147_483_648 {
        return Int32(value.rounded(.towardZero))
    }
    return .min
}

@inline(__always) private func xnaCycleIsOdd(_ cycle: Float) -> Bool {
    (xnaUncheckedFloatToInt32(cycle) & 1) != 0
}
