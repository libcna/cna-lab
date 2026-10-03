import XCTest
@testable import CNA
import CNAShim

/// `Microsoft.Xna.Framework.Content.ContentManager`.
///
/// XNA's managed reader now handles custom registered readers. Projected
/// built-in assets retain CNA's one-route-per-kind backend: `Texture2D` is the
/// only admitted loader, while the other five published routes remain
/// deliberately unbound until their result ownership/adoption contracts are
/// implemented.
final class Foundation87ContentManagerTests: XCTestCase {

    private typealias Manager = Microsoft.Xna.Framework.Content.ContentManager

    // MARK: - The two message formatters, which need no runtime

    /// `FrameworkResources.BadXnbWrongType` is a three-argument format, and
    /// all three slots have to be filled -- a formatter that dropped one would
    /// still produce a plausible sentence.
    func testBadXnbWrongTypeFillsEverySlot() {
        let message = Manager.badXnbWrongTypeMessage("hero", "Texture2D", "SpriteFont")
        XCTAssertEqual(
            message,
            "Error loading \"hero\". File contains Texture2D but trying to load as SpriteFont.")
        XCTAssertFalse(message.contains("{"), "no format placeholder survives")
    }

    func testNoLoaderMessageNamesTheTypeAndSaysWhy() {
        let message = Manager.noLoaderMessage("Effect")
        XCTAssertTrue(message.contains("Effect"), "the refused type is named")
        XCTAssertTrue(message.contains(".xnb"),
                      "the managed-reader boundary is stated")
    }

    /// The mirrored structure must be byte-for-byte the ABI's. CNA refuses
    /// any `struct_size` smaller than its own `sizeof`, so a trailing field
    /// left out of the mirror is not a harmless omission -- it is a call that
    /// can never succeed, reported as an invalid configuration.
    func testCreateInfoMirrorsTheAbiLayout() {
        XCTAssertEqual(MemoryLayout<CNASwift_ContentManagerCreateInfo>.size, 32,
                       "4 + 4 + (8 + 8) + 8; the reserved field is part of sizeof")
        XCTAssertEqual(Manager.createInfoSize, 32,
                       "and the binding itself must see the same 32")
    }

    // MARK: - Everything that needs a live runtime

    func testConstructionReadsBackTheRootDirectory() throws {
        let game = try ContentProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.rootDirectory, "probe-content")
    }

    /// A manager the consumer never disposes must not keep the game alive.
    ///
    /// This is the defect the template's canary carried on every run: it
    /// reads `Game.Content`, installs one of its own through `SetContent`, and
    /// lets the first go -- after which `Game.Dispose` failed with
    ///
    ///     cna_game_destroy failed with result 3: All owned C child resources
    ///     must be destroyed before the game.
    ///
    /// Every other owned type routes its handle through `NativeHandleStorage`,
    /// whose `deinit` releases it. `ContentManager` did not, so it was the one
    /// type that could outlive its Swift object. In .NET a dropped manager
    /// does not stop a game being disposed, so this is the projection catching
    /// up with XNA rather than departing from it.
    func testADroppedManagerDoesNotBlockGameDisposal() throws {
        let game = try ContentProbeGame(dropManager: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertNoThrow(
            try game.Dispose(),
            "a content manager dropped without Dispose must be released by "
            + "deinit, or cna_game_destroy refuses the game around it")
    }

    /// The two-argument constructor assigns through the property, so the null
    /// check belongs to construction as much as to assignment.
    func testSetRootDirectoryRefusesNull() throws {
        let game = try ContentProbeGame(setNullRoot: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let refusal = try XCTUnwrap(game.nullRootFailure as? CNAArgumentNullException)
        XCTAssertEqual(refusal.ParamName, "value")
        XCTAssertEqual(game.rootDirectory, "probe-content",
                       "a refused assignment leaves the old root in place")
    }

    /// The second half of the setter, which Foundation 87 first shipped
    /// without: once anything has been loaded the root is frozen, because
    /// every cached asset was resolved against it.
    func testTheRootIsFrozenOnceAnythingHasBeenLoaded() throws {
        let game = try ContentProbeGame(freezeRoot: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let refusal = try XCTUnwrap(
            game.frozenRootFailure as? CNAInvalidOperationException)
        XCTAssertEqual(
            refusal.Message,
            "This property cannot be changed after content has been loaded "
            + "into the ContentManager.")
        XCTAssertEqual(game.rootAfterFreeze, "probe-content",
                       "the refused change left the root where it was")
        XCTAssertTrue(game.rootMovedWhileEmpty,
                      "an empty manager still accepts a new root")
    }

    /// Order matters: XNA tests disposal *before* it tests the name, so a
    /// disposed manager asked for a null asset reports disposal, not the null.
    func testDisposalIsTestedBeforeTheAssetName() throws {
        let game = try ContentProbeGame(disposeThenLoadNil: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.afterDisposeFailure is CNAObjectDisposedException,
                      "disposed wins over the null name, as in XNA")
        XCTAssertTrue(game.secondDisposeSucceeded,
                      "disposing twice is a no-op")
    }

    func testEmptyAndNullAssetNamesAreRefusedAlike() throws {
        let game = try ContentProbeGame(loadBadNames: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let nilRefusal = try XCTUnwrap(game.nilNameFailure as? CNAArgumentNullException)
        XCTAssertEqual(nilRefusal.ParamName, "assetName")
        let emptyRefusal = try XCTUnwrap(game.emptyNameFailure as? CNAArgumentNullException)
        XCTAssertEqual(emptyRefusal.ParamName, "assetName",
                       "empty is refused on the same branch as null")
    }

    /// The unwired kinds are refused **by name**, not reported as missing
    /// files -- the distinction a caller needs to tell "you cannot load this"
    /// from "this asset is not there".
    func testAnUnwiredAssetKindIsRefusedByName() throws {
        let game = try ContentProbeGame(loadUnwiredKind: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let refusal = try XCTUnwrap(
            game.unwiredFailure as? Microsoft.Xna.Framework.Content.ContentLoadException)
        let message = try XCTUnwrap(refusal.Message)
        XCTAssertTrue(message.contains("SpriteFont"),
                      "the refusal names the type that has no route")
    }

    /// A missing texture must fail through the native route, which is what
    /// separates "no loader for this kind" from "no such asset".
    func testAMissingTextureFailsThroughTheRouteNotTheTypeSwitch() throws {
        let game = try ContentProbeGame(loadMissingTexture: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let error = try XCTUnwrap(game.missingTextureFailure)
        XCTAssertFalse(
            "\(error)".contains("one route per asset kind"),
            "Texture2D IS wired, so this must be the route's failure, not the switch's")
    }

    /// `Unload` empties the cache and leaves the manager usable; `Dispose`
    /// does not. That difference is the whole reason both exist.
    func testUnloadLeavesTheManagerUsable() throws {
        let game = try ContentProbeGame(unloadThenUse: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.usableAfterUnload,
                      "Unload clears assets; it does not dispose the manager")
    }

    // MARK: - Game.Content

    /// `Content` is not Optional, so the empty case can only trap -- there is
    /// no third answer for a getter that is infallible and proven non-null.
    ///
    /// The assertion is the **compiler's**: inside the probe the value binds to
    /// a non-Optional `let` with no unwrapping, a line that does not type-check
    /// against an Optional property. XCTest cannot catch the trap itself, so
    /// the declaration is what is pinned here.
    func testContentBindsWithoutUnwrapping() throws {
        let game = try ContentProbeGame(useGameContent: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.contentBoundNonOptionally,
                      "Game.Content is declared non-Optional")
    }

    func testContentIsBuiltOnceAndCached() throws {
        let game = try ContentProbeGame(useGameContent: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.sameContentFacade,
                      "one manager per runtime generation, not one per read")
        XCTAssertEqual(game.gameContentRoot, "",
                       "the one-argument constructor's root is empty")
    }

    /// The setter's own refusal is unreachable from Swift: the writer takes
    /// the property's non-Optional type, so there is no null to pass. What is
    /// asserted instead is that the type system, not a run-time test, is what
    /// closes that hole -- the probe cannot even build a call that would need
    /// refusing, and the manager it installs survives.
    func testSetContentInstallsTheCallersManager() throws {
        let game = try ContentProbeGame(installOwnContent: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.installedContentReadBack,
                      "the manager the caller set is the one the getter answers")
    }

    /// CNA requires a content manager to be destroyed before its parent game.
    /// A caller who never disposes one must not leak it, so the manager joins
    /// the runtime's child registry and `Game.Dispose` tears it down.
    func testDisposingTheGameDisposesAManagerTheCallerForgot() throws {
        let game = try ContentProbeGame(useGameContent: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        let manager = try XCTUnwrap(game.capturedContent)
        XCTAssertFalse(manager.runtimeObjectIsDisposed, "still alive after the loop")
        try game.Dispose()
        XCTAssertTrue(manager.runtimeObjectIsDisposed,
                      "the parent tore it down, in the order CNA requires")
    }
}

private final class ContentProbeGame: Microsoft.Xna.Framework.Game {
    let setNullRoot: Bool
    let disposeThenLoadNil: Bool
    let loadBadNames: Bool
    let loadUnwiredKind: Bool
    let loadMissingTexture: Bool
    let unloadThenUse: Bool
    let freezeRoot: Bool
    let skipManager: Bool
    let useGameContent: Bool
    let installOwnContent: Bool
    let dropManager: Bool

    var failure: Error?
    var rootDirectory: String?
    var nullRootFailure: Error?
    var afterDisposeFailure: Error?
    var secondDisposeSucceeded = false
    var nilNameFailure: Error?
    var emptyNameFailure: Error?
    var unwiredFailure: Error?
    var missingTextureFailure: Error?
    var usableAfterUnload = false
    var frozenRootFailure: Error?
    var rootAfterFreeze: String?
    var rootMovedWhileEmpty = false
    var sameContentFacade = false
    var gameContentRoot: String?
    var capturedContent: Microsoft.Xna.Framework.Content.ContentManager?
    var installedContentReadBack = false
    var contentBoundNonOptionally = false

    init(setNullRoot: Bool = false, disposeThenLoadNil: Bool = false,
         loadBadNames: Bool = false, loadUnwiredKind: Bool = false,
         loadMissingTexture: Bool = false, unloadThenUse: Bool = false,
         skipManager: Bool = false, useGameContent: Bool = false,
         installOwnContent: Bool = false, freezeRoot: Bool = false,
         dropManager: Bool = false) throws {
        self.dropManager = dropManager
        self.setNullRoot = setNullRoot
        self.disposeThenLoadNil = disposeThenLoadNil
        self.loadBadNames = loadBadNames
        self.loadUnwiredKind = loadUnwiredKind
        self.loadMissingTexture = loadMissingTexture
        self.unloadThenUse = unloadThenUse
        self.freezeRoot = freezeRoot
        self.skipManager = skipManager
        self.useGameContent = useGameContent
        self.installOwnContent = installOwnContent
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            if useGameContent {
                // No `if let`, no `!`: this does not compile if Content is
                // Optional, which is the point of the test that reads the flag.
                let first: Microsoft.Xna.Framework.Content.ContentManager = Content
                contentBoundNonOptionally = true
                capturedContent = first
                sameContentFacade = Content === first
                gameContentRoot = first.RootDirectory
                return
            }
            if dropManager {
                // The consumer pattern the template's canary writes: build a
                // manager, keep no reference, walk away. Nothing here disposes
                // it, and the runtime's child registry holds only a WEAK
                // reference -- so by the time `Game.Dispose` walks that
                // registry the entry is nil and the handle would be destroyed
                // by nobody. `cna_game_destroy` then refuses the whole game.
                _ = try Microsoft.Xna.Framework.Content.ContentManager(
                    serviceProvider: Services, rootDirectory: "dropped")
                return
            }
            if installOwnContent {
                let mine = try Microsoft.Xna.Framework.Content.ContentManager(
                    serviceProvider: Services, rootDirectory: "mine")
                try SetContent(mine)
                installedContentReadBack = Content === mine
                return
            }
            let manager = try Microsoft.Xna.Framework.Content.ContentManager(
                serviceProvider: Services, rootDirectory: "probe-content")
            rootDirectory = manager.RootDirectory

            if freezeRoot {
                // Empty cache: the root still moves.
                try manager.SetRootDirectory("elsewhere")
                rootMovedWhileEmpty = manager.RootDirectory == "elsewhere"
                try manager.SetRootDirectory("probe-content")
                manager.testOnlyRecordLoadedAsset("hero")
                do { try manager.SetRootDirectory("elsewhere") }
                catch { frozenRootFailure = error }
                rootAfterFreeze = manager.RootDirectory
                try manager.Dispose()
                return
            }
            if setNullRoot {
                do { try manager.SetRootDirectory(nil) }
                catch { nullRootFailure = error }
                rootDirectory = manager.RootDirectory
            }
            if loadBadNames {
                do { let _: Microsoft.Xna.Framework.Graphics.Texture2D =
                        try manager.Load(nil) }
                catch { nilNameFailure = error }
                do { let _: Microsoft.Xna.Framework.Graphics.Texture2D =
                        try manager.Load("") }
                catch { emptyNameFailure = error }
            }
            if loadUnwiredKind {
                do { let _: Microsoft.Xna.Framework.Graphics.SpriteFont =
                        try manager.Load("any") }
                catch { unwiredFailure = error }
            }
            if loadMissingTexture {
                do { let _: Microsoft.Xna.Framework.Graphics.Texture2D =
                        try manager.Load("no-such-asset") }
                catch { missingTextureFailure = error }
            }
            if unloadThenUse {
                try manager.Unload()
                do { let _: Microsoft.Xna.Framework.Graphics.Texture2D =
                        try manager.Load("") }
                catch {
                    // Still the name check, not a disposal report: the manager
                    // survived Unload.
                    usableAfterUnload = error is CNAArgumentNullException
                }
            }
            if disposeThenLoadNil {
                try manager.Dispose()
                do { let _: Microsoft.Xna.Framework.Graphics.Texture2D =
                        try manager.Load(nil) }
                catch { afterDisposeFailure = error }
                try manager.Dispose()
                secondDisposeSucceeded = true
            } else {
                try manager.Dispose()
            }
        } catch {
            failure = error
        }
    }
}
