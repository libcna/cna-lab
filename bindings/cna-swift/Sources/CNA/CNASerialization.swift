// SPDX-License-Identifier: MIT

/// The demand-driven projection of
/// `System.Runtime.Serialization.SerializationInfo` used by the two XNA
/// protected serialization constructors.
///
/// The CLR type's public constructor requires `IFormatterConverter`, whose
/// surface in turn names unrelated conversion families. Constructors are not
/// inherited through an XNA constructor parameter, so that branch is not part
/// of this closure. The managed serialization bridge creates this carrier
/// internally with the exact Exception fields its base constructor reads.
public final class CNASerializationInfo {
    internal let className: String
    internal let message: String?
    internal let innerException: CNAException?
    internal let helpLink: String?
    internal let hResult: Int32

    internal init(
        className: String,
        message: String?,
        innerException: CNAException?,
        helpLink: String?,
        hResult: Int32
    ) {
        self.className = className
        self.message = message
        self.innerException = innerException
        self.helpLink = helpLink
        self.hResult = hResult
    }
}

/// The demand-driven projection of
/// `System.Runtime.Serialization.StreamingContext`.
///
/// Exception deserialization observes `State` only to rewrite CLR stack-trace
/// fields for cross-AppDomain remoting. Those fields are deliberately outside
/// the selected Exception projection, so the context remains an opaque value
/// at the XNA boundary while retaining its measured state internally.
public struct CNAStreamingContext {
    internal let state: Int32
    internal let additionalContext: Any?

    internal init(state: Int32 = 0, additionalContext: Any? = nil) {
        self.state = state
        self.additionalContext = additionalContext
    }
}
