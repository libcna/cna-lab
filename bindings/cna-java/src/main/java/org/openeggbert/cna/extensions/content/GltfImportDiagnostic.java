package org.openeggbert.cna.extensions.content;

import java.util.List;
import java.util.Objects;

/**
 * One thing an importer noticed while turning a source scene into a model.
 *
 * <p>A CNA extension. XNA's content pipeline reported this kind of thing to a build log that the
 * runtime never saw; a model here carries its own provenance, so a game or a tool can say what
 * was approximated or dropped on the way in.
 *
 * @param Code a stable lower-case, hyphen-separated identifier a tool can branch on
 * @param Severity whether this is a note or an observable loss
 * @param Kind what the importer did with the data
 * @param Subject the primitive, node, clip or extension it concerns; may be empty
 * @param Count how many occurrences this entry represents
 * @param WorstMagnitude the largest measured magnitude, or zero when none applies
 * @param Details the individual affected names
 * @param Message the human-readable message
 */
public record GltfImportDiagnostic(String Code, GltfImportSeverity Severity, GltfImportKind Kind,
        String Subject, long Count, double WorstMagnitude, List<String> Details, String Message) {

    /** Copies the details and refuses the fields a diagnostic cannot be without. */
    public GltfImportDiagnostic {
        Objects.requireNonNull(Code, "Code");
        Objects.requireNonNull(Severity, "Severity");
        Objects.requireNonNull(Kind, "Kind");
        Objects.requireNonNull(Subject, "Subject");
        Objects.requireNonNull(Message, "Message");
        Details = List.copyOf(Objects.requireNonNull(Details, "Details"));
    }
}
