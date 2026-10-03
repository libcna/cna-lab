package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector4;
import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.NativeBindings;

import java.nio.charset.StandardCharsets;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** XNA graphics-device facade backed by callback-scoped CNA device handles. */
@SuppressWarnings("this-escape")
public class GraphicsDevice implements AutoCloseable {

    private final Game game;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceLostListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceResetListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceResettingListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<ResourceCreatedEventArgs>>
            resourceCreatedListeners = new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<ResourceDestroyedEventArgs>>
            resourceDestroyedListeners = new CopyOnWriteArrayList<>();

    private GraphicsAdapter adapter;
    private BlendState blendState;
    private DepthStencilState depthStencilState;
    private RasterizerState rasterizerState;
    private SamplerStateCollection samplerStates;
    private SamplerStateCollection vertexSamplerStates;
    private TextureCollection textures;
    private TextureCollection vertexTextures;
    private boolean disposingEventRaised;
    private boolean closed;

    GraphicsDevice(Game game) {
        this.game = Objects.requireNonNull(game, "game");
        NativeBindings.registerGraphicsDevice(this, game);
    }

    /**
     * Adopts the live game device associated with an adapter returned by {@link #getAdapter()}.
     * CNA has no stable C route for constructing an independent graphics device.
     */
    public GraphicsDevice(
            GraphicsAdapter adapter,
            GraphicsProfile graphicsProfile,
            PresentationParameters presentationParameters) {
        this(requireOwningGame(adapter));
        GraphicsProfile profile = Objects.requireNonNull(graphicsProfile, "graphicsProfile");
        PresentationParameters parameters =
                Objects.requireNonNull(presentationParameters, "presentationParameters");
        NativeBindings.setGraphicsDeviceProfile(this, profile.ordinal());
        NativeBindings.resetGraphicsDevice(this, parameters, adapter.nativeIndex());
    }

    /** Clears only the current color target, snapshotting the mutable color value. */
    public final void Clear(Color color) {
        Clear(ClearOptions.Target, color, 1.0f, 0);
    }

    /** Clears the selected buffers through CNA's full device clear route. */
    public final void Clear(ClearOptions options, Color color, float depth, int stencil) {
        ensureOpen();
        Color snapshot = new Color(Objects.requireNonNull(color, "color"));
        NativeBindings.clearGraphicsDevice(
                this,
                Objects.requireNonNull(options, "options").getValue(),
                snapshot.getPackedValue().intValue(),
                depth,
                stencil);
    }

    /** Clears the selected buffers after applying XNA's Vector4-to-Color conversion. */
    public final void Clear(ClearOptions options, Vector4 color, float depth, int stencil) {
        Clear(options, new Color(Objects.requireNonNull(color, "color")), depth, stencil);
    }

    public final void DrawIndexedPrimitives(
            PrimitiveType primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount) {
        ensureOpen();
        requireNonNegative(baseVertex, "baseVertex");
        requireNonNegative(minVertexIndex, "minVertexIndex");
        requireNonNegative(numVertices, "numVertices");
        requireNonNegative(startIndex, "startIndex");
        requirePositive(primitiveCount, "primitiveCount");
        NativeBindings.drawIndexedPrimitives(
                this, Objects.requireNonNull(primitiveType, "primitiveType").ordinal(),
                baseVertex, minVertexIndex, numVertices, startIndex, primitiveCount);
    }

    public final void DrawInstancedPrimitives(
            PrimitiveType primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount,
            int instanceCount) {
        ensureOpen();
        requireNonNegative(baseVertex, "baseVertex");
        requireNonNegative(minVertexIndex, "minVertexIndex");
        requireNonNegative(numVertices, "numVertices");
        requireNonNegative(startIndex, "startIndex");
        requirePositive(primitiveCount, "primitiveCount");
        requirePositive(instanceCount, "instanceCount");
        NativeBindings.drawInstancedPrimitives(
                this, Objects.requireNonNull(primitiveType, "primitiveType").ordinal(),
                baseVertex, minVertexIndex, numVertices, startIndex,
                primitiveCount, instanceCount);
    }

    public final void DrawPrimitives(
            PrimitiveType primitiveType,
            int startVertex,
            int primitiveCount) {
        ensureOpen();
        requireNonNegative(startVertex, "startVertex");
        requirePositive(primitiveCount, "primitiveCount");
        NativeBindings.drawPrimitives(
                this, Objects.requireNonNull(primitiveType, "primitiveType").ordinal(),
                startVertex, primitiveCount);
    }

    public final <T> void DrawUserPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int primitiveCount) {
        Objects.requireNonNull(vertexData, "vertexData");
        DrawUserPrimitives(
                primitiveType, vertexData, vertexOffset, primitiveCount,
                VertexDeclaration.fromType(vertexData.getClass().getComponentType()));
    }

    public final <T> void DrawUserPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int primitiveCount,
            VertexDeclaration vertexDeclaration) {
        ensureOpen();
        PrimitiveType topology = Objects.requireNonNull(primitiveType, "primitiveType");
        T[] vertices = requireVertexArray(vertexData);
        VertexDeclaration declaration = Objects.requireNonNull(
                vertexDeclaration, "vertexDeclaration");
        requirePositive(primitiveCount, "primitiveCount");
        int required = primitiveElementCount(topology, primitiveCount);
        if (vertexOffset < 0 || vertexOffset >= vertices.length) {
            throw new IndexOutOfBoundsException("vertexOffset is outside vertexData");
        }
        if ((long)vertexOffset + required > vertices.length) {
            throw new IndexOutOfBoundsException(
                    "primitiveCount consumes vertices outside vertexData");
        }
        VertexDataCodec codec = VertexDataCodec.select(vertices);
        requireRepresentableDeclaration(codec, declaration);
        NativeBindings.drawUserPrimitives(
                this, topology.ordinal(), codec.userSource(),
                codec.encode(vertices, 0, vertices.length), codec.stride(),
                vertexOffset, vertices.length, primitiveCount,
                declaration.descriptorForUse(this));
    }

    public final <T> void DrawUserIndexedPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int numVertices,
            short[] indexData,
            int indexOffset,
            int primitiveCount) {
        Objects.requireNonNull(vertexData, "vertexData");
        DrawUserIndexedPrimitives(
                primitiveType, vertexData, vertexOffset, numVertices,
                indexData, indexOffset, primitiveCount,
                VertexDeclaration.fromType(vertexData.getClass().getComponentType()));
    }

    public final <T> void DrawUserIndexedPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int numVertices,
            short[] indexData,
            int indexOffset,
            int primitiveCount,
            VertexDeclaration vertexDeclaration) {
        drawUserIndexedPrimitives(
                primitiveType, vertexData, vertexOffset, numVertices,
                Objects.requireNonNull(indexData, "indexData"), indexData.length,
                indexOffset, primitiveCount, vertexDeclaration, false);
    }

    public final <T> void DrawUserIndexedPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int numVertices,
            int[] indexData,
            int indexOffset,
            int primitiveCount) {
        Objects.requireNonNull(vertexData, "vertexData");
        DrawUserIndexedPrimitives(
                primitiveType, vertexData, vertexOffset, numVertices,
                indexData, indexOffset, primitiveCount,
                VertexDeclaration.fromType(vertexData.getClass().getComponentType()));
    }

    public final <T> void DrawUserIndexedPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int numVertices,
            int[] indexData,
            int indexOffset,
            int primitiveCount,
            VertexDeclaration vertexDeclaration) {
        drawUserIndexedPrimitives(
                primitiveType, vertexData, vertexOffset, numVertices,
                Objects.requireNonNull(indexData, "indexData"), indexData.length,
                indexOffset, primitiveCount, vertexDeclaration, true);
    }

    public final GraphicsAdapter getAdapter() {
        ensureOpen();
        int nativeIndex = NativeBindings.getGraphicsDeviceAdapterIndex(this);
        if (adapter == null || adapter.nativeIndex() != nativeIndex) {
            adapter = GraphicsAdapter.forDevice(this, nativeIndex);
        }
        return adapter;
    }

    public final Color getBlendFactor() {
        ensureOpen();
        Color result = new Color();
        result.setPackedValue(Integer.toUnsignedLong(
                NativeBindings.getGraphicsDeviceBlendFactor(this)));
        return result;
    }

    public final void setBlendFactor(Color value) {
        ensureOpen();
        Color snapshot = new Color(Objects.requireNonNull(value, "value"));
        NativeBindings.setGraphicsDeviceBlendFactor(
                this, snapshot.getPackedValue().intValue());
    }

    public final BlendState getBlendState() {
        ensureOpen();
        blendState = BlendState.fromNative(
                NativeBindings.getGraphicsDeviceBlendState(this), this, blendState);
        return blendState;
    }

    public final void setBlendState(BlendState value) {
        ensureOpen();
        BlendState selected = Objects.requireNonNull(value, "value");
        if (blendState == selected) {
            return;
        }
        int[] snapshot = selected.snapshotForBinding();
        NativeBindings.setGraphicsDeviceBlendState(this, snapshot);
        selected.bind(this);
        blendState = selected;
    }

    public final DepthStencilState getDepthStencilState() {
        ensureOpen();
        depthStencilState = DepthStencilState.fromNative(
                NativeBindings.getGraphicsDeviceDepthStencilState(this),
                this,
                depthStencilState);
        return depthStencilState;
    }

    public final void setDepthStencilState(DepthStencilState value) {
        ensureOpen();
        DepthStencilState selected = Objects.requireNonNull(value, "value");
        if (depthStencilState == selected) {
            return;
        }
        int[] snapshot = selected.snapshotForBinding();
        NativeBindings.setGraphicsDeviceDepthStencilState(this, snapshot);
        selected.bind(this);
        depthStencilState = selected;
    }

    public final DisplayMode getDisplayMode() {
        ensureOpen();
        return DisplayMode.fromNative(NativeBindings.getGraphicsDeviceDisplayMode(this));
    }

    public final GraphicsDeviceStatus getGraphicsDeviceStatus() {
        ensureOpen();
        return enumValue(
                GraphicsDeviceStatus.values(),
                NativeBindings.getGraphicsDeviceStatus(this),
                "GraphicsDeviceStatus");
    }

    public final GraphicsProfile getGraphicsProfile() {
        ensureOpen();
        return enumValue(
                GraphicsProfile.values(),
                NativeBindings.getGraphicsDeviceProfile(this),
                "GraphicsProfile");
    }

    public final IndexBuffer getIndices() {
        ensureOpen();
        return NativeBindings.getGraphicsDeviceIndexBuffer(this);
    }

    public final void setIndices(IndexBuffer value) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceIndexBuffer(this, value);
    }

    public final RenderTargetBinding[] GetRenderTargets() {
        ensureOpen();
        return NativeBindings.getGraphicsDeviceRenderTargets(this);
    }

    public final <T> void GetBackBufferData(T[] data) {
        Objects.requireNonNull(data, "data");
        GetBackBufferData(null, data, 0, data.length);
    }

    public final <T> void GetBackBufferData(
            T[] data,
            int startIndex,
            int elementCount) {
        GetBackBufferData(null, data, startIndex, elementCount);
    }

    public final <T> void GetBackBufferData(
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureOpen();
        Objects.requireNonNull(data, "data");
        if (!(data instanceof Color[] colors)) {
            throw new UnsupportedOperationException(
                    "CNA's backbuffer C ABI exposes RGBA8 Color readback only");
        }
        if (data.length == 0) {
            throw new IllegalArgumentException("Backbuffer destination must not be empty");
        }
        validateArrayWindow(data.length, startIndex, elementCount);

        int[] info = NativeBindings.getGraphicsDeviceBackBufferInfo(this);
        int sourceWidth = info[0];
        int sourceHeight = info[1];
        Rectangle region = rect == null ? null : new Rectangle(rect);
        long expected = (long)sourceWidth * sourceHeight;
        if (region != null) {
            if (region.X < 0 || region.Y < 0 || region.Width <= 0 || region.Height <= 0
                    || (long)region.X + region.Width > sourceWidth
                    || (long)region.Y + region.Height > sourceHeight) {
                throw new IllegalArgumentException(
                        "Backbuffer source rectangle is outside the logical buffer");
            }
            expected = (long)region.Width * region.Height;
        }
        if (elementCount != expected) {
            throw new IllegalArgumentException(
                    "Backbuffer element count must be exactly " + expected);
        }

        int[] packed = NativeBindings.getGraphicsDeviceBackBufferData(
                this, region, data.length, startIndex, elementCount);
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            Color color = new Color();
            color.setPackedValue(Integer.toUnsignedLong(packed[index]));
            colors[index] = color;
        }
    }

    public final boolean getIsDisposed() {
        return closed || NativeBindings.isGraphicsDeviceNative(this)
                && NativeBindings.getGraphicsDeviceIsDisposed(this);
    }

    public final int getMultiSampleMask() {
        ensureOpen();
        return NativeBindings.getGraphicsDeviceMultiSampleMask(this);
    }

    public final void setMultiSampleMask(int value) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceMultiSampleMask(this, value);
    }

    public final PresentationParameters getPresentationParameters() {
        ensureOpen();
        int[] value = NativeBindings.getGraphicsDevicePresentationParameters(this);
        PresentationParameters result = new PresentationParameters();
        result.setBackBufferFormat(enumValue(
                SurfaceFormat.values(), value[0], "BackBufferFormat"));
        result.setBackBufferWidth(value[1]);
        result.setBackBufferHeight(value[2]);
        result.setDepthStencilFormat(enumValue(
                DepthFormat.values(), value[3], "DepthStencilFormat"));
        result.setMultiSampleCount(value[4]);
        result.setPresentationInterval(enumValue(
                PresentInterval.values(), value[5], "PresentationInterval"));
        result.setDisplayOrientation(
                Microsoft.Xna.Framework.DisplayOrientation.FromValue(value[6]));
        result.setRenderTargetUsage(enumValue(
                RenderTargetUsage.values(), value[7], "RenderTargetUsage"));
        result.setIsFullScreen(value[8] != 0);
        result.setDeviceWindowHandle(game.getWindow().getHandle());
        return result;
    }

    public final RasterizerState getRasterizerState() {
        ensureOpen();
        int[] integers = new int[4];
        float[] floats = new float[2];
        NativeBindings.getGraphicsDeviceRasterizerState(this, integers, floats);
        rasterizerState = RasterizerState.fromNative(
                integers, floats, this, rasterizerState);
        return rasterizerState;
    }

    public final void setRasterizerState(RasterizerState value) {
        ensureOpen();
        RasterizerState selected = Objects.requireNonNull(value, "value");
        if (rasterizerState == selected) {
            return;
        }
        int[] integers = selected.snapshotIntegersForBinding();
        float[] floats = selected.snapshotFloatsForBinding();
        NativeBindings.setGraphicsDeviceRasterizerState(this, integers, floats);
        selected.bind(this);
        rasterizerState = selected;
    }

    public final int getReferenceStencil() {
        ensureOpen();
        return NativeBindings.getGraphicsDeviceReferenceStencil(this);
    }

    public final void setReferenceStencil(int value) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceReferenceStencil(this, value);
    }

    public final Rectangle getScissorRectangle() {
        ensureOpen();
        int[] value = NativeBindings.getGraphicsDeviceScissorRectangle(this);
        return new Rectangle(value[0], value[1], value[2], value[3]);
    }

    public final void setScissorRectangle(Rectangle value) {
        ensureOpen();
        Rectangle snapshot = new Rectangle(Objects.requireNonNull(value, "value"));
        NativeBindings.setGraphicsDeviceScissorRectangle(
                this, snapshot.X, snapshot.Y, snapshot.Width, snapshot.Height);
    }

    public final Viewport getViewport() {
        ensureOpen();
        int[] bounds = new int[4];
        float[] depth = new float[2];
        NativeBindings.getGraphicsDeviceViewport(this, bounds, depth);
        Viewport result = new Viewport(bounds[0], bounds[1], bounds[2], bounds[3]);
        result.setMinDepth(depth[0]);
        result.setMaxDepth(depth[1]);
        return result;
    }

    public final void setViewport(Viewport value) {
        ensureOpen();
        Viewport snapshot = new Viewport(Objects.requireNonNull(value, "value"));
        NativeBindings.setGraphicsDeviceViewport(
                this,
                snapshot.getX(), snapshot.getY(), snapshot.getWidth(), snapshot.getHeight(),
                snapshot.getMinDepth(), snapshot.getMaxDepth());
    }

    public final SamplerStateCollection getSamplerStates() {
        ensureOpen();
        if (samplerStates == null) {
            samplerStates = new SamplerStateCollection(this, 0, 16);
        }
        return samplerStates;
    }

    public final TextureCollection getTextures() {
        ensureOpen();
        if (textures == null) {
            textures = new TextureCollection(this, 0, 16);
        }
        return textures;
    }

    public final SamplerStateCollection getVertexSamplerStates() {
        ensureOpen();
        if (vertexSamplerStates == null) {
            int slots = getGraphicsProfile() == GraphicsProfile.Reach ? 0 : 4;
            vertexSamplerStates = new SamplerStateCollection(this, 1, slots);
        }
        return vertexSamplerStates;
    }

    public final TextureCollection getVertexTextures() {
        ensureOpen();
        if (vertexTextures == null) {
            int slots = getGraphicsProfile() == GraphicsProfile.Reach ? 0 : 4;
            vertexTextures = new TextureCollection(this, 1, slots);
        }
        return vertexTextures;
    }

    public final void Present() {
        ensureOpen();
        NativeBindings.presentGraphicsDevice(this);
    }

    /**
     * CNA 0.7.0 exposes only whole-device presentation and cannot target rectangles or another
     * native window through its stable C boundary.
     */
    public final void Present(
            Rectangle sourceRectangle,
            Rectangle destinationRectangle,
            WindowHandle overrideWindowHandle) {
        ensureOpen();
        throw new UnsupportedOperationException(
                "CNA's C ABI does not expose rectangle/window-targeted GraphicsDevice.Present");
    }

    public final void Reset() {
        ensureOpen();
        NativeBindings.resetGraphicsDevice(this);
        invalidateStateCaches();
    }

    public final void Reset(PresentationParameters presentationParameters) {
        ensureOpen();
        NativeBindings.resetGraphicsDevice(
                this,
                Objects.requireNonNull(presentationParameters, "presentationParameters"),
                -1);
        invalidateStateCaches();
    }

    public final void Reset(
            PresentationParameters presentationParameters,
            GraphicsAdapter graphicsAdapter) {
        ensureOpen();
        GraphicsAdapter selected = Objects.requireNonNull(graphicsAdapter, "graphicsAdapter");
        NativeBindings.resetGraphicsDevice(
                this,
                Objects.requireNonNull(presentationParameters, "presentationParameters"),
                selected.nativeIndex());
        invalidateStateCaches();
    }

    public final void SetRenderTarget(RenderTarget2D renderTarget) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceRenderTarget2D(this, renderTarget);
    }

    public final void SetRenderTarget(
            RenderTargetCube renderTarget,
            CubeMapFace cubeMapFace) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceRenderTargetCube(
                this, renderTarget, Objects.requireNonNull(cubeMapFace, "cubeMapFace"));
    }

    public final void SetRenderTargets(RenderTargetBinding[] renderTargets) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceRenderTargets(
                this,
                renderTargets == null
                        ? new RenderTargetBinding[0]
                        : renderTargets.clone());
    }

    public final VertexBufferBinding[] GetVertexBuffers() {
        ensureOpen();
        return NativeBindings.getGraphicsDeviceVertexBuffers(this);
    }

    public final void SetVertexBuffer(VertexBuffer vertexBuffer) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceVertexBuffer(this, vertexBuffer, 0);
    }

    public final void SetVertexBuffer(VertexBuffer vertexBuffer, int vertexOffset) {
        ensureOpen();
        if (vertexBuffer != null
                && (vertexOffset < 0 || vertexOffset >= vertexBuffer.getVertexCount())) {
            throw new IndexOutOfBoundsException("vertexOffset is outside the vertex buffer");
        }
        NativeBindings.setGraphicsDeviceVertexBuffer(this, vertexBuffer, vertexOffset);
    }

    public final void SetVertexBuffers(VertexBufferBinding[] vertexBuffers) {
        ensureOpen();
        NativeBindings.setGraphicsDeviceVertexBuffers(
                this,
                vertexBuffers == null ? new VertexBufferBinding[0] : vertexBuffers.clone());
    }

    public final void addDeviceLostListener(EventHandler<EventArgs> listener) {
        ensureOpen();
        deviceLostListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceLostListener(EventHandler<EventArgs> listener) {
        deviceLostListeners.remove(listener);
    }

    public final void addDeviceResetListener(EventHandler<EventArgs> listener) {
        ensureOpen();
        deviceResetListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceResetListener(EventHandler<EventArgs> listener) {
        deviceResetListeners.remove(listener);
    }

    public final void addDeviceResettingListener(EventHandler<EventArgs> listener) {
        ensureOpen();
        deviceResettingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceResettingListener(EventHandler<EventArgs> listener) {
        deviceResettingListeners.remove(listener);
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        ensureOpen();
        disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public final void addResourceCreatedListener(
            EventHandler<ResourceCreatedEventArgs> listener) {
        ensureOpen();
        resourceCreatedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeResourceCreatedListener(
            EventHandler<ResourceCreatedEventArgs> listener) {
        resourceCreatedListeners.remove(listener);
    }

    public final void addResourceDestroyedListener(
            EventHandler<ResourceDestroyedEventArgs> listener) {
        ensureOpen();
        resourceDestroyedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeResourceDestroyedListener(
            EventHandler<ResourceDestroyedEventArgs> listener) {
        resourceDestroyedListeners.remove(listener);
    }

    /** Releases this Java facade; the canonical game remains the native device owner. */
    protected void Dispose(boolean arg0) {
        if (!arg0 || closed) {
            return;
        }
        dispatchDisposing();
        closed = true;
        clearListeners();
    }

    public final void Dispose() {
        Dispose(true);
    }

    @Override
    public final void close() {
        Dispose();
    }

    @SuppressWarnings("unused")
    private void nativeGraphicsDeviceEvent(
            int event,
            boolean payloadPresent,
            byte[] nativeName,
            boolean nativeTagPresent) {
        try {
            switch (event) {
                case 0 -> dispatchDisposing();
                case 1 -> invoke(deviceLostListeners, EventArgs.Empty);
                case 2 -> invoke(deviceResetListeners, EventArgs.Empty);
                case 3 -> invoke(deviceResettingListeners, EventArgs.Empty);
                case 4 -> {
                    GraphicsResource resource = NativeBindings.currentGraphicsResourceEvent();
                    invokeResourceCreated(new ResourceCreatedEventArgs(
                            payloadPresent ? resource : null));
                }
                case 5 -> {
                    GraphicsResource resource = NativeBindings.currentGraphicsResourceEvent();
                    String name = resource == null
                            ? new String(nativeName == null ? new byte[0] : nativeName,
                                    StandardCharsets.UTF_8)
                            : resource.getName();
                    Object tag = resource == null ? null : resource.getTag();
                    // The C event can report only native tag presence, not the managed tag object.
                    if (resource == null && !nativeTagPresent) {
                        tag = null;
                    }
                    invokeResourceDestroyed(new ResourceDestroyedEventArgs(name, tag));
                }
                default -> throw new IllegalArgumentException(
                        "Unknown native graphics-device event " + event);
            }
        } catch (Throwable failure) {
            NativeBindings.recordGraphicsDeviceListenerFailure(this, failure);
        }
    }

    private void dispatchDisposing() {
        if (disposingEventRaised) {
            return;
        }
        disposingEventRaised = true;
        invoke(disposingListeners, EventArgs.Empty);
    }

    private void invoke(
            CopyOnWriteArrayList<EventHandler<EventArgs>> listeners,
            EventArgs args) {
        for (EventHandler<EventArgs> listener : listeners) {
            listener.invoke(this, args);
        }
    }

    private void invokeResourceCreated(ResourceCreatedEventArgs args) {
        for (EventHandler<ResourceCreatedEventArgs> listener : resourceCreatedListeners) {
            listener.invoke(this, args);
        }
    }

    private void invokeResourceDestroyed(ResourceDestroyedEventArgs args) {
        for (EventHandler<ResourceDestroyedEventArgs> listener : resourceDestroyedListeners) {
            listener.invoke(this, args);
        }
    }

    private void clearListeners() {
        deviceLostListeners.clear();
        deviceResetListeners.clear();
        deviceResettingListeners.clear();
        disposingListeners.clear();
        resourceCreatedListeners.clear();
        resourceDestroyedListeners.clear();
    }

    final void ensureOpen() {
        if (closed) {
            throw new IllegalStateException("GraphicsDevice is already closed");
        }
    }

    private void invalidateStateCaches() {
        blendState = null;
        depthStencilState = null;
        rasterizerState = null;
        if (samplerStates != null) {
            samplerStates.invalidate();
        }
        if (vertexSamplerStates != null) {
            vertexSamplerStates.invalidate();
        }
        if (textures != null) {
            textures.invalidate();
        }
        if (vertexTextures != null) {
            vertexTextures.invalidate();
        }
    }

    private static Game requireOwningGame(GraphicsAdapter adapter) {
        GraphicsDevice owner = Objects.requireNonNull(adapter, "adapter").owningDevice();
        if (owner == null) {
            throw new UnsupportedOperationException(
                    "CNA can construct GraphicsDevice only from an adapter associated with a live game");
        }
        owner.ensureOpen();
        return owner.game;
    }

    private static <T> T enumValue(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned an invalid " + name + " value " + index);
        }
        return values[index];
    }

    private static void validateArrayWindow(int length, int startIndex, int elementCount) {
        if (startIndex < 0 || startIndex > length) {
            throw new IndexOutOfBoundsException("Backbuffer start index is outside the array");
        }
        if (elementCount <= 0 || elementCount > length - startIndex) {
            throw new IndexOutOfBoundsException("Backbuffer array window is outside the destination");
        }
    }

    private <T> void drawUserIndexedPrimitives(
            PrimitiveType primitiveType,
            T[] vertexData,
            int vertexOffset,
            int numVertices,
            Object indexData,
            int indexLength,
            int indexOffset,
            int primitiveCount,
            VertexDeclaration vertexDeclaration,
            boolean thirtyTwoBitIndices) {
        ensureOpen();
        PrimitiveType topology = Objects.requireNonNull(primitiveType, "primitiveType");
        T[] vertices = requireVertexArray(vertexData);
        VertexDeclaration declaration = Objects.requireNonNull(
                vertexDeclaration, "vertexDeclaration");
        requirePositive(numVertices, "numVertices");
        requirePositive(primitiveCount, "primitiveCount");
        if (vertexOffset < 0 || vertexOffset >= vertices.length
                || (long)vertexOffset + numVertices > vertices.length) {
            throw new IndexOutOfBoundsException("Vertex window is outside vertexData");
        }
        if (indexLength == 0 || indexOffset < 0 || indexOffset >= indexLength) {
            throw new IndexOutOfBoundsException("indexOffset is outside indexData");
        }
        int requiredIndices = primitiveElementCount(topology, primitiveCount);
        if ((long)indexOffset + requiredIndices > indexLength) {
            throw new IndexOutOfBoundsException(
                    "primitiveCount consumes indices outside indexData");
        }
        VertexDataCodec codec = VertexDataCodec.select(vertices);
        requireRepresentableDeclaration(codec, declaration);
        byte[] payload = codec.encode(vertices, 0, vertices.length);
        int[] descriptor = declaration.descriptorForUse(this);
        if (thirtyTwoBitIndices) {
            NativeBindings.drawUserIndexedPrimitives(
                    this, topology.ordinal(), codec.userSource(), payload, codec.stride(),
                    vertexOffset, numVertices, (int[])indexData, indexOffset,
                    primitiveCount, descriptor);
        } else {
            NativeBindings.drawUserIndexedPrimitives(
                    this, topology.ordinal(), codec.userSource(), payload, codec.stride(),
                    vertexOffset, numVertices, (short[])indexData, indexOffset,
                    primitiveCount, descriptor);
        }
    }

    private static <T> T[] requireVertexArray(T[] vertices) {
        Objects.requireNonNull(vertices, "vertexData");
        if (vertices.length == 0) {
            throw new IllegalArgumentException("vertexData must not be empty");
        }
        return vertices;
    }

    private static void requireRepresentableDeclaration(
            VertexDataCodec codec, VertexDeclaration declaration) {
        if (codec.stride() != declaration.getVertexStride()) {
            throw new UnsupportedOperationException(
                    "CNA's user-primitive descriptor cannot represent a declaration stride "
                            + "different from the Java vertex value size");
        }
    }

    private static int primitiveElementCount(PrimitiveType primitiveType, int primitiveCount) {
        long count = switch (primitiveType) {
            case TriangleList -> (long)primitiveCount * 3;
            case TriangleStrip -> (long)primitiveCount + 2;
            case LineList -> (long)primitiveCount * 2;
            case LineStrip -> (long)primitiveCount + 1;
        };
        if (count > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("primitiveCount overflows the Java array range");
        }
        return (int)count;
    }

    private static void requireNonNegative(int value, String name) {
        if (value < 0) {
            throw new IllegalArgumentException(name + " must not be negative");
        }
    }

    private static void requirePositive(int value, String name) {
        if (value <= 0) {
            throw new IllegalArgumentException(name + " must be positive");
        }
    }
}
