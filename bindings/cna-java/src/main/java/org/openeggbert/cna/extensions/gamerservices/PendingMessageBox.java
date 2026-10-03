package org.openeggbert.cna.extensions.gamerservices;

/**
 * The message box a game asked for and now has to show.
 *
 * <p>The captions and the icon are the ones the game itself passed to
 * {@code Guide.BeginShowMessageBox}, so they are not repeated here; what the game does not
 * otherwise know is which button the host wants focused.
 *
 * @param FocusButton the index of the button that should start focused
 */
public record PendingMessageBox(int FocusButton) {
}
