// SPDX-License-Identifier: MIT

import Foundation

// BCL exception support projection.
//
// Eight XNA types are exception classes, and every one of them declares
// nothing but constructors: their entire observable behaviour — the message
// they report, the inner exception they carry, the HResult they expose — is
// inherited from `System.Exception` and, for three of them, from
// `System.Runtime.InteropServices.ExternalException`. Dropping those bases
// would leave the subclasses empty, exactly as dropping `Collection<T>` would
// have left `GameComponentCollection` empty, so the Foundation 27 rule applies
// unchanged:
//
//     a CLR BCL base class with a registered projection becomes a REAL Swift
//     superclass.
//
//     System.Exception                                 -> CNAException
//     System.SystemException                           -> CNASystemException
//     System.Runtime.InteropServices.ExternalException -> CNAExternalException
//
// The middle link is not decoration. `ExternalException`'s exact direct base
// in the admitted `mscorlib` is `SystemException`, whose constructors
// substitute their own default message and set HResult to `0x80131501` before
// `ExternalException` overwrites it with `0x80004005`. Collapsing the chain to
// two links would change both, so it is projected at full length and the
// verifier measures each link.
//
// `CNAException` conforms to Swift `Error`. That is the whole point: a CLR
// `Exception` object is a thing you throw, so its Swift projection must be a
// thing you can `throw`, and `catch is DeviceLostException` must work. A
// struct or an enum could not carry the inheritance chain, and an alias to
// `CNAError` would confuse two different channels — see below.
//
// These three classes live outside `Microsoft.Xna.Framework`. They are
// language/BCL-support API, not XNA types and not XNA identities, are counted
// in no XNA scoreboard, and no `::System` namespace is fabricated for them.
// Their shape is measured against the pinned selected-BCL manifest in
// `tools/api_compat/reference/bcl40-selected-shape.json`, reconstructed from
// the hash-registered Microsoft .NET Framework 4.0 `mscorlib` — see
// `docs/foundation-30-bcl-exception-projection-evidence.md`.
//
// Unlike every other support type here, these three DO carry a `Sendable`
// conformance, and it is worth being exact about why: the Swift standard
// library declares `Error` as refining `Sendable`, so conforming to `Error`
// conforms to `Sendable` and this file can neither add nor decline it. It is
// not a promise the projection makes. `HResult` and `HelpLink` are mutable
// here exactly as `mscorlib` declares them, and all three classes are `open`,
// so a subclass anywhere may add more mutable state; nothing about the
// cross-actor safety of a `CNAException` is being asserted by this file, and
// the conformance the Symbol Graph reports is the language's, not a claim.
//
// ## `CNAError` stays a separate channel
//
// `CNAError` is **not** in this hierarchy and must not be. It reports
// CNA-Swift runtime and language-support failures — a native library that will
// not load, an owner-thread violation, a stale runtime generation, an
// unsupported platform, callback lifecycle misuse — none of which corresponds
// to a selected XNA exception identity. `CNAError` is not a `CNAException`,
// `CNAException` does not wrap `CNAError`, and a test asserts both.
//
// ## What is deliberately absent
//
// `mscorlib` declares more on `System.Exception` than is projected here:
// `StackTrace`, `Source`, `TargetSite`, `Data`, `GetObjectData`, `GetType`,
// `ToString` and the protected `SerializeObjectState` event. Each of them
// needs a CLR runtime service this projection does not have — the runtime's
// captured managed stack, `System.Reflection.MethodBase`, `System.Type`,
// `System.Collections.IDictionary`, or the serialization runtime — and each is
// therefore **forbidden** by the verifier rather than merely missing. A
// `StackTrace` that answered with an empty string, or a `Data` that answered
// with a dictionary XNA never populated, would be a fabrication; an absence
// that the gate enforces is not.

/// The `System.Exception` projection.
///
/// Conforms to Swift `Error`, so every projected XNA exception subclass is a
/// real throwable Swift error.
open class CNAException: Error {
    // ------------------------------------------------------------------
    // Resource strings.
    //
    // `Exception.get_Message`, `SystemException..ctor()` and
    // `ExternalException..ctor()` do NOT carry their default messages as IL
    // literals: each loads a resource KEY and calls
    // `Environment.GetResourceString`. These values were read out of the
    // admitted assembly's own embedded `mscorlib.resources` string table by
    // `tools/api_compat/bcl_authority_audit.py`, are pinned in
    // `reference/bcl40-selected-shape.json` under `resourceStrings`, and the
    // verifier reads these literals back out of this source and compares them.
    // None of them is transcribed from documentation or memory.
    // ------------------------------------------------------------------

    /// `Exception_WasThrown`, formatted with the CLR class name.
    internal static let exceptionWasThrownFormat =
        "Exception of type '{0}' was thrown."

    /// `Arg_SystemException`.
    internal static let argSystemExceptionMessage = "System error."

    /// `Arg_ExternalException`.
    internal static let argExternalExceptionMessage =
        "External component has thrown an exception."

    /// `COR_E_EXCEPTION`, the HResult `Exception::Init` assigns.
    internal static let corExceptionHResult = Int32(bitPattern: 0x8013_1500)

    /// `COR_E_SYSTEM`, the HResult every `SystemException` constructor assigns.
    internal static let corSystemHResult = Int32(bitPattern: 0x8013_1501)

    /// `E_FAIL`, the HResult every `ExternalException` constructor assigns
    /// except the one that takes an explicit error code.
    internal static let externalExceptionHResult = Int32(bitPattern: 0x8000_4005)

    // `Exception._message`. Held as written: the CLR stores the argument
    // without validating it, and `get_Message` branches on it being null, so
    // nil is a real state and not an empty string.
    private let storedMessage: String?

    // `Exception._className`. Ordinary construction leaves this for the CLR
    // runtime to derive from the dynamic type; deserialization restores the
    // serialized value and `Message` observes it when no message was stored.
    private let storedClassName: String?

    // `Exception._innerException`, stored by the constructor and never
    // rewritten.
    private let storedInnerException: CNAException?

    /// `Exception.HResult`.
    ///
    /// `mscorlib` declares this `protected` with both accessors and no virtual
    /// flag. Swift has no `protected`, so it is public here — the same single
    /// widening `CNACollection.Items` already makes — and `final`, because the
    /// CLR does not make it an override point. Its value is what
    /// `ExternalException.ErrorCode` reads.
    public final var HResult: Int32

    /// `Exception.HelpLink`.
    ///
    /// `get_HelpLink` returns the `_helpURL` field and `set_HelpLink` stores
    /// it; both are `virtual` and neither is `final`, so this is an `open`
    /// read/write property. `Init` leaves the field null, so the projection is
    /// Optional and never an empty string.
    open var HelpLink: String?

    /// `Exception..ctor()`.
    ///
    /// The IL is `Object..ctor(); Init()`, and `Init` nulls `_message` and
    /// sets HResult to `0x80131500`. `Message` therefore reports the
    /// synthesized default.
    public init() {
        self.storedMessage = nil
        self.storedClassName = nil
        self.storedInnerException = nil
        self.HelpLink = nil
        self.HResult = CNAException.corExceptionHResult
    }

    /// `Exception..ctor(String message)`.
    ///
    /// The IL is `Object..ctor(); Init(); _message = message`. The argument is
    /// stored unvalidated, so a nil message is a normal observable state that
    /// selects the synthesized default from `Message` — which is why the
    /// parameter is Optional rather than an empty-string stand-in.
    public init(message: String?) {
        self.storedMessage = message
        self.storedClassName = nil
        self.storedInnerException = nil
        self.HelpLink = nil
        self.HResult = CNAException.corExceptionHResult
    }

    /// `Exception..ctor(String message, Exception innerException)`.
    ///
    /// The IL stores both arguments and validates neither.
    public init(message: String?, innerException: CNAException?) {
        self.storedMessage = message
        self.storedClassName = nil
        self.storedInnerException = innerException
        self.HelpLink = nil
        self.HResult = CNAException.corExceptionHResult
    }

    /// `protected Exception(SerializationInfo, StreamingContext)`.
    ///
    /// Swift has no `protected`, so the same visibility widening already used
    /// for protected XNA constructors makes this initializer public. The
    /// admitted carrier contains exactly the state observable through the
    /// selected Exception projection; stack, source, data and remoting fields
    /// remain refused rather than fabricated.
    public init(info: CNASerializationInfo, context: CNAStreamingContext) {
        self.storedMessage = info.message
        self.storedClassName = info.className
        self.storedInnerException = info.innerException
        self.HelpLink = info.helpLink
        self.HResult = info.hResult
        _ = context.state
    }

    /// `Exception.Message`.
    ///
    /// `get_Message` returns `_message` when it is not null, and otherwise
    /// formats the `Exception_WasThrown` resource string with the exception's
    /// own CLR class name. Both halves are reproduced, so
    /// `DeviceLostException().Message` is the sentence XNA reports and not an
    /// empty string. `virtual` and not `final` in the metadata, so `open`.
    open var Message: String {
        if let storedMessage { return storedMessage }
        return CNAException.exceptionWasThrownFormat.replacingOccurrences(
            of: "{0}", with: cnaClassName)
    }

    /// `Exception.InnerException`.
    ///
    /// `get_InnerException` is `virtual final` in the metadata — a sealed
    /// interface implementation, not an override point — so this is `final`
    /// rather than `open`. It returns the field, which is null unless a
    /// two-argument constructor supplied one.
    public final var InnerException: CNAException? { storedInnerException }

    /// `Exception.GetBaseException()`.
    ///
    /// The IL walks the `InnerException` chain to its end and returns the last
    /// non-null exception, which is `self` when there is no inner exception.
    /// `virtual` and not `final`, so `open`.
    open func GetBaseException() -> CNAException {
        var deepest: CNAException = self
        var next = InnerException
        while let current = next {
            deepest = current
            next = current.InnerException
        }
        return deepest
    }

    /// `Exception.GetClassName()` — the CLR full type name.
    ///
    /// `GetClassName` reads `_className`, which the runtime fills from
    /// `RuntimeTypeHandle::ConstructName`: the namespace-qualified CLR name of
    /// the object's own type. The Swift namespace enums mirror the CLR
    /// namespaces exactly, so the reflected Swift name of a projected XNA
    /// exception is that CLR name with the module component in front, and
    /// dropping the module recovers it. The three support classes are the only
    /// types whose CLR name is not recoverable that way — nothing named
    /// `CNAException` exists in `mscorlib` — so they are the only entries in
    /// the table below.
    ///
    /// This is internal: it is the CLR machinery `Message` needs, not a
    /// projected member, and inventing a public `ClassName` would add a member
    /// `mscorlib` does not declare. It is deliberately a single
    /// implementation rather than a per-class override, because an override on
    /// `CNAExternalException` would be inherited by the three XNA subclasses
    /// that derive from it and would report the base's name for all of them. A
    /// test asserts the result for every projected XNA exception and for each
    /// support class.
    internal var cnaClassName: String {
        if let storedClassName { return storedClassName }
        let reflected = String(reflecting: type(of: self))
        // Only THIS module's qualifier is removed, and only when it is
        // actually there. Stripping whichever component happens to come first
        // would also strip a consumer's module from their own subclass of one
        // of the two unsealed XNA exceptions, leaving a bare name where the
        // CLR reports a qualified one -- so the prefix is matched, not
        // counted. The qualifier is read back from a type known to live here
        // rather than written down, so renaming the module cannot desynchronize
        // it.
        let unqualified: String
        if reflected.hasPrefix(CNAException.moduleQualifier) {
            unqualified = String(
                reflected.dropFirst(CNAException.moduleQualifier.count))
        } else {
            unqualified = reflected
        }
        return CNAException.supportClassNames[unqualified] ?? unqualified
    }

    /// `"CNA."` — this module's own reflection qualifier, derived from a type
    /// that is certainly in it.
    private static let moduleQualifier: String = {
        let reflected = String(reflecting: CNAException.self)
        guard let separator = reflected.firstIndex(of: ".") else { return "" }
        return String(reflected[...separator])
    }()

    /// The CLR names of the support classes themselves.
    private static let supportClassNames: [String: String] = [
        "CNAException": "System.Exception",
        "CNASystemException": "System.SystemException",
        "CNAExternalException": "System.Runtime.InteropServices.ExternalException",
        "CNAArgumentException": "System.ArgumentException",
        "CNAArgumentNullException": "System.ArgumentNullException",
        "CNAArgumentOutOfRangeException": "System.ArgumentOutOfRangeException",
        "CNANotSupportedException": "System.NotSupportedException",
        "CNAInvalidOperationException": "System.InvalidOperationException",
        "CNAKeyNotFoundException": "System.Collections.Generic.KeyNotFoundException",
        "CNANullReferenceException": "System.NullReferenceException",
        "CNAIndexOutOfRangeException": "System.IndexOutOfRangeException",
        "CNAObjectDisposedException": "System.ObjectDisposedException",
        "CNAIOException": "System.IO.IOException",
        "CNAEndOfStreamException": "System.IO.EndOfStreamException",
        "CNAFileNotFoundException": "System.IO.FileNotFoundException",
        "CNAFormatException": "System.FormatException",
    ]
}

/// The `System.SystemException` projection.
///
/// Declares no member of its own — `mscorlib` gives it four constructors and
/// nothing else — but it is not a marker. Each constructor sets HResult to
/// `COR_E_SYSTEM`, and the parameterless one substitutes the
/// `Arg_SystemException` resource message, so a projection that skipped this
/// link would report a different message and a different HResult for
/// `ExternalException`'s own parameterless path.
open class CNASystemException: CNAException {
    /// `SystemException..ctor()`.
    ///
    /// The IL is
    /// `base..ctor(GetResourceString("Arg_SystemException")); SetErrorCode(0x80131501)`.
    /// The message is substituted, so `Message` is `"System error."` and never
    /// the `Exception_WasThrown` default.
    public override init() {
        super.init(message: CNAException.argSystemExceptionMessage)
        HResult = CNAException.corSystemHResult
    }

    /// `SystemException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAException.corSystemHResult
    }

    /// `SystemException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAException.corSystemHResult
    }

    /// The serialization constructor forwards without replacing the restored
    /// HResult, exactly as `SystemException` does.
    public override init(
        info: CNASerializationInfo, context: CNAStreamingContext
    ) {
        super.init(info: info, context: context)
    }
}

/// The `System.Runtime.InteropServices.ExternalException` projection.
///
/// The direct base of `InstancePlayLimitException`, `NoAudioHardwareException`
/// and `StorageDeviceNotConnectedException`. It substitutes its own default
/// message, overwrites the HResult its base has just set, and adds the one
/// public member the family has: `ErrorCode`.
open class CNAExternalException: CNASystemException {
    /// `ExternalException..ctor()`.
    ///
    /// `base..ctor(GetResourceString("Arg_ExternalException"))` then
    /// `SetErrorCode(0x80004005)`. Both the message and the HResult replace
    /// what `SystemException..ctor(String)` produced, in that order.
    public override init() {
        super.init(message: CNAException.argExternalExceptionMessage)
        HResult = CNAException.externalExceptionHResult
    }

    /// `ExternalException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAException.externalExceptionHResult
    }

    /// `ExternalException..ctor(String message, Exception inner)`.
    ///
    /// The CLR parameter is named `inner` here and `innerException` on
    /// `System.Exception`. The Swift label follows the base whose initializer
    /// this overrides, because Swift requires an override to keep the
    /// inherited signature; the CLR name difference is recorded rather than
    /// projected, and it is invisible to a caller of any XNA subclass, whose
    /// own labels come from its own metadata.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAException.externalExceptionHResult
    }

    /// The serialization constructor preserves the HResult restored by the
    /// base rather than applying `E_FAIL` again.
    public override init(
        info: CNASerializationInfo, context: CNAStreamingContext
    ) {
        super.init(info: info, context: context)
    }

    /// `ExternalException..ctor(String message, Int32 errorCode)`.
    ///
    /// The one constructor of this family that does not use the fixed HResult:
    /// it calls `SetErrorCode(errorCode)` with the caller's value, which is
    /// exactly what `ErrorCode` then reports.
    public init(message: String?, errorCode: Int32) {
        super.init(message: message)
        HResult = errorCode
    }

    /// `ExternalException.ErrorCode`.
    ///
    /// `get_ErrorCode` is `ldarg.0; call get_HResult; ret` — the same value,
    /// exposed publicly and get-only. `virtual` and not `final`, so `open`.
    open var ErrorCode: Int32 { HResult }
}

// ---------------------------------------------------------------------------
// The six raised exception families.
//
// Foundation 30 projected the three exception BASES eight XNA types inherit
// from. These six are the classes the support layer actually THROWS, and until
// this milestone every one of those failures came out of `CNAError` with the
// right message and the wrong class — so `catch is CNAArgumentException` could
// not work and `ParamName` did not exist.
//
// Every fact below is read out of the admitted `mscorlib`:
//
//   * the base of each class, which decides which `catch` clause sees it;
//   * the HResult each constructor assigns;
//   * the resource message each parameterless constructor substitutes;
//   * `ArgumentException.get_Message`'s composition, including the newline —
//     `System.Environment::get_NewLine`'s entire body in the admitted assembly
//     is `ldstr "\r\n"; ret`, so the separator is a pinned IL literal and not
//     an assertion about the platform this binding runs on. That literal is
//     registered in `bcl-authorities.json` under `selectedIlLiterals` and the
//     BCL authority audit compares it.
//
// `CNAError` remains the separate CNA runtime channel and is untouched by
// this: a native library failure, an owner-thread violation and a stale
// runtime generation are not CLR argument failures and do not become one.
// ---------------------------------------------------------------------------

/// The `System.ArgumentException` projection.
///
/// The one class in this family that carries state of its own: `m_paramName`,
/// the public `ParamName` that reads it, and a `Message` override that appends
/// the parameter name to the base message. That override is why an argument
/// exception's `Message` is not the string its constructor was given.
open class CNAArgumentException: CNASystemException {
    /// `Arg_ArgumentException`.
    internal static let argArgumentExceptionMessage =
        "Value does not fall within the expected range."

    /// `Arg_ParamName_Name`, the template `get_Message` formats.
    internal static let argParamNameNameFormat = "Parameter name: {0}"

    /// `System.Environment.NewLine`, whose whole body in the admitted
    /// assembly is `ldstr "\r\n"; ret`.
    internal static let environmentNewLine = "\r\n"

    /// `COR_E_ARGUMENT`, the HResult every constructor assigns.
    internal static let corArgumentHResult = Int32(bitPattern: 0x8007_0057)

    // `ArgumentException.m_paramName`, stored unvalidated.
    private let storedParamName: String?

    /// `ArgumentException..ctor()` — substitutes `Arg_ArgumentException`.
    public override init() {
        storedParamName = nil
        super.init(message: CNAArgumentException.argArgumentExceptionMessage)
        HResult = CNAArgumentException.corArgumentHResult
    }

    /// `ArgumentException..ctor(String message)`.
    public override init(message: String?) {
        storedParamName = nil
        super.init(message: message)
        HResult = CNAArgumentException.corArgumentHResult
    }

    /// `ArgumentException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        storedParamName = nil
        super.init(message: message, innerException: innerException)
        HResult = CNAArgumentException.corArgumentHResult
    }

    /// `ArgumentException..ctor(String message, String paramName)`.
    ///
    /// The message comes first. `ArgumentNullException` and
    /// `ArgumentOutOfRangeException` both declare a two-argument constructor
    /// whose parameters are the other way round, which is exactly the mistake
    /// this family invites; the labels here keep the CLR's own names so a
    /// transposition cannot be silent.
    public init(message: String?, paramName: String?) {
        storedParamName = paramName
        super.init(message: message)
        HResult = CNAArgumentException.corArgumentHResult
    }

    /// `ArgumentException..ctor(String message, String paramName, Exception innerException)`.
    public init(message: String?, paramName: String?, innerException: CNAException?) {
        storedParamName = paramName
        super.init(message: message, innerException: innerException)
        HResult = CNAArgumentException.corArgumentHResult
    }

    /// `ArgumentException.ParamName`.
    ///
    /// `virtual` and not `final` in the metadata, so `open`. It returns the
    /// field, which is nil unless a paramName-taking constructor supplied one.
    open var ParamName: String? { storedParamName }

    /// `ArgumentException.Message`.
    ///
    /// The IL reads `base.Message`, and when `ParamName` is neither null nor
    /// empty concatenates it with `Environment.NewLine` and the formatted
    /// `Arg_ParamName_Name`. `String.IsNullOrEmpty` is the test, so an empty
    /// parameter name adds nothing — which is not the same as a nil one and is
    /// reproduced rather than collapsed.
    open override var Message: String {
        let base = super.Message
        guard let name = storedParamName, !name.isEmpty else { return base }
        let clause = CNAArgumentException.argParamNameNameFormat
            .replacingOccurrences(of: "{0}", with: name)
        return base + CNAArgumentException.environmentNewLine + clause
    }
}

/// The `System.ArgumentNullException` projection.
///
/// Declares no member of its own. Its two contributions are its identity —
/// `catch is CNAArgumentNullException` is narrower than
/// `catch is CNAArgumentException` — and the HResult `0x80004003`, which is
/// `E_POINTER` and not the `COR_E_ARGUMENT` its base has just assigned.
open class CNAArgumentNullException: CNAArgumentException {
    /// `ArgumentNull_Generic`.
    internal static let argumentNullGenericMessage = "Value cannot be null."

    /// `E_POINTER`.
    internal static let argumentNullHResult = Int32(bitPattern: 0x8000_4003)

    /// `ArgumentNullException..ctor()`.
    public override init() {
        super.init(message: CNAArgumentNullException.argumentNullGenericMessage)
        HResult = CNAArgumentNullException.argumentNullHResult
    }

    /// `ArgumentNullException..ctor(String paramName)`.
    ///
    /// The single-argument constructor of this class takes a **parameter
    /// name**, not a message: the IL passes `ArgumentNull_Generic` as the
    /// message and the argument as the paramName. Its label says so.
    public init(paramName: String?) {
        super.init(
            message: CNAArgumentNullException.argumentNullGenericMessage,
            paramName: paramName)
        HResult = CNAArgumentNullException.argumentNullHResult
    }

    /// `ArgumentNullException..ctor(String message, Exception innerException)`.
    ///
    /// This one *does* take a message, which is why it is not merged with the
    /// constructor above.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAArgumentNullException.argumentNullHResult
    }

    /// `ArgumentNullException..ctor(String paramName, String message)`.
    ///
    /// The parameters are transposed with respect to
    /// `ArgumentException..ctor(message, paramName)`: the IL loads `ldarg.2`
    /// then `ldarg.1`. This is the overload XNA's `GameServiceContainer`
    /// selects, and getting the order wrong would put the parameter name in
    /// the message and the message in the parameter name.
    public init(paramName: String?, message: String?) {
        super.init(message: message, paramName: paramName)
        HResult = CNAArgumentNullException.argumentNullHResult
    }

    /// `ArgumentNullException..ctor(String message)` is **not** declared by
    /// `mscorlib` and is not projected. Swift would otherwise inherit
    /// `CNAArgumentException.init(message:)`, which takes a message where this
    /// class's own one-argument constructor takes a parameter name, so it is
    /// overridden to keep the two distinguishable and to assign this class's
    /// HResult.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAArgumentNullException.argumentNullHResult
    }
}

/// The `System.ArgumentOutOfRangeException` projection.
///
/// `mscorlib` gives it an `ActualValue` of type `System.Object` and a `Message`
/// override that appends `ArgumentOutOfRange_ActualValue` formatted with
/// `actualValue.ToString()`. Neither is projected, and the reason is the
/// `ToString()`: `System.Object.ToString()` is a virtual call whose result for
/// an arbitrary object is not reconstructible from IL, and a Swift
/// `String(describing:)` in its place would be this projection's formatting
/// rather than the CLR's. The constructor that would store one is therefore
/// also absent, which keeps the omission consistent: `ActualValue` is nil on
/// every instance this projection can build, and the `Message` branch that
/// reads it is unreachable rather than wrong.
open class CNAArgumentOutOfRangeException: CNAArgumentException {
    /// `Arg_ArgumentOutOfRangeException`, through the cached `RangeMessage`.
    internal static let argArgumentOutOfRangeMessage =
        "Specified argument was out of the range of valid values."

    /// `COR_E_ARGUMENTOUTOFRANGE`.
    internal static let corArgumentOutOfRangeHResult = Int32(bitPattern: 0x8013_1502)

    /// `ArgumentOutOfRangeException..ctor()`.
    public override init() {
        super.init(message: CNAArgumentOutOfRangeException.argArgumentOutOfRangeMessage)
        HResult = CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
    }

    /// `ArgumentOutOfRangeException..ctor(String paramName)`.
    ///
    /// Like `ArgumentNullException`, the single argument is a parameter name
    /// and the message is the substituted range message.
    public init(paramName: String?) {
        super.init(
            message: CNAArgumentOutOfRangeException.argArgumentOutOfRangeMessage,
            paramName: paramName)
        HResult = CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
    }

    /// `ArgumentOutOfRangeException..ctor(String paramName, String message)`.
    ///
    /// Transposed with respect to `ArgumentException..ctor(message, paramName)`,
    /// exactly as `ArgumentNullException`'s is. This is the overload every
    /// `ThrowHelper` range failure selects.
    public init(paramName: String?, message: String?) {
        super.init(message: message, paramName: paramName)
        HResult = CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
    }

    /// `ArgumentOutOfRangeException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
    }

    /// Inherited `(String message)`, overridden only to assign this class's
    /// HResult.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
    }
}

/// The `System.NotSupportedException` projection.
///
/// Declares no member; its whole contribution is the class identity and the
/// HResult `0x80131515`.
open class CNANotSupportedException: CNASystemException {
    /// `Arg_NotSupportedException`.
    internal static let argNotSupportedMessage = "Specified method is not supported."

    /// `COR_E_NOTSUPPORTED`.
    internal static let corNotSupportedHResult = Int32(bitPattern: 0x8013_1515)

    /// `NotSupportedException..ctor()`.
    public override init() {
        super.init(message: CNANotSupportedException.argNotSupportedMessage)
        HResult = CNANotSupportedException.corNotSupportedHResult
    }

    /// `NotSupportedException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNANotSupportedException.corNotSupportedHResult
    }

    /// `NotSupportedException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNANotSupportedException.corNotSupportedHResult
    }
}

/// The `System.InvalidOperationException` projection.
open class CNAInvalidOperationException: CNASystemException {
    /// `Arg_InvalidOperationException`.
    internal static let argInvalidOperationMessage =
        "Operation is not valid due to the current state of the object."

    /// `COR_E_INVALIDOPERATION`.
    internal static let corInvalidOperationHResult = Int32(bitPattern: 0x8013_1509)

    /// `InvalidOperationException..ctor()`.
    public override init() {
        super.init(message: CNAInvalidOperationException.argInvalidOperationMessage)
        HResult = CNAInvalidOperationException.corInvalidOperationHResult
    }

    /// `InvalidOperationException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAInvalidOperationException.corInvalidOperationHResult
    }

    /// `InvalidOperationException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAInvalidOperationException.corInvalidOperationHResult
    }
}

/// The `System.ObjectDisposedException` projection.
///
/// Read out of the admitted `mscorlib` `5634668d…acc63`. It is the
/// `CNAArgumentException` pattern with one payload property, one HResult and a
/// composed `Message` — with the twist that its getter substitutes the empty
/// string for a nil field, which `ArgumentException.ParamName` does not.
open class CNAObjectDisposedException: CNAInvalidOperationException {
    /// `ObjectDisposed_Generic`, the message the one-argument constructor
    /// substitutes.
    internal static let objectDisposedGenericMessage =
        "Cannot access a disposed object."

    /// `ObjectDisposed_ObjectName_Name`, the template `get_Message` formats.
    internal static let objectDisposedObjectNameFormat = "Object name: '{0}'."

    /// `COR_E_OBJECTDISPOSED`.
    internal static let corObjectDisposedHResult = Int32(bitPattern: 0x8013_1622)

    // `ObjectDisposedException.objectName`, stored unvalidated.
    private let storedObjectName: String?

    /// `ObjectDisposedException..ctor(String objectName)`.
    ///
    /// The single-argument constructor takes an **object name**, not a
    /// message: the IL forwards to `.ctor(objectName, ObjectDisposed_Generic)`.
    public init(objectName: String?) {
        storedObjectName = objectName
        super.init(
            message: CNAObjectDisposedException.objectDisposedGenericMessage)
        HResult = CNAObjectDisposedException.corObjectDisposedHResult
    }

    /// `ObjectDisposedException..ctor(String objectName, String message)`.
    ///
    /// The designated one, and the **object name comes first** — the opposite
    /// order from `ArgumentException(message:paramName:)`. The labels keep the
    /// CLR's own names so a transposition cannot be silent.
    public init(objectName: String?, message: String?) {
        storedObjectName = objectName
        super.init(message: message)
        HResult = CNAObjectDisposedException.corObjectDisposedHResult
    }

    /// `ObjectDisposedException..ctor(String message, Exception innerException)`.
    ///
    /// This one takes a **message** first and stores no object name at all,
    /// which is why it cannot share a label with the constructor above.
    public override init(message: String?, innerException: CNAException?) {
        storedObjectName = nil
        super.init(message: message, innerException: innerException)
        HResult = CNAObjectDisposedException.corObjectDisposedHResult
    }

    /// `ObjectDisposedException.ObjectName`.
    ///
    /// Non-Optional: the getter is `objectName ?? String.Empty`, so it is
    /// proven never to answer null — unlike `ArgumentException.ParamName`,
    /// which returns its field as-is.
    public var ObjectName: String { storedObjectName ?? "" }

    /// `ObjectDisposedException.Message`.
    ///
    /// `String.IsNullOrEmpty(ObjectName)` decides, and because `ObjectName`
    /// already collapses nil to empty, a nil and an empty object name compose
    /// identically here — which is not true of the argument family.
    open override var Message: String {
        let base = super.Message
        let name = ObjectName
        guard !name.isEmpty else { return base }
        let clause = CNAObjectDisposedException.objectDisposedObjectNameFormat
            .replacingOccurrences(of: "{0}", with: name)
        return base + CNAArgumentException.environmentNewLine + clause
    }
}

/// The `System.Collections.Generic.KeyNotFoundException` projection.
///
/// Its base is `SystemException` directly — **not** `ArgumentException` — so a
/// missing key is not an argument failure and `catch is CNAArgumentException`
/// does not see it. `ThrowHelper.ThrowKeyNotFoundException`'s whole body is
/// `new KeyNotFoundException()`, so every dictionary lookup failure carries the
/// substituted `Arg_KeyNotFound` message.
open class CNAKeyNotFoundException: CNASystemException {
    /// `Arg_KeyNotFound`.
    internal static let argKeyNotFoundMessage =
        "The given key was not present in the dictionary."

    /// `COR_E_KEYNOTFOUND`.
    internal static let corKeyNotFoundHResult = Int32(bitPattern: 0x8013_1577)

    /// `KeyNotFoundException..ctor()`.
    public override init() {
        super.init(message: CNAKeyNotFoundException.argKeyNotFoundMessage)
        HResult = CNAKeyNotFoundException.corKeyNotFoundHResult
    }

    /// `KeyNotFoundException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAKeyNotFoundException.corKeyNotFoundHResult
    }

    /// `KeyNotFoundException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAKeyNotFoundException.corKeyNotFoundHResult
    }
}

/// The `System.NullReferenceException` projection.
///
/// Raised where the CLR's own `ldfld` would raise it: `CurveKey.CompareTo`
/// performs no null check, so ECMA-335 decides the class. The **message** the
/// runtime attaches is produced by CLR native code and is in no admitted IL;
/// what is projected is the substituted `Arg_NullReferenceException` that
/// `mscorlib`'s own parameterless constructor carries.
open class CNANullReferenceException: CNASystemException {
    /// `Arg_NullReferenceException`.
    internal static let argNullReferenceMessage =
        "Object reference not set to an instance of an object."

    /// `COR_E_NULLREFERENCE`, which is `E_POINTER`.
    internal static let corNullReferenceHResult = Int32(bitPattern: 0x8000_4003)

    /// `NullReferenceException..ctor()`.
    public override init() {
        super.init(message: CNANullReferenceException.argNullReferenceMessage)
        HResult = CNANullReferenceException.corNullReferenceHResult
    }

    /// `NullReferenceException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNANullReferenceException.corNullReferenceHResult
    }

    /// `NullReferenceException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNANullReferenceException.corNullReferenceHResult
    }
}

/// The `System.IndexOutOfRangeException` projection.
///
/// The one **sealed** class in this family, so it is `public final` where the
/// others are `open`; `mscorlib` sets the sealed bit and this projection does
/// not weaken it. Raised where `ldelema` would raise it: the indexed vector
/// transforms validate the two array lengths and never the indices.
public final class CNAIndexOutOfRangeException: CNASystemException {
    /// `Arg_IndexOutOfRangeException`.
    internal static let argIndexOutOfRangeMessage =
        "Index was outside the bounds of the array."

    /// `COR_E_INDEXOUTOFRANGE`.
    internal static let corIndexOutOfRangeHResult = Int32(bitPattern: 0x8013_1508)

    /// `IndexOutOfRangeException..ctor()`.
    public override init() {
        super.init(message: CNAIndexOutOfRangeException.argIndexOutOfRangeMessage)
        HResult = CNAIndexOutOfRangeException.corIndexOutOfRangeHResult
    }

    /// `IndexOutOfRangeException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAIndexOutOfRangeException.corIndexOutOfRangeHResult
    }

    /// `IndexOutOfRangeException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAIndexOutOfRangeException.corIndexOutOfRangeHResult
    }
}

/// `System.IO.IOException`.
///
/// Admitted for the chain rather than for a member: nothing in this binding
/// raises a bare `IOException`. It sits between `CNASystemException` and
/// `CNAFileNotFoundException`, and a consumer catching it must catch the
/// missing-title-asset failure too, which is the only reason its identity
/// matters here.
open class CNAIOException: CNASystemException {
    /// `COR_E_IO`, the HResult every constructor assigns.
    internal static let corIOHResult = Int32(bitPattern: 0x8013_1620)

    /// `Environment.GetResourceString("Arg_IOException")`.
    internal static let argIOExceptionMessage = "I/O error occurred."

    /// `IOException..ctor()` — substitutes `Arg_IOException`.
    public override init() {
        super.init(message: CNAIOException.argIOExceptionMessage)
        HResult = CNAIOException.corIOHResult
    }

    /// `IOException..ctor(String message)`.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAIOException.corIOHResult
    }

    /// `IOException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAIOException.corIOHResult
    }
}

/// `System.IO.EndOfStreamException`.
open class CNAEndOfStreamException: CNAIOException {
    internal static let corEndOfStreamHResult = Int32(bitPattern: 0x8007_0026)
    internal static let argEndOfStreamMessage =
        "Attempted to read past the end of the stream."

    public override init() {
        super.init(message: CNAEndOfStreamException.argEndOfStreamMessage)
        HResult = CNAEndOfStreamException.corEndOfStreamHResult
    }

    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAEndOfStreamException.corEndOfStreamHResult
    }

    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAEndOfStreamException.corEndOfStreamHResult
    }
}

/// `System.FormatException`.
open class CNAFormatException: CNASystemException {
    internal static let corFormatHResult = Int32(bitPattern: 0x8013_1537)
    internal static let argFormatMessage =
        "One of the identified items was in an invalid format."

    public override init() {
        super.init(message: CNAFormatException.argFormatMessage)
        HResult = CNAFormatException.corFormatHResult
    }

    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAFormatException.corFormatHResult
    }

    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAFormatException.corFormatHResult
    }
}

/// `System.IO.FileNotFoundException`.
///
/// The class `TitleContainer.OpenStream` raises for a missing asset.
///
/// **A measured subset, and the boundary is exact.** The CLR type has six
/// constructors; the three here are the ones whose `Message` is decided
/// entirely in managed code. The three that carry a `fileName` are not
/// projected, and the reason is in `SetMessageField`: when such an instance has
/// no message of its own, `Message` comes from
/// `FileLoadException.FormatFileLoadExceptionMessage`, which is an internal
/// call into the CLR. Reproducing those constructors would mean inventing a
/// message the authority does not give, so `FileName` is `nil` on every
/// instance this binding can produce — which is exactly what the projected
/// constructors leave it, and what XNA's own path produces.
open class CNAFileNotFoundException: CNAIOException {
    /// `COR_E_FILENOTFOUND`, the HResult every constructor assigns. Note that
    /// it is a Win32 facility code, not the `0x8013…` the rest of this
    /// hierarchy uses.
    internal static let corFileNotFoundHResult = Int32(bitPattern: 0x8007_0002)

    /// `Environment.GetResourceString("IO.FileNotFound")`.
    internal static let ioFileNotFoundMessage = "Unable to find the specified file."

    /// `FileNotFoundException..ctor()` — substitutes `IO.FileNotFound`.
    public override init() {
        super.init(message: CNAFileNotFoundException.ioFileNotFoundMessage)
        HResult = CNAFileNotFoundException.corFileNotFoundHResult
    }

    /// `FileNotFoundException..ctor(String message)`.
    ///
    /// The whole of XNA's path. `SetMessageField` returns at its first
    /// instruction for an instance built this way, because `_message` is not
    /// null, so `Message` is the argument unchanged.
    public override init(message: String?) {
        super.init(message: message)
        HResult = CNAFileNotFoundException.corFileNotFoundHResult
    }

    /// `FileNotFoundException..ctor(String message, Exception innerException)`.
    public override init(message: String?, innerException: CNAException?) {
        super.init(message: message, innerException: innerException)
        HResult = CNAFileNotFoundException.corFileNotFoundHResult
    }

    /// `FileNotFoundException.FileName`, `_fileName` read straight back.
    ///
    /// Always `nil` here: see the note on the type. The property is projected
    /// rather than omitted because it is the shape a consumer writes against,
    /// and answering `nil` is what the constructors above actually leave.
    open var FileName: String? { nil }
}
