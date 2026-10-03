// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Myra/Graphics2D/Brushes/SolidBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/MyraEnvironment.hpp"

#include <cstdio>
#include <memory>

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::GraphicsDeviceManager;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::PresentationMode;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Brushes::SolidBrush;

    constexpr int CanvasSize = 64;

    [[nodiscard]] bool IsApproximately(
        const Color& color, const int red, const int green, const int blue)
    {
        constexpr int tolerance = 20;
        const auto difference = [](const int left, const int right) {
            return left > right ? left - right : right - left;
        };
        return difference(color.getRProperty(), red) <= tolerance
            && difference(color.getGProperty(), green) <= tolerance
            && difference(color.getBProperty(), blue) <= tolerance;
    }

    class MyraSdlRendererSmoke final : public Game
    {
    public:
        MyraSdlRendererSmoke()
        {
            graphics_ = std::make_unique<GraphicsDeviceManager>(this);
            graphics_->setPreferredBackBufferWidthProperty(CanvasSize);
            graphics_->setPreferredBackBufferHeightProperty(CanvasSize);
            graphics_->setPreferredPresentationModeProperty(PresentationMode::NativeBackBuffer);
        }

        [[nodiscard]] int getResultProperty() const noexcept { return result_; }

    protected:
        void Initialize() override
        {
            Game::Initialize();
            Myra::MyraEnvironment::setGameProperty(*this);
            context_ = std::make_unique<RenderContext>();
            context_->setOpacityProperty(1.0F);
            context_->setScissorProperty(Rectangle(0, 0, CanvasSize, CanvasSize));
        }

        void Draw(const GameTime&) override
        {
            if (finished_)
            {
                return;
            }
            finished_ = true;

            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color(0, 255, 0, 255));

            SolidBrush brush(Color(255, 0, 0, 255));
            context_->Begin();
            brush.Draw(*context_, Rectangle(10, 10, 20, 20), Color::White);
            context_->End();

            Color inside(0, 0, 0, 0);
            const Rectangle insideRegion(15, 15, 1, 1);
            device.GetBackBufferData(&insideRegion, &inside, 0, 1);

            Color outside(0, 0, 0, 0);
            const Rectangle outsideRegion(0, 0, 1, 1);
            device.GetBackBufferData(&outsideRegion, &outside, 0, 1);

            const bool insideIsRed = IsApproximately(inside, 255, 0, 0);
            const bool outsideIsGreen = IsApproximately(outside, 0, 255, 0);
            std::printf("[%s] Myra SolidBrush inside: (%d,%d,%d)\n",
                insideIsRed ? "PASS" : "FAIL", inside.getRProperty(),
                inside.getGProperty(), inside.getBProperty());
            std::printf("[%s] cleared background outside: (%d,%d,%d)\n",
                outsideIsGreen ? "PASS" : "FAIL", outside.getRProperty(),
                outside.getGProperty(), outside.getBProperty());

            result_ = insideIsRed && outsideIsGreen ? 0 : 1;
            Exit();
        }

    private:
        std::unique_ptr<GraphicsDeviceManager> graphics_;
        std::unique_ptr<RenderContext> context_;
        bool finished_ = false;
        int result_ = 1;
    };
}

int main()
{
    MyraSdlRendererSmoke game;
    game.Run();
    return game.getResultProperty();
}
