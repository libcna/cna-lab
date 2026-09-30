import XCTest
@testable import CNA

/// `GraphicsDeviceInformationComparer` and `GraphicsDeviceManager.RankDevices`.
///
/// The 638-byte comparison chain, asserted one link at a time. Every link
/// returns immediately, so a test that exercised two at once would not tell
/// which one answered.
final class Foundation81DeviceRankingTests: XCTestCase {
    private typealias Info = Microsoft.Xna.Framework.GraphicsDeviceInformation

    private func candidate(
        profile: Microsoft.Xna.Framework.Graphics.GraphicsProfile = .Reach,
        fullScreen: Bool = true,
        format: Microsoft.Xna.Framework.Graphics.SurfaceFormat = .Color,
        samples: Int32 = 0,
        width: Int32 = 800, height: Int32 = 480
    ) -> Info {
        let info = Info()
        info.GraphicsProfile = profile
        info.PresentationParameters.IsFullScreen = fullScreen
        info.PresentationParameters.BackBufferFormat = format
        info.PresentationParameters.MultiSampleCount = samples
        info.PresentationParameters.BackBufferWidth = width
        info.PresentationParameters.BackBufferHeight = height
        return info
    }

    private func comparer(_ game: RankingProbeGame) throws
        -> Microsoft.Xna.Framework.GraphicsDeviceInformationComparer {
        Microsoft.Xna.Framework.GraphicsDeviceInformationComparer(
            try XCTUnwrap(game.manager))
    }

    func testTheChainInOrder() throws {
        let game = try RankingProbeGame()
        let c = try comparer(game)

        // 1. Higher profile first.
        XCTAssertEqual(c.Compare(candidate(profile: .HiDef),
                                 candidate(profile: .Reach)), -1)
        XCTAssertEqual(c.Compare(candidate(profile: .Reach),
                                 candidate(profile: .HiDef)), 1)

        // 2. The candidate agreeing with the MANAGER's IsFullScreen first.
        let managerWantsFullScreen = try XCTUnwrap(game.manager).IsFullScreen
        XCTAssertEqual(
            c.Compare(candidate(fullScreen: managerWantsFullScreen),
                      candidate(fullScreen: !managerWantsFullScreen)), -1,
            "the manager's own flag decides, not either candidate's")

        // 4. Higher multi-sample count first.
        XCTAssertEqual(c.Compare(candidate(samples: 4), candidate(samples: 0)), -1)
        XCTAssertEqual(c.Compare(candidate(samples: 0), candidate(samples: 4)), 1)
    }

    /// Three answers, and the third is `Int32.max` — which is why the rank
    /// sorts ascending: an unrelated format goes to the very end.
    func testRankFormatHasThreeAnswers() throws {
        let game = try RankingProbeGame()
        let c = try comparer(game)
        let preferred = try XCTUnwrap(game.manager).PreferredBackBufferFormat

        XCTAssertEqual(c.RankFormat(preferred), 0, "the preferred format ranks 0")

        typealias Comparer = Microsoft.Xna.Framework.GraphicsDeviceInformationComparer
        let sameDepth = Comparer.surfaceFormatBitDepth(preferred) == 32
            ? Microsoft.Xna.Framework.Graphics.SurfaceFormat.Rgba1010102
            : Microsoft.Xna.Framework.Graphics.SurfaceFormat.Bgr565
        if sameDepth != preferred {
            XCTAssertEqual(c.RankFormat(sameDepth), 1, "same bit depth ranks 1")
        }
        XCTAssertEqual(c.RankFormat(.Vector4), Int32.max,
                       "an unrelated depth ranks Int32.max, not a middling score")

        // And the rank must sort ASCENDING through Compare, which testing
        // RankFormat alone does not show: a mutation reversing that link
        // survived until this pair existed.
        XCTAssertEqual(c.Compare(candidate(format: preferred),
                                 candidate(format: .Vector4)), -1,
                       "the preferred format outranks an unrelated one")
        XCTAssertEqual(c.Compare(candidate(format: .Vector4),
                                 candidate(format: preferred)), 1)
    }

    /// `Color` and `Rgba1010102` are 32; `Bgr565`, `Bgra5551` and `Bgra4444`
    /// are 16; **everything else is zero**, so all the floating-point and
    /// compressed formats share a depth.
    func testTheBitDepthTableIsExactlyXnasFive() {
        typealias Comparer = Microsoft.Xna.Framework.GraphicsDeviceInformationComparer
        XCTAssertEqual(Comparer.surfaceFormatBitDepth(.Color), 32)
        XCTAssertEqual(Comparer.surfaceFormatBitDepth(.Rgba1010102), 32)
        XCTAssertEqual(Comparer.surfaceFormatBitDepth(.Bgr565), 16)
        XCTAssertEqual(Comparer.surfaceFormatBitDepth(.Bgra5551), 16)
        XCTAssertEqual(Comparer.surfaceFormatBitDepth(.Bgra4444), 16)
        for other: Microsoft.Xna.Framework.Graphics.SurfaceFormat in
            [.Dxt1, .Dxt5, .Alpha8, .Single, .Vector4, .HalfSingle, .HdrBlendable] {
            XCTAssertEqual(Comparer.surfaceFormatBitDepth(other), 0,
                           "\(other) is not in XNA's table")
        }
    }

    /// The aspect-ratio link only decides when the two distances differ by
    /// **more than 0.2** — otherwise the pixel count does.
    func testAspectRatioOnlyDecidesBeyondTheTolerance() throws {
        let game = try RankingProbeGame()
        let c = try comparer(game)

        // 800x480 is 1.667; 640x480 is 1.333; 1600x480 is 3.333. Against a
        // 1.667 target the first two differ by 0.333 -- beyond the tolerance --
        // while 800x480 and 816x480 differ by far less.
        let near = candidate(width: 800, height: 480)
        let wide = candidate(width: 1600, height: 480)
        XCTAssertEqual(c.Compare(near, wide), -1, "0.333 apart: shape decides")

        // Within the tolerance the pixel count decides -- and the two must
        // be made to DISAGREE, or removing the tolerance changes nothing and
        // the mutation survives, which is what happened first.
        //
        //   A 1600x960 -> aspect 1.667, exactly the target: shape says A.
        //   B 1500x960 -> aspect 1.563, 0.104 away: inside the 0.2 tolerance.
        //   Areas 1,536,000 and 1,440,000: B is nearer any target below both,
        //   so pixel count says B.
        //
        // With the tolerance the answer is B. Without it, shape would answer A.
        let exactShape = candidate(width: 1600, height: 960)
        let nearerArea = candidate(width: 1500, height: 960)
        XCTAssertEqual(c.Compare(exactShape, nearerArea), 1,
                       "inside the tolerance, area decides against shape")
    }

    /// `RankDevices` reorders the caller's list in place.
    func testRankDevicesSortsTheCallersListInPlace() throws {
        let game = try RankingProbeGame()
        let manager = try XCTUnwrap(game.manager)
        let worst = candidate(profile: .Reach, samples: 0)
        let best = candidate(profile: .HiDef, samples: 4)
        let list = CNAList<Info>()
        try list.Add(worst)
        try list.Add(best)

        try manager.RankDevices(list)

        XCTAssertTrue(try list.Item(0) === best, "the list is reordered")
        XCTAssertTrue(try list.Item(1) === worst)
        XCTAssertEqual(list.Count, 2, "and not replaced or resized")
    }
}

private final class RankingProbeGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?

    override init() throws {
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }
}

/// `GraphicsDeviceManager.FindBestDevice` — the last of the manager's five.
final class Foundation83FindBestDeviceTests: XCTestCase {

    /// Inside a callback the enumeration finds the host's one adapter and
    /// returns the best candidate for it.
    func testItFindsTheHostsAdapter() throws {
        let game = try FindBestProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        let best = try XCTUnwrap(game.best)
        XCTAssertNotNil(best.Adapter, "the candidate names the adapter it is for")
        XCTAssertEqual(best.GraphicsProfile, game.requestedProfile)
        XCTAssertGreaterThan(best.PresentationParameters.BackBufferWidth, 0)
    }

    /// **The retry is destructive**, which is XNA's behaviour and not an
    /// oversight: when multisampling has to be turned off to find anything,
    /// `PreferMultiSampling` stays off afterwards.
    ///
    /// Asserted through the observable half — a successful find with the flag
    /// set leaves it set, so the test pins that the retry did NOT run rather
    /// than pretending to observe one that could not happen here.
    func testAFindThatSucceedsFirstTimeLeavesTheFlagAlone() throws {
        let game = try FindBestProbeGame(preferMultiSampling: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.flagAfter, true,
                       "no retry was needed, so nothing was turned off")
    }

    /// **In full screen the current mode is offered twice, and the duplicate
    /// test is what stops it.**
    ///
    /// `AddDevices` offers the current mode once from `CurrentDisplayMode` and
    /// again from the supported-mode loop whenever it is also a supported mode
    /// of at least 640x480. XNA adds a candidate only when the list holds no
    /// equal one, which is what makes `GraphicsDeviceInformation.Equals`
    /// load-bearing here rather than decorative.
    ///
    /// The modes are the host's, not the test's: CNA 0.21 invented one 800x480
    /// mode, CNA 0.35 reports SDL's display (1024x768 under the dummy video
    /// driver), and an earlier full-screen game in the same process can leave
    /// the dummy display in a mode it does not list. So the count is derived
    /// from what the adapter reports -- one candidate per distinct offered
    /// mode -- and no two are equal.
    func testFullScreenOffersTheCurrentModeOnlyOnce() throws {
        let game = try FindBestProbeGame(fullScreen: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.candidateCount, game.distinctOfferedModes,
                       "one candidate per distinct offered mode; the current "
                       + "mode is added once even when it is also supported")
        XCTAssertEqual(game.equalCandidatePairs, 0)
    }

    /// **The two refusals cannot be reached on this host**, and the message
    /// is asserted directly rather than through a path that does not exist.
    ///
    /// `FindBestDevice` throws when the candidate list is empty. Emptying it
    /// needs either an adapter supporting neither profile or an override of
    /// `RankDevices` that discards — and this host reports one adapter that
    /// supports both. Reaching the throw by subclassing the manager would test
    /// the subclass, not the member.
    ///
    /// So what is pinned is Microsoft's own text and its one substitution,
    /// which is the half a consumer actually sees.
    func testTheRefusalMessagesAreMicrosoftsOwn() {
        typealias Manager = Microsoft.Xna.Framework.GraphicsDeviceManager
        let reach = Manager.noCompatibleDevicesMessage(.Reach)
        XCTAssertTrue(reach.hasPrefix(
            "Could not find a Direct3D device that supports the XNA Framework Reach profile."),
            "the profile is substituted into {0}")
        XCTAssertTrue(reach.contains("\r\n\r\nAvoid running under Remote Desktop"),
                      "and the CRLF paragraphs are Microsoft's, not reflowed")
        XCTAssertEqual(
            Manager.noCompatibleDevicesAfterRankingMessage,
            "The process of ranking devices removed all compatible devices.")
    }
}

private final class FindBestProbeGame: Microsoft.Xna.Framework.Game {
    let preferMultiSampling: Bool
    let fullScreen: Bool
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var best: Microsoft.Xna.Framework.GraphicsDeviceInformation?
    var flagAfter: Bool?
    var candidateCount: Int32?
    var distinctOfferedModes: Int32?
    var equalCandidatePairs: Int?
    var requestedProfile = Microsoft.Xna.Framework.Graphics.GraphicsProfile.Reach

    init(preferMultiSampling: Bool = false, fullScreen: Bool = false) throws {
        self.preferMultiSampling = preferMultiSampling
        self.fullScreen = fullScreen
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        defer { try? Exit() }
        do {
            guard let manager else { return }
            _ = try GraphicsDevice          // fills the adapter list
            manager.PreferMultiSampling = preferMultiSampling
            requestedProfile = manager.GraphicsProfile

            if fullScreen {
                manager.IsFullScreen = true
                let found = CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>()
                try manager.testOnlyAddDevices(true, found)
                candidateCount = found.Count
                var pairs = 0
                for i in 0 ..< found.Count {
                    for j in (i + 1) ..< max(i + 1, found.Count)
                    where try found.Item(i).Equals(found.Item(j)) { pairs += 1 }
                }
                equalCandidatePairs = pairs
                var offered: [String] = []
                var supported: [String] = []
                let adapter = Microsoft.Xna.Framework.Graphics.GraphicsAdapter.DefaultAdapter
                let key = { (mode: Microsoft.Xna.Framework.Graphics.DisplayMode) in
                    "\(mode.Width)x\(mode.Height)/\(mode.Format)" }
                if let current = adapter.CurrentDisplayMode { offered.append(key(current)) }
                if let modes = adapter.SupportedDisplayModes {
                    let cursor = modes.GetEnumerator()
                    while let mode = try cursor.Next() {
                        if mode.Width >= 640 && mode.Height >= 480 { supported.append(key(mode)) }
                    }
                }
                distinctOfferedModes = Int32(Set(offered + supported).count)
                return
            }
            best = try manager.FindBestDevice(true)
            flagAfter = manager.PreferMultiSampling
        } catch {
            failure = error
        }
    }
}
