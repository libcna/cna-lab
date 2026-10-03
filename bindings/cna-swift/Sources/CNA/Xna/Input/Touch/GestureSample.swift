// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned as `.class public sequential ansi sealed beforefieldinit ...
    // extends [mscorlib]System.ValueType` with six `assembly` fields, one
    // public six-argument constructor that stores all six verbatim, and six
    // get-only property loads. It declares no equality identity, no
    // `GetHashCode`, and no `ToString`, so none is added.
    //
    // Completing this type claims no touch capability: `TouchPanel` is not
    // implemented, so nothing produces a gesture.
    public struct GestureSample {
        private let gestureType: Microsoft.Xna.Framework.Input.Touch.GestureType
        private let timestamp: Duration
        private let position: Microsoft.Xna.Framework.Vector2
        private let position2: Microsoft.Xna.Framework.Vector2
        private let delta: Microsoft.Xna.Framework.Vector2
        private let delta2: Microsoft.Xna.Framework.Vector2

        public init(
            _ gestureType: Microsoft.Xna.Framework.Input.Touch.GestureType,
            _ timestamp: Duration,
            _ position: Microsoft.Xna.Framework.Vector2,
            _ position2: Microsoft.Xna.Framework.Vector2,
            _ delta: Microsoft.Xna.Framework.Vector2,
            _ delta2: Microsoft.Xna.Framework.Vector2
        ) {
            self.gestureType = gestureType
            self.timestamp = timestamp
            self.position = position
            self.position2 = position2
            self.delta = delta
            self.delta2 = delta2
        }

        public var GestureType: Microsoft.Xna.Framework.Input.Touch.GestureType {
            gestureType
        }

        public var Timestamp: Duration { timestamp }

        public var Position: Microsoft.Xna.Framework.Vector2 { position }

        public var Position2: Microsoft.Xna.Framework.Vector2 { position2 }

        public var Delta: Microsoft.Xna.Framework.Vector2 { delta }

        public var Delta2: Microsoft.Xna.Framework.Vector2 { delta2 }
    }
}
