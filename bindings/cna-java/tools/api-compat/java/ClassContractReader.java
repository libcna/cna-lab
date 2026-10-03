// SPDX-License-Identifier: MS-PL

package org.openeggbert.cna.tools.apicompat;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Parameter;
import java.lang.reflect.Type;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/** Reads compiled JVM metadata; source text is never inspected. */
public final class ClassContractReader {

    private ClassContractReader() {
    }

    public static void main(String[] arguments) throws Exception {
        if (arguments.length < 2) {
            System.err.println("usage: ClassContractReader <output-json> <class-dir-or-jar>...");
            System.exit(2);
        }
        Path output = Path.of(arguments[0]);
        List<Path> inputs = Arrays.stream(arguments).skip(1).map(Path::of).toList();
        URL[] urls = inputs.stream().map(ClassContractReader::url).toArray(URL[]::new);
        TreeSet<String> names = new TreeSet<>();
        for (Path input : inputs) {
            discover(input, names);
        }

        List<Object> types = new ArrayList<>();
        try (URLClassLoader loader = new URLClassLoader(urls, ClassLoader.getPlatformClassLoader())) {
            for (String name : names) {
                Class<?> type = Class.forName(name, false, loader);
                if (isVisible(type)) {
                    types.add(readType(type));
                }
            }
        }

        Map<String, Object> root = new LinkedHashMap<>();
        root.put("schemaVersion", 1);
        root.put("profile", "compiled cna-java target");
        root.put("types", types);
        Files.writeString(output, Json.write(root), StandardCharsets.UTF_8);
        int memberCount = types.stream().mapToInt(value -> ((List<?>)((Map<?, ?>)value).get("members")).size()).sum();
        System.out.println("TARGET_TYPES=" + types.size());
        System.out.println("TARGET_MEMBERS=" + memberCount);
    }

    private static Map<String, Object> readType(Class<?> type) {
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("name", type.getName());
        result.put("kind", type.isEnum() ? "enum" : type.isInterface() ? "interface" : "class");
        result.put("access", Modifier.isPublic(type.getModifiers()) ? "public" : "protected");
        result.put("abstract", Modifier.isAbstract(type.getModifiers()));
        result.put("sealed", Modifier.isFinal(type.getModifiers()));
        result.put("genericArity", type.getTypeParameters().length);
        result.put("baseType", type.getGenericSuperclass() == null ? null : typeName(type.getGenericSuperclass()));
        result.put("interfaces", Arrays.stream(type.getGenericInterfaces()).map(ClassContractReader::typeName).sorted().toList());

        List<Map<String, Object>> members = new ArrayList<>();
        for (Constructor<?> constructor : type.getDeclaredConstructors()) {
            if (isVisible(constructor.getModifiers()) && !constructor.isSynthetic()) {
                members.add(readConstructor(constructor));
            }
        }
        for (Method method : type.getDeclaredMethods()) {
            if (isVisible(method.getModifiers()) && !method.isSynthetic() && !method.isBridge()
                    && !isMandatoryEnumMethod(type, method)) {
                members.add(readMethod(method));
            }
        }
        for (Field field : type.getDeclaredFields()) {
            if (isVisible(field.getModifiers()) && !field.isSynthetic()) {
                members.add(readField(field));
            }
        }
        members.sort(Comparator.comparing(member -> member.get("kind") + ":" + member.get("name") + ":" + Json.write(member)));
        result.put("members", members);
        return result;
    }

    private static Map<String, Object> readConstructor(Constructor<?> constructor) {
        Map<String, Object> result = callable("constructor", ".ctor", constructor.getModifiers(),
                constructor.getTypeParameters().length, null, constructor.getParameters(), constructor.getGenericParameterTypes());
        return result;
    }

    private static Map<String, Object> readMethod(Method method) {
        return callable("method", method.getName(), method.getModifiers(), method.getTypeParameters().length,
                typeName(method.getGenericReturnType()), method.getParameters(), method.getGenericParameterTypes());
    }

    private static Map<String, Object> callable(
            String kind,
            String name,
            int modifiers,
            int genericArity,
            String returnType,
            Parameter[] parameters,
            Type[] parameterTypes) {
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("kind", kind);
        result.put("name", name);
        result.put("access", Modifier.isPublic(modifiers) ? "public" : "protected");
        result.put("static", Modifier.isStatic(modifiers));
        result.put("abstract", Modifier.isAbstract(modifiers));
        result.put("final", Modifier.isFinal(modifiers));
        result.put("genericArity", genericArity);
        result.put("returnType", returnType);
        List<Object> values = new ArrayList<>();
        for (int index = 0; index < parameters.length; index++) {
            Map<String, Object> parameter = new LinkedHashMap<>();
            parameter.put("name", parameters[index].isNamePresent() ? parameters[index].getName() : "");
            parameter.put("type", typeName(parameterTypes[index]));
            parameter.put("out", false);
            parameter.put("optional", false);
            values.add(parameter);
        }
        result.put("parameters", values);
        return result;
    }

    private static Map<String, Object> readField(Field field) {
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("kind", "field");
        result.put("name", field.getName());
        result.put("access", Modifier.isPublic(field.getModifiers()) ? "public" : "protected");
        result.put("type", typeName(field.getGenericType()));
        result.put("static", Modifier.isStatic(field.getModifiers()));
        result.put("final", Modifier.isFinal(field.getModifiers()));
        Object constant = null;
        if (field.isEnumConstant()
                || Modifier.isStatic(field.getModifiers()) && Modifier.isFinal(field.getModifiers())
                && field.getType() == field.getDeclaringClass()) {
            constant = readNamedValue(field);
        } else if (Modifier.isStatic(field.getModifiers()) && Modifier.isFinal(field.getModifiers())
                && (field.getType().isPrimitive() || field.getType() == String.class)) {
            Object value = readStatic(field);
            constant = value == null ? null : value.toString();
        }
        result.put("constant", constant);
        return result;
    }

    private static Object readNamedValue(Field field) {
        Object value = readStatic(field);
        try {
            Method getValue = field.getType().getDeclaredMethod("getValue");
            if (Modifier.isPublic(getValue.getModifiers()) && getValue.getParameterCount() == 0) {
                Object projected = getValue.invoke(value);
                return projected == null ? null : projected.toString();
            }
        } catch (NoSuchMethodException ignored) {
            // Sequential Java enums need no explicit numeric accessor.
        } catch (ReflectiveOperationException error) {
            throw new IllegalStateException("Cannot read projected named value " + field, error);
        }
        return field.isEnumConstant() ? Integer.toString(((Enum<?>)value).ordinal()) : null;
    }

    private static Object readStatic(Field field) {
        try {
            return field.get(null);
        } catch (IllegalAccessException error) {
            throw new IllegalStateException(error);
        }
    }

    private static boolean isMandatoryEnumMethod(Class<?> type, Method method) {
        return type.isEnum() && (method.getName().equals("values") && method.getParameterCount() == 0
                || method.getName().equals("valueOf") && method.getParameterCount() == 1);
    }

    private static String typeName(Type type) {
        return type.getTypeName().replace('$', '.').replace(", ", ",");
    }

    private static boolean isVisible(Class<?> type) {
        return isVisible(type.getModifiers());
    }

    private static boolean isVisible(int modifiers) {
        return Modifier.isPublic(modifiers) || Modifier.isProtected(modifiers);
    }

    private static void discover(Path input, Collection<String> names) throws IOException {
        if (Files.isDirectory(input)) {
            try (var paths = Files.walk(input)) {
                paths.filter(path -> path.toString().endsWith(".class"))
                        .map(path -> input.relativize(path).toString())
                        .map(ClassContractReader::className)
                        .filter(ClassContractReader::strictName)
                        .forEach(names::add);
            }
        } else {
            try (JarFile jar = new JarFile(input.toFile())) {
                jar.stream().map(JarEntry::getName).filter(name -> name.endsWith(".class"))
                        .map(ClassContractReader::className)
                        .filter(ClassContractReader::strictName)
                        .forEach(names::add);
            }
        }
    }

    private static String className(String value) {
        return value.substring(0, value.length() - ".class".length()).replace('/', '.').replace('\\', '.');
    }

    private static boolean strictName(String name) {
        return name.startsWith("Microsoft.Xna.Framework.") || name.equals("Microsoft.Xna.Framework")
                ? !name.endsWith("package-info") && !name.endsWith("module-info") : false;
    }

    private static URL url(Path path) {
        try {
            return path.toUri().toURL();
        } catch (IOException error) {
            throw new IllegalArgumentException(error);
        }
    }

    private static final class Json {
        private Json() {
        }

        static String write(Object value) {
            StringBuilder output = new StringBuilder();
            append(output, value);
            return output.toString();
        }

        private static void append(StringBuilder output, Object value) {
            if (value == null) {
                output.append("null");
            } else if (value instanceof String text) {
                string(output, text);
            } else if (value instanceof Number || value instanceof Boolean) {
                output.append(value);
            } else if (value instanceof Map<?, ?> map) {
                output.append('{');
                boolean first = true;
                for (Map.Entry<?, ?> entry : map.entrySet()) {
                    if (!first) output.append(',');
                    first = false;
                    string(output, entry.getKey().toString());
                    output.append(':');
                    append(output, entry.getValue());
                }
                output.append('}');
            } else if (value instanceof Iterable<?> iterable) {
                output.append('[');
                boolean first = true;
                for (Object item : iterable) {
                    if (!first) output.append(',');
                    first = false;
                    append(output, item);
                }
                output.append(']');
            } else {
                string(output, value.toString());
            }
        }

        private static void string(StringBuilder output, String value) {
            output.append('"');
            for (int index = 0; index < value.length(); index++) {
                char character = value.charAt(index);
                switch (character) {
                    case '"' -> output.append("\\\"");
                    case '\\' -> output.append("\\\\");
                    case '\b' -> output.append("\\b");
                    case '\f' -> output.append("\\f");
                    case '\n' -> output.append("\\n");
                    case '\r' -> output.append("\\r");
                    case '\t' -> output.append("\\t");
                    default -> {
                        if (character < 0x20) output.append(String.format("\\u%04x", (int)character));
                        else output.append(character);
                    }
                }
            }
            output.append('"');
        }
    }
}
