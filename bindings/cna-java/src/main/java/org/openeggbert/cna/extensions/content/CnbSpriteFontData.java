package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector3;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * A sprite font as {@code .cnb} carries it: an atlas and a glyph table, before either is a GPU
 * resource.
 *
 * <p>A compiled font is exactly two things -- one texture holding every character's ink, and a
 * table saying where each character sits in it and how it advances. Both are readable here with
 * no graphics device, which is what lets a tool inspect or rewrite a font's spacing without
 * opening a window.
 *
 * <p>{@link #toSpriteFont(GraphicsDevice)} is the step that needs one, and it produces an
 * ordinary XNA {@link SpriteFont} that {@code SpriteBatch.DrawString} draws with. There is no
 * public {@code SpriteFont} constructor in XNA, and this does not add one: the font is built
 * through the same internal path {@code ContentManager.Load} uses.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnbSpriteFontData implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnbSpriteFontData(long handle) {
        this.handle = handle;
    }

    /**
     * Creates an empty compiled font, with no atlas and no glyphs.
     *
     * @return the font data, which the caller closes
     */
    public static CnbSpriteFontData create() {
        CnbExtension.requireAvailable();
        long[] font = new long[1];
        CnbExtension.check("CnbSpriteFontData.create",
                NativeCnbRoutes.cnbSpriteFontDataCreate(font));
        return new CnbSpriteFontData(font[0]);
    }

    /** Returns how many glyphs the font's table holds. */
    public int getGlyphCount() {
        return (int) info()[0];
    }

    /** Returns the vertical distance between two baselines, in pixels. */
    public int getLineSpacing() {
        return (int) info()[1];
    }

    /** Returns the extra horizontal space added between characters, in pixels. */
    public float getSpacing() {
        float[] floating = new float[1];
        readInfo(new long[4], floating);
        return floating[0];
    }

    /**
     * Returns the character drawn in place of one the font has no glyph for.
     *
     * @return that character, or null when the font declares none, in which case a missing
     *         character is an error rather than a substitution
     */
    public Character getDefaultCharacter() {
        long[] integral = info();
        return integral[3] != 0 ? (char) integral[2] : null;
    }

    /**
     * Sets the font's spacing, line spacing and default character together.
     *
     * <p>They travel as one because CNA stores them as one description; setting them separately
     * would mean reading and rewriting the other two each time.
     *
     * @param lineSpacing the vertical distance between two baselines, in pixels
     * @param spacing the extra horizontal space between characters, in pixels
     * @param defaultCharacter the substitute for a missing character, or null for none
     */
    public void setMetrics(int lineSpacing, float spacing, Character defaultCharacter) {
        if (!Float.isFinite(spacing)) {
            throw new IllegalArgumentException("spacing must be finite");
        }
        long[] integral = info();
        // The glyph count is CNA's own and is not the caller's to set; it is read back and passed
        // through unchanged so the description stays consistent.
        long[] updated = {
            integral[0], lineSpacing,
            defaultCharacter == null ? 0 : defaultCharacter,
            defaultCharacter == null ? 0 : 1,
        };
        CnbExtension.check("CnbSpriteFontData.setMetrics", NativeCnbRoutes
                .cnbSpriteFontDataSetInfo(open(), new byte[5], updated, new float[] {spacing}));
    }

    /**
     * Appends one glyph to the font's table.
     *
     * @param glyph the character, its place in the atlas and its bearings
     * @return the new glyph's index
     */
    public int addGlyph(CnbGlyph glyph) {
        Objects.requireNonNull(glyph, "glyph");
        long[] index = new long[1];
        CnbExtension.check("CnbSpriteFontData.addGlyph", NativeCnbRoutes
                .cnbSpriteFontDataAddGlyph(open(), integral(glyph), floating(glyph), index));
        return (int) index[0];
    }

    /**
     * Returns one glyph from the font's table.
     *
     * @param index the zero-based glyph index
     * @return that glyph
     */
    public CnbGlyph getGlyph(int index) {
        long[] integral = new long[10];
        float[] floating = new float[3];
        CnbExtension.check("CnbSpriteFontData.getGlyph", NativeCnbRoutes
                .cnbSpriteFontDataGetGlyph(open(), index, integral, floating));
        return new CnbGlyph((char) integral[8],
                new Rectangle((int) integral[0], (int) integral[1],
                        (int) integral[2], (int) integral[3]),
                new Rectangle((int) integral[4], (int) integral[5],
                        (int) integral[6], (int) integral[7]),
                new Vector3(floating[0], floating[1], floating[2]));
    }

    /**
     * Replaces one glyph in the font's table.
     *
     * @param index the zero-based glyph index
     * @param glyph the replacement
     */
    public void setGlyph(int index, CnbGlyph glyph) {
        Objects.requireNonNull(glyph, "glyph");
        CnbExtension.check("CnbSpriteFontData.setGlyph", NativeCnbRoutes
                .cnbSpriteFontDataSetGlyph(open(), index, integral(glyph), floating(glyph)));
    }

    /** Returns every glyph in table order. */
    public List<CnbGlyph> getGlyphs() {
        int count = getGlyphCount();
        List<CnbGlyph> glyphs = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            glyphs.add(getGlyph(index));
        }
        return List.copyOf(glyphs);
    }

    /**
     * Sets the font's atlas.
     *
     * @param atlas the texture holding every glyph's ink; CNA takes its own copy, so the caller
     *        keeps ownership of the one they passed
     */
    public void setAtlas(CnbTextureData atlas) {
        Objects.requireNonNull(atlas, "atlas");
        CnbExtension.check("CnbSpriteFontData.setAtlas",
                NativeCnbRoutes.cnbSpriteFontDataSetAtlas(open(), atlas.handle()));
    }

    /**
     * Returns the font's atlas.
     *
     * @return a copy of the atlas, which the caller closes
     */
    public CnbTextureData copyAtlas() {
        long[] atlas = new long[1];
        CnbExtension.check("CnbSpriteFontData.copyAtlas",
                NativeCnbRoutes.cnbSpriteFontDataCopyAtlas(open(), atlas));
        return new CnbTextureData(atlas[0]);
    }

    /**
     * Uploads the font to the graphics device as an ordinary XNA {@link SpriteFont}.
     *
     * <p>The atlas becomes a {@link Texture2D} on the device, and the glyph table crosses
     * unchanged because both sides describe a glyph the same way. The result is the same kind of
     * object {@code ContentManager.Load(SpriteFont.class, ...)} returns, and it draws the same.
     *
     * @param graphicsDevice the device to create the atlas texture on
     * @return the sprite font
     * @throws ContentNotSupportedException when the atlas has no format XNA can name
     * @throws CnbFormatException when the font has no glyphs, since XNA cannot draw with one
     */
    public SpriteFont toSpriteFont(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        List<CnbGlyph> glyphs = getGlyphs();
        if (glyphs.isEmpty()) {
            throw new CnbFormatException(
                    "this .cnb font has no glyphs, and XNA has nothing to draw with one");
        }
        List<Rectangle> bounds = new ArrayList<>(glyphs.size());
        List<Rectangle> cropping = new ArrayList<>(glyphs.size());
        List<Character> characters = new ArrayList<>(glyphs.size());
        List<Vector3> kerning = new ArrayList<>(glyphs.size());
        for (CnbGlyph glyph : glyphs) {
            bounds.add(glyph.GlyphBounds());
            cropping.add(glyph.Cropping());
            characters.add(glyph.Character());
            kerning.add(glyph.Kerning());
        }
        Texture2D atlas;
        try (CnbTextureData data = copyAtlas()) {
            atlas = data.toTexture2D(graphicsDevice);
        }
        try {
            return NativeBindings.createSpriteFont(atlas, bounds, cropping, characters,
                    getLineSpacing(), getSpacing(), kerning, getDefaultCharacter());
        } catch (RuntimeException failure) {
            atlas.close();
            throw failure;
        }
    }

    long handle() {
        return open();
    }

    /** Releases the font data. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbSpriteFontData.close",
                NativeCnbRoutes.cnbSpriteFontDataDestroy(handle));
    }

    private long[] info() {
        long[] integral = new long[4];
        readInfo(integral, new float[1]);
        return integral;
    }

    private void readInfo(long[] integral, float[] floating) {
        CnbExtension.check("CnbSpriteFontData.getInfo", NativeCnbRoutes
                .cnbSpriteFontDataGetInfo(open(), new byte[5], integral, floating));
    }

    private static long[] integral(CnbGlyph glyph) {
        Rectangle bounds = glyph.GlyphBounds();
        Rectangle cropping = glyph.Cropping();
        return new long[] {
            bounds.X, bounds.Y, bounds.Width, bounds.Height,
            cropping.X, cropping.Y, cropping.Width, cropping.Height,
            glyph.Character(), 0,
        };
    }

    private static float[] floating(CnbGlyph glyph) {
        Vector3 kerning = glyph.Kerning();
        return new float[] {kerning.X, kerning.Y, kerning.Z};
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbSpriteFontData is closed");
            }
        }
        return handle;
    }
}
