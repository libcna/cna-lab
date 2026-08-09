// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TintedRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TintedRegion.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <type_traits>

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
    using Myra::Graphics2D::TextureAtlases::TextureRegion;
    using Myra::Graphics2D::TextureAtlases::TintedRegion;

    class RecordingTextureRegion final : public TextureRegion
    {
    public:
        RecordingTextureRegion(std::shared_ptr<Texture2D> texture, const Rectangle bounds)
            : TextureRegion(std::move(texture), bounds), lastColor(Color::Transparent)
        {
        }

        void Draw(RenderContext&, const Rectangle destination, const Color color) const override
        {
            ++drawCount;
            lastDestination = destination;
            lastColor = color;
        }

        mutable int drawCount = 0;
        mutable Rectangle lastDestination;
        mutable Color lastColor;
    };

    static_assert(std::is_base_of_v<Myra::Graphics2D::IImage, TintedRegion>);
    static_assert(std::is_base_of_v<Myra::MML::IHasColor, TintedRegion>);

    TEST(TintedRegionTests, RetainsImmutableRegionColorAndSize)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 8, 6);
        auto region = std::make_shared<TextureRegion>(texture, Rectangle(1, 2, 3, 4));
        const std::weak_ptr<TextureRegion> retained = region;
        const TintedRegion tinted(region, Color::Red);

        region.reset();
        EXPECT_FALSE(retained.expired());
        EXPECT_EQ(tinted.getRegionProperty(), retained.lock());
        EXPECT_EQ(tinted.getColorProperty(), Color::Red);
        EXPECT_EQ(tinted.getSizeProperty(), Point(3, 4));
    }

    TEST(TintedRegionTests, RejectsNullRegions)
    {
        EXPECT_THROW(
            (void) TintedRegion(std::shared_ptr<TextureRegion>{}, Color::White),
            std::invalid_argument);
    }

    TEST(TintedRegionTests, UsesRegionReferenceIdentityForEqualityAndHashing)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 2, 2);
        auto firstRegion = std::make_shared<TextureRegion>(texture);
        auto equivalentButDistinctRegion = std::make_shared<TextureRegion>(texture);
        const TintedRegion first(firstRegion, Color::Blue);
        const TintedRegion same(firstRegion, Color::Blue);
        const TintedRegion differentColor(firstRegion, Color::Red);
        const TintedRegion differentRegion(equivalentButDistinctRegion, Color::Blue);

        EXPECT_TRUE(first.Equals(same));
        EXPECT_EQ(first.GetHashCode(), same.GetHashCode());
        EXPECT_FALSE(first.Equals(differentColor));
        EXPECT_FALSE(first.Equals(differentRegion));
    }

    TEST(TintedRegionTests, PreservesWhitePassThroughAndUpstreamChannelTinting)
    {
        Game game;
        auto texture = std::make_shared<Texture2D>(game.getGraphicsDeviceProperty(), 2, 2);
        auto whiteRegion =
            std::make_shared<RecordingTextureRegion>(texture, Rectangle(0, 0, 2, 2));
        auto tintedRegion =
            std::make_shared<RecordingTextureRegion>(texture, Rectangle(0, 0, 2, 2));
        const TintedRegion white(whiteRegion, Color::White);
        const TintedRegion tinted(tintedRegion, Color(100, 200, 50, 128));
        RenderContext context(game.getGraphicsDeviceProperty());
        const Rectangle destination(10, 20, 30, 40);
        const Color input(128, 64, 255, 128);

        white.Draw(context, destination, input);
        EXPECT_EQ(whiteRegion->lastColor, input);

        tinted.Draw(context, destination, input);
        EXPECT_EQ(tintedRegion->drawCount, 1);
        EXPECT_EQ(tintedRegion->lastDestination, destination);
        EXPECT_EQ(tintedRegion->lastColor, Color(50, 50, 50, 64));
    }
}
