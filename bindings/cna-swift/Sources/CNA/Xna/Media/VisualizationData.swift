// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Media {
    /// `Microsoft.Xna.Framework.Media.VisualizationData`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.dll` as a public, non-sealed class on
    /// `System.Object` with exactly three public identities: a parameterless
    /// constructor and the two get-only properties `Frequencies` and `Samples`.
    /// Its entire IL is managed and reaches no native surface:
    ///
    /// ```text
    /// .ctor()            frequencies = new float[256]
    ///                    samples     = new float[256]
    ///                    frequenciesCollection = new ReadOnlyCollection<float>(frequencies)
    ///                    samplesCollection     = new ReadOnlyCollection<float>(samples)
    /// get_Frequencies    ldarg.0; ldfld frequenciesCollection; ret
    /// get_Samples        ldarg.0; ldfld samplesCollection; ret
    /// ```
    ///
    /// Both getters are bare field reads with no branch, no call and no throw,
    /// so both are infallible, and the pinned inventory proves both
    /// `PROVEN_NONNULL_SUCCESS` — the constructor assigns each field once and
    /// nothing ever stores null. They are therefore non-Optional and
    /// non-throwing.
    ///
    /// This type was unblocked by the `ReadOnlyCollection<T>` decision and by
    /// nothing else: it is the one BCL-collection consumer whose own IL is
    /// entirely managed. `GraphicsAdapter.Adapters`, `Microphone.All` and
    /// `SpriteFont.Characters` were re-audited at the same time and each still
    /// reaches a display, a capture device or loaded content, so all three stay
    /// deferred.
    ///
    /// **Its producer is deliberately not fabricated.** XNA fills the two
    /// arrays from `MediaPlayer.GetVisualizationData(VisualizationData)`, which
    /// is a media-runtime call on a still-missing type. A freshly constructed
    /// instance therefore reads 256 zeros from each collection — exactly what
    /// XNA's own constructor produces before any producer runs. The same
    /// pattern was already used for `GameComponentCollectionEventArgs`, which
    /// was implemented while its producer was still missing.
    open class VisualizationData {
        /// The CLR field count. `.ctor` allocates `new float[256]` for each
        /// array, and an array's length is fixed, so both collections are
        /// permanently 256 elements long.
        internal static let sampleCount = 256

        // XNA holds two `float[]` fields and wraps each in a
        // `ReadOnlyCollection<float>` **live** — the collection is a view, not
        // a copy, so whatever writes into the array is seen through it. A
        // Swift `Array` would be a value snapshot and would lose that, so the
        // backing store is the reference-typed `CNAList` the collection
        // families are built on and the same object is wrapped once, in the
        // initializer, exactly as the IL does.
        private let frequencies: CNAList<Float>
        private let samples: CNAList<Float>
        private let frequenciesCollection: CNAReadOnlyCollection<Float>
        private let samplesCollection: CNAReadOnlyCollection<Float>

        /// `.ctor()`.
        public init() {
            let frequencies = CNAList<Float>()
            let samples = CNAList<Float>()
            for _ in 0..<Self.sampleCount {
                frequencies.Add(0)
                samples.Add(0)
            }
            self.frequencies = frequencies
            self.samples = samples
            self.frequenciesCollection = CNAReadOnlyCollection(list: frequencies)
            self.samplesCollection = CNAReadOnlyCollection(list: samples)
        }

        /// `Frequencies` — a live read-only view of the frequency array.
        ///
        /// The CLR getter returns the field, so this is the same object every
        /// time and it observes every later write to the underlying storage.
        public var Frequencies: CNAReadOnlyCollection<Float> { frequenciesCollection }

        /// `Samples` — a live read-only view of the sample array.
        public var Samples: CNAReadOnlyCollection<Float> { samplesCollection }

        /// The internal write path XNA's `MediaPlayer.GetVisualizationData`
        /// uses, kept internal because that producer does not exist yet.
        ///
        /// It writes **into the wrapped storage** rather than replacing it, so
        /// the two published collections stay the same objects and stay live —
        /// which is what `ReadOnlyCollection<T>` wrapping a fixed-length array
        /// guarantees in the CLR. The lengths are therefore invariant.
        internal func store(frequencies newFrequencies: [Float], samples newSamples: [Float]) throws {
            guard newFrequencies.count == Self.sampleCount,
                  newSamples.count == Self.sampleCount else {
                // Deliberately CNAError and not a projected CLR exception:
                // `store` is internal, has no XNA counterpart, and guards this
                // binding's own producer invariant rather than a caller's
                // argument. Projecting it as ArgumentException would put a
                // CLR identity on a failure the CLR has no member for.
                throw CNAError.producerInvariant(
                    "VisualizationData carries exactly \(Self.sampleCount) values")
            }
            for index in 0..<Self.sampleCount {
                try frequencies.SetItem(Int32(index), newFrequencies[index])
                try samples.SetItem(Int32(index), newSamples[index])
            }
        }
    }
}
