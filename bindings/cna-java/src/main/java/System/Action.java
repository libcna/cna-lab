package System;

/** Minimal strongly typed projection of CLR's single-argument Action delegate. */
@FunctionalInterface
public interface Action<T> {
    void invoke(T value);
}
