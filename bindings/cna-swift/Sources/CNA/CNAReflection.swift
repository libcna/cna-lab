// SPDX-License-Identifier: MIT

/// The member kinds needed by constructor-backed InstanceDescriptor values.
public enum CNAMemberTypes: Int32 {
    case Constructor = 1
}

/// Demand-driven `System.Reflection.MemberInfo` projection.
open class CNAMemberInfo {
    public let Name: String
    public let DeclaringType: Any.Type?
    public let ReflectedType: Any.Type?
    public let MemberType: CNAMemberTypes

    internal init(name: String, declaringType: Any.Type?, memberType: CNAMemberTypes) {
        Name = name
        DeclaringType = declaringType
        ReflectedType = declaringType
        MemberType = memberType
    }

    open func Equals(_ obj: Any?) -> Bool {
        guard let other = obj as? CNAMemberInfo else { return false }
        return self === other
    }

    open func GetHashCode() -> Int32 {
        Int32(truncatingIfNeeded: ObjectIdentifier(self).hashValue)
    }
}

public final class CNAParameterInfo {
    public let Name: String?
    public let ParameterType: Any.Type
    public let Position: Int32

    internal init(name: String?, parameterType: Any.Type, position: Int32) {
        Name = name
        ParameterType = parameterType
        Position = position
    }
}

/// A constructor identity with ordered parameters and a real invocation path.
public final class CNAConstructorInfo: CNAMemberInfo {
    private let parameters: [CNAParameterInfo]
    private let invokeBody: ([Any?]) throws -> Any?

    internal init(
        declaringType: Any.Type,
        parameters: [CNAParameterInfo],
        invoke: @escaping ([Any?]) throws -> Any?
    ) {
        self.parameters = parameters
        invokeBody = invoke
        super.init(name: ".ctor", declaringType: declaringType, memberType: .Constructor)
    }

    public func GetParameters() -> [CNAParameterInfo] { parameters }

    public func Invoke(_ parameters: [Any?]?) throws -> Any? {
        try invokeBody(parameters ?? [])
    }
}

/// Self-contained projection of
/// `System.ComponentModel.Design.Serialization.InstanceDescriptor`.
public final class CNAInstanceDescriptor {
    public let MemberInfo: CNAMemberInfo
    public let Arguments: [Any?]
    public let IsComplete: Bool

    public init(
        _ member: CNAMemberInfo?,
        arguments: [Any?]?,
        isComplete: Bool
    ) throws {
        guard let member else { throw CNAArgumentNullException(paramName: "member") }
        guard member is CNAConstructorInfo else {
            throw CNAArgumentException(message: "The member must describe a constructor.")
        }
        MemberInfo = member
        Arguments = arguments ?? []
        IsComplete = isComplete
    }

    public convenience init(_ member: CNAMemberInfo?, arguments: [Any?]?) throws {
        try self.init(member, arguments: arguments, isComplete: true)
    }

    public func Invoke() throws -> Any? {
        guard let constructor = MemberInfo as? CNAConstructorInfo else {
            throw CNAInvalidOperationException(message: "The descriptor member is not invocable.")
        }
        return try constructor.Invoke(Arguments)
    }
}
