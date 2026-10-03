// FrameworkDispatcher — Microsoft.Xna.Framework.FrameworkDispatcher.

extension Microsoft.Xna.Framework {

    /// `Microsoft.Xna.Framework.FrameworkDispatcher`, a sealed static class
    /// whose whole public surface is one method.
    ///
    /// XNA's `Update` is 290 bytes of IL and **throws nothing**. It calls
    /// `PollForEvents`, then drains a lock-guarded `List<ManagedCallAndArg>`
    /// under `Monitor.Enter`/`Exit`, dispatching each queued call to
    /// `MediaPlayer.OnActiveSongChanged`, `MediaPlayer.OnMediaStateChanged`,
    /// `Microphone.AllMicrophones`, `DynamicSoundEffectInstance
    /// .RaiseBufferNeededOnInstance` and `FrameworkCallbackLinker
    /// .OnStorageDeviceChanged`. Every one of those is work the native runtime
    /// owns; none of it is a managed decision this binding could reproduce, and
    /// the queue itself is `private static`.
    ///
    /// So this member is a forward, and the whole of what it promises is that
    /// the pump ran.
    public final class FrameworkDispatcher {

        /// `FrameworkDispatcher.Update()`.
        ///
        /// XNA's is static and takes nothing. `cna_framework_dispatcher_update`
        /// takes a game handle, and the header says why: *"The canonical
        /// dispatcher is static and exists for applications that do not run the
        /// game loop; a game handle is taken here only for thread affinity."*
        /// The handle is therefore not a parameter this member should expose --
        /// it is the runtime's, and it is fetched the way every other static
        /// XNA member fetches it.
        ///
        /// Calling this while the game loop runs is documented as harmless and
        /// doing the work twice, which is also XNA's own behaviour: `Game.Tick`
        /// calls `Update` and a caller may call it again.
        public static func Update() throws {
            let runtime = try RuntimeRegistry.current()
            try runtime.functions.check(
                runtime.functions.frameworkDispatcherUpdate(runtime.gameHandle),
                operation: "cna_framework_dispatcher_update"
            )
        }
    }
}
