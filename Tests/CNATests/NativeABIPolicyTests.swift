// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// The admitted CNA C ABI window, tested without a native library.
///
/// The rule is CNA's own — `docs/c-api/ABI_VERSIONING.md` and the installed
/// package's `SameMajorVersion` compatibility file both say a consumer must
/// reject a different major and may require a minimum minor — so these are the
/// cases that rule distinguishes, not a restatement of the implementation.
final class NativeABIPolicyTests: XCTestCase {
    func testEncodingIsCNAsOwnPacking() {
        // bits 31..16 major, 15..8 minor, 7..0 patch.
        XCTAssertEqual(NativeABI.encode(major: 0, minor: 21, patch: 0), 0x0000_1500)
        XCTAssertEqual(NativeABI.encode(major: 0, minor: 7, patch: 0), 0x0000_0700)
        XCTAssertEqual(NativeABI.encode(major: 1, minor: 0, patch: 0), 0x0001_0000)
        XCTAssertEqual(NativeABI.encode(major: 0, minor: 255, patch: 255), 0x0000_FFFF)

        XCTAssertEqual(NativeABI.major(0x0000_1500), 0)
        XCTAssertEqual(NativeABI.minor(0x0000_1500), 21)
        XCTAssertEqual(NativeABI.patch(0x0000_1503), 3)
        XCTAssertEqual(NativeABI.describe(0x0000_1503), "0.21.3")
    }

    func testQualifiedVersionIsTheMinimumOfTheWindow() {
        XCTAssertEqual(NativeABI.admittedMajor, 0)
        XCTAssertEqual(NativeABI.minimumMinor, 35)
        XCTAssertEqual(NativeABI.qualifiedVersion, NativeABI.encode(major: 0, minor: 35, patch: 0))
        XCTAssertTrue(NativeABI.admits(NativeABI.qualifiedVersion))
    }

    func testADifferentMajorIsRejected() {
        XCTAssertFalse(NativeABI.admits(NativeABI.encode(major: 1, minor: 35, patch: 0)))
        XCTAssertFalse(NativeABI.admits(NativeABI.encode(major: 1, minor: 99, patch: 0)))
        XCTAssertFalse(NativeABI.admits(NativeABI.encode(major: 2, minor: 0, patch: 0)))
    }

    func testALowerMinorIsRejected() {
        // Every generation this binding was previously pinned to -- 0.7.0 and
        // 0.21.0 -- and every minor between them and 0.35, none of which was
        // measured by this binding.
        for minor in UInt32(0)...34 {
            XCTAssertFalse(
                NativeABI.admits(NativeABI.encode(major: 0, minor: minor, patch: 0)),
                "0.\(minor).0 is below the admitted minimum")
        }
    }

    func testAHigherMinorAndAnyPatchAreAdmitted() {
        XCTAssertTrue(NativeABI.admits(NativeABI.encode(major: 0, minor: 35, patch: 7)))
        XCTAssertTrue(NativeABI.admits(NativeABI.encode(major: 0, minor: 36, patch: 0)))
        XCTAssertTrue(NativeABI.admits(NativeABI.encode(major: 0, minor: 255, patch: 255)))
    }

    func testTheDiagnosticNamesTheWindowTheActualVersionAndTheSelectedFile() {
        let message = CNAError.unsupportedABIVersion(
            admitted: NativeABI.admittedDescription,
            actual: NativeABI.describe(0x0000_0700),
            actualEncoded: 0x0000_0700,
            path: "/opt/cna/libcna_c_api.so"
        ).description
        XCTAssertTrue(message.contains("/opt/cna/libcna_c_api.so"), message)
        XCTAssertTrue(message.contains("0.7.0"), message)
        XCTAssertTrue(message.contains("0x00000700"), message)
        XCTAssertTrue(message.contains("major 0 with minor 35 or later"), message)
        XCTAssertTrue(message.contains("qualified against 0.35.0"), message)
    }

    /// Every manifest row names a distinct symbol, a distinct Swift property
    /// and a route type of its own, and each route type is the mechanical
    /// function of its symbol that `tools/native_abi/verify.py` re-derives.
    func testManifestRoutesAreDistinctAndDerivable() {
        // The count is asserted so a route cannot be added without a
        // deliberate edit here; the three checks below are what actually hold.
        XCTAssertEqual(nativeManifest.count, 778)
        XCTAssertEqual(Set(nativeManifest.map(\.symbol)).count, nativeManifest.count)
        XCTAssertEqual(Set(nativeManifest.map(\.swiftField)).count, nativeManifest.count)
        XCTAssertEqual(Set(nativeManifest.map(\.routeType)).count, nativeManifest.count)
        for entry in nativeManifest {
            let words = entry.symbol.dropFirst("cna_".count).split(separator: "_")
            let derived = words.map { $0.prefix(1).uppercased() + $0.dropFirst().lowercased() }
                .joined() + "Route"
            XCTAssertEqual(entry.routeType, derived, entry.symbol)
            XCTAssertTrue(entry.symbol.hasPrefix("cna_"), entry.symbol)
        }
    }
}
