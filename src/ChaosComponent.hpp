// SPDX-License-Identifier: MIT
#pragma once

#include <vector>

#include "Microsoft/Xna/Framework/DrawableGameComponent.hpp"
#include "Microsoft/Xna/Framework/GameComponentCollection.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CnaKiller
{
    class ChaosEngine;

    /**
     * @brief A DrawableGameComponent that rearranges the component collection it is iterated from.
     *
     * Its Update() runs one step of a script drawn from the seeded stream when it was created:
     * toggle Enabled or Visible, change UpdateOrder or DrawOrder, remove itself, remove and
     * destroy another component, or add a new one -- all while Game is walking the collection.
     * Draw() only touches DrawOrder and Visible: Draw runs a machine-dependent number of times,
     * so nothing it does may change what later Updates see.
     *
     * It also checks XNA's rule that a component added to a running game is initialized exactly
     * once.
     */
    class ChaosComponent final : public Microsoft::Xna::Framework::DrawableGameComponent
    {
    public:
        ChaosComponent(Microsoft::Xna::Framework::Game& game, ChaosEngine& engine, std::vector<int> script,
                       int id);

        void Initialize() override;
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        /** @brief True once the component removed itself; the engine destroys it next tick. */
        [[nodiscard]] bool PendingDestroy() const { return pendingDestroy_; }
        [[nodiscard]] int InitializeCalls() const { return initializeCalls_; }
        [[nodiscard]] int Id() const { return id_; }

        /** @brief The number of distinct script operations. */
        static constexpr int kOperationCount = 8;

    private:
        void RunOperation(int operation, Microsoft::Xna::Framework::GameComponentCollection& components);

        ChaosEngine& engine_;
        std::vector<int> script_;
        int id_;
        std::size_t updateStep_ = 0;
        std::size_t drawStep_ = 0;
        int initializeCalls_ = 0;
        bool pendingDestroy_ = false;
    };
}
