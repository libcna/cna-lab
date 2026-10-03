// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework {
    /// `Microsoft.Xna.Framework.LaunchParameters`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Game.dll` as a public **unsealed**
    /// class whose direct base is
    /// `System.Collections.Generic.Dictionary<System.String, System.String>`,
    /// declaring **one parameterless constructor** as its only public member.
    /// Every usable member — `Count`, the indexer, `Add`, `Remove`, `Clear`,
    /// `ContainsKey`, `ContainsValue`, `TryGetValue`, `Keys`, `Values`,
    /// `Comparer`, `GetEnumerator` — is inherited.
    ///
    /// That is why the base is a real Swift superclass. `CNADictionary` is the
    /// measured projection of the `Dictionary<TKey,TValue>` shape pinned in
    /// `tools/api_compat/reference/bcl40-selected-shape.json`, and the CLR
    /// generic arguments are preserved in the specialization: the base is
    /// `Dictionary<string, string>`, so the Swift superclass is
    /// `CNADictionary<String, String>` and never `CNADictionary<Any, Any>` or
    /// a flattened Swift `[String: String]`.
    ///
    /// A Swift dictionary would have been the obvious shortcut and is wrong in
    /// three ways at once: it is a value, so a caller would get a copy of the
    /// launch data rather than the object the `Game` owns; it has no `Add`
    /// that refuses a duplicate key; and it has no defined enumeration order,
    /// where the CLR's is insertion order.
    ///
    /// ## The constructor parses the command line
    ///
    /// The constructor is **not** a bare `base..ctor()`: the IL is
    ///
    /// ```text
    /// base..ctor()
    /// ParseCommandLineArguments(Environment.GetCommandLineArgs())
    /// ```
    ///
    /// so a `LaunchParameters` populates itself from the process it is running
    /// in. Reproducing that is what makes the type useful rather than
    /// decorative, and `CommandLine.arguments` is the exact analogue of
    /// `GetCommandLineArgs()`: element zero is the executable and the rest are
    /// the arguments, which is why the parse starts at index one.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class LaunchParameters: CNADictionary<String, String> {
        /// `.ctor()`.
        ///
        /// Reads this process's own arguments, exactly as XNA reads the
        /// arguments of the process the game is running in. It cannot fail:
        /// every insertion is guarded by `ContainsKey`, so the inherited
        /// duplicate-key refusal is unreachable from here.
        public override init() {
            super.init()
            ParseCommandLineArguments(CommandLine.arguments)
        }

        /// `internal void ParseCommandLineArguments(string[] args)`.
        ///
        /// `assembly`-visible in the IL, so it is `internal` here and is not a
        /// projected XNA identity — but it is the whole behaviour of the
        /// public constructor, which is why it is a named method rather than
        /// inline code: the constructor's behaviour is then testable against a
        /// supplied argument vector instead of only against whichever process
        /// happens to be running the tests.
        ///
        /// The IL, exactly:
        ///
        /// ```text
        /// separators = { '/', '-' }
        /// if (args.Length <= 1) return          // element 0 is the executable
        /// for (i = 1; i < args.Length; i++) {
        ///     argument = args[i].TrimStart(separators)
        ///     ParseKeyValuePair(argument, out key, out value)
        ///     if (!ContainsKey(key) && key != "") Add(key, value)
        /// }
        /// ```
        ///
        /// Three details a plausible reimplementation gets wrong, each with a
        /// test: the executable is skipped, **the first occurrence of a
        /// repeated key wins** rather than the last, and an argument that
        /// trims away to nothing is dropped instead of becoming an empty key.
        internal func ParseCommandLineArguments(_ args: [String]) {
            guard args.count > 1 else { return }
            for argument in args.dropFirst() {
                let trimmed = argument.drop {
                    $0 == LaunchParameters.slashSeparator ||
                    $0 == LaunchParameters.dashSeparator
                }
                let pair = LaunchParameters.parseKeyValuePair(String(trimmed))
                if !ContainsKey(pair.key), !pair.key.isEmpty {
                    // `Add` here is guarded by the same `ContainsKey` the IL
                    // guards it with, so the duplicate refusal is unreachable
                    // and the setter -- which cannot fail at all -- is the
                    // identical operation on a key that is known absent.
                    SetItem(pair.key, pair.value)
                }
            }
        }

        /// The two characters `TrimStart` removes, `'/'` and `'-'`.
        private static let slashSeparator: Character = "/"
        private static let dashSeparator: Character = "-"

        /// `private void ParseKeyValuePair(string argument, out string key, out string value)`.
        ///
        /// ```text
        /// key = argument; value = ""
        /// colon = argument.IndexOf(':')
        /// if (colon == -1) return
        /// key   = argument.Substring(0, colon)
        /// value = argument.Substring(colon + 1)
        /// ```
        ///
        /// So an argument with no colon becomes a key with an **empty** value,
        /// not a missing one, and only the FIRST colon splits: `a:b:c` is the
        /// key `a` and the value `b:c`.
        private static func parseKeyValuePair(
            _ argument: String
        ) -> (key: String, value: String) {
            guard let colon = argument.firstIndex(of: ":") else {
                return (argument, "")
            }
            return (
                String(argument[argument.startIndex..<colon]),
                String(argument[argument.index(after: colon)...])
            )
        }
    }
}
