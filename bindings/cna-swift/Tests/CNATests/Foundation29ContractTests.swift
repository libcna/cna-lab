// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for `Microsoft.Xna.Framework.Media.VisualizationData`,
// transcribed from the CIL of the registered, hash-matched
// Microsoft.Xna.Framework.dll (SHA-256 38e7093f…a130).
//
// The whole type is managed and reaches no native surface: the constructor
// allocates two `float[256]` arrays and wraps each in a
// `ReadOnlyCollection<float>`, and both getters are bare field reads.
extension PureValueTests {

    // `.ctor()` runs `newarr Single` with `ldc.i4 0x100` twice, so both
    // collections are exactly 256 elements long.
    func testVisualizationDataCarriesTwoHundredFiftySixValues() {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        XCTAssertEqual(data.Frequencies.Count, 256)
        XCTAssertEqual(data.Samples.Count, 256)
    }

    // A freshly allocated CLR `float[]` is zero-filled, and XNA's constructor
    // writes nothing else, so a new instance reads 256 zeros from each.
    func testVisualizationDataStartsZeroFilled() throws {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        for index in Int32(0)..<256 {
            XCTAssertEqual(try data.Frequencies.Item(index), 0)
            XCTAssertEqual(try data.Samples.Item(index), 0)
        }
    }

    // Both getters are `ldarg.0; ldfld …; ret` -- the field itself -- so each
    // returns the same object every time.
    func testVisualizationDataCollectionsHaveStableIdentity() {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        XCTAssertTrue(data.Frequencies === data.Frequencies)
        XCTAssertTrue(data.Samples === data.Samples)
    }

    // The two arrays are separate allocations, so the two collections are
    // distinct objects over distinct storage.
    func testVisualizationDataFrequenciesAndSamplesAreDistinct() {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        XCTAssertFalse(data.Frequencies === data.Samples)
        XCTAssertFalse(data.Frequencies.Items === data.Samples.Items)
    }

    // Two instances share nothing: each `.ctor` runs `newarr` of its own.
    func testVisualizationDataInstancesShareNoStorage() {
        let first = Microsoft.Xna.Framework.Media.VisualizationData()
        let second = Microsoft.Xna.Framework.Media.VisualizationData()
        XCTAssertFalse(first.Frequencies === second.Frequencies)
        XCTAssertFalse(first.Frequencies.Items === second.Frequencies.Items)
    }

    // `ReadOnlyCollection<T>` wraps the array **live**, so a write into the
    // storage is visible through the already-published collection without the
    // collection object changing. This is the behaviour that made a Swift
    // Array projection wrong.
    func testVisualizationDataCollectionsAreLiveViewsOfTheirStorage() throws {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        let published = data.Frequencies

        var frequencies = [Float](repeating: 0, count: 256)
        var samples = [Float](repeating: 0, count: 256)
        frequencies[0] = 0.25
        frequencies[255] = -1
        samples[7] = 0.5
        try data.store(frequencies: frequencies, samples: samples)

        // The same object, now reading the new values.
        XCTAssertTrue(published === data.Frequencies)
        XCTAssertEqual(try published.Item(0), 0.25)
        XCTAssertEqual(try published.Item(255), -1)
        XCTAssertEqual(try data.Samples.Item(7), 0.5)
        // Untouched positions keep their previous value.
        XCTAssertEqual(try published.Item(1), 0)
    }

    // The CLR wraps a fixed-length array, so the length is invariant: a
    // producer writes into the storage and can never resize it.
    func testVisualizationDataLengthIsInvariantAcrossWrites() throws {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        try data.store(
            frequencies: [Float](repeating: 1, count: 256),
            samples: [Float](repeating: 2, count: 256))
        XCTAssertEqual(data.Frequencies.Count, 256)
        XCTAssertEqual(data.Samples.Count, 256)
    }

    // Indexing outside the fixed range fails, exactly as the backing store's
    // `(uint)index < _size` check does.
    func testVisualizationDataRejectsOutOfRangeIndices() {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        XCTAssertThrowsError(try data.Frequencies.Item(256))
        XCTAssertThrowsError(try data.Frequencies.Item(-1))
        XCTAssertThrowsError(try data.Samples.Item(256))
    }

    // The published surface is read-only: `ReadOnlyCollection<T>` declares no
    // mutator, and searching and copying work as they do on any view.
    func testVisualizationDataViewsSupportTheReadOnlySurface() throws {
        let data = Microsoft.Xna.Framework.Media.VisualizationData()
        var frequencies = [Float](repeating: 0, count: 256)
        frequencies[3] = 0.75
        try data.store(
            frequencies: frequencies, samples: [Float](repeating: 0, count: 256))

        XCTAssertEqual(data.Frequencies.IndexOf(0.75), 3)
        XCTAssertEqual(data.Frequencies.IndexOf(9), -1)
        XCTAssertTrue(data.Frequencies.Contains(0.75))

        var destination = [Float](repeating: -1, count: 256)
        try data.Frequencies.CopyTo(&destination, index: 0)
        XCTAssertEqual(destination[3], 0.75)
        XCTAssertEqual(destination[0], 0)

        let enumerator = data.Frequencies.GetEnumerator()
        var count = 0
        while try enumerator.Next() != nil { count += 1 }
        XCTAssertEqual(count, 256)
    }
}
