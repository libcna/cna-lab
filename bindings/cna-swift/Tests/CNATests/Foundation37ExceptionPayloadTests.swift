// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// The eight raised exception families, and the payload each carries.
///
/// Foundation 30 projected the three exception BASES. These are the classes the
/// support layer actually throws, and before this milestone every one of those
/// failures came out of `CNAError` with the right message and the wrong class.
final class Foundation37ExceptionPayloadTests: XCTestCase {
    // ------------------------------------------------------------------
    // The chain each class sits on decides which `catch` clause sees it.
    // ------------------------------------------------------------------

    func testTheInheritanceChainIsMscorlibsOwn() {
        let argument: Any = CNAArgumentException()
        XCTAssertTrue(argument is CNASystemException)
        XCTAssertTrue(argument is CNAException)
        XCTAssertFalse(argument is CNAExternalException)

        // Both argument specializations derive from ArgumentException, so a
        // handler for it sees a null argument and an out-of-range one alike.
        let null: Any = CNAArgumentNullException()
        let range: Any = CNAArgumentOutOfRangeException()
        XCTAssertTrue(null is CNAArgumentException)
        XCTAssertTrue(range is CNAArgumentException)
        XCTAssertFalse(null is CNAArgumentOutOfRangeException)
        XCTAssertFalse(range is CNAArgumentNullException)

        // These four sit on SystemException DIRECTLY. A missing key is not an
        // argument failure, and neither is a read-only refusal.
        for value in [
            CNANotSupportedException() as Any,
            CNAInvalidOperationException() as Any,
            CNAKeyNotFoundException() as Any,
            CNANullReferenceException() as Any,
            CNAIndexOutOfRangeException() as Any,
        ] {
            XCTAssertTrue(value is CNASystemException, "\(Swift.type(of: value))")
            XCTAssertFalse(value is CNAArgumentException, "\(Swift.type(of: value))")
        }
    }

    // ------------------------------------------------------------------
    // The substituted message and HResult of every parameterless constructor.
    // ------------------------------------------------------------------

    func testEveryParameterlessConstructorSubstitutesItsOwnResourceAndHResult() {
        let expected: [(CNAException, String, UInt32)] = [
            (CNAArgumentException(),
             "Value does not fall within the expected range.", 0x8007_0057),
            (CNAArgumentNullException(),
             "Value cannot be null.", 0x8000_4003),
            (CNAArgumentOutOfRangeException(),
             "Specified argument was out of the range of valid values.", 0x8013_1502),
            (CNANotSupportedException(),
             "Specified method is not supported.", 0x8013_1515),
            (CNAInvalidOperationException(),
             "Operation is not valid due to the current state of the object.", 0x8013_1509),
            (CNAKeyNotFoundException(),
             "The given key was not present in the dictionary.", 0x8013_1577),
            (CNANullReferenceException(),
             "Object reference not set to an instance of an object.", 0x8000_4003),
            (CNAIndexOutOfRangeException(),
             "Index was outside the bounds of the array.", 0x8013_1508),
        ]
        for (exception, message, hResult) in expected {
            XCTAssertEqual(exception.Message, message, "\(Swift.type(of: exception))")
            XCTAssertEqual(
                exception.HResult, Int32(bitPattern: hResult),
                "\(Swift.type(of: exception))")
            XCTAssertNil(exception.InnerException, "\(Swift.type(of: exception))")
        }
    }

    /// A subclass's HResult must be its own, not the base's it just ran.
    ///
    /// Every constructor in this family calls up and *then* overwrites the
    /// HResult its base assigned. A projection that forwarded and stopped
    /// would report `COR_E_ARGUMENT` for a null argument.
    func testASubclassOverwritesTheHResultItsBaseAssigned() {
        XCTAssertNotEqual(
            CNAArgumentNullException().HResult, CNAArgumentException().HResult)
        XCTAssertNotEqual(
            CNAArgumentOutOfRangeException().HResult, CNAArgumentException().HResult)
        XCTAssertNotEqual(
            CNAArgumentException().HResult, CNASystemException().HResult)
        XCTAssertNotEqual(
            CNAKeyNotFoundException().HResult, CNASystemException().HResult)
    }

    // ------------------------------------------------------------------
    // `ArgumentException.get_Message` composition.
    // ------------------------------------------------------------------

    func testMessageComposesTheParameterNameAroundTheIlNewline() {
        let composed = CNAArgumentException(message: "Bad.", paramName: "value")
        XCTAssertEqual(composed.ParamName, "value")
        XCTAssertEqual(composed.Message, "Bad.\r\nParameter name: value")

        // `String.IsNullOrEmpty` is the test, so nil and "" both add nothing --
        // and they are not the same state.
        let none = CNAArgumentException(message: "Bad.")
        XCTAssertNil(none.ParamName)
        XCTAssertEqual(none.Message, "Bad.")
        let empty = CNAArgumentException(message: "Bad.", paramName: "")
        XCTAssertEqual(empty.ParamName, "")
        XCTAssertEqual(empty.Message, "Bad.")

        // A nil message still composes, and what it composes onto is
        // `Exception_WasThrown` formatted with the CLR class name -- NOT
        // `Arg_ArgumentException`, which only the parameterless constructor
        // substitutes. The two are easy to confuse and the CLR distinguishes
        // them, so the class name must be the CLR's own and not the Swift
        // spelling.
        let defaulted = CNAArgumentException(message: nil, paramName: "value")
        XCTAssertEqual(
            defaulted.Message,
            "Exception of type \'System.ArgumentException\' was thrown."
            + "\r\nParameter name: value")
    }

    /// The separator is the assembly's, not the host's.
    ///
    /// `System.Environment::get_NewLine`'s entire body in the admitted
    /// `mscorlib` is `ldstr "\r\n"; ret`, so the composed message carries a
    /// carriage return even though this binding runs on Linux.
    func testTheNewlineIsTheIlLiteralAndNotThePlatformsOwn() {
        let composed = CNAArgumentException(message: "Bad.", paramName: "value")
        XCTAssertTrue(composed.Message.contains("\r\n"))
        XCTAssertEqual(CNAArgumentException.environmentNewLine, "\r\n")
    }

    /// The two-argument constructors of this family are transposed with
    /// respect to each other, and the transposition is observable.
    func testTheTransposedTwoArgumentConstructorsAreNotInterchangeable() {
        let argument = CNAArgumentException(message: "M", paramName: "P")
        let null = CNAArgumentNullException(paramName: "P", message: "M")
        let range = CNAArgumentOutOfRangeException(paramName: "P", message: "M")
        for exception in [argument, null, range] as [CNAArgumentException] {
            XCTAssertEqual(exception.ParamName, "P", "\(Swift.type(of: exception))")
            XCTAssertEqual(
                exception.Message, "M\r\nParameter name: P",
                "\(Swift.type(of: exception))")
        }
    }

    /// `ArgumentNullException`'s single-argument constructor takes a parameter
    /// NAME, not a message. The label says so, and the payload proves it.
    func testArgumentNullExceptionsOneArgumentConstructorTakesAParameterName() {
        let exception = CNAArgumentNullException(paramName: "provider")
        XCTAssertEqual(exception.ParamName, "provider")
        XCTAssertEqual(
            exception.Message, "Value cannot be null.\r\nParameter name: provider")

        // ... while the inherited message-taking one really does take a message.
        let message = CNAArgumentNullException(message: "custom")
        XCTAssertNil(message.ParamName)
        XCTAssertEqual(message.Message, "custom")
        XCTAssertEqual(message.HResult, Int32(bitPattern: 0x8000_4003))
    }

    func testArgumentOutOfRangeExceptionsOneArgumentConstructorTakesAParameterName() {
        let exception = CNAArgumentOutOfRangeException(paramName: "keyIndex")
        XCTAssertEqual(exception.ParamName, "keyIndex")
        XCTAssertEqual(
            exception.Message,
            "Specified argument was out of the range of valid values."
            + "\r\nParameter name: keyIndex")
    }

    // ------------------------------------------------------------------
    // Inner exceptions and GetBaseException still work through the new links.
    // ------------------------------------------------------------------

    /// Every new class reports its CLR name, not its Swift one.
    ///
    /// `Exception.get_Message` formats `Exception_WasThrown` with
    /// `GetClassName()`, so a projected class whose name is not mapped would
    /// put `CNAArgumentException` into a user-visible sentence.
    func testEveryNewSupportClassReportsItsCLRName() {
        let expected: [(CNAException, String)] = [
            (CNAArgumentException(message: nil), "System.ArgumentException"),
            (CNAArgumentNullException(message: nil), "System.ArgumentNullException"),
            (CNAArgumentOutOfRangeException(message: nil),
             "System.ArgumentOutOfRangeException"),
            (CNANotSupportedException(message: nil), "System.NotSupportedException"),
            (CNAInvalidOperationException(message: nil),
             "System.InvalidOperationException"),
            (CNAKeyNotFoundException(message: nil),
             "System.Collections.Generic.KeyNotFoundException"),
            (CNANullReferenceException(message: nil), "System.NullReferenceException"),
            (CNAIndexOutOfRangeException(message: nil),
             "System.IndexOutOfRangeException"),
        ]
        for (exception, clrName) in expected {
            XCTAssertEqual(
                exception.Message,
                "Exception of type \'\(clrName)\' was thrown.",
                "\(Swift.type(of: exception))")
        }
    }

    func testTheInnerExceptionChainSurvivesTheNewSubclasses() {
        let root = CNAKeyNotFoundException()
        let middle = CNAInvalidOperationException(message: "middle", innerException: root)
        let outer = CNAArgumentException(
            message: "outer", paramName: "value", innerException: middle)
        XCTAssertTrue(outer.InnerException === middle)
        XCTAssertTrue(outer.GetBaseException() === root)
        XCTAssertEqual(outer.Message, "outer\r\nParameter name: value")
        XCTAssertEqual(outer.HResult, Int32(bitPattern: 0x8007_0057))
    }

    // ------------------------------------------------------------------
    // The two channels stay apart.
    // ------------------------------------------------------------------

    func testTheProjectedExceptionsAreNotCNAErrorsAndViceVersa() {
        let projected: Error = CNAArgumentOutOfRangeException(paramName: "index")
        XCTAssertFalse(projected is CNAError)
        let runtime: Error = CNAError.ownerThreadViolation("Draw")
        XCTAssertFalse(runtime is CNAException)
        XCTAssertFalse(runtime is CNAArgumentException)

        // `catch is CNAException` must not swallow a native runtime failure.
        do {
            throw CNAError.callbackOutsideGameLifecycle
        } catch is CNAException {
            XCTFail("a CNAError was caught as a CNAException")
        } catch is CNAError {
            // expected
        } catch {
            XCTFail("unexpected: \(error)")
        }
    }

    /// After the conversion, `CNAError` declares no CLR-shaped case at all.
    func testCNAErrorCarriesNoCLRShapedIdentity() {
        // Each of these is a real CNA runtime failure and stays here.
        let runtime: [CNAError] = [
            .nativeLibraryNotFound(candidates: []),
            .nativeLibraryLoadFailed(path: "/x", message: "y"),
            .missingNativeSymbol("cna_game_run"),
            .unsupportedABIVersion(
                admitted: "a", actual: "b", actualEncoded: 0, path: "/x"),
            .nativeFailure(operation: "op", result: 1, message: "m"),
            .unsupportedPlatform("Wasm"),
            .disposedObject("Game"),
            .ownerThreadViolation("Draw"),
            .staleRuntimeGeneration(expected: 1, actual: 2),
            .callbackOutsideGameLifecycle,
            .streamFailure("m"),
            .producerInvariant("m"),
        ]
        XCTAssertEqual(runtime.count, 12)
        for value in runtime {
            XCTAssertFalse((value as Error) is CNAException, "\(value)")
        }
    }

    // ------------------------------------------------------------------
    // The two paths whose message the CLR produces natively.
    // ------------------------------------------------------------------

    /// `List<T>.CopyTo` delegates to `Array.Copy`, whose validating overload is
    /// `internalcall`. The classes are documented and projected; no message is
    /// claimed, because none is in the admitted IL.
    func testTheArrayCopyFailuresCarryTheClassButClaimNoMessage() throws {
        let list = CNAList<Int>()
        list.Add(1)
        list.Add(2)

        var destination = [0, 0]
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: "Specified argument was out of the range of valid values."
                + "\r\nParameter name: arrayIndex",
            paramName: "arrayIndex",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            try list.CopyTo(&destination, arrayIndex: -1)
        }

        var tooShort = [0]
        assertProjected(
            CNAArgumentException.self,
            message: "Value does not fall within the expected range.",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            try list.CopyTo(&tooShort, arrayIndex: 0)
        }
    }

    // ------------------------------------------------------------------
    // The two range resources List<T> uses are different, and the difference
    // is what an insert past the end reports.
    // ------------------------------------------------------------------

    func testInsertAndIndexReportDifferentRangeMessages() throws {
        let list = CNAList<Int>()
        list.Add(1)

        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Index was out of range. Must be non-negative and less than "
                + "the size of the collection.", paramName: "index"),
            paramName: "index",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            _ = try list.Item(5)
        }
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Index must be within the bounds of the List.", paramName: "index"),
            paramName: "index",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            try list.Insert(5, item: 9)
        }
        // `index == Count` is a legal append for Insert and out of range for
        // the indexer, which is the other half of the same distinction.
        XCTAssertNoThrow(try list.Insert(1, item: 2))
    }

    /// `Collection<T>`'s read-only guard raises a different message from the
    /// parameterless `NotSupportedException` XNA's own collections raise.
    func testTheReadOnlyGuardAndTheXnaRefusalCarryDifferentMessages() {
        XCTAssertEqual(
            CNAList<Int>.readOnlyCollectionMessage, "Collection is read-only.")
        XCTAssertEqual(
            CNANotSupportedException().Message,
            "Specified method is not supported.")
        XCTAssertNotEqual(
            CNAList<Int>.readOnlyCollectionMessage,
            CNANotSupportedException().Message)
    }
}
