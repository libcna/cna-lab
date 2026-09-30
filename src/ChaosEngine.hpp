// SPDX-License-Identifier: MIT
#pragma once

#include <atomic>
#include <cstdint>
#include <exception>
#include <functional>
#include <thread>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "System/Random.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Audio/DynamicSoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"

#include "ChaosLog.hpp"
#include "CliOptions.hpp"
#include "Findings.hpp"
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

    /**
     * @brief A procedurally generated sound, shared with the instances created from it.
     *
     * Destroying the pool entry drops only the pool's reference: an XNA SoundEffect stays alive
     * while an instance refers to it, and an instance outliving its parent's C++ object would be
     * cna-killer's own use-after-free rather than a finding about CNA.
     */
    struct ManagedSound
    {
        std::shared_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> soundEffect;
    };

    /** @brief A SoundEffectInstance and the SoundEffect it was created from (destroyed after it). */
    struct ManagedInstance
    {
        std::shared_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> parent;
        std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffectInstance> instance;
    };

    /** @brief A streaming voice with the block size its format needs to stay aligned. */
    struct ManagedDynamicSound
    {
        std::unique_ptr<Microsoft::Xna::Framework::Audio::DynamicSoundEffectInstance> instance;
        int blockAlign = 2;
        int sampleRate = 22050;
    };

    class ChaosComponent;

    /** @brief One worker-thread job: what it was asked to do and what it produced. */
    struct WorkerJob
    {
        std::thread thread;
        std::atomic<bool> done{false};
        std::exception_ptr failure;
        std::string kind;
        int width = 0;
        int height = 0;
        std::vector<Microsoft::Xna::Framework::Color> expected;
        std::vector<Microsoft::Xna::Framework::Color> readBack;
        std::vector<std::unique_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>> textures;
        std::unique_ptr<ManagedMesh> mesh;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::RenderTarget2D> target;
    };

    /**
     * @brief Deliberately hostile stress engine that drives the CNA runtime (../cna) into the
     * ground through its own public XNA-compatible API.
     *
     * Every tick it picks actions from a weighted table. Some only churn -- create, destroy,
     * draw, resize, reset -- and count on a crash to speak for itself. Others check: they read
     * back what they wrote or drew and compare it with what XNA produces, call the API the way
     * XNA refuses and expect XNA's exception, or feed decoders corrupted input and expect a
     * clean refusal. What they catch goes to Findings; a finding does not stop the run unless
     * --strict asks for it.
     *
     * Every decision -- which action, how big a texture, which byte to corrupt -- is drawn from
     * a single seeded System::Random and depends only on the tick count, never on wall-clock
     * time or frame duration, so rerunning with the same --seed (and the same --only/--exclude)
     * replays the identical action sequence tick-for-tick.
     */
    class ChaosEngine
    {
    public:
        ChaosEngine(CliOptions options, ChaosLog& log, Findings& findings);
        ~ChaosEngine();

        ChaosEngine(const ChaosEngine&) = delete;
        ChaosEngine& operator=(const ChaosEngine&) = delete;

        /** @brief Prints every action with its family and weight. */
        static void ListActions(std::ostream& out);

        /** @brief Checks --only/--exclude against the action table; returns false with a reason. */
        [[nodiscard]] static bool ValidateFilter(const CliOptions& options, std::string& error);

        /** @brief Binds the engine to the running game once its GraphicsDevice exists (post-Initialize). */
        void Bind(Microsoft::Xna::Framework::Game& game,
                  Microsoft::Xna::Framework::GraphicsDeviceManager& graphicsManager);

        /**
         * @brief Runs this tick's batch of chaos actions.
         * @return false once the configured stop condition (max ticks / duration / stop-at-tick)
         *         has been reached and the caller should Exit() the game.
         */
        bool Tick(std::uint64_t tick, double totalSeconds);

        /** @brief Draws whatever chaos is currently alive and runs the draw-phase checks queued by Tick. */
        void Draw(Microsoft::Xna::Framework::Graphics::SpriteBatch& spriteBatch);

        /** @brief Releases every resource while the device still exists. */
        void Shutdown();

        /** @brief One-line HUD summary (tick, seed, pool sizes, last action) for the window title. */
        [[nodiscard]] std::string StatsSummary(std::uint64_t tick) const;

        [[nodiscard]] std::uint64_t Seed() const { return seed_; }

    private:
        friend class ChaosComponent;

        struct ActionDef
        {
            const char* name;
            const char* family;
            int weight;
            /** Weight scales with intensity: the actions most likely to break a real application. */
            bool disruptive;
            void (ChaosEngine::*run)();
        };

        static const std::vector<ActionDef>& Actions();

        CliOptions options_;
        ChaosLog& log_;
        Findings& findings_;
        System::Random random_;
        std::uint64_t seed_;
        std::uint64_t tick_ = 0;

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
        std::vector<const ActionDef*> enabled_;

        ResourcePool<Microsoft::Xna::Framework::Graphics::Texture2D> textures_;
        ResourcePool<Microsoft::Xna::Framework::Graphics::RenderTarget2D> renderTargets_;
        ResourcePool<ManagedMesh> meshes_;
        ResourcePool<ManagedSound> sounds_;
        ResourcePool<ManagedInstance> instances_;
        ResourcePool<ManagedDynamicSound> dynamicSounds_;
        ResourcePool<Microsoft::Xna::Framework::Graphics::BasicEffect> effects_;
        std::vector<std::unique_ptr<ChaosComponent>> components_;
        /** Resources a worker job created; the seeded stream never picks from here. */
        ResourcePool<Microsoft::Xna::Framework::Graphics::Texture2D> workerTextures_;
        std::unique_ptr<WorkerJob> workerJob_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> white_;

        /** Checks that must run inside Draw(), queued by the tick that decided them. */
        std::vector<std::pair<std::string, std::function<void()>>> drawQueue_;

        std::string lastActionName_ = "<none>";
        bool mouseVisible_ = true;
        bool borderless_ = false;
        bool minimized_ = false;
        bool audioAvailable_ = true;
        int deviceResetting_ = 0;
        int deviceReset_ = 0;
        std::vector<std::pair<std::uint64_t, long>> rssSamples_;

        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& Device() const;
        [[nodiscard]] Microsoft::Xna::Framework::GameWindow& Window() const;

        [[nodiscard]] const ActionDef& PickAction();
        void RunGuarded(const std::string& name, const std::function<void()>& body);
        void SampleResidentMemory();

        /** @brief Destroys a random pool entry first if @p pool is already at the configured cap. */
        template <typename T>
        void EvictIfFull(ResourcePool<T>& pool)
        {
            if (pool.Size() >= maxPoolSize_)
                pool.DestroyRandom(random_);
        }

        // --- helpers shared by the action files -------------------------------------------
        void Report(FindingKind kind, const std::string& what, const std::string& detail);
        template <typename Expected, typename Call>
        void Expect(const std::string& what, Call&& call)
        {
            ExpectRefusal<Expected>(findings_, lastActionName_, what, std::forward<Call>(call));
        }
        template <typename Call>
        bool Tolerate(const std::string& what, Call&& call)
        {
            return Survive(findings_, lastActionName_, what, std::forward<Call>(call));
        }
        [[nodiscard]] int RandomInt(int minInclusive, int maxExclusive);
        [[nodiscard]] bool Chance(int oneIn);
        [[nodiscard]] float RandomFloat(float minValue, float maxValue);
        [[nodiscard]] std::vector<std::uint8_t> RandomBytes(std::size_t count);
        [[nodiscard]] Microsoft::Xna::Framework::Color RandomOpaqueColor();
        [[nodiscard]] Microsoft::Xna::Framework::Color RandomAnyColor();
        void Mutate(std::vector<std::uint8_t>& bytes);
        [[nodiscard]] Microsoft::Xna::Framework::Graphics::Texture2D& WhiteTexture();
        void QueueDrawCheck(const std::string& name, std::function<void()> check);

        // --- resource churn and window/device chaos (ChaosEngine.cpp) ------------------------
        void ActionCreateTexture();
        void ActionDestroyTexture();
        void ActionCopyTexture();
        void ActionCreateRenderTarget();
        void ActionDestroyRenderTarget();
        void ActionRenderToTarget();
        void ActionCreateMesh();
        void ActionDestroyMesh();
        void ActionDrawMesh();
        void ActionReloadShader();
        void ActionResizeBackBuffer();
        void ActionToggleFullScreen();
        void ActionToggleBorderless();
        void ActionChangeTitle();
        void ActionMinimizeRestore();
        void ActionToggleMouseVisible();
        void ActionAllowUserResizing();
        void ActionResetDevice();
        void ActionChangeGraphicsSettings();
        void ActionSpamRenderState();

        // --- rendering (ChaosRender.cpp) ---------------------------------------------------
        void ActionDrawPrimitiveTypes();
        void ActionDrawStockEffects();
        void ActionSpriteBatchChaos();
        void ActionOcclusionQueries();
        void ActionDrawInstanced();
        void ActionCubeAndVolumeTextures();

        // --- verification (ChaosVerify.cpp) ------------------------------------------------
        void ActionVerifyTextureRoundTrip();
        void ActionVerifyRenderTargetClear();
        void ActionVerifyMultipleRenderTargets();
        void ActionVerifyCubeRenderTarget();
        void ActionVerifySolidQuad();
        void ActionVerifySpriteFill();
        void ActionVerifyBufferRoundTrip();
        void ActionVerifyBackBuffer();

        // --- XNA refusals (ChaosMisuse.cpp) ------------------------------------------------
        void ActionMisuseSpriteBatch();
        void ActionMisuseDeviceState();
        void ActionMisuseDraw();
        void ActionMisuseRenderTargets();
        void ActionMisuseTextureData();
        void ActionMisuseDisposed();
        void ActionMisuseOcclusion();
        void ActionMisuseAudio();
        void ActionMisuseGameTiming();

        // --- corrupted input (ChaosFuzz.cpp) -----------------------------------------------
        void ActionFuzzImage();
        void ActionFuzzWave();
        void ActionFuzzXnb();
        void ActionFuzzEffect();

        // --- audio (ChaosAudio.cpp) --------------------------------------------------------
        void ActionCreateSound();
        void ActionPlaySound();
        void ActionDestroySound();
        void ActionSoundInstances();
        void ActionDynamicSound();
        void ActionAudioGlobals();
        /** @brief Runs @p body; a NoAudioHardwareException turns the audio family off for the run. */
        void WithAudio(const std::function<void()>& body);

        // --- game loop, threads, input (ChaosGame.cpp) -------------------------------------
        void ActionChurnComponents();
        /** @brief Adds one ChaosComponent; safe to call from inside a component's Update. */
        void AddChaosComponent();
        void ActionChangeGameTiming();
        void ActionWorkerThreadResources();
        /** @brief Takes in a finished worker job (or waits for it with @p wait) without using the seeded stream. */
        void CollectWorkerJob(bool wait);
        void ActionPokeInput();
    };
}
