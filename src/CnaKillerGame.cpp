// SPDX-License-Identifier: MIT
#include "CnaKillerGame.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace CnaKiller
{
    CnaKillerGame::CnaKillerGame(const CliOptions& options, ChaosLog& log)
        : graphics_(this)
        , options_(options)
        , chaos_(options, log)
    {
        // 32-bit index buffers, 2048-texel render targets and MSAA are HiDef features; an XNA
        // game without a HiDef RuntimeProfile resource gets Reach, which refuses all three.
        graphics_.setGraphicsProfileProperty(GraphicsProfile::HiDef);
        graphics_.setPreferredBackBufferWidthProperty(1280);
        graphics_.setPreferredBackBufferHeightProperty(720);

        // Hostile from the first frame: nothing here waits for a stable device before the
        // chaos engine starts tearing it apart.
        setIsMouseVisibleProperty(true);
        getWindowProperty().setTitleProperty(
            "cna-killer -- seed=" + std::to_string(options_.seed) + " (starting...)");
    }

    void CnaKillerGame::Initialize()
    {
        Game::Initialize();
    }

    void CnaKillerGame::LoadContent()
    {
        spriteBatch_ = std::make_unique<SpriteBatch>(*graphics_.getGraphicsDeviceProperty());
        chaos_.Bind(*this, graphics_);
    }

    void CnaKillerGame::UnloadContent()
    {
        spriteBatch_.reset();
    }

    void CnaKillerGame::Update(GameTime& gameTime)
    {
        if (GamePad::GetState(PlayerIndex::One).getButtonsProperty().getBackProperty() ==
                ButtonState::Pressed ||
            Keyboard::GetState().IsKeyDown(Keys::Escape))
        {
            Exit();
            Game::Update(gameTime);
            return;
        }

        ++tick_;
        const double totalSeconds = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
        if (!chaos_.Tick(tick_, totalSeconds))
        {
            Exit();
        }
        else if (tick_ % kTitleRefreshIntervalTicks == 0)
        {
            getWindowProperty().setTitleProperty(chaos_.StatsSummary(tick_));
        }

        Game::Update(gameTime);
    }

    void CnaKillerGame::Draw(const GameTime& gameTime)
    {
        GraphicsDevice& device = *graphics_.getGraphicsDeviceProperty();
        device.Clear(Color(16, 16, 20));

        if (spriteBatch_)
            chaos_.Draw(*spriteBatch_);

        Game::Draw(gameTime);
    }
}
