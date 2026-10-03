package org.openeggbert.cna.extensions.gamerservices;

import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * The half of XNA's {@code Guide} that CNA hands back to the game.
 *
 * <p>A CNA extension. {@code Guide.BeginShowKeyboardInput} and {@code Guide.BeginShowMessageBox}
 * are XNA members and stay where they are; what XNA does not say -- because on an Xbox it never
 * had to -- is what happens when the platform has no system overlay to draw them. CNA's answer is
 * that the game finds out something is pending and shows it, and that question has no XNA member
 * at all.
 *
 * <p>A game has two ways to show it. It can read the pending request and draw the prompt in its
 * own art, which is what most games want, and then report what the player did with
 * {@link #CancelKeyboardInput()} or {@link #ClickMessageBox(int)}. Or it can call {@link #Draw}
 * and let CNA draw a plain one over the surfaces the game supplies.
 *
 * <p>Until the outcome is reported the request stays pending and the game's
 * {@code EndShowKeyboardInput} has nothing to return, which is the same shape as XNA: the
 * asynchronous operation completes when the player answers.
 */
public final class GuideExtensions {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_NOT_SUPPORTED = 6;

    private GuideExtensions() {
    }

    /**
     * Returns the keyboard input waiting to be shown.
     *
     * <p>Read each frame while it is non-null: {@link PendingKeyboardInput#Text()} changes as the
     * player types.
     *
     * @return the pending request, or {@code null} when nothing is waiting
     */
    public static PendingKeyboardInput getPendingKeyboardInput() {
        NativeBindings.requireAvailable();
        boolean[] pending = new boolean[1];
        check("getPendingKeyboardInput",
                NativeGamerServicesRoutes.guideGetHasPendingKeyboardInputExt(pending));
        if (!pending[0]) {
            return null;
        }
        return new PendingKeyboardInput(
                text("title",
                        NativeGamerServicesRoutes::guideGetPendingKeyboardInputTitleSizeExt,
                        NativeGamerServicesRoutes::guideCopyPendingKeyboardInputTitleExt),
                text("description",
                        NativeGamerServicesRoutes::guideGetPendingKeyboardInputDescriptionSizeExt,
                        NativeGamerServicesRoutes::guideCopyPendingKeyboardInputDescriptionExt),
                text("text",
                        NativeGamerServicesRoutes::guideGetPendingKeyboardInputDisplayTextSizeExt,
                        NativeGamerServicesRoutes::guideCopyPendingKeyboardInputDisplayTextExt));
    }

    /**
     * Returns the message box waiting to be shown.
     *
     * @return the pending box, or {@code null} when nothing is waiting
     */
    public static PendingMessageBox getPendingMessageBox() {
        NativeBindings.requireAvailable();
        boolean[] pending = new boolean[1];
        check("getPendingMessageBox",
                NativeGamerServicesRoutes.guideGetHasPendingMessageBoxExt(pending));
        if (!pending[0]) {
            return null;
        }
        int[] focus = new int[1];
        check("getPendingMessageBox",
                NativeGamerServicesRoutes.guideGetPendingMessageBoxFocusButtonExt(focus));
        return new PendingMessageBox(focus[0]);
    }

    /**
     * Draws whichever request is pending, in CNA's own plain style.
     *
     * <p>For a game that has no prompt art of its own yet. It draws nothing when nothing is
     * pending, so it is safe to call unconditionally inside a sprite batch.
     *
     * @param spriteBatch the batch to draw into, already begun
     * @param font the font to draw the captions with
     * @param whitePixel a one-pixel white texture, which CNA stretches for the panels
     */
    public static void Draw(SpriteBatch spriteBatch, SpriteFont font, Texture2D whitePixel) {
        Objects.requireNonNull(spriteBatch, "spriteBatch");
        Objects.requireNonNull(font, "font");
        Objects.requireNonNull(whitePixel, "whitePixel");
        long device = NativeBindings.nativeGraphicsDeviceValue(whitePixel.getGraphicsDevice());
        long batch = NativeBindings.nativeResourceHandle(spriteBatch);
        long fontHandle = NativeBindings.nativeSpriteFontHandle(font);
        long texture = NativeBindings.nativeResourceHandle(whitePixel);
        check("Draw", NativeGamerServicesRoutes.guideRenderPendingKeyboardInputExt(
                device, batch, fontHandle, texture));
        check("Draw", NativeGamerServicesRoutes.guideRenderPendingMessageBoxExt(
                device, batch, fontHandle, texture));
    }

    /**
     * Reports that the player cancelled the keyboard input.
     *
     * <p>This completes the pending operation, so {@code Guide.EndShowKeyboardInput} then reports
     * the cancellation as XNA does. There is no matching accept: CNA collects the typed text
     * itself from the host's own text input, which is what {@link PendingKeyboardInput#Text()}
     * reads back.
     */
    public static void CancelKeyboardInput() {
        NativeBindings.requireAvailable();
        check("CancelKeyboardInput",
                NativeGamerServicesRoutes.guideSimulateKeyboardInputCancelExt());
    }

    /**
     * Reports which button the player chose.
     *
     * <p>This completes the pending operation, so {@code Guide.EndShowMessageBox} then reports
     * the choice.
     *
     * @param buttonIndex the index of the chosen button, in the order the captions were passed
     */
    public static void ClickMessageBox(int buttonIndex) {
        NativeBindings.requireAvailable();
        check("ClickMessageBox",
                NativeGamerServicesRoutes.guideSimulateMessageBoxClickExt(buttonIndex));
    }

    /**
     * Drops the pending keyboard input without completing it.
     *
     * <p>Not the same as {@link #CancelKeyboardInput()}: cancelling is an answer the waiting
     * operation receives, and this leaves it unanswered. It is what a game does when it is
     * tearing down a screen rather than when a player pressed something.
     */
    public static void DiscardKeyboardInput() {
        NativeBindings.requireAvailable();
        check("DiscardKeyboardInput",
                NativeGamerServicesRoutes.guideResetPendingKeyboardInputExt());
    }

    /** Drops the pending message box without completing it. See {@link #DiscardKeyboardInput()}. */
    public static void DiscardMessageBox() {
        NativeBindings.requireAvailable();
        check("DiscardMessageBox",
                NativeGamerServicesRoutes.guideResetPendingMessageBoxExt());
    }

    /**
     * Asks the host to show its achievements pane.
     *
     * <p>XNA has {@code Guide.ShowGamerCard} and {@code Guide.ShowMarketplace} but never an
     * achievements pane, which is why this is here rather than beside them.
     *
     * @param player whose achievements to show
     */
    public static void ShowAchievements(PlayerIndex player) {
        NativeBindings.requireAvailable();
        Objects.requireNonNull(player, "player");
        check("ShowAchievements",
                NativeGamerServicesRoutes.guideShowAchievementsExt(player.ordinal()));
    }

    private interface SizeRoute {
        int read(long[] outBytes);
    }

    private interface CopyRoute {
        int read(byte[] destination, long[] outBytes);
    }

    private static String text(String what, SizeRoute size, CopyRoute copy) {
        long[] bytes = new long[1];
        check(what, size.read(bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check(what, copy.read(destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            throw new GuideNotSupportedException("GuideExtensions." + operation
                    + " is not supported by this CNA build");
        }
        throw NativeBindings.failure("GuideExtensions." + operation, result);
    }
}
