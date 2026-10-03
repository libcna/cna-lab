package Microsoft.Xna.Framework;

/** Mutable integer XNA rectangle value. */
public final class Rectangle {

    public int X;
    public int Y;
    public int Width;
    public int Height;

    public Rectangle() {
    }

    public Rectangle(int x, int y, int width, int height) { X = x; Y = y; Width = width; Height = height; }
    public Rectangle(Rectangle value) { this(value.X, value.Y, value.Width, value.Height); }

    public static Rectangle getEmpty() { return new Rectangle(); }
    public int getLeft() { return X; }
    public int getRight() { return X + Width; }
    public int getTop() { return Y; }
    public int getBottom() { return Y + Height; }
    public Point getCenter() { return new Point(X + (Width / 2), Y + (Height / 2)); }
    public Point getLocation() { return new Point(X, Y); }
    public void setLocation(Point value) {
        Point snapshot = new Point(java.util.Objects.requireNonNull(value, "value"));
        X = snapshot.X;
        Y = snapshot.Y;
    }
    public boolean getIsEmpty() { return X == 0 && Y == 0 && Width == 0 && Height == 0; }

    public boolean Contains(int x, int y) { return X <= x && x < getRight() && Y <= y && y < getBottom(); }
    public boolean Contains(Point value) { return Contains(value.X, value.Y); }
    public boolean Contains(Rectangle value) {
        return X <= value.X && value.getRight() <= getRight()
                && Y <= value.Y && value.getBottom() <= getBottom();
    }

    public boolean Intersects(Rectangle value) {
        return value.X < getRight() && X < value.getRight()
                && value.Y < getBottom() && Y < value.getBottom();
    }

    public void Inflate(int horizontalAmount, int verticalAmount) {
        X -= horizontalAmount;
        Y -= verticalAmount;
        Width += horizontalAmount * 2;
        Height += verticalAmount * 2;
    }

    public void Offset(int offsetX, int offsetY) { X += offsetX; Y += offsetY; }
    public void Offset(Point amount) { Offset(amount.X, amount.Y); }

    public static Rectangle Intersect(Rectangle value1, Rectangle value2) {
        int left = Math.max(value1.X, value2.X);
        int top = Math.max(value1.Y, value2.Y);
        int right = Math.min(value1.getRight(), value2.getRight());
        int bottom = Math.min(value1.getBottom(), value2.getBottom());
        return right > left && bottom > top ? new Rectangle(left, top, right - left, bottom - top) : getEmpty();
    }

    public static Rectangle Union(Rectangle value1, Rectangle value2) {
        int left = Math.min(value1.X, value2.X);
        int top = Math.min(value1.Y, value2.Y);
        int right = Math.max(value1.getRight(), value2.getRight());
        int bottom = Math.max(value1.getBottom(), value2.getBottom());
        return new Rectangle(left, top, right - left, bottom - top);
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof Rectangle value
                && X == value.X && Y == value.Y && Width == value.Width && Height == value.Height;
    }

    public boolean equals(Rectangle other) {
        return other != null && X == other.X && Y == other.Y
                && Width == other.Width && Height == other.Height;
    }

    @Override
    public int hashCode() { return X + Y + Width + Height; }

    @Override
    public String toString() { return "{X:" + X + " Y:" + Y + " Width:" + Width + " Height:" + Height + '}'; }
}
