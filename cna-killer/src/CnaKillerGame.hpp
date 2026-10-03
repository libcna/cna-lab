// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <memory>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"

#include "ChaosEngine.hpp"
#include "ChaosLog.hpp"
#include "CliOptions.hpp"
#include "Findings.hpp"

namespace CnaKiller
{
    /**
     * @brief The XNA "game" cna-killer pretends to be.
     *
     * There is nothing to play: every Update() tick hands control straight to ChaosEngine,
     * which creates and destroys GPU/audio resources, resizes and reconfigures the window and
     * device, and occasionally forces a raw device reset -- all on purpose, all reproducible
     * from a single --seed. Draw() just paints whatever chaos happens to be alive so the
     * mayhem is visible, and reports live stats through the window title.
     */
    class CnaKillerGame : public Microsoft::Xna::Framework::Game
    {
    public:
        CnaKillerGame(const CliOptions& options, ChaosLog& log, Findings& findings);
        ~CnaKillerGame() override = default;

        CnaKillerGame(const CnaKillerGame&) = delete;
        CnaKillerGame& operator=(const CnaKillerGame&) = delete;

    protected:
        void Initialize() override;
        void LoadContent() override;
        void UnloadContent() override;
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> spriteBatch_;
        CliOptions options_;
        ChaosEngine chaos_;
        std::uint64_t tick_ = 0;

        static constexpr std::uint64_t kTitleRefreshIntervalTicks = 15;
    };
}
