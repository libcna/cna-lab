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
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

#include "ChaosComponent.hpp"

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
        // XNA 4.0 lets a game create resources and set their data from any thread -- the
        // loading-screen pattern. Every decision is made here, on the main thread, before the
        // worker starts, and the worker is joined before this action returns, so the action
        // sequence stays reproducible even though the device is touched from another thread.
        const int kind = RandomInt(0, 5);
        const int width = RandomInt(1, 257);
        const int height = RandomInt(1, 257);
        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        const std::vector<std::uint8_t> noise = RandomBytes(pixels.size() * 4);
        for (std::size_t i = 0; i < pixels.size(); ++i)
            pixels[i] = Color(noise[i * 4], noise[i * 4 + 1], noise[i * 4 + 2], noise[i * 4 + 3]);

        switch (kind)
        {
            case 0: // create, fill and verify a texture off the main thread; draw it on the main one
            {
                std::unique_ptr<Texture2D> texture;
                std::vector<Color> readBack(pixels.size());
                RunOnWorkerThread([&] {
                    texture = std::make_unique<Texture2D>(Device(), width, height);
                    texture->SetData(pixels.data(), static_cast<int>(pixels.size()));
                    texture->GetData(readBack.data(), static_cast<int>(readBack.size()));
                });
                findings_.CountCheck();
                for (std::size_t i = 0; i < pixels.size(); ++i)
                {
                    if (readBack[i].getPackedValueProperty() != pixels[i].getPackedValueProperty())
                    {
                        Report(FindingKind::Mismatch, "a texture filled on a worker thread reads back different data",
                               std::to_string(width) + "x" + std::to_string(height) + ", first difference at texel " +
                                   std::to_string(i));
                        break;
                    }
                }
                EvictIfFull(textures_);
                textures_.Add(std::move(texture));
                break;
            }
            case 1: // a mesh built on a worker thread
            {
                const int triangles = RandomInt(1, 100);
                std::vector<VertexPositionColor> vertices;
                for (int i = 0; i < triangles * 3; ++i)
                {
                    const Vector3 position{RandomFloat(-1, 1), RandomFloat(-1, 1), 0.0f};
                    vertices.emplace_back(position, RandomOpaqueColor());
                }
                std::vector<std::uint32_t> indices(vertices.size());
                for (std::size_t i = 0; i < indices.size(); ++i)
                    indices[i] = static_cast<std::uint32_t>(i);
                auto mesh = std::make_unique<ManagedMesh>();
                mesh->primitiveCount = triangles;
                RunOnWorkerThread([&] {
                    mesh->vertexBuffer = std::make_unique<VertexBuffer>(
                        Device(), VertexPositionColor::getVertexDeclarationStatic(),
                        static_cast<int>(vertices.size()), BufferUsage::None);
                    mesh->vertexBuffer->SetData(vertices.data(), static_cast<int>(vertices.size()));
                    mesh->indexBuffer = std::make_unique<IndexBuffer>(
                        Device(), IndexElementSize::ThirtyTwoBits, static_cast<int>(indices.size()), BufferUsage::None);
                    mesh->indexBuffer->SetData(indices.data(), static_cast<int>(indices.size()));
                });
                EvictIfFull(meshes_);
                meshes_.Add(std::move(mesh));
                ActionDrawMesh();
                break;
            }
            case 2: // a render target created on a worker thread, then rendered to on the main one
            {
                std::unique_ptr<RenderTarget2D> target;
                RunOnWorkerThread([&] { target = std::make_unique<RenderTarget2D>(Device(), width, height); });
                Device().SetRenderTarget(target.get());
                Device().Clear(RandomOpaqueColor());
                Device().SetRenderTarget(nullptr);
                EvictIfFull(renderTargets_);
                renderTargets_.Add(std::move(target));
                break;
            }
            case 3: // destroy pooled textures from a worker thread
            {
                const int count = RandomInt(1, 4);
                std::vector<int> picks;
                for (int i = 0; i < count; ++i)
                    picks.push_back(RandomInt(0, 1 << 16));
                RunOnWorkerThread([&] {
                    for (const int pick : picks)
                    {
                        if (textures_.Empty())
                            break;
                        textures_.DestroyAt(static_cast<std::size_t>(pick) % textures_.Size());
                    }
                });
                break;
            }
            default: // two workers creating textures at the same moment
            {
                std::unique_ptr<Texture2D> first;
                std::unique_ptr<Texture2D> second;
                std::exception_ptr failure;
                std::thread other([&] {
                    try
                    {
                        second = std::make_unique<Texture2D>(Device(), height, width);
                        second->SetData(pixels.data(), static_cast<int>(pixels.size()));
                    }
                    catch (...)
                    {
                        failure = std::current_exception();
                    }
                });
                try
                {
                    RunOnWorkerThread([&] {
                        first = std::make_unique<Texture2D>(Device(), width, height);
                        first->SetData(pixels.data(), static_cast<int>(pixels.size()));
                    });
                }
                catch (...)
                {
                    other.join();
                    throw;
                }
                other.join();
                if (failure)
                    std::rethrow_exception(failure);
                EvictIfFull(textures_);
                textures_.Add(std::move(first));
                EvictIfFull(textures_);
                textures_.Add(std::move(second));
                break;
            }
        }
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
