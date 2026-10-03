package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** XNA SpriteBatch facade backed by CNA's batched quad commands. */
@SuppressWarnings("this-escape")
public class SpriteBatch extends GraphicsResource {

    private boolean begun;

    public SpriteBatch(GraphicsDevice graphicsDevice) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        NativeBindings.createSpriteBatch(this, graphicsDevice);
    }

    public final void Begin() {
        begin(SpriteSortMode.Deferred);
    }

    public final void Begin(SpriteSortMode sortMode, BlendState blendState) {
        Begin(sortMode, blendState, null, null, null);
    }

    public final void Begin(
            SpriteSortMode sortMode,
            BlendState blendState,
            SamplerState samplerState,
            DepthStencilState depthStencilState,
            RasterizerState rasterizerState) {
        ensureBeginAllowed();
        SpriteSortMode selectedSortMode = Objects.requireNonNull(sortMode, "sortMode");
        BlendState selectedBlend = blendState == null ? BlendState.AlphaBlend : blendState;
        SamplerState selectedSampler = samplerState == null ? SamplerState.LinearClamp : samplerState;
        DepthStencilState selectedDepth =
                depthStencilState == null ? DepthStencilState.None : depthStencilState;
        RasterizerState selectedRasterizer = rasterizerState == null
                ? RasterizerState.CullCounterClockwise : rasterizerState;

        int[] blend = selectedBlend.snapshotForBinding();
        int[] sampler = selectedSampler.snapshotIntegersForBinding();
        float samplerBias = selectedSampler.snapshotBiasForBinding();
        int[] depth = selectedDepth.snapshotForBinding();
        int[] rasterizer = selectedRasterizer.snapshotIntegersForBinding();
        float[] rasterizerFloats = selectedRasterizer.snapshotFloatsForBinding();
        NativeBindings.beginSpriteBatchWithStates(
                this,
                selectedSortMode,
                blend,
                sampler,
                samplerBias,
                depth,
                rasterizer,
                rasterizerFloats);

        GraphicsDevice device = getGraphicsDevice();
        selectedBlend.bind(device);
        selectedSampler.bind(device);
        selectedDepth.bind(device);
        selectedRasterizer.bind(device);
        begun = true;
    }

    public final void Begin(
            SpriteSortMode sortMode,
            BlendState blendState,
            SamplerState samplerState,
            DepthStencilState depthStencilState,
            RasterizerState rasterizerState,
            Effect effect) {
        beginWithEffect(sortMode, blendState, samplerState, depthStencilState,
                rasterizerState, effect, null, false);
    }

    public final void Begin(
            SpriteSortMode sortMode,
            BlendState blendState,
            SamplerState samplerState,
            DepthStencilState depthStencilState,
            RasterizerState rasterizerState,
            Effect effect,
            Matrix transformMatrix) {
        beginWithEffect(sortMode, blendState, samplerState, depthStencilState,
                rasterizerState, effect,
                new Matrix(Objects.requireNonNull(transformMatrix, "transformMatrix")), true);
    }

    public final void End() {
        ensureNotDisposed();
        if (!begun) {
            throw new IllegalStateException("SpriteBatch.End requires a matching Begin");
        }
        try {
            NativeBindings.endSpriteBatch(this);
        } finally {
            begun = false;
        }
    }

    public final void Draw(Texture2D texture, Vector2 position, Color color) {
        drawScaled(texture, position, null, color, 0.0f, new Vector2(),
                new Vector2(1.0f), SpriteEffects.None, 0.0f);
    }

    public final void Draw(
            Texture2D texture,
            Vector2 position,
            Rectangle sourceRectangle,
            Color color) {
        drawScaled(texture, position, sourceRectangle, color, 0.0f, new Vector2(),
                new Vector2(1.0f), SpriteEffects.None, 0.0f);
    }

    public final void Draw(
            Texture2D texture,
            Vector2 position,
            Rectangle sourceRectangle,
            Color color,
            float rotation,
            Vector2 origin,
            float scale,
            SpriteEffects effects,
            float layerDepth) {
        drawScaled(texture, position, sourceRectangle, color, rotation, origin,
                new Vector2(scale), effects, layerDepth);
    }

    public final void Draw(
            Texture2D texture,
            Vector2 position,
            Rectangle sourceRectangle,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        drawScaled(texture, position, sourceRectangle, color, rotation, origin, scale, effects, layerDepth);
    }

    public final void Draw(Texture2D texture, Rectangle destinationRectangle, Color color) {
        drawRectangle(texture, destinationRectangle, null, color, 0.0f, new Vector2(),
                SpriteEffects.None, 0.0f);
    }

    public final void Draw(
            Texture2D texture,
            Rectangle destinationRectangle,
            Rectangle sourceRectangle,
            Color color) {
        drawRectangle(texture, destinationRectangle, sourceRectangle, color, 0.0f, new Vector2(),
                SpriteEffects.None, 0.0f);
    }

    public final void Draw(
            Texture2D texture,
            Rectangle destinationRectangle,
            Rectangle sourceRectangle,
            Color color,
            float rotation,
            Vector2 origin,
            SpriteEffects effects,
            float layerDepth) {
        drawRectangle(texture, destinationRectangle, sourceRectangle, color, rotation, origin,
                effects, layerDepth);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            String text,
            Vector2 position,
            Color color) {
        drawString(spriteFont, text, position, color, 0.0f, new Vector2(),
                new Vector2(1.0f), SpriteEffects.None, 0.0f);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            StringBuilder text,
            Vector2 position,
            Color color) {
        drawString(spriteFont, Objects.requireNonNull(text, "text").toString(),
                position, color, 0.0f, new Vector2(),
                new Vector2(1.0f), SpriteEffects.None, 0.0f);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            String text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            float scale,
            SpriteEffects effects,
            float layerDepth) {
        drawString(spriteFont, text, position, color, rotation, origin,
                new Vector2(scale), effects, layerDepth);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            StringBuilder text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            float scale,
            SpriteEffects effects,
            float layerDepth) {
        drawString(spriteFont, Objects.requireNonNull(text, "text").toString(),
                position, color, rotation, origin, new Vector2(scale), effects, layerDepth);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            String text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        drawString(spriteFont, text, position, color, rotation, origin, scale, effects, layerDepth);
    }

    public final void DrawString(
            SpriteFont spriteFont,
            StringBuilder text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        drawString(spriteFont, Objects.requireNonNull(text, "text").toString(),
                position, color, rotation, origin, scale, effects, layerDepth);
    }

    @Override
    protected void Dispose(boolean disposing) {
        if (disposing && !getIsDisposed()) {
            begun = false;
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(disposing);
    }

    private void begin(SpriteSortMode sortMode) {
        ensureBeginAllowed();
        NativeBindings.beginSpriteBatch(this, sortMode);
        begun = true;
    }

    private void ensureBeginAllowed() {
        ensureNotDisposed();
        if (begun) {
            throw new IllegalStateException("SpriteBatch.Begin cannot be nested");
        }
    }

    private void beginWithEffect(
            SpriteSortMode sortMode,
            BlendState blendState,
            SamplerState samplerState,
            DepthStencilState depthStencilState,
            RasterizerState rasterizerState,
            Effect effect,
            Matrix transformMatrix,
            boolean hasTransform) {
        ensureBeginAllowed();
        SpriteSortMode selectedSortMode = Objects.requireNonNull(sortMode, "sortMode");
        BlendState selectedBlend = blendState == null ? BlendState.AlphaBlend : blendState;
        SamplerState selectedSampler = samplerState == null ? SamplerState.LinearClamp : samplerState;
        DepthStencilState selectedDepth =
                depthStencilState == null ? DepthStencilState.None : depthStencilState;
        RasterizerState selectedRasterizer = rasterizerState == null
                ? RasterizerState.CullCounterClockwise : rasterizerState;
        if (effect != null) {
            if (effect.getIsDisposed()) {
                throw new IllegalStateException("Effect is already disposed");
            }
            if (effect.getGraphicsDevice() != getGraphicsDevice()) {
                throw new IllegalArgumentException("Effect belongs to a different GraphicsDevice");
            }
        }

        int[] blend = selectedBlend.snapshotForBinding();
        int[] sampler = selectedSampler.snapshotIntegersForBinding();
        float samplerBias = selectedSampler.snapshotBiasForBinding();
        int[] depth = selectedDepth.snapshotForBinding();
        int[] rasterizer = selectedRasterizer.snapshotIntegersForBinding();
        float[] rasterizerFloats = selectedRasterizer.snapshotFloatsForBinding();
        NativeBindings.beginSpriteBatchWithEffect(
                this, selectedSortMode, blend, sampler, samplerBias, depth,
                rasterizer, rasterizerFloats, effect,
                hasTransform ? transformMatrix : null);

        GraphicsDevice device = getGraphicsDevice();
        selectedBlend.bind(device);
        selectedSampler.bind(device);
        selectedDepth.bind(device);
        selectedRasterizer.bind(device);
        begun = true;
    }

    private void drawRectangle(
            Texture2D texture,
            Rectangle destinationRectangle,
            Rectangle sourceRectangle,
            Color color,
            float rotation,
            Vector2 origin,
            SpriteEffects effects,
            float layerDepth) {
        ensureBegun();
        NativeBindings.drawSpriteRectangle(this, texture, destinationRectangle, sourceRectangle,
                color, rotation, origin, effects, layerDepth);
    }

    private void drawScaled(
            Texture2D texture,
            Vector2 position,
            Rectangle sourceRectangle,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        ensureBegun();
        NativeBindings.drawSpriteScaled(this, texture, position, sourceRectangle, color, rotation,
                origin, scale, effects, layerDepth);
    }

    private void ensureBegun() {
        ensureNotDisposed();
        if (!begun) {
            throw new IllegalStateException("SpriteBatch.Draw requires a matching Begin");
        }
    }

    private void drawString(
            SpriteFont spriteFont,
            String text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        Objects.requireNonNull(spriteFont, "spriteFont");
        Objects.requireNonNull(text, "text");
        ensureBegun();
        NativeBindings.drawSpriteString(
                this, spriteFont, text, position, color, rotation,
                origin, scale, effects, layerDepth);
    }
}
