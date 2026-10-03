// SPDX-License-Identifier: MIT

/// `System.IServiceProvider`.
///
/// The smallest selected BCL family: one method, and its whole contract is
/// "hand me a type, get an object or null back". It is admitted through
/// `tools/api_compat/bcl-authorities.json` and pinned in
/// `reference/bcl40-selected-shape.json`, where the extractor confirms the
/// arity this file depends on.
///
/// **Why it is a protocol rather than a concrete type.** `ContentManager`'s
/// two constructors declare this interface, not `GameServiceContainer`.
/// Spelling them over the one implementation this binding ships would compile
/// and would look right in every test here, and it would still narrow XNA's
/// contract: a consumer's own service provider could not be handed to a
/// `ContentManager` at all. The narrowing is invisible from inside the
/// binding, which is exactly why it is worth spending a protocol on.
///
/// This is a LANGUAGE/BCL support type, not an XNA type: it lives outside
/// `Microsoft.Xna.Framework` and is counted in no XNA scoreboard.
public protocol CNAServiceProvider: AnyObject {

    /// `GetService(Type serviceType)`.
    ///
    /// Optional because the CLR method returns `System.Object`, and every
    /// implementation of it answers null for a service it does not hold --
    /// including `GameServiceContainer`'s, whose IL tests `ContainsKey` before
    /// it reaches the indexer.
    func GetService(_ serviceType: Any.Type) -> Any?
}
