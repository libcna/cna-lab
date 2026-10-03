package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class GamerServicesComponentTests {

    @Test
    void constructorAndInheritedComponentStateRemainManaged() {
        try (Game game = new Game()) {
            GamerServicesComponent component = new GamerServicesComponent(game);
            AtomicInteger enabledEvents = new AtomicInteger();
            AtomicInteger orderEvents = new AtomicInteger();
            component.addEnabledChangedListener((sender, args) -> enabledEvents.incrementAndGet());
            component.addUpdateOrderChangedListener(
                    (sender, args) -> orderEvents.incrementAndGet());

            assertSame(game, component.getGame());
            component.setEnabled(false);
            component.setUpdateOrder(-7);

            assertFalse(component.getEnabled());
            assertEquals(-7, component.getUpdateOrder());
            assertEquals(1, enabledEvents.get());
            assertEquals(1, orderEvents.get());
        }
        assertThrows(NullPointerException.class, () -> new GamerServicesComponent(null));
    }

    /**
     * The dispatcher is process-wide, as XNA's is: the first game's component initializes it and
     * a later game's component is refused with XNA's InvalidOperationException. The test JVM runs
     * many suites, so this runs in a child JVM where the component's initialization is the
     * process's first.
     */
    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theComponentDrivesTheDispatcherAndASecondGameIsRefusedAsInXna() throws Exception {
        List<String> command = new ArrayList<>();
        command.add(java.nio.file.Path.of(System.getProperty("java.home"), "bin", "java")
                .toString());
        String jniLibrary = System.getProperty("cna.java.jniLibrary");
        if (jniLibrary != null && !jniLibrary.isBlank()) {
            command.add("-Dcna.java.jniLibrary=" + jniLibrary);
        }
        command.add("-cp");
        command.add(System.getProperty("java.class.path"));
        command.add(GamerServicesComponentTests.class.getName());
        command.add("child");
        Process process = new ProcessBuilder(command).redirectErrorStream(true).start();
        if (!process.waitFor(60, java.util.concurrent.TimeUnit.SECONDS)) {
            process.destroyForcibly();
            throw new AssertionError("the child did not finish within 60 seconds");
        }
        String output = new String(process.getInputStream().readAllBytes(),
                java.nio.charset.StandardCharsets.UTF_8);
        assertEquals(0, process.exitValue(), output);
        assertTrue(output.contains(CHILD_PASSED), output);
    }

    private static final String CHILD_PASSED = "CNA_JAVA_GAMER_COMPONENT_PASSED";

    public static void main(String[] arguments) {
        if (arguments.length != 1 || !"child".equals(arguments[0])) {
            throw new IllegalArgumentException("Expected the child marker");
        }
        try (Game game = new Game()) {
            List<String> updateOrder = new ArrayList<>();
            TrackingComponent component = new TrackingComponent(game, updateOrder);
            OrderProbe probe = new OrderProbe(game, updateOrder);
            component.setUpdateOrder(10);
            probe.setUpdateOrder(-10);
            component.setEnabled(false);
            game.getComponents().add(component);
            game.getComponents().add(probe);

            game.RunOneFrame();
            assertEquals(1, component.initializeCount);
            assertEquals(0, component.updateCount);
            assertEquals(List.of("probe"), updateOrder);
            assertTrue(GamerServicesDispatcher.getIsInitialized());

            updateOrder.clear();
            component.setEnabled(true);
            game.RunOneFrame();
            game.RunOneFrame();
            assertEquals(1, component.initializeCount);
            assertEquals(2, component.updateCount);
            assertEquals(List.of("probe", "gamer", "probe", "gamer"), updateOrder);
        }
        try (Game second = new Game()) {
            GamerServicesComponent again = new GamerServicesComponent(second);
            second.getComponents().add(again);
            RuntimeException refused = assertThrows(RuntimeException.class, second::RunOneFrame);
            assertTrue(causedBy(refused, IllegalStateException.class),
                    "a second initialization is XNA's InvalidOperationException: " + refused);
        }
        System.out.println(CHILD_PASSED);
    }

    private static boolean causedBy(Throwable failure, Class<? extends Throwable> type) {
        for (Throwable cause = failure; cause != null; cause = cause.getCause()) {
            if (type.isInstance(cause) || String.valueOf(cause.getMessage())
                    .contains("already initialized")) {
                return true;
            }
        }
        return false;
    }

    private static final class TrackingComponent extends GamerServicesComponent {
        private int initializeCount;
        private int updateCount;
        private final List<String> updateOrder;

        private TrackingComponent(Game game, List<String> updateOrder) {
            super(game);
            this.updateOrder = updateOrder;
        }

        @Override
        public void Initialize() {
            super.Initialize();
            initializeCount++;
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            updateCount++;
            updateOrder.add("gamer");
        }
    }

    private static final class OrderProbe extends GameComponent {
        private final List<String> updateOrder;

        private OrderProbe(Game game, List<String> updateOrder) {
            super(game);
            this.updateOrder = updateOrder;
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            updateOrder.add("probe");
        }
    }
}
