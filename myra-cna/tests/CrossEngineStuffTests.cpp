// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/CrossEngineStuff.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "Myra/MyraEnvironment.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using Myra::MyraEnvironment;
    using Myra::Utility::CrossEngineStuff;

    class CrossEngineStuffLinkedTests : public testing::Test
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

    TEST(CrossEngineStuffTests, MultiplyColorUsesCnaChannelScaling)
    {
        const Color source(200, 100, 50, 240);
        const Color result = CrossEngineStuff::MultiplyColor(source, 0.5F);

        EXPECT_EQ(result.getRProperty(), 100);
        EXPECT_EQ(result.getGProperty(), 50);
        EXPECT_EQ(result.getBProperty(), 25);
        EXPECT_EQ(result.getAProperty(), 120);
    }

    TEST(CrossEngineStuffTests, MultiplyColorPreservesCnaClampingEndpoints)
    {
        const Color source(200, 100, 50, 240);

        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 0.0F), Color(0, 0, 0, 0));
        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 1.0F), source);
        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 2.0F), Color(255, 200, 100, 255));
    }

    TEST_F(CrossEngineStuffLinkedTests, ViewSizeUsesTheConfiguredDeviceViewport)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();
        device.setViewportProperty(Viewport(7, 9, 321, 123));
        MyraEnvironment::setGameProperty(game);

        const Point size = CrossEngineStuff::getViewSizeProperty();

        EXPECT_EQ(size.X, 321);
        EXPECT_EQ(size.Y, 123);
    }

    TEST_F(CrossEngineStuffLinkedTests, CreatesTexturesAndRejectsInvalidDevicesOrDimensions)
    {
        Game game;
        auto& device = game.getGraphicsDeviceProperty();

        Texture2D texture = CrossEngineStuff::CreateTexture(device, 3, 2);

        EXPECT_EQ(texture.getWidthProperty(), 3);
        EXPECT_EQ(texture.getHeightProperty(), 2);
        EXPECT_THROW((void) CrossEngineStuff::CreateTexture(device, 0, 2), std::invalid_argument);
        EXPECT_THROW((void) CrossEngineStuff::CreateTexture(device, 2, -1), std::invalid_argument);

        texture.Dispose();
        device.Dispose();
        EXPECT_THROW((void) CrossEngineStuff::CreateTexture(device, 1, 1), std::invalid_argument);
    }

    TEST_F(CrossEngineStuffLinkedTests, UploadsRgbaBytesToTheRequestedTextureRegion)
    {
        Game game;
        Texture2D texture = CrossEngineStuff::CreateTexture(game.getGraphicsDeviceProperty(), 3, 2);
        const Rectangle bounds(1, 0, 2, 2);
        const std::vector<std::uint8_t> rgba{
            255, 0, 0, 255,
            0, 255, 0, 128,
            0, 0, 255, 64,
            255, 255, 0, 32,
            17, 18, 19, 20,
        };

        CrossEngineStuff::SetTextureData(texture, bounds, rgba);

        std::vector<Color> pixels(6, Color::Transparent);
        texture.GetData(pixels.data(), 0, static_cast<int>(pixels.size()));
        EXPECT_EQ(pixels[0], Color::Transparent);
        EXPECT_EQ(pixels[1], Color(255, 0, 0, 255));
        EXPECT_EQ(pixels[2], Color(0, 255, 0, 128));
        EXPECT_EQ(pixels[3], Color::Transparent);
        EXPECT_EQ(pixels[4], Color(0, 0, 255, 64));
        EXPECT_EQ(pixels[5], Color(255, 255, 0, 32));
    }

    TEST_F(CrossEngineStuffLinkedTests, RejectsInvalidTextureUploadsBeforeCallingCna)
    {
        Game game;
        Texture2D texture = CrossEngineStuff::CreateTexture(game.getGraphicsDeviceProperty(), 2, 2);
        const std::vector<std::uint8_t> onePixel{1, 2, 3, 4};

        EXPECT_THROW(
            CrossEngineStuff::SetTextureData(texture, Rectangle(-1, 0, 1, 1), onePixel),
            std::invalid_argument);
        EXPECT_THROW(
            CrossEngineStuff::SetTextureData(texture, Rectangle(0, 0, 0, 1), onePixel),
            std::invalid_argument);
        EXPECT_THROW(
            CrossEngineStuff::SetTextureData(texture, Rectangle(1, 1, 2, 1), onePixel),
            std::out_of_range);
        EXPECT_THROW(
            CrossEngineStuff::SetTextureData(texture, Rectangle(0, 0, 2, 1), onePixel),
            std::out_of_range);

        texture.Dispose();
        EXPECT_THROW(
            CrossEngineStuff::SetTextureData(texture, Rectangle(0, 0, 1, 1), onePixel),
            std::invalid_argument);
    }
}
