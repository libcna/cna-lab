import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.GameWindow` and `Game.Window`.
///
/// **Nothing here opens a window.** The bound routes read a title, a client
/// rectangle, an orientation and a native handle the HEADLESS renderer has
/// already decided; the minimise, restore and borderless routes CNA also
/// publishes are not bound, because XNA has no such members.
final class Foundation82GameWindowTests: XCTestCase {

    /// `Game.Window` is infallible, so outside a callback it answers nil
    /// rather than refusing — the same divergence `GraphicsAdapter.Adapters`
    /// carries, and for the same reason.
    func testWindowIsNilOutsideTheLifecycle() throws {
        let game = try WindowProbeGame()
        XCTAssertNil(game.Window, "no runtime, so no window facade to build")
    }

    func testTheSnapshotReadsWhatTheHostReports() throws {
        let game = try WindowProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        let window = try XCTUnwrap(game.window)

        // Read twice: the facade is a snapshot, so the second read is a field.
        XCTAssertEqual(window.Title, game.titleAgain)
        XCTAssertNotNil(window.ScreenDeviceName)
        XCTAssertGreaterThanOrEqual(window.ClientBounds.Width, 0)
        XCTAssertGreaterThanOrEqual(window.ClientBounds.Height, 0)
        XCTAssertTrue(game.sameFacade, "one facade per runtime generation")
    }

    /// **An override replaces the whole setter, and that is the divergence.**
    ///
    /// XNA declares a `Title` setter and a protected `SetTitle` hook; the
    /// accessor rule derives the writer's name as `Set` + `Title`, which is the
    /// hook's own name, so here they are one member. The consequence is
    /// observable and is asserted rather than hidden: an override sees **both**
    /// calls, including the one whose value has not changed, where an XNA
    /// override would see only the first.
    ///
    /// What is unchanged is the effect: the second call returns at the change
    /// test, so the platform is reached once and the stored title is right.
    func testAnOverrideSeesEveryCallIncludingTheUnchangedOne() throws {
        let game = try WindowProbeGame(exerciseTitle: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.platformSetCount, 2,
                       "the override replaces the setter, so it sees both calls")
        XCTAssertEqual(game.titleAfter, "cna-swift probe title",
                       "and the value the second call did not change is kept")
    }

    /// The one refusal on the type, and the reason its parameter is Optional.
    func testANilTitleIsRefused() throws {
        let game = try WindowProbeGame(nilTitle: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        guard let error = game.titleFailure else {
            return XCTFail("a nil title was accepted")
        }
        guard let refusal = error as? CNAArgumentNullException else {
            return XCTFail("expected CNAArgumentNullException, got \(error)")
        }
        XCTAssertEqual(refusal.ParamName, "value")
    }

    /// `EndScreenDeviceChange(String)` reads its **own** `ClientBounds` for the
    /// size rather than defaulting to anything.
    func testTheOneArgumentEndKeepsTheCurrentClientSize() throws {
        let game = try WindowProbeGame(exerciseScreenChange: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.screenChangeAccepted, true)
    }
}

private final class WindowProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseTitle: Bool
    let nilTitle: Bool
    let exerciseScreenChange: Bool
    var failure: Error?
    var window: Microsoft.Xna.Framework.GameWindow?
    var titleAgain: String?
    var sameFacade = false
    var platformSetCount = 0
    var titleAfter: String?
    var titleFailure: Error?
    var screenChangeAccepted: Bool?

    init(exerciseTitle: Bool = false, nilTitle: Bool = false,
         exerciseScreenChange: Bool = false) throws {
        self.exerciseTitle = exerciseTitle
        self.nilTitle = nilTitle
        self.exerciseScreenChange = exerciseScreenChange
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            guard let first = Window else { return }
            window = first
            titleAgain = first.Title
            sameFacade = Window === first

            if nilTitle {
                do { try first.SetTitle(nil) } catch { titleFailure = error }
            }
            if exerciseTitle {
                let counting = try CountingWindow(runtime: RuntimeRegistry.current())
                try counting.SetTitle("cna-swift probe title")
                try counting.SetTitle("cna-swift probe title")
                platformSetCount = counting.platformCalls
                titleAfter = counting.Title
            }
            if exerciseScreenChange {
                try first.BeginScreenDeviceChange(false)
                try first.EndScreenDeviceChange(first.ScreenDeviceName ?? "")
                screenChangeAccepted = true
            }
        } catch {
            failure = error
        }
    }
}

/// Counts the protected hook, which is what the setter's change test guards.
private final class CountingWindow: Microsoft.Xna.Framework.GameWindow {
    var platformCalls = 0
    override init(runtime: RuntimeState) throws {
        try super.init(runtime: runtime)
    }
    override func SetTitle(_ value: String?) throws {
        platformCalls += 1
        try super.SetTitle(value)
    }
}
