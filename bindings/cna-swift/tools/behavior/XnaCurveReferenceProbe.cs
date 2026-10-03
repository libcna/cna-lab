// Compile and execute this probe on Windows against the pinned Microsoft XNA
// Framework 4.0 runtime assembly. The binary is intentionally not retained.
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;

internal static class XnaCurveReferenceProbe
{
    private static uint Bits(float value)
    {
        return BitConverter.ToUInt32(BitConverter.GetBytes(value), 0);
    }

    private static string ExceptionName(Action action)
    {
        try { action(); return "NONE"; }
        catch (Exception exception) { return exception.GetType().FullName; }
    }

    private static void PrintKeys(string name, CurveKeyCollection keys)
    {
        Console.Write(name + " count=" + keys.Count);
        for (int index = 0; index < keys.Count; index++)
        {
            Console.Write(" " + Bits(keys[index].Position).ToString("X8") +
                ":" + Bits(keys[index].Value).ToString("X8"));
        }
        Console.WriteLine();
    }

    private static void Main()
    {
        CurveKey nan = new CurveKey(float.NaN, 1f);
        CurveKey finite = new CurveKey(0f, 1f);
        Console.WriteLine("COMPARE finite=" + new CurveKey(-1f, 0f).CompareTo(finite) +
            "," + finite.CompareTo(new CurveKey(0f, 9f)) +
            "," + new CurveKey(1f, 0f).CompareTo(finite));
        Console.WriteLine("COMPARE_ZERO " + new CurveKey(+0f, 0f).CompareTo(
            new CurveKey(-0f, 0f)));
        Console.WriteLine("COMPARE_NAN " + nan.CompareTo(finite) + "," +
            finite.CompareTo(nan) + "," + nan.CompareTo(new CurveKey(float.NaN, 2f)));
        Console.WriteLine("COMPARE_NULL " + ExceptionName(() => finite.CompareTo(null)));

        CurveKey values = new CurveKey(2f, 4f, -3f, 7f, CurveContinuity.Step);
        CurveKey clone = values.Clone();
        Console.WriteLine("KEY cloneIdentity=" + Object.ReferenceEquals(values, clone) +
            " equal=" + values.Equals(clone) + " hash=" + values.GetHashCode());
        Console.WriteLine("KEY_NULL equals=" + values.Equals(null) +
            " operators=" + (((CurveKey)null == null) ? "T" : "F") +
            (((CurveKey)null != values) ? "T" : "F"));

        CurveKeyCollection collection = new CurveKeyCollection();
        CurveKey fiveA = new CurveKey(5f, 10f);
        collection.Add(fiveA);
        collection.Add(new CurveKey(1f, 20f));
        collection.Add(new CurveKey(5f, 30f));
        collection.Add(new CurveKey(3f, 40f));
        collection.Add(fiveA);
        PrintKeys("ADD", collection);
        collection.Add(new CurveKey(float.NaN, 1f));
        collection.Add(new CurveKey(float.NaN, 2f));
        PrintKeys("ADD_NAN", collection);

        CurveKeyCollection replace = new CurveKeyCollection();
        replace.Add(new CurveKey(1f, 1f));
        replace.Add(new CurveKey(3f, 3f));
        replace.Add(new CurveKey(5f, 5f));
        replace[1] = new CurveKey(3f, 30f);
        replace[1] = new CurveKey(100f, 100f);
        PrintKeys("REPLACE", replace);
        Console.WriteLine("INDEX_ERRORS " + ExceptionName(() => { var ignored = replace[-1]; }) +
            "," + ExceptionName(() => replace.RemoveAt(replace.Count)));

        CurveKeyCollection enumerable = replace.Clone();
        IEnumerator<CurveKey> addEnumerator = enumerable.GetEnumerator();
        enumerable.Add(new CurveKey(200f, 200f));
        Console.WriteLine("ENUM_ADD " + ExceptionName(() => addEnumerator.MoveNext()));
        IEnumerator<CurveKey> failedRemoveEnumerator = enumerable.GetEnumerator();
        enumerable.Remove(new CurveKey(999f, 999f));
        Console.WriteLine("ENUM_FAILED_REMOVE " +
            ExceptionName(() => failedRemoveEnumerator.MoveNext()));
        IEnumerator<CurveKey> copyEnumerator = enumerable.GetEnumerator();
        CurveKey[] destination = new CurveKey[enumerable.Count];
        enumerable.CopyTo(destination, 0);
        Console.WriteLine("ENUM_COPY " + ExceptionName(() => copyEnumerator.MoveNext()) +
            " same=" + Object.ReferenceEquals(destination[0], enumerable[0]));

        Curve tangents = new Curve();
        tangents.Keys.Add(new CurveKey(1f, 2f));
        tangents.Keys.Add(new CurveKey(4f, 8f));
        tangents.Keys.Add(new CurveKey(10f, 5f));
        tangents.ComputeTangents(CurveTangent.Smooth);
        for (int index = 0; index < tangents.Keys.Count; index++)
        {
            Console.WriteLine("TANGENT " + index + " " +
                Bits(tangents.Keys[index].TangentIn).ToString("X8") + "," +
                Bits(tangents.Keys[index].TangentOut).ToString("X8"));
        }

        Curve evaluate = new Curve();
        evaluate.Keys.Add(new CurveKey(2f, 10f, 3f, 0f));
        evaluate.Keys.Add(new CurveKey(5f, 22f, 0f, -4f));
        foreach (CurveLoopType loop in Enum.GetValues(typeof(CurveLoopType)))
        {
            evaluate.PreLoop = loop;
            evaluate.PostLoop = loop;
            Console.WriteLine("LOOP " + loop + " " +
                Bits(evaluate.Evaluate(-1f)).ToString("X8") + "," +
                Bits(evaluate.Evaluate(1f)).ToString("X8") + "," +
                Bits(evaluate.Evaluate(6f)).ToString("X8") + "," +
                Bits(evaluate.Evaluate(8f)).ToString("X8"));
        }

        Curve widened = new Curve();
        widened.Keys.Add(new CurveKey(
            BitConverter.ToSingle(BitConverter.GetBytes(0xC75E47C4u), 0), 0f));
        widened.Keys.Add(new CurveKey(
            BitConverter.ToSingle(BitConverter.GetBytes(0x46194550u), 0), 1f));
        float target = BitConverter.ToSingle(BitConverter.GetBytes(0x44A2282Cu), 0);
        Console.WriteLine("WIDENED " + Bits(widened.Evaluate(target)).ToString("X8"));
    }
}
