/**
 * Input CNA has and XNA 4.0 never did: typed text and the mouse cursor.
 *
 * <p>XNA's desktop input is three snapshots -- keyboard, mouse, gamepad -- and one Xbox-only
 * on-screen keyboard through the Guide. It has no way to receive the character a keyboard layout
 * actually produced, and no way to change the cursor. Both live here rather than in
 * {@code Microsoft.Xna.Framework.Input}, which stays a faithful XNA projection.
 */
package org.openeggbert.cna.extensions.input;
