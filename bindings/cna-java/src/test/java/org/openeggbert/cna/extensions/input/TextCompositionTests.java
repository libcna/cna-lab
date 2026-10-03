package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;
import java.util.function.IntConsumer;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Input-method composition and candidate lists, against the live runtime.
 *
 * <p>These are the events the fixed numeric record could never carry, so they travel over a
 * second transport that copies each string in the callback and frees it before Java sees it.
 * What the tests have to establish is that the text arrives intact -- including text no ASCII
 * assumption survives -- that it arrives in the order CNA raised it relative to committed
 * characters, and that nothing outlives its event.
 *
 * <p>No input method is installed on the qualification host. It is not needed: CNA exposes the
 * same dispatch an input method drives, so the events are real CNA events with real copies
 * behind them, raised deliberately instead of by a person typing.
 */
final class TextCompositionTests {

    /** Latin with diacritics, CJK, and a code point above the basic plane. */
    private static final String ACCENTED = "prílis žluťoučký kůň";
    private static final String CJK = "日本語入力";
    private static final String SUPPLEMENTARY = "a🎮b";

    @Test
    void aCompositionMapsTheHostsByteOffsetsToJavaIndices() {
        // CNA forwards the host's byte offsets into UTF-8 unchecked, and one character can be
        // one to four bytes and one or two Java chars. Nothing but walking the encoded form
        // gets this right, and getting it wrong would underline the wrong text.
        TextComposition composition = new TextComposition(CJK, 3, 6);
        assertEquals(1, composition.getStart(), "each of these characters is three UTF-8 bytes");
        assertEquals(3, composition.getEnd());
        assertEquals("本語", composition.getActiveText());

        TextComposition emoji = new TextComposition(SUPPLEMENTARY, 1, 4);
        assertEquals(1, emoji.getStart());
        assertEquals(3, emoji.getEnd(), "the code point above the basic plane is two Java chars");
        assertEquals(SUPPLEMENTARY.substring(1, 3), emoji.getActiveText());

        // An offset that lands inside a character names nothing, and saying so beats returning
        // a plausible neighbour.
        TextComposition inside = new TextComposition(CJK, 1, 3);
        assertEquals(-1, inside.getStart());
        assertNull(inside.getActiveText());
        assertEquals(-1, new TextComposition(CJK, 99, 1).getStart());
        assertEquals(-1, new TextComposition(CJK, 0, -1).getEnd());
        assertEquals(0, new TextComposition("", 0, 0).getStart());
        assertEquals("", new TextComposition("", 0, 0).getActiveText());
        assertEquals(ACCENTED.length(),
                new TextComposition(ACCENTED, ACCENTED.getBytes(StandardCharsets.UTF_8).length, 0)
                        .getStart());
    }

    @Test
    void aCandidateListReportsOnlyASelectionItActuallySent() {
        TextCandidates candidates = new TextCandidates(List.of("日本", "日本語"), 1, true);
        assertEquals("日本語", candidates.getSelected());
        assertNull(new TextCandidates(List.of("a"), -1, false).getSelected());
        // CNA passes the host's index through without range-checking it, so an index past the
        // end is possible and must not become an exception in a listener.
        assertNull(new TextCandidates(List.of("a"), 7, false).getSelected());
        List<String> mutable = new ArrayList<>(List.of("a"));
        TextCandidates copied = new TextCandidates(mutable, 0, false);
        mutable.clear();
        assertEquals(1, copied.Candidates().size(), "the record copied the list it was given");
        assertThrows(UnsupportedOperationException.class, () -> copied.Candidates().add("b"));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void compositionEventsReachJavaIntactAndInOrder() {
        try (Game game = new Game()) {
            CompositionProbe probe = new CompositionProbe(game);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class CompositionProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private CompositionProbe(Game game) {
            super(game);
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            List<TextComposition> compositions = new ArrayList<>();
            List<TextCandidates> candidates = new ArrayList<>();
            Consumer<TextComposition> onComposition = compositions::add;
            Consumer<TextCandidates> onCandidates = candidates::add;
            TextInput.addCompositionListener(onComposition);
            TextInput.addCandidatesListener(onCandidates);
            try {
                unicodeSurvivesTheTransport(compositions);
                orderingIsPreservedAcrossBothTransports(compositions);
                candidateListsArriveWhole(candidates);
                emptyAndUnselectedAreRealAnswers(compositions, candidates);
                listenersStopWhenRemoved(compositions);
                aFullQueueDropsTheOldestAndKeepsTheRestIntact(compositions);
                aThrowingListenerDoesNotSwallowTheEventsBehindIt(compositions);
            } finally {
                TextInput.removeCompositionListener(onComposition);
                TextInput.removeCandidatesListener(onCandidates);
            }

            // A queued event outliving its subscription must not reach a listener that is gone,
            // and must not strand the copy the callback made either.
            List<TextComposition> late = new ArrayList<>();
            Consumer<TextComposition> lateListener = late::add;
            TextInput.addCompositionListener(lateListener);
            TextInput.RaiseComposition("queued", 0, 0);
            TextInput.removeCompositionListener(lateListener);
            org.openeggbert.cna.internal.GamerEventPump.releaseTextComposition();
            FrameworkDispatcher.Update();
            assertEquals(List.of(), late,
                    "a removed listener receives nothing, even from an event already queued");

            // Re-subscribing after a release has to work, or a game that stopped listening
            // could never start again.
            List<TextComposition> resumed = new ArrayList<>();
            Consumer<TextComposition> resumedListener = resumed::add;
            TextInput.addCompositionListener(resumedListener);
            try {
                TextInput.RaiseComposition("again", 0, 5);
                FrameworkDispatcher.Update();
                assertEquals(1, resumed.size());
                assertEquals("again", resumed.get(0).Text());
            } finally {
                TextInput.removeCompositionListener(resumedListener);
            }

            assertThrows(NullPointerException.class,
                    () -> TextInput.addCompositionListener(null));
            assertThrows(NullPointerException.class,
                    () -> TextInput.addCandidatesListener(null));
            assertThrows(NullPointerException.class,
                    () -> TextInput.RaiseComposition(null, 0, 0));
            assertThrows(NullPointerException.class,
                    () -> TextInput.RaiseCandidates(null, 0, false));
        }

        private void unicodeSurvivesTheTransport(List<TextComposition> compositions) {
            compositions.clear();
            // Three texts no byte-per-character assumption survives. The copy is made inside
            // CNA's callback from a borrowed, non-terminated UTF-8 view; if the length were
            // taken as a character count or the terminator assumed, one of these would come
            // back truncated or with a replacement character.
            for (String text : List.of(ACCENTED, CJK, SUPPLEMENTARY)) {
                TextInput.RaiseComposition(text, 0, text.getBytes(StandardCharsets.UTF_8).length);
            }
            FrameworkDispatcher.Update();
            assertEquals(3, compositions.size());
            assertEquals(ACCENTED, compositions.get(0).Text());
            assertEquals(CJK, compositions.get(1).Text());
            assertEquals(SUPPLEMENTARY, compositions.get(2).Text());
            assertEquals(SUPPLEMENTARY, compositions.get(2).getActiveText(),
                    "the whole text is the active region here, surrogate pair and all");
        }

        private void orderingIsPreservedAcrossBothTransports(List<TextComposition> compositions) {
            compositions.clear();
            List<String> order = new ArrayList<>();
            IntConsumer typed = unit -> order.add("typed:" + (char) unit);
            Consumer<TextComposition> drafted = event -> order.add("draft:" + event.Text());
            TextInput.addTextInputListener(typed);
            TextInput.addCompositionListener(drafted);
            try {
                // A committed character and a composition update travel over two different
                // transports. They are one order to the game, so a shared sequence decides it.
                TextInput.RaiseComposition("draft one", 0, 0);
                TextInput.RaiseTypedCharacter('x');
                TextInput.RaiseComposition("draft two", 0, 0);
                TextInput.RaiseTypedCharacter('y');
                FrameworkDispatcher.Update();
                assertEquals(
                        List.of("draft:draft one", "typed:x", "draft:draft two", "typed:y"),
                        order,
                        "events from both transports arrive in the order CNA raised them");
            } finally {
                TextInput.removeTextInputListener(typed);
                TextInput.removeCompositionListener(drafted);
            }
        }

        private void candidateListsArriveWhole(List<TextCandidates> candidates) {
            candidates.clear();
            TextInput.RaiseCandidates(List.of("日本", "日本語", "にほん"), 1, true);
            FrameworkDispatcher.Update();
            assertEquals(1, candidates.size());
            TextCandidates event = candidates.get(0);
            assertEquals(List.of("日本", "日本語", "にほん"), event.Candidates());
            assertEquals(1, event.SelectedIndex());
            assertEquals("日本語", event.getSelected());
            assertTrue(event.Horizontal());
        }

        private void emptyAndUnselectedAreRealAnswers(
                List<TextComposition> compositions, List<TextCandidates> candidates) {
            compositions.clear();
            candidates.clear();
            // An empty composition is how a host says the draft was cancelled or committed,
            // so it must arrive as an event with empty text, not be dropped as nothing.
            TextInput.RaiseComposition("", 0, 0);
            TextInput.RaiseCandidates(List.of(), -1, false);
            FrameworkDispatcher.Update();
            assertEquals(1, compositions.size());
            assertEquals("", compositions.get(0).Text());
            assertEquals(1, candidates.size());
            assertEquals(List.of(), candidates.get(0).Candidates());
            assertNull(candidates.get(0).getSelected());
            assertNotNull(candidates.get(0).Candidates());
        }

        /**
         * Overruns the queue on purpose.
         *
         * <p>This is the control for the transport's memory. Each raise makes CNA build a
         * borrowed view over its own copy, which dies when the callback returns; the transport
         * has to make its own copy, and when the ring buffer is full it has to free what it
         * drops. If it stored the borrowed pointer instead, these texts would come back as
         * something other than what was raised, hundreds of allocations later. If it dropped
         * without freeing, this would leak a little on every run.
         */
        private void aFullQueueDropsTheOldestAndKeepsTheRestIntact(
                List<TextComposition> compositions) {
            compositions.clear();
            long droppedBefore = org.openeggbert.cna.internal.GamerEventPump.droppedEventCount();
            int raised = 400;
            for (int index = 0; index < raised; index++) {
                TextInput.RaiseComposition(CJK + "-" + index + "-" + ACCENTED, 0, 0);
            }
            FrameworkDispatcher.Update();
            long dropped = org.openeggbert.cna.internal.GamerEventPump.droppedEventCount()
                    - droppedBefore;
            assertTrue(dropped > 0, "a queue this small must have dropped something");
            assertEquals(raised, compositions.size() + dropped,
                    "every raised event is either delivered or counted as dropped");
            // The survivors are the newest, and each must be byte-for-byte what was raised.
            int first = raised - compositions.size();
            for (int index = 0; index < compositions.size(); index++) {
                assertEquals(CJK + "-" + (first + index) + "-" + ACCENTED,
                        compositions.get(index).Text(),
                        "a surviving composition must be exactly the text that was raised");
            }
        }

        /**
         * A listener that throws must not cost the events behind it.
         *
         * <p>The drain owns native copies that are already freed by the time a listener runs, so
         * the failure can be carried to the end without leaking; what must not happen is the
         * queue keeping events nobody will ever be given again.
         */
        private void aThrowingListenerDoesNotSwallowTheEventsBehindIt(
                List<TextComposition> compositions) {
            compositions.clear();
            Consumer<TextComposition> angry = event -> {
                throw new IllegalStateException("listener refused " + event.Text());
            };
            TextInput.addCompositionListener(angry);
            try {
                TextInput.RaiseComposition("first", 0, 0);
                TextInput.RaiseComposition("second", 0, 0);
                assertThrows(IllegalStateException.class, FrameworkDispatcher::Update);
            } finally {
                TextInput.removeCompositionListener(angry);
            }
            assertEquals(List.of("first", "second"),
                    compositions.stream().map(TextComposition::Text).toList(),
                    "the well-behaved listener still saw both events");
            TextInput.RaiseComposition("after", 0, 0);
            FrameworkDispatcher.Update();
            assertEquals(3, compositions.size(), "the queue kept working after the failure");
        }

        private void listenersStopWhenRemoved(List<TextComposition> compositions) {
            compositions.clear();
            Consumer<TextComposition> second = event -> { };
            TextInput.addCompositionListener(second);
            TextInput.RaiseComposition("both", 0, 0);
            FrameworkDispatcher.Update();
            assertEquals(1, compositions.size(), "two listeners, one event each");
            TextInput.removeCompositionListener(second);
            TextInput.RaiseComposition("one", 0, 0);
            FrameworkDispatcher.Update();
            assertEquals(2, compositions.size());
        }
    }
}
