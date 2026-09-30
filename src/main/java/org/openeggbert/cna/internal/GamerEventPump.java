package org.openeggbert.cna.internal;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Delivers CNA's gamer-service and network-session events to the Java types that own them.
 *
 * <p>CNA may raise these from any thread, so the JNI callbacks record them rather than calling
 * into the JVM. Java drains the record immediately after pumping the dispatcher or the session,
 * which puts every event on the game thread during {@code Update} -- where XNA raises it too.
 *
 * <p>The gamer and session handlers are installed by the packages that own the listeners:
 * {@code Microsoft.Xna.Framework.GamerServices} and {@code Microsoft.Xna.Framework.Net}. CLR
 * reaches across those namespaces through assembly-internal access, which Java has no
 * equivalent for. Installing at class initialization is safe because a handler is only needed
 * once one of that package's types exists.
 *
 * <p>The input range is shared: several CNA extension families raise events there, so it takes a
 * list of handlers and each filters on the kinds it owns.
 *
 * <p>This class is not application API.
 */
public final class GamerEventPump {

    /** The lowest event kind that belongs to a network session rather than to gamer services. */
    private static final int FIRST_SESSION_KIND = 10;

    /** The lowest event kind that belongs to the input extensions. */
    private static final int FIRST_INPUT_KIND = 30;

    private static volatile Handler gamerHandler;
    private static volatile Handler sessionHandler;
    // Several extension families share the input kind range, and each filters on the kind it
    // owns, so this is a list rather than the single handler the other two ranges use.
    private static final List<Handler> INPUT_HANDLERS = new CopyOnWriteArrayList<>();
    private static final List<TextHandler> TEXT_HANDLERS = new CopyOnWriteArrayList<>();
    private static volatile boolean subscribed;
    private static long textInputRegistration;
    private static long[] inputDeviceRegistrations;
    private static long[] joystickRegistrations;
    private static long mouseClickedRegistration;
    private static long[] textCompositionRegistrations;

    private GamerEventPump() {
    }

    /** Receives one drained numeric event. */
    public interface Handler {
        void handle(long kind, long session, long first, long second, long flag);
    }

    /**
     * Receives one drained string-carrying event.
     *
     * <p>The payload is UTF-8 bytes the native side has already copied and freed, so the arrays
     * belong to Java alone and outlive the drain.
     */
    public interface TextHandler {
        void handle(long kind, long start, long length, long selected, long horizontal,
                byte[][] payload);
    }

    public static void setGamerHandler(Handler handler) {
        gamerHandler = Objects.requireNonNull(handler, "handler");
    }

    public static void setSessionHandler(Handler handler) {
        sessionHandler = Objects.requireNonNull(handler, "handler");
    }

    /**
     * Adds a handler for the input kind range.
     *
     * <p>Registering the same handler twice would deliver every event twice, so a family
     * installs its handler once from its own class initializer.
     */
    public static void addInputHandler(Handler handler) {
        INPUT_HANDLERS.add(Objects.requireNonNull(handler, "handler"));
    }

    /** Adds a handler for the string-carrying events. */
    public static void addTextHandler(TextHandler handler) {
        TEXT_HANDLERS.add(Objects.requireNonNull(handler, "handler"));
    }

    /**
     * Subscribes CNA's composition and candidate-list events once.
     *
     * <p>Both or neither: a caller that got only the composition could not tell that the
     * candidate list was missing rather than empty.
     */
    public static synchronized void ensureTextCompositionSubscribed() {
        if (textCompositionRegistrations != null) {
            return;
        }
        long[] registrations = new long[2];
        NativeGamerServices.check("Text composition events",
                NativeGamerServices.nativeSubscribeTextComposition(registrations));
        textCompositionRegistrations = registrations;
    }

    /** Releases the composition registrations. */
    public static synchronized void releaseTextComposition() {
        if (textCompositionRegistrations == null) {
            return;
        }
        long[] registrations = textCompositionRegistrations;
        textCompositionRegistrations = null;
        NativeGamerServices.check("Text composition events",
                NativeGamerServices.nativeUnsubscribeTextComposition(registrations));
    }

    /**
     * Subscribes CNA's four mouse and keyboard hot-plug events once.
     *
     * <p>They are static CNA events, so the registrations belong to the process rather than to a
     * game and outlive any one game. Subscribing twice would deliver every event twice, so the
     * decision lives here rather than at each call site.
     */
    public static synchronized void ensureInputDevicesSubscribed() {
        if (inputDeviceRegistrations != null) {
            return;
        }
        long[] registrations = new long[4];
        NativeGamerServices.check("Input device hot-plug events",
                NativeGamerServices.nativeSubscribeInputDeviceEvents(registrations));
        inputDeviceRegistrations = registrations;
    }

    /** Releases the hot-plug registrations, for a test that needs to prove they can be released. */
    public static synchronized void releaseInputDevices() {
        if (inputDeviceRegistrations == null) {
            return;
        }
        long[] registrations = inputDeviceRegistrations;
        inputDeviceRegistrations = null;
        NativeGamerServices.check("Input device hot-plug events",
                NativeGamerServices.nativeUnsubscribeInputDeviceEvents(registrations));
    }

    /**
     * Subscribes CNA's two raw-joystick hot-plug events once.
     *
     * <p>Process-wide, for the same reason the mouse and keyboard ones are.
     */
    public static synchronized void ensureJoysticksSubscribed() {
        if (joystickRegistrations != null) {
            return;
        }
        long[] registrations = new long[2];
        NativeGamerServices.check("Joystick hot-plug events",
                NativeGamerServices.nativeSubscribeJoystickEvents(registrations));
        joystickRegistrations = registrations;
    }

    /** Releases the raw-joystick registrations. */
    public static synchronized void releaseJoysticks() {
        if (joystickRegistrations == null) {
            return;
        }
        long[] registrations = joystickRegistrations;
        joystickRegistrations = null;
        NativeGamerServices.check("Joystick hot-plug events",
                NativeGamerServices.nativeUnsubscribeJoystickEvents(registrations));
    }

    /** Subscribes CNA's mouse-click event once. Process-wide, like the hot-plug events. */
    public static synchronized void ensureMouseClickedSubscribed() {
        if (mouseClickedRegistration != 0L) {
            return;
        }
        long[] registration = new long[1];
        NativeGamerServices.check("Mouse click events",
                NativeGamerServices.nativeSubscribeMouseClicked(registration));
        mouseClickedRegistration = registration[0];
    }

    /** Releases the mouse-click registration. */
    public static synchronized void releaseMouseClicked() {
        if (mouseClickedRegistration == 0L) {
            return;
        }
        long registration = mouseClickedRegistration;
        mouseClickedRegistration = 0L;
        NativeGamerServices.check("Mouse click events",
                NativeGamerServices.nativeUnsubscribeMouseClicked(registration));
    }

    /** Subscribes CNA's typed-character event once. */
    public static synchronized void ensureTextInputSubscribed() {
        if (textInputRegistration != 0L) {
            return;
        }
        long[] registration = new long[1];
        NativeGamerServices.check("TextInput events",
                NativeGamerServices.nativeSubscribeTextInput(registration));
        textInputRegistration = registration[0];
    }

    /**
     * Subscribes the process-wide gamer-service events once.
     *
     * <p>Subscribing twice would deliver every event twice, so the decision lives here rather
     * than at each call site.
     */
    public static synchronized void ensureSubscribed() {
        if (subscribed) {
            return;
        }
        long[] registrations = new long[3];
        NativeGamerServices.check("GamerServices events",
                NativeGamerServices.nativeSubscribeGamerEvents(registrations));
        subscribed = true;
    }

    /** Subscribes one session's events and returns its registrations. */
    public static long[] subscribeSession(long session) {
        long[] registrations = new long[10];
        NativeGamerServices.check("NetworkSession events",
                NativeGamerServices.nativeSubscribeSessionEvents(session, registrations));
        return registrations;
    }

    /** Releases one session's registrations. */
    public static void unsubscribeSession(long[] registrations) {
        NativeGamerServices.check("NetworkSession events",
                NativeGamerServices.nativeUnsubscribeSessionEvents(registrations));
    }

    /**
     * Delivers every event queued so far.
     *
     * <p>A listener that throws must not swallow the events behind it, so the first failure is
     * kept, the drain continues, and the failure is rethrown once the queue is empty.
     */
    public static void drain() {
        RuntimeException completion = completeWatches();
        long[] numeric = new long[6];
        long[] text = new long[6];
        boolean hasNumeric = NativeGamerServices.nativePollEvent(numeric);
        byte[][] payload = NativeGamerServices.nativePollTextEvent(text);
        RuntimeException failure = null;
        while (hasNumeric || payload != null) {
            // Two queues, one order. Every event carries a sequence stamped when CNA raised
            // it, so the older head goes first: a committed character and the composition
            // update that cleared it reach the game in the order they happened.
            boolean takeNumeric = payload == null || (hasNumeric && numeric[0] < text[0]);
            try {
                if (takeNumeric) {
                    dispatchNumeric(numeric);
                } else {
                    dispatchText(text, payload);
                }
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
            if (takeNumeric) {
                hasNumeric = NativeGamerServices.nativePollEvent(numeric);
            } else {
                payload = NativeGamerServices.nativePollTextEvent(text);
            }
        }
        if (completion != null) {
            if (failure == null) {
                failure = completion;
            } else {
                failure.addSuppressed(completion);
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    /**
     * Runs {@code onCompletion} from the first pump that finds {@code completed} true.
     *
     * <p>For an operation CNA completes later without calling back into Java -- a Guide screen
     * waiting for its player -- so its XNA callback runs from the same pump XNA's own
     * dispatcher completes it from.
     */
    public static void watch(java.util.function.BooleanSupplier completed, Runnable onCompletion) {
        WATCHES.add(new Watch(completed, onCompletion));
    }

    private record Watch(java.util.function.BooleanSupplier completed, Runnable onCompletion) {
    }

    private static final List<Watch> WATCHES = new CopyOnWriteArrayList<>();

    private static RuntimeException completeWatches() {
        RuntimeException failure = null;
        for (Watch watch : WATCHES) {
            try {
                if (watch.completed().getAsBoolean() && WATCHES.remove(watch)) {
                    watch.onCompletion().run();
                }
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        return failure;
    }

    /**
     * Delivers one numeric event to every handler that owns its kind.
     *
     * <p>A listener that throws must not swallow the listeners behind it, so each is called and
     * the first failure is kept for the caller to rethrow.
     */
    private static void dispatchNumeric(long[] record) {
        Iterable<Handler> handlers;
        if (record[1] >= FIRST_INPUT_KIND) {
            handlers = INPUT_HANDLERS;
        } else if (record[1] >= FIRST_SESSION_KIND) {
            handlers = single(sessionHandler);
        } else {
            handlers = single(gamerHandler);
        }
        RuntimeException failure = null;
        for (Handler handler : handlers) {
            try {
                handler.handle(record[1], record[2], record[3], record[4], record[5]);
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    private static void dispatchText(long[] header, byte[][] payload) {
        RuntimeException failure = null;
        for (TextHandler handler : TEXT_HANDLERS) {
            try {
                handler.handle(header[1], header[2], header[3], header[4], header[5], payload);
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    private static Iterable<Handler> single(Handler handler) {
        return handler == null ? List.of() : List.of(handler);
    }

    /** Returns how many events CNA raised that either queue could not hold. */
    public static long droppedEventCount() {
        return NativeGamerServices.nativeDroppedEventCount()
                + NativeGamerServices.nativeDroppedTextEventCount();
    }

    /** Discards every queued event, for a new game lifetime or a test. */
    public static void reset() {
        NativeGamerServices.nativeResetEvents();
        NativeGamerServices.nativeResetTextEvents();
    }
}
