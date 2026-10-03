// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    open class CurveKey {
        public let Position: Float
        public var Value: Float
        public var TangentIn: Float
        public var TangentOut: Float
        public var Continuity: CurveContinuity

        public init(position: Float, value: Float) {
            Position = position
            Value = value
            TangentIn = 0
            TangentOut = 0
            Continuity = .Smooth
        }

        public init(position: Float, value: Float, tangentIn: Float, tangentOut: Float) {
            Position = position
            Value = value
            TangentIn = tangentIn
            TangentOut = tangentOut
            Continuity = .Smooth
        }

        public init(
            position: Float,
            value: Float,
            tangentIn: Float,
            tangentOut: Float,
            continuity: CurveContinuity
        ) {
            Position = position
            Value = value
            TangentIn = tangentIn
            TangentOut = tangentOut
            Continuity = continuity
        }

        public func Clone() -> CurveKey {
            CurveKey(
                position: Position,
                value: Value,
                tangentIn: TangentIn,
                tangentOut: TangentOut,
                continuity: Continuity
            )
        }

        public func Equals(_ other: CurveKey?) -> Bool {
            guard let other else { return false }
            return other.Position == Position &&
                other.Value == Value &&
                other.TangentIn == TangentIn &&
                other.TangentOut == TangentOut &&
                other.Continuity == Continuity
        }

        public func Equals(_ obj: Any?) -> Bool {
            Equals(obj as? CurveKey)
        }

        public func GetHashCode() -> Int32 {
            xnaFloatHash(Position)
                &+ xnaFloatHash(Value)
                &+ xnaFloatHash(TangentIn)
                &+ xnaFloatHash(TangentOut)
                &+ Continuity.rawValue
        }

        /// `CompareTo(CurveKey other)`.
        ///
        /// The IL performs no null check: its first instructions are
        /// `ldarg.1; ldfld position`, and `ldfld` through a null reference is
        /// specified by ECMA-335 to raise `NullReferenceException`. The class
        /// is therefore certain, while the message is produced by the CLR's
        /// own native code and is not in any admitted IL — so none is claimed,
        /// and `Message` reports the class's substituted default.
        public func CompareTo(_ other: CurveKey?) throws -> Int32 {
            guard let other else { throw CNANullReferenceException() }
            return xnaPositionCompare(to: other)
        }

        internal func xnaPositionCompare(to other: CurveKey) -> Int32 {
            if Position == other.Position { return 0 }
            if Position < other.Position { return -1 }
            return 1
        }

    }
}

public func == (
    a: Microsoft.Xna.Framework.CurveKey?,
    b: Microsoft.Xna.Framework.CurveKey?
) -> Bool {
    switch (a, b) {
    case (nil, nil):
        return true
    case (nil, _), (_, nil):
        return false
    case let (a?, b?):
        return a.Equals(b)
    }
}

public func != (
    a: Microsoft.Xna.Framework.CurveKey?,
    b: Microsoft.Xna.Framework.CurveKey?
) -> Bool {
    !(a == b)
}
