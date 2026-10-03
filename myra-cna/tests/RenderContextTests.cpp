// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/RenderContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/RenderContext.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace Myra::Graphics2D
{
    struct RenderContextTestAccess
    {
        static void SetTextureFiltering(RenderContext& context, const TextureFiltering value)
        {
            context.SetTextureFiltering(value);
        }
    };
}

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::TextureFiltering;
    using Myra::Graphics2D::Transform;
    using Myra::MyraEnvironment;
    using Myra::Tests::DummyTextureBackend;
    using Myra::Tests::RecordingSpriteBatchBackend;

    class RenderContextTests : public testing::Test
    {
    protected:
        void SetUp() override
        {
            MyraEnvironment::ClearGame();
            MyraEnvironment::setDisableClippingProperty(false);
        }

        void TearDown() override
        {
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::ClearGame();
        }
    };

    TEST_F(RenderContextTests, DefaultConstructionRequiresTheConfiguredEnvironmentDevice)
    {
        EXPECT_THROW((void) RenderContext(), std::logic_error);

        Game game;
        MyraEnvironment::setGameProperty(game);
        EXPECT_NO_THROW((void) RenderContext());
    }

    TEST_F(RenderContextTests, PreservesDefaultsAndEnforcesTheBatchStateMachine)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        RenderContext context(device);

        EXPECT_FLOAT_EQ(context.getOpacityProperty(), 0.0F);
        EXPECT_EQ(context.getTextureFilteringProperty(), TextureFiltering::Nearest);
        EXPECT_FALSE(context.getIsRenderingProperty());
        EXPECT_FALSE(context.getIsDisposedProperty());
        EXPECT_EQ(context.getTransformProperty().Apply(Vector2(3.0F, 4.0F)), Vector2(3.0F, 4.0F));
        EXPECT_NO_THROW(context.Flush());
        EXPECT_THROW(context.End(), std::logic_error);

        context.Begin();
        EXPECT_TRUE(context.getIsRenderingProperty());
        EXPECT_TRUE(device.getRasterizerStateProperty().getScissorTestEnableProperty());
        EXPECT_THROW(context.Begin(), std::logic_error);
        EXPECT_NO_THROW(context.Flush());
        EXPECT_TRUE(context.getIsRenderingProperty());
        context.End();
        EXPECT_FALSE(context.getIsRenderingProperty());
    }

    TEST_F(RenderContextTests, AppliesViewportRelativeScissorsAndHonorsTheDebugOverride)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        device.setViewportProperty(Viewport(7, 9, 320, 200));
        RenderContext context(device);

        context.Begin();
        context.setScissorProperty(Rectangle(1, 2, 30, 40));
        EXPECT_TRUE(context.getIsRenderingProperty());
        EXPECT_EQ(context.getScissorProperty(), Rectangle(1, 2, 30, 40));
        EXPECT_EQ(context.getDeviceScissorProperty(), Rectangle(8, 11, 30, 40));

        MyraEnvironment::setDisableClippingProperty(true);
        context.setScissorProperty(Rectangle(5, 6, 10, 20));
        EXPECT_EQ(context.getScissorProperty(), Rectangle(5, 6, 10, 20));
        EXPECT_EQ(context.getDeviceScissorProperty(), Rectangle(8, 11, 30, 40));
        context.End();
    }

    TEST_F(RenderContextTests, DrawsAllTextureOverloadShapesAndSwitchesFilteringAtBatchBoundaries)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        Texture2D texture(device, 2, 2);
        const std::vector<Color> pixels(4, Color::White);
        texture.SetData(pixels.data(), static_cast<int>(pixels.size()));

        RenderContext context(device);
        context.setOpacityProperty(0.5F);
        context.setTransformProperty(Transform(
            Vector2(10.0F, 20.0F), Vector2::Zero, Vector2(2.0F, 3.0F), 0.25F));
        context.Begin();

        EXPECT_NO_THROW(context.Draw(texture, Rectangle(1, 2, 8, 6), Color::White));
        EXPECT_NO_THROW(context.Draw(
            texture, Rectangle(1, 2, 8, 6), Rectangle(0, 0, 2, 2), Color::Red));
        EXPECT_NO_THROW(context.Draw(
            texture, Rectangle(1, 2, 8, 6), Rectangle(0, 0, 2, 2), Color::Green, 0.1F));
        EXPECT_NO_THROW(context.Draw(
            texture, Rectangle(1, 2, 8, 6), Rectangle(0, 0, 2, 2), Color::Blue, 0.1F, 0.4F));
        EXPECT_NO_THROW(context.Draw(texture, Vector2(1.0F, 2.0F), Color::White));
        EXPECT_NO_THROW(context.Draw(
            texture, Vector2(1.0F, 2.0F), Rectangle(0, 0, 2, 2), Color::White));
        EXPECT_NO_THROW(context.Draw(
            texture, Vector2(1.0F, 2.0F), Rectangle(0, 0, 2, 2), Color::White, 0.2F));
        EXPECT_NO_THROW(context.Draw(
            texture, Vector2(1.0F, 2.0F), Color::White, Vector2(2.0F, 3.0F), 0.2F));

        context.SetAnisotropicFilteringMode(true);
        EXPECT_NO_THROW(context.Draw(texture, Vector2::Zero, Color::White));
        EXPECT_EQ(context.getTextureFilteringProperty(), TextureFiltering::Anisotropic);
        EXPECT_TRUE(context.getIsRenderingProperty());

        context.SetAnisotropicFilteringMode(false);
        EXPECT_NO_THROW(context.Draw(texture, Vector2::Zero, Color::White));
        EXPECT_EQ(context.getTextureFilteringProperty(), TextureFiltering::Nearest);
        context.End();
    }

    TEST_F(RenderContextTests, RejectsInvalidSourcesAndUseAfterDispose)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        Texture2D texture(device, 2, 2);
        RenderContext context(device);
        context.Begin();

        EXPECT_THROW(
            context.Draw(texture, Rectangle(0, 0, 1, 1), Rectangle(0, 0, 0, 1), Color::White),
            std::invalid_argument);
        context.End();

        context.Dispose();
        context.Dispose();
        EXPECT_TRUE(context.getIsDisposedProperty());
        EXPECT_THROW(context.Begin(), std::logic_error);
        EXPECT_THROW(context.Flush(), std::logic_error);
        EXPECT_THROW((void) context.getDeviceScissorProperty(), std::logic_error);
    }

    TEST_F(RenderContextTests, ForwardsTransformedAndOpacityAdjustedDrawParametersToSpriteBatch)
    {
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend* const recording = backend.get();
        auto spriteBatch =
            std::make_unique<Microsoft::Xna::Framework::Graphics::SpriteBatch>(std::move(backend));
        RenderContext context(std::move(spriteBatch));
        const auto textureBackend = std::make_shared<DummyTextureBackend>(8, 6);
        const Texture2D texture = Texture2D::CreateWithRendererForTests(8, 6, textureBackend);
        const Transform transform(
            Vector2(10.0F, 20.0F), Vector2::Zero, Vector2(2.0F, 3.0F), 0.25F);

        context.setOpacityProperty(0.5F);
        context.setTransformProperty(transform);
        context.Begin();
        context.Draw(
            texture,
            Vector2(1.0F, 2.0F),
            Rectangle(2, 1, 3, 2),
            Color::White,
            0.1F,
            Vector2(4.0F, 5.0F),
            0.4F);
        context.End();

        ASSERT_EQ(recording->beginCount, 1);
        ASSERT_EQ(recording->endCount, 1);
        ASSERT_EQ(recording->samplerFilters.size(), 1U);
        EXPECT_EQ(
            recording->samplerFilters.front(),
            static_cast<int>(Microsoft::Xna::Framework::Graphics::TextureFilter::Point));
        ASSERT_EQ(recording->draws.size(), 1U);

        const Vector2 expectedPosition = transform.Apply(Vector2(1.0F, 2.0F));
        const auto& draw = recording->draws.front();
        EXPECT_EQ(draw.destination.X, static_cast<int>(expectedPosition.X));
        EXPECT_EQ(draw.destination.Y, static_cast<int>(expectedPosition.Y));
        EXPECT_EQ(draw.destination.Width, 24);
        EXPECT_EQ(draw.destination.Height, 30);
        EXPECT_EQ(draw.source, Rectangle(2, 1, 3, 2));
        EXPECT_EQ(draw.color, Myra::Utility::CrossEngineStuff::MultiplyColor(Color::White, 0.5F));
        EXPECT_FLOAT_EQ(draw.rotation, 0.35F);
        EXPECT_EQ(draw.origin, Vector2::Zero);
        EXPECT_EQ(
            draw.effects, Microsoft::Xna::Framework::Graphics::SpriteEffects::None);
        EXPECT_FLOAT_EQ(draw.depth, 0.4F);
    }

    TEST_F(RenderContextTests, MapsEveryFilteringModeAndFlushesBeforeChangingSampler)
    {
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend* const recording = backend.get();
        auto spriteBatch =
            std::make_unique<Microsoft::Xna::Framework::Graphics::SpriteBatch>(std::move(backend));
        RenderContext context(std::move(spriteBatch));

        context.Begin();
        Myra::Graphics2D::RenderContextTestAccess::SetTextureFiltering(
            context, TextureFiltering::Linear);
        Myra::Graphics2D::RenderContextTestAccess::SetTextureFiltering(
            context, TextureFiltering::Anisotropic);
        context.End();

        EXPECT_EQ(recording->beginCount, 3);
        EXPECT_EQ(recording->endCount, 3);
        ASSERT_EQ(recording->samplerFilters.size(), 3U);
        EXPECT_EQ(
            recording->samplerFilters[0],
            static_cast<int>(Microsoft::Xna::Framework::Graphics::TextureFilter::Point));
        EXPECT_EQ(
            recording->samplerFilters[1],
            static_cast<int>(Microsoft::Xna::Framework::Graphics::TextureFilter::Linear));
        EXPECT_EQ(
            recording->samplerFilters[2],
            static_cast<int>(Microsoft::Xna::Framework::Graphics::TextureFilter::Anisotropic));
    }

    TEST_F(RenderContextTests, SupportsNestedClipTransitionsAndRestoresTheOuterScissor)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        device.setViewportProperty(Viewport(7, 9, 320, 200));
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend* const recording = backend.get();
        auto spriteBatch =
            std::make_unique<Microsoft::Xna::Framework::Graphics::SpriteBatch>(
                std::move(backend));
        RenderContext context(std::move(spriteBatch), &device);

        context.Begin();
        const Rectangle outer(10, 20, 100, 80);
        context.setScissorProperty(outer);
        EXPECT_EQ(context.getDeviceScissorProperty(), Rectangle(17, 29, 100, 80));

        const Rectangle savedOuter = context.getScissorProperty();
        const Rectangle nested = Rectangle::Intersect(
            savedOuter, Rectangle(50, 0, 100, 50));
        context.setScissorProperty(nested);
        EXPECT_EQ(nested, Rectangle(50, 20, 60, 30));
        EXPECT_EQ(context.getDeviceScissorProperty(), Rectangle(57, 29, 60, 30));

        context.setScissorProperty(savedOuter);
        EXPECT_EQ(context.getScissorProperty(), outer);
        EXPECT_EQ(context.getDeviceScissorProperty(), Rectangle(17, 29, 100, 80));
        EXPECT_TRUE(context.getIsRenderingProperty());
        context.End();

        // Every active scissor transition flushes before changing device state.
        EXPECT_EQ(recording->beginCount, 4);
        EXPECT_EQ(recording->endCount, 4);
    }

    TEST_F(RenderContextTests, RejectsANullInjectedSpriteBatch)
    {
        EXPECT_THROW(
            (void) RenderContext(
                std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch>{}),
            std::invalid_argument);
    }
}
