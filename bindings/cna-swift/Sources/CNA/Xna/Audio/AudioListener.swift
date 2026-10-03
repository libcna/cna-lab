// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Audio {
    // Pinned in Microsoft.Xna.Framework.dll as a public, non-sealed class
    // extending System.Object, with a public parameterless constructor and
    // four read/write Vector3 properties. Despite the Audio namespace it is
    // pure managed *data*: it holds 3D listener state and depends on nothing
    // but Vector3. No XACT engine, audio device or sound backend is involved
    // in anything it does; the engine appears only in `Cue.Apply3D`, which
    // consumes a listener and is not implemented. Constructing one claims no
    // audio capability.
    //
    // Storage mirrors the pinned IL rather than being simplified, because the
    // IL's XACT-space storage is publicly observable. The instance holds an
    // `XACT_LISTENER_DATA` interop structure in X3DAudio's left-handed space,
    // and every accessor passes through `UnsafeNativeStructures.FlipHandedness`,
    // which is exactly `Vector3(v.X, v.Y, -v.Z)`.
    //
    // The flip is a bitwise involution -- negating twice restores the original
    // bit pattern, negative zero and NaN payloads included -- so a value
    // written and read back comes out unchanged and the flip is invisible for
    // round-trips. It is *not* invisible in the defaults: the constructor
    // seeds `_Position` and `_Velocity` with `Vector3.Zero` **without**
    // flipping, while the getters flip on the way out, so the default
    // `Position` and `Velocity` have a **negative zero** Z. `Forward` and `Up`
    // are seeded through the flip and therefore read back as exactly
    // `Vector3.Forward` and `Vector3.Up`. A "just store the value" projection
    // would silently get the first two wrong.
    public class AudioListener {
        private var position = Microsoft.Xna.Framework.Vector3.Zero
        private var velocity = Microsoft.Xna.Framework.Vector3.Zero
        private var forward = Microsoft.Xna.Framework.Audio.flipHandedness(
            Microsoft.Xna.Framework.Vector3.Forward)
        private var up = Microsoft.Xna.Framework.Audio.flipHandedness(
            Microsoft.Xna.Framework.Vector3.Up)

        public init() {}

        public var Position: Microsoft.Xna.Framework.Vector3 {
            get { Microsoft.Xna.Framework.Audio.flipHandedness(position) }
            set { position = Microsoft.Xna.Framework.Audio.flipHandedness(newValue) }
        }

        public var Velocity: Microsoft.Xna.Framework.Vector3 {
            get { Microsoft.Xna.Framework.Audio.flipHandedness(velocity) }
            set { velocity = Microsoft.Xna.Framework.Audio.flipHandedness(newValue) }
        }

        public var Forward: Microsoft.Xna.Framework.Vector3 {
            get { Microsoft.Xna.Framework.Audio.flipHandedness(forward) }
            set { forward = Microsoft.Xna.Framework.Audio.flipHandedness(newValue) }
        }

        public var Up: Microsoft.Xna.Framework.Vector3 {
            get { Microsoft.Xna.Framework.Audio.flipHandedness(up) }
            set { up = Microsoft.Xna.Framework.Audio.flipHandedness(newValue) }
        }

        /// The mirrored `CNA_AudioListener` this listener's four vectors make.
        ///
        /// Built on demand rather than stored: XNA's listener is four settable
        /// vectors and nothing else, and `Apply3D` is the only place the native
        /// shape is needed. The vectors read here are the PROPERTIES, so the
        /// handedness flip those accessors apply is already in them.
        internal func nativeDescriptor() -> CNASwift_AudioListener {
            var native = CNASwift_AudioListener()
            native.struct_size = UInt32(MemoryLayout<CNASwift_AudioListener>.size)
            native.struct_version = 1
            native.forward = CNASwift_Vector3(x: Forward.X, y: Forward.Y, z: Forward.Z)
            native.position = CNASwift_Vector3(x: Position.X, y: Position.Y, z: Position.Z)
            native.up = CNASwift_Vector3(x: Up.X, y: Up.Y, z: Up.Z)
            native.velocity = CNASwift_Vector3(x: Velocity.X, y: Velocity.Y, z: Velocity.Z)
            return native
        }
    }

    // `UnsafeNativeStructures.FlipHandedness` from the pinned IL:
    // `Vector3(vector.X, vector.Y, -vector.Z)`. Internal support, not an XNA
    // identity. `AudioEmitter` uses the same helper.
    internal static func flipHandedness(
        _ vector: Microsoft.Xna.Framework.Vector3
    ) -> Microsoft.Xna.Framework.Vector3 {
        Microsoft.Xna.Framework.Vector3(vector.X, vector.Y, -vector.Z)
    }
}
