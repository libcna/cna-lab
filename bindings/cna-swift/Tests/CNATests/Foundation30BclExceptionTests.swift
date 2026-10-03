// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure BCL-derived behaviour for the three exception support classes,
// transcribed from the CIL and the embedded resource table of the
// hash-registered Microsoft .NET Framework 4.0 `mscorlib` admitted in
// `tools/api_compat/bcl-authorities.json` (SHA-256 5634668d…acc63) and pinned
// in `tools/api_compat/reference/bcl40-selected-shape.json`.
//
// Nothing here is an XNA identity and nothing here is counted as one. These
// are the semantics the eight XNA exception types inherit rather than declare.
extension PureValueTests {

    // ------------------------------------------------------------------
    // The resource strings.
    //
    // None of the three default messages is an IL literal: each constructor or
    // getter loads a resource KEY. These are the values the audit read out of
    // the assembly's own `mscorlib.resources`, and the verifier separately
    // proves the Swift source reproduces them.
    // ------------------------------------------------------------------

    func testBclExceptionResourceStringsAreTheAdmittedAssemblysOwn() {
        XCTAssertEqual(
            CNAException.exceptionWasThrownFormat,
            "Exception of type '{0}' was thrown.")
        XCTAssertEqual(CNAException.argSystemExceptionMessage, "System error.")
        XCTAssertEqual(
            CNAException.argExternalExceptionMessage,
            "External component has thrown an exception.")
    }

    // ------------------------------------------------------------------
    // Construction, and the state each constructor leaves behind.
    // ------------------------------------------------------------------

    // `Exception::.ctor()` is `Object::.ctor(); Init()`, and `Init` is
    // `_message = null; ...; set_HResult(0x80131500)`. A null message is not an
    // empty message: `get_Message` branches on it and synthesizes.
    func testExceptionDefaultConstructorSynthesizesTheClassNameMessage() {
        let exception = CNAException()
        XCTAssertEqual(
            exception.Message, "Exception of type 'System.Exception' was thrown.")
        XCTAssertNil(exception.InnerException)
        XCTAssertNil(exception.HelpLink)
        XCTAssertEqual(exception.HResult, Int32(bitPattern: 0x8013_1500))
    }

    // `.ctor(string message)` is `Init(); _message = message`, with no
    // validation, so a nil argument is stored and reaches the same synthesized
    // default as the parameterless constructor. That equivalence is why the
    // Optional projection loses nothing.
    func testExceptionNullMessageIsStoredAndSelectsTheSynthesizedDefault() {
        XCTAssertEqual(CNAException(message: nil).Message,
                       CNAException().Message)
        // An EMPTY message is a different state, and stays empty. Substituting
        // "" for null would have collapsed the two.
        XCTAssertEqual(CNAException(message: "").Message, "")
        XCTAssertEqual(CNAException(message: "boom").Message, "boom")
    }

    // `.ctor(string, Exception)` stores both arguments and validates neither.
    func testExceptionInnerConstructorStoresBothArguments() {
        let inner = CNAException(message: "inner")
        let outer = CNAException(message: "outer", innerException: inner)
        XCTAssertEqual(outer.Message, "outer")
        XCTAssertTrue(outer.InnerException === inner)
        XCTAssertNil(inner.InnerException)
        // A nil message beside a non-nil inner exception is a state no other
        // constructor can reach, which is the second half of the Optional
        // argument decision.
        let anonymous = CNAException(message: nil, innerException: inner)
        XCTAssertEqual(anonymous.Message,
                       "Exception of type 'System.Exception' was thrown.")
        XCTAssertTrue(anonymous.InnerException === inner)
    }

    // ------------------------------------------------------------------
    // `GetBaseException`.
    // ------------------------------------------------------------------

    // The IL walks `InnerException` to the end and returns the LAST non-null
    // exception, which is `this` when the chain is empty. It is not the
    // immediate inner exception and it is not the outermost one.
    func testGetBaseExceptionReturnsTheDeepestExceptionInTheChain() {
        let deepest = CNAException(message: "deepest")
        let middle = CNAException(message: "middle", innerException: deepest)
        let outer = CNAException(message: "outer", innerException: middle)
        XCTAssertTrue(outer.GetBaseException() === deepest)
        XCTAssertTrue(middle.GetBaseException() === deepest)
        XCTAssertTrue(deepest.GetBaseException() === deepest)
    }

    // ------------------------------------------------------------------
    // `SystemException` is not a marker.
    // ------------------------------------------------------------------

    // `SystemException::.ctor()` substitutes `Arg_SystemException` and sets
    // `0x80131501`; the message-taking constructors keep the caller's message
    // but still set the HResult. A projection that collapsed this link would
    // report the `Exception_WasThrown` default and `0x80131500` instead.
    func testSystemExceptionSubstitutesItsOwnMessageAndHResult() {
        let fromDefault = CNASystemException()
        XCTAssertEqual(fromDefault.Message, "System error.")
        XCTAssertEqual(fromDefault.HResult, Int32(bitPattern: 0x8013_1501))

        let withMessage = CNASystemException(message: "explicit")
        XCTAssertEqual(withMessage.Message, "explicit")
        XCTAssertEqual(withMessage.HResult, Int32(bitPattern: 0x8013_1501))

        let inner = CNAException()
        let withInner = CNASystemException(message: "explicit", innerException: inner)
        XCTAssertTrue(withInner.InnerException === inner)
        XCTAssertEqual(withInner.HResult, Int32(bitPattern: 0x8013_1501))
    }

    // ------------------------------------------------------------------
    // `ExternalException` overwrites what `SystemException` has just set.
    // ------------------------------------------------------------------

    func testExternalExceptionSubstitutesItsOwnMessageAndHResult() {
        let fromDefault = CNAExternalException()
        XCTAssertEqual(fromDefault.Message,
                       "External component has thrown an exception.")
        XCTAssertEqual(fromDefault.HResult, Int32(bitPattern: 0x8000_4005))
        XCTAssertEqual(fromDefault.ErrorCode, Int32(bitPattern: 0x8000_4005))

        let withMessage = CNAExternalException(message: "explicit")
        XCTAssertEqual(withMessage.Message, "explicit")
        XCTAssertEqual(withMessage.ErrorCode, Int32(bitPattern: 0x8000_4005))
    }

    // `.ctor(string, int errorCode)` is the one constructor of the family that
    // passes the caller's value to `SetErrorCode`, and `get_ErrorCode` is a
    // bare `call get_HResult`, so the two are always the same number.
    func testExternalExceptionErrorCodeIsTheHResult() {
        let explicitCode = CNAExternalException(message: "device", errorCode: 0x2A)
        XCTAssertEqual(explicitCode.ErrorCode, 0x2A)
        XCTAssertEqual(explicitCode.HResult, 0x2A)
        XCTAssertEqual(explicitCode.Message, "device")

        // `HResult` is the protected CLR state, widened to public because
        // Swift has no `protected`; writing it moves `ErrorCode` with it.
        explicitCode.HResult = -5
        XCTAssertEqual(explicitCode.ErrorCode, -5)
    }

    // ------------------------------------------------------------------
    // The chain is three links, and it is a real Swift chain.
    // ------------------------------------------------------------------

    // Every check goes through `Any`, so the chain is tested at RUNTIME. A
    // direct `external is CNASystemException` would be answered by the
    // compiler -- which is itself evidence the declaration is right, and is
    // also why it is a warning, and why warnings are errors here.
    func testExceptionSupportChainIsExactlyThreeLinks() {
        let external: Any = CNAExternalException()
        XCTAssertTrue(external is CNASystemException)
        XCTAssertTrue(external is CNAException)
        XCTAssertTrue(external is Error)

        let system: Any = CNASystemException()
        XCTAssertTrue(system is CNAException)
        // ... and not in the other direction: the chain is not a cycle and the
        // base is not the derived class.
        let base: Any = CNAException()
        XCTAssertFalse(base is CNASystemException)
        XCTAssertFalse(system is CNAExternalException)
    }

    // ------------------------------------------------------------------
    // `HelpLink` is plain storage, mutable and Optional.
    // ------------------------------------------------------------------

    func testHelpLinkIsNilUntilAssigned() {
        let exception = CNAException(message: "m")
        XCTAssertNil(exception.HelpLink)
        exception.HelpLink = "https://example.invalid/help"
        XCTAssertEqual(exception.HelpLink, "https://example.invalid/help")
        exception.HelpLink = nil
        XCTAssertNil(exception.HelpLink)
    }

    // ------------------------------------------------------------------
    // `CNAError` is a different channel and must stay one.
    // ------------------------------------------------------------------

    // The architectural claim, asserted rather than left to the prose: the
    // binding's own runtime failure channel is not a CLR exception identity,
    // so it is not a `CNAException` and a `catch is CNAException` must not
    // swallow it.
    func testCNAErrorIsNotACNAException() {
        let runtimeFailure: Error = CNAError.callbackOutsideGameLifecycle
        XCTAssertFalse(runtimeFailure is CNAException)
        XCTAssertFalse(runtimeFailure is CNAExternalException)

        // ... and an exception is not a CNAError either.
        let exception: Any = CNAException(message: "m")
        XCTAssertFalse(exception is CNAError)

        var caughtAsException = false
        var caughtAsError = false
        do {
            throw CNAError.ownerThreadViolation("Draw")
        } catch is CNAException {
            caughtAsException = true
        } catch is CNAError {
            caughtAsError = true
        } catch {
            XCTFail("unreachable")
        }
        XCTAssertFalse(caughtAsException,
                       "a CNAError was caught as a projected CLR exception")
        XCTAssertTrue(caughtAsError)
    }

    // ------------------------------------------------------------------
    // A support exception is a real Swift error.
    // ------------------------------------------------------------------

    func testSupportExceptionsAreThrowableAndCatchable() {
        func failing() throws {
            throw CNAExternalException(message: "external", errorCode: 7)
        }
        do {
            try failing()
            XCTFail("the call did not throw")
        } catch let error as CNAExternalException {
            XCTAssertEqual(error.ErrorCode, 7)
            XCTAssertEqual(error.Message, "external")
        } catch {
            XCTFail("caught the wrong error: \(error)")
        }
    }
}
