// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for reference-return nullability, transcribed
// from the registered, hash-matched Microsoft.Xna.Framework.Graphics.dll IL.
//
// A normal return of null and a thrown exception are different observations of
// XNA, and they stay different here: nothing below merges them into one
// "failure" bucket.
extension PureValueTests {
    // `ResourceCreatedEventArgs..ctor(object resource)` is
    // `ldarg.0; call EventArgs::.ctor(); ldarg.0; ldarg.1; stfld _resource;
    // ret`, and `get_Resource` is `ldarg.0; ldfld _resource; ret`. The stored
    // object is returned verbatim, so a null resource is observably null and
    // nothing intervenes.
    func testResourceCreatedEventArgsReturnsItsFieldVerbatim() {
        typealias Args = Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs

        XCTAssertNil(Args(resource: nil).Resource)

        let resource = Microsoft.Xna.Framework.CurveKeyCollection()
        let carried = Args(resource: resource)
        XCTAssertNotNil(carried.Resource)
        XCTAssertTrue(carried.Resource as AnyObject === resource)
    }

    // Why that null is a *normal* result and not an error state:
    // `GraphicsDevice.FireCreatedEvent(object resource)` keeps one cached
    // `ResourceCreatedEventArgs`, writes the resource into `_resource` before
    // raising the event, and then executes `ldnull; stfld _resource` at
    // IL_003c/IL_003d once the handlers have returned, so the cached instance
    // does not keep the resource alive. A handler that retains the args and
    // reads `Resource` afterwards observes null through a getter that cannot
    // fail. That is the whole reason the CLR return is nullable.
    //
    // The mirror case is the evidence that this is a measured fact rather than
    // a habit: `FireDestroyedEvent` overwrites `_name` and `_tag` on the same
    // cached-args path and never nulls either afterwards, so neither
    // `ResourceDestroyedEventArgs.Name` nor `.Tag` is proven nullable, and
    // neither is projected Optional on that ground.
    func testResourceDestroyedEventArgsReturnsItsFieldsVerbatim() {
        typealias Args = Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs

        // `.ctor(string name, object tag)` stores `tag` first (IL_0008) and
        // `name` second (IL_000f); both getters are a bare `ldfld`.
        let args = Args(name: "surface", tag: nil)
        XCTAssertEqual(args.Name, "surface")
        XCTAssertNil(args.Tag)

        let tagged = Args(name: "", tag: Microsoft.Xna.Framework.Vector3(1, 2, 3))
        XCTAssertEqual(tagged.Name, "")
        let carried = tagged.Tag as? Microsoft.Xna.Framework.Vector3
        XCTAssertEqual(carried?.X, 1)
        XCTAssertEqual(carried?.Y, 2)
        XCTAssertEqual(carried?.Z, 3)
    }

    // Null and failure are two outcomes, not two spellings of one. XNA answers
    // "no intersection" with a `Nullable<Single>` whose `HasValue` is false --
    // a normal return through a method that has no throw at all -- and answers
    // an out-of-range index with a thrown `ArgumentOutOfRangeException`. Both
    // are observed here, and the observation that distinguishes them is that
    // the first produces a value the caller can test and the second produces
    // no value at all.
    func testNullResultAndThrownFailureRemainDistinct() throws {
        typealias F = Microsoft.Xna.Framework

        let sphere = try F.BoundingSphere(F.Vector3(0, 0, 0), 1)
        let hit = F.Ray(F.Vector3(0, 0, -5), F.Vector3(0, 0, 1)).Intersects(sphere)
        let miss = F.Ray(F.Vector3(0, 0, -5), F.Vector3(0, 1, 0)).Intersects(sphere)
        XCTAssertEqual(hit, 4)
        XCTAssertNil(miss)

        let keys = F.CurveKeyCollection()
        keys.Add(F.CurveKey(position: 1, value: 2))
        XCTAssertEqual(try keys.Item(0).Value, 2)
        var thrown: Error?
        do {
            _ = try keys.Item(1)
        } catch {
            thrown = error
        }
        guard let failure = thrown as? CNAArgumentOutOfRangeException else {
            return XCTFail("the out-of-range index did not produce an XNA failure")
        }
        XCTAssertEqual(failure.ParamName, "index")
        XCTAssertEqual(failure.HResult, Int32(bitPattern: 0x8013_1502))

        // The two outcomes are not interchangeable: the miss carried a normal
        // result and raised nothing, and the out-of-range index raised and
        // carried nothing.
        XCTAssertNotNil(thrown)
        XCTAssertTrue(miss == nil && hit != nil)
    }
}
