// SPDX-License-Identifier: MIT
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <exception>
#include <fstream>
#include <limits>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include <unistd.h>

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

#include "ChaosComponent.hpp"
#include "ChaosSupport.hpp"

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

        std::vector<std::string> SplitList(const std::string& text)
        {
            std::vector<std::string> items;
            std::stringstream stream(text);
            std::string item;
            while (std::getline(stream, item, ','))
            {
                if (!item.empty())
                    items.push_back(item);
            }
            return items;
        }

        /** @brief splitmix64: bulk bytes from one draw of the seeded stream, cheap and deterministic. */
        std::uint64_t SplitMix(std::uint64_t& state)
        {
            std::uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        }
    }

    // -----------------------------------------------------------------------------------------
    // The action table
    // -----------------------------------------------------------------------------------------

    const std::vector<ChaosEngine::ActionDef>& ChaosEngine::Actions()
    {
        static const std::vector<ActionDef> table{
            // Resource churn: the steady background every other action runs against.
            {"CreateTexture",              "resource", 10, false, &ChaosEngine::ActionCreateTexture},
            {"DestroyTexture",             "resource",  6, false, &ChaosEngine::ActionDestroyTexture},
            {"CopyTexture",                "resource",  2, false, &ChaosEngine::ActionCopyTexture},
            {"CreateRenderTarget",         "resource",  6, false, &ChaosEngine::ActionCreateRenderTarget},
            {"DestroyRenderTarget",        "resource",  4, false, &ChaosEngine::ActionDestroyRenderTarget},
            {"CreateMesh",                 "resource",  8, false, &ChaosEngine::ActionCreateMesh},
            {"DestroyMesh",                "resource",  5, false, &ChaosEngine::ActionDestroyMesh},
            {"ReloadShader",               "resource",  5, false, &ChaosEngine::ActionReloadShader},
            {"CubeAndVolumeTextures",      "resource",  2, false, &ChaosEngine::ActionCubeAndVolumeTextures},
            // Drawing outside Draw(), with every primitive type, effect and sprite mode.
            {"RenderToTarget",             "render",    8, false, &ChaosEngine::ActionRenderToTarget},
            {"DrawMesh",                   "render",    8, false, &ChaosEngine::ActionDrawMesh},
            {"SpamRenderState",            "render",    6, false, &ChaosEngine::ActionSpamRenderState},
            {"DrawPrimitiveTypes",         "render",    5, false, &ChaosEngine::ActionDrawPrimitiveTypes},
            {"DrawStockEffects",           "render",    5, false, &ChaosEngine::ActionDrawStockEffects},
            {"SpriteBatchChaos",           "render",    5, false, &ChaosEngine::ActionSpriteBatchChaos},
            {"OcclusionQueries",           "render",    2, false, &ChaosEngine::ActionOcclusionQueries},
            {"DrawInstanced",              "render",    2, false, &ChaosEngine::ActionDrawInstanced},
            // Read back what was written or drawn and compare it with XNA's result.
            {"VerifyTextureRoundTrip",     "verify",    5, false, &ChaosEngine::ActionVerifyTextureRoundTrip},
            {"VerifyRenderTargetClear",    "verify",    4, false, &ChaosEngine::ActionVerifyRenderTargetClear},
            {"VerifyMultipleRenderTargets","verify",    2, false, &ChaosEngine::ActionVerifyMultipleRenderTargets},
            {"VerifyCubeRenderTarget",     "verify",    2, false, &ChaosEngine::ActionVerifyCubeRenderTarget},
            {"VerifySolidQuad",            "verify",    4, false, &ChaosEngine::ActionVerifySolidQuad},
            {"VerifySpriteFill",           "verify",    3, false, &ChaosEngine::ActionVerifySpriteFill},
            {"VerifyBufferRoundTrip",      "verify",    4, false, &ChaosEngine::ActionVerifyBufferRoundTrip},
            {"VerifyBackBuffer",           "verify",    2, false, &ChaosEngine::ActionVerifyBackBuffer},
            // Call the API the way XNA refuses, and expect XNA's exception.
            {"MisuseSpriteBatch",          "misuse",    2, false, &ChaosEngine::ActionMisuseSpriteBatch},
            {"MisuseDeviceState",          "misuse",    3, false, &ChaosEngine::ActionMisuseDeviceState},
            {"MisuseDraw",                 "misuse",    2, false, &ChaosEngine::ActionMisuseDraw},
            {"MisuseRenderTargets",        "misuse",    2, false, &ChaosEngine::ActionMisuseRenderTargets},
            {"MisuseTextureData",          "misuse",    2, false, &ChaosEngine::ActionMisuseTextureData},
            {"MisuseDisposed",             "misuse",    2, false, &ChaosEngine::ActionMisuseDisposed},
            {"MisuseOcclusion",            "misuse",    1, false, &ChaosEngine::ActionMisuseOcclusion},
            {"MisuseAudio",                "misuse",    2, false, &ChaosEngine::ActionMisuseAudio},
            {"MisuseGameTiming",           "misuse",    1, false, &ChaosEngine::ActionMisuseGameTiming},
            // Corrupted input for every decoder a game can hand bytes to.
            {"FuzzImage",                  "fuzz",      3, false, &ChaosEngine::ActionFuzzImage},
            {"FuzzWave",                   "fuzz",      2, false, &ChaosEngine::ActionFuzzWave},
            {"FuzzXnb",                    "fuzz",      3, false, &ChaosEngine::ActionFuzzXnb},
            {"FuzzEffect",                 "fuzz",      1, false, &ChaosEngine::ActionFuzzEffect},
            // Audio.
            {"CreateSound",                "audio",     4, false, &ChaosEngine::ActionCreateSound},
            {"PlaySound",                  "audio",     6, false, &ChaosEngine::ActionPlaySound},
            {"DestroySound",               "audio",     3, false, &ChaosEngine::ActionDestroySound},
            {"SoundInstances",             "audio",     4, false, &ChaosEngine::ActionSoundInstances},
            {"DynamicSound",               "audio",     3, false, &ChaosEngine::ActionDynamicSound},
            {"AudioGlobals",               "audio",     1, false, &ChaosEngine::ActionAudioGlobals},
            // Window chaos.
            {"ResizeBackBuffer",           "window",    2, true,  &ChaosEngine::ActionResizeBackBuffer},
            {"ToggleFullScreen",           "window",    1, true,  &ChaosEngine::ActionToggleFullScreen},
            {"ToggleBorderless",           "window",    1, true,  &ChaosEngine::ActionToggleBorderless},
            {"ChangeTitle",                "window",    3, false, &ChaosEngine::ActionChangeTitle},
            {"MinimizeRestore",            "window",    1, true,  &ChaosEngine::ActionMinimizeRestore},
            {"ToggleMouseVisible",         "window",    2, false, &ChaosEngine::ActionToggleMouseVisible},
            {"AllowUserResizing",          "window",    1, false, &ChaosEngine::ActionAllowUserResizing},
            // Device loss and reconfiguration.
            {"ResetDevice",                "device",    1, true,  &ChaosEngine::ActionResetDevice},
            {"ChangeGraphicsSettings",     "device",    1, true,  &ChaosEngine::ActionChangeGraphicsSettings},
            // The game loop itself.
            {"ChurnComponents",            "loop",      3, false, &ChaosEngine::ActionChurnComponents},
            {"ChangeGameTiming",           "loop",      1, false, &ChaosEngine::ActionChangeGameTiming},
            {"WorkerThreadResources",      "thread",    2, false, &ChaosEngine::ActionWorkerThreadResources},
            {"PokeInput",                  "input",     2, false, &ChaosEngine::ActionPokeInput},
        };
        return table;
    }

    void ChaosEngine::ListActions(std::ostream& out)
    {
        for (const ActionDef& action : Actions())
        {
            out << action.family << '\t' << action.name << "\tweight=" << action.weight
                << (action.disruptive ? " (scales with intensity)" : "") << '\n';
        }
    }

    bool ChaosEngine::ValidateFilter(const CliOptions& options, std::string& error)
    {
        std::set<std::string> known;
        for (const ActionDef& action : Actions())
        {
            known.insert(action.name);
            known.insert(action.family);
        }
        for (const std::string& list : {options.onlyActions, options.excludedActions})
        {
            for (const std::string& item : SplitList(list))
            {
                if (known.count(item) == 0)
                {
                    error = "unknown action or family '" + item + "' (see --list-actions)";
                    return false;
                }
            }
        }
        return true;
    }

    ChaosEngine::ChaosEngine(CliOptions options, ChaosLog& log, Findings& findings)
        : options_(std::move(options))
        , log_(log)
        , findings_(findings)
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

        const std::vector<std::string> only = SplitList(options_.onlyActions);
        const std::vector<std::string> excluded = SplitList(options_.excludedActions);
        const auto matches = [](const std::vector<std::string>& list, const ActionDef& action) {
            return std::any_of(list.begin(), list.end(), [&](const std::string& item) {
                return item == action.name || item == action.family;
            });
        };
        for (const ActionDef& action : Actions())
        {
            if (!only.empty() && !matches(only, action))
                continue;
            if (matches(excluded, action))
                continue;
            enabled_.push_back(&action);
        }
    }

    ChaosEngine::~ChaosEngine()
    {
        if (workerJob_ && workerJob_->thread.joinable())
            workerJob_->thread.join();
        // The game's component collection outlives this engine; it must not keep pointers to
        // the components destroyed with it.
        if (game_ != nullptr)
        {
            for (const std::unique_ptr<ChaosComponent>& component : components_)
                (void)game_->getComponentsProperty().Remove(component.get());
        }
    }

    void ChaosEngine::Bind(Game& game, GraphicsDeviceManager& graphicsManager)
    {
        game_ = &game;
        graphicsManager_ = &graphicsManager;
        Device().DeviceResetting += [this](System::Object*, const System::EventArgs&) { ++deviceResetting_; };
        Device().DeviceReset += [this](System::Object*, const System::EventArgs&) { ++deviceReset_; };
        log_.Note(std::to_string(enabled_.size()) + " of " + std::to_string(Actions().size()) +
                  " actions enabled");
    }

    GraphicsDevice& ChaosEngine::Device() const
    {
        return *graphicsManager_->getGraphicsDeviceProperty();
    }

    GameWindow& ChaosEngine::Window() const
    {
        return game_->getWindowProperty();
    }

    const ChaosEngine::ActionDef& ChaosEngine::PickAction()
    {
        // Base weights favor steady resource churn; the "global disruption" actions
        // (resize/fullscreen/reset/window-state) get scaled up at higher intensities since those
        // are the ones most likely to actually break a real application.
        const int disruptionScale = 1 + static_cast<int>(options_.intensity); // Low=1 .. Nightmare=4
        const auto weightOf = [&](const ActionDef* action) {
            return action->disruptive ? action->weight * disruptionScale : action->weight;
        };

        int total = 0;
        for (const ActionDef* action : enabled_)
            total += weightOf(action);

        int roll = random_.Next(0, total);
        for (const ActionDef* action : enabled_)
        {
            if (roll < weightOf(action))
                return *action;
            roll -= weightOf(action);
        }
        return *enabled_.back();
    }

    bool ChaosEngine::Tick(std::uint64_t tick, double totalSeconds)
    {
        tick_ = tick;
        findings_.SetTick(tick);

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
        if (enabled_.empty())
        {
            log_.Note("no action is enabled; exiting");
            return false;
        }

        // Components that removed themselves last tick are destroyed only now, outside the
        // component iteration they were part of.
        std::erase_if(components_, [](const std::unique_ptr<ChaosComponent>& component) {
            return component->PendingDestroy();
        });

        CollectWorkerJob(false);

        if (tick % 500 == 0)
            SampleResidentMemory();

        const int actionCount = (minActionsPerTick_ == maxActionsPerTick_)
            ? minActionsPerTick_
            : random_.Next(minActionsPerTick_, maxActionsPerTick_ + 1);

        for (int i = 0; i < actionCount; ++i)
        {
            const ActionDef& action = PickAction();
            lastActionName_ = action.name;
            log_.BeginAction(tick, lastActionName_, StatsSummary(tick));
            RunGuarded(lastActionName_, [&] { (this->*action.run)(); });
        }
        return true;
    }

    void ChaosEngine::RunGuarded(const std::string& name, const std::function<void()>& body)
    {
        try
        {
            body();
            return;
        }
        catch (const StrictStop&)
        {
            throw;
        }
        catch (const std::exception& exception)
        {
            findings_.Report(FindingKind::UnexpectedException, name, ExceptionTypeName(exception),
                             exception.what());
        }

        // Whatever the failed action left bound must not turn every later Present() into a
        // second, unrelated finding.
        try
        {
            Device().SetRenderTarget(nullptr);
        }
        catch (const std::exception&)
        {
        }
    }

    void ChaosEngine::Report(FindingKind kind, const std::string& what, const std::string& detail)
    {
        findings_.Report(kind, lastActionName_, what, detail);
    }

    // -----------------------------------------------------------------------------------------
    // Shared helpers
    // -----------------------------------------------------------------------------------------

    int ChaosEngine::RandomInt(int minInclusive, int maxExclusive)
    {
        return random_.Next(minInclusive, maxExclusive);
    }

    bool ChaosEngine::Chance(int oneIn)
    {
        return random_.Next(0, oneIn) == 0;
    }

    float ChaosEngine::RandomFloat(float minValue, float maxValue)
    {
        return minValue + static_cast<float>(random_.NextDouble()) * (maxValue - minValue);
    }

    std::vector<std::uint8_t> ChaosEngine::RandomBytes(std::size_t count)
    {
        std::uint64_t state = static_cast<std::uint64_t>(static_cast<std::uint32_t>(random_.Next())) << 32 |
                              static_cast<std::uint32_t>(random_.Next());
        std::vector<std::uint8_t> bytes(count);
        for (std::size_t i = 0; i < count; i += 8)
        {
            const std::uint64_t word = SplitMix(state);
            std::memcpy(bytes.data() + i, &word, std::min<std::size_t>(8, count - i));
        }
        return bytes;
    }

    Color ChaosEngine::RandomOpaqueColor()
    {
        return RandomColor(random_);
    }

    Color ChaosEngine::RandomAnyColor()
    {
        return RandomColor(random_, /*randomAlpha=*/true);
    }

    void ChaosEngine::Mutate(std::vector<std::uint8_t>& bytes)
    {
        switch (random_.Next(0, 6))
        {
            case 0: // flip a few bits
            {
                const int flips = random_.Next(1, 9);
                for (int i = 0; i < flips && !bytes.empty(); ++i)
                    bytes[static_cast<std::size_t>(random_.Next(0, static_cast<int>(bytes.size())))] ^=
                        static_cast<std::uint8_t>(1 << random_.Next(0, 8));
                break;
            }
            case 1: // overwrite a run with a boundary value
            {
                if (bytes.empty())
                    break;
                static constexpr std::uint8_t kValues[] = {0x00, 0xFF, 0x7F, 0x80, 0x01, 0xFE};
                const auto start = static_cast<std::size_t>(random_.Next(0, static_cast<int>(bytes.size())));
                const auto length = std::min<std::size_t>(bytes.size() - start,
                                                          static_cast<std::size_t>(random_.Next(1, 9)));
                const std::uint8_t value = kValues[random_.Next(0, 6)];
                std::fill_n(bytes.begin() + static_cast<std::ptrdiff_t>(start), length, value);
                break;
            }
            case 2: // truncate
                bytes.resize(static_cast<std::size_t>(random_.Next(0, static_cast<int>(bytes.size()) + 1)));
                break;
            case 3: // append garbage
            {
                const std::vector<std::uint8_t> tail = RandomBytes(static_cast<std::size_t>(random_.Next(1, 256)));
                bytes.insert(bytes.end(), tail.begin(), tail.end());
                break;
            }
            case 4: // corrupt a 32-bit little-endian length-looking field inside the first 64 bytes
            {
                if (bytes.size() < 8)
                    break;
                const auto at = static_cast<std::size_t>(
                    random_.Next(0, static_cast<int>(std::min<std::size_t>(bytes.size(), 64) - 4)));
                static constexpr std::uint32_t kLengths[] = {0u, 1u, 0x7FFFFFFFu, 0x80000000u,
                                                            0xFFFFFFFFu, 0x00010000u};
                const std::uint32_t value = kLengths[random_.Next(0, 6)];
                std::memcpy(bytes.data() + at, &value, 4);
                break;
            }
            default: // shuffle two chunks
            {
                if (bytes.size() < 16)
                    break;
                const auto length = static_cast<std::size_t>(random_.Next(1, static_cast<int>(bytes.size() / 4)));
                const auto a = static_cast<std::size_t>(random_.Next(0, static_cast<int>(bytes.size() - length)));
                const auto b = static_cast<std::size_t>(random_.Next(0, static_cast<int>(bytes.size() - length)));
                std::vector<std::uint8_t> chunk(bytes.begin() + static_cast<std::ptrdiff_t>(a),
                                                bytes.begin() + static_cast<std::ptrdiff_t>(a + length));
                std::copy(chunk.begin(), chunk.end(), bytes.begin() + static_cast<std::ptrdiff_t>(b));
                break;
            }
        }
    }

    Texture2D& ChaosEngine::WhiteTexture()
    {
        if (!white_)
        {
            white_ = std::make_unique<Texture2D>(Device(), 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
        }
        return *white_;
    }

    void ChaosEngine::QueueDrawCheck(const std::string& name, std::function<void()> check)
    {
        drawQueue_.emplace_back(name, std::move(check));
    }

    void ChaosEngine::SampleResidentMemory()
    {
        std::ifstream statm("/proc/self/statm");
        long pages = 0;
        long resident = 0;
        if (statm >> pages >> resident)
        {
            const long kib = resident * (sysconf(_SC_PAGESIZE) / 1024);
            rssSamples_.emplace_back(tick_, kib);
            log_.Note("rss tick=" + std::to_string(tick_) + " kib=" + std::to_string(kib));
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

    void ChaosEngine::ActionCopyTexture()
    {
        // CNA's Texture2D copy is a second handle to one texture, copy-on-write for a full-level
        // SetData (REMED-GFX-223): the copy joins the pool, so the original may be destroyed first
        // and the copy drawn afterwards, and a full-level upload through the original must leave
        // the copy's pixels as they were.
        if (textures_.Empty())
            return;
        EvictIfFull(textures_);
        Texture2D& original = textures_.RandomItem(random_);
        auto copy = std::make_unique<Texture2D>(original);

        if (original.getFormatProperty() == SurfaceFormat::Color && original.getWidthProperty() <= 256 &&
            original.getHeightProperty() <= 256)
        {
            const int count = original.getWidthProperty() * original.getHeightProperty();
            Support::UnbindAll(Device());
            std::vector<Color> before(static_cast<std::size_t>(count));
            copy->GetData(before.data(), count);
            const std::vector<Color> written = MakeNoiseTexture(random_, original.getWidthProperty(),
                                                                original.getHeightProperty());
            original.SetData(written.data(), count);
            std::vector<Color> after(static_cast<std::size_t>(count));
            copy->GetData(after.data(), count);
            findings_.CountCheck();
            for (int i = 0; i < count; ++i)
            {
                if (after[static_cast<std::size_t>(i)].getPackedValueProperty() !=
                    before[static_cast<std::size_t>(i)].getPackedValueProperty())
                {
                    Report(FindingKind::Mismatch,
                           "a full-level SetData through one Texture2D handle changed a copy's pixels",
                           std::to_string(original.getWidthProperty()) + "x" +
                               std::to_string(original.getHeightProperty()) + ", first difference at texel " +
                               std::to_string(i));
                    break;
                }
            }
        }
        textures_.Add(std::move(copy));
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
        const Color color = RandomColor(random_);
        device.SetRenderTarget(&target);
        device.Clear(color);
        device.SetRenderTarget(nullptr);

        // Nothing earlier in this tick has necessarily touched the device, so this is also the
        // check that a bind and clear issued from Update() reach the target at all.
        if (target.getWidthProperty() * target.getHeightProperty() <= 256 * 256)
        {
            std::vector<Color> pixels(static_cast<std::size_t>(target.getWidthProperty()) *
                                      static_cast<std::size_t>(target.getHeightProperty()));
            target.GetData(pixels.data(), static_cast<int>(pixels.size()));
            findings_.CountCheck();
            const auto wrong = std::find_if(pixels.begin(), pixels.end(), [&](const Color& p) {
                return p.getPackedValueProperty() != color.getPackedValueProperty();
            });
            if (wrong != pixels.end())
            {
                Report(FindingKind::Mismatch, "a render target cleared from Update() does not hold the colour",
                       std::to_string(target.getWidthProperty()) + "x" + std::to_string(target.getHeightProperty()) +
                           " target with " + std::to_string(target.getMultiSampleCountProperty()) +
                           " samples, texel " + std::to_string(wrong - pixels.begin()) + " reads " +
                           std::to_string(wrong->getPackedValueProperty()) + " for " +
                           std::to_string(color.getPackedValueProperty()));
            }
        }
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

        // Windowed, XNA gives the back buffer exactly the preferred size; fullscreen may pick a
        // display mode instead.
        if (!graphicsManager_->getIsFullScreenProperty())
        {
            const PresentationParameters& pp = Device().getPresentationParametersProperty();
            findings_.CountCheck();
            if (pp.getBackBufferWidthProperty() != width || pp.getBackBufferHeightProperty() != height)
            {
                Report(FindingKind::Mismatch, "ApplyChanges in a window did not give the preferred back buffer size",
                       "preferred " + std::to_string(width) + "x" + std::to_string(height) + ", got " +
                           std::to_string(pp.getBackBufferWidthProperty()) + "x" +
                           std::to_string(pp.getBackBufferHeightProperty()));
            }
        }
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
        switch (random_.Next(0, 6))
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
            case 3:
            {
                // Multi-byte UTF-8: Czech, CJK, emoji and a right-to-left run.
                static constexpr const char* kWords[] = {"\xC5\x99\xC3\xAD\xC5\xA1\x65", "\xE6\xBC\xA2\xE5\xAD\x97",
                                                         "\xF0\x9F\x92\xA3", "\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D"};
                const int length = random_.Next(1, 40);
                for (int i = 0; i < length; ++i)
                    title << kWords[random_.Next(0, 4)];
                break;
            }
            case 4:
            {
                // Bytes that are not UTF-8 at all, including an embedded NUL.
                const int length = random_.Next(1, 64);
                for (int i = 0; i < length; ++i)
                    title << static_cast<char>(random_.Next(0, 256));
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

    void ChaosEngine::ActionAllowUserResizing()
    {
        Window().setAllowUserResizingProperty(!Window().getAllowUserResizingProperty());
    }

    void ChaosEngine::ActionResetDevice()
    {
        GraphicsDevice& device = Device();
        const int resettingBefore = deviceResetting_;
        const int resetBefore = deviceReset_;
        if (random_.Next(0, 2) == 0)
        {
            device.Reset();
        }
        else
        {
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

        // XNA's Reset raises DeviceResetting and DeviceReset exactly once each.
        findings_.CountCheck();
        if (deviceResetting_ - resettingBefore != 1 || deviceReset_ - resetBefore != 1)
        {
            Report(FindingKind::Mismatch, "GraphicsDevice::Reset did not raise DeviceResetting and DeviceReset once each",
                   "DeviceResetting x" + std::to_string(deviceResetting_ - resettingBefore) +
                       ", DeviceReset x" + std::to_string(deviceReset_ - resetBefore));
        }
    }

    void ChaosEngine::ActionChangeGraphicsSettings()
    {
        static constexpr std::array<SurfaceFormat, 5> kBackBufferFormats{
            SurfaceFormat::Color, SurfaceFormat::Bgr565, SurfaceFormat::Bgra5551, SurfaceFormat::Bgra4444,
            SurfaceFormat::Rgba1010102};
        static constexpr std::array<DepthFormat, 4> kDepthFormats{
            DepthFormat::None, DepthFormat::Depth16, DepthFormat::Depth24, DepthFormat::Depth24Stencil8};

        graphicsManager_->setPreferMultiSamplingProperty(Chance(2));
        graphicsManager_->setSynchronizeWithVerticalRetraceProperty(Chance(2));
        graphicsManager_->setPreferredBackBufferFormatProperty(
            kBackBufferFormats[static_cast<std::size_t>(RandomInt(0, 5))]);
        graphicsManager_->setPreferredDepthStencilFormatProperty(
            kDepthFormats[static_cast<std::size_t>(RandomInt(0, 4))]);
        graphicsManager_->ApplyChanges();
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
        // Draw-phase checks first, against a freshly cleared back buffer.
        std::vector<std::pair<std::string, std::function<void()>>> queued;
        queued.swap(drawQueue_);
        for (auto& [name, check] : queued)
        {
            const std::string previous = lastActionName_;
            lastActionName_ = name;
            RunGuarded(name, check);
            lastActionName_ = previous;
        }

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

    void ChaosEngine::Shutdown()
    {
        // Called outside the frame, so a job still waiting for the device can finish.
        CollectWorkerJob(true);
        SampleResidentMemory();

        // A pool that is bounded while memory still climbs steadily is a leak somewhere below
        // the public API. Compare the second quarter of the run with the last, so start-up
        // allocation and the first pool fill are not counted.
        if (rssSamples_.size() >= 8)
        {
            const std::size_t quarter = rssSamples_.size() / 4;
            const auto mean = [&](std::size_t from, std::size_t to) {
                long sum = 0;
                for (std::size_t i = from; i < to; ++i)
                    sum += rssSamples_[i].second;
                return sum / static_cast<long>(to - from);
            };
            const long early = mean(quarter, 2 * quarter);
            const long late = mean(rssSamples_.size() - quarter, rssSamples_.size());
            if (late > early + early / 2 && late - early > 200 * 1024)
            {
                findings_.Report(FindingKind::Leak, "Shutdown",
                                 "resident memory kept growing while every pool stayed bounded",
                                 "mean RSS " + std::to_string(early / 1024) + " MiB in the second quarter, " +
                                     std::to_string(late / 1024) + " MiB in the last");
            }
        }

        if (game_ != nullptr)
        {
            for (const std::unique_ptr<ChaosComponent>& component : components_)
                (void)game_->getComponentsProperty().Remove(component.get());
        }
        components_.clear();
        drawQueue_.clear();
        workerTextures_.Clear();
        instances_.Clear();
        dynamicSounds_.Clear();
        sounds_.Clear();
        effects_.Clear();
        meshes_.Clear();
        renderTargets_.Clear();
        textures_.Clear();
        white_.reset();
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
            << " inst=" << instances_.Size()
            << " dyn=" << dynamicSounds_.Size()
            << " fx=" << effects_.Size()
            << " comp=" << components_.size()
            << " findings=" << findings_.Distinct()
            << " last=" << lastActionName_;
        return out.str();
    }
}
