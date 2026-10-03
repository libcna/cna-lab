// SPDX-License-Identifier: MIT
// Game-loop chaos: components that rearrange the collection they are iterated from, timing
// changes, resources created and destroyed on worker threads, and input poking.
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <thread>

#include "System/ArgumentException.hpp"
#include "System/TimeSpan.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameComponentCollection.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

#include "ChaosComponent.hpp"
#include "ChaosSupport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace CnaKiller
{
    namespace
    {
        enum Operation
        {
            Nothing,
            ToggleEnabled,
            ChangeUpdateOrder,
            ChangeDrawOrder,
            ToggleVisible,
            RemoveSelf,
            RemoveAndDestroyOther,
            AddAnother,
        };
    }

    ChaosComponent::ChaosComponent(Game& game, ChaosEngine& engine, std::vector<int> script, const int id)
        : DrawableGameComponent(game)
        , engine_(engine)
        , script_(std::move(script))
        , id_(id)
    {
    }

    void ChaosComponent::Initialize()
    {
        ++initializeCalls_;
        if (initializeCalls_ == 2)
        {
            engine_.Report(FindingKind::Mismatch, "a GameComponent was initialized more than once",
                           "component #" + std::to_string(id_));
        }
        DrawableGameComponent::Initialize();
    }

    void ChaosComponent::Update(GameTime& gameTime)
    {
        DrawableGameComponent::Update(gameTime);
        if (pendingDestroy_ || script_.empty())
            return;

        const int operation = script_[updateStep_++ % script_.size()];
        GameComponentCollection& components = getGameProperty().getComponentsProperty();
        // Runs inside Game's component iteration: guarded like an action, and named as one.
        const std::string previous = engine_.lastActionName_;
        engine_.lastActionName_ = "ChaosComponent.Update";
        engine_.RunGuarded(engine_.lastActionName_, [&] { RunOperation(operation, components); });
        engine_.lastActionName_ = previous;
    }

    void ChaosComponent::RunOperation(const int operation, GameComponentCollection& components)
    {
        switch (operation)
        {
            case ToggleEnabled:
                setEnabledProperty(!getEnabledProperty());
                break;
            case ChangeUpdateOrder:
                setUpdateOrderProperty(engine_.RandomInt(-5, 6));
                break;
            case ChangeDrawOrder:
                setDrawOrderProperty(engine_.RandomInt(-5, 6));
                break;
            case ToggleVisible:
                setVisibleProperty(!getVisibleProperty());
                break;
            case RemoveSelf:
                (void)components.Remove(this);
                pendingDestroy_ = true;
                break;
            case RemoveAndDestroyOther:
            {
                // Destroyed on the spot, while Game may still hold it in this frame's snapshot:
                // the owner freeing what it removed is the ordinary C++ lifetime.
                std::vector<std::unique_ptr<ChaosComponent>>& owned = engine_.components_;
                std::vector<std::size_t> candidates;
                for (std::size_t i = 0; i < owned.size(); ++i)
                {
                    if (owned[i].get() != this && !owned[i]->pendingDestroy_)
                        candidates.push_back(i);
                }
                if (candidates.empty())
                    break;
                const std::size_t victim =
                    candidates[static_cast<std::size_t>(engine_.RandomInt(0, static_cast<int>(candidates.size())))];
                (void)components.Remove(owned[victim].get());
                owned.erase(owned.begin() + static_cast<std::ptrdiff_t>(victim));
                break;
            }
            case AddAnother:
                if (engine_.components_.size() < engine_.maxPoolSize_)
                    engine_.AddChaosComponent();
                break;
            default:
                break;
        }
    }

    void ChaosComponent::Draw(const GameTime& gameTime)
    {
        DrawableGameComponent::Draw(gameTime);
        if (script_.empty())
            return;
        const int operation = script_[drawStep_++ % script_.size()];
        if (operation == ChangeDrawOrder)
            setDrawOrderProperty(static_cast<int>(drawStep_ % 7) - 3);
    }

    void ChaosEngine::ActionChurnComponents()
    {
        GameComponentCollection& components = game_->getComponentsProperty();

        // Re-enable one component now and then, or a disabled component would sit out the run.
        if (!components_.empty() && Chance(3))
        {
            ChaosComponent& component = *components_[static_cast<std::size_t>(RandomInt(0, static_cast<int>(components_.size())))];
            component.setEnabledProperty(true);
            component.setVisibleProperty(true);
        }

        if (components_.size() >= maxPoolSize_ || (!components_.empty() && Chance(3)))
        {
            if (Chance(8))
            {
                for (const std::unique_ptr<ChaosComponent>& component : components_)
                    (void)components.Remove(component.get());
                components_.clear();
                return;
            }
            const auto victim = static_cast<std::size_t>(RandomInt(0, static_cast<int>(components_.size())));
            if (!components_[victim]->PendingDestroy())
            {
                (void)components.Remove(components_[victim].get());
                components_.erase(components_.begin() + static_cast<std::ptrdiff_t>(victim));
            }
            return;
        }

        AddChaosComponent();
    }

    void ChaosEngine::AddChaosComponent()
    {
        GameComponentCollection& components = game_->getComponentsProperty();
        std::vector<int> script(static_cast<std::size_t>(RandomInt(1, 9)));
        for (int& operation : script)
            operation = RandomInt(0, ChaosComponent::kOperationCount);
        static int nextId = 0;
        auto component = std::make_unique<ChaosComponent>(*game_, *this, std::move(script), ++nextId);
        component->setUpdateOrderProperty(RandomInt(-5, 6));
        component->setDrawOrderProperty(RandomInt(-5, 6));
        ChaosComponent* raw = component.get();
        components_.push_back(std::move(component));

        if (Chance(4) && components.getCountProperty() > 0)
            components.Insert(static_cast<GameComponentCollection::size_type>(
                                  RandomInt(0, static_cast<int>(components.getCountProperty()) + 1)),
                              raw);
        else
            components.Add(raw);

        // XNA initializes a component added to a running game inside Add.
        findings_.CountCheck();
        if (raw->InitializeCalls() != 1)
        {
            Report(FindingKind::Mismatch, "a GameComponent added to a running game was not initialized once by Add",
                   "Initialize ran " + std::to_string(raw->InitializeCalls()) + " times");
        }

        if (Chance(6))
            Expect<System::ArgumentException>("adding the same GameComponent twice",
                                              [&] { components.Add(raw); });
    }

    void ChaosEngine::ActionChangeGameTiming()
    {
        switch (RandomInt(0, 5))
        {
            case 0:
                game_->setTargetElapsedTimeProperty(System::TimeSpan::FromMilliseconds(RandomInt(1, 41)));
                break;
            case 1:
                game_->setIsFixedTimeStepProperty(!game_->getIsFixedTimeStepProperty());
                break;
            case 2:
                game_->ResetElapsedTime();
                break;
            case 3:
                game_->SuppressDraw();
                break;
            default:
                game_->setInactiveSleepTimeProperty(System::TimeSpan::FromMilliseconds(RandomInt(0, 30)));
                break;
        }
    }

    void ChaosEngine::ActionWorkerThreadResources()
    {
        // XNA 4.0 lets a game create resources and set their data on any thread; the idiomatic
        // form is the loading screen, where the game keeps running frames while a worker loads.
        // CNA gives a worker the device between frames, so the game thread must not wait for it
        // inside Update or Draw -- that would deadlock, as it does on FNA and MonoGame. The job
        // is decided here from the seeded stream, runs on its own, and is collected by
        // CollectWorkerJob() once it has finished. Whatever it creates lives in workerTextures_,
        // which the seeded stream never picks from, so when it finishes cannot change the run.
        const int kind = RandomInt(0, 5);
        const int width = RandomInt(1, 257);
        const int height = RandomInt(1, 257);
        const std::uint64_t jobSeed = static_cast<std::uint64_t>(static_cast<std::uint32_t>(random_.Next())) << 32 |
                                      static_cast<std::uint32_t>(random_.Next());
        if (workerJob_)
        {
            log_.Note("worker-thread job still running; this one is skipped");
            return;
        }

        static constexpr const char* kKinds[] = {"texture", "mesh", "render-target", "destroy-textures",
                                                 "two-workers"};
        auto job = std::make_unique<WorkerJob>();
        job->kind = kKinds[kind];
        job->width = width;
        job->height = height;
        log_.Note(std::string("worker-thread job started: ") + job->kind);

        std::uint64_t state = jobSeed;
        job->expected.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        for (Color& pixel : job->expected)
        {
            const std::uint64_t bits = SplitMixNext(state);
            pixel = Color(static_cast<int>(bits & 0xFF), static_cast<int>((bits >> 8) & 0xFF),
                          static_cast<int>((bits >> 16) & 0xFF), static_cast<int>((bits >> 24) & 0xFF));
        }

        WorkerJob* const raw = job.get();
        GraphicsDevice* const device = &Device();
        const int destroyCount = static_cast<int>(state % 4) + 1;
        job->thread = std::thread([raw, device, kind, width, height, destroyCount, this] {
            try
            {
                switch (kind)
                {
                    case 0: // create, fill and read back a texture
                    {
                        auto texture = std::make_unique<Texture2D>(*device, width, height);
                        texture->SetData(raw->expected.data(), static_cast<int>(raw->expected.size()));
                        raw->readBack.resize(raw->expected.size());
                        texture->GetData(raw->readBack.data(), static_cast<int>(raw->readBack.size()));
                        raw->textures.push_back(std::move(texture));
                        break;
                    }
                    case 1: // a mesh
                    {
                        const int triangles = 1 + width % 100;
                        std::vector<VertexPositionColor> vertices;
                        for (int i = 0; i < triangles * 3; ++i)
                        {
                            const Color& c = raw->expected[static_cast<std::size_t>(i) % raw->expected.size()];
                            vertices.emplace_back(Vector3(c.getRProperty() / 128.0f - 1, c.getGProperty() / 128.0f - 1, 0),
                                                  c);
                        }
                        std::vector<std::uint32_t> indices(vertices.size());
                        for (std::size_t i = 0; i < indices.size(); ++i)
                            indices[i] = static_cast<std::uint32_t>(i);
                        raw->mesh = std::make_unique<ManagedMesh>();
                        raw->mesh->primitiveCount = triangles;
                        raw->mesh->vertexBuffer = std::make_unique<VertexBuffer>(
                            *device, VertexPositionColor::getVertexDeclarationStatic(),
                            static_cast<int>(vertices.size()), BufferUsage::None);
                        raw->mesh->vertexBuffer->SetData(vertices.data(), static_cast<int>(vertices.size()));
                        raw->mesh->indexBuffer = std::make_unique<IndexBuffer>(
                            *device, IndexElementSize::ThirtyTwoBits, static_cast<int>(indices.size()),
                            BufferUsage::None);
                        raw->mesh->indexBuffer->SetData(indices.data(), static_cast<int>(indices.size()));
                        break;
                    }
                    case 2: // a render target, rendered to on the game thread when collected
                        raw->target = std::make_unique<RenderTarget2D>(*device, width, height);
                        break;
                    case 3: // destroy textures an earlier job created
                        for (int i = 0; i < destroyCount && !workerTextures_.Empty(); ++i)
                            workerTextures_.DestroyOldest();
                        break;
                    default: // two workers creating textures at the same moment
                    {
                        std::unique_ptr<Texture2D> second;
                        std::exception_ptr secondFailure;
                        std::thread other([&] {
                            try
                            {
                                second = std::make_unique<Texture2D>(*device, height, width);
                                second->SetData(raw->expected.data(), static_cast<int>(raw->expected.size()));
                            }
                            catch (...)
                            {
                                secondFailure = std::current_exception();
                            }
                        });
                        auto first = std::make_unique<Texture2D>(*device, width, height);
                        first->SetData(raw->expected.data(), static_cast<int>(raw->expected.size()));
                        other.join();
                        if (secondFailure)
                            std::rethrow_exception(secondFailure);
                        raw->textures.push_back(std::move(first));
                        raw->textures.push_back(std::move(second));
                        break;
                    }
                }
            }
            catch (...)
            {
                raw->failure = std::current_exception();
            }
            raw->done.store(true, std::memory_order_release);
        });
        workerJob_ = std::move(job);
    }

    void ChaosEngine::CollectWorkerJob(const bool wait)
    {
        if (!workerJob_ || (!wait && !workerJob_->done.load(std::memory_order_acquire)))
            return;
        workerJob_->thread.join();
        std::unique_ptr<WorkerJob> job = std::move(workerJob_);
        const std::string previous = lastActionName_;
        lastActionName_ = "WorkerThreadResources";
        RunGuarded(lastActionName_, [&] {
            if (job->failure)
                std::rethrow_exception(job->failure);
            if (job->kind == std::string("texture"))
            {
                findings_.CountCheck();
                for (std::size_t i = 0; i < job->expected.size(); ++i)
                {
                    if (job->readBack[i].getPackedValueProperty() != job->expected[i].getPackedValueProperty())
                    {
                        Report(FindingKind::Mismatch, "a texture filled on a worker thread reads back different data",
                               std::to_string(job->width) + "x" + std::to_string(job->height) +
                                   ", first difference at texel " + std::to_string(i));
                        break;
                    }
                }
            }
            if (job->mesh)
            {
                // Drawn on the game thread from buffers another thread created.
                GraphicsDevice& device = Device();
                BasicEffect effect(device);
                effect.VertexColorEnabled = true;
                device.SetVertexBuffer(job->mesh->vertexBuffer.get());
                device.SetIndexBuffer(job->mesh->indexBuffer.get());
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, job->mesh->primitiveCount * 3, 0,
                                             job->mesh->primitiveCount);
                Support::UnbindAll(device);
            }
            if (job->target)
            {
                GraphicsDevice& device = Device();
                device.SetRenderTarget(job->target.get());
                device.Clear(Color::Magenta);
                device.SetRenderTarget(nullptr);
                std::vector<Color> pixels(static_cast<std::size_t>(job->width) * static_cast<std::size_t>(job->height));
                job->target->GetData(pixels.data(), static_cast<int>(pixels.size()));
                findings_.CountCheck();
                if (pixels.front() != Color::Magenta || pixels.back() != Color::Magenta)
                {
                    Report(FindingKind::Mismatch,
                           "a render target created on a worker thread does not hold what the game thread cleared it to",
                           std::to_string(job->width) + "x" + std::to_string(job->height));
                }
            }
            for (std::unique_ptr<Texture2D>& texture : job->textures)
            {
                if (workerTextures_.Size() >= maxPoolSize_)
                    workerTextures_.DestroyOldest();
                workerTextures_.Add(std::move(texture));
            }
        });
        lastActionName_ = previous;
    }

    void ChaosEngine::ActionPokeInput()
    {
        switch (RandomInt(0, 4))
        {
            case 0:
                // Includes positions outside the window and far outside any screen.
            {
                const int x = RandomInt(-5000, 5000);
                const int y = RandomInt(-5000, 5000);
                Mouse::SetPosition(x, y);
                (void)Mouse::GetState();
                break;
            }
            case 1:
                (void)Keyboard::GetState();
                (void)Keyboard::GetState(static_cast<PlayerIndex>(RandomInt(0, 4)));
                break;
            case 2:
            {
                const auto player = static_cast<PlayerIndex>(RandomInt(0, 4));
                (void)GamePad::GetState(player);
                (void)GamePad::GetCapabilities(player);
                break;
            }
            default:
            {
                const auto player = static_cast<PlayerIndex>(RandomInt(0, 4));
                const float left = RandomFloat(-2.0f, 3.0f);
                const float right = RandomFloat(-2.0f, 3.0f);
                (void)GamePad::SetVibration(player, left, right);
                break;
            }
        }
    }
}
