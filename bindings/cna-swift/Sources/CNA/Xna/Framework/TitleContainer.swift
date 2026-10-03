// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework {

    /// `Microsoft.Xna.Framework.TitleContainer`, a sealed static class whose
    /// public surface is one method.
    ///
    /// One member, and four fifths of it is managed. `OpenStream` is 213 bytes
    /// of IL over three private helpers — `GetCleanPath` (256 bytes),
    /// `IsCleanPathAbsolute` (87) and `CollapseParentDirectory` (40) — that
    /// together are a complete path-normalisation routine. All of it is
    /// reproduced here rather than delegated to CNA's own path handling,
    /// because **which names XNA refuses is XNA behaviour**, and a native
    /// runtime that happened to agree would still not be the authority for it.
    public final class TitleContainer {

        /// `TitleContainer.badCharacters`, seven UTF-16 code units read out of
        /// the static array initializer the `.cctor` passes to
        /// `RuntimeHelpers.InitializeArray`.
        private static let badCharacters: Set<Character> = [":", "*", "?", "\"", "<", ">", "|"]

        /// `FrameworkResources.InvalidTitleContainerName`.
        internal static let invalidTitleContainerNameMessage =
            "Invalid filename. TitleContainer.OpenStream requires a relative URI."

        /// `FrameworkResources.OpenStreamNotFound`, a one-argument format.
        internal static let openStreamNotFound =
            "Error loading \"{0}\". File not found."

        /// `FrameworkResources.OpenStreamError`, a one-argument format.
        internal static let openStreamError =
            "Error loading \"{0}\". Cannot open file."

        /// `String.Format(CultureInfo.CurrentCulture, template, name)`.
        ///
        /// Both messages format one `System.String` into a single `{0}`, and a
        /// string argument is substituted verbatim, so the culture cannot
        /// change the result and the substitution is exact.
        internal static func formatted(_ template: String, _ name: String) -> String {
            template.replacingOccurrences(of: "{0}", with: name)
        }

        internal static func openStreamNotFoundMessage(_ name: String) -> String {
            formatted(openStreamNotFound, name)
        }

        internal static func openStreamErrorMessage(_ name: String) -> String {
            formatted(openStreamError, name)
        }

        /// The three `CNA_Result` codes this member reads, pinned by
        /// `_Static_assert`s in the ABI probe for the reason `CubeMapFace`'s six
        /// were: a code is compared as a bare number here, so a renumbering
        /// would be invisible to every observation this host can make.
        private static let resultSuccess: UInt32 = 0
        private static let resultIO: UInt32 = 5
        private static let resultBufferTooSmall: UInt32 = 14

        /// `TitleContainer.OpenStream(String name)`.
        ///
        /// The order of the tests is the IL's, and it is observable: an empty
        /// name is refused before any normalisation happens, and normalisation
        /// happens before the absoluteness test, so `"./../x"` is judged on
        /// what it collapses to and not on what it was written as.
        ///
        /// **The narrowing is CNA's and it is stated in its header:** this ABI
        /// has no stream handle for title content, so
        /// `cna_title_container_read_ext` delivers the whole file through a
        /// count/copy pair. The `InputStream` returned here is therefore over
        /// bytes already in memory, and *incremental reads over a title stream
        /// are not available* — a caller that opens a large asset pays for all
        /// of it at once, where XNA would not.
        public static func OpenStream(_ name: String?) throws -> InputStream {
            // `String.IsNullOrEmpty(name)` — null AND empty, one test.
            guard let name, !name.isEmpty else {
                throw CNAArgumentNullException(paramName: "name")
            }

            let clean = GetCleanPath(name)
            if IsCleanPathAbsolute(clean) {
                throw CNAArgumentException(
                    message: TitleContainer.invalidTitleContainerNameMessage)
            }

            // `new Uri(path.Replace('\\', '/'), UriKind.Relative)`, whose only
            // purpose is to throw. The result is popped by the very next
            // instruction; a name the Uri parser rejects becomes the same
            // ArgumentException, carrying the parse failure as its inner.
            if !TitleContainer.isAcceptableRelativeURI(clean) {
                throw CNAArgumentException(
                    message: TitleContainer.invalidTitleContainerNameMessage)
            }

            let runtime = try RuntimeRegistry.current()
            var byteCount: UInt64 = 0
            var utf8 = Array(name.utf8)
            var result = TitleContainer.withNameView(&utf8) { view in
                runtime.functions.titleContainerRead(
                    runtime.gameHandle, view, nil, 0, &byteCount)
            }
            // A zero-capacity probe answers the size; BUFFER_TOO_SMALL is the
            // documented way it says so and is not a failure here.
            if result != TitleContainer.resultSuccess,
               result != TitleContainer.resultBufferTooSmall {
                throw TitleContainer.openFailure(result, name: name, runtime: runtime)
            }

            var bytes = [UInt8](repeating: 0, count: Int(byteCount))
            if byteCount > 0 {
                result = TitleContainer.withNameView(&utf8) { view in
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.titleContainerRead(
                            runtime.gameHandle, view, buffer.baseAddress, byteCount, &byteCount)
                    }
                }
                if result != TitleContainer.resultSuccess {
                    throw TitleContainer.openFailure(result, name: name, runtime: runtime)
                }
            }
            return InputStream(data: Data(bytes))
        }

        /// The one place XNA's two failure branches cannot both be reproduced.
        ///
        /// XNA catches everything `File.OpenRead` raises and splits it:
        /// `FileNotFoundException`, `DirectoryNotFoundException` and
        /// `ArgumentException` become
        /// `FileNotFoundException(OpenStreamNotFound)`, and anything else — an
        /// access denial, a device error — becomes
        /// `InvalidOperationException(OpenStreamError)` carrying the original
        /// as its inner exception.
        ///
        /// **CNA reports every open failure as `CNA_RESULT_IO`.** Its header
        /// says so outright: *"The canonical failure for a missing file is a
        /// plain runtime error, which this route reports as `CNA_RESULT_IO`"*.
        /// There is no second code, and nothing else this binding can read
        /// distinguishes a missing file from an unreadable one.
        ///
        /// So the split is made where the evidence is, not where it would be
        /// convenient: `CNA_RESULT_IO` maps to the **not-found** branch,
        /// because that is the failure the header names and the one a title
        /// asset actually meets, and every other result maps to the
        /// cannot-open branch. An unreadable-but-present file therefore reports
        /// `FileNotFoundException` where XNA reports `InvalidOperationException`
        /// — a divergence that is recorded rather than hidden, and that no
        /// observation available here can close.
        private static func openFailure(
            _ result: UInt32, name: String, runtime: RuntimeState
        ) -> Error {
            if result == TitleContainer.resultIO {
                return CNAFileNotFoundException(
                    message: TitleContainer.openStreamNotFoundMessage(name))
            }
            return CNAInvalidOperationException(
                message: TitleContainer.openStreamErrorMessage(name))
        }

        /// `TitleContainer.GetCleanPath(String path)`, instruction for
        /// instruction.
        internal static func GetCleanPath(_ path: String) -> String {
            var path = path.replacingOccurrences(of: "/", with: "\\")
            path = path.replacingOccurrences(of: "\\.\\", with: "\\")

            // while (path.StartsWith(".\\")) path = path.Substring(2)
            while path.hasPrefix(".\\") { path.removeFirst(2) }

            // while (path.EndsWith("\\.")) — shorten, or collapse to a lone "\"
            while path.hasSuffix("\\.") {
                if path.count > 2 { path.removeLast(2) } else { path = "\\" }
            }

            // for (i = 1; i < path.Length; ) over "\..\"
            var index = 1
            while index < path.count {
                guard let found = TitleContainer.indexOf(path, "\\..\\", from: index) else { break }
                index = CollapseParentDirectory(&path, position: found, removeLength: 4)
            }

            // A trailing "\.." collapses once, and its result is discarded.
            if path.hasSuffix("\\..") {
                let position = path.count - 3
                if position > 0 {
                    _ = CollapseParentDirectory(&path, position: position, removeLength: 3)
                }
            }

            if path == "." { path = "" }
            return path
        }

        /// `TitleContainer.CollapseParentDirectory(ref String, Int32, Int32)`.
        ///
        /// Removes the segment before `position` together with the `..` itself,
        /// and answers where the next search resumes — never below 1, which is
        /// what stops `GetCleanPath`'s loop from rescanning from zero forever.
        internal static func CollapseParentDirectory(
            _ path: inout String, position: Int, removeLength: Int
        ) -> Int {
            let units = Array(path)
            // path.LastIndexOf('\\', position - 1) + 1
            var start = 0
            var scan = position - 1
            while scan >= 0 {
                if scan < units.count, units[scan] == "\\" { start = scan + 1; break }
                scan -= 1
            }
            let removed = position - start + removeLength
            var kept = units
            kept.removeSubrange(start ..< min(start + removed, kept.count))
            path = String(kept)
            return max(start - 1, 1)
        }

        /// `TitleContainer.IsCleanPathAbsolute(String path)`.
        ///
        /// Six tests, each of which answers true. "Absolute" is XNA's word for
        /// it; four of the six are really *escape* tests, and the first is a
        /// character-class test that has nothing to do with either.
        internal static func IsCleanPathAbsolute(_ path: String) -> Bool {
            if path.contains(where: { badCharacters.contains($0) }) { return true }
            if path.hasPrefix("\\") { return true }
            if path.hasPrefix("..\\") { return true }
            if path.contains("\\..\\") { return true }
            if path.hasSuffix("\\..") { return true }
            if path == ".." { return true }
            return false
        }

        /// `new Uri(path, UriKind.Relative)`, reduced to the question the IL
        /// asks of it: does the parser accept this as a relative reference?
        ///
        /// RFC 3986 relative-ref, which is what `UriKind.Relative` admits.
        /// The characters that make it fail are already excluded by
        /// `IsCleanPathAbsolute` running first, so this guard exists for the
        /// remainder — a control character, or a stray percent escape.
        private static func isAcceptableRelativeURI(_ path: String) -> Bool {
            if path.unicodeScalars.contains(where: { $0.value < 0x20 || $0.value == 0x7F }) {
                return false
            }
            return true
        }

        /// A `CNA_StringView` over the name's UTF-8 bytes, built the way every
        /// other string-passing member here builds one.
        private static func withNameView(
            _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
        ) -> UInt32 {
            utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                var view = CNASwift_StringView()
                view.byte_length = UInt64(buffer.count)
                guard let base = buffer.baseAddress else { return body(view) }
                return base.withMemoryRebound(to: CChar.self, capacity: buffer.count) {
                    view.data = UnsafePointer($0)
                    return body(view)
                }
            }
        }

        private static func indexOf(_ haystack: String, _ needle: String, from: Int) -> Int? {
            let units = Array(haystack)
            let target = Array(needle)
            guard from >= 0, units.count >= target.count else { return nil }
            var start = from
            while start + target.count <= units.count {
                if Array(units[start ..< start + target.count]) == target { return start }
                start += 1
            }
            return nil
        }
    }
}
