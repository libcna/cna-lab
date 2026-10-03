// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the eight projected exception types,
// transcribed from the CIL of the hash-registered assemblies in
// `tools/api_compat/registered-assemblies.json`:
// `Microsoft.Xna.Framework.dll` (38e7093f…a130) declares the three Audio types
// and `ContentLoadException`, `Microsoft.Xna.Framework.Graphics.dll`
// (560080fc…9f55) the three device types, and
// `Microsoft.Xna.Framework.Storage.dll` (798f678e…cbb8)
// `StorageDeviceNotConnectedException`.
//
// Every constructor body in all eight is a bare forward to the base, so what
// these tests assert about messages and HResults is the BCL behaviour arriving
// unchanged through an XNA type — which is exactly the claim the projection
// makes.
extension PureValueTests {

    // ------------------------------------------------------------------
    // Exact declared bases.
    //
    // Five derive from `System.Exception` and three from
    // `System.Runtime.InteropServices.ExternalException`. The split is not
    // cosmetic: it changes the default `Message` and the `HResult`, and it
    // decides whether `ErrorCode` exists at all.
    // ------------------------------------------------------------------

    func testXnaExceptionsDeriveFromTheirExactDeclaredBase() {
        let onException: [CNAException] = [
            Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException(),
            Microsoft.Xna.Framework.Content.ContentLoadException(),
            Microsoft.Xna.Framework.Graphics.DeviceLostException(),
            Microsoft.Xna.Framework.Graphics.DeviceNotResetException(),
            Microsoft.Xna.Framework.Graphics.NoSuitableGraphicsDeviceException(),
        ]
        for exception in onException {
            XCTAssertFalse(
                exception is CNAExternalException,
                "\(type(of: exception)) derives from System.Exception in the IL")
            XCTAssertFalse(exception is CNASystemException)
        }

        let onExternal: [CNAException] = [
            Microsoft.Xna.Framework.Audio.InstancePlayLimitException(),
            Microsoft.Xna.Framework.Audio.NoAudioHardwareException(),
            Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException(),
        ]
        for exception in onExternal {
            let erased: Any = exception
            XCTAssertTrue(
                erased is CNAExternalException,
                "\(type(of: exception)) derives from ExternalException in the IL")
            // ... and therefore, through the exact chain, from SystemException.
            XCTAssertTrue(erased is CNASystemException)
        }
    }

    // ------------------------------------------------------------------
    // The CLR class name, which `Message` synthesizes from.
    // ------------------------------------------------------------------

    // `Exception.get_Message` formats `Exception_WasThrown` with
    // `GetClassName()`, the namespace-qualified CLR name. The projection
    // derives that name by reflection over the Swift namespace enums, so the
    // derivation is asserted here for every projected type rather than
    // assumed: if the compiler ever spelled a nested type differently, these
    // would fail rather than silently produce a wrong message.
    func testProjectedClrClassNamesAreDerivedExactly() {
        let expected: [(CNAException, String)] = [
            (CNAException(), "System.Exception"),
            (CNASystemException(), "System.SystemException"),
            (CNAExternalException(),
             "System.Runtime.InteropServices.ExternalException"),
            (Microsoft.Xna.Framework.Audio.InstancePlayLimitException(),
             "Microsoft.Xna.Framework.Audio.InstancePlayLimitException"),
            (Microsoft.Xna.Framework.Audio.NoAudioHardwareException(),
             "Microsoft.Xna.Framework.Audio.NoAudioHardwareException"),
            (Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException(),
             "Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException"),
            (Microsoft.Xna.Framework.Content.ContentLoadException(),
             "Microsoft.Xna.Framework.Content.ContentLoadException"),
            (Microsoft.Xna.Framework.Graphics.DeviceLostException(),
             "Microsoft.Xna.Framework.Graphics.DeviceLostException"),
            (Microsoft.Xna.Framework.Graphics.DeviceNotResetException(),
             "Microsoft.Xna.Framework.Graphics.DeviceNotResetException"),
            (Microsoft.Xna.Framework.Graphics.NoSuitableGraphicsDeviceException(),
             "Microsoft.Xna.Framework.Graphics.NoSuitableGraphicsDeviceException"),
            (Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException(),
             "Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException"),
        ]
        for (exception, name) in expected {
            XCTAssertEqual(exception.cnaClassName, name)
        }
    }

    // ------------------------------------------------------------------
    // What the parameterless constructor of each type actually reports.
    // ------------------------------------------------------------------

    // The five `System.Exception` subclasses reach `Init()`, so their message
    // is synthesized from their own class name and the HResult is
    // `COR_E_EXCEPTION`.
    func testExceptionDerivedXnaTypesSynthesizeTheirOwnClassNameMessage() {
        let lost = Microsoft.Xna.Framework.Graphics.DeviceLostException()
        XCTAssertEqual(
            lost.Message,
            "Exception of type "
            + "'Microsoft.Xna.Framework.Graphics.DeviceLostException' "
            + "was thrown.")
        XCTAssertEqual(lost.HResult, Int32(bitPattern: 0x8013_1500))

        let content = Microsoft.Xna.Framework.Content.ContentLoadException()
        XCTAssertEqual(
            content.Message,
            "Exception of type "
            + "'Microsoft.Xna.Framework.Content.ContentLoadException' "
            + "was thrown.")

        // Two different types must not report the same sentence: the class
        // name is the whole content of the default message.
        XCTAssertNotEqual(
            lost.Message,
            Microsoft.Xna.Framework.Graphics.DeviceNotResetException().Message)
    }

    // The three `ExternalException` subclasses never reach the synthesized
    // default at all: `ExternalException::.ctor()` substitutes
    // `Arg_ExternalException` before `Exception` ever sees a null message.
    func testExternalDerivedXnaTypesReportTheExternalComponentMessage() {
        for exception in [
            Microsoft.Xna.Framework.Audio.InstancePlayLimitException()
                as CNAExternalException,
            Microsoft.Xna.Framework.Audio.NoAudioHardwareException(),
            Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException(),
        ] {
            XCTAssertEqual(exception.Message,
                           "External component has thrown an exception.")
            XCTAssertEqual(exception.HResult, Int32(bitPattern: 0x8000_4005))
            XCTAssertEqual(exception.ErrorCode, Int32(bitPattern: 0x8000_4005))
        }

        // The difference between the two halves of the cluster is observable,
        // which is what makes getting the base right matter.
        XCTAssertNotEqual(
            Microsoft.Xna.Framework.Audio.NoAudioHardwareException().Message,
            Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException().Message)
        XCTAssertNotEqual(
            Microsoft.Xna.Framework.Audio.NoAudioHardwareException().HResult,
            Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException().HResult)
    }

    // ------------------------------------------------------------------
    // Constructor forwarding.
    //
    // Every XNA body is `ldarg.0; <args>; call base..ctor; ret`. No XNA type
    // synthesizes a message of its own, so a supplied message survives
    // unchanged and a supplied inner exception is the identical object.
    // ------------------------------------------------------------------

    func testXnaExceptionConstructorsForwardWithoutSynthesizingAnything() {
        let inner = Microsoft.Xna.Framework.Graphics.DeviceLostException(
            message: "root cause")

        let named = Microsoft.Xna.Framework.Graphics.DeviceNotResetException(
            message: "not reset")
        XCTAssertEqual(named.Message, "not reset")
        XCTAssertNil(named.InnerException)

        // The CLR parameter is `inner` on the six types whose metadata names
        // it that way, and `innerException` on the two that do not. Both
        // labels are the assembly's own.
        let wrapped = Microsoft.Xna.Framework.Graphics.DeviceNotResetException(
            message: "not reset", inner: inner)
        XCTAssertTrue(wrapped.InnerException === inner)
        XCTAssertTrue(wrapped.GetBaseException() === inner)

        let content = Microsoft.Xna.Framework.Content.ContentLoadException(
            message: "could not load", innerException: inner)
        XCTAssertEqual(content.Message, "could not load")
        XCTAssertTrue(content.InnerException === inner)

        // Forwarding a nil message reaches the base's own default, unchanged
        // by the XNA subclass.
        XCTAssertEqual(
            Microsoft.Xna.Framework.Graphics.DeviceLostException(message: nil)
                .Message,
            Microsoft.Xna.Framework.Graphics.DeviceLostException().Message)
        // ... and on the ExternalException half, the substituted message wins
        // only when no message is supplied at all.
        XCTAssertEqual(
            Microsoft.Xna.Framework.Audio.NoAudioHardwareException(
                message: "no device").Message,
            "no device")
    }

    /// Both protected serialization constructors are bare base forwards in
    /// the pinned XNA IL. The admitted carriers therefore have to restore the
    /// base state without applying either XNA defaults or ExternalException's
    /// normal-construction E_FAIL value a second time.
    func testXnaSerializationConstructorsRestoreObservableBaseState() {
        let inner = CNAException(message: "serialized inner")
        let context = CNAStreamingContext(state: 0x80)

        let contentInfo = CNASerializationInfo(
            className: "Fixture.SerializedContentFailure",
            message: nil,
            innerException: inner,
            helpLink: "https://example.invalid/content-help",
            hResult: 0x1234)
        let content = Microsoft.Xna.Framework.Content.ContentLoadException(
            info: contentInfo, context: context)
        XCTAssertEqual(
            content.Message,
            "Exception of type 'Fixture.SerializedContentFailure' was thrown.")
        XCTAssertTrue(content.InnerException === inner)
        XCTAssertEqual(content.HelpLink, "https://example.invalid/content-help")
        XCTAssertEqual(content.HResult, 0x1234)

        let storageInfo = CNASerializationInfo(
            className: "Fixture.SerializedStorageFailure",
            message: "device vanished",
            innerException: inner,
            helpLink: nil,
            hResult: -77)
        let storage = Microsoft.Xna.Framework.Storage
            .StorageDeviceNotConnectedException(
                info: storageInfo, context: context)
        XCTAssertEqual(storage.Message, "device vanished")
        XCTAssertTrue(storage.InnerException === inner)
        XCTAssertNil(storage.HelpLink)
        XCTAssertEqual(storage.HResult, -77)
        XCTAssertEqual(storage.ErrorCode, -77,
                       "ExternalException serialization preserves HResult")
    }

    // ------------------------------------------------------------------
    // They are real Swift errors.
    // ------------------------------------------------------------------

    // The point of the class projection: a projected XNA exception can be
    // thrown, caught by its own type, and caught by any of its bases.
    func testXnaExceptionsThrowAndCatchByExactTypeAndByBase() {
        func lose() throws {
            throw Microsoft.Xna.Framework.Graphics.DeviceLostException(
                message: "the device was lost")
        }

        do {
            try lose()
            XCTFail("the call did not throw")
        } catch let error as Microsoft.Xna.Framework.Graphics.DeviceLostException {
            XCTAssertEqual(error.Message, "the device was lost")
        } catch {
            XCTFail("caught the wrong error: \(error)")
        }

        // A base catch must see it, and a SIBLING catch must not.
        var caughtAsBase = false
        do {
            try lose()
        } catch is Microsoft.Xna.Framework.Graphics.DeviceNotResetException {
            XCTFail("a sibling exception type caught it")
        } catch is CNAException {
            caughtAsBase = true
        } catch {
            XCTFail("caught the wrong error: \(error)")
        }
        XCTAssertTrue(caughtAsBase)
    }

    func testExternalXnaExceptionIsCaughtByEachLinkOfTheChain() {
        func noHardware() throws {
            throw Microsoft.Xna.Framework.Audio.NoAudioHardwareException(
                message: "silent host", inner: CNAException(message: "root"))
        }

        for depth in 0..<3 {
            var caught = false
            do {
                try noHardware()
            } catch {
                switch depth {
                case 0: caught = error is Microsoft.Xna.Framework.Audio
                    .NoAudioHardwareException
                case 1: caught = error is CNAExternalException
                default: caught = error is CNAException
                }
            }
            XCTAssertTrue(caught, "not catchable at chain depth \(depth)")
        }
    }

    // A projected XNA exception must not be confused with the binding's own
    // runtime failure channel in either direction.
    func testXnaExceptionsAreNotCNAErrors() {
        let exception: Error =
            Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException()
        XCTAssertFalse(exception is CNAError)

        let runtimeFailure: Error = CNAError.unsupportedPlatform("Wasm")
        XCTAssertFalse(
            runtimeFailure is Microsoft.Xna.Framework.Graphics.DeviceLostException)
        XCTAssertFalse(runtimeFailure is CNAException)
    }

    // ------------------------------------------------------------------
    // Sealed status.
    //
    // Six of the eight are `sealed` in the IL and are Swift `final`; the two
    // the CLR leaves open are `open`, so a consumer can derive from exactly
    // the two XNA allows and no others. The `final` half cannot be expressed
    // as a subclass here at all -- the compiler refuses -- so this asserts the
    // open half, and the verifier's INHERITANCE_MAPPING_MISMATCH rule covers
    // the sealed half mechanically for every implemented class.
    // ------------------------------------------------------------------

    func testTheTwoUnsealedXnaExceptionsAreDerivable() {
        let derived = DerivedContentLoadException(message: "from a subclass")
        XCTAssertEqual(derived.Message, "from a subclass")
        let erasedDerived: Any = derived
        XCTAssertTrue(
            erasedDerived is Microsoft.Xna.Framework.Content.ContentLoadException)
        XCTAssertTrue(erasedDerived is CNAException)

        // A subclass that adds nothing still reports its OWN class name in the
        // synthesized default, because `GetClassName` resolves from the
        // object's dynamic type and not from the declaring type. A consumer's
        // type keeps its own module qualifier: only CNA's own is removed, so a
        // subclass declared elsewhere is not left with a bare name where the
        // CLR would report a qualified one.
        XCTAssertEqual(
            DerivedContentLoadException().Message,
            "Exception of type 'CNATests.DerivedContentLoadException' "
            + "was thrown.")

        let storage = DerivedStorageDeviceNotConnectedException()
        XCTAssertTrue((storage as Any) is CNAExternalException)
        XCTAssertEqual(storage.ErrorCode, Int32(bitPattern: 0x8000_4005))
        XCTAssertEqual(storage.Message,
                       "External component has thrown an exception.")
    }

    // The one divergence in the class-name derivation, asserted rather than
    // left to be discovered.
    //
    // `String(reflecting:)` gives a stable qualified name for a type with
    // module visibility -- declared at file scope, or nested in the namespace
    // enums, which is every type the projection itself declares and is proved
    // above for all eleven. A type declared `private`/`fileprivate`, or inside
    // a function, has no nominal context to name, so the compiler substitutes
    // an `(unknown context at $ADDRESS)` token that is not even stable between
    // runs. The CLR would report the real nested name there.
    //
    // This is reachable only by a consumer subclassing one of the two unsealed
    // XNA exceptions in such a scope, and it affects nothing but the
    // synthesized default `Message` of that consumer's own type -- a message
    // that is only produced when no message was supplied at all. It is a Swift
    // reflection limit, it is recorded rather than papered over, and no
    // projected XNA type is subject to it.
    func testALocallyDeclaredSubclassGetsAMangledClassNameInItsDefaultMessage() {
        final class Local: Microsoft.Xna.Framework.Content.ContentLoadException {}
        let message = Local().Message
        XCTAssertTrue(message.hasPrefix("Exception of type '"))
        XCTAssertTrue(
            message.contains("Local"),
            "the dynamic type still drives the message")
        XCTAssertTrue(
            message.contains("unknown context"),
            "a locally declared type is expected to carry a mangled context; "
            + "if Swift has started spelling it exactly, this divergence is "
            + "gone and the note above should go with it")
        // A supplied message is unaffected, which is why the divergence is
        // narrow enough to record rather than block on.
        XCTAssertEqual(Local(message: "explicit").Message, "explicit")
    }
}

// Declared at file scope with module visibility, which is what a consumer
// subclassing an unsealed XNA exception would normally do, and what gives
// `String(reflecting:)` a stable qualified name. Marking either of them
// `private` would not change what they inherit, but it WOULD change the
// synthesized default message -- see the divergence test above.
final class DerivedContentLoadException:
    Microsoft.Xna.Framework.Content.ContentLoadException {}

final class DerivedStorageDeviceNotConnectedException:
    Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException {}
