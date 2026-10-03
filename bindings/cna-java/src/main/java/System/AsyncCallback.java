package System;

/** Callback used by XNA's synchronously completed storage Begin methods. */
@FunctionalInterface
public interface AsyncCallback {
    void invoke(IAsyncResult result);
}
