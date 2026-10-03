package Microsoft.Xna.Framework.Media;

final class MediaObjectCoreFailure {
    private MediaObjectCoreFailure() { }

    static void rethrow(Throwable failure, String subject) {
        if (failure == null) return;
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        throw new IllegalStateException("Failed to release " + subject, failure);
    }
}
