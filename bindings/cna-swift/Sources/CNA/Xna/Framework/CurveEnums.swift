// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public enum CurveContinuity: Int32 {
        case Smooth = 0
        case Step = 1
    }

    public enum CurveLoopType: Int32 {
        case Constant = 0
        case Cycle = 1
        case CycleOffset = 2
        case Oscillate = 3
        case Linear = 4
    }

    public enum CurveTangent: Int32 {
        case Flat = 0
        case Linear = 1
        case Smooth = 2
    }
}
