package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Graphics.BasicEffect;
import Microsoft.Xna.Framework.Graphics.BufferUsage;
import Microsoft.Xna.Framework.Graphics.DynamicIndexBuffer;
import Microsoft.Xna.Framework.Graphics.DynamicVertexBuffer;
import Microsoft.Xna.Framework.Graphics.IndexElementSize;
import Microsoft.Xna.Framework.Graphics.VertexPositionColor;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

/** Isolates process teardown with a live native ownership graph from the Gradle test worker. */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class NativeShutdownSubprocessTests {

    private static ShutdownGame retainedGame;

    @Test
    void jvmShutdownWithLiveEffectChildrenBuffersAndCallbacksExitsCleanly() throws Exception {
        List<String> command = new ArrayList<>();
        command.add(Path.of(System.getProperty("java.home"), "bin", "java").toString());
        String jniLibrary = System.getProperty("cna.java.jniLibrary");
        if (jniLibrary != null && !jniLibrary.isBlank()) {
            command.add("-Dcna.java.jniLibrary=" + jniLibrary);
        }
        command.add("-cp");
        command.add(System.getProperty("java.class.path"));
        command.add(NativeShutdownSubprocessTests.class.getName());
        command.add("child");

        Process process = new ProcessBuilder(command)
                .redirectErrorStream(true)
                .start();
        if (!process.waitFor(30, TimeUnit.SECONDS)) {
            process.destroyForcibly();
            throw new AssertionError("Native shutdown subprocess did not terminate within 30 seconds");
        }
        String output = new String(
                process.getInputStream().readAllBytes(), StandardCharsets.UTF_8);
        // The Java-side behaviour under test happens before the exit: the graph is built, the
        // frame runs, and the marker is printed. That is asserted first and unconditionally,
        // because it is what this suite is for.
        assertTrue(output.contains("CNA_JAVA_SHUTDOWN_GRAPH_READY"), output);

        // Every renderer, the EasyGL family included. On those a process that exited while a
        // vertex buffer was alive and the thread that created it had ended used to abort in a
        // static destructor -- JAVA-UPSTREAM-014, which every JVM satisfies because the launcher
        // runs main on a thread it creates. CNA fixed it (BINDFIX-041), and
        // tools/native-abi/probes/exit_with_live_graph.c reproduces the shape with no Java in it.
        assertEquals(0, process.exitValue(),
                "exit with a live graph on " + rendererOf(output) + ": " + output);
    }

    /** The renderer the child really used, which it reports rather than the parent assuming. */
    private static String rendererOf(String output) {
        for (String line : output.split("\n")) {
            if (line.startsWith(RENDERER_MARKER)) {
                return line.substring(RENDERER_MARKER.length()).trim();
            }
        }
        return "";
    }

    private static final String RENDERER_MARKER = "CNA_JAVA_SHUTDOWN_RENDERER=";

    public static void main(String[] arguments) throws IOException {
        if (arguments.length != 1 || !"child".equals(arguments[0])) {
            throw new IllegalArgumentException("Expected the child marker");
        }
        retainedGame = new ShutdownGame();
        retainedGame.RunOneFrame();
        if (!retainedGame.completed) {
            throw new IllegalStateException("Shutdown ownership graph was not initialized");
        }
        System.out.println(RENDERER_MARKER + retainedGame.rendererName);
        System.out.println("CNA_JAVA_SHUTDOWN_GRAPH_READY");
        // Deliberately retain the live graph. Natural JVM/process teardown is the behavior under test.
    }

    private static final class ShutdownGame extends Game {
        private boolean completed;
        private String rendererName = "";
        private BasicEffect effect;
        private DynamicVertexBuffer vertices;
        private DynamicIndexBuffer indices;

        @Override
        protected void Update(GameTime gameTime) {
            effect = new BasicEffect(getGraphicsDevice());
            effect.getDirectionalLight0().setEnabled(true);
            effect.getCurrentTechnique().getPasses().get(0);

            vertices = new DynamicVertexBuffer(
                    getGraphicsDevice(), VertexPositionColor.class, 3, BufferUsage.WriteOnly);
            indices = new DynamicIndexBuffer(
                    getGraphicsDevice(), IndexElementSize.SixteenBits, 3, BufferUsage.WriteOnly);
            vertices.addContentLostListener((sender, args) -> { });
            indices.addContentLostListener((sender, args) -> { });
            getGraphicsDevice().SetVertexBuffer(vertices);
            getGraphicsDevice().setIndices(indices);
            rendererName = org.openeggbert.cna.extensions.graphics.RendererCapabilities
                    .getRendererName(getGraphicsDevice());
            completed = true;
        }
    }
}
