// SPDX-License-Identifier: MIT

import Foundation

private let cnaDesignInvalidStringFormat =
    "Invalid string format. Expected a string in the format \"{0}\"."
private let cnaDesignNullNotAllowed =
    "This method does not accept null for this parameter."

private func cnaDesignSameType(_ lhs: Any.Type?, _ rhs: Any.Type) -> Bool {
    guard let lhs else { return false }
    return lhs == rhs
}

private func cnaDesignCulture(_ culture: CNACultureInfo?) -> CNACultureInfo {
    culture ?? .CurrentCulture
}

private func cnaDesignFormat(_ value: Float, culture: CNACultureInfo) -> String {
    let invariant = xnaFloatString(value)
    guard culture.decimalSeparator != "." else { return invariant }
    return invariant.replacingOccurrences(of: ".", with: culture.decimalSeparator)
}

private func cnaDesignFormat(_ values: [String], culture: CNACultureInfo) -> String {
    values.joined(separator: culture.TextInfo.ListSeparator + " ")
}

private func cnaDesignInvalidFormat(
    culture: CNACultureInfo,
    names: [String],
    inner: CNAException? = nil
) -> CNAArgumentException {
    let expected = names.joined(separator: culture.TextInfo.ListSeparator)
    let message = cnaDesignInvalidStringFormat.replacingOccurrences(of: "{0}", with: expected)
    return CNAArgumentException(message: message, innerException: inner)
}

private func cnaDesignParts(
    _ value: Any?,
    culture: CNACultureInfo?,
    names: [String]
) throws -> ([String], CNACultureInfo)? {
    guard let text = value as? String else { return nil }
    let effective = cnaDesignCulture(culture)
    let parts = text.trimmingCharacters(in: .whitespacesAndNewlines)
        .components(separatedBy: effective.TextInfo.ListSeparator)
    guard parts.count == names.count else {
        throw cnaDesignInvalidFormat(culture: effective, names: names)
    }
    return (parts, effective)
}

private func cnaDesignInt32Values(
    _ value: Any?, culture: CNACultureInfo?, names: [String]
) throws -> [Int32]? {
    guard let (parts, effective) = try cnaDesignParts(value, culture: culture, names: names)
    else { return nil }
    do {
        return try parts.map {
            guard let result = Int32($0.trimmingCharacters(in: .whitespacesAndNewlines)) else {
                throw CNAFormatException()
            }
            return result
        }
    } catch let error as CNAException {
        throw cnaDesignInvalidFormat(culture: effective, names: names, inner: error)
    }
}

private func cnaDesignFloatValues(
    _ value: Any?, culture: CNACultureInfo?, names: [String]
) throws -> [Float]? {
    guard let (parts, effective) = try cnaDesignParts(value, culture: culture, names: names)
    else { return nil }
    do {
        return try parts.map {
            var text = $0.trimmingCharacters(in: .whitespacesAndNewlines)
            if effective.decimalSeparator != "." {
                text = text.replacingOccurrences(of: effective.decimalSeparator, with: ".")
            }
            guard let result = Float(text) else { throw CNAFormatException() }
            return result
        }
    } catch let error as CNAException {
        throw cnaDesignInvalidFormat(culture: effective, names: names, inner: error)
    }
}

private func cnaDesignByteValues(
    _ value: Any?, culture: CNACultureInfo?, names: [String]
) throws -> [UInt8]? {
    guard let integers = try cnaDesignInt32Values(value, culture: culture, names: names)
    else { return nil }
    let effective = cnaDesignCulture(culture)
    guard integers.allSatisfy({ 0...255 ~= $0 }) else {
        throw cnaDesignInvalidFormat(culture: effective, names: names)
    }
    return integers.map(UInt8.init)
}

private func cnaDesignProperties(
    componentType: Any.Type,
    entries: [(String, Any.Type, (Any?) throws -> Any?)]
) -> CNAPropertyDescriptorCollection {
    CNAPropertyDescriptorCollection(entries.map { name, propertyType, getter in
        CNAPropertyDescriptor(
            name: name,
            componentType: componentType,
            propertyType: propertyType,
            get: getter)
    }, readOnly: true)
}

private func cnaDesignConstructor(
    type: Any.Type,
    names: [String],
    types: [Any.Type],
    invoke: @escaping ([Any?]) throws -> Any?
) -> CNAConstructorInfo {
    CNAConstructorInfo(
        declaringType: type,
        parameters: zip(names, types).enumerated().map { index, pair in
            CNAParameterInfo(name: pair.0, parameterType: pair.1, position: Int32(index))
        },
        invoke: invoke)
}

private func cnaDesignDescriptor(
    constructor: CNAConstructorInfo,
    arguments: [Any?]
) throws -> CNAInstanceDescriptor {
    try CNAInstanceDescriptor(constructor, arguments: arguments, isComplete: true)
}

private func cnaDesignRequireValues(
    _ values: (any CNAIDictionary)?
) throws -> any CNAIDictionary {
    guard let values else {
        throw CNAArgumentNullException(
            paramName: "propertyValues", message: cnaDesignNullNotAllowed)
    }
    return values
}

private func cnaDesignValue<T>(
    _ values: any CNAIDictionary, _ key: String, as type: T.Type
) throws -> T {
    guard let value = values.Item(key) as? T else {
        throw CNAArgumentException(message: "Property '\(key)' has the wrong value type.")
    }
    return value
}

extension Microsoft.Xna.Framework.Design {
    open class MathTypeConverter: CNAExpandableObjectConverter {
        public var propertyDescriptions: CNAPropertyDescriptorCollection?
        public var supportStringConvert: Bool

        public override init() {
            propertyDescriptions = nil
            supportStringConvert = true
            super.init()
        }

        open override func CanConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, sourceType: Any.Type?
        ) -> Bool {
            supportStringConvert && cnaDesignSameType(sourceType, String.self)
                || super.CanConvertFrom(context, sourceType: sourceType)
        }

        open override func CanConvertTo(
            _ context: (any CNATypeDescriptorContext)?, destinationType: Any.Type?
        ) -> Bool {
            cnaDesignSameType(destinationType, CNAInstanceDescriptor.self)
                || super.CanConvertTo(context, destinationType: destinationType)
        }

        open override func GetCreateInstanceSupported(
            _ context: (any CNATypeDescriptorContext)?
        ) -> Bool { true }

        open override func GetPropertiesSupported(
            _ context: (any CNATypeDescriptorContext)?
        ) -> Bool { true }

        open override func GetProperties(
            _ context: (any CNATypeDescriptorContext)?,
            value: Any?,
            attributes: [CNAAttribute]
        ) -> CNAPropertyDescriptorCollection? { propertyDescriptions }
    }

    open class PointConverter: MathTypeConverter {
        private static let names = ["X", "Y"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Point.self,
            names: ["x", "y"], types: [Int32.self, Int32.self]
        ) { values in
            guard values.count == 2,
                  let x = values[0] as? Int32, let y = values[1] as? Int32 else {
                throw CNAArgumentException(message: "Constructor arguments do not match Point.")
            }
            return Microsoft.Xna.Framework.Point(x, y)
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaDesignProperties(
                componentType: Microsoft.Xna.Framework.Point.self,
                entries: [
                    ("X", Int32.self, { ($0 as? Microsoft.Xna.Framework.Point)?.X }),
                    ("Y", Int32.self, { ($0 as? Microsoft.Xna.Framework.Point)?.Y }),
                ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let parts = try cnaDesignInt32Values(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Point(parts[0], parts[1])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?,
            value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self),
               let point = value as? Microsoft.Xna.Framework.Point {
                return cnaDesignFormat([String(point.X), String(point.Y)], culture: cnaDesignCulture(culture))
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self),
               let point = value as? Microsoft.Xna.Framework.Point {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [point.X, point.Y])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let bag = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Point(
                try cnaDesignValue(bag, "X", as: Int32.self),
                try cnaDesignValue(bag, "Y", as: Int32.self))
        }
    }

    open class RectangleConverter: MathTypeConverter {
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Rectangle.self,
            names: ["x", "y", "width", "height"],
            types: [Int32.self, Int32.self, Int32.self, Int32.self]
        ) { values in
            guard values.count == 4,
                  let x = values[0] as? Int32, let y = values[1] as? Int32,
                  let width = values[2] as? Int32, let height = values[3] as? Int32 else {
                throw CNAArgumentException(message: "Constructor arguments do not match Rectangle.")
            }
            return Microsoft.Xna.Framework.Rectangle(x, y, width, height)
        }

        public override init() {
            super.init()
            supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(
                componentType: Microsoft.Xna.Framework.Rectangle.self,
                entries: [
                    ("X", Int32.self, { ($0 as? Microsoft.Xna.Framework.Rectangle)?.X }),
                    ("Y", Int32.self, { ($0 as? Microsoft.Xna.Framework.Rectangle)?.Y }),
                    ("Width", Int32.self, { ($0 as? Microsoft.Xna.Framework.Rectangle)?.Width }),
                    ("Height", Int32.self, { ($0 as? Microsoft.Xna.Framework.Rectangle)?.Height }),
                ])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?,
            value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self),
               let rectangle = value as? Microsoft.Xna.Framework.Rectangle {
                return try cnaDesignDescriptor(
                    constructor: Self.constructor,
                    arguments: [rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let bag = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Rectangle(
                try cnaDesignValue(bag, "X", as: Int32.self),
                try cnaDesignValue(bag, "Y", as: Int32.self),
                try cnaDesignValue(bag, "Width", as: Int32.self),
                try cnaDesignValue(bag, "Height", as: Int32.self))
        }
    }
}

extension Microsoft.Xna.Framework.Design {
    open class Vector2Converter: MathTypeConverter {
        private static let names = ["X", "Y"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Vector2.self,
            names: ["x", "y"], types: [Float.self, Float.self]
        ) { values in
            guard values.count == 2,
                  let x = values[0] as? Float, let y = values[1] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match Vector2.")
            }
            return Microsoft.Xna.Framework.Vector2(x, y)
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaDesignProperties(
                componentType: Microsoft.Xna.Framework.Vector2.self,
                entries: [
                    ("X", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector2)?.X }),
                    ("Y", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector2)?.Y }),
                ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let p = try cnaDesignFloatValues(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Vector2(p[0], p[1])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?,
            value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self),
               let v = value as? Microsoft.Xna.Framework.Vector2 {
                let c = cnaDesignCulture(culture)
                return cnaDesignFormat([cnaDesignFormat(v.X, culture: c), cnaDesignFormat(v.Y, culture: c)], culture: c)
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self),
               let v = value as? Microsoft.Xna.Framework.Vector2 {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.X, v.Y])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let bag = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Vector2(
                try cnaDesignValue(bag, "X", as: Float.self),
                try cnaDesignValue(bag, "Y", as: Float.self))
        }
    }

    open class Vector3Converter: MathTypeConverter {
        private static let names = ["X", "Y", "Z"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Vector3.self,
            names: ["x", "y", "z"], types: [Float.self, Float.self, Float.self]
        ) { values in
            guard values.count == 3,
                  let x = values[0] as? Float, let y = values[1] as? Float,
                  let z = values[2] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match Vector3.")
            }
            return Microsoft.Xna.Framework.Vector3(x, y, z)
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.Vector3.self, entries: [
                ("X", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.X }),
                ("Y", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Y }),
                ("Z", Float.self, { ($0 as? Microsoft.Xna.Framework.Vector3)?.Z }),
            ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let p = try cnaDesignFloatValues(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Vector3(p[0], p[1], p[2])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?,
            value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self), let v = value as? Microsoft.Xna.Framework.Vector3 {
                let c = cnaDesignCulture(culture)
                return cnaDesignFormat([cnaDesignFormat(v.X, culture: c), cnaDesignFormat(v.Y, culture: c), cnaDesignFormat(v.Z, culture: c)], culture: c)
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Vector3 {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.X, v.Y, v.Z])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let bag = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Vector3(
                try cnaDesignValue(bag, "X", as: Float.self),
                try cnaDesignValue(bag, "Y", as: Float.self),
                try cnaDesignValue(bag, "Z", as: Float.self))
        }
    }
}

private func cnaFourFloatProperties<T>(
    _ type: T.Type,
    get: @escaping (T) -> (Float, Float, Float, Float)
) -> CNAPropertyDescriptorCollection {
    let names = ["X", "Y", "Z", "W"]
    return cnaDesignProperties(componentType: type, entries: names.enumerated().map { index, name in
        (name, Float.self, { value in
            guard let typed = value as? T else { return nil }
            let parts = get(typed)
            return [parts.0, parts.1, parts.2, parts.3][index]
        })
    })
}

extension Microsoft.Xna.Framework.Design {
    open class Vector4Converter: MathTypeConverter {
        private static let names = ["X", "Y", "Z", "W"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Vector4.self,
            names: ["x", "y", "z", "w"], types: [Float.self, Float.self, Float.self, Float.self]
        ) { v in
            guard v.count == 4, let x = v[0] as? Float, let y = v[1] as? Float,
                  let z = v[2] as? Float, let w = v[3] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match Vector4.")
            }
            return Microsoft.Xna.Framework.Vector4(x, y, z, w)
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaFourFloatProperties(Microsoft.Xna.Framework.Vector4.self) { ($0.X, $0.Y, $0.Z, $0.W) }
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let p = try cnaDesignFloatValues(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Vector4(p[0], p[1], p[2], p[3])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self), let v = value as? Microsoft.Xna.Framework.Vector4 {
                let c = cnaDesignCulture(culture)
                return cnaDesignFormat([v.X, v.Y, v.Z, v.W].map { cnaDesignFormat($0, culture: c) }, culture: c)
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Vector4 {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.X, v.Y, v.Z, v.W])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Vector4(
                try cnaDesignValue(b, "X", as: Float.self), try cnaDesignValue(b, "Y", as: Float.self),
                try cnaDesignValue(b, "Z", as: Float.self), try cnaDesignValue(b, "W", as: Float.self))
        }
    }

    open class QuaternionConverter: MathTypeConverter {
        private static let names = ["X", "Y", "Z", "W"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Quaternion.self,
            names: ["x", "y", "z", "w"], types: [Float.self, Float.self, Float.self, Float.self]
        ) { v in
            guard v.count == 4, let x = v[0] as? Float, let y = v[1] as? Float,
                  let z = v[2] as? Float, let w = v[3] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match Quaternion.")
            }
            return Microsoft.Xna.Framework.Quaternion(x, y, z, w)
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaFourFloatProperties(Microsoft.Xna.Framework.Quaternion.self) { ($0.X, $0.Y, $0.Z, $0.W) }
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let p = try cnaDesignFloatValues(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Quaternion(p[0], p[1], p[2], p[3])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self), let v = value as? Microsoft.Xna.Framework.Quaternion {
                let c = cnaDesignCulture(culture)
                return cnaDesignFormat([v.X, v.Y, v.Z, v.W].map { cnaDesignFormat($0, culture: c) }, culture: c)
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Quaternion {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.X, v.Y, v.Z, v.W])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Quaternion(
                try cnaDesignValue(b, "X", as: Float.self), try cnaDesignValue(b, "Y", as: Float.self),
                try cnaDesignValue(b, "Z", as: Float.self), try cnaDesignValue(b, "W", as: Float.self))
        }
    }
}

extension Microsoft.Xna.Framework.Design {
    open class ColorConverter: MathTypeConverter {
        private static let names = ["R", "G", "B", "A"]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Color.self,
            names: ["r", "g", "b", "a"], types: [UInt8.self, UInt8.self, UInt8.self, UInt8.self]
        ) { v in
            guard v.count == 4, let r = v[0] as? UInt8, let g = v[1] as? UInt8,
                  let b = v[2] as? UInt8, let a = v[3] as? UInt8 else {
                throw CNAArgumentException(message: "Constructor arguments do not match Color.")
            }
            return Microsoft.Xna.Framework.Color(Int32(r), Int32(g), Int32(b), Int32(a))
        }

        public override init() {
            super.init()
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.Color.self, entries: [
                ("R", UInt8.self, { ($0 as? Microsoft.Xna.Framework.Color)?.R }),
                ("G", UInt8.self, { ($0 as? Microsoft.Xna.Framework.Color)?.G }),
                ("B", UInt8.self, { ($0 as? Microsoft.Xna.Framework.Color)?.B }),
                ("A", UInt8.self, { ($0 as? Microsoft.Xna.Framework.Color)?.A }),
            ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? {
            guard let p = try cnaDesignByteValues(value, culture: culture, names: Self.names)
            else { return try super.ConvertFrom(context, culture: culture, value: value) }
            return Microsoft.Xna.Framework.Color(Int32(p[0]), Int32(p[1]), Int32(p[2]), Int32(p[3]))
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, String.self), let v = value as? Microsoft.Xna.Framework.Color {
                return cnaDesignFormat([String(v.R), String(v.G), String(v.B), String(v.A)], culture: cnaDesignCulture(culture))
            }
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Color {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.R, v.G, v.B, v.A])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Color(
                Int32(try cnaDesignValue(b, "R", as: UInt8.self)),
                Int32(try cnaDesignValue(b, "G", as: UInt8.self)),
                Int32(try cnaDesignValue(b, "B", as: UInt8.self)),
                Int32(try cnaDesignValue(b, "A", as: UInt8.self)))
        }
    }
}

extension Microsoft.Xna.Framework.Design {
    open class BoundingBoxConverter: MathTypeConverter {
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.BoundingBox.self,
            names: ["min", "max"], types: [Microsoft.Xna.Framework.Vector3.self, Microsoft.Xna.Framework.Vector3.self]
        ) { v in
            guard v.count == 2, let min = v[0] as? Microsoft.Xna.Framework.Vector3,
                  let max = v[1] as? Microsoft.Xna.Framework.Vector3 else {
                throw CNAArgumentException(message: "Constructor arguments do not match BoundingBox.")
            }
            return Microsoft.Xna.Framework.BoundingBox(min, max)
        }

        public override init() {
            super.init(); supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.BoundingBox.self, entries: [
                ("Min", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.BoundingBox)?.Min }),
                ("Max", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.BoundingBox)?.Max }),
            ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? { try super.ConvertFrom(context, culture: culture, value: value) }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.BoundingBox {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.Min, v.Max])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.BoundingBox(
                try cnaDesignValue(b, "Min", as: Microsoft.Xna.Framework.Vector3.self),
                try cnaDesignValue(b, "Max", as: Microsoft.Xna.Framework.Vector3.self))
        }
    }

    open class BoundingSphereConverter: MathTypeConverter {
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.BoundingSphere.self,
            names: ["center", "radius"], types: [Microsoft.Xna.Framework.Vector3.self, Float.self]
        ) { v in
            guard v.count == 2, let center = v[0] as? Microsoft.Xna.Framework.Vector3,
                  let radius = v[1] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match BoundingSphere.")
            }
            return try Microsoft.Xna.Framework.BoundingSphere(center, radius)
        }

        public override init() {
            super.init(); supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.BoundingSphere.self, entries: [
                ("Center", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.BoundingSphere)?.Center }),
                ("Radius", Float.self, { ($0 as? Microsoft.Xna.Framework.BoundingSphere)?.Radius }),
            ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? { try super.ConvertFrom(context, culture: culture, value: value) }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.BoundingSphere {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.Center, v.Radius])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return try Microsoft.Xna.Framework.BoundingSphere(
                try cnaDesignValue(b, "Center", as: Microsoft.Xna.Framework.Vector3.self),
                try cnaDesignValue(b, "Radius", as: Float.self))
        }
    }

    open class PlaneConverter: MathTypeConverter {
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Plane.self,
            names: ["normal", "d"], types: [Microsoft.Xna.Framework.Vector3.self, Float.self]
        ) { v in
            guard v.count == 2, let normal = v[0] as? Microsoft.Xna.Framework.Vector3,
                  let d = v[1] as? Float else {
                throw CNAArgumentException(message: "Constructor arguments do not match Plane.")
            }
            return Microsoft.Xna.Framework.Plane(normal, d)
        }

        public override init() {
            super.init(); supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.Plane.self, entries: [
                ("Normal", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.Plane)?.Normal }),
                ("D", Float.self, { ($0 as? Microsoft.Xna.Framework.Plane)?.D }),
            ])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Plane {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.Normal, v.D])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Plane(
                try cnaDesignValue(b, "Normal", as: Microsoft.Xna.Framework.Vector3.self),
                try cnaDesignValue(b, "D", as: Float.self))
        }
    }

    open class RayConverter: MathTypeConverter {
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Ray.self,
            names: ["position", "direction"], types: [Microsoft.Xna.Framework.Vector3.self, Microsoft.Xna.Framework.Vector3.self]
        ) { v in
            guard v.count == 2, let position = v[0] as? Microsoft.Xna.Framework.Vector3,
                  let direction = v[1] as? Microsoft.Xna.Framework.Vector3 else {
                throw CNAArgumentException(message: "Constructor arguments do not match Ray.")
            }
            return Microsoft.Xna.Framework.Ray(position, direction)
        }

        public override init() {
            super.init(); supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.Ray.self, entries: [
                ("Position", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.Ray)?.Position }),
                ("Direction", Microsoft.Xna.Framework.Vector3.self, { ($0 as? Microsoft.Xna.Framework.Ray)?.Direction }),
            ])
        }

        open override func ConvertFrom(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?
        ) throws -> Any? { try super.ConvertFrom(context, culture: culture, value: value) }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let v = value as? Microsoft.Xna.Framework.Ray {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [v.Position, v.Direction])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            return Microsoft.Xna.Framework.Ray(
                try cnaDesignValue(b, "Position", as: Microsoft.Xna.Framework.Vector3.self),
                try cnaDesignValue(b, "Direction", as: Microsoft.Xna.Framework.Vector3.self))
        }
    }
}

extension Microsoft.Xna.Framework.Design {
    open class MatrixConverter: MathTypeConverter {
        private static let names = [
            "M11", "M12", "M13", "M14", "M21", "M22", "M23", "M24",
            "M31", "M32", "M33", "M34", "M41", "M42", "M43", "M44",
        ]
        private static let constructor = cnaDesignConstructor(
            type: Microsoft.Xna.Framework.Matrix.self,
            names: names.map { $0.lowercased() }, types: Array(repeating: Float.self, count: 16)
        ) { v in
            guard v.count == 16 else { throw CNAArgumentException(message: "Constructor arguments do not match Matrix.") }
            let p = try v.map { value -> Float in
                guard let value = value as? Float else { throw CNAArgumentException(message: "Constructor arguments do not match Matrix.") }
                return value
            }
            return Microsoft.Xna.Framework.Matrix(
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15])
        }

        public override init() {
            super.init(); supportStringConvert = false
            propertyDescriptions = cnaDesignProperties(componentType: Microsoft.Xna.Framework.Matrix.self, entries: [
                ("M11", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M11 }),
                ("M12", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M12 }),
                ("M13", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M13 }),
                ("M14", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M14 }),
                ("M21", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M21 }),
                ("M22", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M22 }),
                ("M23", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M23 }),
                ("M24", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M24 }),
                ("M31", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M31 }),
                ("M32", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M32 }),
                ("M33", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M33 }),
                ("M34", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M34 }),
                ("M41", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M41 }),
                ("M42", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M42 }),
                ("M43", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M43 }),
                ("M44", Float.self, { ($0 as? Microsoft.Xna.Framework.Matrix)?.M44 }),
            ])
        }

        open override func ConvertTo(
            _ context: (any CNATypeDescriptorContext)?, culture: CNACultureInfo?, value: Any?, destinationType: Any.Type?
        ) throws -> Any? {
            if cnaDesignSameType(destinationType, CNAInstanceDescriptor.self), let m = value as? Microsoft.Xna.Framework.Matrix {
                return try cnaDesignDescriptor(constructor: Self.constructor, arguments: [
                    m.M11, m.M12, m.M13, m.M14, m.M21, m.M22, m.M23, m.M24,
                    m.M31, m.M32, m.M33, m.M34, m.M41, m.M42, m.M43, m.M44,
                ])
            }
            return try super.ConvertTo(context, culture: culture, value: value, destinationType: destinationType)
        }

        open override func CreateInstance(
            _ context: (any CNATypeDescriptorContext)?, propertyValues: (any CNAIDictionary)?
        ) throws -> Any? {
            let b = try cnaDesignRequireValues(propertyValues)
            let p = try Self.names.map { try cnaDesignValue(b, $0, as: Float.self) }
            return Microsoft.Xna.Framework.Matrix(
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15])
        }
    }
}
