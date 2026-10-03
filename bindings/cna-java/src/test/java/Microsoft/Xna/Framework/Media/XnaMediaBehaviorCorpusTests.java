package Microsoft.Xna.Framework.Media;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

final class XnaMediaBehaviorCorpusTests {

    @Test
    void MatchesReferenceBackedNormalizedMediaObservations() {
        List<String> observations = XnaMediaBehaviorCorpus.capture();

        assertEquals(List.of(
                "media.state=Stopped,Playing,Paused",
                "media.source=LocalDevice:0,WindowsMediaConnect:4",
                "video.soundtrack=Music,Dialog,MusicAndDialog",
                "media.visualization.count=256,256",
                "media.visualization.identity=1,1",
                "media.visualization.zero=1,1",
                "media.visualization.frequency.oob=ArrayIndexOutOfBoundsException",
                "media.visualization.sample.oob=ArrayIndexOutOfBoundsException",
                "media.visualization.frequency.readonly=UnsupportedOperationException",
                "media.visualization.sample.readonly=UnsupportedOperationException"
        ), observations);
    }
}
