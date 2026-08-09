// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/DefaultAssets.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/DefaultAssets.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Myra::DefaultAssets;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::TextureAtlases::TextureRegion;

    class DefaultAssetsTests : public testing::Test
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

    TEST_F(DefaultAssetsTests, RequiresAConfiguredLiveGraphicsDevice)
    {
        EXPECT_THROW((void) DefaultAssets::getWhiteRegionProperty(), std::logic_error);
    }

    TEST_F(DefaultAssetsTests, CreatesAndCachesAnOpaqueWhitePixel)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);

        const auto first = DefaultAssets::getWhiteRegionProperty();
        const auto second = DefaultAssets::getWhiteRegionProperty();

        ASSERT_NE(first, nullptr);
        EXPECT_EQ(first, second);
        EXPECT_EQ(first->getSizeProperty(), Point(1, 1));
        const auto texture = first->getTextureProperty();
        ASSERT_NE(texture, nullptr);
        Color pixel = Color::Transparent;
        texture->GetData(&pixel, 1);
        EXPECT_EQ(pixel, Color::White);
    }

    TEST_F(DefaultAssetsTests, RebuildsAfterExplicitDisposalOrTextureDisposal)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        const auto first = DefaultAssets::getWhiteRegionProperty();

        DefaultAssets::Dispose();
        const auto second = DefaultAssets::getWhiteRegionProperty();
        EXPECT_NE(first, second);

        second->getTextureProperty()->Dispose();
        const auto third = DefaultAssets::getWhiteRegionProperty();
        EXPECT_NE(second, third);
        EXPECT_FALSE(third->getTextureProperty()->getIsDisposedProperty());
    }

    TEST_F(DefaultAssetsTests, EnvironmentCleanupReleasesTheCacheAndSelectsTheNewDevice)
    {
        Game firstGame;
        Game secondGame;
        MyraEnvironment::setGameProperty(firstGame);
        auto first = DefaultAssets::getWhiteRegionProperty();
        std::weak_ptr<TextureRegion> released = first;
        first.reset();

        MyraEnvironment::setGameProperty(secondGame);
        EXPECT_TRUE(released.expired());
        const auto second = DefaultAssets::getWhiteRegionProperty();
        EXPECT_EQ(
            second->getTextureProperty()->getGraphicsDeviceProperty(),
            &secondGame.getGraphicsDeviceProperty());

        MyraEnvironment::ClearGame();
        EXPECT_THROW((void) DefaultAssets::getWhiteRegionProperty(), std::logic_error);
    }

    TEST_F(DefaultAssetsTests, NormalGameDestructionReleasesTheCachedRegion)
    {
        std::weak_ptr<TextureRegion> released;
        {
            Game game;
            MyraEnvironment::setGameProperty(game);
            auto region = DefaultAssets::getWhiteRegionProperty();
            released = region;
        }

        EXPECT_TRUE(released.expired());
        EXPECT_THROW((void) MyraEnvironment::getGameProperty(), std::logic_error);
    }
}
