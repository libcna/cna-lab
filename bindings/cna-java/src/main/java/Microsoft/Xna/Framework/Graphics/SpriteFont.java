package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector2;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.List;
import java.util.Objects;

/**
 * XNA sprite-font projection backed by an owned CNA font and glyph atlas.
 *
 * <p>XNA exposes no public {@code SpriteFont} constructor. Instances are produced by
 * {@code ContentManager.Load(SpriteFont.class, ...)}.</p>
 */
public final class SpriteFont {

    SpriteFont() {
    }

    public Vector2 MeasureString(String text) {
        return NativeBindings.measureSpriteFont(
                this, Objects.requireNonNull(text, "text"));
    }

    public Vector2 MeasureString(StringBuilder text) {
        return NativeBindings.measureSpriteFont(
                this, Objects.requireNonNull(text, "text").toString());
    }

    public List<Character> getCharacters() {
        return NativeBindings.getSpriteFontCharacters(this);
    }

    public Character getDefaultCharacter() {
        return NativeBindings.getSpriteFontDefaultCharacter(this);
    }

    public void setDefaultCharacter(Character value) {
        if (value != null && !getCharacters().contains(value)) {
            throw new IllegalArgumentException(
                    "DefaultCharacter must be present in the SpriteFont character set");
        }
        NativeBindings.setSpriteFontDefaultCharacter(this, value);
    }

    public int getLineSpacing() {
        return NativeBindings.getSpriteFontLineSpacing(this);
    }

    public void setLineSpacing(int value) {
        NativeBindings.setSpriteFontLineSpacing(this, value);
    }

    public float getSpacing() {
        return NativeBindings.getSpriteFontSpacing(this);
    }

    public void setSpacing(float value) {
        if (!Float.isFinite(value)) {
            throw new IllegalArgumentException("SpriteFont spacing must be finite");
        }
        NativeBindings.setSpriteFontSpacing(this, value);
    }
}
