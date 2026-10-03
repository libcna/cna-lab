// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.OcclusionQuery`.
    ///
    /// A `GraphicsResource` that counts the pixels a batch of draws actually
    /// wrote. `Begin`, draw, `End`, then wait for `IsComplete` and read
    /// `PixelCount` -- the loop XNA's own documentation writes.
    ///
    /// **Unsealed in the metadata**, so unsealed here, and `Dispose(Boolean)`
    /// is its one override point.
    public class OcclusionQuery: GraphicsResource {

        /// `OcclusionQuery..ctor(GraphicsDevice graphicsDevice)`.
        ///
        /// CNA answers `CNA_RESULT_NOT_SUPPORTED` on a backend with no query
        /// object, and that refusal is passed through rather than swallowed: a
        /// query that cannot count is not a query, and a consumer that gets one
        /// back would read zeros and believe them.
        public init(graphicsDevice: GraphicsDevice) throws {
            let deviceHandle = try graphicsDevice.validatedHandle("OcclusionQuery.init")
            let runtime = graphicsDevice.runtimeState

            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.occlusionQueryCreate(deviceHandle, &handle),
                operation: "cna_occlusion_query_create")

            super.init(
                storage: NativeHandleStorage(
                    handle: handle,
                    typeName: "OcclusionQuery",
                    ownership: .owned,
                    runtime: runtime,
                    destroy: runtime.functions.occlusionQueryDestroy),
                device: graphicsDevice)
            // CNA refuses to destroy a game while any owned child survives, so
            // the query joins the runtime's registry exactly as every other
            // owned GraphicsResource does. Without this line a consumer who
            // forgets one line turns game teardown into a failure.
            runtime.register(self)
        }

        /// `OcclusionQuery.Begin()`.
        ///
        /// Every draw between this and `End` is counted.
        public func Begin() throws {
            let handle = try validatedHandle("OcclusionQuery.Begin")
            // XNA refuses a second Begin until the previous result has been
            // *looked at*: reading IsComplete is what arms the next one. CNA
            // makes no such check, so the flag is the binding's own -- and it
            // is what stops a caller silently discarding a query they are
            // still waiting on.
            guard !awaitingCompletionCheck else {
                throw CNAInvalidOperationException(
                    message: OcclusionQuery.isCompleteMustBeCalledMessage)
            }
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.occlusionQueryBegin(handle),
                operation: "cna_occlusion_query_begin")
        }

        /// `OcclusionQuery.End()`.
        ///
        /// Ends the query and submits it; the result becomes readable later,
        /// which is the whole reason `IsComplete` exists.
        public func End() throws {
            let handle = try validatedHandle("OcclusionQuery.End")
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.occlusionQueryEnd(handle),
                operation: "cna_occlusion_query_end")
            awaitingCompletionCheck = true
        }

        /// XNA's rearm rule: `End` sets it, reading `IsComplete` clears it.
        private var awaitingCompletionCheck = false

        /// `OcclusionQuery.IsComplete`.
        ///
        /// `IL_NO_FAILURE_PATH`, so it cannot refuse -- and the route behind it
        /// can. **A failed read answers `true`, and that is a deliberate
        /// choice rather than an omission.**
        ///
        /// The member exists to be spun on: `while !query.IsComplete { }` is
        /// the loop XNA's documentation writes. Answering `false` on a route
        /// that keeps failing would hang that loop forever with nothing to
        /// report. Answering `true` ends it and hands the question to
        /// `PixelCount`, which is `IL_DIRECT_THROW` and *can* say what went
        /// wrong. An infallible getter should not be the member that traps a
        /// caller; it should defer to the one that can explain.
        ///
        /// The failure is not discarded: it is kept in `lastStatusFailure` so a
        /// test can assert it, and `PixelCount` will raise its own.
        public var IsComplete: Bool {
            // A disposed query takes the same road: it answers true so the
            // wait ends, and it records WHY, because a promise to keep the
            // failure is worth nothing if one of the two paths drops it.
            let handle: UInt64
            do {
                handle = try validatedHandle("OcclusionQuery.IsComplete")
            } catch {
                lastStatusFailure = error
                return true
            }
            var value: UInt8 = 0
            let result = nativeStorage.runtime.functions.occlusionQueryGetIsComplete(
                handle, &value)
            guard result == 0 else {
                lastStatusFailure = CNAError.nativeFailure(
                    operation: "cna_occlusion_query_get_is_complete",
                    result: result,
                    message: "the query could not report whether it has finished")
                return true
            }
            lastStatusFailure = nil
            awaitingCompletionCheck = false
            return value != 0
        }

        /// `FrameworkResources.IsCompleteMustBeCalled`, read out of the
        /// registered `Microsoft.Xna.Framework.dll`.
        internal static let isCompleteMustBeCalledMessage =
            "Begin may not be called on this query object again before "
            + "IsComplete has been checked."

        /// `FrameworkResources.DataNotAvailable`.
        internal static let dataNotAvailableMessage =
            "The query data is not yet available. Use the IsComplete property "
            + "to determine if the data is available before attempting to "
            + "retrieve it."

        /// The error `IsComplete` could not raise, kept for the test that
        /// proves the divergence above is real rather than described.
        internal private(set) var lastStatusFailure: Error?

        /// `OcclusionQuery.Dispose(Boolean)`.
        ///
        /// **Declared, not inherited.** XNA's `OcclusionQuery` declares its own
        /// override -- it releases the native query object before forwarding --
        /// and a consumer subclassing this type overrides *this* member. Here
        /// the handle release belongs to `NativeHandleStorage`, which the base
        /// already drives, so the body is the forward and the value is the
        /// declaration: it is the override point XNA gives, in the place XNA
        /// gives it.
        ///
        /// Swift has no `protected`, so this is public, as every other
        /// `Dispose(Boolean)` in this binding is.
        public override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        /// `OcclusionQuery.PixelCount`.
        ///
        /// `IL_DIRECT_THROW` with `InvalidOperationException`: XNA raises when
        /// the count is read before the query has finished. So this is a
        /// throwing getter, and it is where a failure `IsComplete` had to
        /// swallow finally surfaces.
        public var PixelCount: Int32 {
            get throws {
                let handle = try validatedHandle("OcclusionQuery.PixelCount")
                // **Measured: CNA does not refuse this and XNA does.** A query
                // that was never begun answers a count on this backend --
                // `cna_occlusion_query_get_pixel_count` returned success for a
                // query with no Begin/End at all. XNA's getter is
                // IL_DIRECT_THROW with InvalidOperationException, so the
                // refusal is reproduced here rather than left to the runtime,
                // which is the same managed half every argument check in this
                // binding is.
                guard IsComplete else {
                    throw CNAInvalidOperationException(
                        message: OcclusionQuery.dataNotAvailableMessage)
                }
                var value: Int32 = 0
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.occlusionQueryGetPixelCount(
                        handle, &value),
                    operation: "cna_occlusion_query_get_pixel_count")
                return value
            }
        }
    }
}
