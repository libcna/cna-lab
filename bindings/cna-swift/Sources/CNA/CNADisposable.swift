// SPDX-License-Identifier: MIT

/// `System.IDisposable`.
///
/// Earlier milestones could project this interface structurally: every XNA
/// owner simply exposed its `Dispose()` member. `ContentManager.ReadAsset` is
/// the first selected signature that carries an `IDisposable` value in a
/// delegate, so the interface now needs a name a Swift caller can use.
///
/// This is BCL support, not an XNA namespace member.
public protocol CNADisposable: AnyObject {
    func Dispose() throws
}
