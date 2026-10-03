package Microsoft.Xna.Framework.Media;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** Deterministic Media observations mirrored from XNA metadata and reference-runtime probes. */
final class XnaMediaBehaviorCorpus {

    private XnaMediaBehaviorCorpus() {
    }

    static List<String> capture() {
        ArrayList<String> observations = new ArrayList<>();
        observations.add("media.state=" + join(MediaState.values()));
        observations.add("media.source=" + MediaSourceType.LocalDevice + ':'
                + MediaSourceType.LocalDevice.getValue() + ','
                + MediaSourceType.WindowsMediaConnect + ':'
                + MediaSourceType.WindowsMediaConnect.getValue());
        observations.add("video.soundtrack=" + join(VideoSoundtrackType.values()));

        VisualizationData data = new VisualizationData();
        observations.add("media.visualization.count=" + data.getFrequencies().size()
                + ',' + data.getSamples().size());
        observations.add("media.visualization.identity="
                + flag(data.getFrequencies() == data.getFrequencies()) + ','
                + flag(data.getSamples() == data.getSamples()));
        observations.add("media.visualization.zero="
                + flag(data.getFrequencies().stream().allMatch(value -> value == 0f)) + ','
                + flag(data.getSamples().stream().allMatch(value -> value == 0f)));
        observations.add("media.visualization.frequency.oob="
                + exceptionName(() -> data.getFrequencies().get(256)));
        observations.add("media.visualization.sample.oob="
                + exceptionName(() -> data.getSamples().get(256)));
        observations.add("media.visualization.frequency.readonly="
                + exceptionName(() -> data.getFrequencies().set(0, 1f)));
        observations.add("media.visualization.sample.readonly="
                + exceptionName(() -> data.getSamples().set(0, 1f)));
        return List.copyOf(observations);
    }

    private static String join(Object[] values) {
        return String.join(",", Arrays.stream(values).map(Object::toString).toList());
    }

    private static String flag(boolean value) {
        return value ? "1" : "0";
    }

    private static String exceptionName(Runnable operation) {
        try {
            operation.run();
            return "none";
        } catch (RuntimeException exception) {
            return exception.getClass().getSimpleName();
        }
    }
}
