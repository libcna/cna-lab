// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A game that reads a disposed resource inside `LoadContent`, where a
/// callback-scoped graphics device exists.
private final class DisposedProbeGame: Microsoft.Xna.Framework.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var sawCNAError = false
    var seenClass: String?
    var seenObjectName: String?
    var seenMessage: String?
    var seenHResult: Int32?
    var batchObjectName: String?
    var batchMessage: String?

    override init() throws {
        try super.init()
        manager = try F.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant(
                    "the registered graphics device service produced no device")
            }
            // `RenderTarget2D` is the publicly constructible resource, and
            // it is also the one whose derived name the storage records — so
            // this pins both the class and the name in one probe.
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 8, height: 8)
            try target.Dispose()
            do {
                _ = try target.validatedHandle("probe")
                failure = CNAError.producerInvariant(
                    "reading a disposed RenderTarget2D succeeded")
            } catch let error as CNAObjectDisposedException {
                seenClass = String(describing: type(of: error))
                seenObjectName = error.ObjectName
                seenMessage = error.Message
                seenHResult = error.HResult
            } catch is CNAError {
                sawCNAError = true
            }

            // Every graphics resource guards the same way and reports the same
            // class. SpriteBatch reached its handle through the storage
            // directly until this milestone, so it alone still answered on the
            // CNA runtime channel while its neighbours did not.
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Dispose()
            do {
                try batch.Begin()
                failure = CNAError.producerInvariant(
                    "Begin on a disposed SpriteBatch succeeded")
            } catch let error as CNAObjectDisposedException {
                batchObjectName = error.ObjectName
                batchMessage = error.Message
            } catch is CNAError {
                sawCNAError = true
            }
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws {
        try Exit()
    }
}

/// Foundation 42a: `System.ObjectDisposedException` as a projected payload.
///
/// Every value here is read out of the admitted `mscorlib`
/// `5634668d…acc63` — the class shape from its metadata, the two messages from
/// its embedded string table, and the HResult from the `SetErrorCode` literal
/// in its constructors.
final class Foundation42ObjectDisposedTests: XCTestCase {

    private let objectDisposedHResult = Int32(bitPattern: 0x8013_1622)

    // MARK: - The payload

    /// `.ctor(String objectName)` forwards to `.ctor(objectName, ObjectDisposed_Generic)`,
    /// so the single-argument constructor takes an **object name** and not a
    /// message. Getting that backwards is the mistake this family invites.
    func testSingleArgumentConstructorTakesAnObjectNameNotAMessage() {
        assertProjected(
            CNAObjectDisposedException.self,
            message: "Cannot access a disposed object.\r\nObject name: 'Texture2D'.",
            hResult: objectDisposedHResult
        ) { throw CNAObjectDisposedException(objectName: "Texture2D") }
    }

    func testDesignatedConstructorComposesTheCallersOwnMessage() {
        assertProjected(
            CNAObjectDisposedException.self,
            message: "custom\r\nObject name: 'SpriteBatch'.",
            hResult: objectDisposedHResult
        ) {
            throw CNAObjectDisposedException(objectName: "SpriteBatch", message: "custom")
        }
    }

    /// The third public constructor takes a **message** first and stores no
    /// object name, which is why it cannot share a label with the one above.
    func testInnerExceptionConstructorStoresNoObjectName() {
        let inner = CNAInvalidOperationException(message: "inner")
        let error = CNAObjectDisposedException(message: "outer", innerException: inner)
        XCTAssertEqual(error.ObjectName, "")
        XCTAssertEqual(error.Message, "outer", "an empty ObjectName appends nothing")
        XCTAssertEqual(error.HResult, objectDisposedHResult)
        XCTAssertTrue(error.InnerException === inner)
    }

    /// `get_ObjectName` is `objectName ?? String.Empty` — it is proven never to
    /// answer null, unlike `ArgumentException.ParamName`, which returns its
    /// field as it stands.
    func testObjectNameSubstitutesEmptyForNilWhereParamNameDoesNot() {
        XCTAssertEqual(CNAObjectDisposedException(objectName: nil).ObjectName, "")
        XCTAssertNil(CNAArgumentException(message: "m", paramName: nil).ParamName)
    }

    func testAnEmptyObjectNameAppendsNothing() {
        let error = CNAObjectDisposedException(objectName: "", message: "base")
        XCTAssertEqual(error.Message, "base")
    }

    // MARK: - Identity

    /// The base decides which `catch` sees a use-after-dispose. It is
    /// `InvalidOperationException`, **not** `SystemException` directly, which
    /// is the one place this family differs from the six admitted before it.
    func testItIsAnInvalidOperationExceptionAndKeepsItsOwnHResult() {
        // Erased to `Error`, so each `is` is a real runtime test rather than
        // a statically-known one the compiler folds away.
        let error: Error = CNAObjectDisposedException(objectName: "X")
        XCTAssertTrue(error is CNAInvalidOperationException)
        XCTAssertTrue(error is CNASystemException)
        XCTAssertTrue(error is CNAException)
        XCTAssertFalse(error is CNAArgumentException)
        guard let error = error as? CNAObjectDisposedException else {
            return XCTFail("not the projected class")
        }
        XCTAssertEqual(
            error.HResult, objectDisposedHResult,
            "0x80131622, not InvalidOperationException's 0x80131509")
        XCTAssertNotEqual(
            error.HResult, CNAInvalidOperationException.corInvalidOperationHResult)
    }

    func testItReportsItsCLRNameAndNotItsSwiftName() {
        XCTAssertEqual(
            CNAObjectDisposedException(objectName: "X").cnaClassName,
            "System.ObjectDisposedException")
    }

    /// A projected CLR failure is never on the CNA runtime channel.
    func testItIsNotACNAError() {
        let error: Error = CNAObjectDisposedException(objectName: "X")
        XCTAssertFalse(error is CNAError)
        XCTAssertTrue(error is CNAException)
    }

    // MARK: - The raise site this milestone moved

    /// XNA guards every native member of a graphics resource with
    /// `Helpers.CheckDisposed(this, pComPtr)`, which raises
    /// `ObjectDisposedException(obj.GetType().Name)` — the **dynamic** type.
    /// Before this milestone the binding reported the same failure on
    /// `CNAError`, which `catch is CNAException` could not see.
    func testUsingADisposedResourceRaisesTheProjectedClass() throws {
        guard ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil else {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
        let game = try DisposedProbeGame()
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertFalse(
            game.sawCNAError, "a CLR-shaped failure must not stay on CNAError")
        XCTAssertEqual(game.seenClass, "CNAObjectDisposedException")
        XCTAssertEqual(game.seenObjectName, "RenderTarget2D")
        XCTAssertEqual(
            game.seenMessage,
            "Cannot access a disposed object.\r\nObject name: 'RenderTarget2D'.")
        XCTAssertEqual(game.seenHResult, Int32(bitPattern: 0x8013_1622))
        XCTAssertEqual(game.batchObjectName, "SpriteBatch")
        XCTAssertEqual(
            game.batchMessage,
            "Cannot access a disposed object.\r\nObject name: 'SpriteBatch'.")
    }
}
