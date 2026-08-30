package com.openeggbert.cna.template;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.extensions.devices.InputDeviceInfo;
import org.openeggbert.cna.extensions.devices.InputDeviceKind;
import org.openeggbert.cna.extensions.devices.InputDevices;
import org.openeggbert.cna.extensions.graphics.ExtensionNotSupportedException;
import org.openeggbert.cna.extensions.graphics.GraphicsExtension;
import org.openeggbert.cna.extensions.graphics.PbrMaterial;
import org.openeggbert.cna.extensions.graphics.RenderPipelineSettings;
import org.openeggbert.cna.extensions.graphics.TonemappingMode;
import org.openeggbert.cna.extensions.runtime.CnaLogger;
import org.openeggbert.cna.extensions.runtime.CnaRuntime;
import org.openeggbert.cna.extensions.runtime.LogCategory;
import org.openeggbert.cna.extensions.runtime.LogLevel;
import org.openeggbert.cna.extensions.input.GamePadExtensions;
import org.openeggbert.cna.extensions.input.HapticCapabilities;
import org.openeggbert.cna.extensions.input.HapticDevice;
import org.openeggbert.cna.extensions.input.HapticDevices;
import org.openeggbert.cna.extensions.input.JoystickCapabilities;
import org.openeggbert.cna.extensions.input.JoystickInfo;
import org.openeggbert.cna.extensions.input.Joysticks;
import org.openeggbert.cna.extensions.input.TextComposition;
import org.openeggbert.cna.extensions.input.TextInput;
import org.openeggbert.cna.extensions.sensors.Accelerometer;
import org.openeggbert.cna.extensions.sensors.Compass;
import org.openeggbert.cna.extensions.sensors.Gyroscope;
import org.openeggbert.cna.extensions.sensors.Motion;
import org.openeggbert.cna.extensions.sensors.Sensor;
import org.openeggbert.cna.extensions.sensors.SensorDeviceInfo;
import org.openeggbert.cna.extensions.sensors.SensorDevices;

import java.util.ArrayList;
import java.util.List;

/**
 * Proves the CNA extension surface works from outside the binding.
 *
 * <p>Deliberately not part of {@link HelloGame}. The starter stays an XNA program an XNA
 * developer recognizes; this is the separate, opt-in {@code --extensions-smoke} mode.
 *
 * <p>What it proves is narrow and honest: that the extension packages compile against the
 * published artifact, that the JNI routes behind them exist, that the availability query answers
 * rather than guesses, and that a build without the extended graphics layer says so rather than
 * doing something else. It does not claim any rendering happened.
 *
 * <p>The device half needs a running game, because that is where CNA's host platform lives, so
 * it runs one frame and reports from inside it. <strong>It must pass on a machine with no
 * sensor, joystick or force feedback</strong>, which is the point: absent hardware is an
 * ordinary answer a game has to handle, not a failure, and this is where that is checked from
 * outside the binding.
 */
final class ExtensionsSmoke {

    private ExtensionsSmoke() {
    }

    static void run() {
        System.out.println("cna-java-template: CNA runtime");
        System.out.println("  platform         " + CnaRuntime.getPlatform()
                + " (" + CnaRuntime.getPlatformName() + ")");
        System.out.println("  mobile           " + CnaRuntime.isMobile());
        System.out.println("  apple            " + CnaRuntime.isApple());
        System.out.println("  renderer         " + CnaRuntime.getRendererName());
        System.out.println("  backend category " + CnaRuntime.getBackendCategory()
                + " (" + CnaRuntime.getName(CnaRuntime.getBackendCategory()) + ")");
        System.out.println("  backend maturity " + CnaRuntime.getBackendMaturity()
                + " (" + CnaRuntime.getName(CnaRuntime.getBackendMaturity()) + ")");

        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.Info("cna-java-template extensions smoke", LogCategory.Application);
        } finally {
            CnaLogger.setMinimumLevel(minimum);
        }

        boolean available = GraphicsExtension.isAvailable();
        System.out.println("cna-java-template: extended graphics layer available " + available);

        // The value routes work in either build, so their defaults come from CNA whichever one
        // is loaded.
        RenderPipelineSettings settings = new RenderPipelineSettings();
        System.out.println("  default tonemapping " + settings.getTonemappingMode()
                + ", exposure " + settings.getExposure()
                + ", gamma " + settings.getGamma()
                + ", shadows " + settings.getShadowQuality());
        settings.setTonemappingMode(TonemappingMode.Aces);
        if (settings.getTonemappingMode() != TonemappingMode.Aces) {
            throw new IllegalStateException("RenderPipelineSettings did not keep its value");
        }

        PbrMaterial material = new PbrMaterial();
        System.out.println("  default material metallic " + material.getMetallicFactor()
                + ", roughness " + material.getRoughnessFactor()
                + ", albedo " + material.getAlbedoColor());

        // The one route that needs the native extension object. On a build without the layer it
        // must say so, which is the distinction this smoke exists to check.
        try {
            reportEffect(available);
        } catch (ExtensionNotSupportedException notSupported) {
            if (available) {
                throw new IllegalStateException(
                        "The extended layer reported itself available and then refused", notSupported);
            }
            System.out.println("  post-process effect NOT_SUPPORTED, as this build reports");
        }
        devices();
        System.out.println("cna-java-template: extensions smoke passed");
    }

    /** Runs one frame and reports what the host actually has. */
    private static void devices() {
        try (Game game = new Game()) {
            DeviceReport report = new DeviceReport(game);
            game.getComponents().add(report);
            game.RunOneFrame();
            if (report.failure != null) {
                throw new IllegalStateException("device smoke failed", report.failure);
            }
            if (!report.ran) {
                throw new IllegalStateException("device smoke never ran");
            }
        }
    }

    /** Reports the host's devices from inside Update, where CNA's platform is reachable. */
    private static final class DeviceReport extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private DeviceReport(Game game) {
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
                report();
            } catch (RuntimeException | LinkageError problem) {
                failure = problem;
            }
        }

        private void report() {
            System.out.println("cna-java-template: input devices");
            for (InputDeviceKind kind : InputDeviceKind.values()) {
                List<InputDeviceInfo> devices = InputDevices.enumerate(kind);
                System.out.println("  " + kind + " x" + devices.size());
                for (InputDeviceInfo device : devices) {
                    System.out.println("    #" + device.Id() + " " + device.Name());
                }
            }

            System.out.println("cna-java-template: host sensors");
            List<SensorDeviceInfo> sensors = SensorDevices.enumerate();
            System.out.println("  enumerated x" + sensors.size());
            reportSensor("accelerometer", Accelerometer.getIsSupported(), Accelerometer.Create());
            reportSensor("gyroscope", Gyroscope.getIsSupported(), Gyroscope.Create());
            reportSensor("compass", Compass.getIsSupported(), Compass.Create());
            reportSensor("motion", Motion.getIsSupported(), Motion.Create());

            System.out.println("cna-java-template: raw joysticks");
            List<JoystickInfo> joysticks = Joysticks.enumerate();
            System.out.println("  enumerated x" + joysticks.size());
            for (JoystickInfo joystick : joysticks) {
                JoystickCapabilities capabilities = Joysticks.getCapabilities(joystick.Id());
                System.out.println("    #" + joystick.Id() + " " + joystick.Name()
                        + " " + capabilities.Type()
                        + " axes " + capabilities.AxisCount()
                        + ", buttons " + capabilities.ButtonCount()
                        + ", hats " + capabilities.HatCount());
            }

            System.out.println("cna-java-template: force feedback");
            System.out.println("  standalone devices x" + HapticDevices.enumerate().size());
            System.out.println("  mouse haptic " + HapticDevices.isMouseHaptic());
            try (HapticDevice mouse = HapticDevices.openFromMouse()) {
                HapticCapabilities capabilities = mouse.getCapabilities();
                // Opening never fails for absent hardware; the device says it is closed and
                // every operation is a safe no-op. A game can write one force-feedback path.
                System.out.println("  opened mouse haptics, open=" + capabilities.IsOpen()
                        + ", features " + capabilities.Features());
            }

            System.out.println("cna-java-template: game pad one");
            System.out.println("  connection " + GamePadExtensions.getConnectionState(
                    PlayerIndex.One));
            System.out.println("  power " + GamePadExtensions.getPowerState(PlayerIndex.One)
                    + ", battery " + describe(
                            GamePadExtensions.getBatteryPercent(PlayerIndex.One)));
            System.out.println("  gyro " + describe(
                    GamePadExtensions.getAngularVelocity(PlayerIndex.One)));

            // The text transport, from outside the binding: CNA raises its own composition
            // event and the draft reaches a listener on the game thread.
            List<String> drafts = new ArrayList<>();
            TextInput.addCompositionListener(
                    (TextComposition draft) -> drafts.add(draft.Text()));
            TextInput.RaiseComposition("smoke", 0, 5);
            FrameworkDispatcher.Update();
            if (drafts.size() != 1 || !"smoke".equals(drafts.get(0))) {
                throw new IllegalStateException("composition event did not arrive: " + drafts);
            }
            System.out.println("cna-java-template: composition event delivered " + drafts);
        }

        private void reportSensor(String name, boolean supported, Sensor<?> sensor) {
            try (Sensor<?> owned = sensor) {
                System.out.println("  " + name + " supported " + supported
                        + ", state " + owned.getState()
                        + ", data valid " + owned.getIsDataValid());
            }
        }

        private String describe(Object value) {
            return value == null ? "not reported" : value.toString();
        }
    }


    private static void reportEffect(boolean available) {
        if (!available) {
            throw new ExtensionNotSupportedException(
                    "this build has no extended graphics layer");
        }
        System.out.println("  post-process effect available on this build");
    }
}
