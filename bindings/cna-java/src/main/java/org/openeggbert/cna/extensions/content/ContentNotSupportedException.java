package org.openeggbert.cna.extensions.content;

/**
 * Thrown when this CNA build cannot serve a content route at all.
 *
 * <p>Not the same as an asset that is missing or is not a model: those are ordinary failures with
 * their own identity. This means the route itself answered {@code CNA_RESULT_NOT_SUPPORTED}.
 */
public final class ContentNotSupportedException extends UnsupportedOperationException {

    private static final long serialVersionUID = 1L;

    ContentNotSupportedException(String message) {
        super(message);
    }
}
