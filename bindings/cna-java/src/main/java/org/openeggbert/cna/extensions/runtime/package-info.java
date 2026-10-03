/**
 * What CNA can tell a game about the runtime it is running on, and CNA's logger.
 *
 * <p>Nothing here is part of Microsoft XNA 4.0. XNA never told a game which renderer it got, how
 * mature that backend is, or which platform family it was on, and it had no logging API at all.
 * That is why this lives outside {@code Microsoft.Xna.Framework}: the strict packages stay a
 * faithful XNA projection.
 *
 * <p>Everything here is compile-time or process-wide CNA state. None of it needs a graphics
 * device, a window or a running game, so a title can ask before it creates any of those.
 */
package org.openeggbert.cna.extensions.runtime;
