package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

final class GraphicsResourceTests {

    @Test
    void SurfaceAndSpriteEnumsPreserveXnaIdentityAndValues() {
        assertEquals(0, SurfaceFormat.Color.ordinal());
        assertEquals(19, SurfaceFormat.HdrBlendable.ordinal());
        assertEquals(4, SpriteSortMode.FrontToBack.ordinal());
        assertEquals(3, SpriteEffects.FlipHorizontally.Or(SpriteEffects.FlipVertically).getValue());
        assertTrue(SpriteEffects.FromValue(3).Contains(SpriteEffects.FlipHorizontally));
        assertEquals(SpriteEffects.None, SpriteEffects.FromValue(0));
        assertEquals(7, ClearOptions.Target.Or(ClearOptions.DepthBuffer)
                .Or(ClearOptions.Stencil).getValue());
        assertTrue(ClearOptions.FromValue(7).Contains(ClearOptions.DepthBuffer));
        assertEquals(ClearOptions.Target, ClearOptions.FromValue(1));
        assertEquals(3, BlendFunction.Min.ordinal());
        assertEquals(4, BlendFunction.Max.ordinal());
        assertEquals(12, Blend.SourceAlphaSaturation.ordinal());
        assertEquals(7, CompareFunction.NotEqual.ordinal());
        assertEquals(7, StencilOperation.Invert.ordinal());
        assertEquals(2, CullMode.CullCounterClockwiseFace.ordinal());
        assertEquals(1, FillMode.WireFrame.ordinal());
        assertEquals(2, TextureAddressMode.Mirror.ordinal());
        assertEquals(8, TextureFilter.MinPointMagLinearMipPoint.ordinal());
        assertEquals(15, ColorWriteChannels.Red.Or(ColorWriteChannels.Green)
                .Or(ColorWriteChannels.Blue).Or(ColorWriteChannels.Alpha).getValue());
        assertTrue(ColorWriteChannels.All.Contains(ColorWriteChannels.Blue));
    }

    @Test
    void GraphicsStatesHaveXnaDefaultsPresetIdentityAndSnapshotSemantics() {
        BlendState blend = new BlendState();
        assertEquals(Blend.One, blend.getColorSourceBlend());
        assertEquals(Blend.Zero, blend.getColorDestinationBlend());
        assertEquals(Blend.One, blend.getAlphaSourceBlend());
        assertEquals(Blend.Zero, blend.getAlphaDestinationBlend());
        assertEquals(BlendFunction.Add, blend.getColorBlendFunction());
        assertEquals(ColorWriteChannels.All, blend.getColorWriteChannels());
        assertEquals(Color.White, blend.getBlendFactor());
        assertEquals(-1, blend.getMultiSampleMask());

        Color assigned = new Color(1, 2, 3, 4);
        blend.setBlendFactor(assigned);
        assigned.setR(99);
        assertEquals(new Color(1, 2, 3, 4), blend.getBlendFactor());
        Color returned = blend.getBlendFactor();
        returned.setG(88);
        assertEquals(new Color(1, 2, 3, 4), blend.getBlendFactor());

        DepthStencilState depth = new DepthStencilState();
        assertTrue(depth.getDepthBufferEnable());
        assertTrue(depth.getDepthBufferWriteEnable());
        assertEquals(CompareFunction.LessEqual, depth.getDepthBufferFunction());
        assertFalse(depth.getStencilEnable());
        // XNA's constructor writes -1 (every bit) to both masks; CNA once used Int32.MaxValue.
        assertEquals(-1, depth.getStencilMask());
        assertEquals(-1, depth.getStencilWriteMask());
        assertEquals(StencilOperation.Keep, depth.getStencilPass());

        RasterizerState rasterizer = new RasterizerState();
        assertEquals(CullMode.CullCounterClockwiseFace, rasterizer.getCullMode());
        assertEquals(FillMode.Solid, rasterizer.getFillMode());
        assertTrue(rasterizer.getMultiSampleAntiAlias());
        assertFalse(rasterizer.getScissorTestEnable());
        assertEquals(0.0f, rasterizer.getDepthBias());

        SamplerState sampler = new SamplerState();
        assertEquals(TextureFilter.Linear, sampler.getFilter());
        assertEquals(TextureAddressMode.Wrap, sampler.getAddressU());
        assertEquals(TextureAddressMode.Wrap, sampler.getAddressV());
        assertEquals(TextureAddressMode.Wrap, sampler.getAddressW());
        assertEquals(4, sampler.getMaxAnisotropy());
        assertEquals(0, sampler.getMaxMipLevel());
        assertEquals(0.0f, sampler.getMipMapLevelOfDetailBias());

        assertSame(BlendState.Opaque, BlendState.Opaque);
        assertEquals("BlendState.Opaque", BlendState.Opaque.getName());
        assertEquals(Blend.SourceAlpha, BlendState.Additive.getColorSourceBlend());
        assertEquals(Blend.InverseSourceAlpha,
                BlendState.NonPremultiplied.getColorDestinationBlend());
        assertFalse(DepthStencilState.None.getDepthBufferEnable());
        assertFalse(DepthStencilState.DepthRead.getDepthBufferWriteEnable());
        assertEquals(CullMode.None, RasterizerState.CullNone.getCullMode());
        assertEquals(TextureFilter.Point, SamplerState.PointClamp.getFilter());
        assertEquals(TextureAddressMode.Clamp, SamplerState.AnisotropicClamp.getAddressW());
    }

    @Test
    void GraphicsStatesFreezeAfterBindingAndRetainDisposedDescriptorValues() {
        try (Game game = new Game()) {
            GraphicsDevice device = game.getGraphicsDevice();
            BlendState state = new BlendState();
            state.setColorSourceBlend(Blend.SourceAlpha);
            assertNull(state.getGraphicsDevice());
            state.bind(device);
            assertSame(device, state.getGraphicsDevice());
            assertThrows(IllegalStateException.class,
                    () -> state.setColorSourceBlend(Blend.One));
            state.close();
            state.close();
            assertTrue(state.getIsDisposed());

            BlendState disposedUnbound = new BlendState();
            disposedUnbound.close();
            // XNA keeps descriptor properties accessible even after an unbound state is disposed.
            disposedUnbound.setColorSourceBlend(Blend.SourceAlpha);
            assertEquals(Blend.SourceAlpha, disposedUnbound.getColorSourceBlend());

            assertThrows(IllegalStateException.class,
                    () -> BlendState.Opaque.setColorSourceBlend(Blend.Zero));
            assertThrows(IllegalStateException.class,
                    () -> SamplerState.LinearWrap.setFilter(TextureFilter.Point));
        }
    }

    @Test
    void GraphicsResourcePropertiesEventsAndCloseAreDeterministic() {
        try (Game game = new Game()) {
            DummyResource resource = new DummyResource(game.getGraphicsDevice());
            List<String> events = new ArrayList<>();
            resource.setName("probe");
            resource.setTag(17);
            resource.addDisposingListener((sender, args) -> events.add(sender.toString()));

            assertSame(game.getGraphicsDevice(), resource.getGraphicsDevice());
            assertEquals("probe", resource.getName());
            assertEquals(17, resource.getTag());
            assertFalse(resource.getIsDisposed());
            resource.close();
            resource.close();

            assertTrue(resource.getIsDisposed());
            assertEquals(1, resource.disposeCalls);
            assertEquals(List.of("probe"), events);
        }
    }

    @Test
    void NativeGraphicsConstructionRequiresAnActiveLifecycleCallback() {
        try (Game game = new Game()) {
            assertThrows(IllegalStateException.class,
                    () -> new Texture2D(game.getGraphicsDevice(), 1, 1));
            assertThrows(IllegalStateException.class,
                    () -> new SpriteBatch(game.getGraphicsDevice()));
        }
    }

    @Test
    void GraphicsDeviceManagedDisposalIsIdempotentAndPublicConstructionNeedsLiveAdapter() {
        try (Game game = new Game()) {
            GraphicsDevice device = game.getGraphicsDevice();
            List<String> events = new ArrayList<>();
            device.addDisposingListener((sender, args) -> events.add("disposing"));
            assertFalse(device.getIsDisposed());
            assertThrows(UnsupportedOperationException.class,
                    () -> new GraphicsDevice(
                            GraphicsAdapter.getDefaultAdapter(),
                            GraphicsProfile.Reach,
                            new PresentationParameters()));
            device.close();
            device.close();
            assertTrue(device.getIsDisposed());
            assertEquals(List.of("disposing"), events);
            assertThrows(IllegalStateException.class,
                    () -> device.Clear(Microsoft.Xna.Framework.Color.Black));
        }
    }

    private static final class DummyResource extends GraphicsResource {
        private int disposeCalls;

        private DummyResource(GraphicsDevice graphicsDevice) {
            super(graphicsDevice);
        }

        @Override
        protected void Dispose(boolean arg0) {
            disposeCalls++;
            super.Dispose(arg0);
        }
    }
}
