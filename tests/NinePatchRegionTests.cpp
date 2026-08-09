// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/NinePatchRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/NinePatchRegion.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::TextureAtlases::NinePatchRegion;
    using Myra::Tests::DummyTextureBackend;
    using Myra::Tests::RecordingSpriteBatchBackend;

    struct RecordingContext
    {
        RecordingContext(const int textureWidth, const int textureHeight)
        {
            auto textureBackend =
                std::make_shared<DummyTextureBackend>(textureWidth, textureHeight);
            texture = std::make_shared<Texture2D>(
                Texture2D::CreateWithBackendForTests(
                    textureWidth, textureHeight, std::move(textureBackend)));

            auto spriteBackend = std::make_unique<RecordingSpriteBatchBackend>();
            recording = spriteBackend.get();
            context = std::make_unique<RenderContext>(
                std::make_unique<SpriteBatch>(std::move(spriteBackend)));
            context->setOpacityProperty(1.0F);
        }

        std::shared_ptr<Texture2D> texture;
        RecordingSpriteBatchBackend* recording = nullptr;
        std::unique_ptr<RenderContext> context;
    };

    void ExpectDraw(
        const RecordingSpriteBatchBackend::DrawCall& call,
        const Rectangle destination,
        const Rectangle source,
        const Color color)
    {
        EXPECT_EQ(call.destination, destination);
        EXPECT_EQ(call.source, source);
        EXPECT_EQ(call.color, color);
    }

    TEST(NinePatchRegionTests, DrawsAllNineSlicesInUpstreamOrder)
    {
        RecordingContext fixture(9, 9);
        const Thickness info(2, 3, 2, 1);
        const NinePatchRegion patch(fixture.texture, Rectangle(0, 0, 9, 9), info);
        const Color color(10, 20, 30, 40);

        EXPECT_EQ(patch.getInfoProperty(), info);
        fixture.context->Begin();
        patch.Draw(*fixture.context, Rectangle(10, 20, 20, 12), color);
        fixture.context->End();

        ASSERT_EQ(fixture.recording->draws.size(), 9U);
        const std::vector<Rectangle> expectedDestinations{
            {10, 20, 2, 3}, {12, 20, 16, 3}, {28, 20, 2, 3},
            {10, 23, 2, 8}, {12, 23, 16, 8}, {28, 23, 2, 8},
            {10, 31, 2, 1}, {12, 31, 16, 1}, {28, 31, 2, 1}};
        const std::vector<Rectangle> expectedSources{
            {0, 0, 2, 3}, {2, 0, 5, 3}, {7, 0, 2, 3},
            {0, 3, 2, 5}, {2, 3, 5, 5}, {7, 3, 2, 5},
            {0, 8, 2, 1}, {2, 8, 5, 1}, {7, 8, 2, 1}};

        for (std::size_t index = 0; index < expectedSources.size(); ++index)
        {
            ExpectDraw(fixture.recording->draws[index],
                expectedDestinations[index], expectedSources[index], color);
        }
    }

    TEST(NinePatchRegionTests, PreservesDegenerateDestinationCornerPositioning)
    {
        RecordingContext fixture(10, 10);
        const NinePatchRegion patch(
            fixture.texture, Rectangle(0, 0, 10, 10), Thickness(5));

        fixture.context->Begin();
        patch.Draw(*fixture.context, Rectangle(0, 0, 3, 3), Color::White);
        fixture.context->End();

        ASSERT_EQ(fixture.recording->draws.size(), 4U);
        ExpectDraw(fixture.recording->draws[0],
            Rectangle(0, 0, 3, 3), Rectangle(0, 0, 5, 5), Color::White);
        ExpectDraw(fixture.recording->draws[1],
            Rectangle(5, 0, 3, 3), Rectangle(5, 0, 5, 5), Color::White);
        ExpectDraw(fixture.recording->draws[2],
            Rectangle(0, 3, 3, 3), Rectangle(0, 5, 5, 5), Color::White);
        ExpectDraw(fixture.recording->draws[3],
            Rectangle(5, 3, 3, 3), Rectangle(5, 5, 5, 5), Color::White);
    }

    TEST(NinePatchRegionTests, ZeroThicknessProducesOnlyTheStretchableCenter)
    {
        RecordingContext fixture(1, 1);
        const NinePatchRegion patch(
            fixture.texture, Rectangle(0, 0, 1, 1), Thickness::Zero);

        fixture.context->Begin();
        patch.Draw(*fixture.context, Rectangle(7, 8, 11, 12), Color::Blue);
        fixture.context->End();

        ASSERT_EQ(fixture.recording->draws.size(), 1U);
        ExpectDraw(fixture.recording->draws.front(),
            Rectangle(7, 8, 11, 12), Rectangle(0, 0, 1, 1), Color::Blue);
    }

    TEST(NinePatchRegionTests, RetainsTheTextureAndRejectsOverflowingGeometry)
    {
        RecordingContext fixture(2, 1);
        std::weak_ptr<Texture2D> retained = fixture.texture;
        const NinePatchRegion patch(fixture.texture, Rectangle(0, 0, 2, 1), Thickness::Zero);
        fixture.texture.reset();
        EXPECT_FALSE(retained.expired());

        EXPECT_THROW(
            (void) NinePatchRegion(
                retained.lock(),
                Rectangle(std::numeric_limits<int>::max(), 0, 2, 1),
                Thickness(1, 0, 0, 0)),
            std::overflow_error);

        fixture.context->Begin();
        EXPECT_THROW(
            patch.Draw(
                *fixture.context,
                Rectangle(std::numeric_limits<int>::max(), 0, 2, 1),
                Color::White),
            std::overflow_error);
        fixture.context->End();
    }
}
