package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Rectangle;

/** Immutable snapshot of one adapter display mode. */
public class DisplayMode {

    private final int width;
    private final int height;
    private final SurfaceFormat format;

    DisplayMode(int width, int height, SurfaceFormat format) {
        this.width = width;
        this.height = height;
        this.format = format;
    }

    static DisplayMode fromNative(int[] value) {
        if (value.length != 4) {
            throw new IllegalArgumentException("Invalid native display-mode length");
        }
        int formatIndex = value[3];
        if (formatIndex < 0 || formatIndex >= SurfaceFormat.values().length) {
            throw new IllegalArgumentException(
                    "CNA returned an invalid SurfaceFormat value " + formatIndex);
        }
        return new DisplayMode(value[0], value[1], SurfaceFormat.values()[formatIndex]);
    }

    public final float getAspectRatio() {
        if (height == 0 || width == 0) {
            return 0.0f;
        }
        return (float)width / (float)height;
    }

    public final SurfaceFormat getFormat() {
        return format;
    }

    public final int getHeight() {
        return height;
    }

    public final Rectangle getTitleSafeArea() {
        return new Rectangle(0, 0, width, height);
    }

    public final int getWidth() {
        return width;
    }

    @Override
    public String toString() {
        return "{Width:" + width + " Height:" + height + " Format:" + format
                + " AspectRatio:" + getAspectRatio() + '}';
    }
}
