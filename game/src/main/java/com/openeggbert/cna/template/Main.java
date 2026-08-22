package com.openeggbert.cna.template;

/** Desktop entry point with deterministic CI frame modes. */
public final class Main {

    private static final int SMOKE_FRAMES = 60;
    private static final int STABILITY_FRAMES = 600;

    private Main() {
    }

    public static void main(String[] arguments) {
        int frames = parseFrameLimit(arguments);
        try (HelloGame game = new HelloGame(frames)) {
            game.Run();
        } catch (RuntimeException | LinkageError failure) {
            failure.printStackTrace(System.err);
            System.exit(1);
        }
    }

    static int parseFrameLimit(String[] arguments) {
        int frames = 0;
        for (int index = 0; index < arguments.length; index++) {
            String argument = arguments[index];
            if ("--smoke-test".equals(argument)) {
                frames = SMOKE_FRAMES;
            } else if ("--stability-test".equals(argument)) {
                frames = STABILITY_FRAMES;
            } else if (argument.startsWith("--frames=")) {
                frames = positiveFrameCount(argument.substring("--frames=".length()));
            } else if ("--frames".equals(argument)) {
                if (++index >= arguments.length) {
                    throw new IllegalArgumentException("--frames requires a positive integer");
                }
                frames = positiveFrameCount(arguments[index]);
            } else {
                throw new IllegalArgumentException("Unknown argument: " + argument);
            }
        }
        return frames;
    }

    private static int positiveFrameCount(String value) {
        try {
            int frames = Integer.parseInt(value);
            if (frames <= 0) {
                throw new IllegalArgumentException("frame count must be positive: " + value);
            }
            return frames;
        } catch (NumberFormatException exception) {
            throw new IllegalArgumentException("invalid frame count: " + value, exception);
        }
    }
}
