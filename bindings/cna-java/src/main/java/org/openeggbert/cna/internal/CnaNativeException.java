package org.openeggbert.cna.internal;

/** Internal exception carrying a fixed-width CNA result and its thread-local diagnostic. */
public final class CnaNativeException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    private final int result;

    CnaNativeException(String operation, int result, String diagnostic) {
        super(operation + " failed with CNA result " + result
                + (diagnostic == null || diagnostic.isBlank() ? "" : ": " + diagnostic));
        this.result = result;
    }

    public int getResult() {
        return result;
    }
}
