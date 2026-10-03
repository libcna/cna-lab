package org.openeggbert.cna.extensions.gamerservices;

/**
 * The on-screen keyboard a game asked for and now has to show.
 *
 * <p>Everything a prompt needs, so a game can draw the keyboard in its own art rather than
 * calling {@link GuideExtensions#Draw}. {@link #Text()} is what has been entered so far, and it
 * changes as the player types, so it is read again each frame rather than kept.
 *
 * @param Title the caption the game passed to {@code Guide.BeginShowKeyboardInput}
 * @param Description the longer prompt it passed
 * @param Text what has been entered so far, empty before anything is
 */
public record PendingKeyboardInput(String Title, String Description, String Text) {
}
