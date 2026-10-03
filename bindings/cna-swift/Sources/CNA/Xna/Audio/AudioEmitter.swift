// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Audio {
    // Pinned in Microsoft.Xna.Framework.dll as a public, non-sealed class
    // extending System.Object, with a public parameterless constructor, four
    // read/write Vector3 properties and one read/write Single. It is the
    // emitter counterpart of `AudioListener` -- same file in the IL, same
    // `FlipHandedness` accessors, one extra property -- and is pure managed
    // data for exactly the same reason: no XACT engine, audio device or sound
    // backend is touched by any member. The engine appears only in
    // `Cue.Apply3D`, which consumes an emitter and is not implemented.
    // Constructing one claims no audio capability.
    //
    // Storage mirrors the pinned IL rather than being simplified, because the
    // IL's XACT-space storage is publicly observable. The instance holds an
    // `XACT_EMITTER_DATA` interop structure in X3DAudio's left-handed space,
    // and every Vector3 accessor passes through
    // `UnsafeNativeStructures.FlipHandedness`, which is exactly
    // `Vector3(v.X, v.Y, -v.Z)`.
    //
    // As on `AudioListener`, the flip is invisible for round-trips and visible
    // in the defaults: the constructor seeds `_Position` and `_Velocity` with
    // `Vector3.Zero` **without** flipping while the getters flip on the way
    // out, so the default `Position` and `Velocity` have a **negative zero**
    // Z; `_Forward` and `_Up` are seeded through the flip and read back as
    // exactly `Vector3.Forward` and `Vector3.Up`.
    //
    // The constructor also seeds `ChannelCount = 1`, `ChannelRadius = 1` and
    // `CurveDistanceScaler = 1` in the interop structure. None of those is a
    // public member of this type in the pinned contract, so none is projected.
    public class AudioEmitter {
        private var position = Microsoft.Xna.Framework.Vector3.Zero
        private var velocity = Microsoft.Xna.Framework.Vector3.Zero
        private var forward = Microsoft.Xna.Framework.Audio.flipHandedness(
            Microsoft.Xna.Framework.Vector3.Forward)
        private var up = Microsoft.Xna.Framework.Audio.flipHandedness(
            Microsoft.Xna.Framework.Vector3.Up)
        // `ldc.r4 1` into `_DopplerScale`.
        private var dopplerScale: Float = 1

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

        // `get_DopplerScale` is a bare field read -- no flip, no validation:
        //
        //     ldarg.0; ldflda emitterData; ldfld _DopplerScale; ret
        //
        // so the reader is an ordinary Swift property getter.
        public var DopplerScale: Float { dopplerScale }

        // `set_DopplerScale` validates before storing:
        //
        //     IL_0000:  ldarg.1
        //     IL_0001:  ldc.r4     0.0
        //     IL_0006:  bge.un.s   IL_0018
        //     IL_0008:  ldstr      "value"
        //     IL_000d:  call       FrameworkResources::get_InvalidEmitterDopplerScale()
        //     IL_0012:  newobj     ArgumentOutOfRangeException::.ctor(string, string)
        //     IL_0017:  throw
        //     IL_0018:  ldarg.0; ldflda emitterData; ldarg.1; stfld _DopplerScale; ret
        //
        // `bge.un.s` is the **unordered** form: it takes the branch when the
        // value is greater than or equal to zero *or when the comparison is
        // unordered*, which is precisely the NaN case. So the throw is reached
        // only on an ordered `value < 0`:
        //
        //     -0.0                accepted (-0.0 >= 0.0 is ordered and true)
        //     +0.0, +finite       accepted
        //     +Infinity           accepted
        //     NaN                 accepted (comparison is unordered)
        //     -Float.leastNonzeroMagnitude, -finite, -Infinity   throws
        //
        // Swift's `<` on Float is the same ordered comparison and is false for
        // NaN, so `value < 0` reproduces the branch exactly, with no explicit
        // NaN special case to get wrong.
        //
        // Swift has no throwing property setter, so this CLR setter accessor
        // projects to the writer method the general accessor rule names. It is
        // the setter of `DopplerScale`, not a second XNA member, and there is
        // deliberately no writable `DopplerScale` property beside it: an
        // unchecked write path would accept values XNA rejects.
        public func SetDopplerScale(_ value: Float) throws {
            guard !(value < 0) else {
                // `ArgumentOutOfRangeException::.ctor(string paramName,
                // string message)` -- paramName first, then the resource.
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: "The doppler scale of an audio emitter must be greater "
                    + "than or equal to zero.")
            }
            dopplerScale = value
        }

        /// The mirrored `CNA_AudioEmitter`, which carries the emitter's own
        /// `DopplerScale` beside the same four vectors the listener has.
        internal func nativeDescriptor() -> CNASwift_AudioEmitter {
            var native = CNASwift_AudioEmitter()
            native.struct_size = UInt32(MemoryLayout<CNASwift_AudioEmitter>.size)
            native.struct_version = 1
            native.doppler_scale = DopplerScale
            native.forward = CNASwift_Vector3(x: Forward.X, y: Forward.Y, z: Forward.Z)
            native.position = CNASwift_Vector3(x: Position.X, y: Position.Y, z: Position.Z)
            native.up = CNASwift_Vector3(x: Up.X, y: Up.Y, z: Up.Z)
            native.velocity = CNASwift_Vector3(x: Velocity.X, y: Velocity.Y, z: Velocity.Z)
            return native
        }
    }
}
