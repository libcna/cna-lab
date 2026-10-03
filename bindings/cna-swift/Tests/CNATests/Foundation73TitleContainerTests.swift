import XCTest
import Foundation
@testable import CNA

/// `Microsoft.Xna.Framework.TitleContainer`.
///
/// One public member, and most of what is asserted here is the managed
/// path-normalisation underneath it -- `GetCleanPath`, `IsCleanPathAbsolute`
/// and `CollapseParentDirectory` -- because which names XNA refuses is XNA
/// behaviour, and a native runtime that happened to agree would still not be
/// the authority for it.
final class Foundation73TitleContainerTests: XCTestCase {

    private typealias Title = Microsoft.Xna.Framework.TitleContainer

    // MARK: - GetCleanPath, instruction for instruction

    func testGetCleanPathNormalisesSeparatorsAndCurrentDirectory() {
        XCTAssertEqual(Title.GetCleanPath("a/b/c"), "a\\b\\c",
                       "forward slashes become backslashes first")
        XCTAssertEqual(Title.GetCleanPath("a\\.\\b"), "a\\b",
                       #"\.\ collapses"#)
        XCTAssertEqual(Title.GetCleanPath(".\\a"), "a",
                       "a leading .\\ is stripped")
        XCTAssertEqual(Title.GetCleanPath(".\\.\\a"), "a")

        // The loop needs an input that SURVIVES the "\.\" replacement two
        // steps earlier, and ".\.\a" is not one: that replacement already
        // collapses it to ".\a", so one strip finishes the job and an `if`
        // would pass. A mutation turning the while into an if survived on
        // exactly that, and this is the input that fails it.
        //
        //   "./././a" -> "\" swap    -> ".\.\.\a"
        //             -> "\.\" swap -> ".\.\a"     (non-overlapping, so one match)
        //             -> strip ".\"  -> ".\a"
        //             -> strip ".\"  -> "a"           <- the second iteration
        XCTAssertEqual(Title.GetCleanPath("./././a"), "a",
                       "the leading .\\ strip is a loop, not a single test")
        XCTAssertEqual(Title.GetCleanPath("."), "",
                       "a lone dot becomes empty, the last thing the method does")
    }

    /// The trailing-`\.` loop has two arms and the short one is easy to miss:
    /// when what is left is no longer than `"\."` itself the whole path becomes
    /// a single separator rather than the empty string.
    func testGetCleanPathTrailingDotHasTwoArms() {
        XCTAssertEqual(Title.GetCleanPath("a\\."), "a")
        XCTAssertEqual(Title.GetCleanPath("\\."), "\\")
    }

    func testGetCleanPathCollapsesParentDirectories() {
        XCTAssertEqual(Title.GetCleanPath("a\\..\\b"), "b")
        XCTAssertEqual(Title.GetCleanPath("a\\b\\..\\c"), "a\\c")
        // A trailing "\.." leaves the SEPARATOR behind, and this is XNA's
        // arithmetic rather than an oversight here. CollapseParentDirectory is
        // called with position = Length - 3 = 3, finds the previous separator
        // at index 1 so start = 2, and removes position - start + removeLength
        // = 4 characters from index 2 -- indices 2 through 5 of "a\b\..",
        // which is "b\..", leaving "a\".
        //
        // Asserted as "a" first, which is what the shape of the operation
        // suggests. The IL says otherwise and the IL is the authority.
        XCTAssertEqual(Title.GetCleanPath("a\\b\\.."), "a\\")
    }

    /// `CollapseParentDirectory` answers `Math.Max(start - 1, 1)`, never zero.
    /// That floor is what stops `GetCleanPath`'s loop from rescanning from the
    /// beginning forever, so it is asserted directly rather than inferred from
    /// the loop terminating.
    func testCollapseParentDirectoryNeverAnswersBelowOne() {
        var path = "a\\..\\b"
        let resumed = Title.CollapseParentDirectory(&path, position: 1, removeLength: 4)
        XCTAssertEqual(path, "b")
        XCTAssertGreaterThanOrEqual(resumed, 1)
    }

    // MARK: - IsCleanPathAbsolute, six tests that all answer true

    func testTheSevenBadCharactersAreRefused() {
        for bad in [":", "*", "?", "\"", "<", ">", "|"] {
            XCTAssertTrue(Title.IsCleanPathAbsolute("a\(bad)b"),
                          "\(bad) is one of badCharacters, read from the .cctor's array initializer")
        }
        XCTAssertFalse(Title.IsCleanPathAbsolute("ab"))
    }

    func testEscapesAreRefusedAndTheyAreFourSeparateTests() {
        XCTAssertTrue(Title.IsCleanPathAbsolute("\\rooted"))
        XCTAssertTrue(Title.IsCleanPathAbsolute("..\\up"))
        XCTAssertTrue(Title.IsCleanPathAbsolute("a\\..\\b"))
        XCTAssertTrue(Title.IsCleanPathAbsolute("a\\.."))
        XCTAssertTrue(Title.IsCleanPathAbsolute(".."))
        XCTAssertFalse(Title.IsCleanPathAbsolute("a\\b"))
    }

    // MARK: - OpenStream's own refusals

    func testNullAndEmptyAreOneTestAndRefuseBeforeAnythingElse() {
        for name in [nil, ""] as [String?] {
            XCTAssertThrowsError(try Title.OpenStream(name)) { error in
                guard let failure = error as? CNAArgumentNullException else {
                    return XCTFail("expected CNAArgumentNullException, got \(error)")
                }
                XCTAssertEqual(failure.ParamName, "name")
            }
        }
    }

    /// Normalisation runs BEFORE the absoluteness test, so a name is judged on
    /// what it collapses to and not on what it was written as. `"./../x"` is
    /// refused because it becomes `"..\x"`, not because it contains a dot.
    func testANameIsJudgedAfterNormalisationNotBefore() {
        XCTAssertThrowsError(try Title.OpenStream("./../x")) { error in
            guard let failure = error as? CNAArgumentException else {
                return XCTFail("expected CNAArgumentException, got \(error)")
            }
            XCTAssertEqual(failure.Message,
                           Title.invalidTitleContainerNameMessage)
        }
        // A forward-slash absolute path reaches the same refusal, because
        // GetCleanPath turns "/" into "\" and a leading "\" is absolute.
        XCTAssertThrowsError(try Title.OpenStream("/etc/hostname")) { error in
            XCTAssertTrue(error is CNAArgumentException, "got \(error)")
        }
    }

    /// A refusal that happens without any runtime, which is the point: every
    /// managed test above is reached before `RuntimeRegistry.current()`, so a
    /// consumer outside a lifecycle callback still gets XNA's exception for a
    /// bad name rather than a capability failure.
    func testTheManagedRefusalsDoNotNeedARuntime() {
        XCTAssertThrowsError(try Title.OpenStream("..\\escape")) { error in
            XCTAssertTrue(error is CNAArgumentException,
                          "a bad name must not be reported as a missing runtime: \(error)")
        }
    }

    /// A well-formed name needs the runtime, and outside a callback there is
    /// none.
    func testAWellFormedNameOutsideTheLifecycleReportsTheMissingRuntime() {
        XCTAssertThrowsError(try Title.OpenStream("asset.txt")) { error in
            guard case CNAError.callbackOutsideGameLifecycle = error else {
                return XCTFail("expected callbackOutsideGameLifecycle, got \(error)")
            }
        }
    }

    // MARK: - The exception family this milestone admitted

    func testTheIOExceptionChainIsTheCLRChain() {
        let notFound = CNAFileNotFoundException(message: "m")
        XCTAssertTrue(notFound is CNAIOException)
        XCTAssertTrue(notFound is CNASystemException)
        XCTAssertTrue(notFound is CNAException)
        XCTAssertEqual(notFound.Message, "m")
        XCTAssertEqual(notFound.HResult, Int32(bitPattern: 0x8007_0002),
                       "COR_E_FILENOTFOUND is a Win32 facility code, not 0x8013…")
        XCTAssertNil(notFound.FileName,
                     "the projected constructors are exactly the ones that leave it null")
        XCTAssertEqual(CNAIOException().Message, "I/O error occurred.")
        XCTAssertEqual(CNAIOException().HResult, Int32(bitPattern: 0x8013_1620))
        XCTAssertEqual(CNAFileNotFoundException().Message, "Unable to find the specified file.")
    }

    func testTheTwoOpenStreamMessagesFormatTheNameTheyWereGiven() {
        XCTAssertEqual(Title.openStreamNotFoundMessage("a/b.txt"),
                       "Error loading \"a/b.txt\". File not found.")
        XCTAssertEqual(Title.openStreamErrorMessage("a/b.txt"),
                       "Error loading \"a/b.txt\". Cannot open file.")
    }

    // MARK: - The route, inside a lifecycle

    /// A missing asset reaches `CNA_RESULT_IO` and becomes XNA's
    /// `FileNotFoundException` carrying the formatted name.
    func testAMissingAssetInsideTheLifecycleRaisesFileNotFound() throws {
        let game = try TitleProbeGame()
        try game.Run()
        guard let error = game.missingAssetFailure else {
            return XCTFail("a missing asset did not fail")
        }
        guard let failure = error as? CNAFileNotFoundException else {
            return XCTFail("expected CNAFileNotFoundException, got \(error)")
        }
        XCTAssertEqual(failure.Message,
                       "Error loading \"definitely-absent-\(game.token).bin\". File not found.")
    }

    /// A file that IS there reads back byte for byte.
    ///
    /// The fixture is written next to the test executable, which is the title
    /// path CNA reports, and removed afterwards. It is project-controlled: no
    /// user document is read or written anywhere in this milestone.
    func testAPresentAssetReadsBackExactly() throws {
        let game = try TitleProbeGame(writeFixture: true)
        try game.Run()
        XCTAssertNil(game.presentAssetFailure,
                     "reading a fixture that exists failed: "
                     + String(describing: game.presentAssetFailure))
        XCTAssertEqual(game.readBack, Array(TitleProbeGame.fixtureBytes),
                       "the bytes handed back are the bytes on disk")
    }
}

private final class TitleProbeGame: Microsoft.Xna.Framework.Game {
    static let fixtureBytes = Array("cna-swift title fixture\u{0}\u{1}\u{2}".utf8)

    let token = UUID().uuidString.prefix(8).lowercased()
    let writeFixture: Bool
    var missingAssetFailure: Error?
    var presentAssetFailure: Error?
    var readBack: [UInt8] = []
    private var fixtureURL: URL?

    init(writeFixture: Bool = false) throws {
        self.writeFixture = writeFixture
        try super.init()
    }

    /// The title path CNA reports is the directory holding the running
    /// executable, measured in build-probe/f73_title.c.
    private var titleDirectory: URL {
        URL(fileURLWithPath: CommandLine.arguments[0]).deletingLastPathComponent()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        let missing = "definitely-absent-\(token).bin"
        do {
            _ = try Microsoft.Xna.Framework.TitleContainer.OpenStream(missing)
            missingAssetFailure = nil
        } catch {
            missingAssetFailure = error
        }

        if writeFixture {
            let name = "cna-fixture-\(token).bin"
            let url = titleDirectory.appendingPathComponent(name)
            fixtureURL = url
            do {
                try Data(TitleProbeGame.fixtureBytes).write(to: url)
                let stream = try Microsoft.Xna.Framework.TitleContainer.OpenStream(name)
                stream.open()
                defer { stream.close() }
                var buffer = [UInt8](repeating: 0, count: 64)
                let read = stream.read(&buffer, maxLength: buffer.count)
                readBack = read > 0 ? Array(buffer.prefix(read)) : []
            } catch {
                presentAssetFailure = error
            }
            try? FileManager.default.removeItem(at: url)
        }
        try Exit()
    }
}
