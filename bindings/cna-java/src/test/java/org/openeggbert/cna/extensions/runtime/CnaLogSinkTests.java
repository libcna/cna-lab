package org.openeggbert.cna.extensions.runtime;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Where CNA's own log lines go, measured by catching them in Java.
 *
 * <p>The sink is process-wide state and so is the minimum level, so every test here restores both
 * in a {@code finally} -- a test that left a sink installed would send the rest of the suite's
 * runtime logging into a dead object.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaLogSinkTests {

    /** One line as the sink saw it. */
    private record Line(LogLevel Level, LogCategory Category, String Message) {
    }

    private static void withSink(List<Line> lines, Runnable body) {
        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.setMinimumLevel(LogLevel.Trace);
            CnaLogger.setSink((level, category, message) ->
                    lines.add(new Line(level, category, message)));
            body.run();
        } finally {
            CnaLogger.resetSink();
            CnaLogger.setMinimumLevel(minimum);
        }
    }

    @Test
    void everyLineCnaWritesReachesTheJavaSinkWithItsLevelAndCategory() {
        List<Line> lines = new CopyOnWriteArrayList<>();
        withSink(lines, () -> {
            CnaLogger.Log(LogLevel.Warn, "a warning from the sink test", LogCategory.Test);
            CnaLogger.Info("an info line from the sink test", LogCategory.Application);
        });

        // CNA formats the line, so the message the sink receives contains what was logged
        // rather than being equal to it -- which is the honest assertion to make here.
        List<Line> mine = new ArrayList<>();
        for (Line line : lines) {
            if (line.Message().contains("from the sink test")) {
                mine.add(line);
            }
        }
        assertEquals(2, mine.size(), "both lines arrived: " + lines);
        assertEquals(LogLevel.Warn, mine.get(0).Level());
        assertEquals(LogCategory.Test, mine.get(0).Category());
        assertTrue(mine.get(0).Message().contains("a warning from the sink test"));
        assertEquals(LogLevel.Info, mine.get(1).Level());
        assertEquals(LogCategory.Application, mine.get(1).Category(),
                "the category travels with the line rather than being guessed from the level");
    }

    @Test
    void aSecondSinkReplacesTheFirstAndResettingStopsThemBoth() {
        List<Line> first = new CopyOnWriteArrayList<>();
        List<Line> second = new CopyOnWriteArrayList<>();
        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.setMinimumLevel(LogLevel.Trace);
            CnaLogger.setSink((level, category, message) ->
                    first.add(new Line(level, category, message)));
            CnaLogger.Info("only the first sink", LogCategory.Test);

            CnaLogger.setSink((level, category, message) ->
                    second.add(new Line(level, category, message)));
            CnaLogger.Info("only the second sink", LogCategory.Test);
        } finally {
            CnaLogger.resetSink();
            CnaLogger.setMinimumLevel(minimum);
        }
        CnaLogger.Info("after the reset", LogCategory.Test);

        assertTrue(first.stream().anyMatch(line -> line.Message().contains("only the first")),
                "the first sink saw the line written while it was installed");
        assertTrue(first.stream().noneMatch(line -> line.Message().contains("only the second")),
                "a replaced sink stops receiving lines");
        assertTrue(second.stream().anyMatch(line -> line.Message().contains("only the second")));
        assertTrue(second.stream().noneMatch(line -> line.Message().contains("after the reset")),
                "resetting returns CNA to its own stderr sink");
    }

    @Test
    void aSinkThatThrowsLosesItsOwnLineAndNothingElse() {
        List<Line> lines = new CopyOnWriteArrayList<>();
        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.setMinimumLevel(LogLevel.Trace);
            CnaLogger.setSink((level, category, message) -> {
                if (message.contains("explodes")) {
                    throw new IllegalStateException("a sink that throws");
                }
                lines.add(new Line(level, category, message));
            });
            // The exception is described on stderr and cleared: CNA's sink returns void and must
            // return normally, so there is nowhere to carry it, and a sink that throws while
            // reporting a line must not take the process down with it.
            CnaLogger.Info("this one explodes", LogCategory.Test);
            CnaLogger.Info("this one survives", LogCategory.Test);
        } finally {
            CnaLogger.resetSink();
            CnaLogger.setMinimumLevel(minimum);
        }
        assertTrue(lines.stream().anyMatch(line -> line.Message().contains("this one survives")),
                "logging carried on after a sink threw");
    }

    @Test
    void theMinimumLevelStillFiltersBeforeTheSinkSeesAnything() {
        List<Line> lines = new CopyOnWriteArrayList<>();
        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.setMinimumLevel(LogLevel.Warn);
            CnaLogger.setSink((level, category, message) ->
                    lines.add(new Line(level, category, message)));
            CnaLogger.Trace("below the threshold", LogCategory.Test);
            CnaLogger.Error("above the threshold", LogCategory.Test);
        } finally {
            CnaLogger.resetSink();
            CnaLogger.setMinimumLevel(minimum);
        }
        assertTrue(lines.stream().noneMatch(line -> line.Message().contains("below the")),
                "a sink is not a way around the minimum level");
        assertTrue(lines.stream().anyMatch(line -> line.Message().contains("above the")));
    }

    @Test
    void resettingWithNoSinkInstalledIsHowACallerGetsBackToTheDefault() {
        CnaLogger.resetSink();
        CnaLogger.resetSink();
        assertThrows(NullPointerException.class, () -> CnaLogger.setSink(null));
    }
}
