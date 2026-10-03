package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Sensor readings on a machine with no sensors, which is every machine this projection runs on.
 *
 * <p>A hundred and two routes were unbound behind "host sensors are a CNA device extension; XNA
 * 4.0 exposes no sensor API on the desktop profile" -- true about XNA, and silent about why the
 * family was absent. The assumption behind leaving it was that an accelerometer API needs an
 * accelerometer. It does not, and the shape of what it does need was measured in
 * {@code tools/native-abi/probes/sensor_injection.c} before any of this was written:
 *
 * <ul>
 *   <li>{@code Start()} is refused on a machine with no such sensor, because there is no platform
 *       subsystem to hold. Forcing the started flag is the way in, and it is why
 *       {@link SensorTestBackends} exists rather than being hidden.</li>
 *   <li>An injection then raises the subscription <strong>synchronously on the injecting
 *       thread</strong>, with the handler's own context intact.</li>
 *   <li>The injected value is in <strong>platform units</strong> and the reading is
 *       <strong>canonical</strong>: 9.80665 metres per second squared reads back as one g. That
 *       is arithmetic, and it is what this file asserts rather than a result code.</li>
 * </ul>
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class SensorInjectionTests {

    /** Standard gravity, which is the number the unit conversion turns into one. */
    private static final float ONE_G = 9.80665f;

    @Test
    void anInjectedReadingArrivesInCanonicalUnitsThroughASubscription() {
        run(() -> {
            try (Accelerometer sensor = Accelerometer.Create()) {
                // Start is refused with no hardware, which is the fact the whole family turns on.
                assertThrows(RuntimeException.class, sensor::Start);
                // Two forced flags, not one, and the difference is measured: started is what
                // makes an injection deliver, and supported is what makes a reading readable.
                SensorTestBackends.forceSupported(sensor, true);
                SensorTestBackends.forceStarted(sensor, true);
                assertFalse(SensorTestBackends.getSubsystemHeld(sensor),
                        "a forced sensor reports itself started and still holds nothing");

                List<AccelerometerReading> readings = new ArrayList<>();
                try (SensorSubscription subscription =
                        sensor.addCurrentValueChangedListener(readings::add)) {
                    assertNotNull(subscription);
                    sensor.injectSyntheticUpdate(0f, ONE_G, 0f);
                    assertEquals(1, readings.size(),
                            "the handler ran, and it ran before the injection returned");

                    // The conversion, which is the header's own promise: metres per second
                    // squared in, g out.
                    AccelerometerReading first = readings.get(0);
                    assertEquals(0f, first.Acceleration().X, 1e-5f);
                    assertEquals(1f, first.Acceleration().Y, 1e-5f);
                    assertEquals(0f, first.Acceleration().Z, 1e-5f);
                    assertNotNull(first.Timestamp());

                    // And the current value agrees with what the handler was given, which says
                    // the subscription and the getter read one reading rather than two.
                    assertEquals(first.Acceleration().Y, sensor.getCurrentValue()
                            .Acceleration().Y, 1e-5f);
                    assertTrue(sensor.getIsDataValid(),
                            "an injected reading is a real one, not the zeroed default");

                    // A second injection on a different axis, so a reading that never changes
                    // is visible rather than agreeing by accident.
                    sensor.injectSyntheticUpdate(2f * ONE_G, 0f, 0f);
                    assertEquals(2, readings.size());
                    assertEquals(2f, readings.get(1).Acceleration().X, 1e-5f);
                    assertEquals(0f, readings.get(1).Acceleration().Y, 1e-5f);
                }

                // Closed, so nothing more arrives. That is the whole reason the registration is
                // a handle rather than a fire-and-forget.
                sensor.injectSyntheticUpdate(0f, 0f, ONE_G);
                assertEquals(2, readings.size(), "an unsubscribed handler never runs again");
            }
        });
    }

    @Test
    void theObsoleteReadingEventIsRaisedAfterTheCurrentValueOne() {
        run(() -> {
            try (Accelerometer sensor = Accelerometer.Create()) {
                SensorTestBackends.forceStarted(sensor, true);
                List<String> order = new ArrayList<>();
                try (SensorSubscription current = sensor.addCurrentValueChangedListener(
                                reading -> order.add("current"));
                        SensorSubscription legacy = sensor.addReadingChangedListener(
                                reading -> order.add("legacy"))) {
                    assertNotNull(current);
                    assertNotNull(legacy);
                    sensor.injectSyntheticUpdate(0f, ONE_G, 0f);
                    // The canonical order is fixed and CNA reproduces it: current first, the
                    // obsolete event second. Asserting the order is what says both were raised
                    // for one reading rather than one of them twice.
                    assertEquals(List.of("current", "legacy"), order);
                }
            }
        });
    }

    @Test
    void aGyroscopeRoundTripsItsOwnUnitAndADispatchNamesItsRecipients() {
        run(() -> {
            try (Gyroscope first = Gyroscope.Create(); Gyroscope second = Gyroscope.Create()) {
                SensorTestBackends.forceSupported(first, true);
                SensorTestBackends.forceSupported(second, true);
                SensorTestBackends.forceStarted(first, true);
                SensorTestBackends.forceStarted(second, true);
                // A dispatch names its recipients, and CNA delivers to the ones it has been told
                // are started -- so both are registered before it, which is the difference
                // between naming a sensor and it being live.
                SensorTestBackends.registerStarted(first);
                SensorTestBackends.registerStarted(second);
                AtomicInteger firstCalls = new AtomicInteger();
                AtomicInteger secondCalls = new AtomicInteger();
                List<GyroscopeReading> readings = new ArrayList<>();
                try (SensorSubscription one = first.addCurrentValueChangedListener(reading -> {
                            firstCalls.incrementAndGet();
                            readings.add(reading);
                        });
                        SensorSubscription two = second.addCurrentValueChangedListener(
                                reading -> secondCalls.incrementAndGet())) {
                    assertNotNull(one);
                    assertNotNull(two);

                    // Injecting into one reaches one.
                    first.injectSyntheticUpdate(1f, 2f, 3f);
                    assertEquals(1, firstCalls.get());
                    assertEquals(0, secondCalls.get(),
                            "an injection into one sensor is not a broadcast");
                    // A gyroscope's canonical unit is radians per second, which is the
                    // platform's too, so this one is an identity rather than a conversion --
                    // and asserting it is what would catch a conversion applied by mistake.
                    assertEquals(1f, readings.get(0).RotationRate().X, 1e-5f);
                    assertEquals(2f, readings.get(0).RotationRate().Y, 1e-5f);
                    assertEquals(3f, readings.get(0).RotationRate().Z, 1e-5f);

                    // And a dispatch that names both reaches both.
                    SensorTestBackends.dispatchToInstances(List.of(first, second), 4f, 5f, 6f);
                    assertEquals(2, firstCalls.get());
                    assertEquals(1, secondCalls.get());
                    assertEquals(4f, readings.get(1).RotationRate().X, 1e-5f);

                    assertEquals(0, SensorTestBackends.getDispatchExceptionCount(
                            SensorType.Gyroscope), "no handler threw");
                }
                SensorTestBackends.unregisterStarted(first);
                SensorTestBackends.unregisterStarted(second);
            }
        });
    }

    @Test
    void aHandlerThatThrowsIsCountedRatherThanPropagated() {
        run(() -> {
            try (Accelerometer sensor = Accelerometer.Create()) {
                SensorTestBackends.forceStarted(sensor, true);
                int before = SensorTestBackends.getDispatchExceptionCount(
                        SensorType.Accelerometer);
                AtomicInteger calls = new AtomicInteger();
                try (SensorSubscription subscription = sensor.addCurrentValueChangedListener(
                        reading -> {
                            calls.incrementAndGet();
                            throw new IllegalStateException("handler refused this reading");
                        })) {
                    assertNotNull(subscription);
                    // A sensor event returns nothing, so a throwing handler cannot fail the
                    // dispatch and must not unwind into C. It is described and cleared at the
                    // boundary, and this is how a test proves the handler ran at all.
                    sensor.injectSyntheticUpdate(0f, ONE_G, 0f);
                    assertEquals(1, calls.get(), "the handler ran");
                }
                assertEquals(before, SensorTestBackends.getDispatchExceptionCount(
                                SensorType.Accelerometer),
                        "the exception was stopped at the Java boundary, so CNA never saw one");
            }
        });
    }

    @Test
    void aCompassBackendMakesTheWholeFamilyReachable() {
        run(() -> {
            try (Compass sensor = Compass.Create()) {
                sensor.setTestBackend(true, true);
                // The static query still answers about the platform, which is not a defect: the
                // backend is on this sensor and getIsSupported asks the host. Measured.
                assertFalse(Compass.getIsSupported(),
                        "a per-sensor backend does not change what the host reports");
                sensor.Start();

                // Installing while started is refused, which the header states.
                assertThrows(RuntimeException.class, () -> sensor.setTestBackend(true, true));

                AtomicInteger calibrations = new AtomicInteger();
                List<CompassReading> readings = new ArrayList<>();
                try (SensorSubscription calibrate =
                                sensor.addCalibrateListener(calibrations::incrementAndGet);
                        SensorSubscription changed =
                                sensor.addCurrentValueChangedListener(readings::add)) {
                    assertNotNull(calibrate);
                    assertNotNull(changed);
                    sensor.injectCalibrationRequest();
                    assertEquals(1, calibrations.get(),
                            "the calibration request carries no reading and still arrives");
                    assertTrue(readings.isEmpty(),
                            "and it is not a reading, so the reading handler stays quiet");

                    OffsetDateTime when =
                            OffsetDateTime.of(2026, 8, 31, 12, 0, 0, 0, ZoneOffset.UTC);
                    CompassReading injected = new CompassReading(when, 1.25, 42.5, 43.5,
                            new Vector3(1f, 2f, 3f));
                    sensor.injectSyntheticUpdate(injected);

                    CompassReading read = sensor.getCurrentValue();
                    assertEquals(42.5, read.MagneticHeading(), 1e-9);
                    assertEquals(43.5, read.TrueHeading(), 1e-9);
                    assertEquals(1.25, read.HeadingAccuracy(), 1e-9);
                    assertEquals(1f, read.MagnetometerReading().X, 1e-5f);
                    assertEquals(3f, read.MagnetometerReading().Z, 1e-5f);
                    // Three doubles that differ, so a projection that read them in the wrong
                    // order is caught rather than agreeing by symmetry.
                    assertEquals(1, readings.size());
                    assertEquals(42.5, readings.get(0).MagneticHeading(), 1e-9);
                    assertEquals(43.5, readings.get(0).TrueHeading(), 1e-9);
                    assertEquals(1.25, readings.get(0).HeadingAccuracy(), 1e-9);
                }
                sensor.Stop();
                sensor.setTestBackend(false, false);
            }
        });
    }

    @Test
    void aMotionBackendCarriesAWholeAttitude() {
        run(() -> {
            try (Motion sensor = Motion.Create()) {
                sensor.setTestBackend(true, true, true);
                assertTrue(sensor.getIsAttitudeNorthReferenced(),
                        "the backend was installed north-referenced and says so");
                sensor.Start();

                OffsetDateTime when =
                        OffsetDateTime.of(2026, 8, 31, 12, 0, 0, 0, ZoneOffset.UTC);
                AttitudeReading attitude = new AttitudeReading(when, 0.25f, 0.5f, 0.75f,
                        new Quaternion(0.1f, 0.2f, 0.3f, 0.4f),
                        Matrix.CreateTranslation(new Vector3(7f, 8f, 9f)));
                MotionReading injected = new MotionReading(when, attitude,
                        new Vector3(1f, 2f, 3f), new Vector3(4f, 5f, 6f),
                        new Vector3(0f, -1f, 0f));

                List<MotionReading> readings = new ArrayList<>();
                try (SensorSubscription changed =
                        sensor.addCurrentValueChangedListener(readings::add)) {
                    assertNotNull(changed);
                    sensor.injectSyntheticUpdate(injected);
                    assertEquals(1, readings.size());
                    MotionReading read = readings.get(0);
                    // The three vectors are different, so a flattening that transposed two of
                    // them is visible rather than agreeing by symmetry.
                    assertEquals(1f, read.DeviceAcceleration().X, 1e-5f);
                    assertEquals(4f, read.DeviceRotationRate().X, 1e-5f);
                    assertEquals(-1f, read.Gravity().Y, 1e-5f);
                    // And the attitude, whose sixteen matrix elements are the longest run of
                    // leaves anything here crosses with.
                    assertEquals(0.25f, read.Attitude().Pitch(), 1e-5f);
                    assertEquals(0.5f, read.Attitude().Roll(), 1e-5f);
                    assertEquals(0.75f, read.Attitude().Yaw(), 1e-5f);
                    assertEquals(0.3f, read.Attitude().Quaternion().Z, 1e-5f);
                    assertEquals(7f, read.Attitude().RotationMatrix().M41, 1e-5f);
                    assertEquals(9f, read.Attitude().RotationMatrix().M43, 1e-5f);
                    assertEquals(1f, read.Attitude().RotationMatrix().M11, 1e-5f);
                }
                sensor.Stop();
                sensor.setTestBackend(false, false, false);
            }
        });
    }

    @Test
    void disposeStopsASensorWithoutReleasingItAndIsNotIdempotent() {
        run(() -> {
            Accelerometer sensor = Accelerometer.Create();
            try {
                SensorTestBackends.forceStarted(sensor, true);
                sensor.dispose();
                // Afterwards the object is still in hand and every route on it refuses, which is
                // the difference from close. Measured, including that a second dispose is
                // REFUSED rather than ignored -- unlike the haptic device's, which is idempotent.
                assertThrows(RuntimeException.class, sensor::getState);
                assertThrows(RuntimeException.class, sensor::dispose);
            } finally {
                sensor.close();
            }
            // And close after dispose still succeeds, so the two are a sequence rather than
            // alternatives.
        });
    }

    @Test
    void whatIsRefusedIsRefused() {
        run(() -> {
            try (Accelerometer sensor = Accelerometer.Create()) {
                assertThrows(NullPointerException.class,
                        () -> sensor.addCurrentValueChangedListener(null));
                assertThrows(NullPointerException.class,
                        () -> sensor.addReadingChangedListener(null));
                assertThrows(NullPointerException.class,
                        () -> SensorTestBackends.forceStarted(null, true));
                assertThrows(NullPointerException.class,
                        () -> SensorTestBackends.dispatchToInstances(null, 0f, 0f, 0f));
                assertThrows(NullPointerException.class,
                        () -> SensorTestBackends.getDispatchExceptionCount(null));
            }
            try (Compass sensor = Compass.Create()) {
                assertThrows(NullPointerException.class, () -> sensor.injectSyntheticUpdate(null));
                assertThrows(NullPointerException.class, () -> sensor.addCalibrateListener(null));
            }
        });
    }

    /** Runs one body inside a frame, because a sensor reaches the host through its game. */
    private static void run(Runnable body) {
        try (Game game = new Game()) {
            SensorProbe probe = new SensorProbe(game, body);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure instanceof RuntimeException runtime) {
                throw runtime;
            }
            if (probe.failure instanceof Error error) {
                throw error;
            }
            if (probe.failure != null) {
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class SensorProbe extends GameComponent {

        private final Runnable body;
        private boolean ran;
        private Throwable failure;

        private SensorProbe(Game game, Runnable body) {
            super(game);
            this.body = body;
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                body.run();
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }
}
