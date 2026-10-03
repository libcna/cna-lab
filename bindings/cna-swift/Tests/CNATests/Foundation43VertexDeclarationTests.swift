// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias G = Microsoft.Xna.Framework.Graphics
private typealias V = Microsoft.Xna.Framework.Graphics.VertexElement

/// Foundation 43: `VertexDeclaration` and the internal `VertexElementValidator`.
///
/// Every value and every message here is read out of the pinned
/// `Microsoft.Xna.Framework.Graphics.dll` `560080fc39021c61` and
/// `Microsoft.Xna.Framework.dll`. CNA also computes a vertex stride; it was not
/// consulted.
final class Foundation43VertexDeclarationTests: XCTestCase {

    private let argumentHResult = Int32(bitPattern: 0x8007_0057)
    private let rangeHResult = Int32(bitPattern: 0x8013_1502)

    private func element(
        _ offset: Int32,
        _ format: G.VertexElementFormat,
        _ usage: G.VertexElementUsage,
        _ usageIndex: Int32 = 0
    ) -> V {
        V(offset, format, usage, usageIndex)
    }

    // MARK: - GetTypeSize

    /// The switch in `VertexElementValidator.GetTypeSize`, every arm.
    func testTypeSizeIsTheSwitchTheILDeclares() {
        let expected: [(G.VertexElementFormat, Int32)] = [
            (.Single, 4), (.Vector2, 8), (.Vector3, 12), (.Vector4, 16),
            (.Color, 4), (.Byte4, 4), (.Short2, 4), (.Short4, 8),
            (.NormalizedShort2, 4), (.NormalizedShort4, 8),
            (.HalfVector2, 4), (.HalfVector4, 8),
        ]
        XCTAssertEqual(expected.count, 12, "every VertexElementFormat is covered")
        for (format, size) in expected {
            XCTAssertEqual(
                G.VertexElementValidator.typeSize(format), size,
                String(describing: format))
        }
    }

    // MARK: - GetVertexStride

    /// The stride is the **maximum end offset**, not a sum and not the last
    /// element's end. Elements may be given in any order and may overlap.
    func testStrideIsTheMaximumEndOffsetAndNotASum() {
        // Given out of order, the widest end still wins.
        XCTAssertEqual(
            G.VertexElementValidator.vertexStride([
                element(16, .Single, .Color),
                element(0, .Vector3, .Position),
            ]),
            20)
        // Overlapping elements are not summed.
        XCTAssertEqual(
            G.VertexElementValidator.vertexStride([
                element(0, .Vector4, .Position),
                element(0, .Single, .Fog),
            ]),
            16)
        XCTAssertEqual(G.VertexElementValidator.vertexStride([]), 0)
    }

    // MARK: - The constructors

    func testAnEmptyElementArrayIsAcceptedInSilence() throws {
        // The IL's two `leave` instructions: a null or empty array leaves the
        // declaration with no elements and a stride of zero, with no complaint
        // and with the supplied stride discarded.
        let declaration = try G.VertexDeclaration(
            vertexStride: 64, elements: [])
        XCTAssertEqual(declaration.VertexStride, 0)
        XCTAssertFalse(declaration.IsDisposed)
        XCTAssertNil(declaration.Name)
    }

    /// `GetVertexElements` dereferences `_elements`, which is exactly the
    /// state the empty constructor leaves, so the CLR's own `callvirt` raises
    /// `NullReferenceException`.
    func testGetVertexElementsOnAnEmptyDeclarationRaisesNullReference() throws {
        let declaration = try G.VertexDeclaration(
            vertexStride: 64, elements: [])
        XCTAssertThrowsError(try declaration.GetVertexElements()) { error in
            XCTAssertTrue(error is CNANullReferenceException, "\(error)")
        }
    }

    func testTheStrideOnlyConstructorComputesTheStride() throws {
        let declaration = try G.VertexDeclaration(elements: [
            element(0, .Vector3, .Position),
            element(12, .Color, .Color),
        ])
        XCTAssertEqual(declaration.VertexStride, 16)
        XCTAssertEqual(try declaration.GetVertexElements().count, 2)
    }

    func testTheElementsAreCopiedAndTheResultIsACopyToo() throws {
        var source = [element(0, .Vector3, .Position),
                      element(12, .Color, .Color)]
        let declaration = try G.VertexDeclaration(
            vertexStride: 16, elements: source)
        source[0] = element(0, .Single, .Fog)
        var read = try declaration.GetVertexElements()
        read[1] = element(12, .Single, .Fog)
        let again = try declaration.GetVertexElements()
        XCTAssertEqual(again[0].VertexElementUsage, .Position,
                       "the constructor copied the caller's array")
        XCTAssertEqual(again[1].VertexElementUsage, .Color,
                       "GetVertexElements returned a copy")
    }

    // MARK: - Validation, in the order the IL checks it

    /// The stride guard runs before anything is looked at.
    func testANonPositiveStrideIsOutOfRange() {
        for stride: Int32 in [0, -4] {
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "Specified argument was out of the range of valid values.",
                    paramName: "vertexStride"),
                paramName: "vertexStride",
                hResult: rangeHResult
            ) {
                _ = try G.VertexDeclaration(
                    vertexStride: stride,
                    elements: [self.element(0, .Single, .Position)])
            }
        }
    }

    func testAStrideThatIsNotAMultipleOfFourIsRefused() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Vertex stride and "
                + "VertexElement.Offset must be multiples of four.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(
                vertexStride: 6, elements: [self.element(0, .Single, .Position)])
        }
    }

    func testAnElementThatDoesNotFitTheStrideIsRefused() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Element Position0 does not fit "
                + "within the specified vertex stride.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(
                vertexStride: 8,
                elements: [self.element(0, .Vector4, .Position)])
        }
    }

    func testANegativeOffsetIsReportedAsOutsideTheStride() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Element Color2 does not fit "
                + "within the specified vertex stride.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(
                vertexStride: 16,
                elements: [self.element(-4, .Single, .Color, 2)])
        }
    }

    func testAnElementOffsetThatIsNotAMultipleOfFourIsRefused() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Vertex stride and "
                + "VertexElement.Offset must be multiples of four.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(
                vertexStride: 16,
                elements: [self.element(2, .Single, .Position)])
        }
    }

    func testTwoElementsSharingAUsageAndIndexAreDuplicates() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Duplicate element TextureCoordinate1.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(vertexStride: 16, elements: [
                self.element(0, .Single, .TextureCoordinate, 1),
                self.element(4, .Single, .TextureCoordinate, 1),
            ])
        }
    }

    /// The same usage with different indices is legal, which is what makes the
    /// duplicate check a pair test rather than a usage test.
    func testTheSameUsageWithDifferentIndicesIsLegal() throws {
        let declaration = try G.VertexDeclaration(vertexStride: 16, elements: [
            element(0, .Single, .TextureCoordinate, 0),
            element(4, .Single, .TextureCoordinate, 1),
        ])
        XCTAssertEqual(try declaration.GetVertexElements().count, 2)
    }

    /// The overlap message names **both** elements: the one that already owned
    /// the byte first, then the one that collided with it.
    func testOverlappingElementsNameBothOffenders() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Elements Position0 and Color3 "
                + "are overlapping.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(vertexStride: 16, elements: [
                self.element(0, .Vector2, .Position, 0),
                self.element(4, .Single, .Color, 3),
            ])
        }
    }

    // MARK: - The order of the checks is observable

    /// An element that is both misaligned and outside the stride reports the
    /// **stride** message, because that check runs first.
    func testOutsideStrideBeatsMisalignment() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Element Normal0 does not fit "
                + "within the specified vertex stride.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(
                vertexStride: 8,
                elements: [self.element(6, .Vector4, .Normal)])
        }
    }

    /// An element that is both misaligned and overlapping reports the
    /// **alignment** message, because that check runs first.
    func testMisalignmentBeatsOverlap() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Vertex stride and "
                + "VertexElement.Offset must be multiples of four.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(vertexStride: 16, elements: [
                self.element(0, .Vector2, .Position),
                self.element(2, .Single, .Color),
            ])
        }
    }

    /// A pair that is both a duplicate and an overlap reports the
    /// **duplicate** message, because that check runs first.
    func testDuplicateBeatsOverlap() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Duplicate element Position0.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(vertexStride: 16, elements: [
                self.element(0, .Single, .Position),
                self.element(0, .Single, .Position),
            ])
        }
    }

    /// The one-argument constructor computes the stride and then validates it,
    /// so a layout the caller never sized is still refused — and by the
    /// **stride** message, not the element one, because the computed stride is
    /// checked before any element is looked at.
    ///
    /// Every `GetTypeSize` answer is a multiple of four, so a computed stride
    /// can only be misaligned when an element's offset is. One element at
    /// offset 2 gives a computed stride of 6.
    func testAComputedStrideIsValidatedToo() {
        assertProjected(
            CNAArgumentException.self,
            message: "Invalid VertexDeclaration. Vertex stride and "
                + "VertexElement.Offset must be multiples of four.",
            hResult: argumentHResult
        ) {
            _ = try G.VertexDeclaration(elements: [
                self.element(2, .Single, .Position),
            ])
        }
    }

    /// The complement: a layout the caller never sized and that IS well formed
    /// is accepted, with the stride the maximum end offset gives.
    func testAWellFormedComputedStrideIsAccepted() throws {
        let declaration = try G.VertexDeclaration(elements: [
            element(0, .Vector3, .Position),
            element(12, .Short2, .Color),
        ])
        XCTAssertEqual(declaration.VertexStride, 16)
    }

    // MARK: - Shape

    func testItIsAGraphicsResourceWithNoNativeObject() throws {
        let declaration = try G.VertexDeclaration(vertexStride: 16, elements: [
            element(0, .Vector4, .Position),
        ])
        XCTAssertTrue((declaration as Any) is G.GraphicsResource)
        XCTAssertFalse((declaration as Any) is G.Texture)
        XCTAssertNil(declaration.GraphicsDevice)
        XCTAssertNil(declaration.Tag)
    }

    func testDisposalIsIdempotentAndRaisesDisposingOnce() throws {
        let declaration = try G.VertexDeclaration(vertexStride: 16, elements: [
            element(0, .Vector4, .Position),
        ])
        var raises = 0
        _ = declaration.Disposing.Add { _, _ in raises += 1 }
        try declaration.Dispose()
        XCTAssertTrue(declaration.IsDisposed)
        XCTAssertEqual(raises, 1)
        try declaration.Dispose()
        XCTAssertEqual(raises, 1)
        // The elements survive disposal: XNA's Dispose(bool) only unbinds.
        XCTAssertEqual(try declaration.GetVertexElements().count, 1)
    }
}
