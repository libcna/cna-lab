import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.FrameworkDispatcher`.
///
/// What a passing test here means: the pump was ACCEPTED by the runtime. XNA's
/// `Update` raises media, microphone and storage events out of a private static
/// queue, and none of those are observable from here -- no song is playing, no
/// microphone is opened (opening one would need a new explicit authorization),
/// and no storage device changes. The claim is deliberately the narrow one.
final class Foundation72FrameworkDispatcherTests: XCTestCase {

    /// The handle `cna_framework_dispatcher_update` needs for thread affinity
    /// exists only inside a lifecycle callback, so the static member refuses
    /// outside one. XNA has no such refusal -- its dispatcher is genuinely
    /// static -- and this is the binding being honest about a capability token
    /// it does not hold rather than pretending the pump ran.
    func testUpdateOutsideTheLifecycleRefuses() {
        XCTAssertThrowsError(try Microsoft.Xna.Framework.FrameworkDispatcher.Update()) { error in
            guard case CNAError.callbackOutsideGameLifecycle = error else {
                return XCTFail("expected callbackOutsideGameLifecycle, got \(error)")
            }
        }
    }

    /// Inside a callback the route is accepted, and accepted twice: the header
    /// says calling it while the loop runs "is harmless and does the work
    /// twice", which is the only part of that sentence this host can check.
    func testUpdateInsideTheLifecycleIsAcceptedAndRepeatable() throws {
        let game = try DispatcherProbeGame(frameLimit: 2)
        try game.Run()
        XCTAssertEqual(game.pumps, 4, "two pumps per frame over two frames")
        XCTAssertNil(game.failure, "the dispatcher refused inside a callback: "
                     + String(describing: game.failure))
    }
}

private final class DispatcherProbeGame: Microsoft.Xna.Framework.Game {
    let frameLimit: Int
    var frames = 0
    var pumps = 0
    var failure: Error?

    init(frameLimit: Int) throws {
        self.frameLimit = frameLimit
        try super.init()
    }

    // The exit is driven by FRAMES, never by pumps.
    //
    // Counting pumps here is what a first draft did, and the mutation harness
    // found it: with a broken dispatcher the counter never advances, `Exit` is
    // never reached, and the game runs until the harness's 600-second deadline
    // kills it. That verdict is HUNG, not CAUGHT -- a test that hangs on a
    // defect reports through a timeout instead of an assertion, and costs ten
    // minutes to say so.
    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        frames += 1
        do {
            try Microsoft.Xna.Framework.FrameworkDispatcher.Update()
            pumps += 1
            // Twice in one callback: documented as harmless.
            try Microsoft.Xna.Framework.FrameworkDispatcher.Update()
            pumps += 1
        } catch {
            failure = failure ?? error
        }
        if frames >= frameLimit { try Exit() }
    }
}
