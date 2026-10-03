// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// Foundation 44: `IVertexType` and the four `VertexPositionX` structs.
///
/// Every element quadruple, every name and every `ToString` template comes from
/// the pinned `Microsoft.Xna.Framework.Graphics.dll` `560080fc39021c61`. The
/// four strides are **not** transcribed: they are what Foundation 43's
/// maximum-end-offset rule computes, which is the cheapest end-to-end check
/// that the rule is right, because XNA's own class constructors rely on it.
final class Foundation44VertexTypeTests: XCTestCase {

    // MARK: - The four static declarations

    func testEachStaticDeclarationHasTheStrideTheRuleComputes() {
        XCTAssertEqual(G.VertexPositionColor.VertexDeclaration.VertexStride, 16)
        XCTAssertEqual(
            G.VertexPositionColorTexture.VertexDeclaration.VertexStride, 24)
        XCTAssertEqual(
            G.VertexPositionNormalTexture.VertexDeclaration.VertexStride, 32)
        XCTAssertEqual(G.VertexPositionTexture.VertexDeclaration.VertexStride, 20)
    }

    func testEachStaticDeclarationCarriesTheNameTheCctorSets() {
        XCTAssertEqual(
            G.VertexPositionColor.VertexDeclaration.Name,
            "VertexPositionColor.VertexDeclaration")
        XCTAssertEqual(
            G.VertexPositionColorTexture.VertexDeclaration.Name,
            "VertexPositionColorTexture.VertexDeclaration")
        XCTAssertEqual(
            G.VertexPositionNormalTexture.VertexDeclaration.Name,
            "VertexPositionNormalTexture.VertexDeclaration")
        XCTAssertEqual(
            G.VertexPositionTexture.VertexDeclaration.Name,
            "VertexPositionTexture.VertexDeclaration")
    }

    func testEachStaticDeclarationHasTheElementsTheCctorBuilds() throws {
        let expected: [(String, [(Int32, G.VertexElementFormat, G.VertexElementUsage)])] = [
            ("VertexPositionColor", [(0, .Vector3, .Position), (12, .Color, .Color)]),
            ("VertexPositionColorTexture", [
                (0, .Vector3, .Position), (12, .Color, .Color),
                (16, .Vector2, .TextureCoordinate)]),
            ("VertexPositionNormalTexture", [
                (0, .Vector3, .Position), (12, .Vector3, .Normal),
                (24, .Vector2, .TextureCoordinate)]),
            ("VertexPositionTexture", [
                (0, .Vector3, .Position), (12, .Vector2, .TextureCoordinate)]),
        ]
        let declarations = [
            G.VertexPositionColor.VertexDeclaration,
            G.VertexPositionColorTexture.VertexDeclaration,
            G.VertexPositionNormalTexture.VertexDeclaration,
            G.VertexPositionTexture.VertexDeclaration,
        ]
        for (declaration, (name, wanted)) in zip(declarations, expected) {
            let elements = try declaration.GetVertexElements()
            XCTAssertEqual(elements.count, wanted.count, name)
            for (element, (offset, format, usage)) in zip(elements, wanted) {
                XCTAssertEqual(element.Offset, offset, name)
                XCTAssertEqual(element.VertexElementFormat, format, name)
                XCTAssertEqual(element.VertexElementUsage, usage, name)
                XCTAssertEqual(element.UsageIndex, 0, name)
            }
        }
    }

    /// XNA's is a single `initonly` field, so every read is the same object and
    /// consuming code may rely on that identity.
    func testTheStaticDeclarationIsOneObjectPerType() {
        XCTAssertTrue(
            G.VertexPositionColor.VertexDeclaration
                === G.VertexPositionColor.VertexDeclaration)
        XCTAssertFalse(
            G.VertexPositionColor.VertexDeclaration
                === G.VertexPositionTexture.VertexDeclaration)
    }

    // MARK: - The explicit interface implementation

    /// Each struct implements `IVertexType` explicitly, and the witness returns
    /// the static field.
    func testTheWitnessReturnsTheStaticDeclaration() {
        let vertex = G.VertexPositionColor(F.Vector3(1, 2, 3), F.Color.White)
        XCTAssertTrue(
            vertex.VertexDeclaration === G.VertexPositionColor.VertexDeclaration)
        // Reached through the protocol, which is what the conformance is for.
        //
        // Through a GENERIC CONSTRAINT and not an existential:
        // `any Microsoft.Xna.Framework.Graphics.IVertexType` crashes
        // swift-frontend 6.0.3 in IR generation with
        // "Forming a CanType out of a non-canonical type!", so this toolchain
        // cannot compile an existential of this protocol at all. The
        // constraint form is equivalent here, is what a consumer should write,
        // and compiles. See
        // docs/foundation-44-vertex-types-evidence.md for the reproducer.
        XCTAssertTrue(
            Self.declaration(of: vertex) === G.VertexPositionColor.VertexDeclaration)
    }

    private static func declaration<T: G.IVertexType>(
        of vertex: T
    ) -> G.VertexDeclaration {
        vertex.VertexDeclaration
    }

    /// All four conform, reached through the constraint rather than an array of
    /// existentials, for the toolchain reason above.
    func testAllFourConformToIVertexType() {
        XCTAssertEqual(
            [
                Self.declaration(
                    of: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.White)),
                Self.declaration(of: G.VertexPositionColorTexture(
                    F.Vector3(0, 0, 0), F.Color.White, F.Vector2(0, 0))),
                Self.declaration(of: G.VertexPositionNormalTexture(
                    F.Vector3(0, 0, 0), F.Vector3(0, 1, 0), F.Vector2(0, 0))),
                Self.declaration(of: G.VertexPositionTexture(
                    F.Vector3(0, 0, 0), F.Vector2(0, 0))),
            ].map(\.VertexStride),
            [16, 24, 32, 20])
    }

    // MARK: - Value semantics

    func testTheConstructorsAssignAndNothingElse() {
        let vertex = G.VertexPositionNormalTexture(
            F.Vector3(1, 2, 3), F.Vector3(0, 1, 0), F.Vector2(0.25, 0.5))
        XCTAssertEqual(vertex.Position.X, 1)
        XCTAssertEqual(vertex.Normal.Y, 1)
        XCTAssertEqual(vertex.TextureCoordinate.Y, 0.5)
    }

    func testEqualityIsFieldByFieldAndEqualsRejectsOtherTypes() {
        let a = G.VertexPositionColor(F.Vector3(1, 2, 3), F.Color.White)
        let b = G.VertexPositionColor(F.Vector3(1, 2, 3), F.Color.White)
        let c = G.VertexPositionColor(F.Vector3(1, 2, 4), F.Color.White)
        XCTAssertTrue(a == b)
        XCTAssertFalse(a == c)
        XCTAssertTrue(a != c)
        XCTAssertTrue(a.Equals(b))
        XCTAssertFalse(a.Equals(nil))
        XCTAssertFalse(a.Equals("not a vertex"))
        XCTAssertFalse(
            a.Equals(G.VertexPositionTexture(F.Vector3(1, 2, 3), F.Vector2(0, 0))),
            "a different boxed runtime type is never equal")
    }

    /// `op_Equality` compares component floats, so a NaN component makes a
    /// vertex unequal to itself; `GetHashCode` goes through
    /// `SmartGetHashCode`, which is bitwise, so it still matches. The CLR has
    /// exactly this disagreement and it is reproduced rather than repaired.
    func testANaNComponentIsUnequalToItselfButHashesTheSame() {
        let nan = G.VertexPositionTexture(
            F.Vector3(Float.nan, 0, 0), F.Vector2(0, 0))
        XCTAssertFalse(nan == nan)
        XCTAssertEqual(nan.GetHashCode(), nan.GetHashCode())
    }

    /// `SmartGetHashCode` XORs the words and substitutes `0x7FFFFFFF` for a
    /// zero result, which an all-zero vertex is the way to reach.
    func testAnAllZeroVertexHashesToTheSubstitutedValue() {
        let zero = G.VertexPositionTexture(F.Vector3(0, 0, 0), F.Vector2(0, 0))
        XCTAssertEqual(zero.GetHashCode(), Int32.max)
    }

    /// Every word of every layout is folded. One component at a time is moved,
    /// so a hash that omitted any single word would still match on that word's
    /// own case — which is the defect a "change one field per struct" test
    /// misses and the mutation gate caught.
    func testTheHashFoldsEveryWordOfEveryLayout() {
        // The UNCHANGED vertex is the first entry of every list. Without it a
        // dropped word still passes: the case that varies the dropped
        // component simply reproduces the baseline hash, which is distinct
        // from all the others. Comparing against the baseline is what makes
        // "this component moves the hash" an assertion rather than
        // "these components differ from each other".
        func distinct(_ hashes: [Int32], _ label: String) {
            XCTAssertEqual(
                Set(hashes).count, hashes.count,
                "\(label): every single-component change must move the hash, "
                + "and none may reproduce the unchanged vertex's")
        }

        // VertexPositionNormalTexture — eight words.
        var eight: [Int32] = [G.VertexPositionNormalTexture(
            F.Vector3(1, 2, 3), F.Vector3(4, 5, 6), F.Vector2(7, 8)).GetHashCode()]
        for component in 0..<8 {
            var p: [Float] = [1, 2, 3]
            var n: [Float] = [4, 5, 6]
            var t: [Float] = [7, 8]
            switch component {
            case 0...2: p[component] += 100
            case 3...5: n[component - 3] += 100
            default: t[component - 6] += 100
            }
            eight.append(G.VertexPositionNormalTexture(
                F.Vector3(p[0], p[1], p[2]), F.Vector3(n[0], n[1], n[2]),
                F.Vector2(t[0], t[1])).GetHashCode())
        }
        distinct(eight, "VertexPositionNormalTexture")

        // VertexPositionTexture — five words.
        var five: [Int32] = [G.VertexPositionTexture(
            F.Vector3(1, 2, 3), F.Vector2(7, 8)).GetHashCode()]
        for component in 0..<5 {
            var p: [Float] = [1, 2, 3]
            var t: [Float] = [7, 8]
            if component < 3 { p[component] += 100 } else { t[component - 3] += 100 }
            five.append(G.VertexPositionTexture(
                F.Vector3(p[0], p[1], p[2]), F.Vector2(t[0], t[1])).GetHashCode())
        }
        distinct(five, "VertexPositionTexture")

        // VertexPositionColorTexture — six words, the fourth being the packed
        // colour rather than a float.
        var six: [Int32] = [G.VertexPositionColorTexture(
            F.Vector3(1, 2, 3), F.Color.White, F.Vector2(7, 8)).GetHashCode()]
        for component in 0..<6 {
            var p: [Float] = [1, 2, 3]
            var t: [Float] = [7, 8]
            var colour = F.Color.White
            switch component {
            case 0...2: p[component] += 100
            case 3: colour = F.Color.CornflowerBlue
            default: t[component - 4] += 100
            }
            six.append(G.VertexPositionColorTexture(
                F.Vector3(p[0], p[1], p[2]), colour,
                F.Vector2(t[0], t[1])).GetHashCode())
        }
        distinct(six, "VertexPositionColorTexture")

        // VertexPositionColor — four words.
        var four: [Int32] = [G.VertexPositionColor(
            F.Vector3(1, 2, 3), F.Color.White).GetHashCode()]
        for component in 0..<4 {
            var p: [Float] = [1, 2, 3]
            var colour = F.Color.White
            if component < 3 { p[component] += 100 } else { colour = F.Color.CornflowerBlue }
            four.append(G.VertexPositionColor(
                F.Vector3(p[0], p[1], p[2]), colour).GetHashCode())
        }
        distinct(four, "VertexPositionColor")
    }

    /// Every field of every struct participates in `op_Equality`. Two vertices
    /// differing in exactly one field must be unequal, for each field of each
    /// of the four — which is what stops a field being quietly dropped from
    /// the comparison of a struct the earlier test never built.
    func testEqualityUsesEveryFieldOfEveryStruct() {
        let p0 = F.Vector3(1, 2, 3), p1 = F.Vector3(9, 2, 3)
        let n0 = F.Vector3(4, 5, 6), n1 = F.Vector3(4, 9, 6)
        let t0 = F.Vector2(7, 8), t1 = F.Vector2(7, 9)

        XCTAssertFalse(
            G.VertexPositionColor(p0, F.Color.White)
                == G.VertexPositionColor(p1, F.Color.White), "Position")
        XCTAssertFalse(
            G.VertexPositionColor(p0, F.Color.White)
                == G.VertexPositionColor(p0, F.Color.CornflowerBlue), "Color")

        XCTAssertFalse(
            G.VertexPositionTexture(p0, t0) == G.VertexPositionTexture(p1, t0),
            "Position")
        XCTAssertFalse(
            G.VertexPositionTexture(p0, t0) == G.VertexPositionTexture(p0, t1),
            "TextureCoordinate")

        XCTAssertFalse(
            G.VertexPositionNormalTexture(p0, n0, t0)
                == G.VertexPositionNormalTexture(p1, n0, t0), "Position")
        XCTAssertFalse(
            G.VertexPositionNormalTexture(p0, n0, t0)
                == G.VertexPositionNormalTexture(p0, n1, t0), "Normal")
        XCTAssertFalse(
            G.VertexPositionNormalTexture(p0, n0, t0)
                == G.VertexPositionNormalTexture(p0, n0, t1), "TextureCoordinate")

        XCTAssertFalse(
            G.VertexPositionColorTexture(p0, F.Color.White, t0)
                == G.VertexPositionColorTexture(p1, F.Color.White, t0), "Position")
        XCTAssertFalse(
            G.VertexPositionColorTexture(p0, F.Color.White, t0)
                == G.VertexPositionColorTexture(p0, F.Color.CornflowerBlue, t0),
            "Color")
        XCTAssertFalse(
            G.VertexPositionColorTexture(p0, F.Color.White, t0)
                == G.VertexPositionColorTexture(p0, F.Color.White, t1),
            "TextureCoordinate")
    }

    // MARK: - ToString

    func testToStringUsesTheCompositeTemplates() {
        XCTAssertEqual(
            G.VertexPositionColor(F.Vector3(1, 2, 3), F.Color.White).ToString(),
            "{Position:\(F.Vector3(1, 2, 3).ToString()) "
                + "Color:\(F.Color.White.ToString())}")
        XCTAssertEqual(
            G.VertexPositionTexture(F.Vector3(1, 2, 3), F.Vector2(4, 5)).ToString(),
            "{Position:\(F.Vector3(1, 2, 3).ToString()) "
                + "TextureCoordinate:\(F.Vector2(4, 5).ToString())}")
        XCTAssertTrue(
            G.VertexPositionNormalTexture(
                F.Vector3(0, 0, 0), F.Vector3(0, 0, 0), F.Vector2(0, 0)
            ).ToString().contains("Normal:"))
        XCTAssertTrue(
            G.VertexPositionColorTexture(
                F.Vector3(0, 0, 0), F.Color.White, F.Vector2(0, 0)
            ).ToString().hasPrefix("{Position:"))
    }
}
