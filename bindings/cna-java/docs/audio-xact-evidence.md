# Audio and XACT compatibility evidence

## Evidence layers

The selected XNA 4.0 Windows metadata/IL contract is projected independently of runtime
capability. Evidence is reported in three layers:

1. strict API completeness against the mapped XNA contract;
2. deterministic managed behavior and ownership tests;
3. execution through the CNA 0.7 C ABI on Linux HEADLESS/NULL audio.

All 19 mapped Audio/XACT types are strict-complete. Authored XACT playback and real microphone
capture are not inferred from signatures or exported symbols.

## Ownership and callback graph

```text
Game
├── SoundEffect (owned effect)
│   └── SoundEffectInstance* (each owned; strongly retains parent)
├── DynamicSoundEffectInstance (owned voice)
│   └── BufferNeeded registration (owned until successful unsubscribe)
└── AudioEngine (owned XACT root)
    ├── AudioCategory handles
    ├── WaveBank*
    ├── SoundBank*
    └── Cue*
```

Every native destruction is transactional. A Java wrapper becomes disposed only after CNA accepts
the release; a wrong-thread failure leaves the handle, owner registration, and callback token live
for retry. Parent teardown walks children first. Callback contexts own JNI global references and
are freed only after successful CNA unsubscribe. BufferNeeded and BufferReady listener exceptions
are contained in Java and surface on the next owner call or close, never across a native frame.

Microphone wrappers are stable within one Game generation. Game destruction advances the
generation and invalidates stale wrappers even if a later native allocation reuses the same address.

## Qualified SoundEffect and dynamic behavior

The NULL-audio runtime executes:

- 16-bit PCM constructors and deterministic generated PCM-WAV `FromStream`;
- XNA-derived sample size/duration validation and float arithmetic;
- Name, MasterVolume, DistanceScale, DopplerScale, and SpeedOfSound;
- both SoundEffect Play overloads (honest `false` on NULL audio);
- multiple independently owned SoundEffectInstance voices;
- Volume, Pitch, Pan, IsLooped, Play, Pause, Resume, immediate/as-authored Stop, and State;
- single-listener and one-element-array Apply3D;
- Dynamic buffer and slice submission, pending count, transport, properties, and callbacks;
- parent-before-child, child-before-parent, double close, use after close, wrong-thread refusal and
  retry, failed native creation recovery, repeated construction, and Game shutdown with a live
  effect.

CNA's atomic multi-listener Apply3D extension reports NOT_SUPPORTED for two listeners on this
runtime. The Java implementation propagates that result rather than applying one listener at a
time. Empty listener arrays fail explicitly. Neither case is presented as successful XNA 3D
mixing.

## Microphone qualification

The public Microphone shape, validation, collection/default identity, duration/sample helpers,
Start/Stop/GetData overloads, state, and BufferReady event are implemented. The qualified NULL
runtime executes count/default queries and returns an empty immutable collection plus null default.
It does not fabricate a device or samples.

No capture device was available, so actual Start/Stop state, captured bytes, device BufferDuration,
and native BufferReady delivery remain hardware-pending. Callback ownership and exception
containment share the verified Audio registration machinery but are not claimed as a real-device
run.

## XACT qualification

AudioEngine, AudioCategory, WaveBank, SoundBank, and Cue have complete mapped surfaces and a real
JNI/CNA ownership implementation. Managed evidence covers constructor prevalidation, null/range
behavior, default AudioCategory/RendererDetail values, dependent registration, reverse teardown,
idempotence, disposed access, transactional release, and post-disposal one-shot event cleanup.
The manifest, signature probe, library ABI, and exports verify every native route.

No legal redistributable authored XGS/XSB/XWB fixture exists in the writable or inspected reference
corpus. Consequently these remain asset-pending:

- successful AudioEngine creation from an authored settings bank;
- category/global variable lookup against authored data;
- WaveBank/SoundBank creation and ordering;
- Cue lookup, variables, state transitions, 3D, and playback;
- authored-bank failure modes that require otherwise-valid bank structure.

No bank bytes are fabricated to turn those into passing tests. CNA accepts but currently ignores
the AudioEngine look-ahead and renderer arguments; NULL audio provides no renderer details. These
limitations are explicit rather than hidden by Java-side state.

## ABI and measured tests

Audio/XACT expanded the manifest from 399 to 487 canonical C functions. JNI is compiled as C11
with `-Wall -Wextra -Werror`; manifest/JNI consistency, C header layout/signature probes, ABI 0.7.0,
and 487/487 exports pass against the qualified library.

The native-enabled Java suite totals 118 tests with zero failures/errors/skips. Audio stress adds
100 effect/dynamic ownership cycles plus 25 repeated SoundEffect/instance pairs. No compatible
sanitizer-built CNA runtime was available, so allocator-level leak freedom is not claimed.
