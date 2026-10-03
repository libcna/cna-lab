// Compile and execute this probe against the pinned Microsoft XNA Framework 4.0
// Windows runtime assemblies. Microsoft.Xna.Framework.Graphics.dll declares
// DisplayMode with no public constructor, so the probe reaches the real
// `assembly`-accessible .ctor through reflection rather than adding surface to
// the source contract. No Microsoft binary is copied into this repository or
// into the exact source archive; only this derived query source is retained.
//
// Both pinned assemblies are mixed-mode C++/CLI images: their metadata is
// readable everywhere, but the module initializer requires native Windows code,
// so the value-producing part of this probe executes only on the Windows CLR.
// The metadata half (kind, sealed, base, public member closure, constructor
// accessibility, parameter names and order, backing field names) is portable
// and was captured on the qualification host.
using System;
using System.Globalization;
using System.Reflection;
using System.Threading;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

internal static class XnaDisplayModeReferenceProbe
{
    private static ConstructorInfo constructor;

    private static DisplayMode Make(int width, int height, SurfaceFormat format)
    {
        return (DisplayMode)constructor.Invoke(
            new object[] { width, height, format });
    }

    private static string Bits(float value)
    {
        return BitConverter.ToUInt32(BitConverter.GetBytes(value), 0).ToString("X8");
    }

    private static void Main()
    {
        Type type = typeof(DisplayMode);
        Console.WriteLine("ASSEMBLY " + type.Assembly.FullName);
        Console.WriteLine("KIND isClass=" + type.IsClass +
            " sealed=" + type.IsSealed + " abstract=" + type.IsAbstract +
            " base=" + type.BaseType.FullName +
            " interfaces=" + type.GetInterfaces().Length);
        Console.WriteLine("PUBLIC_CTORS " + type.GetConstructors(
            BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static).Length);

        foreach (ConstructorInfo candidate in type.GetConstructors(
            BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static))
        {
            string parameters = "";
            foreach (ParameterInfo parameter in candidate.GetParameters())
            {
                parameters += " " + parameter.ParameterType.FullName +
                    ":" + parameter.Name;
            }
            Console.WriteLine("CTOR assembly=" + candidate.IsAssembly +
                " family=" + candidate.IsFamily + " private=" + candidate.IsPrivate +
                " public=" + candidate.IsPublic +
                " params=" + candidate.GetParameters().Length + parameters);
            constructor = candidate;
        }

        foreach (MemberInfo member in type.GetMembers(
            BindingFlags.Public | BindingFlags.Instance |
            BindingFlags.Static | BindingFlags.DeclaredOnly))
        {
            Console.WriteLine("PUBLIC_DECLARED " + member.MemberType + " " + member.Name);
        }

        foreach (FieldInfo field in type.GetFields(
            BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.DeclaredOnly))
        {
            Console.WriteLine("FIELD assembly=" + field.IsAssembly + " " +
                field.FieldType.FullName + " " + field.Name);
        }

        Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
        int[,] dimensions = new int[,] {
            {800,480},{1920,1080},{1024,768},{1280,720},{1366,768},{1600,900},
            {640,480},{1,3},{1023,769},{7,13},{3,7},{16777217,1},{16777217,3},
            {0,480},{800,0},{0,0},
            {-800,480},{800,-480},{-800,-480},{-1,-1},{-1,3},
            {2147483647,1},{1,2147483647},{2147483647,2147483647},
            {-2147483648,1},{1,-2147483648},{-2147483648,-2147483648},
            {2147483647,-2147483648},{-2147483648,2147483647}
        };
        for (int index = 0; index < dimensions.GetLength(0); index++)
        {
            int width = dimensions[index, 0];
            int height = dimensions[index, 1];
            DisplayMode mode = Make(width, height, SurfaceFormat.Color);
            Rectangle area = mode.TitleSafeArea;
            Console.WriteLine("DIM\t" + width + "\t" + height +
                "\t" + mode.Width + "\t" + mode.Height +
                "\t" + Bits(mode.AspectRatio) +
                "\t" + area.X + "\t" + area.Y + "\t" + area.Width + "\t" + area.Height +
                "\t" + mode.ToString());
        }

        foreach (SurfaceFormat format in Enum.GetValues(typeof(SurfaceFormat)))
        {
            DisplayMode mode = Make(800, 480, format);
            Console.WriteLine("FMT\t" + (int)format + "\t" + format +
                "\t" + mode.Format + "\t" + mode.ToString());
        }

        // XNA formats through CultureInfo.CurrentCulture, so the Single element
        // is culture sensitive while the labels, braces and Int32 digits are not.
        foreach (string culture in new string[] { "", "en-US", "de-DE", "fr-FR", "sv-SE" })
        {
            Thread.CurrentThread.CurrentCulture = culture.Length == 0
                ? CultureInfo.InvariantCulture : new CultureInfo(culture);
            DisplayMode mode = Make(1920, 1080, SurfaceFormat.Color);
            Console.WriteLine("CULTURE\t" +
                (culture.Length == 0 ? "INVARIANT" : culture) + "\t" + mode.ToString());
        }
        Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;

        DisplayMode original = Make(800, 480, SurfaceFormat.Color);
        DisplayMode alias = original;
        DisplayMode separate = Make(800, 480, SurfaceFormat.Color);
        Console.WriteLine("IDENTITY alias=" + Object.ReferenceEquals(original, alias) +
            " distinct=" + Object.ReferenceEquals(original, separate) +
            " equals=" + original.Equals(separate));

        Rectangle retrieved = original.TitleSafeArea;
        retrieved.X = 42;
        retrieved.Width = 7;
        Rectangle refetched = original.TitleSafeArea;
        Console.WriteLine("TITLE_SAFE_VALUE_SEMANTICS mutated=" +
            retrieved.X + "," + retrieved.Width + " refetched=" +
            refetched.X + "," + refetched.Width);
    }
}
