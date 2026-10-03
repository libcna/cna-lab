// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.Graphics.OcclusionQuery`.
///
/// What this suite can assert depends on whether the qualified HEADLESS
/// renderer has a query object at all, so the probe records the refusal as a
/// first-class outcome rather than skipping. Either way the projection is
/// measured: a backend that supports queries must count, and one that does not
/// must **say so at construction** rather than hand back a query that answers
/// zero and looks truthful.
final class Foundation88OcclusionQueryTests: XCTestCase {

    func testConstructionEitherSucceedsOrRefusesByName() throws {
        let game = try QueryProbeGame()
        try game.Run()
        let query = game.query
        try game.Dispose()
        if let failure = game.failure { throw failure }

        if let refusal = game.createFailure {
            // The backend has no query object. That is a documented CNA
            // outcome, and what matters is that it reached the caller.
            XCTAssertTrue("\(refusal)".contains("cna_occlusion_query_create"),
                          "the refusal names the route that refused")
            XCTAssertNil(query, "a refused construction yields no object")
            return
        }

        XCTAssertEqual(game.disposedDuringLoop, false,
                       "the query is alive while the game is")
        XCTAssertTrue(game.carriedItsDevice, "a resource carries its device")
        XCTAssertTrue(game.beginEndAccepted, "Begin and End were accepted")

        // CNA refuses to destroy a game while an owned child survives, so a
        // query the caller never disposed must be torn down by the parent --
        // and the game above was disposed without raising, which is the proof.
        let released = try XCTUnwrap(query)
        XCTAssertTrue(released.IsDisposed,
                      "the parent released the query the caller forgot")
    }

    /// The divergence worth pinning: an infallible getter that cannot report a
    /// failure answers `true` so the caller's wait loop ends, and the failure
    /// surfaces from `PixelCount`, which can raise.
    func testAFailedStatusReadEndsTheWaitRatherThanHangingIt() throws {
        let game = try QueryProbeGame(disposeBeforeReading: true)
        try game.Run()
        // One C-owned CNA game may be active at a time, so the probe is
        // released before the assertions rather than left to a deinit.
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object to dispose")
        }
        XCTAssertEqual(game.completeAfterDispose, true,
                       "a query that cannot answer must not hang a wait loop")
        XCTAssertNotNil(game.pixelCountFailure,
                        "the fallible member is where the failure surfaces")
        XCTAssertNotNil(game.statusFailureAfterDispose,
                        "the error IsComplete could not raise is kept, not dropped")
    }

    /// The round trip the type exists for: begin, draw, end, wait, read.
    /// The count itself is whatever the qualified renderer produced -- what is
    /// asserted is that a completed query **answers** rather than raising, so
    /// the throwing getter is not simply a member that always throws.
    func testACompletedQueryAnswersItsCount() throws {
        let game = try QueryProbeGame(runAFullQuery: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object")
        }
        XCTAssertNil(game.pixelCountFailure,
                     "a completed query reports a count instead of refusing")
        let count = try XCTUnwrap(game.pixelCount)
        XCTAssertGreaterThanOrEqual(count, 0, "a pixel tally is never negative")
        XCTAssertTrue(game.completedWithinBudget,
                      "the query finished inside a bounded wait")
    }

    /// A query that was never submitted has no count, and the route says so.
    ///
    /// The handle is perfectly valid here, so the refusal is the **managed**
    /// one: CNA answered a count for a query that was never begun, and XNA's
    /// `IL_DIRECT_THROW` InvalidOperationException is reproduced over it.
    func testAnUnfinishedQueryRefusesItsCount() throws {
        let game = try QueryProbeGame(readCountBeforeAnyQuery: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object")
        }
        XCTAssertTrue(game.unfinishedCountFailureIsFromTheRoute,
                      "an unfinished query raises InvalidOperationException; "
                      + "measured: \(game.unfinishedCountDescription ?? "nothing")")
    }

    /// XNA's rearm rule, which CNA does not enforce: a second `Begin` is
    /// refused until the previous result has been *looked at*, and reading
    /// `IsComplete` is what looks at it.
    func testASecondBeginIsRefusedUntilTheResultIsChecked() throws {
        let game = try QueryProbeGame(rearmRule: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object")
        }
        let refusal = try XCTUnwrap(
            game.rearmFailure as? CNAInvalidOperationException)
        XCTAssertEqual(
            refusal.Message,
            "Begin may not be called on this query object again before "
            + "IsComplete has been checked.")
        XCTAssertTrue(game.beginAcceptedAfterCheck,
                      "reading IsComplete rearms the query")
    }

    /// The refusal `PixelCount` raises carries XNA's own text, which tells the
    /// caller what to do about it -- check `IsComplete` first.
    func testTheUnfinishedCountRefusalCarriesTheXnaMessage() throws {
        let game = try QueryProbeGame(readCountBeforeAnyQuery: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object")
        }
        let refusal = try XCTUnwrap(
            game.unfinishedCountFailure as? CNAInvalidOperationException)
        XCTAssertEqual(
            refusal.Message,
            "The query data is not yet available. Use the IsComplete property "
            + "to determine if the data is available before attempting to "
            + "retrieve it.")
    }

    func testDisposalReleasesTheQueryOnce() throws {
        let game = try QueryProbeGame(disposeBeforeReading: true)
        try game.Run()
        // One C-owned CNA game may be active at a time, so the probe is
        // released before the assertions rather than left to a deinit.
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("this backend has no query object to dispose")
        }
        XCTAssertTrue(game.disposedFlag, "IsDisposed follows the release")
        XCTAssertTrue(game.secondDisposeSucceeded, "disposing twice is a no-op")
    }
}

private final class QueryProbeGame: Microsoft.Xna.Framework.Game {
    let disposeBeforeReading: Bool
    let runAFullQuery: Bool
    let readCountBeforeAnyQuery: Bool
    let rearmRule: Bool
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?

    var failure: Error?
    var createFailure: Error?
    var query: Microsoft.Xna.Framework.Graphics.OcclusionQuery?
    var beginEndAccepted = false
    var disposedDuringLoop: Bool?
    var carriedItsDevice = false
    var completeAfterDispose: Bool?
    var pixelCountFailure: Error?
    var disposedFlag = false
    var secondDisposeSucceeded = false

    var pixelCount: Int32?
    var statusFailureAfterDispose: Error?
    var unfinishedCountFailure: Error?
    var unfinishedCountDescription: String?
    var rearmFailure: Error?
    var beginAcceptedAfterCheck = false
    var unfinishedCountFailureIsFromTheRoute = false
    var completedWithinBudget = false

    init(disposeBeforeReading: Bool = false, runAFullQuery: Bool = false,
         readCountBeforeAnyQuery: Bool = false, rearmRule: Bool = false) throws {
        self.disposeBeforeReading = disposeBeforeReading
        self.runAFullQuery = runAFullQuery
        self.readCountBeforeAnyQuery = readCountBeforeAnyQuery
        self.rearmRule = rearmRule
        try super.init()
        // Game.GraphicsDevice resolves IGraphicsDeviceService out of Services,
        // and raises NoGraphicsDeviceService when there is none.
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
        // XNA builds an OcclusionQuery only when the profile advertises one,
        // and Reach does not; CNA follows that IL since ABI 0.35.
        manager?.GraphicsProfile = .HiDef
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            guard let device = try GraphicsDevice else { return }
            let made: Microsoft.Xna.Framework.Graphics.OcclusionQuery
            do {
                made = try Microsoft.Xna.Framework.Graphics.OcclusionQuery(
                    graphicsDevice: device)
            } catch {
                createFailure = error
                return
            }
            query = made
            if rearmRule {
                try made.Begin()
                try made.End()
                do { try made.Begin() } catch { rearmFailure = error }
                _ = made.IsComplete
                do { try made.Begin(); beginAcceptedAfterCheck = true }
                catch { beginAcceptedAfterCheck = false }
                try made.End()
                try made.Dispose()
                return
            }
            if readCountBeforeAnyQuery {
                do { _ = try made.PixelCount }
                catch {
                    unfinishedCountFailure = error
                    unfinishedCountFailureIsFromTheRoute =
                        error is CNAInvalidOperationException
                }
                unfinishedCountDescription =
                    unfinishedCountFailure.map { "\($0)" } ?? "SUCCEEDED"
                try made.Dispose()
                return
            }
            disposedDuringLoop = made.IsDisposed
            carriedItsDevice = made.GraphicsDevice === device

            do {
                try made.Begin()
                try made.End()
                beginEndAccepted = true
            } catch {
                // Begin/End may refuse on a backend that created the object but
                // cannot run it; that is recorded, not hidden.
                beginEndAccepted = false
            }

            if runAFullQuery {
                // A bounded wait, not a spin: a query that never completes is
                // a defect to report, not a reason to hang the suite.
                var spins = 0
                while !made.IsComplete && spins < 10_000 { spins += 1 }
                completedWithinBudget = spins < 10_000
                do { pixelCount = try made.PixelCount }
                catch { pixelCountFailure = error }
                try made.Dispose()
                return
            }
            if disposeBeforeReading {
                try made.Dispose()
                disposedFlag = made.IsDisposed
                completeAfterDispose = made.IsComplete
                statusFailureAfterDispose = made.lastStatusFailure
                do { _ = try made.PixelCount } catch { pixelCountFailure = error }
                try made.Dispose()
                secondDisposeSucceeded = true
            }
        } catch {
            failure = error
        }
    }
}
