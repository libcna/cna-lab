// SPDX-License-Identifier: MIT

/// The `IAsyncResult` every Storage `Begin` method returns.
///
/// **It is always already finished.** CNA's storage routes are synchronous --
/// `cna_storage_device_show_selector`'s own header says the completion
/// callback is "invoked before this call returns" -- so a `Begin` method here
/// performs the work, invokes the caller's callback, and hands back a result
/// that reports `IsCompleted` and `CompletedSynchronously`.
///
/// That is not this projection taking a shortcut. `CompletedSynchronously` is
/// in the CLR contract precisely so an operation that finishes on the calling
/// thread can say so, and a caller that polls or blocks is told the truth.
/// The alternative -- a thread that pretends to work -- would report a
/// concurrency this runtime does not have.
internal final class StorageAsyncResult: CNAAsyncResult {

    /// The handle the Begin call produced: a device or a container.
    let produced: UInt64

    /// What kind of Begin made it, so an End method can refuse a result that
    /// came from the other one rather than reading a handle of the wrong kind.
    let kind: Kind

    enum Kind { case device, container }

    let AsyncState: Any?

    init(produced: UInt64, kind: Kind, state: Any?) {
        self.produced = produced
        self.kind = kind
        self.AsyncState = state
    }

    var IsCompleted: Bool { true }

    var CompletedSynchronously: Bool { true }
}
