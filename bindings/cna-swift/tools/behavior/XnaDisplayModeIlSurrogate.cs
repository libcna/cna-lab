// Independently authored IL-equivalent re-expression of the pinned
// Microsoft.Xna.Framework.Graphics.DisplayMode method bodies. It contains no
// Microsoft source and no Microsoft binary; it exists only because both pinned
// XNA 4.0 Windows assemblies are mixed-mode C++/CLI images whose module
// initializer needs native Windows code, so their metadata is readable on the
// Linux qualification host while their managed bodies cannot be executed there.
//
// `XnaDisplayModeReferenceProbe.cs` remains the direct-runtime query against the
// real assembly. This surrogate re-expresses the disassembled IL of that same
// type so the exact instruction sequence can be executed against a CLR to
// observe the BCL-produced binary32 and string results. Equivalence is
// established by disassembling this file's output and comparing method bodies
// with the reference IL: the constructor, all five getters, the static
// title-safe helper and ToString match instruction for instruction, modulo
// compiler-chosen branch polarity/ordering in get_AspectRatio, `dup` versus
// `stloc.0`/`ldloc.0` array staging in ToString, `maxstack`, and short versus
// long branch encodings. None of those alter observable behaviour.
using System;
using System.Globalization;

namespace XnaDisplayModeIlSurrogate
{
    public enum SurfaceFormat
    {
        Color = 0, Bgr565 = 1, Bgra5551 = 2, Bgra4444 = 3, Dxt1 = 4, Dxt3 = 5,
        Dxt5 = 6, NormalizedByte2 = 7, NormalizedByte4 = 8, Rgba1010102 = 9,
        Rg32 = 10, Rgba64 = 11, Alpha8 = 12, Single = 13, Vector2 = 14,
        Vector4 = 15, HalfSingle = 16, HalfVector2 = 17, HalfVector4 = 18,
        HdrBlendable = 19,
    }

    // Microsoft.Xna.Framework.Rectangle::.ctor(int32, int32, int32, int32) is a
    // plain four-field store with no validation.
    public struct Rectangle
    {
        public int X;
        public int Y;
        public int Width;
        public int Height;

        public Rectangle(int x, int y, int width, int height)
        {
            this.X = x;
            this.Y = y;
            this.Width = width;
            this.Height = height;
        }
    }

    public class DisplayMode
    {
        internal int _width;
        internal int _height;
        internal SurfaceFormat _format;

        internal DisplayMode(int width, int height, SurfaceFormat format)
        {
            this._width = width;
            this._height = height;
            this._format = format;
        }

        public SurfaceFormat Format { get { return this._format; } }
        public int Height { get { return this._height; } }
        public int Width { get { return this._width; } }

        public float AspectRatio
        {
            get
            {
                if (this._height != 0 && this._width != 0)
                {
                    return (float)this._width / (float)this._height;
                }
                return 0f;
            }
        }

        public Rectangle TitleSafeArea
        {
            get { return GetTitleSafeArea(0, 0, this._width, this._height); }
        }

        // Microsoft.Xna.Framework.Graphics.Viewport::GetTitleSafeArea on the
        // Windows runtime returns the unmodified rectangle; there is no
        // overscan inset and no display query.
        internal static Rectangle GetTitleSafeArea(int x, int y, int w, int h)
        {
            return new Rectangle(x, y, w, h);
        }

        public override string ToString()
        {
            return string.Format(CultureInfo.CurrentCulture,
                "{{Width:{0} Height:{1} Format:{2} AspectRatio:{3}}}",
                new object[] { this._width, this._height, this.Format, this.AspectRatio });
        }
    }
}
