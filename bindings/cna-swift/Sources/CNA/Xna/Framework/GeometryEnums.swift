// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public enum PlaneIntersectionType: Int32 {
        case Front = 0
        case Back = 1
        case Intersecting = 2
    }

    public enum ContainmentType: Int32 {
        case Disjoint = 0
        case Contains = 1
        case Intersects = 2
    }
}
