package org.openeggbert.cna.content;

import Microsoft.Xna.Framework.Content.ContentTypeReader;

import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Supplier;

/** Explicit Java activation registry for custom XNB ContentTypeReader implementations. */
public final class ContentTypeReaderRegistry {

    private static final ConcurrentHashMap<String, Supplier<? extends ContentTypeReader>> READERS =
            new ConcurrentHashMap<>();

    private ContentTypeReaderRegistry() {
    }

    public static AutoCloseable register(
            String serializedReaderName,
            Supplier<? extends ContentTypeReader> factory) {
        String name = requireName(serializedReaderName);
        Supplier<? extends ContentTypeReader> value = Objects.requireNonNull(factory, "factory");
        if (READERS.putIfAbsent(name, value) != null) {
            throw new IllegalStateException("A content type reader is already registered as " + name);
        }
        return () -> READERS.remove(name, value);
    }

    public static ContentTypeReader create(String serializedReaderName) {
        String name = requireName(serializedReaderName);
        Supplier<? extends ContentTypeReader> factory = READERS.get(name);
        if (factory == null) {
            factory = READERS.get(stripAssemblyQualification(name));
        }
        return factory == null ? null : Objects.requireNonNull(
                factory.get(), "Content type reader factory returned null");
    }

    public static String stripAssemblyQualification(String name) {
        int depth = 0;
        for (int index = 0; index < name.length(); index++) {
            char value = name.charAt(index);
            if (value == '[') depth++;
            else if (value == ']') depth--;
            else if (value == ',' && depth == 0) return name.substring(0, index).trim();
        }
        return name.trim();
    }

    private static String requireName(String value) {
        String name = Objects.requireNonNull(value, "serializedReaderName").trim();
        if (name.isEmpty()) {
            throw new IllegalArgumentException("Serialized reader name must not be empty");
        }
        return name;
    }
}
