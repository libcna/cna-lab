/**
 * The host device capabilities CNA exposes and XNA 4.0 never had.
 *
 * <p>System information, host power, display metrics, the player's preferred locales, opening a
 * URL, the clipboard and controller vibration. XNA had none of it on the desktop -- the closest
 * it came was {@code Microsoft.Devices} on Windows Phone -- so it lives here rather than in
 * {@code Microsoft.Xna.Framework}.
 *
 * <p>Everything here needs a running game, because CNA reaches the host through the platform the
 * game created. Each capability reports whether the host supports it rather than pretending: a
 * headless platform has no vibration motor and says so.
 */
package org.openeggbert.cna.extensions.devices;
