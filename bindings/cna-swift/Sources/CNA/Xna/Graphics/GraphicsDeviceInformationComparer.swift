// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework {

    /// `Microsoft.Xna.Framework.GraphicsDeviceInformationComparer`.
    ///
    /// Internal in XNA and internal here: it exists only as the ordering
    /// `GraphicsDeviceManager.RankDevices` sorts with. Its `Compare` is 638
    /// bytes and is transcribed rather than approximated, because which device
    /// a consumer ends up with is decided by this order alone.
    ///
    /// **Every comparison answers -1 or 1 and returns immediately**; there is
    /// no accumulated score. The chain, in the IL's order:
    ///
    /// 1. `GraphicsProfile` — **higher first**.
    /// 2. `IsFullScreen` — the one matching the *manager's* setting first.
    /// 3. `RankFormat` of the back-buffer format — **lower rank first**.
    /// 4. `MultiSampleCount` — **higher first**.
    /// 5. Aspect ratio distance from the preferred one, but only when the two
    ///    distances differ by **more than 0.2**.
    /// 6. Pixel-count distance from the target area.
    /// 7. The default adapter first.
    /// 8. Otherwise equal.
    internal final class GraphicsDeviceInformationComparer {

        /// `GraphicsDeviceManager.DefaultBackBufferWidth` and `-Height`,
        /// `0x320` and `0x1e0` in the class constructor.
        internal static let defaultBackBufferWidth: Int32 = 800
        internal static let defaultBackBufferHeight: Int32 = 480

        private let graphics: Microsoft.Xna.Framework.GraphicsDeviceManager

        internal init(_ graphics: Microsoft.Xna.Framework.GraphicsDeviceManager) {
            self.graphics = graphics
        }

        /// `RankFormat(SurfaceFormat format)`, 48 bytes and three answers.
        ///
        /// Zero when the format is exactly the preferred one, one when it
        /// merely has the same bit depth, and **`Int32.max`** otherwise — which
        /// is why step 3 sorts ascending: an unrelated format is pushed to the
        /// very end rather than scored.
        internal func RankFormat(
            _ format: Microsoft.Xna.Framework.Graphics.SurfaceFormat
        ) -> Int32 {
            let preferred = graphics.PreferredBackBufferFormat
            if format == preferred { return 0 }
            if GraphicsDeviceInformationComparer.surfaceFormatBitDepth(format)
                == GraphicsDeviceInformationComparer.surfaceFormatBitDepth(preferred) {
                return 1
            }
            return Int32.max
        }

        /// `SurfaceFormatBitDepth(SurfaceFormat)`, a switch over **four** raw
        /// values plus one explicit test, and zero for everything else.
        ///
        /// `Color` (0) and `Rgba1010102` (9) are 32; `Bgr565` (1),
        /// `Bgra5551` (2) and `Bgra4444` (3) are 16. Every other format --
        /// including all the floating-point and compressed ones -- answers
        /// **zero**, so they all share a bit depth and rank alike. That is
        /// XNA's table, not a simplification.
        internal static func surfaceFormatBitDepth(
            _ format: Microsoft.Xna.Framework.Graphics.SurfaceFormat
        ) -> Int32 {
            switch format.rawValue {
            case 0, 9: return 32
            case 1, 2, 3: return 16
            default: return 0
            }
        }

        internal func Compare(
            _ d1: Microsoft.Xna.Framework.GraphicsDeviceInformation,
            _ d2: Microsoft.Xna.Framework.GraphicsDeviceInformation
        ) -> Int32 {
            if d1.GraphicsProfile != d2.GraphicsProfile {
                return d1.GraphicsProfile.rawValue > d2.GraphicsProfile.rawValue ? -1 : 1
            }
            let p1 = d1.PresentationParameters
            let p2 = d2.PresentationParameters

            if p1.IsFullScreen != p2.IsFullScreen {
                // The candidate agreeing with the manager wins. Note it is the
                // MANAGER's flag that decides, not either candidate's.
                return graphics.IsFullScreen == p1.IsFullScreen ? -1 : 1
            }

            let r1 = RankFormat(p1.BackBufferFormat)
            let r2 = RankFormat(p2.BackBufferFormat)
            if r1 != r2 { return r1 < r2 ? -1 : 1 }

            if p1.MultiSampleCount != p2.MultiSampleCount {
                return p1.MultiSampleCount > p2.MultiSampleCount ? -1 : 1
            }

            // Aspect ratio, in Float exactly as the IL's `conv.r4` demands --
            // the 0.2 tolerance below is a single-precision comparison and a
            // Double one would order differently at the boundary.
            let preferredWidth = graphics.PreferredBackBufferWidth
            let preferredHeight = graphics.PreferredBackBufferHeight
            let target: Float
            if preferredWidth == 0 || preferredHeight == 0 {
                target = Float(GraphicsDeviceInformationComparer.defaultBackBufferWidth)
                    / Float(GraphicsDeviceInformationComparer.defaultBackBufferHeight)
            } else {
                target = Float(preferredWidth) / Float(preferredHeight)
            }
            let a1 = Float(p1.BackBufferWidth) / Float(p1.BackBufferHeight)
            let a2 = Float(p2.BackBufferWidth) / Float(p2.BackBufferHeight)
            let off1 = abs(a1 - target)
            let off2 = abs(a2 - target)
            // **Only decided when the two differ by more than 0.2.** Two modes
            // of nearly the same shape fall through to pixel count instead,
            // which is what stops a 16:10 mode losing to a 16:9 one purely on
            // shape.
            if abs(off1 - off2) > 0.2 { return off1 < off2 ? -1 : 1 }

            var target1: Int32 = 0
            var target2: Int32 = 0
            if graphics.IsFullScreen {
                if preferredWidth == 0 || preferredHeight == 0 {
                    // **The only branch where the two targets differ**: each
                    // candidate is measured against its own adapter's current
                    // mode.
                    let m1 = d1.Adapter?.CurrentDisplayMode
                    let m2 = d2.Adapter?.CurrentDisplayMode
                    target1 = (m1?.Width ?? 0) * (m1?.Height ?? 0)
                    target2 = (m2?.Width ?? 0) * (m2?.Height ?? 0)
                } else {
                    target1 = preferredWidth * preferredHeight
                    target2 = target1
                }
            } else if preferredWidth == 0 || preferredHeight == 0 {
                target1 = GraphicsDeviceInformationComparer.defaultBackBufferWidth
                    * GraphicsDeviceInformationComparer.defaultBackBufferHeight
                target2 = target1
            } else {
                target1 = preferredWidth * preferredHeight
                target2 = target1
            }

            let area1 = abs(p1.BackBufferWidth * p1.BackBufferHeight - target1)
            let area2 = abs(p2.BackBufferWidth * p2.BackBufferHeight - target2)
            if area1 != area2 { return area1 < area2 ? -1 : 1 }

            if d1.Adapter !== d2.Adapter {
                if d1.Adapter?.IsDefaultAdapter == true { return -1 }
                if d2.Adapter?.IsDefaultAdapter == true { return 1 }
            }
            return 0
        }
    }
}
