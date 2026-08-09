// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::TextureAtlases::TextureRegion;

    TEST(TextureRegionTests, RetainsTextureIdentityBoundsSizeAndNullableName)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 8, 6);
        const std::weak_ptr<Texture2D> weakTexture = texture;
        TextureRegion region(texture, Rectangle(2, 1, 4, 3));

        EXPECT_EQ(region.getTextureProperty(), texture);
        EXPECT_EQ(region.getBoundsProperty(), Rectangle(2, 1, 4, 3));
        EXPECT_EQ(region.getSizeProperty(), Point(4, 3));
        EXPECT_FALSE(region.getNameProperty().has_value());
        EXPECT_EQ(region.ToString(), "");

        region.setNameProperty("button");
        EXPECT_EQ(region.getNameProperty(), "button");
        EXPECT_EQ(region.ToString(), "button");

        texture.reset();
        EXPECT_FALSE(weakTexture.expired());
        EXPECT_EQ(region.getTextureProperty(), weakTexture.lock());
    }

    TEST(TextureRegionTests, BuildsWholeTextureAndCheckedRelativeRegions)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 8, 6);
        const TextureRegion whole(texture);
        const TextureRegion parent(texture, Rectangle(2, 1, 4, 3));
        const TextureRegion child(parent, Rectangle(1, 1, 2, 1));

        EXPECT_EQ(whole.getBoundsProperty(), Rectangle(0, 0, 8, 6));
        EXPECT_EQ(child.getTextureProperty(), texture);
        EXPECT_EQ(child.getBoundsProperty(), Rectangle(3, 2, 2, 1));

        const TextureRegion overflowing(
            texture, Rectangle(std::numeric_limits<int>::max(), 0, 1, 1));
        EXPECT_THROW(
            (void) TextureRegion(overflowing, Rectangle(1, 0, 1, 1)),
            std::overflow_error);
    }

    TEST(TextureRegionTests, RejectsNullTextures)
    {
        EXPECT_THROW(
            (void) TextureRegion(std::shared_ptr<Texture2D>{}, Rectangle(0, 0, 1, 1)),
            std::invalid_argument);
        EXPECT_THROW(
            (void) TextureRegion(std::shared_ptr<Texture2D>{}),
            std::invalid_argument);
    }

    TEST(TextureRegionTests, DrawsItsSourceBoundsThroughRenderContext)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 4, 4);
        const TextureRegion region(texture, Rectangle(1, 1, 2, 2));
        RenderContext context(game.getGraphicsDeviceProperty());
        context.setOpacityProperty(1.0F);

        context.Begin();
        EXPECT_NO_THROW(region.Draw(context, Rectangle(10, 20, 30, 40), Color::White));
        context.End();
    }
}
