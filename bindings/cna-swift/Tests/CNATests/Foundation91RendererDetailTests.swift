// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.Audio.RendererDetail`.
///
/// Entirely managed, so entirely testable -- and untestable through anything
/// that produces one, because `AudioEngine.RendererDetails` is blocked on a
/// settings file this repository does not have. What is asserted here is the
/// behaviour a consumer can reach: value equality, hashing and rendering.
final class Foundation91RendererDetailTests: XCTestCase {

    private typealias Detail = Microsoft.Xna.Framework.Audio.RendererDetail

    /// **Equality is over the id alone**, which is the point of having both
    /// fields: the friendly name is what a person reads and the id is what
    /// identifies the renderer, so two details naming the same device with
    /// different labels are the same device.
    func testEqualityComparesTheIdNotTheLabel() {
        let a = Detail(friendlyName: "Speakers", rendererId: "{0}")
        let b = Detail(friendlyName: "Headphones", rendererId: "{0}")
        let c = Detail(friendlyName: "Speakers", rendererId: "{1}")

        XCTAssertTrue(a == b, "same id, different label: the same renderer")
        XCTAssertFalse(a != b)
        XCTAssertFalse(a == c, "same label, different id: a different renderer")
        XCTAssertTrue(a != c)
        XCTAssertTrue(a.Equals(b))
        XCTAssertFalse(a.Equals(c))
    }

    /// `Equals(Object)` answers false for anything that is not a detail, which
    /// is the branch a boxed comparison takes.
    func testEqualsRefusesAnotherType() {
        let a = Detail(friendlyName: "Speakers", rendererId: "{0}")
        XCTAssertFalse(a.Equals("{0}"), "a string with the same text is not a detail")
        XCTAssertFalse(a.Equals(nil))
    }

    /// Equal values must hash equally, which is the contract a hash has. The
    /// exact number is deliberately NOT asserted: the CLR's own
    /// `String.GetHashCode` is unspecified across runtimes, so claiming a
    /// particular value would be claiming something Microsoft does not promise.
    func testEqualValuesHashEqually() {
        let a = Detail(friendlyName: "Speakers", rendererId: "{0}")
        let b = Detail(friendlyName: "Headphones", rendererId: "{0}")
        XCTAssertEqual(a.GetHashCode(), b.GetHashCode())
    }

    /// `ToString` is the friendly name itself, not a braced field list -- the
    /// geometry types render `{X:1 Y:2}`, and this one does not.
    func testToStringIsTheFriendlyName() {
        XCTAssertEqual(
            Detail(friendlyName: "Speakers", rendererId: "{0}").ToString(),
            "Speakers")
    }

    /// An unpopulated detail reads two empty strings, because neither return
    /// is proven nullable and the projection stays non-Optional.
    func testTheDefaultReadsEmptyStringsRatherThanNil() {
        let empty = Detail()
        XCTAssertEqual(empty.FriendlyName, "")
        XCTAssertEqual(empty.RendererId, "")
        XCTAssertEqual(empty.ToString(), "")
        XCTAssertTrue(empty == Detail(), "two defaults are the same renderer")
    }
}
