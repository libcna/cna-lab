// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra and MonoGame.Extended, MIT Licenses,
// Copyright (c) 2017-2020 The Myra Team and Copyright (c) 2015 Dylan Wilson.
// Ported from: src/Myra/Graphics2D/RenderContext.Shapes.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md, THIRD_PARTY_NOTICES.md, and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/RenderContext.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <numbers>
#include <span>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Myra::Graphics2D::RenderContext;
    using Myra::MyraEnvironment;
    using Myra::Tests::RecordingSpriteBatchBackend;

    class RenderContextShapesTests : public testing::Test
    {
    protected:
        void SetUp() override
        {
            MyraEnvironment::ClearGame();
            MyraEnvironment::setGameProperty(game_);
        }

        void TearDown() override
        {
            MyraEnvironment::ClearGame();
        }

        [[nodiscard]] static std::unique_ptr<RenderContext> CreateContext(
            RecordingSpriteBatchBackend*& recording)
        {
            auto backend = std::make_unique<RecordingSpriteBatchBackend>();
            recording = backend.get();
            auto spriteBatch = std::make_unique<SpriteBatch>(std::move(backend));
            auto context = std::make_unique<RenderContext>(std::move(spriteBatch));
            context->setOpacityProperty(1.0F);
            context->Begin();
            return context;
        }

        Game game_;
    };

    TEST_F(RenderContextShapesTests, FillRectangleOverloadsPreserveOrderAndTruncateFloats)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);

        context->FillRectangle(Rectangle(1, 2, 3, 4), Color::Red);
        context->FillRectangle(Vector2(5.9F, -6.9F), Vector2(7.9F, 8.9F), Color::Green);
        context->FillRectangle(9.9F, 10.1F, 11.9F, 12.9F, Color::Blue);
        context->End();

        ASSERT_EQ(recording->draws.size(), 3U);
        EXPECT_EQ(recording->draws[0].destination, Rectangle(1, 2, 3, 4));
        EXPECT_EQ(recording->draws[0].color, Color::Red);
        EXPECT_EQ(recording->draws[1].destination, Rectangle(5, -6, 7, 8));
        EXPECT_EQ(recording->draws[1].color, Color::Green);
        EXPECT_EQ(recording->draws[2].destination, Rectangle(9, 10, 11, 12));
        EXPECT_EQ(recording->draws[2].color, Color::Blue);
    }

    TEST_F(RenderContextShapesTests, DrawRectanglePreservesEdgeOrderAndDoubleOpacity)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);
        context->setOpacityProperty(0.5F);
        const Color color(200, 100, 50, 240);

        context->DrawRectangle(Rectangle(10, 20, 30, 40), color, 2.9F);
        context->DrawRectangle(Vector2(1.9F, 2.9F), Vector2(8.9F, 6.9F), color);
        context->End();

        ASSERT_EQ(recording->draws.size(), 8U);
        EXPECT_EQ(recording->draws[0].destination, Rectangle(10, 20, 30, 2));
        EXPECT_EQ(recording->draws[1].destination, Rectangle(10, 58, 30, 2));
        EXPECT_EQ(recording->draws[2].destination, Rectangle(10, 20, 2, 40));
        EXPECT_EQ(recording->draws[3].destination, Rectangle(38, 20, 2, 40));
        EXPECT_EQ(recording->draws[4].destination, Rectangle(1, 2, 8, 1));
        EXPECT_EQ(recording->draws[5].destination, Rectangle(1, 7, 8, 1));
        EXPECT_EQ(recording->draws[6].destination, Rectangle(1, 2, 1, 6));
        EXPECT_EQ(recording->draws[7].destination, Rectangle(8, 2, 1, 6));

        const Color once = Myra::Utility::CrossEngineStuff::MultiplyColor(color, 0.5F);
        const Color twice = Myra::Utility::CrossEngineStuff::MultiplyColor(once, 0.5F);
        for (const auto& draw : recording->draws)
        {
            EXPECT_EQ(draw.color, twice);
        }
    }

    TEST_F(RenderContextShapesTests, PolygonClosesItsLastEdgeAndPreservesOnePointOffsetDefect)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);
        const std::array<Vector2, 3> triangle{
            Vector2(0.0F, 0.0F), Vector2(3.0F, 0.0F), Vector2(3.0F, 4.0F)};
        const std::array<Vector2, 1> point{Vector2(7.0F, 8.0F)};

        context->DrawPolygon(Vector2(10.0F, 20.0F), triangle, Color::White, 2.0F);
        context->DrawPolygon(Vector2(100.0F, 200.0F), point, Color::Red, 3.8F);
        context->End();

        ASSERT_EQ(recording->draws.size(), 4U);
        EXPECT_EQ(recording->draws[0].destination, Rectangle(10, 20, 3, 2));
        EXPECT_FLOAT_EQ(recording->draws[0].rotation, 0.0F);
        EXPECT_EQ(recording->draws[1].destination, Rectangle(13, 20, 4, 2));
        EXPECT_FLOAT_EQ(recording->draws[1].rotation, std::numbers::pi_v<float> / 2.0F);
        EXPECT_EQ(recording->draws[2].destination, Rectangle(13, 24, 5, 2));
        EXPECT_FLOAT_EQ(recording->draws[2].rotation, std::atan2(-4.0F, -3.0F));

        // The selected upstream ignores DrawPolygon's offset for one point.
        EXPECT_EQ(recording->draws[3].destination, Rectangle(6, 7, 3, 3));
        EXPECT_EQ(recording->draws[3].color, Color::Red);
    }

    TEST_F(RenderContextShapesTests, LineAndPointOverloadsPreserveOffsetsAndScale)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);

        context->DrawLine(0.0F, 0.0F, 3.0F, 4.0F, Color::Red, 2.0F);
        context->DrawLine(Vector2(10.0F, 20.0F), 6.0F, 0.0F, Color::Green, 4.0F);
        context->DrawPoint(Vector2(8.0F, 9.0F), Color::Blue, 3.0F);
        context->DrawPoint(1.0F, 2.0F, Color::White);
        context->End();

        ASSERT_EQ(recording->draws.size(), 4U);
        EXPECT_EQ(recording->draws[0].destination, Rectangle(0, 0, 5, 2));
        EXPECT_FLOAT_EQ(recording->draws[0].rotation, std::atan2(4.0F, 3.0F));
        EXPECT_EQ(recording->draws[1].destination, Rectangle(10, 18, 6, 4));
        EXPECT_FLOAT_EQ(recording->draws[1].rotation, 0.0F);
        EXPECT_EQ(recording->draws[2].destination, Rectangle(7, 8, 3, 3));
        EXPECT_EQ(recording->draws[3].destination, Rectangle(1, 2, 1, 1));
    }

    TEST_F(RenderContextShapesTests, CircleAndArcUseClosedPolygonEdgeCounts)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);

        context->DrawCircle(Vector2(10.0F, 20.0F), 5.0F, 4, Color::Red, 1.0F);
        context->DrawCircle(30.0F, 40.0F, 3.0F, 3, Color::Green, 2.0F);
        context->DrawArc(
            Vector2(50.0F, 60.0F), 4.0F, 3, Color::Blue,
            0.0F, std::numbers::pi_v<float>, 1.0F);
        context->DrawArc(
            70.0F, 80.0F, 2.0F, 2, Color::White,
            1.0F, 0.0F, 1.0F);
        context->End();

        // DrawPolygon closes both circles and both arcs, including a zero-span arc.
        EXPECT_EQ(recording->draws.size(), 4U + 3U + 3U + 2U);
        EXPECT_EQ(recording->draws.front().destination.X, 15);
        EXPECT_EQ(recording->draws.front().destination.Y, 20);
    }

    TEST_F(RenderContextShapesTests, EmptyShapesDoNotRequireEnvironmentOrAnActiveBatch)
    {
        MyraEnvironment::ClearGame();
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend* const recording = backend.get();
        auto spriteBatch = std::make_unique<SpriteBatch>(std::move(backend));
        RenderContext context(std::move(spriteBatch));

        EXPECT_NO_THROW(context.DrawPolygon(
            Vector2(std::numeric_limits<float>::quiet_NaN()),
            std::span<const Vector2>{},
            Color::White,
            std::numeric_limits<float>::quiet_NaN()));
        EXPECT_NO_THROW(context.DrawCircle(
            Vector2(std::numeric_limits<float>::quiet_NaN()),
            std::numeric_limits<float>::quiet_NaN(),
            0,
            Color::White));
        EXPECT_NO_THROW(context.DrawArc(
            Vector2(std::numeric_limits<float>::quiet_NaN()),
            std::numeric_limits<float>::quiet_NaN(),
            0,
            Color::White,
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN()));
        EXPECT_TRUE(recording->draws.empty());
    }

    TEST_F(RenderContextShapesTests, RejectsUnsafeNumericGeometryDeterministically)
    {
        RecordingSpriteBatchBackend* recording = nullptr;
        auto context = CreateContext(recording);
        const float infinity = std::numeric_limits<float>::infinity();
        const float nan = std::numeric_limits<float>::quiet_NaN();

        EXPECT_THROW(
            context->FillRectangle(Vector2(infinity, 0.0F), Vector2::One, Color::White),
            std::invalid_argument);
        EXPECT_THROW(
            context->DrawRectangle(
                Rectangle(std::numeric_limits<int>::max(), 0, 1, 1), Color::White),
            std::overflow_error);
        EXPECT_THROW(
            context->DrawLine(Vector2(nan, 0.0F), Vector2::One, Color::White),
            std::invalid_argument);
        EXPECT_THROW(
            context->DrawPoint(Vector2::Zero, Color::White, infinity),
            std::invalid_argument);
        EXPECT_THROW(
            context->DrawCircle(Vector2::Zero, 1.0F, -1, Color::White),
            std::invalid_argument);
        EXPECT_THROW(
            context->DrawArc(Vector2::Zero, 1.0F, -1, Color::White, 0.0F, 1.0F),
            std::invalid_argument);

        context->End();
        EXPECT_TRUE(recording->draws.empty());
    }
}
