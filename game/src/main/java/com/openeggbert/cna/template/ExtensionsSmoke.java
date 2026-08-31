package com.openeggbert.cna.template;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.extensions.content.Cnb;
import org.openeggbert.cna.extensions.content.CnbDocument;
import org.openeggbert.cna.extensions.content.CnbImport;
import org.openeggbert.cna.extensions.content.CnbReadLimits;
import org.openeggbert.cna.extensions.content.CnbSoundEffectData;
import org.openeggbert.cna.extensions.content.CnbSoundEffectInfo;
import org.openeggbert.cna.extensions.content.CnbTextureData;
import org.openeggbert.cna.extensions.devices.InputDeviceInfo;
import org.openeggbert.cna.extensions.devices.InputDeviceKind;
import org.openeggbert.cna.extensions.devices.InputDevices;
import org.openeggbert.cna.extensions.graphics.ExtensionNotSupportedException;
import org.openeggbert.cna.extensions.graphics.GraphicsExtension;
import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.extensions.graphics.DirectionalLight;
import org.openeggbert.cna.extensions.graphics.FrustumCuller;
import org.openeggbert.cna.extensions.graphics.LightProbe;
import org.openeggbert.cna.extensions.graphics.LodGroup;
import org.openeggbert.cna.extensions.graphics.PbrMaterial;
import org.openeggbert.cna.extensions.graphics.RenderPipeline;
import org.openeggbert.cna.extensions.graphics.RenderPipelineFrameStatistics;
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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
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

 * <p>The content half runs outside the game on purpose, because that is where a build step runs:
 * writing a {@code .cnb} and importing a WAV need no window, no device and no frame.
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
        engine();
        devices();
        content();
        System.out.println("cna-java-template: extensions smoke passed");
    }

    /**
     * Four small things from CNA's engine layer, three of which need no device at all.
     *
     * <p>Deliberately small. The point is that an external consumer can reach the engine layer
     * and get an answer back, not to build a scene: one LOD selection, one culled box, one
     * light's defaults, one probe's irradiance, and one real pipeline frame reporting what it
     * cost.
     */
    private static void engine() {
        System.out.println("cna-java-template: engine layer");
        System.out.println("  revision         " + GraphicsExtension.getEngineLayerVersion());

        // Level of detail: two thresholds, and the group picks by distance.
        try (LodGroup lod = LodGroup.create()) {
            lod.addLevel(10.0f);
            lod.addLevel(50.0f);
            System.out.println("  lod at 5 units   level " + lod.selectIndex(5.0f)
                    + ", at 30 units level " + lod.selectIndex(30.0f));
        }

        // Culling: one box in front of the camera and one behind it.
        try (FrustumCuller culler = FrustumCuller.create()) {
            culler.setCamera(
                    Matrix.CreateLookAt(new Vector3(0f, 0f, 10f), new Vector3(0f, 0f, 0f),
                            new Vector3(0f, 1f, 0f)),
                    Matrix.CreatePerspectiveFieldOfView(1.0f, 1.0f, 1.0f, 100.0f));
            List<BoundingBox> boxes = List.of(
                    new BoundingBox(new Vector3(-1f, -1f, -1f), new Vector3(1f, 1f, 1f)),
                    new BoundingBox(new Vector3(-1f, -1f, 39f), new Vector3(1f, 1f, 41f)));
            System.out.println("  visible boxes    "
                    + Arrays.toString(culler.cullBoxes(boxes)) + " of 2");
        }

        DirectionalLight sun = DirectionalLight.createDefault();
        System.out.println("  default sun      direction " + sun.getDirection()
                + ", intensity " + sun.getIntensity());

        // Indirect light: a probe holding only its constant term lights every normal alike.
        try (LightProbe probe = LightProbe.create()) {
            probe.setCoefficient(0, new Vector3(1f, 1f, 1f));
            System.out.println("  probe irradiance " + probe.getIrradiance(new Vector3(0f, 1f, 0f)));
        }

        pipelineFrame();
    }

    /** One real pipeline frame, which is the only part of the engine smoke that needs a device. */
    private static void pipelineFrame() {
        try (Game game = new Game()) {
            new GraphicsDeviceManager(game);
            PipelineReport report = new PipelineReport(game);
            game.getComponents().add(report);
            game.RunOneFrame();
            if (report.failure != null) {
                throw new IllegalStateException("engine smoke failed", report.failure);
            }
            if (!report.ran) {
                throw new IllegalStateException("engine smoke never ran");
            }
        }
    }

    /** Runs one pipeline frame from inside Update, where the graphics device is reachable. */
    private static final class PipelineReport extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private PipelineReport(Game game) {
            super(game);
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try (RenderPipeline pipeline = RenderPipeline.create(getGame().getGraphicsDevice())) {
                pipeline.resize(320, 240);
                pipeline.setCamera(
                        Matrix.CreateLookAt(new Vector3(0f, 0f, 4f), new Vector3(0f, 0f, 0f),
                                new Vector3(0f, 1f, 0f)),
                        Matrix.CreatePerspectiveFieldOfView(1.0f, 4f / 3f, 0.5f, 200.0f),
                        0.5f, 200.0f);
                pipeline.begin(Color.CornflowerBlue);
                pipeline.end();
                RenderPipelineFrameStatistics statistics = pipeline.getStatistics();
                System.out.println("  pipeline frame   " + statistics.passesRun()
                        + " passes, " + statistics.targetSwitches() + " target switches, "
                        + statistics.gpuMemoryEstimateBytes() + " estimated bytes");
            } catch (RuntimeException | LinkageError problem) {
                failure = problem;
            }
        }
    }

    /**
     * Builds content the way a build step would, with no window and no device.
     *
     * <p>Outside a game on purpose: this is the half of the content pipeline that runs before a
     * game exists. A WAV an artist hands over becomes a compiled sound, a texture becomes a
     * {@code .cnb} file, and the file reads back as the picture that went in -- none of which
     * needs a graphics or audio backend, which is exactly what makes it usable in a build.
     */
    private static void content() {
        System.out.println("cna-java-template: content");

        byte[] pixels = {
            (byte) 0xFF, 0x00, 0x00, (byte) 0xFF,
            0x00, (byte) 0xFF, 0x00, (byte) 0xFF,
            0x00, 0x00, (byte) 0xFF, (byte) 0xFF,
            (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        };
        byte[] file;
        try (CnbTextureData texture = CnbTextureData.ofRgba8(2, 2, pixels)) {
            file = Cnb.encodeTexture2D(texture, "textures/four-pixels");
        }
        System.out.println("  encoded a Texture2D .cnb of " + file.length + " bytes");
        try (CnbDocument document = CnbDocument.parse(
                     file, "four-pixels.cnb", CnbReadLimits.standard());
             CnbTextureData decoded = document.decodeTexture2D()) {
            System.out.println("  asset type " + document.getAssetType().getName()
                    + ", " + decoded.getInfo().Width() + "x" + decoded.getInfo().Height());
            if (!Arrays.equals(pixels, decoded.readLevel(0, 0))) {
                throw new IllegalStateException("the .cnb round trip changed the pixels");
            }
        }

        try (CnbSoundEffectData sound = CnbImport.wav(wav(), "template.wav")) {
            CnbSoundEffectInfo info = sound.getInfo();
            System.out.println("  imported a WAV: " + info.Format() + " "
                    + info.SampleRate() + "Hz x" + info.Channels()
                    + ", " + info.FrameCount() + " frames");
            if (info.FrameCount() != 8) {
                throw new IllegalStateException("the WAV importer lost frames");
            }
        }
    }

    /** Eight frames of 16-bit mono PCM, written from the WAV layout. */
    private static byte[] wav() {
        ByteBuffer buffer = ByteBuffer.allocate(44 + 16).order(ByteOrder.LITTLE_ENDIAN);
        buffer.put(new byte[] {'R', 'I', 'F', 'F'});
        buffer.putInt(36 + 16);
        buffer.put(new byte[] {'W', 'A', 'V', 'E'});
        buffer.put(new byte[] {'f', 'm', 't', ' '});
        buffer.putInt(16);
        buffer.putShort((short) 1);
        buffer.putShort((short) 1);
        buffer.putInt(22050);
        buffer.putInt(22050 * 2);
        buffer.putShort((short) 2);
        buffer.putShort((short) 16);
        buffer.put(new byte[] {'d', 'a', 't', 'a'});
        buffer.putInt(16);
        for (int frame = 0; frame < 8; frame++) {
            buffer.putShort((short) (frame * 4000 - 16000));
        }
        return buffer.array();
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
