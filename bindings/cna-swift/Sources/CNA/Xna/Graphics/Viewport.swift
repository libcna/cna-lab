// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    public struct Viewport {
        public var X: Int32
        public var Y: Int32
        public var Width: Int32
        public var Height: Int32
        public var MinDepth: Float
        public var MaxDepth: Float

        internal init() {
            X = 0
            Y = 0
            Width = 0
            Height = 0
            MinDepth = 0
            MaxDepth = 1
        }

        public init(_ x: Int32, _ y: Int32, _ width: Int32, _ height: Int32) {
            X = x
            Y = y
            Width = width
            Height = height
            MinDepth = 0
            MaxDepth = 1
        }

        public init(_ bounds: Microsoft.Xna.Framework.Rectangle) {
            self.init(bounds.X, bounds.Y, bounds.Width, bounds.Height)
        }

        public var AspectRatio: Float {
            Width == 0 || Height == 0 ? 0 : Float(Width) / Float(Height)
        }

        public var Bounds: Microsoft.Xna.Framework.Rectangle {
            get { Microsoft.Xna.Framework.Rectangle(X, Y, Width, Height) }
            set { X = newValue.X; Y = newValue.Y; Width = newValue.Width; Height = newValue.Height }
        }

        public var TitleSafeArea: Microsoft.Xna.Framework.Rectangle { Bounds }

        public func Project(
            _ source: Microsoft.Xna.Framework.Vector3,
            projection: Microsoft.Xna.Framework.Matrix,
            view: Microsoft.Xna.Framework.Matrix,
            world: Microsoft.Xna.Framework.Matrix
        ) -> Microsoft.Xna.Framework.Vector3 {
            var matrix = Microsoft.Xna.Framework.Matrix.Multiply(world, matrix2: view)
            matrix = Microsoft.Xna.Framework.Matrix.Multiply(matrix, matrix2: projection)
            var result = Microsoft.Xna.Framework.Vector3.Transform(source, matrix: matrix)
            let w = source.X * matrix.M14 + source.Y * matrix.M24 + source.Z * matrix.M34 + matrix.M44
            if !Self.withinEpsilon(w, 1) { result = result / w }
            result.X = (result.X + 1) * 0.5 * Float(Width) + Float(X)
            result.Y = (-result.Y + 1) * 0.5 * Float(Height) + Float(Y)
            result.Z = result.Z * (MaxDepth - MinDepth) + MinDepth
            return result
        }

        public func Unproject(
            _ source: Microsoft.Xna.Framework.Vector3,
            projection: Microsoft.Xna.Framework.Matrix,
            view: Microsoft.Xna.Framework.Matrix,
            world: Microsoft.Xna.Framework.Matrix
        ) -> Microsoft.Xna.Framework.Vector3 {
            var matrix = Microsoft.Xna.Framework.Matrix.Multiply(world, matrix2: view)
            matrix = Microsoft.Xna.Framework.Matrix.Multiply(matrix, matrix2: projection)
            matrix = Microsoft.Xna.Framework.Matrix.Invert(matrix)
            var source = source
            source.X = (source.X - Float(X)) / Float(Width) * 2 - 1
            source.Y = -((source.Y - Float(Y)) / Float(Height) * 2 - 1)
            source.Z = (source.Z - MinDepth) / (MaxDepth - MinDepth)
            var result = Microsoft.Xna.Framework.Vector3.Transform(source, matrix: matrix)
            let w = source.X * matrix.M14 + source.Y * matrix.M24 + source.Z * matrix.M34 + matrix.M44
            if !Self.withinEpsilon(w, 1) { result = result / w }
            return result
        }

        public func ToString() -> String {
            "{X:\(X) Y:\(Y) Width:\(Width) Height:\(Height) MinDepth:\(xnaFloatString(MinDepth)) MaxDepth:\(xnaFloatString(MaxDepth))}"
        }

        internal init(native: CNASwift_Viewport) {
            X = native.x
            Y = native.y
            Width = native.width
            Height = native.height
            MinDepth = native.min_depth
            MaxDepth = native.max_depth
        }

        private static func withinEpsilon(_ value1: Float, _ value2: Float) -> Bool {
            let difference = value1 - value2
            return -Float.leastNonzeroMagnitude <= difference && difference <= Float.leastNonzeroMagnitude
        }
    }
}
