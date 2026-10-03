// SPDX-License-Identifier: MIT

import CNAShim

internal final class CallbackContext {
    private(set) var pointer: UnsafeMutableRawPointer?

    init(runtime: RuntimeState) {
        pointer = Unmanaged.passRetained(runtime).toOpaque()
    }

    func releaseAfterNativeStopsCalling() {
        guard let pointer else { return }
        Unmanaged<RuntimeState>.fromOpaque(pointer).release()
        self.pointer = nil
    }
}

private enum LifecyclePhase {
    case initialize
    case loadContent
    case update
    case draw
    case unloadContent
    case exiting
    case beginRun
    case endRun
    case endDraw
}

private func dispatchLifecycle(
    context: UnsafeMutableRawPointer?,
    nativeGame: UInt64,
    nativeTime: UnsafePointer<CNASwift_GameTime>?,
    phase: LifecyclePhase
) -> UInt32 {
    guard let context else { return 9 }
    let runtime = Unmanaged<RuntimeState>.fromOpaque(context).takeUnretainedValue()
    do {
        try runtime.enterCallback(nativeGame: nativeGame)
        defer { runtime.leaveCallback() }
        guard let game = runtime.game else { throw CNAError.disposedObject("Game") }
        switch phase {
        case .initialize: try game.Initialize()
        case .loadContent:
            // The CNA host issues this immediately after `initialize`, and it
            // is the projection of the `LoadContent()` call XNA makes at the
            // end of `Game.Initialize()`. Assigning that one semantic
            // invocation to this one callback path is what keeps the
            // invariant: one XNA lifecycle occurrence, one virtual
            // invocation. It is why `Game.Initialize`'s base body does NOT
            // call `LoadContent` -- doing both would give a consumer two.
            //
            // Two differences remain, and both need `IGraphicsDeviceService`,
            // which is not projected, to close. They are recorded in
            // `Game.Initialize`'s documentation rather than half-reproduced
            // with a flag, because a flag that suppressed this callback would
            // also suppress a device-reset reload -- which XNA does issue,
            // through the handlers `HookDeviceEvents` installs, and which
            // this host's behaviour has not been measured for.
            try game.LoadContent()
        case .update: try game.Update(runtime.gameTime(from: nativeTime))
        case .draw: try game.Draw(runtime.gameTime(from: nativeTime))
        case .unloadContent: try game.UnloadContent()
        case .exiting:
            // Deliberately NOT `OnExiting`. CNA raises two different things
            // that both read as "exiting", and they are measured apart:
            //
            //   CNA_GameCallbacks.exiting  fires at cna_game_request_exit AND
            //                              again at cna_game_destroy for a
            //                              game that never ran;
            //   CNA_GAME_EVENT_EXITING     fires only at cna_game_request_exit.
            //
            // XNA raises `Exiting` when the game is exiting, and disposing a
            // game that never ran raises nothing. The event is therefore the
            // one that matches, and `Game` subscribes to it; this callback is
            // a teardown notification with no XNA counterpart, so it maps to
            // nothing rather than to the nearest-looking member. Mapping it
            // was harmless while `OnExiting`'s body was empty and became a
            // real divergence the moment the body raised the event -- which is
            // how it was found.
            break
        case .beginRun: try game.BeginRun()
        case .endRun: try game.EndRun()
        case .endDraw: try game.EndDraw()
        }
        return 0
    } catch {
        runtime.storeCallbackError(error)
        return 9
    }
}

internal let gameInitializeCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .initialize)
}

internal let gameLoadContentCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .loadContent)
}

internal let gameUpdateCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .update)
}

internal let gameDrawCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .draw)
}

internal let gameUnloadContentCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .unloadContent)
}

internal let gameExitingCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .exiting)
}

internal let gameBeginRunCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .beginRun)
}

internal let gameEndRunCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .endRun)
}

internal let gameEndDrawCallback: CNASwift_GameLifecycleCallback = { game, time, context, _ in
    dispatchLifecycle(context: context, nativeGame: game, nativeTime: time, phase: .endDraw)
}

internal let gameBeginDrawCallback: CNASwift_GameBeginDrawCallback = { game, _, context, shouldDraw, _ in
    guard let context else { return 9 }
    let runtime = Unmanaged<RuntimeState>.fromOpaque(context).takeUnretainedValue()
    do {
        try runtime.enterCallback(nativeGame: game)
        defer { runtime.leaveCallback() }
        guard let swiftGame = runtime.game else { throw CNAError.disposedObject("Game") }
        shouldDraw?.pointee = try swiftGame.BeginDraw() ? 1 : 0
        return 0
    } catch {
        runtime.storeCallbackError(error)
        return 9
    }
}
