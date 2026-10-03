package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Game;

/** Owned JNI-side game/context wrapper. */
public final class NativeGameHandle extends NativeHandle {

    NativeGameHandle(long value, Game game) {
        super(value, Ownership.OWNED, handle -> NativeBindings.destroyGame(game, handle));
    }
}
