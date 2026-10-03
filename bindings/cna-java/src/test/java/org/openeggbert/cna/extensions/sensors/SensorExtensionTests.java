package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.Duration;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CNA's host motion sensors, against the live runtime.
 *
 * <p>The qualification host is headless and has none of these sensors. That is the branch these
 * tests exercise deliberately: an absent sensor must say so through {@link SensorState} and must
 * refuse a reading rather than answering with zeros, and none of that may be hardcoded on the
 * Java side. What is asserted is CNA's own answer, whatever it is.
 */
final class SensorExtensionTests {

    @Test
    void stateIdentitiesAreCnasOwn() {
        assertEquals(6, SensorState.values().length);
        assertEquals(0, SensorState.NotSupported.ordinal());
        assertEquals(1, SensorState.Ready.ordinal());
        assertEquals(5, SensorState.Disabled.ordinal());
    }

    @Test
    void aReadingIsAValueWithTheUnitsItsSensorReports() {
        // The records carry no native handle and no lifetime, so a reading outlives the sensor
        // it came from and comparing two is ordinary value equality.
        OffsetDateTime when = OffsetDateTime.of(2026, 8, 30, 12, 0, 0, 0, ZoneOffset.UTC);
        CompassReading first = new CompassReading(when, 1.5, 90.25, 91.5,
                new Microsoft.Xna.Framework.Vector3(1, 2, 3));
        CompassReading second = new CompassReading(when, 1.5, 90.25, 91.5,
                new Microsoft.Xna.Framework.Vector3(1, 2, 3));
        assertEquals(first, second);
        assertEquals(first.hashCode(), second.hashCode());
        assertEquals(90.25, first.MagneticHeading());
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void sensorsAnswerTruthfullyOnAHostThatHasNone() {
        try (Game game = new Game()) {
            SensorProbe probe = new SensorProbe(game);
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

    private static final class SensorProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private SensorProbe(Game game) {
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
            // Support is CNA's answer, not this test's assumption. Whatever it says, creation
            // has to succeed, which is the contract that lets a game ask a sensor about itself.
            boolean accelerometerSupported = Accelerometer.getIsSupported();
            try (Accelerometer accelerometer = Accelerometer.Create()) {
                assertNotNull(accelerometer);
                SensorState state = accelerometer.getState();
                assertNotNull(state);
                assertEquals(accelerometerSupported, state != SensorState.NotSupported,
                        "the state has to agree with the support query");
                if (state == SensorState.NotSupported) {
                    // An unsupported sensor refuses a reading rather than answering zeros.
                    // That refusal is the whole point: a game that skipped the state check
                    // must not silently receive a vector of zeros it would treat as level.
                    assertThrows(IllegalStateException.class, accelerometer::getCurrentValue);
                    assertFalse(accelerometer.getIsDataValid());
                } else {
                    AccelerometerReading reading = accelerometer.getCurrentValue();
                    assertNotNull(reading.Timestamp());
                    assertNotNull(reading.Acceleration());
                }
                // The sampling interval round-trips through CNA whether or not hardware is
                // behind it, because CNA records the request itself.
                accelerometer.setTimeBetweenUpdates(Duration.ofMillis(20));
                assertEquals(Duration.ofMillis(20), accelerometer.getTimeBetweenUpdates());
                assertThrows(NullPointerException.class,
                        () -> accelerometer.setTimeBetweenUpdates(null));
                assertThrows(IllegalArgumentException.class,
                        () -> accelerometer.setTimeBetweenUpdates(Duration.ofMillis(-1)));
            }

            try (Gyroscope gyroscope = Gyroscope.Create()) {
                assertEquals(Gyroscope.getIsSupported(),
                        gyroscope.getState() != SensorState.NotSupported);
            }
            try (Compass compass = Compass.Create()) {
                assertEquals(Compass.getIsSupported(),
                        compass.getState() != SensorState.NotSupported);
            }
            try (Motion motion = Motion.Create()) {
                assertEquals(Motion.getIsSupported(),
                        motion.getState() != SensorState.NotSupported);
                // Deliberately vacuous before a backend starts: true here means "nothing is
                // drifting yet", which is why it is reported rather than interpreted.
                assertDoesNotThrow(motion::getIsAttitudeNorthReferenced);
            }

            // The machine's own sensors, enumerated. Zero is an ordinary answer here, and the
            // two quick reads report absent rather than a zero vector -- a zero acceleration
            // vector would mean free fall, which is a measurement, not a missing sensor.
            assertNotNull(SensorDevices.enumerate());
            for (SensorDeviceInfo sensor : SensorDevices.enumerate()) {
                assertNotNull(sensor.Type());
                assertNotNull(sensor.Name());
            }
            assertThrows(UnsupportedOperationException.class,
                    () -> SensorDevices.enumerate().add(
                            new SensorDeviceInfo(1, SensorType.Unknown, "x")));
            // The quick reads and the enumeration are the same subsystem, so they have to
            // agree: a host that lists no accelerometer cannot produce an acceleration.
            boolean listsAccelerometer = SensorDevices.enumerate().stream().anyMatch(
                    sensor -> sensor.Type() == SensorType.Accelerometer
                            || sensor.Type() == SensorType.AccelerometerLeft
                            || sensor.Type() == SensorType.AccelerometerRight);
            if (!listsAccelerometer) {
                assertNull(SensorDevices.getAcceleration(),
                        "no enumerated accelerometer, so no reading may be produced");
            }
            boolean listsGyroscope = SensorDevices.enumerate().stream().anyMatch(
                    sensor -> sensor.Type() == SensorType.Gyroscope
                            || sensor.Type() == SensorType.GyroscopeLeft
                            || sensor.Type() == SensorType.GyroscopeRight);
            if (!listsGyroscope) {
                assertNull(SensorDevices.getAngularVelocity(),
                        "no enumerated gyroscope, so no reading may be produced");
            }
            assertEquals(7, SensorType.values().length);

            // Closing releases the owned handle. A second close is a no-op even though CNA
            // refuses a second disposal, and every other member refuses afterwards.
            Accelerometer closed = Accelerometer.Create();
            closed.close();
            closed.close();
            assertThrows(IllegalStateException.class, closed::getState);
            assertThrows(IllegalStateException.class, closed::getCurrentValue);
            assertThrows(IllegalStateException.class, closed::Start);
        }
    }
}
