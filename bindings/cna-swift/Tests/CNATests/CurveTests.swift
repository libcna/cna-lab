// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

private final class CurveSubclassProbe: Microsoft.Xna.Framework.Curve {}
private final class CurveKeySubclassProbe: Microsoft.Xna.Framework.CurveKey {}
private final class CurveKeyCollectionSubclassProbe: Microsoft.Xna.Framework.CurveKeyCollection {}

extension PureValueTests {
    func testCurveEnumsAndClassReferenceSemantics() {
        typealias F = Microsoft.Xna.Framework
        XCTAssertEqual(F.CurveContinuity.Smooth.rawValue, 0)
        XCTAssertEqual(F.CurveContinuity.Step.rawValue, 1)
        XCTAssertEqual(F.CurveLoopType.Constant.rawValue, 0)
        XCTAssertEqual(F.CurveLoopType.Cycle.rawValue, 1)
        XCTAssertEqual(F.CurveLoopType.CycleOffset.rawValue, 2)
        XCTAssertEqual(F.CurveLoopType.Oscillate.rawValue, 3)
        XCTAssertEqual(F.CurveLoopType.Linear.rawValue, 4)
        XCTAssertEqual(F.CurveTangent.Flat.rawValue, 0)
        XCTAssertEqual(F.CurveTangent.Linear.rawValue, 1)
        XCTAssertEqual(F.CurveTangent.Smooth.rawValue, 2)

        let curve = F.Curve()
        let curveAlias = curve
        curveAlias.PreLoop = .Cycle
        XCTAssertTrue(curve === curveAlias)
        XCTAssertEqual(curve.PreLoop, .Cycle)

        let key = F.CurveKey(position: 1, value: 2)
        let keyAlias = key
        keyAlias.Value = 9
        XCTAssertTrue(key === keyAlias)
        XCTAssertEqual(key.Value, 9)

        let collection = F.CurveKeyCollection()
        let collectionAlias = collection
        collectionAlias.Add(key)
        XCTAssertTrue(collection === collectionAlias)
        XCTAssertEqual(collection.Count, 1)

        let curveSubclass = CurveSubclassProbe()
        let keySubclass = CurveKeySubclassProbe(position: 0, value: 0)
        let collectionSubclass = CurveKeyCollectionSubclassProbe()
        XCTAssertTrue(curveSubclass.Keys === curveSubclass.Keys)
        XCTAssertEqual(keySubclass.Position, 0)
        XCTAssertEqual(collectionSubclass.Count, 0)
    }

    func testCurveKeyConstructorsCloneEqualityHashAndCompare() throws {
        typealias F = Microsoft.Xna.Framework
        let basic = F.CurveKey(position: -0.0, value: 3.5)
        XCTAssertEqual(basic.Position.bitPattern, Float(-0.0).bitPattern)
        XCTAssertEqual(basic.Value.bitPattern, Float(3.5).bitPattern)
        XCTAssertEqual(basic.TangentIn.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(basic.TangentOut.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(basic.Continuity, .Smooth)

        let tangent = F.CurveKey(position: 2, value: 4, tangentIn: -3, tangentOut: 7)
        XCTAssertEqual(tangent.Continuity, .Smooth)
        XCTAssertEqual(tangent.TangentIn, -3)
        XCTAssertEqual(tangent.TangentOut, 7)

        let full = F.CurveKey(
            position: 2,
            value: 4,
            tangentIn: -3,
            tangentOut: 7,
            continuity: .Step
        )
        let cloned = full.Clone()
        XCTAssertFalse(cloned === full)
        XCTAssertTrue(cloned.Equals(full))
        cloned.Value = 19
        cloned.TangentIn = 23
        cloned.TangentOut = 29
        cloned.Continuity = .Smooth
        XCTAssertEqual(full.Value, 4)
        XCTAssertEqual(full.TangentIn, -3)
        XCTAssertEqual(full.TangentOut, 7)
        XCTAssertEqual(full.Continuity, .Step)
        XCTAssertEqual(cloned.Position, full.Position)

        let equal = F.CurveKey(
            position: 2,
            value: 4,
            tangentIn: -3,
            tangentOut: 7,
            continuity: .Step
        )
        XCTAssertTrue(full.Equals(equal))
        XCTAssertTrue(full.Equals(equal as Any))
        XCTAssertFalse(full.Equals(nil as F.CurveKey?))
        XCTAssertFalse(full.Equals("not a key" as Any))
        XCTAssertTrue(full == equal)
        XCTAssertFalse(full != equal)
        XCTAssertTrue((nil as F.CurveKey?) == nil)
        XCTAssertTrue((nil as F.CurveKey?) != full)

        let signedZero = F.CurveKey(
            position: 0,
            value: -0.0,
            tangentIn: 0,
            tangentOut: -0.0,
            continuity: .Smooth
        )
        let positiveZero = F.CurveKey(
            position: -0.0,
            value: 0,
            tangentIn: -0.0,
            tangentOut: 0,
            continuity: .Smooth
        )
        XCTAssertTrue(signedZero.Equals(positiveZero))
        XCTAssertEqual(signedZero.GetHashCode(), 0)
        XCTAssertEqual(positiveZero.GetHashCode(), 0)

        let hashKey = F.CurveKey(
            position: Float(bitPattern: 0x3F80_0000),
            value: Float(bitPattern: 0xC000_0000),
            tangentIn: Float(bitPattern: 0x4040_0000),
            tangentOut: Float(bitPattern: 0xC080_0000),
            continuity: .Step
        )
        XCTAssertEqual(
            hashKey.GetHashCode(),
            Int32(bitPattern: 0x3F80_0000)
                &+ Int32(bitPattern: 0xC000_0000)
                &+ Int32(bitPattern: 0x4040_0000)
                &+ Int32(bitPattern: 0xC080_0000)
                &+ 1
        )
        let nanPayload = F.CurveKey(position: Float(bitPattern: 0x7FC1_2345), value: 0)
        XCTAssertEqual(nanPayload.GetHashCode(), Int32(bitPattern: 0x7FC1_2345))
        XCTAssertFalse(nanPayload.Equals(nanPayload))
        XCTAssertFalse(nanPayload == nanPayload)

        XCTAssertEqual(try F.CurveKey(position: -2, value: 99).CompareTo(
            F.CurveKey(position: 4, value: -99)
        ), -1)
        XCTAssertEqual(try F.CurveKey(position: 4, value: 1).CompareTo(
            F.CurveKey(position: 4, value: 999)
        ), 0)
        XCTAssertEqual(try F.CurveKey(position: 6, value: 0).CompareTo(
            F.CurveKey(position: 4, value: 0)
        ), 1)
        XCTAssertEqual(try signedZero.CompareTo(positiveZero), 0)
        XCTAssertEqual(try positiveZero.CompareTo(signedZero), 0)
        XCTAssertEqual(try F.CurveKey(position: -.infinity, value: 0).CompareTo(
            F.CurveKey(position: .infinity, value: 0)
        ), -1)
        XCTAssertEqual(try F.CurveKey(position: .infinity, value: 0).CompareTo(
            F.CurveKey(position: -.infinity, value: 0)
        ), 1)

        let nan = F.CurveKey(position: .nan, value: 0)
        let finite = F.CurveKey(position: 1, value: 0)
        XCTAssertEqual(try nan.CompareTo(finite), 1)
        XCTAssertEqual(try finite.CompareTo(nan), 1)
        XCTAssertEqual(try nan.CompareTo(F.CurveKey(position: .nan, value: 0)), 1)
        // The IL has no null check at all: `ldfld` through a null reference
        // is what raises, and ECMA-335 names the class.
        assertProjected(
            CNANullReferenceException.self,
            message: "Object reference not set to an instance of an object.",
            hResult: Int32(bitPattern: 0x8000_4003)
        ) {
            _ = try finite.CompareTo(nil)
        }
    }

    func testCurveCollectionOrderingIndexerCopyCloneAndSearch() throws {
        typealias F = Microsoft.Xna.Framework
        let keys = F.CurveKeyCollection()
        XCTAssertEqual(keys.Count, 0)
        XCTAssertFalse(keys.IsReadOnly)

        let fiveA = F.CurveKey(position: 5, value: 10)
        let one = F.CurveKey(position: 1, value: 20)
        let fiveB = F.CurveKey(position: 5, value: 30)
        let three = F.CurveKey(position: 3, value: 40)
        keys.Add(fiveA)
        keys.Add(one)
        keys.Add(fiveB)
        keys.Add(three)
        keys.Add(fiveA)
        XCTAssertEqual(keys.Count, 5)
        XCTAssertTrue(try keys.Item(0) === one)
        XCTAssertTrue(try keys.Item(1) === three)
        XCTAssertTrue(try keys.Item(2) === fiveA)
        XCTAssertTrue(try keys.Item(3) === fiveB)
        XCTAssertTrue(try keys.Item(4) === fiveA)

        let negativeInfinity = F.CurveKey(position: -.infinity, value: 1)
        let positiveInfinity = F.CurveKey(position: .infinity, value: 2)
        keys.Add(positiveInfinity)
        keys.Add(negativeInfinity)
        XCTAssertTrue(try keys.Item(0) === negativeInfinity)
        XCTAssertTrue(try keys.Item(Int32(keys.Count - 1)) === positiveInfinity)

        let zeros = F.CurveKeyCollection()
        let positive = F.CurveKey(position: 0, value: 1)
        let negative = F.CurveKey(position: -0.0, value: 2)
        zeros.Add(positive)
        zeros.Add(negative)
        XCTAssertTrue(try zeros.Item(0) === positive)
        XCTAssertTrue(try zeros.Item(1) === negative)

        let nanFirst = F.CurveKey(position: .nan, value: 1)
        let nanSecond = F.CurveKey(position: .nan, value: 2)
        let nanOrder = F.CurveKeyCollection()
        nanOrder.Add(F.CurveKey(position: 4, value: 4))
        nanOrder.Add(nanFirst)
        nanOrder.Add(nanSecond)
        XCTAssertTrue(try nanOrder.Item(0) === nanSecond)
        XCTAssertTrue(try nanOrder.Item(1) === nanFirst)
        XCTAssertEqual(try nanOrder.Item(2).Position, 4)

        let fieldEqual = F.CurveKey(position: 5, value: 10)
        XCTAssertEqual(keys.IndexOf(fieldEqual), 3)
        XCTAssertTrue(keys.Contains(fieldEqual))
        XCTAssertFalse(keys.Contains(nil))
        XCTAssertEqual(keys.IndexOf(nil), -1)
        XCTAssertFalse(keys.Remove(nil))
        XCTAssertTrue(keys.Remove(fieldEqual))
        XCTAssertTrue(try keys.Item(3) === fiveB)
        XCTAssertFalse(keys.Remove(F.CurveKey(position: 5, value: 999)))

        let replacements = F.CurveKeyCollection()
        replacements.Add(F.CurveKey(position: 1, value: 1))
        replacements.Add(F.CurveKey(position: 3, value: 3))
        replacements.Add(F.CurveKey(position: 5, value: 5))
        let replacementSamePosition = F.CurveKey(position: 3, value: 77)
        try replacements.SetItem(1, replacementSamePosition)
        XCTAssertTrue(try replacements.Item(1) === replacementSamePosition)
        let replacementMovesLast = F.CurveKey(position: 100, value: 88)
        try replacements.SetItem(1, replacementMovesLast)
        XCTAssertTrue(try replacements.Item(Int32(replacements.Count - 1)) === replacementMovesLast)
        let replacementMovesFirst = F.CurveKey(position: -100, value: 99)
        try replacements.SetItem(Int32(replacements.Count - 1), replacementMovesFirst)
        XCTAssertTrue(try replacements.Item(0) === replacementMovesFirst)

        // The indexer delegates to List<CurveKey>.get_Item, so both ends of
        // the range report ArgumentOutOfRange_Index with paramName "index".
        for index in [Int32(-1), keys.Count] {
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "Index was out of range. Must be non-negative and less "
                    + "than the size of the collection.", paramName: "index"),
                paramName: "index",
                hResult: Int32(bitPattern: 0x8013_1502)
            ) {
                _ = try keys.Item(index)
            }
        }
        XCTAssertThrowsError(try keys.SetItem(-1, one))
        XCTAssertThrowsError(try keys.SetItem(keys.Count, one))
        XCTAssertThrowsError(try keys.RemoveAt(-1))
        XCTAssertThrowsError(try keys.RemoveAt(keys.Count))

        let sentinelA = F.CurveKey(position: -9, value: -9)
        let sentinelB = F.CurveKey(position: -8, value: -8)
        var destination = Array(repeating: sentinelA, count: Int(keys.Count) + 3)
        destination[destination.count - 1] = sentinelB
        try keys.CopyTo(&destination, arrayIndex: 1)
        XCTAssertTrue(destination[0] === sentinelA)
        for index in 0..<Int(keys.Count) {
            XCTAssertTrue(destination[index + 1] === (try keys.Item(Int32(index))))
        }
        XCTAssertTrue(destination[destination.count - 1] === sentinelB)
        XCTAssertThrowsError(try keys.CopyTo(&destination, arrayIndex: -1))
        XCTAssertThrowsError(try keys.CopyTo(&destination, arrayIndex: Int32(destination.count + 1)))
        var tooShort = Array(repeating: sentinelA, count: Int(keys.Count))
        XCTAssertThrowsError(try keys.CopyTo(&tooShort, arrayIndex: 1))
        var exact = Array(repeating: sentinelA, count: Int(keys.Count))
        try keys.CopyTo(&exact, arrayIndex: 0)
        XCTAssertTrue(exact[0] === (try keys.Item(0)))

        let empty = F.CurveKeyCollection()
        var emptyDestination: [F.CurveKey] = []
        try empty.CopyTo(&emptyDestination, arrayIndex: 0)
        XCTAssertThrowsError(try empty.CopyTo(&emptyDestination, arrayIndex: 1))

        let clone = keys.Clone()
        XCTAssertFalse(clone === keys)
        XCTAssertEqual(clone.Count, keys.Count)
        for index in 0..<keys.Count {
            XCTAssertTrue(try clone.Item(index) === keys.Item(index))
        }
        let shared = try keys.Item(0)
        shared.Value = 1234
        XCTAssertEqual(try clone.Item(0).Value, 1234)
        clone.Add(F.CurveKey(position: 200, value: 200))
        XCTAssertEqual(clone.Count, keys.Count + 1)
        XCTAssertFalse(keys.Contains(F.CurveKey(position: 200, value: 200)))

        let duplicateReplacement = F.CurveKeyCollection()
        let a = F.CurveKey(position: 2, value: 1)
        let b = F.CurveKey(position: 2, value: 2)
        let c = F.CurveKey(position: 2, value: 3)
        duplicateReplacement.Add(a)
        duplicateReplacement.Add(b)
        try duplicateReplacement.SetItem(0, c)
        XCTAssertTrue(try duplicateReplacement.Item(0) === c)
        XCTAssertTrue(try duplicateReplacement.Item(1) === b)
    }

    func testCurveCollectionEnumeratorLiveOrderFreshnessAndInvalidation() throws {
        typealias F = Microsoft.Xna.Framework

        func makeCollection() -> F.CurveKeyCollection {
            let result = F.CurveKeyCollection()
            result.Add(F.CurveKey(position: 1, value: 10))
            result.Add(F.CurveKey(position: 2, value: 20))
            result.Add(F.CurveKey(position: 3, value: 30))
            return result
        }

        let ordered = makeCollection()
        let firstEnumerator = ordered.GetEnumerator()
        let secondEnumerator = ordered.GetEnumerator()
        let first = try firstEnumerator.Next()
        let secondFreshFirst = try secondEnumerator.Next()
        XCTAssertTrue(first === secondFreshFirst)
        XCTAssertEqual(try firstEnumerator.Next()?.Position, 2)
        XCTAssertEqual(try firstEnumerator.Next()?.Position, 3)
        XCTAssertNil(try firstEnumerator.Next())
        XCTAssertNil(try firstEnumerator.Next())
        XCTAssertEqual(try secondEnumerator.Next()?.Position, 2)

        let add = makeCollection()
        let addEnumerator = add.GetEnumerator()
        add.Add(F.CurveKey(position: 4, value: 40))
        assertProjected(
            CNAInvalidOperationException.self,
            message: "Collection was modified; enumeration operation may not "
                + "execute.",
            hResult: Int32(bitPattern: 0x8013_1509)
        ) {
            _ = try addEnumerator.Next()
        }

        let successfulRemove = makeCollection()
        let successfulRemoveEnumerator = successfulRemove.GetEnumerator()
        XCTAssertTrue(successfulRemove.Remove(F.CurveKey(position: 2, value: 20)))
        XCTAssertThrowsError(try successfulRemoveEnumerator.Next())

        let failedRemove = makeCollection()
        let failedRemoveEnumerator = failedRemove.GetEnumerator()
        XCTAssertFalse(failedRemove.Remove(F.CurveKey(position: 2, value: 999)))
        XCTAssertEqual(try failedRemoveEnumerator.Next()?.Position, 1)

        let removeAt = makeCollection()
        let removeAtEnumerator = removeAt.GetEnumerator()
        try removeAt.RemoveAt(1)
        XCTAssertThrowsError(try removeAtEnumerator.Next())

        let failedRemoveAt = makeCollection()
        let failedRemoveAtEnumerator = failedRemoveAt.GetEnumerator()
        XCTAssertThrowsError(try failedRemoveAt.RemoveAt(-1))
        XCTAssertEqual(try failedRemoveAtEnumerator.Next()?.Position, 1)

        let clear = makeCollection()
        let clearEnumerator = clear.GetEnumerator()
        clear.Clear()
        XCTAssertThrowsError(try clearEnumerator.Next())

        let empty = F.CurveKeyCollection()
        let emptyEnumerator = empty.GetEnumerator()
        empty.Clear()
        XCTAssertThrowsError(try emptyEnumerator.Next())

        let replace = makeCollection()
        let replaceEnumerator = replace.GetEnumerator()
        try replace.SetItem(1, F.CurveKey(position: 2, value: 200))
        XCTAssertThrowsError(try replaceEnumerator.Next())

        let move = makeCollection()
        let moveEnumerator = move.GetEnumerator()
        try move.SetItem(1, F.CurveKey(position: 9, value: 200))
        XCTAssertThrowsError(try moveEnumerator.Next())

        let copy = makeCollection()
        let copyEnumerator = copy.GetEnumerator()
        let sentinel = F.CurveKey(position: -1, value: -1)
        var destination = Array(repeating: sentinel, count: 3)
        try copy.CopyTo(&destination, arrayIndex: 0)
        XCTAssertEqual(try copyEnumerator.Next()?.Position, 1)
        XCTAssertTrue(destination[0] === (try copy.Item(0)))

        let exhausted = makeCollection()
        let exhaustedEnumerator = exhausted.GetEnumerator()
        _ = try exhaustedEnumerator.Next()
        _ = try exhaustedEnumerator.Next()
        _ = try exhaustedEnumerator.Next()
        XCTAssertNil(try exhaustedEnumerator.Next())
        exhausted.Add(F.CurveKey(position: 4, value: 4))
        XCTAssertThrowsError(try exhaustedEnumerator.Next())
    }

    func testCurveDefaultsCloneTangentsAndInvalidIndexes() throws {
        typealias F = Microsoft.Xna.Framework
        let empty = F.Curve()
        XCTAssertEqual(empty.PreLoop, .Constant)
        XCTAssertEqual(empty.PostLoop, .Constant)
        XCTAssertTrue(empty.Keys === empty.Keys)
        XCTAssertTrue(empty.IsConstant)
        XCTAssertEqual(empty.Evaluate(-.infinity).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(empty.Evaluate(0).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(empty.Evaluate(.nan).bitPattern, Float(0).bitPattern)

        let only = F.CurveKey(position: 3, value: -7)
        empty.Keys.Add(only)
        XCTAssertTrue(empty.IsConstant)
        XCTAssertEqual(empty.Evaluate(-100), -7)
        XCTAssertEqual(empty.Evaluate(3), -7)
        XCTAssertEqual(empty.Evaluate(100), -7)
        empty.Keys.Add(F.CurveKey(position: 5, value: -7))
        XCTAssertFalse(empty.IsConstant)

        let curve = F.Curve()
        let first = F.CurveKey(position: 1, value: 2)
        let middle = F.CurveKey(position: 4, value: 8)
        let last = F.CurveKey(position: 10, value: 5)
        curve.Keys.Add(first)
        curve.Keys.Add(middle)
        curve.Keys.Add(last)

        try curve.ComputeTangent(1, tangentType: .Flat)
        XCTAssertEqual(middle.TangentIn.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(middle.TangentOut.bitPattern, Float(0).bitPattern)

        try curve.ComputeTangent(0, tangentType: .Linear)
        try curve.ComputeTangent(1, tangentType: .Linear)
        try curve.ComputeTangent(2, tangentType: .Linear)
        XCTAssertEqual(first.TangentIn, 0)
        XCTAssertEqual(first.TangentOut, 6)
        XCTAssertEqual(middle.TangentIn, 6)
        XCTAssertEqual(middle.TangentOut, -3)
        XCTAssertEqual(last.TangentIn, -3)
        XCTAssertEqual(last.TangentOut, 0)

        curve.ComputeTangents(.Smooth)
        XCTAssertEqual(first.TangentIn, 0)
        XCTAssertEqual(first.TangentOut, 6)
        XCTAssertEqual(middle.TangentIn, 1)
        XCTAssertEqual(middle.TangentOut, 2)
        XCTAssertEqual(last.TangentIn, -3)
        XCTAssertEqual(last.TangentOut.bitPattern, Float(-0.0).bitPattern)

        try curve.ComputeTangent(1, tangentInType: .Flat, tangentOutType: .Smooth)
        XCTAssertEqual(middle.TangentIn, 0)
        XCTAssertEqual(middle.TangentOut, 2)
        try curve.ComputeTangent(1, tangentInType: .Linear, tangentOutType: .Flat)
        XCTAssertEqual(middle.TangentIn, 6)
        XCTAssertEqual(middle.TangentOut, 0)
        try curve.ComputeTangent(1, tangentInType: .Smooth, tangentOutType: .Linear)
        XCTAssertEqual(middle.TangentIn, 1)
        XCTAssertEqual(middle.TangentOut, -3)

        curve.ComputeTangents(.Flat, tangentOutType: .Linear)
        XCTAssertEqual(first.TangentIn, 0)
        XCTAssertEqual(first.TangentOut, 6)
        XCTAssertEqual(middle.TangentIn, 0)
        XCTAssertEqual(middle.TangentOut, -3)
        XCTAssertEqual(last.TangentIn, 0)
        XCTAssertEqual(last.TangentOut, 0)
        curve.ComputeTangents(.Flat, tangentOutType: .Linear)
        XCTAssertEqual(middle.TangentOut, -3)

        let tiny = F.Curve()
        tiny.Keys.Add(F.CurveKey(position: 0, value: 0))
        tiny.Keys.Add(F.CurveKey(position: 1, value: Float.ulpOfOne / 2))
        try tiny.ComputeTangent(0, tangentType: .Smooth)
        XCTAssertEqual(try tiny.Keys.Item(0).TangentOut.bitPattern, Float(0).bitPattern)

        let duplicate = F.Curve()
        duplicate.Keys.Add(F.CurveKey(position: 2, value: 1))
        duplicate.Keys.Add(F.CurveKey(position: 2, value: 3))
        try duplicate.ComputeTangent(0, tangentType: .Smooth)
        XCTAssertTrue(try duplicate.Keys.Item(0).TangentOut.isNaN)

        // The paramName-ONLY constructor: the message is the substituted
        // Arg_ArgumentOutOfRangeException, composed with the parameter name.
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Specified argument was out of the range of valid values.",
                paramName: "keyIndex"),
            paramName: "keyIndex",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            try curve.ComputeTangent(-1, tangentType: .Flat)
        }
        XCTAssertThrowsError(try curve.ComputeTangent(curve.Keys.Count, tangentType: .Flat))

        curve.PreLoop = .Oscillate
        curve.PostLoop = .CycleOffset
        _ = curve.Evaluate(20)
        let clone = curve.Clone()
        XCTAssertFalse(clone === curve)
        XCTAssertFalse(clone.Keys === curve.Keys)
        XCTAssertEqual(clone.PreLoop, .Oscillate)
        XCTAssertEqual(clone.PostLoop, .CycleOffset)
        XCTAssertEqual(clone.Keys.Count, curve.Keys.Count)
        for index in 0..<curve.Keys.Count {
            XCTAssertTrue(try clone.Keys.Item(index) === curve.Keys.Item(index))
        }
        let sharedKey = try curve.Keys.Item(0)
        sharedKey.Value = 444
        XCTAssertEqual(try clone.Keys.Item(0).Value, 444)
        clone.Keys.Add(F.CurveKey(position: 100, value: 100))
        XCTAssertEqual(clone.Keys.Count, curve.Keys.Count + 1)
    }

    func testCurveEvaluateSegmentsStepHermitePrecisionAndLoops() throws {
        typealias F = Microsoft.Xna.Framework

        let segment = F.Curve()
        segment.Keys.Add(F.CurveKey(position: 0, value: 0))
        segment.Keys.Add(F.CurveKey(position: 1, value: 10))
        segment.Keys.Add(F.CurveKey(position: 2, value: 20))
        XCTAssertEqual(segment.Evaluate(0), 0)
        XCTAssertEqual(segment.Evaluate(1), 10)
        XCTAssertEqual(segment.Evaluate(2), 20)
        XCTAssertLessThan(segment.Evaluate(Float(bitPattern: 0x3F7F_FFFF)), 10)
        XCTAssertGreaterThan(segment.Evaluate(1.01), 10)

        let duplicates = F.Curve()
        let duplicateFirst = F.CurveKey(position: 2, value: 3)
        duplicateFirst.Continuity = .Step
        duplicates.Keys.Add(duplicateFirst)
        duplicates.Keys.Add(F.CurveKey(position: 2, value: 9))
        XCTAssertEqual(duplicates.Evaluate(2), 3)
        XCTAssertEqual(duplicates.Evaluate(1), 3)
        XCTAssertEqual(duplicates.Evaluate(3), 9)

        let step = F.Curve()
        let stepFirst = F.CurveKey(position: 2, value: -4)
        stepFirst.Continuity = .Step
        step.Keys.Add(stepFirst)
        step.Keys.Add(F.CurveKey(position: 6, value: 12))
        XCTAssertEqual(step.Evaluate(2), -4)
        XCTAssertEqual(step.Evaluate(3), -4)
        XCTAssertEqual(step.Evaluate(Float(bitPattern: Float(6).bitPattern - 1)), -4)
        XCTAssertEqual(step.Evaluate(6), 12)

        let hermite = F.Curve()
        hermite.Keys.Add(F.CurveKey(
            position: 2,
            value: 3.25,
            tangentIn: 0,
            tangentOut: 1.75
        ))
        hermite.Keys.Add(F.CurveKey(
            position: 7,
            value: -4.5,
            tangentIn: -2.25,
            tangentOut: 0
        ))
        XCTAssertEqual(hermite.Evaluate(3.375).bitPattern, 0x400C_2F5B)

        let widened = F.Curve()
        widened.Keys.Add(F.CurveKey(
            position: Float(bitPattern: 0xC75E_47C4), value: 0
        ))
        widened.Keys.Add(F.CurveKey(
            position: Float(bitPattern: 0x4619_4550), value: 1
        ))
        let widenedResult = widened.Evaluate(Float(bitPattern: 0x44A2_282C))
        XCTAssertEqual(widenedResult.bitPattern, 0x3F74_8F88)
        XCTAssertNotEqual(widenedResult.bitPattern, 0x3F74_8F84)

        let special = F.Curve()
        special.Keys.Add(F.CurveKey(position: 0, value: -0.0))
        special.Keys.Add(F.CurveKey(position: 1, value: .nan))
        XCTAssertEqual(special.Evaluate(-1).bitPattern, Float(-0.0).bitPattern)
        XCTAssertTrue(special.Evaluate(2).isNaN)
        XCTAssertTrue(segment.Evaluate(.nan).isNaN)
    }

    func testCurveLoopsConstantLinearCycleOffsetOscillateAndBoundaries() {
        typealias F = Microsoft.Xna.Framework
        let loops = F.Curve()
        loops.Keys.Add(F.CurveKey(
            position: 2,
            value: 10,
            tangentIn: 3,
            tangentOut: 0
        ))
        loops.Keys.Add(F.CurveKey(
            position: 5,
            value: 22,
            tangentIn: 0,
            tangentOut: -4
        ))

        XCTAssertEqual(loops.Evaluate(0), 10)
        XCTAssertEqual(loops.Evaluate(7), 22)
        loops.PreLoop = .Linear
        loops.PostLoop = .Linear
        XCTAssertEqual(loops.Evaluate(0), 4)
        XCTAssertEqual(loops.Evaluate(7), 14)

        loops.PreLoop = .Cycle
        loops.PostLoop = .Cycle
        XCTAssertEqual(loops.Evaluate(1).bitPattern, loops.Evaluate(4).bitPattern)
        XCTAssertEqual(loops.Evaluate(6).bitPattern, loops.Evaluate(3).bitPattern)
        XCTAssertEqual(loops.Evaluate(-1).bitPattern, loops.Evaluate(5).bitPattern)
        XCTAssertEqual(loops.Evaluate(8).bitPattern, loops.Evaluate(2).bitPattern)

        loops.PreLoop = .CycleOffset
        loops.PostLoop = .CycleOffset
        XCTAssertEqual(loops.Evaluate(1), loops.Evaluate(4) - 12)
        XCTAssertEqual(loops.Evaluate(-1), loops.Evaluate(5) - 24)
        XCTAssertEqual(loops.Evaluate(6), loops.Evaluate(3) + 12)
        XCTAssertEqual(loops.Evaluate(8), loops.Evaluate(2) + 24)

        loops.PreLoop = .Oscillate
        loops.PostLoop = .Oscillate
        XCTAssertEqual(loops.Evaluate(1).bitPattern, loops.Evaluate(3).bitPattern)
        XCTAssertEqual(loops.Evaluate(-2).bitPattern, loops.Evaluate(4).bitPattern)
        XCTAssertEqual(loops.Evaluate(-1).bitPattern, loops.Evaluate(5).bitPattern)
        XCTAssertEqual(loops.Evaluate(6).bitPattern, loops.Evaluate(4).bitPattern)
        XCTAssertEqual(loops.Evaluate(8).bitPattern, loops.Evaluate(2).bitPattern)
        XCTAssertTrue(loops.Evaluate(.infinity).isNaN)
        XCTAssertTrue(loops.Evaluate(Float.greatestFiniteMagnitude).isNaN)
        XCTAssertTrue(loops.Evaluate(-Float.greatestFiniteMagnitude).isNaN)
    }
}
