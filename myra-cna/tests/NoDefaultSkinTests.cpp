// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
#include "Myra/Myra.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "RecordingSpriteBatchBackend.hpp"

#if !defined(MYRA_CNA_NO_DEFAULT_SKIN)
#error "This smoke test requires Myra-CNA's explicit no-default-skin mode"
#endif

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Brushes::SolidBrush;
    using Myra::Graphics2D::UI::Button;
    using Myra::Graphics2D::UI::VerticalStackPanel;
    using Myra::Tests::RecordingSpriteBatchBackend;

    class NoDefaultSkinTests : public testing::Test
    {
      protected:
        void SetUp() override { MyraEnvironment::ClearGame(); }

        void TearDown() override { MyraEnvironment::ClearGame(); }
    };

    TEST_F(NoDefaultSkinTests, ConstructsWidgetsAndDrawsAFontIndependentPrimitive)
    {
        auto root = std::make_shared<VerticalStackPanel>();
        auto button = std::make_shared<Button>();
        root->AddWidget(button);
        ASSERT_EQ(root->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(root->getWidgetsProperty().front(), button);

        Game game;
        MyraEnvironment::setGameProperty(game);
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend *const recording = backend.get();
        RenderContext context(std::make_unique<SpriteBatch>(std::move(backend)));
        context.setOpacityProperty(1.0F);

        context.Begin();
        SolidBrush(Color::Green).Draw(context, Rectangle(4, 5, 6, 7), Color::White);
        context.End();

        ASSERT_EQ(recording->draws.size(), 1U);
        EXPECT_EQ(recording->draws.front().destination, Rectangle(4, 5, 6, 7));
        EXPECT_EQ(recording->draws.front().source, Rectangle(0, 0, 1, 1));
        EXPECT_EQ(recording->draws.front().color, Color::Green);
    }
} // namespace
