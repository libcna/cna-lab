// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    /// `open`, not `final`: XNA leaves `GameTime` derivable, and nothing
    /// about it is native — it is three stored values with three public
    /// constructors, so a subclass costs the projection nothing and refusing
    /// one would strengthen a contract XNA left open.
    open class GameTime {
        public let TotalGameTime: Duration
        public let ElapsedGameTime: Duration
        public let IsRunningSlowly: Bool

        public init() {
            TotalGameTime = .zero
            ElapsedGameTime = .zero
            IsRunningSlowly = false
        }

        public init(totalGameTime: Duration, elapsedGameTime: Duration) {
            TotalGameTime = totalGameTime
            ElapsedGameTime = elapsedGameTime
            IsRunningSlowly = false
        }

        public init(totalGameTime: Duration, elapsedGameTime: Duration, isRunningSlowly: Bool) {
            TotalGameTime = totalGameTime
            ElapsedGameTime = elapsedGameTime
            IsRunningSlowly = isRunningSlowly
        }

        internal convenience init(totalTicks: Int64, elapsedTicks: Int64, runningSlowly: Bool) {
            self.init(
                totalGameTime: Self.duration(fromTicks: totalTicks),
                elapsedGameTime: Self.duration(fromTicks: elapsedTicks),
                isRunningSlowly: runningSlowly
            )
        }

        private static func duration(fromTicks ticks: Int64) -> Duration {
            let ticksPerSecond: Int64 = 10_000_000
            let seconds = ticks / ticksPerSecond
            let remainder = ticks % ticksPerSecond
            return Duration(
                secondsComponent: seconds,
                attosecondsComponent: remainder * 100_000_000_000
            )
        }
    }
}
