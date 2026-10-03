package Microsoft.Xna.Framework;

/** Mutable integer XNA point value. */
public final class Point {

    public int X;
    public int Y;

    public Point() {
    }

    public Point(int x, int y) { X = x; Y = y; }
    public Point(Point value) { this(value.X, value.Y); }

    public static Point getZero() { return new Point(); }

    @Override
    public boolean equals(Object obj) { return this == obj || obj instanceof Point value && X == value.X && Y == value.Y; }

    public boolean equals(Point other) { return other != null && X == other.X && Y == other.Y; }

    @Override
    public int hashCode() { return X + Y; }

    @Override
    public String toString() { return "{X:" + X + " Y:" + Y + '}'; }
}
