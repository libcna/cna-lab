// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class Foundation105DesignTests: XCTestCase {
    private typealias Framework = Microsoft.Xna.Framework
    private typealias Design = Microsoft.Xna.Framework.Design

    func testStringConvertersRoundTripWithInvariantCulture() throws {
        let point = Design.PointConverter()
        let pointValue = try XCTUnwrap(
            try point.ConvertFrom(nil, culture: .InvariantCulture, value: " 12, -34 ")
                as? Framework.Point)
        XCTAssertEqual(pointValue.X, 12)
        XCTAssertEqual(pointValue.Y, -34)
        XCTAssertEqual(
            try point.ConvertTo(nil, culture: .InvariantCulture, value: pointValue,
                                destinationType: String.self) as? String,
            "12, -34")

        let vector2 = Design.Vector2Converter()
        let v2 = try XCTUnwrap(
            try vector2.ConvertFrom(nil, culture: .InvariantCulture, value: "1.25,-2.5")
                as? Framework.Vector2)
        XCTAssertEqual(v2.X, 1.25)
        XCTAssertEqual(v2.Y, -2.5)
        XCTAssertEqual(try vector2.ConvertToString(v2), "1.25, -2.5")

        let vector3 = Design.Vector3Converter()
        let v3 = try XCTUnwrap(
            try vector3.ConvertFrom(nil, culture: .InvariantCulture, value: "1,2,3")
                as? Framework.Vector3)
        XCTAssertEqual([v3.X, v3.Y, v3.Z], [1, 2, 3])

        let vector4 = Design.Vector4Converter()
        let v4 = try XCTUnwrap(
            try vector4.ConvertFrom(nil, culture: .InvariantCulture, value: "1,2,3,4")
                as? Framework.Vector4)
        XCTAssertEqual(v4.W, 4)

        let quaternion = Design.QuaternionConverter()
        let q = try XCTUnwrap(
            try quaternion.ConvertFrom(nil, culture: .InvariantCulture, value: "0,0,0,1")
                as? Framework.Quaternion)
        XCTAssertEqual(q.W, 1)

        let color = Design.ColorConverter()
        let c = try XCTUnwrap(
            try color.ConvertFrom(nil, culture: .InvariantCulture, value: "1, 2, 3, 4")
                as? Framework.Color)
        XCTAssertEqual([c.R, c.G, c.B, c.A], [1, 2, 3, 4])
        XCTAssertEqual(try color.ConvertToString(c), "1, 2, 3, 4")
    }

    func testCultureControlsBothDecimalAndListSeparators() throws {
        let culture = CNACultureInfo("de-DE", useUserOverride: false)
        let converter = Design.Vector2Converter()
        let value = try XCTUnwrap(
            try converter.ConvertFrom(nil, culture: culture, value: "1,5; -2,25")
                as? Framework.Vector2)
        XCTAssertEqual(value.X, 1.5)
        XCTAssertEqual(value.Y, -2.25)
        XCTAssertEqual(
            try converter.ConvertTo(nil, culture: culture, value: value,
                                    destinationType: String.self) as? String,
            "1,5; -2,25")
    }

    func testInvalidStringsUseThePinnedXnaMessage() throws {
        let converter = Design.Vector3Converter()
        XCTAssertThrowsError(
            try converter.ConvertFrom(nil, culture: .InvariantCulture, value: "1,2")) { error in
                guard let argument = error as? CNAArgumentException else {
                    return XCTFail("expected CNAArgumentException, got \(error)")
                }
                XCTAssertEqual(
                    argument.Message,
                    "Invalid string format. Expected a string in the format \"X,Y,Z\".")
            }
        XCTAssertThrowsError(
            try converter.ConvertFrom(nil, culture: .InvariantCulture, value: "1,nope,3"))
        XCTAssertThrowsError(
            try Design.ColorConverter().ConvertFrom(
                nil, culture: .InvariantCulture, value: "0,0,0,256"))
    }

    func testInstanceDescriptorCarriesConstructorIdentityAndInvokes() throws {
        let converter = Design.QuaternionConverter()
        let original = Framework.Quaternion(1, 2, 3, 4)
        let descriptor = try XCTUnwrap(
            try converter.ConvertTo(nil, culture: nil, value: original,
                                    destinationType: CNAInstanceDescriptor.self)
                as? CNAInstanceDescriptor)
        XCTAssertTrue(descriptor.IsComplete)
        XCTAssertEqual(descriptor.MemberInfo.Name, ".ctor")
        XCTAssertEqual(descriptor.MemberInfo.MemberType, .Constructor)
        XCTAssertTrue(descriptor.MemberInfo.DeclaringType == Framework.Quaternion.self)
        XCTAssertEqual(descriptor.Arguments.count, 4)

        let constructor = try XCTUnwrap(descriptor.MemberInfo as? CNAConstructorInfo)
        let parameters = constructor.GetParameters()
        XCTAssertEqual(parameters.map(\.Name), ["x", "y", "z", "w"])
        XCTAssertEqual(parameters.map(\.Position), [0, 1, 2, 3])
        XCTAssertTrue(parameters.allSatisfy { $0.ParameterType == Float.self })

        let invoked = try XCTUnwrap(try descriptor.Invoke() as? Framework.Quaternion)
        XCTAssertEqual([invoked.X, invoked.Y, invoked.Z, invoked.W], [1, 2, 3, 4])
        let viaBase = try XCTUnwrap(try CNATypeConverter().ConvertFrom(descriptor)
            as? Framework.Quaternion)
        XCTAssertEqual(viaBase.W, 4)
    }

    func testCreateInstanceCoversEveryNonStringConverter() throws {
        let rectangle = try XCTUnwrap(try Design.RectangleConverter().CreateInstance(
            nil, propertyValues: ObjectBag([
                "X": Int32(1), "Y": Int32(2), "Width": Int32(3), "Height": Int32(4),
            ])) as? Framework.Rectangle)
        XCTAssertEqual([rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height], [1, 2, 3, 4])

        var matrixValues: [String: Any] = [:]
        for row in 1...4 {
            for column in 1...4 {
                matrixValues["M\(row)\(column)"] = Float(row * 10 + column)
            }
        }
        let matrix = try XCTUnwrap(try Design.MatrixConverter().CreateInstance(
            nil, propertyValues: ObjectBag(matrixValues)) as? Framework.Matrix)
        XCTAssertEqual(matrix.M11, 11)
        XCTAssertEqual(matrix.M44, 44)

        let min = Framework.Vector3(-1, -2, -3)
        let max = Framework.Vector3(4, 5, 6)
        let box = try XCTUnwrap(try Design.BoundingBoxConverter().CreateInstance(
            nil, propertyValues: ObjectBag(["Min": min, "Max": max])) as? Framework.BoundingBox)
        XCTAssertEqual(box.Min.X, -1)
        XCTAssertEqual(box.Max.Z, 6)

        let sphere = try XCTUnwrap(try Design.BoundingSphereConverter().CreateInstance(
            nil, propertyValues: ObjectBag(["Center": max, "Radius": Float(7)]))
            as? Framework.BoundingSphere)
        XCTAssertEqual(sphere.Center.Y, 5)
        XCTAssertEqual(sphere.Radius, 7)

        let plane = try XCTUnwrap(try Design.PlaneConverter().CreateInstance(
            nil, propertyValues: ObjectBag(["Normal": min, "D": Float(8)])) as? Framework.Plane)
        XCTAssertEqual(plane.Normal.Z, -3)
        XCTAssertEqual(plane.D, 8)

        let ray = try XCTUnwrap(try Design.RayConverter().CreateInstance(
            nil, propertyValues: ObjectBag(["Position": min, "Direction": max])) as? Framework.Ray)
        XCTAssertEqual(ray.Position.Y, -2)
        XCTAssertEqual(ray.Direction.X, 4)
    }

    func testEveryConverterProducesAnInvocableDescriptor() throws {
        let cases: [(Design.MathTypeConverter, Any, Any.Type)] = [
            (Design.PointConverter(), Framework.Point(1, 2), Framework.Point.self),
            (Design.RectangleConverter(), Framework.Rectangle(1, 2, 3, 4), Framework.Rectangle.self),
            (Design.Vector2Converter(), Framework.Vector2(1, 2), Framework.Vector2.self),
            (Design.Vector3Converter(), Framework.Vector3(1, 2, 3), Framework.Vector3.self),
            (Design.Vector4Converter(), Framework.Vector4(1, 2, 3, 4), Framework.Vector4.self),
            (Design.QuaternionConverter(), Framework.Quaternion(1, 2, 3, 4), Framework.Quaternion.self),
            (Design.MatrixConverter(), Framework.Matrix.Identity, Framework.Matrix.self),
            (Design.BoundingBoxConverter(), Framework.BoundingBox(.Zero, .One), Framework.BoundingBox.self),
            (Design.BoundingSphereConverter(), try Framework.BoundingSphere(.Zero, 1), Framework.BoundingSphere.self),
            (Design.PlaneConverter(), Framework.Plane(.Up, 2), Framework.Plane.self),
            (Design.RayConverter(), Framework.Ray(.Zero, .Forward), Framework.Ray.self),
            (Design.ColorConverter(), Framework.Color(
                Int32(1), Int32(2), Int32(3), Int32(4)), Framework.Color.self),
        ]
        for (converter, value, expectedType) in cases {
            let descriptor = try XCTUnwrap(try converter.ConvertTo(
                nil, culture: nil, value: value,
                destinationType: CNAInstanceDescriptor.self) as? CNAInstanceDescriptor)
            let invoked = try XCTUnwrap(try descriptor.Invoke())
            XCTAssertTrue(type(of: invoked) == expectedType)
        }
    }

    func testPropertyDescriptionsAreStableAndReadable() throws {
        let converter = Design.Vector3Converter()
        XCTAssertTrue(converter.GetCreateInstanceSupported(nil))
        XCTAssertTrue(converter.GetPropertiesSupported(nil))
        XCTAssertTrue(converter.CanConvertFrom(nil, sourceType: String.self))
        XCTAssertTrue(converter.CanConvertTo(nil, destinationType: CNAInstanceDescriptor.self))

        let descriptors = try XCTUnwrap(converter.GetProperties(nil, value: nil, attributes: []))
        XCTAssertEqual(descriptors.Count, 3)
        XCTAssertEqual(try descriptors.Item(0).Name, "X")
        XCTAssertEqual(try descriptors.Item(1).Name, "Y")
        XCTAssertEqual(try descriptors.Item(2).Name, "Z")
        let value = Framework.Vector3(9, 8, 7)
        XCTAssertEqual(try descriptors.Item("Y")?.GetValue(value) as? Float, 8)

        let rectangle = Design.RectangleConverter()
        XCTAssertFalse(rectangle.CanConvertFrom(nil, sourceType: String.self))
        XCTAssertThrowsError(try rectangle.CreateInstance(nil, propertyValues: nil)) { error in
            guard let argument = error as? CNAArgumentNullException else {
                return XCTFail("expected CNAArgumentNullException")
            }
            XCTAssertEqual(argument.ParamName, "propertyValues")
            XCTAssertTrue(argument.Message.hasPrefix(
                "This method does not accept null for this parameter."))
        }
    }

    func testNilDestinationTypeIsRefusedBeforeConversion() {
        XCTAssertThrowsError(
            try Design.PointConverter().ConvertTo(
                nil, culture: nil, value: Framework.Point.Zero, destinationType: nil)) { error in
                    XCTAssertEqual((error as? CNAArgumentNullException)?.ParamName, "destinationType")
                }
    }
}

private final class ObjectBag: CNAIDictionary {
    private var storage: [String: Any]

    init(_ values: [String: Any]) { storage = values }

    var IsFixedSize: Bool { false }
    var IsReadOnly: Bool { false }
    var Keys: [Any] { storage.keys.sorted() }
    var Values: [Any?] { storage.keys.sorted().map { storage[$0] } }

    func Item(_ key: Any) -> Any? {
        guard let key = key as? String else { return nil }
        return storage[key]
    }

    func SetItem(_ key: Any, _ value: Any?) throws {
        guard let key = key as? String, let value else {
            throw CNAArgumentException(message: "ObjectBag requires non-null String entries.")
        }
        storage[key] = value
    }

    func Add(_ key: Any, value: Any?) throws {
        guard let key = key as? String, storage[key] == nil, let value else {
            throw CNAArgumentException(message: "ObjectBag duplicate or invalid entry.")
        }
        storage[key] = value
    }

    func Clear() { storage.removeAll() }
    func Contains(_ key: Any) -> Bool { (key as? String).map { storage[$0] != nil } ?? false }
    func Remove(_ key: Any) { if let key = key as? String { storage.removeValue(forKey: key) } }
}
