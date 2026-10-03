package org.openeggbert.cna.extensions.content;

import org.junit.jupiter.api.Test;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * What a model remembers about where it came from.
 *
 * <p>The measurement that matters here is the split: twelve counts are state a caller writes, and
 * five are answers CNA computes from the diagnostics every time they are asked for. A test that
 * only round-tripped the twelve would pass against an implementation that stored all seventeen,
 * so each of the five is provoked separately -- a warning entry, a dropped-data occurrence, an
 * unsupported-feature occurrence and an approximation -- and read back at a value nothing wrote.
 */
final class CnaGltfImportReportTests {

    private static GltfImportSourceCounts counts() {
        return new GltfImportSourceCounts(9L, 7L, 4L, 2L, 3L, 1L, 5L, 4L, 11L, 2L, 6L, 5L);
    }

    private static GltfImportDiagnostic diagnostic(String code, GltfImportSeverity severity,
            GltfImportKind kind, long count, double magnitude, List<String> details) {
        return new GltfImportDiagnostic(code, severity, kind, "mesh/0/primitive/1", count,
                magnitude, details, "the message for " + code);
    }

    @Test
    void aModelFromAnotherContentPathHasEmptyProvenanceRatherThanNone() {
        CnaModelProbe.run(model -> {
            GltfImportReport report = model.getGltfImportReport();
            assertEquals(GltfImportSourceCounts.EMPTY, report.SourceCounts(),
                    "an XNB-loaded model was not imported from a source scene, and says so with"
                            + " zeros rather than by refusing the question");
            assertEquals(0L, report.DiagnosticCount());
            assertFalse(report.AnythingLost());
            assertEquals(List.of(), model.getGltfImportDiagnostics());
        });
    }

    @Test
    void theTwelveWrittenCountsRoundTripAndTheFiveDerivedOnesStayZero() {
        CnaModelProbe.run(model -> {
            model.setGltfImportSourceCounts(counts());
            GltfImportReport report = model.getGltfImportReport();
            assertEquals(counts(), report.SourceCounts());
            assertEquals(0L, report.DiagnosticCount(), "nothing was added, so nothing is counted");
            assertEquals(0L, report.WarningCount());
            assertEquals(0L, report.DroppedFeatureCount());
            assertEquals(0L, report.ApproximationCount());
            assertFalse(report.AnythingLost(),
                    "counts alone are not a loss: only a warning entry makes one");
        });
    }

    @Test
    void aDiagnosticRoundTripsEveryFieldIncludingItsDetails() {
        CnaModelProbe.run(model -> {
            model.setGltfImportSourceCounts(counts());
            GltfImportDiagnostic added = diagnostic("texture-transform-baked",
                    GltfImportSeverity.Warning, GltfImportKind.Approximation, 3L, 0.125,
                    List.of("baseColorTexture", "normalTexture"));
            model.addGltfImportDiagnostic(added);

            List<GltfImportDiagnostic> read = model.getGltfImportDiagnostics();
            assertEquals(1, read.size());
            assertEquals(added, read.get(0),
                    "the code, severity, kind, subject, count, magnitude, details and message all"
                            + " survive the copy CNA makes of the borrowed strings");
        });
    }

    @Test
    void theFiveDerivedValuesAreComputedFromTheDiagnosticsRatherThanStored() {
        CnaModelProbe.run(model -> {
            model.setGltfImportSourceCounts(counts());
            model.addGltfImportDiagnostic(diagnostic("normals-generated",
                    GltfImportSeverity.Information, GltfImportKind.GeneratedData, 40L, 0.0,
                    List.of()));
            model.addGltfImportDiagnostic(diagnostic("vertex-colors-dropped",
                    GltfImportSeverity.Warning, GltfImportKind.DroppedData, 7L, 0.0, List.of()));
            model.addGltfImportDiagnostic(diagnostic("sheen-unsupported",
                    GltfImportSeverity.Warning, GltfImportKind.UnsupportedFeature, 2L, 0.0,
                    List.of()));
            model.addGltfImportDiagnostic(diagnostic("tangents-approximated",
                    GltfImportSeverity.Warning, GltfImportKind.Approximation, 5L, 0.5, List.of()));

            GltfImportReport report = model.getGltfImportReport();
            assertEquals(counts(), report.SourceCounts(),
                    "adding diagnostics does not disturb the counts");
            assertEquals(4L, report.DiagnosticCount());
            assertEquals(3L, report.WarningCount(),
                    "warnings are counted as entries, so the 7 and the 2 do not add up here");
            assertEquals(9L, report.DroppedFeatureCount(),
                    "dropped data and unsupported features are summed by occurrence: 7 + 2");
            assertEquals(5L, report.ApproximationCount(),
                    "approximations are summed by occurrence, and the informational entry's 40"
                            + " belongs to none of these sums");
            assertTrue(report.AnythingLost());
            assertNotEquals(0L, report.DiagnosticCount(),
                    "the four derived values were never written by this test");
        });
    }

    @Test
    void aReportCarryingADerivedValueIsRefusedRatherThanQuietlyDropped() {
        CnaModelProbe.run(model -> {
            // This is why setGltfImportSourceCounts takes twelve counts and not a whole report:
            // the ABI refuses the other five, so a Java API that accepted them would be offering
            // a call that cannot succeed.
            long[] leaves = GltfImportReport.toLeaves(counts());
            assertEquals(GltfImportReport.LEAVES, leaves.length);
            for (int leaf = GltfImportReport.WRITTEN_LEAVES; leaf < leaves.length; leaf++) {
                assertEquals(0L, leaves[leaf], "leaf " + leaf + " is derived, so it goes out zero");
                long[] tampered = leaves.clone();
                tampered[leaf] = 1L;
                assertNotEquals(0, NativeModelExtensionRoutes
                                .modelSetGltfImportReportExt(model.open(), tampered),
                        "leaf " + leaf + " is derived and CNA must refuse it");
            }
            assertEquals(0, NativeModelExtensionRoutes
                    .modelSetGltfImportReportExt(model.open(), leaves));
        });
    }

    @Test
    void rewritingTheCountsClearsTheDiagnosticsTheyNoLongerDescribe() {
        CnaModelProbe.run(model -> {
            model.setGltfImportSourceCounts(counts());
            model.addGltfImportDiagnostic(diagnostic("sheen-unsupported",
                    GltfImportSeverity.Warning, GltfImportKind.UnsupportedFeature, 2L, 0.0,
                    List.of()));
            assertEquals(1, model.getGltfImportDiagnostics().size());

            model.setGltfImportSourceCounts(GltfImportSourceCounts.EMPTY);
            assertEquals(List.of(), model.getGltfImportDiagnostics(),
                    "the diagnostics described the scene the old counts came from");
            assertFalse(model.getGltfImportReport().AnythingLost());
        });
    }

    @Test
    void aClosedModelAndAMissingDiagnosticAreRefused() {
        CnaModelProbe.run(model -> {
            assertThrows(NullPointerException.class,
                    () -> model.setGltfImportSourceCounts(null));
            assertThrows(NullPointerException.class,
                    () -> model.addGltfImportDiagnostic(null));
        });
        assertThrows(IllegalArgumentException.class,
                () -> new GltfImportSourceCounts(-1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L));
        assertThrows(NullPointerException.class, () -> new GltfImportDiagnostic("c",
                GltfImportSeverity.Warning, GltfImportKind.DroppedData, "s", 1L, 0.0, null, "m"));
    }
}
