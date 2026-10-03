package System;

/** Java carrier for the completed-result shape used by XNA's Begin/End storage APIs. */
public interface IAsyncResult {
    Object getAsyncState();
    boolean getCompletedSynchronously();
    boolean getIsCompleted();
}
