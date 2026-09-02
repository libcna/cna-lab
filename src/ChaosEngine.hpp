// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "System/Random.hpp"

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include "ChaosLog.hpp"
#include "CliOptions.hpp"
#include "ResourcePool.hpp"

namespace CnaKiller
{
    /** @brief A GPU mesh's vertex/index buffer pair, churned and destroyed as a single unit. */
    struct ManagedMesh
    {
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> vertexBuffer;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::IndexBuffer> indexBuffer;
        int primitiveCount = 0;
    };

    /** @brief A procedurally generated audio resource churned by the audio chaos actions. */
    struct ManagedSound
    {
        std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> soundEffect;
    };

    /**
     * @brief Deliberately hostile stress engine that drives the CNA runtime (../cnanext) into
     * the ground through its own public XNA-compatible API.
     *
     * It continuously creates and destroys textures, render targets, meshes and audio
     * resources; resizes and toggles the window between fullscreen/windowed/borderless;
     * simulates minimize/restore ("alt-tab"); forces raw GraphicsDevice::Reset() calls
     * ("device loss"); and churns BasicEffect instances to simulate a shader hot-reload
     * pipeline swapping effects out from under whatever is mid-frame.
     *
     * Every one of those decisions -- which action, how big a texture, which pool entry to
     * kill -- is drawn from a single seeded System::Random and depends only on the tick
     * count, never on wall-clock time or frame duration. That is what makes a crash
     * reproducible: rerunning with the same --seed replays the identical action sequence
     * tick-for-tick, and --stop-at-tick lets that sequence be bisected down to a minimal
     * repro.
     */
    class ChaosEngine
    {
    public:
        ChaosEngine(CliOptions options, ChaosLog& log);

        /** @brief Binds the engine to the running game once its GraphicsDevice exists (post-Initialize). */
        void Bind(Microsoft::Xna::Framework::Game& game,
                  Microsoft::Xna::Framework::GraphicsDeviceManager& graphicsManager);

        /**
         * @brief Runs this tick's batch of chaos actions.
         * @return false once the configured stop condition (max ticks / duration / stop-at-tick)
         *         has been reached and the caller should Exit() the game.
         */
        bool Tick(std::uint64_t tick, double totalSeconds);

        /** @brief Draws whatever chaos is currently alive so the mayhem is visible on screen. */
        void Draw(Microsoft::Xna::Framework::Graphics::SpriteBatch& spriteBatch);

        /** @brief One-line HUD summary (tick, seed, pool sizes, last action) for the window title. */
        [[nodiscard]] std::string StatsSummary(std::uint64_t tick) const;

        [[nodiscard]] std::uint64_t Seed() const { return seed_; }

    private:
        enum class ActionId
        {
            CreateTexture,
            DestroyTexture,
            CreateRenderTarget,
            DestroyRenderTarget,
            RenderToTarget,
            CreateMesh,
            DestroyMesh,
            DrawMesh,
            CreateSound,
            PlaySound,
            DestroySound,
            ReloadShader,
            ResizeBackBuffer,
            ToggleFullScreen,
            ToggleBorderless,
            ChangeTitle,
            MinimizeRestore,
            ToggleMouseVisible,
            ResetDevice,
            SpamRenderState,
        };

        CliOptions options_;
        ChaosLog& log_;
        System::Random random_;
        std::uint64_t seed_;

        // Draw() runs once per rendered frame, which -- unlike Update()'s fixed-step ticks --
        // is not guaranteed to happen the same number of times across two runs with identical
        // chaos (frame pacing, vsync, and a slower/faster machine all affect it). It gets its
        // own seeded stream so cosmetic sprite placement can vary run to run without ever
        // perturbing the action-selection stream that --seed is actually reproducing.
        System::Random visualRandom_;

        Microsoft::Xna::Framework::Game* game_ = nullptr;
        Microsoft::Xna::Framework::GraphicsDeviceManager* graphicsManager_ = nullptr;

        std::size_t maxPoolSize_;
        int minActionsPerTick_;
        int maxActionsPerTick_;

        ResourcePool<Microsoft::Xna::Framework::Graphics::Texture2D> textures_;
        ResourcePool<Microsoft::Xna::Framework::Graphics::RenderTarget2D> renderTargets_;
        ResourcePool<ManagedMesh> meshes_;
        ResourcePool<ManagedSound> sounds_;
        ResourcePool<Microsoft::Xna::Framework::Graphics::BasicEffect> effects_;

        std::string lastActionName_ = "<none>";
        bool mouseVisible_ = true;
        bool borderless_ = false;
        bool minimized_ = false;

        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& Device() const;
        [[nodiscard]] Microsoft::Xna::Framework::GameWindow& Window() const;

        [[nodiscard]] ActionId PickAction();
        [[nodiscard]] const char* NameOf(ActionId id) const;
        void Dispatch(ActionId id);

        /** @brief Destroys a random pool entry first if @p pool is already at the configured cap. */
        template <typename T>
        void EvictIfFull(ResourcePool<T>& pool)
        {
            if (pool.Size() >= maxPoolSize_)
                pool.DestroyRandom(random_);
        }

        void ActionCreateTexture();
        void ActionDestroyTexture();
        void ActionCreateRenderTarget();
        void ActionDestroyRenderTarget();
        void ActionRenderToTarget();
        void ActionCreateMesh();
        void ActionDestroyMesh();
        void ActionDrawMesh();
        void ActionCreateSound();
        void ActionPlaySound();
        void ActionDestroySound();
        void ActionReloadShader();
        void ActionResizeBackBuffer();
        void ActionToggleFullScreen();
        void ActionToggleBorderless();
        void ActionChangeTitle();
        void ActionMinimizeRestore();
        void ActionToggleMouseVisible();
        void ActionResetDevice();
        void ActionSpamRenderState();
    };
}
