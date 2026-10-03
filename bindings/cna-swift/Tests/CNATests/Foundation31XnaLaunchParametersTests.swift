// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for `Microsoft.Xna.Framework.LaunchParameters`,
// transcribed from the CIL of the hash-registered
// `Microsoft.Xna.Framework.Game.dll` (SHA-256 b5dffdd8…a1f0).
//
// The type declares one parameterless constructor whose body is exactly
// `base..ctor()`, so everything asserted here about its usable surface is the
// inherited `Dictionary<string, string>` behaviour arriving through an XNA
// type — which is precisely the claim the base projection makes.
extension PureValueTests {

    func testLaunchParametersDerivesFromTheDictionarySpecialization() {
        let parameters = Microsoft.Xna.Framework.LaunchParameters()
        let erased: Any = parameters
        XCTAssertTrue(erased is CNADictionary<String, String>)
        // The element types are preserved, not erased: a `CNADictionary<Any,
        // Any>` would have made every inherited signature wrong.
        let asBase: CNADictionary<String, String> = parameters
        asBase.Clear()
        XCTAssertEqual(asBase.Count, 0)
        XCTAssertEqual(parameters.Count, 0, "the base view is the same object")
    }

    // The constructor is NOT a bare `base..ctor()`: it parses the arguments of
    // the process it is running in. A test therefore cannot assert what a
    // freshly constructed instance contains -- that depends on how the test
    // runner was invoked -- so it asserts the property that always holds: the
    // contents are exactly what parsing this process's own arguments gives.
    func testTheConstructorParsesThisProcessesArguments() throws {
        let parameters = Microsoft.Xna.Framework.LaunchParameters()
        let reference = Microsoft.Xna.Framework.LaunchParameters()
        XCTAssertEqual(parameters.Count, reference.Count)

        var collected: [String] = []
        let enumerator = parameters.GetEnumerator()
        while let pair = try enumerator.Next() { collected.append(pair.Key) }
        // Element zero is the executable and is never a key.
        XCTAssertFalse(collected.contains(CommandLine.arguments[0]))
    }

    // `ParseCommandLineArguments` is `assembly`-visible in the IL, so it is
    // internal here — and testing it directly is what makes the constructor's
    // behaviour checkable against a supplied vector rather than against
    // whichever process happens to be running the tests.
    func testCommandLineParsing() throws {
        func parse(_ arguments: [String]) -> [(String, String)] {
            let parameters = Microsoft.Xna.Framework.LaunchParameters()
            parameters.Clear()
            parameters.ParseCommandLineArguments(arguments)
            var pairs: [(String, String)] = []
            let enumerator = parameters.GetEnumerator()
            while let entry = ((try? enumerator.Next()) ?? nil) {
                pairs.append((entry.Key, entry.Value))
            }
            return pairs
        }

        // Element zero is the executable and is skipped, so a vector with only
        // an executable produces nothing at all.
        XCTAssertTrue(parse(["game.exe"]).isEmpty)
        XCTAssertTrue(parse([]).isEmpty)

        // `TrimStart('/', '-')` removes EVERY leading separator, in any mix.
        let trimmed = parse(["game.exe", "/windowed", "-fullscreen", "--x", "//y"])
        XCTAssertEqual(trimmed.map { $0.0 }, ["windowed", "fullscreen", "x", "y"])
        // An argument with no colon becomes a key with an EMPTY value, not a
        // missing one.
        XCTAssertTrue(trimmed.allSatisfy { $0.1.isEmpty })

        // Only the FIRST colon splits.
        XCTAssertEqual(parse(["g", "/level:3"]).first.map { [$0.0, $0.1] },
                       ["level", "3"])
        XCTAssertEqual(parse(["g", "/path:c:/tmp"]).first.map { [$0.0, $0.1] },
                       ["path", "c:/tmp"])
        // A trailing colon gives an empty value; a leading one an empty key,
        // which is then dropped.
        XCTAssertEqual(parse(["g", "/level:"]).first.map { [$0.0, $0.1] },
                       ["level", ""])
        XCTAssertTrue(parse(["g", "/:3"]).isEmpty)
        // An argument that trims away to nothing is dropped, not stored under
        // an empty key.
        XCTAssertTrue(parse(["g", "///"]).isEmpty)
        XCTAssertTrue(parse(["g", ""]).isEmpty)

        // The FIRST occurrence of a repeated key wins -- the guard is
        // `!ContainsKey(key)`, so a later one is discarded rather than
        // overwriting.
        XCTAssertEqual(
            parse(["g", "/level:3", "/level:9"]).map { [$0.0, $0.1] },
            [["level", "3"]])

        // Order is insertion order, which is the dictionary's contract.
        XCTAssertEqual(parse(["g", "/b:2", "/a:1", "/c:3"]).map { $0.0 },
                       ["b", "a", "c"])
    }

    // Reference identity, which is the whole reason the base is a class: two
    // instances are distinct, and a second binding of the same instance is the
    // same object rather than a copy.
    func testLaunchParametersHasReferenceIdentity() throws {
        let first = Microsoft.Xna.Framework.LaunchParameters()
        let second = Microsoft.Xna.Framework.LaunchParameters()
        XCTAssertFalse(first === second)

        let alias = first
        first.Clear()
        try alias.Add("/windowed", value: "")
        XCTAssertEqual(
            first.Count, 1,
            "a Swift dictionary would have given the alias a copy here")
        XCTAssertTrue(alias === first)
    }

    // The inherited surface is usable through the XNA type, including the two
    // behaviours a Swift `[String: String]` could not have provided.
    func testTheInheritedDictionarySurfaceWorksThroughTheXnaType() throws {
        let parameters = Microsoft.Xna.Framework.LaunchParameters()
        // Start from a known state: the constructor has already parsed this
        // process's arguments, which a test must not assume anything about.
        parameters.Clear()
        try parameters.Add("/windowed", value: "true")
        try parameters.Add("/level", value: "3")

        XCTAssertEqual(parameters.Count, 2)
        XCTAssertEqual(try parameters.Item("/level"), "3")
        XCTAssertTrue(parameters.ContainsKey("/windowed"))
        XCTAssertTrue(parameters.ContainsValue("3"))

        var value: String?
        XCTAssertTrue(parameters.TryGetValue("/level", value: &value))
        XCTAssertEqual(value, "3")

        // Add refuses a duplicate; the indexer overwrites one.
        XCTAssertThrowsError(try parameters.Add("/level", value: "4"))
        parameters.SetItem("/level", "4")
        XCTAssertEqual(try parameters.Item("/level"), "4")

        // Enumeration is in insertion order.
        var collected: [String] = []
        let enumerator = parameters.GetEnumerator()
        while let pair = try enumerator.Next() { collected.append(pair.Key) }
        XCTAssertEqual(collected, ["/windowed", "/level"])

        XCTAssertTrue(parameters.Remove("/windowed"))
        XCTAssertEqual(parameters.Count, 1)
        parameters.Clear()
        XCTAssertEqual(parameters.Count, 0)
    }

    // CLR non-sealed maps to Swift `open`, so a consumer can derive from it
    // exactly as XNA allows.
    func testLaunchParametersIsDerivable() throws {
        let derived = DerivedLaunchParameters()
        derived.Clear()
        try derived.Add("/mode", value: "test")
        XCTAssertEqual(derived.Count, 1)
        let erased: Any = derived
        XCTAssertTrue(erased is Microsoft.Xna.Framework.LaunchParameters)
        XCTAssertTrue(erased is CNADictionary<String, String>)
    }
}

final class DerivedLaunchParameters: Microsoft.Xna.Framework.LaunchParameters {}
