// SPDX-License-Identifier: MS-PL

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Web.Script.Serialization;

internal static class XnaContractExtractor
{
    private const BindingFlags DeclaredMembers = BindingFlags.DeclaredOnly | BindingFlags.Public |
        BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static;

    private static string referenceDirectory = "";

    public static int Main(string[] args)
    {
        if (args.Length < 3)
        {
            Console.Error.WriteLine("usage: XnaContractExtractor <reference-dir> <output-json> <assembly>...");
            return 2;
        }

        referenceDirectory = Path.GetFullPath(args[0]);
        AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;

        var types = new Dictionary<string, Type>(StringComparer.Ordinal);
        foreach (string assemblyName in args.Skip(2))
        {
            string path = Path.Combine(referenceDirectory, assemblyName);
            if (!File.Exists(path))
            {
                Console.Error.WriteLine("missing XNA reference assembly: " + path);
                return 2;
            }
            Assembly assembly = Assembly.LoadFrom(path);
            foreach (Type type in SafeTypes(assembly))
            {
                if (IsContractType(type))
                {
                    types[type.FullName] = type;
                }
            }
        }

        var contractTypes = types.Values.OrderBy(type => type.FullName, StringComparer.Ordinal)
            .Select(ReadType).ToList();
        var root = new Dictionary<string, object>
        {
            ["schemaVersion"] = 1,
            ["profile"] = "XNA 4.0 Windows runtime",
            ["types"] = contractTypes
        };
        var serializer = new JavaScriptSerializer { MaxJsonLength = Int32.MaxValue, RecursionLimit = 256 };
        File.WriteAllText(args[1], serializer.Serialize(root));
        Console.WriteLine("REFERENCE_TYPES=" + contractTypes.Count);
        Console.WriteLine("REFERENCE_MEMBERS=" + contractTypes.Sum(type => ((List<object>)type["members"]).Count));
        return 0;
    }

    private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
    {
        string candidate = Path.Combine(referenceDirectory, new AssemblyName(args.Name).Name + ".dll");
        return File.Exists(candidate) ? Assembly.LoadFrom(candidate) : null;
    }

    private static IEnumerable<Type> SafeTypes(Assembly assembly)
    {
        try { return assembly.GetTypes(); }
        catch (ReflectionTypeLoadException error) { return error.Types.Where(type => type != null); }
    }

    private static bool IsContractType(Type type)
    {
        return IsEffectivelyVisible(type) && type.FullName != null &&
            type.FullName.StartsWith("Microsoft.Xna.Framework", StringComparison.Ordinal);
    }

    private static bool IsEffectivelyVisible(Type type)
    {
        if (type.IsPublic) return true;
        bool nestedVisible = type.IsNestedPublic || type.IsNestedFamily || type.IsNestedFamORAssem;
        return nestedVisible && type.DeclaringType != null && IsEffectivelyVisible(type.DeclaringType);
    }

    private static Dictionary<string, object> ReadType(Type type)
    {
        var members = new List<object>();
        foreach (ConstructorInfo constructor in type.GetConstructors(DeclaredMembers).Where(IsVisible))
            members.Add(ReadCallable("constructor", ".ctor", constructor, null));
        foreach (MethodInfo method in type.GetMethods(DeclaredMembers).Where(IsVisible))
        {
            if (!method.IsSpecialName || method.Name.StartsWith("op_", StringComparison.Ordinal))
                members.Add(ReadCallable("method", method.Name, method, method.ReturnType));
        }
        foreach (PropertyInfo property in type.GetProperties(DeclaredMembers))
        {
            MethodInfo getter = property.GetGetMethod(true);
            MethodInfo setter = property.GetSetMethod(true);
            if ((getter != null && IsVisible(getter)) || (setter != null && IsVisible(setter)))
                members.Add(ReadProperty(property, getter, setter));
        }
        foreach (EventInfo eventInfo in type.GetEvents(DeclaredMembers))
        {
            MethodInfo adder = eventInfo.GetAddMethod(true);
            MethodInfo remover = eventInfo.GetRemoveMethod(true);
            if ((adder != null && IsVisible(adder)) || (remover != null && IsVisible(remover)))
                members.Add(ReadEvent(eventInfo, adder, remover));
        }
        foreach (FieldInfo field in type.GetFields(DeclaredMembers).Where(IsVisible))
            members.Add(ReadField(field));

        List<object> explicitInterfaceMethods = ReadExplicitInterfaceMethods(type);

        return new Dictionary<string, object>
        {
            ["name"] = type.FullName,
            ["kind"] = Kind(type),
            ["access"] = TypeAccess(type),
            ["abstract"] = type.IsAbstract,
            ["sealed"] = type.IsSealed,
            ["flags"] = type.IsEnum && type.IsDefined(typeof(FlagsAttribute), false),
            ["genericArity"] = type.IsGenericTypeDefinition ? type.GetGenericArguments().Length : 0,
            ["baseType"] = TypeName(type.BaseType),
            ["interfaces"] = DirectInterfaces(type).Select(TypeName).OrderBy(name => name, StringComparer.Ordinal).ToList(),
            ["explicitInterfaceMethods"] = explicitInterfaceMethods,
            ["members"] = members.OrderBy(MemberSortKey, StringComparer.Ordinal).ToList()
        };
    }

    private static List<object> ReadExplicitInterfaceMethods(Type type)
    {
        var methods = new Dictionary<string, object>(StringComparer.Ordinal);
        if (type.IsInterface)
        {
            return methods.Values.ToList();
        }
        foreach (Type interfaceType in type.GetInterfaces().Where(IsContractType))
        {
            InterfaceMapping mapping = type.GetInterfaceMap(interfaceType);
            for (int index = 0; index < mapping.InterfaceMethods.Length; index++)
            {
                MethodInfo interfaceMethod = mapping.InterfaceMethods[index];
                MethodInfo targetMethod = mapping.TargetMethods[index];
                if (targetMethod.DeclaringType != type || IsVisible(targetMethod))
                {
                    continue;
                }

                string projectedName = interfaceMethod.Name;
                if (interfaceMethod.IsSpecialName)
                {
                    if (projectedName.StartsWith("get_", StringComparison.Ordinal))
                        projectedName = "get" + projectedName.Substring(4);
                    else if (projectedName.StartsWith("set_", StringComparison.Ordinal))
                        projectedName = "set" + projectedName.Substring(4);
                    else if (projectedName.StartsWith("add_", StringComparison.Ordinal))
                        projectedName = "add" + projectedName.Substring(4) + "Listener";
                    else if (projectedName.StartsWith("remove_", StringComparison.Ordinal))
                        projectedName = "remove" + projectedName.Substring(7) + "Listener";
                    else
                        continue;
                }

                Dictionary<string, object> promoted = ReadCallable(
                    "method", projectedName, targetMethod, interfaceMethod.ReturnType);
                promoted["access"] = "public";
                promoted["parameters"] = interfaceMethod.GetParameters().Select(ReadParameter).ToList();
                methods[MemberSortKey(promoted)] = promoted;
            }
        }
        return methods.Values.OrderBy(MemberSortKey, StringComparer.Ordinal).ToList();
    }

    private static Dictionary<string, object> ReadCallable(string kind, string name, MethodBase callable, Type returnType)
    {
        return new Dictionary<string, object>
        {
            ["kind"] = kind,
            ["name"] = name,
            ["access"] = Access(callable),
            ["static"] = callable.IsStatic,
            ["abstract"] = callable.IsAbstract,
            ["final"] = callable.IsFinal,
            ["virtual"] = callable.IsVirtual,
            ["genericArity"] = callable.IsGenericMethodDefinition ? callable.GetGenericArguments().Length : 0,
            ["returnType"] = TypeName(returnType),
            ["parameters"] = callable.GetParameters().Select(ReadParameter).ToList()
        };
    }

    private static Dictionary<string, object> ReadParameter(ParameterInfo parameter)
    {
        return new Dictionary<string, object>
        {
            ["name"] = parameter.Name ?? "",
            ["type"] = TypeName(parameter.ParameterType),
            ["out"] = parameter.IsOut,
            ["optional"] = parameter.IsOptional
        };
    }

    private static Dictionary<string, object> ReadProperty(PropertyInfo property, MethodInfo getter, MethodInfo setter)
    {
        return new Dictionary<string, object>
        {
            ["kind"] = "property",
            ["name"] = property.Name,
            ["type"] = TypeName(property.PropertyType),
            ["static"] = (getter ?? setter).IsStatic,
            ["getterAccess"] = getter == null ? "none" : Access(getter),
            ["setterAccess"] = setter == null ? "none" : Access(setter),
            ["getterAbstract"] = getter != null && getter.IsAbstract,
            ["setterAbstract"] = setter != null && setter.IsAbstract,
            ["getterFinal"] = getter != null && getter.IsFinal,
            ["setterFinal"] = setter != null && setter.IsFinal,
            ["getterVirtual"] = getter != null && getter.IsVirtual,
            ["setterVirtual"] = setter != null && setter.IsVirtual,
            ["parameters"] = property.GetIndexParameters().Select(ReadParameter).ToList()
        };
    }

    private static Dictionary<string, object> ReadEvent(EventInfo eventInfo, MethodInfo adder, MethodInfo remover)
    {
        return new Dictionary<string, object>
        {
            ["kind"] = "event",
            ["name"] = eventInfo.Name,
            ["type"] = TypeName(eventInfo.EventHandlerType),
            ["static"] = (adder ?? remover).IsStatic,
            ["addAccess"] = adder == null ? "none" : Access(adder),
            ["removeAccess"] = remover == null ? "none" : Access(remover),
            ["addAbstract"] = adder != null && adder.IsAbstract,
            ["removeAbstract"] = remover != null && remover.IsAbstract,
            ["addFinal"] = adder != null && adder.IsFinal,
            ["removeFinal"] = remover != null && remover.IsFinal,
            ["addVirtual"] = adder != null && adder.IsVirtual,
            ["removeVirtual"] = remover != null && remover.IsVirtual
        };
    }

    private static Dictionary<string, object> ReadField(FieldInfo field)
    {
        object constant = null;
        if (field.IsLiteral)
        {
            try { constant = Convert.ToString(field.GetRawConstantValue(), System.Globalization.CultureInfo.InvariantCulture); }
            catch (InvalidOperationException) { constant = null; }
        }
        return new Dictionary<string, object>
        {
            ["kind"] = "field",
            ["name"] = field.Name,
            ["access"] = Access(field),
            ["type"] = TypeName(field.FieldType),
            ["static"] = field.IsStatic,
            ["final"] = field.IsInitOnly || field.IsLiteral,
            ["constant"] = constant
        };
    }

    private static bool IsVisible(MethodBase method) { return method.IsPublic || method.IsFamily || method.IsFamilyOrAssembly; }
    private static bool IsVisible(FieldInfo field) { return field.IsPublic || field.IsFamily || field.IsFamilyOrAssembly; }

    private static string Kind(Type type)
    {
        if (type.IsEnum) return "enum";
        if (type.IsInterface) return "interface";
        if (type.BaseType == typeof(MulticastDelegate)) return "delegate";
        if (type.IsValueType) return "struct";
        return "class";
    }

    private static IEnumerable<Type> DirectInterfaces(Type type)
    {
        var inherited = new HashSet<Type>();
        if (type.BaseType != null)
        {
            foreach (Type value in type.BaseType.GetInterfaces()) inherited.Add(value);
        }
        foreach (Type value in type.GetInterfaces())
        {
            foreach (Type parent in value.GetInterfaces()) inherited.Add(parent);
        }
        return type.GetInterfaces().Where(value => !inherited.Contains(value));
    }

    private static string TypeAccess(Type type)
    {
        if (type.IsPublic || type.IsNestedPublic) return "public";
        return "protected";
    }

    private static string Access(MethodBase method)
    {
        if (method.IsPublic) return "public";
        return "protected";
    }

    private static string Access(FieldInfo field)
    {
        if (field.IsPublic) return "public";
        return "protected";
    }

    private static string TypeName(Type type)
    {
        if (type == null) return null;
        if (type.IsByRef) return TypeName(type.GetElementType()) + "&";
        if (type.IsArray) return TypeName(type.GetElementType()) + "[]";
        if (type.IsGenericParameter) return "!" + type.GenericParameterPosition;
        if (type.IsGenericType)
        {
            string definition = type.GetGenericTypeDefinition().FullName;
            return definition + "[" + String.Join(",", type.GetGenericArguments().Select(TypeName)) + "]";
        }
        return type.FullName ?? type.Name;
    }

    private static string MemberSortKey(object value)
    {
        var member = (Dictionary<string, object>)value;
        var serializer = new JavaScriptSerializer();
        return (string)member["kind"] + ":" + (string)member["name"] + ":" + serializer.Serialize(member);
    }
}
