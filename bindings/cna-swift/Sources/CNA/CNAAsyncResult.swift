// SPDX-License-Identifier: MIT

/// `System.IAsyncResult`.
///
/// Admitted at Foundation 102 through `tools/api_compat/bcl-authorities.json`
/// and pinned in `reference/bcl40-selected-shape.json`, where the extractor
/// confirms the four properties and their types.
///
/// **Three of the four are here.** The CLR interface also declares
/// `AsyncWaitHandle`, returning `System.Threading.WaitHandle` — 26 methods and
/// 2 properties over a `MarshalByRefObject` base. It is not admitted, so there
/// is no Swift type to spell the member with, and it is recorded as a missing
/// member rather than declared. That is a weaker position than
/// `Exception.StackTrace`, which is DECLARED and refused because its type is
/// `String` and can be spelled; a member whose very TYPE is unprojectable
/// cannot be declared at all. The distinction is worth keeping straight,
/// because the first draft of this file's own commit message got it wrong.
///
/// Not that a handle would be useful here: CNA's storage routes complete
/// inline, so `CompletedSynchronously` is true and there is nothing a caller
/// could wait for.
///
/// This is a LANGUAGE/BCL support type, not an XNA type: it lives outside
/// `Microsoft.Xna.Framework` and is counted in no XNA scoreboard.
public protocol CNAAsyncResult: AnyObject {

    /// `IAsyncResult.IsCompleted`.
    var IsCompleted: Bool { get }

    /// `IAsyncResult.CompletedSynchronously`.
    ///
    /// True for every result this binding produces. The CLR contract is that
    /// this reports whether the operation finished on the calling thread, and
    /// CNA's storage routes do exactly that.
    var CompletedSynchronously: Bool { get }

    /// `IAsyncResult.AsyncState`.
    ///
    /// Optional because the CLR property is `System.Object` and every Begin
    /// method takes the state a caller chose to pass, which is routinely null.
    var AsyncState: Any? { get }
}

/// `System.AsyncCallback`.
///
/// A delegate, so what is projected is its shape: one `IAsyncResult`, no
/// return. Optional at every use site because both `Begin` methods accept a
/// null callback, which is how a caller that intends to poll `IsCompleted`
/// or call `End` directly asks for no notification.
public typealias CNAAsyncCallback = (any CNAAsyncResult) -> Void
