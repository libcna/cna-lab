import Foundation
import XCTest
@testable import CNA

final class Foundation104ContentReaderTests: XCTestCase {
    private typealias Content = Microsoft.Xna.Framework.Content

    func testPublicContentManagerLoadsAProjectAuthoredXnbAndCachesIt() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }

        var payloadCreates = 0
        var sharedCreates = 0
        var payloadInitializes = 0
        var sharedInitializes = 0
        try registerReaders(
            payloadCreates: { payloadCreates += 1 },
            sharedCreates: { sharedCreates += 1 },
            payloadInitializes: { payloadInitializes += 1 },
            sharedInitializes: { sharedInitializes += 1 })
        defer { unregisterReaders() }

        try fixture.write(asset: "nested/fixture", bytes: makePayloadXnb())
        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(),
            rootDirectory: fixture.path)

        let first: ReaderPayload = try manager.Load("nested/fixture")
        let second: ReaderPayload = try manager.Load("NESTED\\FIXTURE")
        XCTAssertTrue(first === second, "the case-insensitive cache owns one identity")
        XCTAssertEqual(first.number, 0x1234_5678)
        XCTAssertEqual(first.text, "foundation-104")
        XCTAssertEqual(first.vector.X, 1.25)
        XCTAssertEqual(first.vector.Y, -2.5)
        XCTAssertEqual(first.shared?.name, "one shared instance")
        XCTAssertTrue(first.shared === first.sharedAgain)
        XCTAssertEqual(payloadCreates, 1)
        XCTAssertEqual(sharedCreates, 1)
        XCTAssertEqual(payloadInitializes, 1)
        XCTAssertEqual(sharedInitializes, 1)

        let managerView = Content.ContentTypeReaderManager(contentReader: nil)
        let registered = try managerView.GetTypeReader(ReaderPayload.self)
        let registeredAgain = try managerView.GetTypeReader(ReaderPayload.self)
        XCTAssertEqual(
            ObjectIdentifier(registered.TargetType),
            ObjectIdentifier(ReaderPayload.self))
        XCTAssertTrue(registered === registeredAgain)
        XCTAssertThrowsError(try managerView.GetTypeReader(String.self)) { error in
            XCTAssertTrue(error is Content.ContentLoadException)
        }

        try manager.Unload()
        XCTAssertTrue(first.disposed)
        XCTAssertTrue(first.shared?.disposed == true)
        let third: ReaderPayload = try manager.Load("nested/fixture")
        XCTAssertFalse(third === first, "Unload clears the asset cache")
        try manager.Dispose()
    }

    func testTypeCreatorRegistrationIsCaseSensitiveAndRejectsDuplicates() throws {
        let exact = "CNA.Tests.Foundation104.RegistryCase"
        try Content.ContentTypeReaderManager.registerTypeCreator(exact) {
            PayloadReader()
        }
        defer { Content.ContentTypeReaderManager.unregisterTypeCreator(exact) }

        XCTAssertThrowsError(
            try Content.ContentTypeReaderManager.registerTypeCreator(exact) {
                PayloadReader()
            }) { error in
                XCTAssertTrue(error is CNAArgumentException)
            }

        try Content.ContentTypeReaderManager.registerTypeCreator(exact.lowercased()) {
            SharedReader()
        }
        Content.ContentTypeReaderManager.unregisterTypeCreator(exact.lowercased())
    }

    func testRawObjectBridgePreservesExistingIdentityAndRejectsWrongType() throws {
        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: "")
        let readerType = PayloadReader()

        let existing = ReaderPayload()
        let existingReader = try rawReader(
            manager: manager, bytes: makeRawPayload(sharedIndex: 0))
        let filled: ReaderPayload? = try existingReader.ReadRawObject(
            readerType, existingInstance: existing)
        XCTAssertTrue(filled === existing)
        XCTAssertEqual(filled?.text, "foundation-104")

        let wrongReader = try rawReader(
            manager: manager, bytes: makeRawPayload(sharedIndex: 0))
        XCTAssertThrowsError(
            try { () -> Void in
                let _: String? = try wrongReader.ReadRawObject(readerType)
            }()) { error in
                XCTAssertTrue(error is Content.ContentLoadException)
            }

        let nullReader = try rawReader(manager: manager, bytes: [0])
        let absent: ReaderPayload? = try nullReader.ReadObject()
        XCTAssertNil(absent, "XNA reader index zero is CLR default(T)")

        let nilFixupReader = try rawReader(manager: manager, bytes: [0])
        XCTAssertThrowsError(
            try nilFixupReader.ReadSharedResource(
                nil as ((ReaderShared) throws -> Void)?)) {
                    XCTAssertEqual(
                        ($0 as? CNAArgumentNullException)?.ParamName, "fixup")
                }
        try manager.Dispose()
    }

    func testSharedResourceIndexIsOneBasedAndFixupRunsAfterThePrimaryObject() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }
        try registerReaders(payloadCreates: {}, sharedCreates: {})
        defer { unregisterReaders() }
        try fixture.write(asset: "fixture", bytes: makePayloadXnb())

        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: fixture.path)
        let value: ReaderPayload = try manager.Load("fixture")
        XCTAssertEqual(value.shared?.name, "one shared instance")

        var bad = XnbWriter()
        bad.appendManifestHeader([
            (Self.payloadReaderName, 7), (Self.sharedReaderName, 3)
        ], sharedCount: 1)
        bad.append7Bit(1)
        bad.append(contentsOf: makeRawPayload(sharedIndex: 2))
        bad.append7Bit(2)
        bad.appendString("unused")
        try fixture.write(asset: "bad-shared-index", bytes: bad.finish())
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("bad-shared-index") }()) {
                XCTAssertTrue($0 is Content.ContentLoadException)
            }
        try manager.Dispose()
    }

    func testExternalReferenceNormalizesRelativeToTheCurrentAsset() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }
        try registerReaders(payloadCreates: {}, sharedCreates: {})
        defer { unregisterReaders() }
        try fixture.write(asset: "nested/fixture", bytes: makePayloadXnb())

        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: fixture.path)
        let expected: ReaderPayload = try manager.Load("nested/fixture")
        var reference = XnbWriter(header: false)
        reference.appendString("../fixture")
        let reader = try rawReader(
            manager: manager, bytes: reference.bytes,
            assetName: "nested/sub/main")
        let actual: ReaderPayload? = try reader.ReadExternalReference()
        XCTAssertTrue(actual === expected)

        var empty = XnbWriter(header: false)
        empty.appendString("")
        let emptyReader = try rawReader(
            manager: manager, bytes: empty.bytes,
            assetName: "nested/sub/main")
        let absent: ReaderPayload? = try emptyReader.ReadExternalReference()
        XCTAssertNil(absent)

        var missing = XnbWriter(header: false)
        missing.appendString("missing")
        let missingReader = try rawReader(
            manager: manager, bytes: missing.bytes,
            assetName: "nested/sub/main")
        XCTAssertThrowsError(
            try { let _: ReaderPayload? = try missingReader.ReadExternalReference() }()) {
                XCTAssertTrue($0 is Content.ContentLoadException)
            }
        try manager.Dispose()
    }

    func testReadAssetCallbackOwnsManagedDisposablesInsteadOfTheManager() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }
        try registerReaders(payloadCreates: {}, sharedCreates: {})
        defer { unregisterReaders() }
        try fixture.write(asset: "fixture", bytes: makePayloadXnb())

        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: fixture.path)
        var recorded: [any CNADisposable] = []
        let value: ReaderPayload = try manager.ReadAsset(
            "fixture", recordDisposableObject: { recorded.append($0) })
        XCTAssertEqual(recorded.count, 2, "primary and shared objects are recorded once")
        try manager.Unload()
        XCTAssertFalse(value.disposed, "the callback, not the manager, owns these objects")
        for item in recorded { try item.Dispose() }
        XCTAssertTrue(value.disposed)
        XCTAssertTrue(value.shared?.disposed == true)
        try manager.Dispose()
    }

    func testOpenStreamUsesRootDirectoryAndReportsMissingAsset() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }
        try fixture.write(asset: "nested/raw", bytes: [1, 2, 3])
        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: fixture.path)

        let stream = try manager.OpenStream("nested/raw")
        let reader = try CNABinaryReader(stream)
        XCTAssertEqual(try reader.ReadBytes(3), [1, 2, 3])
        try reader.Close()

        XCTAssertThrowsError(try manager.OpenStream("missing")) { error in
            let content = error as? Content.ContentLoadException
            XCTAssertEqual(content?.Message, "Error loading \"missing\". File not found.")
            XCTAssertTrue(content?.InnerException is CNAFileNotFoundException)
        }
        XCTAssertThrowsError(try manager.OpenStream("bad\0path")) { error in
            let content = error as? Content.ContentLoadException
            XCTAssertEqual(
                content?.Message, "Error loading \"bad\0path\". Cannot open file.")
            XCTAssertTrue(content?.InnerException is CNAArgumentException)
        }
        try manager.Dispose()
    }

    func testXnbEnvelopeRefusesMagicVersionCompressionAndUnknownReader() throws {
        let fixture = try FixtureRoot()
        defer { fixture.remove() }
        try registerReaders(payloadCreates: {}, sharedCreates: {})
        defer { unregisterReaders() }
        let manager = try Content.ContentManager(
            serviceProvider: EmptyServiceProvider(), rootDirectory: fixture.path)

        try fixture.write(asset: "magic", bytes: [0, 0, 0, 119, 5, 0, 10, 0, 0, 0])
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("magic") }()) {
                XCTAssertEqual(
                    ($0 as? Content.ContentLoadException)?.Message,
                    "Error loading \"magic\". This is not a compiled content file.")
            }

        try fixture.write(
            asset: "version", bytes: [88, 78, 66, 119, 4, 0, 10, 0, 0, 0])
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("version") }()) {
                XCTAssertEqual(
                    ($0 as? Content.ContentLoadException)?.Message,
                    "Error loading \"version\". This file was compiled using the wrong version of the XNA Framework.")
            }

        try fixture.write(
            asset: "size", bytes: [88, 78, 66, 119, 5, 0, 11, 0, 0, 0])
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("size") }()) {
                XCTAssertEqual(
                    ($0 as? Content.ContentLoadException)?.Message,
                    "Error loading \"size\". File has been truncated.")
            }

        try fixture.write(
            asset: "compressed",
            bytes: [88, 78, 66, 119, 5, 128, 14, 0, 0, 0, 0, 0, 0, 0])
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("compressed") }()) {
                XCTAssertEqual(
                    ($0 as? Content.ContentLoadException)?.Message,
                    "Error decompressing content data.")
            }

        var unknown = XnbWriter()
        unknown.appendManifestHeader([("No.Such.Reader", 0)], sharedCount: 0)
        unknown.append7Bit(0)
        try fixture.write(asset: "unknown", bytes: unknown.finish())
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("unknown") }()) {
                XCTAssertEqual(
                    ($0 as? Content.ContentLoadException)?.Message,
                    "Error loading \"unknown\". Cannot find ContentTypeReader No.Such.Reader.")
            }

        var typeVersion = XnbWriter()
        typeVersion.appendManifestHeader(
            [(Self.payloadReaderName, 8)], sharedCount: 0)
        typeVersion.append7Bit(0)
        try fixture.write(asset: "reader-version", bytes: typeVersion.finish())
        XCTAssertThrowsError(
            try { let _: ReaderPayload = try manager.Load("reader-version") }()) {
                let content = $0 as? Content.ContentLoadException
                XCTAssertTrue(content?.Message.contains(
                    "File contains the wrong version of type") == true)
            }
        try manager.Dispose()
    }

    func testResourceManagerLookupCaseTypeAndReleaseSemantics() throws {
        var objectLoads = 0
        let manager = CNAResourceManager(
            baseName: "Fixture.Resources",
            resourceFactories: [
                "Greeting": { "hello" },
                "Blob": {
                    objectLoads += 1
                    return ResourceIdentity(objectLoads)
                },
                "Bytes": { [UInt8]([1, 2, 3]) }
            ])
        XCTAssertEqual(manager.BaseName, "Fixture.Resources")
        XCTAssertEqual(try manager.GetString("Greeting"), "hello")
        XCTAssertNil(try manager.GetObject("greeting"))
        manager.IgnoreCase = true
        XCTAssertEqual(try manager.GetString("greeting"), "hello")
        XCTAssertNil(try manager.GetObject("wrong-key"))
        XCTAssertThrowsError(try manager.GetString("Bytes")) { error in
            XCTAssertEqual(
                (error as? CNAInvalidOperationException)?.Message,
                "Resource was of type 'System.Byte[]' instead of String - call GetObject instead.")
        }
        XCTAssertThrowsError(try manager.GetObject(nil)) { error in
            XCTAssertEqual((error as? CNAArgumentNullException)?.ParamName, "name")
        }

        let first = try XCTUnwrap(try manager.GetObject("Blob") as? ResourceIdentity)
        let cached = try XCTUnwrap(try manager.GetObject("Blob") as? ResourceIdentity)
        XCTAssertTrue(first === cached)
        manager.ReleaseAllResources()
        let reloaded = try XCTUnwrap(try manager.GetObject("Blob") as? ResourceIdentity)
        XCTAssertFalse(first === reloaded)
        XCTAssertEqual(objectLoads, 2)
        XCTAssertEqual(
            ObjectIdentifier(manager.ResourceSetType),
            ObjectIdentifier(manager.ResourceSetType))
    }

    func testResourceContentManagerUsesVirtualResourceManagerLookupAndSameXnbPath() throws {
        try registerReaders(payloadCreates: {}, sharedCreates: {})
        defer { unregisterReaders() }
        let resources = TrackingResourceManager([
            "fixture": makePayloadXnb(),
            "wrong": "not bytes"
        ])
        let manager = try Content.ResourceContentManager(
            serviceProvider: EmptyServiceProvider(), resourceManager: resources)

        let first: ReaderPayload = try manager.Load("fixture")
        let second: ReaderPayload = try manager.Load("fixture")
        XCTAssertTrue(first === second)
        XCTAssertEqual(resources.lookups, ["fixture"], "ContentManager cache is shared")
        XCTAssertEqual(first.shared?.name, "one shared instance")

        XCTAssertThrowsError(try manager.OpenStream("missing")) { error in
            XCTAssertEqual(
                (error as? Content.ContentLoadException)?.Message,
                "Error loading \"missing\". Resource not found.")
        }
        XCTAssertThrowsError(try manager.OpenStream("wrong")) { error in
            XCTAssertEqual(
                (error as? Content.ContentLoadException)?.Message,
                "Error loading \"wrong\". Not a binary resource.")
        }
        try manager.Dispose()

        XCTAssertThrowsError(
            try Content.ResourceContentManager(
                serviceProvider: EmptyServiceProvider(), resourceManager: nil)) {
                    XCTAssertEqual(
                        ($0 as? CNAArgumentNullException)?.ParamName,
                        "resourceManager")
                }
    }

    private func rawReader(
        manager: Content.ContentManager,
        bytes: [UInt8],
        assetName: String = "raw"
    ) throws -> Content.ContentReader {
        try Content.ContentReader(
            contentManager: manager,
            input: InputStream(data: Data(bytes)),
            assetName: assetName,
            recordDisposableObject: nil,
            graphicsProfile: 0)
    }

    private func registerReaders(
        payloadCreates: @escaping () -> Void,
        sharedCreates: @escaping () -> Void,
        payloadInitializes: @escaping () -> Void = {},
        sharedInitializes: @escaping () -> Void = {}
    ) throws {
        try Content.ContentTypeReaderManager.registerTypeCreator(
            Self.payloadReaderName) {
                payloadCreates()
                return PayloadReader(onInitialize: payloadInitializes)
            }
        try Content.ContentTypeReaderManager.registerTypeCreator(
            Self.sharedReaderName) {
                sharedCreates()
                return SharedReader(onInitialize: sharedInitializes)
            }
    }

    private func unregisterReaders() {
        Content.ContentTypeReaderManager.unregisterTypeCreator(Self.payloadReaderName)
        Content.ContentTypeReaderManager.unregisterTypeCreator(Self.sharedReaderName)
    }

    private func makePayloadXnb() -> [UInt8] {
        var writer = XnbWriter()
        writer.appendManifestHeader([
            (Self.payloadReaderName, 7), (Self.sharedReaderName, 3)
        ], sharedCount: 1)
        writer.append7Bit(1)
        writer.append(contentsOf: makeRawPayload(sharedIndex: 1))
        writer.append7Bit(2)
        writer.appendString("one shared instance")
        return writer.finish()
    }

    private func makeRawPayload(sharedIndex: Int32) -> [UInt8] {
        var writer = XnbWriter(header: false)
        writer.appendInt32(0x1234_5678)
        writer.appendString("foundation-104")
        writer.appendSingle(1.25)
        writer.appendSingle(-2.5)
        writer.append7Bit(sharedIndex)
        writer.append7Bit(sharedIndex)
        return writer.bytes
    }

    private static let payloadReaderName =
        "CNA.Tests.Foundation104.PayloadReader"
    private static let sharedReaderName =
        "CNA.Tests.Foundation104.SharedReader"
}

private final class EmptyServiceProvider: CNAServiceProvider {
    func GetService(_ serviceType: Any.Type) -> Any? { nil }
}

private final class ReaderPayload: CNADisposable {
    var number: Int32 = 0
    var text = ""
    var vector = Microsoft.Xna.Framework.Vector2.Zero
    var shared: ReaderShared?
    var sharedAgain: ReaderShared?
    var disposed = false
    func Dispose() throws { disposed = true }
}

private final class ReaderShared: CNADisposable {
    var name = ""
    var disposed = false
    func Dispose() throws { disposed = true }
}

private final class PayloadReader:
    Microsoft.Xna.Framework.Content.ContentTypeReaderOfT<ReaderPayload> {
    private let onInitialize: () -> Void

    init(onInitialize: @escaping () -> Void = {}) {
        self.onInitialize = onInitialize
        super.init()
    }

    override var TypeVersion: Int32 { 7 }
    override var CanDeserializeIntoExistingObject: Bool { true }

    override func Initialize(
        _ manager: Microsoft.Xna.Framework.Content.ContentTypeReaderManager
    ) throws {
        onInitialize()
    }

    override func Read(
        _ input: Microsoft.Xna.Framework.Content.ContentReader,
        existingInstance: ReaderPayload?
    ) throws -> ReaderPayload {
        let result = existingInstance ?? ReaderPayload()
        result.number = try input.ReadInt32()
        result.text = try input.ReadString()
        result.vector = try input.ReadVector2()
        try input.ReadSharedResource { (shared: ReaderShared) in
            result.shared = shared
        }
        try input.ReadSharedResource { (shared: ReaderShared) in
            result.sharedAgain = shared
        }
        return result
    }
}

private final class SharedReader:
    Microsoft.Xna.Framework.Content.ContentTypeReaderOfT<ReaderShared> {
    private let onInitialize: () -> Void

    init(onInitialize: @escaping () -> Void = {}) {
        self.onInitialize = onInitialize
        super.init()
    }

    override var TypeVersion: Int32 { 3 }

    override func Initialize(
        _ manager: Microsoft.Xna.Framework.Content.ContentTypeReaderManager
    ) throws {
        onInitialize()
    }

    override func Read(
        _ input: Microsoft.Xna.Framework.Content.ContentReader,
        existingInstance: ReaderShared?
    ) throws -> ReaderShared {
        let result = existingInstance ?? ReaderShared()
        result.name = try input.ReadString()
        return result
    }
}

private final class ResourceIdentity {
    let number: Int
    init(_ number: Int) { self.number = number }
}

private final class TrackingResourceManager: CNAResourceManager {
    private let resources: [String: Any]
    var lookups: [String] = []

    init(_ resources: [String: Any]) {
        self.resources = resources
        super.init()
    }

    override func GetObject(_ name: String?) throws -> Any? {
        guard let name else { throw CNAArgumentNullException(paramName: "name") }
        lookups.append(name)
        return resources[name]
    }
}

private struct FixtureRoot {
    let url: URL
    var path: String { url.path }

    init() throws {
        url = FileManager.default.temporaryDirectory.appendingPathComponent(
            "cna-swift-foundation-104-\(UUID().uuidString)", isDirectory: true)
        try FileManager.default.createDirectory(
            at: url, withIntermediateDirectories: false)
    }

    func write(asset: String, bytes: [UInt8]) throws {
        let file = url.appendingPathComponent(asset + ".xnb")
        try FileManager.default.createDirectory(
            at: file.deletingLastPathComponent(),
            withIntermediateDirectories: true)
        try Data(bytes).write(to: file)
    }

    func remove() {
        // `url` is the exact unique child created by this value's initializer.
        try? FileManager.default.removeItem(at: url)
    }
}

private struct XnbWriter {
    var bytes: [UInt8]

    init(header: Bool = true) {
        bytes = header ? [88, 78, 66, 119, 5, 0, 0, 0, 0, 0] : []
    }

    mutating func appendManifestHeader(
        _ readers: [(String, Int32)], sharedCount: Int32
    ) {
        append7Bit(Int32(readers.count))
        for reader in readers {
            appendString(reader.0)
            appendInt32(reader.1)
        }
        append7Bit(sharedCount)
    }

    mutating func append7Bit(_ value: Int32) {
        var remaining = UInt32(bitPattern: value)
        repeat {
            var byte = UInt8(remaining & 0x7f)
            remaining >>= 7
            if remaining != 0 { byte |= 0x80 }
            bytes.append(byte)
        } while remaining != 0
    }

    mutating func appendInt32(_ value: Int32) {
        appendUInt32(UInt32(bitPattern: value))
    }

    mutating func appendUInt32(_ value: UInt32) {
        for shift in stride(from: 0, through: 24, by: 8) {
            bytes.append(UInt8(truncatingIfNeeded: value >> UInt32(shift)))
        }
    }

    mutating func appendSingle(_ value: Float) {
        appendUInt32(value.bitPattern)
    }

    mutating func appendString(_ value: String) {
        let encoded = Array(value.utf8)
        append7Bit(Int32(encoded.count))
        bytes.append(contentsOf: encoded)
    }

    mutating func append(contentsOf values: [UInt8]) {
        bytes.append(contentsOf: values)
    }

    mutating func finish() -> [UInt8] {
        let size = UInt32(bytes.count)
        bytes[6] = UInt8(truncatingIfNeeded: size)
        bytes[7] = UInt8(truncatingIfNeeded: size >> 8)
        bytes[8] = UInt8(truncatingIfNeeded: size >> 16)
        bytes[9] = UInt8(truncatingIfNeeded: size >> 24)
        return bytes
    }
}
