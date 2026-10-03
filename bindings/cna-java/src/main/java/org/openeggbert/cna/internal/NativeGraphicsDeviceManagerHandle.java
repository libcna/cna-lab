package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.GraphicsDeviceManager;

/** Owned JNI-side graphics-device-manager/context wrapper. */
public final class NativeGraphicsDeviceManagerHandle extends NativeHandle {

    NativeGraphicsDeviceManagerHandle(long value, GraphicsDeviceManager manager) {
        super(value, Ownership.OWNED,
                handle -> NativeBindings.destroyGraphicsDeviceManager(manager, handle));
    }
}
