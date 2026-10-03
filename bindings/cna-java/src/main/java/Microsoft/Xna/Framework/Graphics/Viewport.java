package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Mutable XNA viewport value with pure-Java projection math. */
public final class Viewport {

    private int x;
    private int y;
    private int width;
    private int height;
    private float minDepth;
    private float maxDepth;

    public Viewport() {
    }

    public Viewport(Viewport value) {
        Viewport source = Objects.requireNonNull(value, "value");
        x = source.x;
        y = source.y;
        width = source.width;
        height = source.height;
        minDepth = source.minDepth;
        maxDepth = source.maxDepth;
    }

    public Viewport(int x, int y, int width, int height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        minDepth = 0.0f;
        maxDepth = 1.0f;
    }

    public Viewport(Rectangle bounds) {
        Rectangle value = new Rectangle(Objects.requireNonNull(bounds, "bounds"));
        x = value.X;
        y = value.Y;
        width = value.Width;
        height = value.Height;
        minDepth = 0.0f;
        maxDepth = 1.0f;
    }

    public int getX() {
        return x;
    }

    public void setX(int value) {
        x = value;
    }

    public int getY() {
        return y;
    }

    public void setY(int value) {
        y = value;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int value) {
        width = value;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int value) {
        height = value;
    }

    public float getMinDepth() {
        return minDepth;
    }

    public void setMinDepth(float value) {
        minDepth = value;
    }

    public float getMaxDepth() {
        return maxDepth;
    }

    public void setMaxDepth(float value) {
        maxDepth = value;
    }

    public Rectangle getBounds() {
        return new Rectangle(x, y, width, height);
    }

    public void setBounds(Rectangle value) {
        Rectangle bounds = new Rectangle(Objects.requireNonNull(value, "value"));
        x = bounds.X;
        y = bounds.Y;
        width = bounds.Width;
        height = bounds.Height;
    }

    public float getAspectRatio() {
        if (height == 0 || width == 0) {
            return 0.0f;
        }
        return (float)width / (float)height;
    }

    public Rectangle getTitleSafeArea() {
        return new Rectangle(x, y, width, height);
    }

    public Vector3 Project(Vector3 source, Matrix projection, Matrix view, Matrix world) {
        Objects.requireNonNull(source, "source");
        Matrix matrix = Matrix.Multiply(
                Matrix.Multiply(Objects.requireNonNull(world, "world"), Objects.requireNonNull(view, "view")),
                Objects.requireNonNull(projection, "projection"));
        Vector3 result = Vector3.Transform(source, matrix);
        float divisor = source.X * matrix.M14 + source.Y * matrix.M24
                + source.Z * matrix.M34 + matrix.M44;
        if (!withinEpsilon(divisor, 1.0f)) {
            result = Vector3.Divide(result, divisor);
        }
        result.X = (result.X + 1.0f) * 0.5f * (float)width + (float)x;
        result.Y = (-result.Y + 1.0f) * 0.5f * (float)height + (float)y;
        result.Z = result.Z * (maxDepth - minDepth) + minDepth;
        return result;
    }

    public Vector3 Unproject(Vector3 source, Matrix projection, Matrix view, Matrix world) {
        Vector3 value = new Vector3(Objects.requireNonNull(source, "source"));
        Matrix matrix = Matrix.Invert(Matrix.Multiply(
                Matrix.Multiply(Objects.requireNonNull(world, "world"), Objects.requireNonNull(view, "view")),
                Objects.requireNonNull(projection, "projection")));
        value.X = (value.X - (float)x) / (float)width * 2.0f - 1.0f;
        value.Y = -((value.Y - (float)y) / (float)height * 2.0f - 1.0f);
        value.Z = (value.Z - minDepth) / (maxDepth - minDepth);
        Vector3 result = Vector3.Transform(value, matrix);
        float divisor = value.X * matrix.M14 + value.Y * matrix.M24
                + value.Z * matrix.M34 + matrix.M44;
        if (!withinEpsilon(divisor, 1.0f)) {
            result = Vector3.Divide(result, divisor);
        }
        return result;
    }

    private static boolean withinEpsilon(float a, float b) {
        float difference = a - b;
        return -Float.MIN_VALUE <= difference && difference <= Float.MIN_VALUE;
    }

    @Override
    public String toString() {
        return "{X:" + x + " Y:" + y + " Width:" + width + " Height:" + height
                + " MinDepth:" + minDepth + " MaxDepth:" + maxDepth + '}';
    }
}
