// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift-language qualification of the reference-return nullability projection.
// None of this is XNA runtime behaviour and none of it is counted as such: it
// measures the shape the Swift compiler actually emitted for the two axes the
// projection keeps apart.
final class Foundation23ProjectionTests: XCTestCase {
    typealias Created = Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs
    typealias Destroyed = Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs

    // A key path can only be written when the property's declared type matches
    // exactly, so these bindings are the compiler agreeing that a proven
    // nullable reference return is Optional and one the CIL does not prove
    // nullable is not. Swift additionally refuses a key path to a throwing
    // property, so forming either one is also the compiler agreeing that these
    // readers do not throw.
    func testDeclaredReturnTypesCarryTheProvenNullability() {
        let nullable: KeyPath<Created, Any?> = \Created.Resource
        let unproven: KeyPath<Destroyed, String> = \Destroyed.Name
        XCTAssertNotNil(nullable)
        XCTAssertNotNil(unproven)

        // Both are read-only: a CLR getter with no setter gives no writable
        // path, Optional or not.
        XCTAssertNil(nullable as? ReferenceWritableKeyPath<Created, Any?>)
        XCTAssertNil(unproven as? ReferenceWritableKeyPath<Destroyed, String>)
    }

    // Nullability is not fallibility. The Optional getter is reached with no
    // `try` in sight, and its nil is a value the caller tests rather than an
    // error the caller catches.
    func testOptionalReferenceReturnNeedsNoTry() {
        let empty = Created(resource: nil)
        let filled = Created(resource: "resource")

        // No `try`, no `do`/`catch`: the whole expression is non-throwing.
        let described = [empty, filled].map { $0.Resource == nil ? 0 : 1 }
        XCTAssertEqual(described, [0, 1])

        if let value = filled.Resource {
            XCTAssertEqual(value as? String, "resource")
        } else {
            XCTFail("a non-nil reference return did not unwrap")
        }
    }

    // A fallible member still requires `try` and still surfaces an error, and
    // the two axes compose rather than substitute: this reader is non-Optional
    // *and* throwing, which is one of the four legitimate combinations.
    func testFallibleReaderStillRequiresTry() throws {
        typealias F = Microsoft.Xna.Framework
        let keys = F.CurveKeyCollection()
        keys.Add(F.CurveKey(position: 1, value: 2))

        let reader: (F.CurveKeyCollection) -> (Int32) throws -> F.CurveKey = { collection in
            collection.Item
        }
        let key: F.CurveKey = try reader(keys)(0)
        XCTAssertEqual(key.Value, 2)
        XCTAssertThrowsError(try reader(keys)(1))
    }

    // `System.Object` arrives Optional from the type mapping itself, so a
    // proven-nullable `object` return must not be double-wrapped. The key path
    // is the proof: it only type-checks against `Any?`, and reading a nil
    // resource through it is nil rather than a wrapped nil.
    func testObjectReturnsAreNotDoubleWrapped() {
        let path: KeyPath<Created, Any?> = \Created.Resource
        XCTAssertNil(Created(resource: nil)[keyPath: path])
        XCTAssertNotNil(Created(resource: 1)[keyPath: path])
    }
}
