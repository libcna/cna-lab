package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.Consumer;
import java.util.function.IntConsumer;

/**
 * Receives the characters a keyboard layout actually produced.
 *
 * <p>A CNA extension. XNA's {@code Keyboard} reports which physical keys are down, which is the
 * wrong question for a name-entry field: it cannot tell an accented character from the dead key
 * that composed it, and it does not know the player's layout. This does.
 *
 * <p>Text input is off until {@link #Start()} is called, because a host with a screen keyboard
 * shows it while input is active. Characters arrive during {@code FrameworkDispatcher.Update},
 * which the game loop already pumps, so a listener runs on the game thread.
 *
 * <p>A character is delivered as an {@code int} code unit, not a {@code char}: a character
 * outside the basic plane arrives as two surrogate code units, exactly as CNA delivers it, so
 * nothing is lost or silently replaced.
 *
 * <p>Two further events describe what an input method is doing before anything is committed: the
 * draft {@link TextComposition} and the {@link TextCandidates} on offer. Both carry text, which
 * the numeric event record cannot hold, so they travel over their own transport -- but they are
 * ordered against the committed characters by a sequence CNA stamped when it raised them, so a
 * commit and the composition update that cleared it arrive in the order they happened.
 */
public final class TextInput {

    private static final int KIND_TEXT_INPUT = 30;

    private static final int TEXT_KIND_COMPOSITION = 0;
    private static final int TEXT_KIND_CANDIDATES = 1;

    private static final List<IntConsumer> LISTENERS = new CopyOnWriteArrayList<>();
    private static final List<Consumer<TextComposition>> COMPOSITION_LISTENERS =
            new CopyOnWriteArrayList<>();
    private static final List<Consumer<TextCandidates>> CANDIDATE_LISTENERS =
            new CopyOnWriteArrayList<>();

    static {
        GamerEventPump.addInputHandler(TextInput::dispatch);
        GamerEventPump.addTextHandler(TextInput::dispatchText);
    }

    private TextInput() {
    }

    /** Adds a listener for each code unit the host produces. */
    public static void addTextInputListener(IntConsumer listener) {
        LISTENERS.add(Objects.requireNonNull(listener, "listener"));
        subscribe();
    }

    public static void removeTextInputListener(IntConsumer listener) {
        LISTENERS.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Adds a listener for the draft text an input method is composing.
     *
     * <p>A composition update replaces the previous draft rather than adding to it, so a game
     * draws the latest one it was given and nothing else. An update with empty text means the
     * composition was cancelled or committed.
     *
     * @param listener called with each composition update, on the game thread
     */
    public static void addCompositionListener(Consumer<TextComposition> listener) {
        COMPOSITION_LISTENERS.add(Objects.requireNonNull(listener, "listener"));
        subscribeComposition();
    }

    /** Removes a composition listener. */
    public static void removeCompositionListener(Consumer<TextComposition> listener) {
        COMPOSITION_LISTENERS.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Adds a listener for the candidate list an input method is offering.
     *
     * @param listener called with each candidate-list update, on the game thread
     */
    public static void addCandidatesListener(Consumer<TextCandidates> listener) {
        CANDIDATE_LISTENERS.add(Objects.requireNonNull(listener, "listener"));
        subscribeComposition();
    }

    /** Removes a candidate-list listener. */
    public static void removeCandidatesListener(Consumer<TextCandidates> listener) {
        CANDIDATE_LISTENERS.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Raises the host's own committed-character event.
     *
     * <p>CNA's route, not a Java simulation. A code point above the basic plane is committed as
     * two calls -- a high surrogate then a low one -- which is how a real keyboard layout
     * delivers it, so a game exercising its text field sees exactly what it would see in play.
     *
     * @param codeUnit the UTF-16 code unit to report
     */
    public static void RaiseTypedCharacter(char codeUnit) {
        check("TextInput.RaiseTypedCharacter",
                NativeInputExtensionRoutes.textInputRaiseTextInputExt(game(), codeUnit));
    }

    /**
     * Raises the host's own composition event.
     *
     * <p>CNA's route, not a Java simulation: it dispatches the same event an input method
     * dispatches, which is how a game exercises its composition drawing on a machine with no IME
     * installed. CNA refuses text that is not valid UTF-8.
     *
     * @param text the draft composition
     * @param byteStart the active region's byte offset, which CNA forwards unchecked
     * @param byteLength the active region's byte length, which CNA forwards unchecked
     */
    public static void RaiseComposition(String text, int byteStart, int byteLength) {
        Objects.requireNonNull(text, "text");
        check("TextInput.RaiseComposition",
                NativeInputExtensionRoutes.textInputRaiseTextEditingExt(game(),
                        text.getBytes(StandardCharsets.UTF_8), byteStart, byteLength));
    }

    /**
     * Raises the host's own candidate-list event.
     *
     * @param candidates the words on offer
     * @param selectedIndex the pre-selected index, or -1 for none
     * @param horizontal whether the list is laid out horizontally
     */
    public static void RaiseCandidates(
            List<String> candidates, int selectedIndex, boolean horizontal) {
        Objects.requireNonNull(candidates, "candidates");
        byte[][] encoded = new byte[candidates.size()][];
        for (int index = 0; index < encoded.length; index++) {
            encoded[index] = Objects.requireNonNull(candidates.get(index), "candidate")
                    .getBytes(StandardCharsets.UTF_8);
        }
        check("TextInput.RaiseCandidates", NativeGamerServices.nativeRaiseTextCandidates(
                game(), encoded, selectedIndex, horizontal));
    }

    /** Starts plain text input. */
    public static void Start() {
        check("TextInput.Start",
                NativeInputExtensionRoutes.textInputStartExt(game()));
    }

    /** Starts text input of a stated kind, so a screen keyboard can match it. */
    public static void Start(TextInputType type) {
        check("TextInput.Start", NativeInputExtensionRoutes.textInputStartWithTypeExt(
                game(), Objects.requireNonNull(type, "type").ordinal()));
    }

    public static void Stop() {
        check("TextInput.Stop", NativeInputExtensionRoutes.textInputStopExt(game()));
    }

    public static boolean getIsActive() {
        boolean[] active = new boolean[1];
        check("TextInput.IsActive",
                NativeInputExtensionRoutes.textInputIsActiveExt(game(), active));
        return active[0];
    }

    public static boolean getIsScreenKeyboardShown() {
        boolean[] shown = new boolean[1];
        check("TextInput.IsScreenKeyboardShown",
                NativeInputExtensionRoutes.textInputIsScreenKeyboardShownExt(game(), shown));
        return shown[0];
    }

    /**
     * Returns the host window the text-input APIs are bound to.
     *
     * <p>A game with one window never needs this: CNA binds the game's own. A game that opens a
     * second window -- a tool's inspector, a level editor's preview -- has to say which one an
     * input method should attach to, and that is what this pair is for.
     *
     * @return the bound window, or {@code null} when none is bound
     */
    public static WindowHandle getWindowHandle() {
        long[] window = new long[1];
        check("TextInput.WindowHandle",
                NativeInputExtensionRoutes.textInputGetWindowHandleExt(game(), window));
        return NativeBindings.knownWindowHandle(window[0], "TextInput.WindowHandle");
    }

    /**
     * Binds the text-input APIs to a host window.
     *
     * @param window the window to bind, or {@code null} to unbind
     */
    public static void setWindowHandle(WindowHandle window) {
        check("TextInput.WindowHandle", NativeInputExtensionRoutes.textInputSetWindowHandleExt(
                game(), NativeBindings.knownWindowValue(window, "TextInput.WindowHandle")));
    }

    /**
     * Reports whether the screen keyboard is shown for one particular window.
     *
     * <p>Different from {@link #getIsScreenKeyboardShown()} in the case a game with two windows
     * has: that one asks about the bound window, and this one asks about the window named.
     *
     * @param window the window to ask about
     * @return whether a screen keyboard is shown for it
     */
    public static boolean getIsScreenKeyboardShown(WindowHandle window) {
        boolean[] shown = new boolean[1];
        check("TextInput.IsScreenKeyboardShown",
                NativeInputExtensionRoutes.textInputIsScreenKeyboardShownForWindowExt(game(),
                        NativeBindings.knownWindowValue(window, "TextInput.WindowHandle"),
                        shown));
        return shown[0];
    }

    /**
     * Tells the host where the text being edited is on screen.
     *
     * <p>A host that shows an input method window places it clear of this rectangle.
     */
    public static void setInputRectangle(Rectangle rectangle) {
        Objects.requireNonNull(rectangle, "rectangle");
        check("TextInput.setInputRectangle",
                NativeInputExtensionRoutes.textInputSetInputRectangleExt(game(),
                        new long[] {rectangle.X, rectangle.Y,
                            rectangle.Width, rectangle.Height}));
    }

    private static void subscribe() {
        NativeBindings.requireAvailable();
        GamerEventPump.ensureTextInputSubscribed();
    }

    private static void subscribeComposition() {
        NativeBindings.requireAvailable();
        GamerEventPump.ensureTextCompositionSubscribed();
    }

    /**
     * Turns one string event into its record and hands it to the listeners that want it.
     *
     * <p>The payload is UTF-8 the native side copied and freed before Java saw it, so decoding
     * is a plain {@code new String}: there is no pointer here, and no lifetime to respect.
     */
    private static void dispatchText(long kind, long start, long length, long selected,
            long horizontal, byte[][] payload) {
        if ((int) kind == TEXT_KIND_COMPOSITION) {
            String text = payload.length == 0
                    ? "" : new String(payload[0], StandardCharsets.UTF_8);
            TextComposition composition =
                    new TextComposition(text, (int) start, (int) length);
            for (Consumer<TextComposition> listener : COMPOSITION_LISTENERS) {
                listener.accept(composition);
            }
            return;
        }
        if ((int) kind != TEXT_KIND_CANDIDATES) {
            return;
        }
        List<String> candidates = new ArrayList<>(payload.length);
        for (byte[] candidate : payload) {
            candidates.add(new String(candidate, StandardCharsets.UTF_8));
        }
        TextCandidates event =
                new TextCandidates(candidates, (int) selected, horizontal != 0L);
        for (Consumer<TextCandidates> listener : CANDIDATE_LISTENERS) {
            listener.accept(event);
        }
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        if ((int) kind != KIND_TEXT_INPUT) {
            return;
        }
        for (IntConsumer listener : LISTENERS) {
            listener.accept((int) first);
        }
    }

    private static long game() {
        return InputExtension.game("TextInput");
    }

    private static void check(String operation, int result) {
        InputExtension.check(operation, result);
    }
}
