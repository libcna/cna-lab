// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/ColoredRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/ColoredRegion.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::TextureAtlases::ColoredRegion;
    using Myra::Graphics2D::TextureAtlases::TextureRegion;

    class RecordingTextureRegion final : public TextureRegion
    {
    public:
        RecordingTextureRegion(std::shared_ptr<Texture2D> texture, const Rectangle bounds)
            : TextureRegion(std::move(texture), bounds), lastColor(Color::Transparent)
        {
        }

        void Draw(
            RenderContext&,
            const Rectangle destination,
            const Color color) const override
        {
            ++drawCount;
            lastDestination = destination;
            lastColor = color;
        }

        mutable int drawCount = 0;
        mutable Rectangle lastDestination;
        mutable Color lastColor;
    };

    TEST(ColoredRegionTests, RetainsMutableRegionAndColorProperties)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 8, 6);
        auto first = std::make_shared<TextureRegion>(texture, Rectangle(1, 2, 3, 4));
        auto second = std::make_shared<TextureRegion>(texture, Rectangle(0, 0, 5, 2));
        const std::weak_ptr<TextureRegion> retained = first;
        ColoredRegion colored(first, Color::Red);

        first.reset();
        EXPECT_FALSE(retained.expired());
        EXPECT_EQ(colored.getTextureRegionProperty(), retained.lock());
        EXPECT_EQ(colored.getSizeProperty(), Point(3, 4));
        EXPECT_EQ(colored.getColorProperty(), Color::Red);

        colored.setTextureRegionProperty(second);
        colored.setColorProperty(Color::Blue);
        EXPECT_EQ(colored.getTextureRegionProperty(), second);
        EXPECT_EQ(colored.getSizeProperty(), Point(5, 2));
        EXPECT_EQ(colored.getColorProperty(), Color::Blue);
    }

    TEST(ColoredRegionTests, RejectsNullRegionConstructionAndAssignment)
    {
        EXPECT_THROW(
            (void) ColoredRegion(std::shared_ptr<TextureRegion>{}, Color::White),
            std::invalid_argument);

        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 1, 1);
        auto region = std::make_shared<TextureRegion>(texture);
        ColoredRegion colored(region, Color::White);
        EXPECT_THROW(
            colored.setTextureRegionProperty(std::shared_ptr<TextureRegion>{}),
            std::invalid_argument);
        EXPECT_EQ(colored.getTextureRegionProperty(), region);
    }

    TEST(ColoredRegionTests, PreservesWhiteFastPathAndUpstreamChannelTinting)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 2, 2);
        auto region =
            std::make_shared<RecordingTextureRegion>(texture, Rectangle(0, 0, 2, 2));
        const Color base(100, 200, 50, 128);
        const ColoredRegion colored(region, base);
        RenderContext context(game.getGraphicsDeviceProperty());
        const Rectangle destination(10, 20, 30, 40);

        colored.Draw(context, destination, Color::White);
        EXPECT_EQ(region->drawCount, 1);
        EXPECT_EQ(region->lastDestination, destination);
        EXPECT_EQ(region->lastColor, base);

        colored.Draw(context, destination, Color(128, 64, 255, 128));
        EXPECT_EQ(region->drawCount, 2);
        EXPECT_EQ(region->lastColor, Color(50, 50, 50, 64));
    }
}
