// SPDX-License-Identifier: MIT
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Audio;

namespace CnaKiller
{
    namespace
    {
        Color RandomColor(System::Random& rng, bool randomAlpha = false)
        {
            return {rng.Next(0, 256), rng.Next(0, 256), rng.Next(0, 256),
                    randomAlpha ? rng.Next(0, 256) : 255};
        }

        /**
         * @brief Fills a width*height buffer with adversarial noise from a tiny random palette.
         *
         * Deliberately avoids drawing one random number per pixel: a couple of RNG draws pick a
         * palette and a scatter stride, so even a 1024x1024 spike texture stays cheap to fill
         * while still landing every pixel index through the single seeded stream.
         */
        std::vector<Color> MakeNoiseTexture(System::Random& rng, int width, int height)
        {
            const int paletteSize = rng.Next(2, 9);
            std::vector<Color> palette;
            palette.reserve(static_cast<std::size_t>(paletteSize));
            for (int i = 0; i < paletteSize; ++i)
                palette.push_back(RandomColor(rng, /*randomAlpha=*/true));

            const int strideA = rng.Next(1, 17);
            const int strideB = rng.Next(1, 13);

            std::vector<Color> data(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
            for (std::size_t i = 0; i < data.size(); ++i)
                data[i] = palette[(i * static_cast<std::size_t>(strideA) + static_cast<std::size_t>(strideB))
                                   % palette.size()];
            return data;
        }

        /** @brief Picks a chaotic texture/render-target dimension: usually small, occasionally a huge spike. */
        int RandomDimension(System::Random& rng, int typicalMax, int spikeMax, int spikeOneInN)
        {
            if (rng.Next(0, spikeOneInN) == 0)
                return rng.Next(1, spikeMax + 1);
            return rng.Next(1, typicalMax + 1);
        }

    }

    ChaosEngine::ChaosEngine(CliOptions options, ChaosLog& log)
        : options_(std::move(options))
        , log_(log)
        , random_(static_cast<SharpRuntime::intcs>(options_.seed))
        , seed_(options_.seed)
        , visualRandom_(static_cast<SharpRuntime::intcs>(~options_.seed))
    {
        switch (options_.intensity)
        {
            case Intensity::Low:
                maxPoolSize_ = 8;
                minActionsPerTick_ = 1;
                maxActionsPerTick_ = 1;
                break;
            case Intensity::Medium:
                maxPoolSize_ = 16;
                minActionsPerTick_ = 1;
                maxActionsPerTick_ = 2;
                break;
            case Intensity::High:
                maxPoolSize_ = 32;
                minActionsPerTick_ = 2;
                maxActionsPerTick_ = 4;
                break;
            case Intensity::Nightmare:
                maxPoolSize_ = 64;
                minActionsPerTick_ = 3;
                maxActionsPerTick_ = 6;
                break;
        }
    }

    void ChaosEngine::Bind(Game& game, GraphicsDeviceManager& graphicsManager)
    {
        game_ = &game;
        graphicsManager_ = &graphicsManager;
    }

    GraphicsDevice& ChaosEngine::Device() const
    {
        return *graphicsManager_->getGraphicsDeviceProperty();
    }

    GameWindow& ChaosEngine::Window() const
    {
        return game_->getWindowProperty();
    }

    const char* ChaosEngine::NameOf(ActionId id) const
    {
        switch (id)
        {
            case ActionId::CreateTexture:       return "CreateTexture";
            case ActionId::DestroyTexture:      return "DestroyTexture";
            case ActionId::CreateRenderTarget:  return "CreateRenderTarget";
            case ActionId::DestroyRenderTarget: return "DestroyRenderTarget";
            case ActionId::RenderToTarget:      return "RenderToTarget";
            case ActionId::CreateMesh:          return "CreateMesh";
            case ActionId::DestroyMesh:         return "DestroyMesh";
            case ActionId::DrawMesh:            return "DrawMesh";
            case ActionId::CreateSound:         return "CreateSound";
            case ActionId::PlaySound:           return "PlaySound";
            case ActionId::DestroySound:        return "DestroySound";
            case ActionId::ReloadShader:        return "ReloadShader";
            case ActionId::ResizeBackBuffer:    return "ResizeBackBuffer";
            case ActionId::ToggleFullScreen:    return "ToggleFullScreen";
            case ActionId::ToggleBorderless:    return "ToggleBorderless";
            case ActionId::ChangeTitle:         return "ChangeTitle";
            case ActionId::MinimizeRestore:     return "MinimizeRestore";
            case ActionId::ToggleMouseVisible:  return "ToggleMouseVisible";
            case ActionId::ResetDevice:         return "ResetDevice";
            case ActionId::SpamRenderState:     return "SpamRenderState";
        }
        return "Unknown";
    }

    ChaosEngine::ActionId ChaosEngine::PickAction()
    {
        struct Entry { ActionId id; int weight; };

        // Base weights favor steady resource churn; the "global disruption" actions near the
        // bottom (resize/fullscreen/reset/window-state) get scaled up at higher intensities
        // since those are the ones most likely to actually break a real application.
        const int disruptionScale = 1 + static_cast<int>(options_.intensity); // Low=1 .. Nightmare=4

        const std::array<Entry, 20> table{{
            {ActionId::CreateTexture,       10},
            {ActionId::DestroyTexture,       6},
            {ActionId::CreateRenderTarget,   6},
            {ActionId::DestroyRenderTarget,  4},
            {ActionId::RenderToTarget,       8},
            {ActionId::CreateMesh,           8},
            {ActionId::DestroyMesh,          5},
            {ActionId::DrawMesh,             8},
            {ActionId::CreateSound,          4},
            {ActionId::PlaySound,            6},
            {ActionId::DestroySound,         3},
            {ActionId::ReloadShader,         5},
            {ActionId::ResizeBackBuffer,     2 * disruptionScale},
            {ActionId::ToggleFullScreen,     1 * disruptionScale},
            {ActionId::ToggleBorderless,     1 * disruptionScale},
            {ActionId::ChangeTitle,          3},
            {ActionId::MinimizeRestore,      1 * disruptionScale},
            {ActionId::ToggleMouseVisible,   2},
            {ActionId::ResetDevice,          1 * disruptionScale},
            {ActionId::SpamRenderState,      6},
        }};

        int total = 0;
        for (const auto& entry : table)
            total += entry.weight;

        int roll = random_.Next(0, total);
        for (const auto& entry : table)
        {
            if (roll < entry.weight)
                return entry.id;
            roll -= entry.weight;
        }
        return table.back().id;
    }

    bool ChaosEngine::Tick(std::uint64_t tick, double totalSeconds)
    {
        if (options_.stopAtTick != 0 && tick >= options_.stopAtTick)
        {
            log_.Note("stop-at-tick " + std::to_string(options_.stopAtTick) + " reached; exiting cleanly");
            return false;
        }
        if (options_.maxTicks != 0 && tick > options_.maxTicks)
        {
            log_.Note("max-ticks " + std::to_string(options_.maxTicks) + " reached; exiting cleanly");
            return false;
        }
        if (options_.maxSeconds > 0.0 && totalSeconds >= options_.maxSeconds)
        {
            log_.Note("duration limit reached; exiting cleanly");
            return false;
        }

        const int actionCount = (minActionsPerTick_ == maxActionsPerTick_)
            ? minActionsPerTick_
            : random_.Next(minActionsPerTick_, maxActionsPerTick_ + 1);

        for (int i = 0; i < actionCount; ++i)
        {
            const ActionId id = PickAction();
            lastActionName_ = NameOf(id);
            log_.BeginAction(tick, lastActionName_, StatsSummary(tick));
            Dispatch(id);
        }
        return true;
    }

    void ChaosEngine::Dispatch(ActionId id)
    {
        switch (id)
        {
            case ActionId::CreateTexture:       ActionCreateTexture(); break;
            case ActionId::DestroyTexture:      ActionDestroyTexture(); break;
            case ActionId::CreateRenderTarget:  ActionCreateRenderTarget(); break;
            case ActionId::DestroyRenderTarget: ActionDestroyRenderTarget(); break;
            case ActionId::RenderToTarget:      ActionRenderToTarget(); break;
            case ActionId::CreateMesh:          ActionCreateMesh(); break;
            case ActionId::DestroyMesh:         ActionDestroyMesh(); break;
            case ActionId::DrawMesh:            ActionDrawMesh(); break;
            case ActionId::CreateSound:         ActionCreateSound(); break;
            case ActionId::PlaySound:           ActionPlaySound(); break;
            case ActionId::DestroySound:        ActionDestroySound(); break;
            case ActionId::ReloadShader:        ActionReloadShader(); break;
            case ActionId::ResizeBackBuffer:    ActionResizeBackBuffer(); break;
            case ActionId::ToggleFullScreen:    ActionToggleFullScreen(); break;
            case ActionId::ToggleBorderless:    ActionToggleBorderless(); break;
            case ActionId::ChangeTitle:         ActionChangeTitle(); break;
            case ActionId::MinimizeRestore:     ActionMinimizeRestore(); break;
            case ActionId::ToggleMouseVisible:  ActionToggleMouseVisible(); break;
            case ActionId::ResetDevice:         ActionResetDevice(); break;
            case ActionId::SpamRenderState:     ActionSpamRenderState(); break;
        }
    }

    // -----------------------------------------------------------------------------------------
    // Textures
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionCreateTexture()
    {
        EvictIfFull(textures_);
        const int width = RandomDimension(random_, 128, 1024, 20);
        const int height = RandomDimension(random_, 128, 1024, 20);

        auto texture = std::make_unique<Texture2D>(Device(), width, height);
        const std::vector<Color> data = MakeNoiseTexture(random_, width, height);
        texture->SetData(data.data(), static_cast<int>(data.size()));
        textures_.Add(std::move(texture));
    }

    void ChaosEngine::ActionDestroyTexture()
    {
        textures_.DestroyRandom(random_);
    }

    // -----------------------------------------------------------------------------------------
    // Render targets
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionCreateRenderTarget()
    {
        EvictIfFull(renderTargets_);
        const int width = RandomDimension(random_, 256, 2048, 15);
        const int height = RandomDimension(random_, 256, 2048, 15);

        if (random_.Next(0, 2) == 0)
        {
            renderTargets_.Add(std::make_unique<RenderTarget2D>(Device(), width, height));
        }
        else
        {
            // Occasionally exercise the fuller constructor: mip chains, depth-stencil, MSAA and
            // content-preservation are all extra ways for a render target's lifetime to go wrong.
            const bool mipMap = random_.Next(0, 2) == 0;
            const std::array<DepthFormat, 4> depthFormats{
                DepthFormat::None, DepthFormat::Depth16, DepthFormat::Depth24, DepthFormat::Depth24Stencil8};
            const DepthFormat depthFormat = depthFormats[static_cast<std::size_t>(random_.Next(0, 4))];
            const std::array<RenderTargetUsage, 3> usages{
                RenderTargetUsage::DiscardContents, RenderTargetUsage::PreserveContents,
                RenderTargetUsage::PlatformContents};
            const RenderTargetUsage usage = usages[static_cast<std::size_t>(random_.Next(0, 3))];
            const int multiSample = random_.Next(0, 2) == 0 ? 0 : (1 << random_.Next(0, 3)) * 2;

            renderTargets_.Add(std::make_unique<RenderTarget2D>(
                Device(), width, height, mipMap, SurfaceFormat::Color, depthFormat, multiSample, usage));
        }
    }

    void ChaosEngine::ActionDestroyRenderTarget()
    {
        renderTargets_.DestroyRandom(random_);
    }

    void ChaosEngine::ActionRenderToTarget()
    {
        // Deliberately misbehaved: a well-formed XNA game only touches render targets inside
        // Draw(), between BeginDraw()/EndDraw(). Chaos actions run from Update(), so this binds
        // and clears an offscreen target completely outside that contract on purpose.
        if (renderTargets_.Empty())
        {
            ActionCreateRenderTarget();
            if (renderTargets_.Empty())
                return;
        }

        RenderTarget2D& target = renderTargets_.RandomItem(random_);
        GraphicsDevice& device = Device();
        device.SetRenderTarget(&target);
        device.Clear(RandomColor(random_));
        device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // Meshes
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionCreateMesh()
    {
        EvictIfFull(meshes_);

        const int triangleCount = random_.Next(1, 200);
        const int vertexCount = triangleCount * 3;

        auto mesh = std::make_unique<ManagedMesh>();
        mesh->primitiveCount = triangleCount;
        mesh->vertexBuffer = std::make_unique<VertexBuffer>(
            Device(), VertexPositionColor::getVertexDeclarationStatic(), vertexCount, BufferUsage::None);
        mesh->indexBuffer = std::make_unique<IndexBuffer>(
            Device(), IndexElementSize::ThirtyTwoBits, vertexCount, BufferUsage::None);

        std::vector<VertexPositionColor> vertices;
        vertices.reserve(static_cast<std::size_t>(vertexCount));
        std::vector<std::uint32_t> indices;
        indices.reserve(static_cast<std::size_t>(vertexCount));
        for (int i = 0; i < vertexCount; ++i)
        {
            const float x = static_cast<float>(random_.NextDouble()) * 2.0f - 1.0f;
            const float y = static_cast<float>(random_.NextDouble()) * 2.0f - 1.0f;
            vertices.emplace_back(Vector3(x, y, 0.0f), RandomColor(random_));
            indices.push_back(static_cast<std::uint32_t>(i));
        }

        mesh->vertexBuffer->SetData(vertices.data(), static_cast<int>(vertices.size()));
        mesh->indexBuffer->SetData(indices.data(), static_cast<int>(indices.size()));
        meshes_.Add(std::move(mesh));
    }

    void ChaosEngine::ActionDestroyMesh()
    {
        meshes_.DestroyRandom(random_);
    }

    void ChaosEngine::ActionDrawMesh()
    {
        if (meshes_.Empty())
        {
            ActionCreateMesh();
            if (meshes_.Empty())
                return;
        }
        if (effects_.Empty())
            ActionReloadShader();
        if (effects_.Empty())
            return;

        ManagedMesh& mesh = meshes_.RandomItem(random_);
        BasicEffect& effect = effects_.RandomItem(random_);
        GraphicsDevice& device = Device();

        device.SetVertexBuffer(mesh.vertexBuffer.get());
        device.SetIndexBuffer(mesh.indexBuffer.get());
        effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0,
                                     mesh.primitiveCount * 3, 0, mesh.primitiveCount);
    }

    // -----------------------------------------------------------------------------------------
    // Audio
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionCreateSound()
    {
        EvictIfFull(sounds_);

        const int sampleRate = random_.Next(8000, 48001);
        const AudioChannels channels = random_.Next(0, 2) == 0 ? AudioChannels::Mono : AudioChannels::Stereo;
        const double durationSeconds = 0.05 + random_.NextDouble() * 0.4;
        const int sampleCount =
            static_cast<int>(durationSeconds * sampleRate) * static_cast<int>(channels);

        // A short, loud burst of tone-ish noise: cheap to synthesize and unpleasant on purpose --
        // this "game" is not trying to be pleasant, it is trying to break the audio backend.
        const double frequency = 80.0 + random_.NextDouble() * 4000.0;
        std::vector<SharpRuntime::bytecs> pcm(static_cast<std::size_t>(sampleCount) * 2);
        for (int i = 0; i < sampleCount; ++i)
        {
            const double t = static_cast<double>(i) / sampleRate;
            const double sample = std::sin(2.0 * MathHelper::Pi * frequency * t);
            const auto amplitude = static_cast<std::int16_t>(sample * 20000.0);
            pcm[static_cast<std::size_t>(i) * 2 + 0] = static_cast<SharpRuntime::bytecs>(amplitude & 0xFF);
            pcm[static_cast<std::size_t>(i) * 2 + 1] = static_cast<SharpRuntime::bytecs>((amplitude >> 8) & 0xFF);
        }

        auto sound = std::make_unique<ManagedSound>();
        sound->soundEffect = std::make_unique<SoundEffect>(pcm, sampleRate, channels);
        sounds_.Add(std::move(sound));
    }

    void ChaosEngine::ActionPlaySound()
    {
        if (sounds_.Empty())
        {
            ActionCreateSound();
            if (sounds_.Empty())
                return;
        }
        sounds_.RandomItem(random_).soundEffect->Play();
    }

    void ChaosEngine::ActionDestroySound()
    {
        sounds_.DestroyRandom(random_);
    }

    // -----------------------------------------------------------------------------------------
    // "Shader hot reload" -- BasicEffect churn
    // -----------------------------------------------------------------------------------------
    //
    // CNA's public API only accepts pre-compiled effect bytecode (Effect(device, bytes)), the
    // same as real XNA/FNA; there is no runtime GLSL/HLSL source compiler to hot-swap. What a
    // shader hot-reload system actually does to a running game, though, is exactly this:
    // dispose the effect object that every mesh is holding a technique/pass from and replace it
    // with a freshly constructed one with different parameters, while draws may still be in
    // flight. Churning BasicEffect instances (which own real per-permutation GPU shader
    // programs selected by VertexColorEnabled/TextureEnabled/LightingEnabled) reproduces that
    // failure mode faithfully without needing an offline content pipeline asset on disk.

    void ChaosEngine::ActionReloadShader()
    {
        EvictIfFull(effects_);

        auto effect = std::make_unique<BasicEffect>(Device());
        effect->VertexColorEnabled = random_.Next(0, 2) == 0;
        effect->setTextureEnabledProperty(false);
        effect->setLightingEnabledProperty(false);
        effect->setAlphaProperty(0.4f + static_cast<float>(random_.NextDouble()) * 0.6f);
        effect->setDiffuseColorProperty(Vector3(
            static_cast<float>(random_.NextDouble()),
            static_cast<float>(random_.NextDouble()),
            static_cast<float>(random_.NextDouble())));
        effect->World = Matrix::getIdentityProperty();
        effect->View = Matrix::getIdentityProperty();
        effect->Projection = Matrix::getIdentityProperty();

        effects_.Add(std::move(effect));
    }

    // -----------------------------------------------------------------------------------------
    // Window / device chaos
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionResizeBackBuffer()
    {
        // Mostly plausible desktop sizes, with rare extreme edge values (as small as 1x1) to
        // stress whatever clamps -- or fails to clamp -- degenerate back buffer dimensions.
        const int width = RandomDimension(random_, 1920, 4096, 12);
        const int height = RandomDimension(random_, 1080, 4096, 12);
        graphicsManager_->setPreferredBackBufferWidthProperty(width);
        graphicsManager_->setPreferredBackBufferHeightProperty(height);
        graphicsManager_->ApplyChanges();
    }

    void ChaosEngine::ActionToggleFullScreen()
    {
        graphicsManager_->ToggleFullScreen();
    }

    void ChaosEngine::ActionToggleBorderless()
    {
        borderless_ = !borderless_;
        Window().setIsBorderlessEXTProperty(borderless_);
    }

    void ChaosEngine::ActionChangeTitle()
    {
        std::ostringstream title;
        switch (random_.Next(0, 4))
        {
            case 0:
                title << "";
                break;
            case 1:
                title << "cna-killer :: seed=" << seed_;
                break;
            case 2:
            {
                static constexpr const char* kGlyphs = "!@#$%^&*()_+-=[]{}|;:,.<>/?~`\"'\\";
                const int length = random_.Next(1, 64);
                for (int i = 0; i < length; ++i)
                    title << kGlyphs[random_.Next(0, static_cast<int>(std::char_traits<char>::length(kGlyphs)))];
                break;
            }
            default:
            {
                const int length = random_.Next(500, 2000);
                for (int i = 0; i < length; ++i)
                    title << static_cast<char>('a' + random_.Next(0, 26));
                break;
            }
        }
        Window().setTitleProperty(title.str());
    }

    void ChaosEngine::ActionMinimizeRestore()
    {
        minimized_ = !minimized_;
        if (minimized_)
            Window().MinimizeEXT();
        else
            Window().RestoreEXT();
    }

    void ChaosEngine::ActionToggleMouseVisible()
    {
        mouseVisible_ = !mouseVisible_;
        game_->setIsMouseVisibleProperty(mouseVisible_);
    }

    void ChaosEngine::ActionResetDevice()
    {
        GraphicsDevice& device = Device();
        if (random_.Next(0, 2) == 0)
        {
            device.Reset();
            return;
        }

        // The nastier path: mutate a copy of the live presentation parameters and hand it back
        // directly to GraphicsDevice::Reset(), bypassing GraphicsDeviceManager entirely. Real
        // games never do this -- they always go through ApplyChanges() -- so this is exactly
        // the kind of desync between the manager's cached preferences and the device's actual
        // state that a well-behaved app is never supposed to trigger.
        PresentationParameters pp = device.getPresentationParametersProperty();
        pp.setBackBufferWidthProperty(RandomDimension(random_, 1280, 3840, 10));
        pp.setBackBufferHeightProperty(RandomDimension(random_, 720, 2160, 10));
        device.Reset(pp);
    }

    void ChaosEngine::ActionSpamRenderState()
    {
        GraphicsDevice& device = Device();

        const std::array<BlendState, 4> blends{
            BlendState::Opaque, BlendState::AlphaBlend, BlendState::Additive, BlendState::NonPremultiplied};
        device.setBlendStateProperty(blends[static_cast<std::size_t>(random_.Next(0, 4))]);

        const std::array<DepthStencilState, 3> depthStates{
            DepthStencilState::Default, DepthStencilState::DepthRead, DepthStencilState::None};
        device.setDepthStencilStateProperty(depthStates[static_cast<std::size_t>(random_.Next(0, 3))]);

        const std::array<RasterizerState, 3> rasterStates{
            RasterizerState::CullClockwise, RasterizerState::CullCounterClockwise, RasterizerState::CullNone};
        device.setRasterizerStateProperty(rasterStates[static_cast<std::size_t>(random_.Next(0, 3))]);

        const Viewport& current = device.getViewportProperty();
        const int maxWidth = std::max(1, current.getWidthProperty());
        const int maxHeight = std::max(1, current.getHeightProperty());
        const int w = random_.Next(1, maxWidth + 1);
        const int h = random_.Next(1, maxHeight + 1);
        const int x = random_.Next(0, std::max(1, maxWidth - w + 1));
        const int y = random_.Next(0, std::max(1, maxHeight - h + 1));
        device.setScissorRectangleProperty(Rectangle(x, y, w, h));
    }

    // -----------------------------------------------------------------------------------------
    // Rendering the chaos and reporting on it
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::Draw(SpriteBatch& spriteBatch)
    {
        if (textures_.Empty())
            return;

        const int viewportWidth = std::max(1, Device().getViewportProperty().getWidthProperty());
        const int viewportHeight = std::max(1, Device().getViewportProperty().getHeightProperty());

        spriteBatch.Begin();
        const int spriteCount = std::min<int>(static_cast<int>(textures_.Size()), 24);
        for (int i = 0; i < spriteCount; ++i)
        {
            Texture2D& texture = textures_.RandomItem(visualRandom_);
            const Vector2 position(
                static_cast<float>(visualRandom_.Next(0, viewportWidth)),
                static_cast<float>(visualRandom_.Next(0, viewportHeight)));
            const float rotation = static_cast<float>(visualRandom_.NextDouble()) * MathHelper::TwoPi;
            const float scale = 0.1f + static_cast<float>(visualRandom_.NextDouble()) * 0.5f;
            spriteBatch.Draw(texture, position, std::nullopt, RandomColor(visualRandom_, /*randomAlpha=*/true),
                             rotation, Vector2::Zero, scale, SpriteEffects::None, 0.0f);
        }
        spriteBatch.End();
    }

    std::string ChaosEngine::StatsSummary(std::uint64_t tick) const
    {
        std::ostringstream out;
        out << "tick=" << tick
            << " seed=" << seed_
            << " intensity=" << ToString(options_.intensity)
            << " tex=" << textures_.Size()
            << " rt=" << renderTargets_.Size()
            << " mesh=" << meshes_.Size()
            << " snd=" << sounds_.Size()
            << " fx=" << effects_.Size()
            << " last=" << lastActionName_;
        return out.str();
    }
}
