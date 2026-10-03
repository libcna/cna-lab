import XCTest
import CNAShim
@testable import CNA

/// `Microsoft.Xna.Framework.Graphics.GraphicsAdapter`.
///
/// A snapshot type whose list is filled at the first moment a device exists,
/// which is this binding's stand-in for XNA's class constructor.
final class Foundation77GraphicsAdapterTests: XCTestCase {
    private typealias A = Microsoft.Xna.Framework.Graphics.GraphicsAdapter

    /// The three shim structures must be exactly the sizes their CNA originals
    /// are, because each is passed with a `struct_size` the runtime validates.
    /// Two of them were wrong on the first attempt and the runtime said so
    /// rather than reading past the end.
    func testTheShimStructuresMatchTheirNativeSizes() {
        XCTAssertEqual(MemoryLayout<CNASwift_DisplayMode>.size, 24)
        XCTAssertEqual(MemoryLayout<CNASwift_GraphicsAdapterInfo>.size, 48)
        XCTAssertEqual(MemoryLayout<CNASwift_GraphicsFormatSelection>.size, 24)
    }

    /// Both device-preference flags are stored values with infallible
    /// accessors, so they read and write with no runtime at all — exactly as
    /// XNA's static backing fields do.
    func testTheDevicePreferenceFlagsNeedNoRuntime() {
        let nullBefore = A.UseNullDevice
        let referenceBefore = A.UseReferenceDevice
        defer {
            A.UseNullDevice = nullBefore
            A.UseReferenceDevice = referenceBefore
        }
        A.UseNullDevice = true
        A.UseReferenceDevice = true
        XCTAssertTrue(A.UseNullDevice)
        XCTAssertTrue(A.UseReferenceDevice)
        A.UseNullDevice = false
        XCTAssertFalse(A.UseNullDevice)
        XCTAssertTrue(A.UseReferenceDevice, "the two flags are independent")
    }

    /// The list is filled by the first `GraphicsDevice.borrow`, and everything
    /// this host reports about the adapter is read back through the snapshot.
    func testTheListIsFilledWhenADeviceFirstExists() throws {
        let game = try AdapterProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }

        if A.Adapters == nil, let why = A.lastEnumerationFailure {
            XCTFail("the adapter list was not filled: \(why)")
        }
        let adapters = try XCTUnwrap(A.Adapters, "the list is filled by then")
        XCTAssertEqual(adapters.Count, 1)
        let adapter = A.DefaultAdapter

        XCTAssertTrue(adapter.IsDefaultAdapter)
        XCTAssertNotEqual(adapter.VendorId, 0, "the host reports a real vendor")
        XCTAssertNotEqual(adapter.DeviceId, 0)
        XCTAssertEqual(adapter.Revision, 0, "CNA documents revision as always zero")
        XCTAssertEqual(adapter.SubSystemId, 0, "and the subsystem id likewise")
        XCTAssertFalse(adapter.DeviceName.isEmpty)
        XCTAssertFalse(adapter.Description.isEmpty)
        XCTAssertEqual(adapter.MonitorHandle, 0,
                       "no monitor handle is bound on a headless renderer")
        XCTAssertNotNil(adapter.CurrentDisplayMode)
        XCTAssertNotNil(adapter.SupportedDisplayModes)

        // The list is filled ONCE per runtime generation, so every device
        // facade the same game hands out sees the same adapter objects. XNA's
        // list is built by a class constructor and never rebuilt, so an
        // adapter a caller holds stays the one the list holds -- a projection
        // that re-enumerated per facade would silently break that identity.
        XCTAssertEqual(game.borrowCount, 2, "the probe borrowed twice")
        XCTAssertTrue(game.firstAdapter === game.secondAdapter,
                      "a second borrow must not rebuild the list")
    }

    /// Both profiles read supported here. Recorded as a native observation:
    /// nothing in this projection decides what a device supports.
    func testProfileSupportIsWhatTheRuntimeAnswers() throws {
        let game = try AdapterProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        let adapter = A.DefaultAdapter
        XCTAssertTrue(adapter.IsProfileSupported(.Reach))
        XCTAssertTrue(adapter.IsProfileSupported(.HiDef))
    }

    /// The two `Query*` members are methods rather than accessors, so they may
    /// ask the route at the call site — and they write three values back
    /// through `inout` parameters, which is XNA's three `out`s.
    func testTheFormatNegotiationWritesAllThreeResults() throws {
        let game = try AdapterProbeGame(query: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let result = try XCTUnwrap(game.queryResult)
        // What is asserted is that all three were WRITTEN -- the negotiation's
        // own choices are the runtime's, not this binding's, so the values are
        // reported rather than expected.
        XCTAssertNotNil(result.format)
        XCTAssertNotNil(result.depth)
        // Seeded with -777, which no negotiation can answer, so this asserts
        // the third `inout` was WRITTEN rather than merely left alone. A
        // mutation that dropped the write survived a test asserting only that
        // the value was non-negative.
        XCTAssertNotEqual(result.samples, -777,
                          "the negotiated multi-sample count must be written back")
    }
}

private final class AdapterProbeGame: Microsoft.Xna.Framework.Game {
    let query: Bool
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var queryResult: (format: Microsoft.Xna.Framework.Graphics.SurfaceFormat?,
                      depth: Microsoft.Xna.Framework.Graphics.DepthFormat?,
                      samples: Int32)?
    var borrowCount = 0
    var firstAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
    var secondAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?

    init(query: Bool = false) throws {
        self.query = query
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        defer { try? Exit() }
        do {
            _ = try GraphicsDevice          // borrowing fills the adapter list
            firstAdapter = Microsoft.Xna.Framework.Graphics.GraphicsAdapter
                .Adapters.flatMap { try? $0.Item(0) }
            borrowCount += 1
            _ = try GraphicsDevice          // a second facade, same generation
            secondAdapter = Microsoft.Xna.Framework.Graphics.GraphicsAdapter
                .Adapters.flatMap { try? $0.Item(0) }
            borrowCount += 1
            guard query else { return }
            let adapter = Microsoft.Xna.Framework.Graphics.GraphicsAdapter
                .DefaultAdapter
            var format = Microsoft.Xna.Framework.Graphics.SurfaceFormat.Color
            var depth = Microsoft.Xna.Framework.Graphics.DepthFormat.Depth24
            var samples: Int32 = -777
            _ = try adapter.QueryBackBufferFormat(
                .Reach, format: .Color, depthFormat: .Depth24,
                multiSampleCount: 0,
                selectedFormat: &format, selectedDepthFormat: &depth,
                selectedMultiSampleCount: &samples)
            queryResult = (format, depth, samples)
        } catch {
            failure = error
        }
    }
}
