// SPDX-License-Identifier: MIT

import Foundation

/// The demand-driven `System.Collections.IDictionary` projection used by the
/// XNA Design converters. Keys and values remain object-typed, as they are in
/// the CLR interface; concrete callers may provide any reference-backed bag.
public protocol CNAIDictionary: AnyObject {
    var IsFixedSize: Bool { get }
    var IsReadOnly: Bool { get }
    var Keys: [Any] { get }
    var Values: [Any?] { get }

    func Item(_ key: Any) -> Any?
    func SetItem(_ key: Any, _ value: Any?) throws
    func Add(_ key: Any, value: Any?) throws
    func Clear()
    func Contains(_ key: Any) -> Bool
    func Remove(_ key: Any)
}

/// A small culture object carrying precisely the number/list formatting state
/// used by XNA's math converters. Microsoft assemblies are authority only;
/// the packaged runtime remains self-contained.
open class CNACultureInfo {
    public static var CurrentCulture: CNACultureInfo {
        CNACultureInfo(locale: Locale.current, useUserOverride: true)
    }

    public static let InvariantCulture = CNACultureInfo(
        locale: Locale(identifier: "en_US_POSIX"), useUserOverride: false)

    public let Name: String
    public let UseUserOverride: Bool
    public let TextInfo: CNATextInfo
    internal let decimalSeparator: String

    public convenience init(_ name: String) {
        self.init(name, useUserOverride: true)
    }

    public convenience init(_ name: String, useUserOverride: Bool) {
        self.init(locale: Locale(identifier: name), useUserOverride: useUserOverride)
    }

    private init(locale: Locale, useUserOverride: Bool) {
        Name = locale.identifier
        UseUserOverride = useUserOverride
        let decimal = locale.decimalSeparator ?? "."
        decimalSeparator = decimal
        // .NET's comma-decimal cultures use semicolon to keep component lists
        // unambiguous; dot-decimal cultures use comma.
        TextInfo = CNATextInfo(listSeparator: decimal == "," ? ";" : ",")
    }

    open func Clone() -> Any? { CNACultureInfo(Name, useUserOverride: UseUserOverride) }
    open func Equals(_ value: Any?) -> Bool {
        guard let other = value as? CNACultureInfo else { return false }
        return Name == other.Name && UseUserOverride == other.UseUserOverride
    }
    open func GetHashCode() -> Int32 { Int32(truncatingIfNeeded: Name.hashValue) }
    open func ToString() -> String { Name }
}

public final class CNATextInfo {
    public let ListSeparator: String

    internal init(listSeparator: String) {
        ListSeparator = listSeparator
    }
}

/// The selected `ITypeDescriptorContext` surface. It refines the already
/// projected IServiceProvider, matching System.dll.
public protocol CNATypeDescriptorContext: CNAServiceProvider {
    var Container: Any? { get }
    var Instance: Any? { get }
    var PropertyDescriptor: CNAPropertyDescriptor? { get }

    func OnComponentChanged()
    func OnComponentChanging() -> Bool
}

/// Functional projection of the property descriptors XNA creates for fields
/// and properties of its value types.
open class CNAPropertyDescriptor {
    private let getter: (Any?) throws -> Any?
    private let setter: ((Any?, Any?) throws -> Void)?

    public let Name: String
    public let ComponentType: Any.Type
    public let PropertyType: Any.Type

    public init(
        name: String,
        componentType: Any.Type,
        propertyType: Any.Type,
        get: @escaping (Any?) throws -> Any?,
        set: ((Any?, Any?) throws -> Void)? = nil
    ) {
        Name = name
        ComponentType = componentType
        PropertyType = propertyType
        getter = get
        setter = set
    }

    open var IsReadOnly: Bool { false }
    open var IsLocalizable: Bool { false }
    open var SupportsChangeEvents: Bool { false }
    open var Converter: CNATypeConverter { CNATypeConverter() }

    open func CanResetValue(_ component: Any?) -> Bool { false }
    open func ResetValue(_ component: Any?) {}
    open func ShouldSerializeValue(_ component: Any?) -> Bool { true }
    open func GetValue(_ component: Any?) throws -> Any? { try getter(component) }

    open func SetValue(_ component: Any?, value: Any?) throws {
        // XNA's descriptors are writable. A reference-backed component can be
        // changed by the supplied setter; a boxed Swift value has no mutable
        // identity, so the converter's CreateInstance path is the faithful
        // value-type editing operation.
        if let setter { try setter(component, value) }
    }

    open func Equals(_ obj: Any?) -> Bool {
        guard let other = obj as? CNAPropertyDescriptor else { return false }
        return Name == other.Name && ComponentType == other.ComponentType
    }

    open func GetHashCode() -> Int32 {
        Int32(truncatingIfNeeded: Name.hashValue ^ ObjectIdentifier(ComponentType).hashValue)
    }
}

/// Reference-backed, optionally read-only property collection.
open class CNAPropertyDescriptorCollection {
    public static let Empty = CNAPropertyDescriptorCollection([], readOnly: true)

    private var values: [CNAPropertyDescriptor]
    private let readOnly: Bool

    public init(_ properties: [CNAPropertyDescriptor]) {
        values = properties
        readOnly = false
    }

    public init(_ properties: [CNAPropertyDescriptor], readOnly: Bool) {
        values = properties
        self.readOnly = readOnly
    }

    public var Count: Int32 { Int32(values.count) }

    open func Item(_ index: Int32) throws -> CNAPropertyDescriptor {
        guard index >= 0, Int(index) < values.count else {
            throw CNAArgumentOutOfRangeException(paramName: "index")
        }
        return values[Int(index)]
    }

    open func Item(_ name: String) -> CNAPropertyDescriptor? {
        Find(name, ignoreCase: false)
    }

    @discardableResult
    public func Add(_ value: CNAPropertyDescriptor) throws -> Int32 {
        try requireMutable()
        values.append(value)
        return Int32(values.count - 1)
    }

    public func Clear() throws {
        try requireMutable()
        values.removeAll(keepingCapacity: false)
    }

    public func Contains(_ value: CNAPropertyDescriptor) -> Bool {
        values.contains { $0 === value || $0.Equals(value) }
    }

    public func CopyTo(_ array: inout [CNAPropertyDescriptor?], index: Int32) throws {
        guard index >= 0, Int(index) + values.count <= array.count else {
            throw CNAArgumentException(message: "Destination array is not long enough.")
        }
        for offset in values.indices { array[Int(index) + offset] = values[offset] }
    }

    open func Find(_ name: String, ignoreCase: Bool) -> CNAPropertyDescriptor? {
        values.first {
            ignoreCase
                ? $0.Name.compare(name, options: .caseInsensitive) == .orderedSame
                : $0.Name == name
        }
    }

    public func IndexOf(_ value: CNAPropertyDescriptor) -> Int32 {
        Int32(values.firstIndex { $0 === value || $0.Equals(value) } ?? -1)
    }

    public func Insert(_ index: Int32, value: CNAPropertyDescriptor) throws {
        try requireMutable()
        guard index >= 0, Int(index) <= values.count else {
            throw CNAArgumentOutOfRangeException(paramName: "index")
        }
        values.insert(value, at: Int(index))
    }

    public func Remove(_ value: CNAPropertyDescriptor) throws {
        try requireMutable()
        if let index = values.firstIndex(where: { $0 === value || $0.Equals(value) }) {
            values.remove(at: index)
        }
    }

    public func RemoveAt(_ index: Int32) throws {
        try requireMutable()
        guard index >= 0, Int(index) < values.count else {
            throw CNAArgumentOutOfRangeException(paramName: "index")
        }
        values.remove(at: Int(index))
    }

    open func Sort() -> CNAPropertyDescriptorCollection {
        CNAPropertyDescriptorCollection(values.sorted { $0.Name < $1.Name }, readOnly: readOnly)
    }

    open func Sort(_ names: [String]) -> CNAPropertyDescriptorCollection {
        let ranks = Dictionary(uniqueKeysWithValues: names.enumerated().map { ($1, $0) })
        return CNAPropertyDescriptorCollection(values.sorted {
            let left = ranks[$0.Name] ?? Int.max
            let right = ranks[$1.Name] ?? Int.max
            return left == right ? $0.Name < $1.Name : left < right
        }, readOnly: readOnly)
    }

    public func GetEnumerator() -> CNAEnumerator<CNAPropertyDescriptor> {
        let list = CNAList<CNAPropertyDescriptor>()
        for value in values { list.Add(value) }
        return list.GetEnumerator()
    }

    private func requireMutable() throws {
        if readOnly { throw CNANotSupportedException(message: "Collection is read-only.") }
    }
}

/// `TypeConverter.StandardValuesCollection`.
public final class CNATypeConverterStandardValuesCollection {
    private let values: [Any?]
    public init(_ values: [Any?]?) { self.values = values ?? [] }
    public var Count: Int32 { Int32(values.count) }
    public func Item(_ index: Int32) -> Any? { values[Int(index)] }
    public func CopyTo(_ array: inout [Any?], index: Int32) {
        for offset in values.indices { array[Int(index) + offset] = values[offset] }
    }
}

/// The .NET Framework 4.0 TypeConverter behavior reached through
/// MathTypeConverter's inherited public surface.
open class CNATypeConverter {
    public init() {}

    open func CanConvertFrom(
        _ context: (any CNATypeDescriptorContext)?, sourceType: Any.Type?
    ) -> Bool {
        guard let sourceType else { return false }
        return sourceType == CNAInstanceDescriptor.self
    }

    public final func CanConvertFrom(_ sourceType: Any.Type?) -> Bool {
        CanConvertFrom(nil, sourceType: sourceType)
    }

    open func CanConvertTo(
        _ context: (any CNATypeDescriptorContext)?, destinationType: Any.Type?
    ) -> Bool {
        destinationType == String.self
    }

    public final func CanConvertTo(_ destinationType: Any.Type?) -> Bool {
        CanConvertTo(nil, destinationType: destinationType)
    }

    open func ConvertFrom(
        _ context: (any CNATypeDescriptorContext)?,
        culture: CNACultureInfo?,
        value: Any?
    ) throws -> Any? {
        if let descriptor = value as? CNAInstanceDescriptor { return try descriptor.Invoke() }
        throw CNAArgumentException(message: "Type converter cannot convert from this value.")
    }

    public final func ConvertFrom(_ value: Any?) throws -> Any? {
        try ConvertFrom(nil, culture: nil, value: value)
    }

    public final func ConvertFromInvariantString(
        _ context: (any CNATypeDescriptorContext)?, text: String
    ) throws -> Any? {
        try ConvertFromString(context, culture: .InvariantCulture, text: text)
    }

    public final func ConvertFromInvariantString(_ text: String) throws -> Any? {
        try ConvertFromInvariantString(nil, text: text)
    }

    public final func ConvertFromString(
        _ context: (any CNATypeDescriptorContext)?,
        culture: CNACultureInfo?,
        text: String
    ) throws -> Any? {
        try ConvertFrom(context, culture: culture, value: text)
    }

    public final func ConvertFromString(
        _ context: (any CNATypeDescriptorContext)?, text: String
    ) throws -> Any? {
        try ConvertFromString(context, culture: .CurrentCulture, text: text)
    }

    public final func ConvertFromString(_ text: String) throws -> Any? {
        try ConvertFrom(nil, culture: nil, value: text)
    }

    open func ConvertTo(
        _ context: (any CNATypeDescriptorContext)?,
        culture: CNACultureInfo?,
        value: Any?,
        destinationType: Any.Type?
    ) throws -> Any? {
        guard let destinationType else {
            throw CNAArgumentNullException(paramName: "destinationType")
        }
        guard destinationType == String.self else {
            throw CNAArgumentException(message: "Type converter cannot convert to the requested type.")
        }
        guard let value else { return "" }
        return String(describing: value)
    }

    public final func ConvertTo(_ value: Any?, destinationType: Any.Type?) throws -> Any? {
        try ConvertTo(nil, culture: nil, value: value, destinationType: destinationType)
    }

    public final func ConvertToInvariantString(
        _ context: (any CNATypeDescriptorContext)?, value: Any?
    ) throws -> String {
        try ConvertToString(context, culture: .InvariantCulture, value: value)
    }

    public final func ConvertToInvariantString(_ value: Any?) throws -> String {
        try ConvertToInvariantString(nil, value: value)
    }

    public final func ConvertToString(
        _ context: (any CNATypeDescriptorContext)?,
        culture: CNACultureInfo?,
        value: Any?
    ) throws -> String {
        try ConvertTo(context, culture: culture, value: value, destinationType: String.self)
            as? String ?? ""
    }

    public final func ConvertToString(
        _ context: (any CNATypeDescriptorContext)?, value: Any?
    ) throws -> String {
        try ConvertToString(context, culture: .CurrentCulture, value: value)
    }

    public final func ConvertToString(_ value: Any?) throws -> String {
        try ConvertToString(nil, culture: .CurrentCulture, value: value)
    }

    open func CreateInstance(
        _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
    ) throws -> Any? { nil }

    public final func CreateInstance(_ propertyValues: (any CNAIDictionary)?) throws -> Any? {
        try CreateInstance(nil, propertyValues: propertyValues)
    }

    public final func GetCreateInstanceSupported() -> Bool {
        GetCreateInstanceSupported(nil)
    }

    open func GetCreateInstanceSupported(
        _ context: (any CNATypeDescriptorContext)?
    ) -> Bool { false }

    open func GetProperties(
        _ context: (any CNATypeDescriptorContext)?,
        value: Any?,
        attributes: [CNAAttribute]
    ) -> CNAPropertyDescriptorCollection? { nil }

    public final func GetProperties(
        _ context: (any CNATypeDescriptorContext)?, value: Any?
    ) -> CNAPropertyDescriptorCollection? {
        GetProperties(context, value: value, attributes: [])
    }

    public final func GetProperties(_ value: Any?) -> CNAPropertyDescriptorCollection? {
        GetProperties(nil, value: value, attributes: [])
    }

    public final func GetPropertiesSupported() -> Bool { GetPropertiesSupported(nil) }

    open func GetPropertiesSupported(
        _ context: (any CNATypeDescriptorContext)?
    ) -> Bool { false }

    public final func GetStandardValues() -> [Any?] { GetStandardValues(nil)?.allValues ?? [] }

    open func GetStandardValues(
        _ context: (any CNATypeDescriptorContext)?
    ) -> CNATypeConverterStandardValuesCollection? { nil }

    public final func GetStandardValuesExclusive() -> Bool {
        GetStandardValuesExclusive(nil)
    }

    open func GetStandardValuesExclusive(
        _ context: (any CNATypeDescriptorContext)?
    ) -> Bool { false }

    public final func GetStandardValuesSupported() -> Bool {
        GetStandardValuesSupported(nil)
    }

    open func GetStandardValuesSupported(
        _ context: (any CNATypeDescriptorContext)?
    ) -> Bool { false }

    open func IsValid(
        _ context: (any CNATypeDescriptorContext)?, value: Any?
    ) -> Bool { true }

    public final func IsValid(_ value: Any?) -> Bool { IsValid(nil, value: value) }

    public final func SortProperties(
        _ props: CNAPropertyDescriptorCollection, names: [String]
    ) -> CNAPropertyDescriptorCollection { props.Sort(names) }
}

private extension CNATypeConverterStandardValuesCollection {
    var allValues: [Any?] {
        (0..<Int(Count)).map { Item(Int32($0)) }
    }
}

open class CNAExpandableObjectConverter: CNATypeConverter {
    public override init() { super.init() }

    open override func GetProperties(
        _ context: (any CNATypeDescriptorContext)?,
        value: Any?,
        attributes: [CNAAttribute]
    ) -> CNAPropertyDescriptorCollection? { nil }

    open override func GetPropertiesSupported(
        _ context: (any CNATypeDescriptorContext)?
    ) -> Bool { true }
}
