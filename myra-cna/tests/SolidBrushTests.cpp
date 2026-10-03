// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/Brushes/SolidBrush.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/Brushes/SolidBrush.hpp"
#include "ComparisonHelpers.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Brushes::SolidBrush;
    using Myra::MyraEnvironment;
    using Myra::Tests::RecordingSpriteBatchBackend;

    static_assert(std::is_base_of_v<Myra::Graphics2D::IBrush, SolidBrush>);
    static_assert(std::is_base_of_v<Myra::MML::IHasColor, SolidBrush>);

    class SolidBrushTests : public testing::Test
    {
    protected:
        void SetUp() override
        {
            MyraEnvironment::ClearGame();
        }

        void TearDown() override
        {
            MyraEnvironment::ClearGame();
        }
    };

    TEST_F(SolidBrushTests, PreservesMutableColor)
    {
        SolidBrush brush(Color::Red);
        EXPECT_EQ(brush.getColorProperty(), Color::Red);

        brush.setColorProperty(Color::Blue);
        EXPECT_EQ(brush.getColorProperty(), Color::Blue);
    }

    TEST_F(SolidBrushTests, DrawsThroughTheCachedWhiteRegionWithUpstreamTinting)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend* const recording = backend.get();
        RenderContext context(std::make_unique<SpriteBatch>(std::move(backend)));
        context.setOpacityProperty(1.0F);
        const Rectangle destination(10, 20, 30, 40);
        const SolidBrush brush(Color(100, 200, 50, 128));

        context.Begin();
        brush.Draw(context, destination, Color::White);
        brush.Draw(context, destination, Color(128, 64, 255, 128));
        context.End();

        ASSERT_EQ(recording->draws.size(), 2U);
        EXPECT_EQ(recording->draws[0].destination, destination);
        EXPECT_EQ(recording->draws[0].source, Rectangle(0, 0, 1, 1));
        EXPECT_MYRA_COLOR_EQ(Color(100, 200, 50, 128), recording->draws[0].color);
        EXPECT_EQ(recording->draws[1].destination, destination);
        EXPECT_EQ(recording->draws[1].source, Rectangle(0, 0, 1, 1));
        EXPECT_MYRA_COLOR_EQ(Color(50, 50, 50, 64), recording->draws[1].color);
    }

    TEST_F(SolidBrushTests, DrawRequiresTheEnvironmentWhiteRegion)
    {
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RenderContext context(std::make_unique<SpriteBatch>(std::move(backend)));
        const SolidBrush brush(Color::Red);

        context.Begin();
        EXPECT_THROW(
            brush.Draw(context, Rectangle(0, 0, 1, 1), Color::White),
            std::logic_error);
        context.End();
    }
}
