// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public enum StencilOperation: Int32 {
        case Keep = 0
        case Zero = 1
        case Replace = 2
        case Increment = 3
        case Decrement = 4
        case IncrementSaturation = 5
        case DecrementSaturation = 6
        case Invert = 7
    }
}
