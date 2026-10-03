// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    // The pinned XNA 4.0 Windows metadata declares
    // `.class public auto ansi beforefieldinit PresentationParameters
    //  extends [mscorlib]System.Object` — public, not sealed, not abstract,
    // with a public parameterless `.ctor`. A public constructor on a
    // non-sealed CLR class is externally derivable, so this projection is
    // `open`. Every declared member is `hidebysig` without `virtual`, so each
    // one maps to a `public` — never `open` — Swift member: CLR non-virtual
    // members can be hidden by a derived `new`, but never overridden.
    //
    // The pinned type declares exactly one constructor, one method, and
    // eleven properties. There is no `Clear`, no `ToString`, `Equals`, or
    // `GetHashCode` override, no operator, no event, and no custom attribute
    // on the type. Nothing below is added beyond that contract.
    open class PresentationParameters {
        // Pinned nested storage:
        // `.class sequential ansi sealed nested assembly beforefieldinit
        //  Settings extends [mscorlib]System.ValueType`, holding all ten
        // fields, reached through the single `.field assembly ... settings`.
        // Both are `assembly`, so both stay `internal` and neither reaches the
        // public Swift surface. Transcribing the struct verbatim is what makes
        // `Clone` a single wholesale value copy, exactly as the IL performs it.
        //
        // Every field is left at its CLR default by the constructor; the zero
        // literal of each mapped enum is its pinned zero-valued case.
        internal struct Settings {
            var BackBufferWidth: Int32 = 0
            var BackBufferHeight: Int32 = 0
            var BackBufferFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat = .Color
            var DepthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat = .None
            var MultiSampleCount: Int32 = 0
            var DisplayOrientation: Microsoft.Xna.Framework.DisplayOrientation = []
            var PresentationInterval: Microsoft.Xna.Framework.Graphics.PresentInterval = .Default
            var RenderTargetUsage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage = .DiscardContents
            // `native int DeviceWindowHandle`. `System.IntPtr` maps to Swift
            // `Int` under the existing general BCL rule: the opaque
            // pointer-width signed numeric value, not a Swift pointer and not
            // a dereferenceable address.
            var DeviceWindowHandle: Int = 0
            // `IsFullScreen` is stored as `int32`, not `bool`; the accessors
            // below normalise it, so the 0/1 storage is unobservable through
            // the public projection.
            var IsFullScreen: Int32 = 0
        }

        internal var settings = Settings()

        // `.ctor()` calls `System.Object::.ctor()` and then
        // `ldc.i4.1; call set_IsFullScreen(bool)`. It sets `IsFullScreen` and
        // nothing else, so every other property starts at its CLR default.
        // The pinned literal is 1: a freshly constructed
        // `PresentationParameters` is full-screen. This is read out of the
        // pinned IL and is deliberately not the remembered MonoGame/FNA
        // default.
        /// Builds the value CNA reports for the device it owns.
        ///
        /// Every field is copied; nothing is defaulted here, because a value
        /// this binding invented would be indistinguishable from one the
        /// runtime chose.
        internal convenience init(native: CNASwift_PresentationParameters) throws {
            self.init()
            guard let backBuffer = Microsoft.Xna.Framework.Graphics
                    .SurfaceFormat(rawValue: Int32(native.back_buffer_format)),
                  let depth = Microsoft.Xna.Framework.Graphics
                    .DepthFormat(rawValue: Int32(native.depth_stencil_format)),
                  let interval = Microsoft.Xna.Framework.Graphics
                    .PresentInterval(rawValue: Int32(native.presentation_interval)),
                  let usage = Microsoft.Xna.Framework.Graphics
                    .RenderTargetUsage(rawValue: Int32(native.render_target_usage)) else {
                throw CNAError.producerInvariant(
                    "the device reported presentation parameters carrying a value "
                    + "outside one of the pinned enumerations")
            }
            BackBufferWidth = native.back_buffer_width
            BackBufferHeight = native.back_buffer_height
            BackBufferFormat = backBuffer
            DepthStencilFormat = depth
            MultiSampleCount = native.multi_sample_count
            PresentationInterval = interval
            DisplayOrientation = Microsoft.Xna.Framework
                .DisplayOrientation(rawValue: Int32(native.display_orientation))
            RenderTargetUsage = usage
            IsFullScreen = native.is_full_screen != 0
        }

        /// The C-safe descriptor the reset route consumes.
        internal func nativeDescriptor() -> CNASwift_PresentationParameters {
            var native = CNASwift_PresentationParameters()
            native.struct_size = UInt32(MemoryLayout<CNASwift_PresentationParameters>.size)
            native.struct_version = 1
            native.back_buffer_format = UInt32(BackBufferFormat.rawValue)
            native.back_buffer_width = BackBufferWidth
            native.back_buffer_height = BackBufferHeight
            native.depth_stencil_format = UInt32(DepthStencilFormat.rawValue)
            native.multi_sample_count = MultiSampleCount
            native.presentation_interval = UInt32(PresentationInterval.rawValue)
            native.display_orientation = UInt32(DisplayOrientation.rawValue)
            native.render_target_usage = UInt32(RenderTargetUsage.rawValue)
            native.is_full_screen = IsFullScreen ? 1 : 0
            return native
        }

        public init() {
            IsFullScreen = true
        }

        // `newobj .ctor()`, then a single `ldfld`/`stfld` of the whole
        // `settings` value struct. The wholesale copy overwrites the
        // constructor's `IsFullScreen = true`, so the result is an exact
        // field-by-field copy of the source. `Clone` is non-virtual and
        // constructs `PresentationParameters` itself rather than the dynamic
        // type, so a derived instance still clones to a base instance.
        public func Clone() -> PresentationParameters {
            let clone = PresentationParameters()
            clone.settings = settings
            return clone
        }

        // Each accessor pair below is a plain `ldflda settings` followed by a
        // field load or store. Nothing is validated, clamped, or rejected.
        public var BackBufferWidth: Int32 {
            get { settings.BackBufferWidth }
            set { settings.BackBufferWidth = newValue }
        }

        public var BackBufferHeight: Int32 {
            get { settings.BackBufferHeight }
            set { settings.BackBufferHeight = newValue }
        }

        public var BackBufferFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat {
            get { settings.BackBufferFormat }
            set { settings.BackBufferFormat = newValue }
        }

        public var DepthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat {
            get { settings.DepthStencilFormat }
            set { settings.DepthStencilFormat = newValue }
        }

        public var MultiSampleCount: Int32 {
            get { settings.MultiSampleCount }
            set { settings.MultiSampleCount = newValue }
        }

        public var DisplayOrientation: Microsoft.Xna.Framework.DisplayOrientation {
            get { settings.DisplayOrientation }
            set { settings.DisplayOrientation = newValue }
        }

        public var PresentationInterval: Microsoft.Xna.Framework.Graphics.PresentInterval {
            get { settings.PresentationInterval }
            set { settings.PresentationInterval = newValue }
        }

        public var RenderTargetUsage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage {
            get { settings.RenderTargetUsage }
            set { settings.RenderTargetUsage = newValue }
        }

        // Managed descriptor state only. The stored value is never
        // dereferenced, validated against a real window, resolved through
        // SDL, or handed to CNA; zero is `IntPtr.Zero`.
        public var DeviceWindowHandle: Int {
            get { settings.DeviceWindowHandle }
            set { settings.DeviceWindowHandle = newValue }
        }

        // `get_IsFullScreen` is `ldfld int32; ldc.i4.0; ceq; ldc.i4.0; ceq`,
        // a double comparison that normalises any non-zero word to true.
        // `set_IsFullScreen` stores exactly 1 or 0.
        public var IsFullScreen: Bool {
            get { settings.IsFullScreen != 0 }
            set { settings.IsFullScreen = newValue ? 1 : 0 }
        }

        // `get_Bounds` is `new Rectangle(0, 0, settings.BackBufferWidth,
        // settings.BackBufferHeight)` built from direct field loads. It is
        // get-only, it always starts at the origin, and it never consults a
        // device, adapter, or window.
        public var Bounds: Microsoft.Xna.Framework.Rectangle {
            Microsoft.Xna.Framework.Rectangle(
                0, 0, settings.BackBufferWidth, settings.BackBufferHeight
            )
        }
    }
}
