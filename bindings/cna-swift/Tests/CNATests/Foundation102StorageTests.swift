// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// Storage, projected at Foundation 102.
///
/// **These tests need no `Game`.** Every other runtime-backed suite here drives
/// a probe game, because every other owned type reaches its routes through
/// `RuntimeRegistry.current()`. `cna_storage_device_show_selector` takes no
/// game handle, so `StorageDevice` does not either, and the tests say so by
/// simply calling it.
///
/// **They write to a real directory and clean up after themselves.** CNA's
/// storage root is `~/.local/share/game`, which on this machine holds forty
/// directories belonging to other agents' runs. The container name below is
/// this suite's own, nothing else is touched, and `DeleteContainer` removes it
/// at the end. Foundation 102 learned that the hard way by deleting a
/// neighbour's empty directory it had mistaken for its own.
final class Foundation102StorageTests: XCTestCase {

    private static let container = "cna-swift-f102-tests"

    private func device() throws -> Microsoft.Xna.Framework.Storage.StorageDevice {
        var delivered: (any CNAAsyncResult)?
        let result = try Microsoft.Xna.Framework.Storage.StorageDevice
            .BeginShowSelector({ delivered = $0 }, state: nil)
        XCTAssertNotNil(
            delivered,
            "the callback must have run before BeginShowSelector returned: CNA's "
            + "header says the completion callback fires before the call does")
        return try Microsoft.Xna.Framework.Storage.StorageDevice
            .EndShowSelector(result)
    }

    // MARK: - Selection

    func testBeginShowSelectorCompletesSynchronously() throws {
        let result = try Microsoft.Xna.Framework.Storage.StorageDevice
            .BeginShowSelector({ _ in }, state: nil)
        XCTAssertTrue(result.IsCompleted)
        XCTAssertTrue(
            result.CompletedSynchronously,
            "CNA's storage routes finish on the calling thread, and the CLR "
            + "contract has a property for saying exactly that")
    }

    func testTheStateIsHandedBackUnchanged() throws {
        let token = NSObject()
        let result = try Microsoft.Xna.Framework.Storage.StorageDevice
            .BeginShowSelector({ _ in }, state: token)
        XCTAssertTrue(result.AsyncState as? NSObject === token)
    }

    func testANilStateIsAllowedAndReportedAsNil() throws {
        let result = try Microsoft.Xna.Framework.Storage.StorageDevice
            .BeginShowSelector({ _ in }, state: nil)
        XCTAssertNil(result.AsyncState)
    }

    func testEndShowSelectorRefusesAContainerResult() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        XCTAssertThrowsError(
            try Microsoft.Xna.Framework.Storage.StorageDevice
                .EndShowSelector(opened),
            "a result from BeginOpenContainer is not a device result"
        ) { error in
            XCTAssertTrue(error is CNAArgumentException)
        }
        try selector.EndOpenContainer(opened).Dispose()
    }

    // MARK: - Device state

    func testTheDeviceAnswersItsSpaceAndConnection() throws {
        let selector = try device()
        XCTAssertTrue(selector.IsConnected)
        let total = try selector.TotalSpace
        let free = try selector.FreeSpace
        XCTAssertGreaterThan(total, 0)
        XCTAssertGreaterThan(free, 0)
        // STRICTLY less, not `<=`. The weaker form let
        // `storage-free-space-answers-the-total` survive: a FreeSpace that
        // reads the total-space route answers free == total, which satisfies
        // `<=` and reports a nearly full device as empty.
        //
        // This does lean on the host having something on its storage volume.
        // A completely empty one would make free == total honestly and fail
        // here -- loudly, which is the right way round for an assertion that
        // has stopped being able to tell two routes apart.
        XCTAssertLessThan(
            free, total,
            "FreeSpace and TotalSpace must be different routes, and on any "
            + "host with a byte written they answer differently")
    }

    // MARK: - Containers

    func testAContainerReportsTheNameItWasOpenedWith() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        XCTAssertEqual(try container.DisplayName, Self.container)
        XCTAssertFalse(container.IsDisposed)
        try container.Dispose()
        XCTAssertTrue(container.IsDisposed)
    }

    func testTheContainerRefusesEveryCallAfterDisposal() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        try container.Dispose()
        XCTAssertThrowsError(try container.FileExists("anything")) { error in
            XCTAssertTrue(error is CNAObjectDisposedException)
        }
        XCTAssertThrowsError(try container.DisplayName) { error in
            XCTAssertTrue(error is CNAObjectDisposedException)
        }
    }

    func testDisposingTwiceIsSilent() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        try container.Dispose()
        XCTAssertNoThrow(try container.Dispose())
    }

    func testDisposingEventFiresOnceAfterNativeDisposal() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        var observations: [Bool] = []
        _ = container.Disposing.Add { sender, _ in
            XCTAssertTrue(sender as AnyObject === container)
            observations.append(container.IsDisposed)
        }

        try container.Dispose()
        try container.Dispose()
        XCTAssertEqual(observations, [true])
    }

    func testDeviceChangedHasStableProcessWideIdentity() {
        XCTAssertTrue(
            Microsoft.Xna.Framework.Storage.StorageDevice.DeviceChanged ===
            Microsoft.Xna.Framework.Storage.StorageDevice.DeviceChanged)
    }

    func testTheContainerCarriesTheDeviceItCameFrom() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        XCTAssertTrue(try container.StorageDevice === selector)
        try container.Dispose()
    }

    func testDroppingAContainerReleasesItsHandleBeforeItsDevice() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        var container: Microsoft.Xna.Framework.Storage.StorageContainer? =
            try selector.EndOpenContainer(opened)
        weak var released = container

        container = nil
        XCTAssertNil(released, "the last Swift reference must run deinit")

        // CNA refuses to destroy a StorageDevice while any native container
        // handle remains open. This is the direct native observation that a
        // weak-reference-only test lacked: replacing the deinit destroy call
        // with a no-op leaves openContainers non-zero and returns
        // CNA_RESULT_INVALID_STATE here.
        let functions = try NativeFunctions.load()
        XCTAssertEqual(
            functions.storageDeviceDestroy(selector.nativeHandle), 0,
            "StorageContainer.deinit must release the child before its device")
    }

    // MARK: - Files and directories

    func testAFileRoundTripsThroughExistenceAndNames() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        defer { try? container.Dispose() }

        XCTAssertFalse(try container.DirectoryExists("f102"))
        try container.CreateDirectory("f102")
        XCTAssertTrue(try container.DirectoryExists("f102"))
        XCTAssertTrue(try container.GetDirectoryNames().contains("f102"))
        XCTAssertTrue(try container.GetDirectoryNames("f10*").contains("f102"))
        XCTAssertFalse(try container.GetDirectoryNames("zzz*").contains("f102"))

        try container.DeleteDirectory("f102")
        XCTAssertFalse(try container.DirectoryExists("f102"))
    }

    func testAnAbsentFileIsReportedAbsent() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        defer { try? container.Dispose() }
        XCTAssertFalse(try container.FileExists("no-such-file"))
        XCTAssertEqual(try container.GetFileNames("no-such-*"), [])
    }

    func testDuplexStorageStreamWritesSeeksReadsAndResizes() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        let stream = try container.CreateFile("duplex.bin")

        XCTAssertTrue(try stream.CanWrite)
        XCTAssertTrue(try stream.CanSeek)
        let bytes: [UInt8] = [0x10, 0x20, 0x30, 0x40]
        try stream.Write(bytes, offset: 0, count: Int32(bytes.count))
        try stream.Flush()
        XCTAssertEqual(try stream.Length, 4)
        XCTAssertEqual(try stream.Seek(0, origin: .Begin), 0)
        var read = [UInt8](repeating: 0, count: 4)
        XCTAssertEqual(try stream.Read(&read, offset: 0, count: 4), 4)
        XCTAssertEqual(read, bytes)
        try stream.SetLength(2)
        XCTAssertEqual(try stream.Length, 2)
        try stream.Close()
        XCTAssertNoThrow(try stream.Close())
        XCTAssertThrowsError(try stream.Flush()) { error in
            XCTAssertTrue(error is CNAObjectDisposedException)
        }
        XCTAssertTrue(try container.FileExists("duplex.bin"))
        try container.Dispose()
    }

    func testEveryOpenFileOverloadReturnsAWorkingStream() throws {
        let selector = try device()
        let opened = try selector.BeginOpenContainer(
            Self.container, callback: { _ in }, state: nil)
        let container = try selector.EndOpenContainer(opened)
        let created = try container.CreateFile("overloads.bin")
        try created.Write([1, 2, 3], offset: 0, count: 3)
        try created.Close()

        let byMode = try container.OpenFile("overloads.bin", fileMode: .Open)
        XCTAssertEqual(try byMode.Length, 3)
        try byMode.Close()
        let byAccess = try container.OpenFile(
            "overloads.bin", fileMode: .Open, fileAccess: .Read)
        XCTAssertTrue(try byAccess.CanRead)
        try byAccess.Close()
        let byShare = try container.OpenFile(
            "overloads.bin", fileMode: .Open, fileAccess: .Read,
            fileShare: .Read)
        XCTAssertEqual(try byShare.Length, 3)
        try byShare.Close()
        try container.Dispose()
    }

    // MARK: - Cleanup

    /// Removes what this suite created, and only that.
    override class func tearDown() {
        let selector = try? Microsoft.Xna.Framework.Storage.StorageDevice
            .BeginShowSelector({ _ in }, state: nil)
        if let selector,
           let opened = try? Microsoft.Xna.Framework.Storage.StorageDevice
            .EndShowSelector(selector) {
            try? opened.DeleteContainer(container)
        }
        super.tearDown()
    }
}
