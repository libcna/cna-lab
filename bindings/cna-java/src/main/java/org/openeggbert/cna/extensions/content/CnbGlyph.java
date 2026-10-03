package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector3;

/**
 * One character's place in a compiled font's atlas.
 *
 * <p>The three parts are XNA's own, and they mean what they mean in XNA: {@code GlyphBounds} is
 * where the ink sits in the atlas, {@code Cropping} is the offset and box the ink is drawn into,
 * and {@code Kerning} is the left bearing, the advance width and the right bearing in that order.
 *
 * @param Character the character this glyph draws
 * @param GlyphBounds the glyph's rectangle inside the atlas
 * @param Cropping the glyph's offset and box within its cell
 * @param Kerning left bearing, advance width and right bearing
 */
public record CnbGlyph(
        char Character, Rectangle GlyphBounds, Rectangle Cropping, Vector3 Kerning) {
}
