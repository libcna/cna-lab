package org.openeggbert.cna.extensions.gamerservices;

/** Thrown when this CNA build carries no gamer-services layer for a Guide extension route. */
public final class GuideNotSupportedException extends UnsupportedOperationException {

    private static final long serialVersionUID = 1L;

    GuideNotSupportedException(String message) {
        super(message);
    }
}
