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
import org.openeggbert.cna.extensions.graphics.DebugDraw;
import org.openeggbert.cna.extensions.graphics.PbrEffect;
import org.openeggbert.cna.extensions.graphics.ShaderEffect;
import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.extensions.graphics.PbrEffect;
import org.openeggbert.cna.extensions.graphics.GraphicsCapability;
import org.openeggbert.cna.extensions.graphics.GraphicsRenderer;
import org.openeggbert.cna.extensions.graphics.RendererCapabilities;
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
        graphics(available);
        devices();
        content();
        System.out.println("cna-java-template: extensions smoke passed");
    }

    /**
     * The renderer, and the extension effects a game reaches from outside the binding.
     *
     * <p>Run inside one real frame, because that is where the graphics device is reachable. A
     * build without the extended graphics layer must refuse DebugDraw rather than hand back an
     * object that queues nothing, and that refusal is what is checked there.
     */
    private static void graphics(boolean available) {
        try (Game game = new Game()) {
            new GraphicsDeviceManager(game);
            GraphicsReport report = new GraphicsReport(game, available);
            game.getComponents().add(report);
            game.RunOneFrame();
            if (report.failure != null) {
                throw new IllegalStateException("graphics smoke failed", report.failure);
            }
            if (!report.ran) {
                throw new IllegalStateException("graphics smoke never ran");
            }
        }
    }

    /** Reports from inside Update, where the graphics device is reachable. */
    private static final class GraphicsReport extends GameComponent {

        private final boolean available;
        private boolean ran;
        private Throwable failure;

        private GraphicsReport(Game game, boolean available) {
            super(game);
            this.available = available;
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                report(getGame().getGraphicsDevice());
            } catch (RuntimeException | LinkageError problem) {
                failure = problem;
            }
        }

        private void report(GraphicsDevice device) {
            System.out.println("cna-java-template: renderer");
            System.out.println("  name             " + RendererCapabilities.getRendererName(device));
            System.out.println("  build has        " + GraphicsRenderer.available());
            System.out.println("  selected         " + GraphicsRenderer.getSelected()
                    + ", active " + GraphicsRenderer.getActive()
                    + ", latched " + GraphicsRenderer.isLatched());
            System.out.println("  3D               "
                    + RendererCapabilities.supports(device, GraphicsCapability.ThreeD)
                    + ", compiled effects "
                    + RendererCapabilities.supports(device, GraphicsCapability.CompiledEffects));

            try (PbrEffect pbr = new PbrEffect(device)) {
                pbr.setMetallicFactor(0.25f);
                pbr.setRoughnessFactor(0.75f);
                if (pbr.getMetallicFactor() != 0.25f || pbr.getRoughnessFactor() != 0.75f) {
                    throw new IllegalStateException("PbrEffect did not keep its values");
                }
                System.out.println("  pbr effect       metallic " + pbr.getMetallicFactor()
                        + ", roughness " + pbr.getRoughnessFactor());
            }

            try (ShaderEffect shader = ShaderEffect.compile(device, VERTEX_SOURCE,
                    FRAGMENT_SOURCE)) {
                System.out.println("  shader effect    valid " + shader.isValid());
            }

            if (!available) {
                try {
                    DebugDraw.create(device).close();
                    throw new IllegalStateException(
                            "DebugDraw was created on a build without the extended layer");
                } catch (ExtensionNotSupportedException refused) {
                    System.out.println("  debug draw       NOT_SUPPORTED, as this build reports");
                }
                return;
            }
            try (DebugDraw debug = DebugDraw.create(device)) {
                debug.begin(Matrix.getIdentity(), Matrix.getIdentity());
                debug.addBox(new BoundingBox(new Vector3(-1f, -1f, -1f),
                        new Vector3(1f, 1f, 1f)), Color.Lime);
                if (debug.getLineCount() != 12) {
                    throw new IllegalStateException("a box is twelve edges, not "
                            + debug.getLineCount());
                }
                System.out.println("  debug draw       a box is " + debug.getLineCount()
                        + " edges");
                debug.clear();
            }
        }
    }

    /** SpriteBatch's vertex layout, in the dialect CNA's own shaders use. */
    private static final String VERTEX_SOURCE = String.join("\n",
            "#version 300 es",
            "precision highp float;",
            "layout(location = 0) in vec2 aPos;",
            "layout(location = 1) in vec2 aTexCoord;",
            "layout(location = 2) in vec4 aColor;",
            "out vec2 TexCoord;",
            "uniform mat4 projection;",
            "void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); TexCoord = aTexCoord; }",
            "");
    private static final String FRAGMENT_SOURCE = String.join("\n",
            "#version 300 es",
            "precision highp float;",
            "in vec2 TexCoord;",
            "out vec4 FragColor;",
            "void main() { FragColor = vec4(1.0, 0.0, 0.0, 1.0); }",
            "");

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
}
