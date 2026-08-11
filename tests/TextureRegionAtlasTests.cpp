// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/TextureAtlases/NinePatchRegion.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::TextureAtlases::NinePatchRegion;
    using Myra::Graphics2D::TextureAtlases::TextureRegion;
    using Myra::Graphics2D::TextureAtlases::TextureRegionAtlas;
    using Myra::Tests::DummyTextureBackend;

    [[nodiscard]] std::shared_ptr<Texture2D> MakeTexture(
        const int width = 32, const int height = 32)
    {
        return std::make_shared<Texture2D>(Texture2D::CreateWithRendererForTests(
            width, height, std::make_shared<DummyTextureBackend>(width, height)));
    }

    TEST(TextureRegionAtlasTests, LoadsRegularAndNinePatchRegionsAndRetainsTheirTexture)
    {
        const std::string xml =
            "<TextureAtlas Image=\"atlas.png\">"
            "<TextureRegion Id=\"plain\" Left=\"1\" Top=\"2\" Width=\"3\" Height=\"4\"/>"
            "<NinePatchRegion Id=\"panel\" Left=\"5\" Top=\"6\" Width=\"9\" Height=\"10\" "
            "NinePatchLeft=\"1\" NinePatchTop=\"2\" NinePatchRight=\"3\" NinePatchBottom=\"4\"/>"
            "</TextureAtlas>";
        auto texture = MakeTexture();
        std::weak_ptr<Texture2D> retained = texture;
        int getterCalls = 0;
        std::string requestedImage;

        TextureRegionAtlas atlas = TextureRegionAtlas::FromXml(
            xml,
            [&](const std::string& image) {
                ++getterCalls;
                requestedImage = image;
                return texture;
            });

        EXPECT_EQ(getterCalls, 1);
        EXPECT_EQ(requestedImage, "atlas.png");
        EXPECT_EQ(atlas.getImageProperty(), std::optional<std::string>("atlas.png"));
        EXPECT_EQ(atlas.getTextureProperty(), texture);
        ASSERT_EQ(atlas.getRegionsProperty().size(), 2U);

        const auto plain = atlas.EnsureRegion("plain");
        EXPECT_EQ(plain->getNameProperty(), std::optional<std::string>("plain"));
        EXPECT_EQ(plain->getBoundsProperty(), Rectangle(1, 2, 3, 4));
        EXPECT_EQ(plain->getTextureProperty(), texture);

        const auto panel = std::dynamic_pointer_cast<NinePatchRegion>(
            atlas.getItemProperty("panel"));
        ASSERT_NE(panel, nullptr);
        EXPECT_EQ(panel->getBoundsProperty(), Rectangle(5, 6, 9, 10));
        EXPECT_EQ(panel->getInfoProperty(), Thickness(1, 2, 3, 4));

        texture.reset();
        EXPECT_FALSE(retained.expired());
        EXPECT_EQ(atlas.getTextureProperty(), retained.lock());
    }

    TEST(TextureRegionAtlasTests, SerializesRegionNamesAndRoundTripsAtlasGeometry)
    {
        auto texture = MakeTexture();
        TextureRegionAtlas atlas;
        atlas.setNameProperty("skin");
        atlas.setImageProperty("atlas&skin.png");
        auto plain = std::make_shared<TextureRegion>(texture, Rectangle(1, 2, 3, 4));
        plain->setNameProperty("plain&named");
        auto panel = std::make_shared<NinePatchRegion>(
            texture, Rectangle(5, 6, 9, 10), Thickness(1, 2, 3, 4));
        panel->setNameProperty("panel");
        atlas.setItemProperty("dictionary-key-is-not-the-id", plain);
        atlas.getRegionsProperty().emplace("panel", panel);

        EXPECT_EQ(atlas.ToString(), "skin");
        const std::string serialized = atlas.ToXml();
        TextureRegionAtlas roundTrip = TextureRegionAtlas::FromXml(
            serialized, [&](const std::string& image) {
                EXPECT_EQ(image, "atlas&skin.png");
                return texture;
            });

        EXPECT_EQ(roundTrip.getRegionsProperty().size(), 2U);
        EXPECT_EQ(roundTrip.EnsureRegion("plain&named")->getBoundsProperty(),
            Rectangle(1, 2, 3, 4));
        const auto roundTripPanel = std::dynamic_pointer_cast<NinePatchRegion>(
            roundTrip.EnsureRegion("panel"));
        ASSERT_NE(roundTripPanel, nullptr);
        EXPECT_EQ(roundTripPanel->getInfoProperty(), Thickness(1, 2, 3, 4));
    }

    TEST(TextureRegionAtlasTests, SplitsOnlySeparatorsFollowedByLettersOrDigits)
    {
        std::optional<std::string> region = "stale";
        std::string asset = " atlas.xmat :button ";
        EXPECT_TRUE(TextureRegionAtlas::TryGetRegionName(asset, region));
        EXPECT_EQ(asset, "atlas.xmat");
        EXPECT_EQ(region, std::optional<std::string>("button"));

        asset = "C:\\skins\\atlas.xmat:";
        EXPECT_FALSE(TextureRegionAtlas::TryGetRegionName(asset, region));
        EXPECT_EQ(asset, "C:\\skins\\atlas.xmat:");
        EXPECT_EQ(region, std::nullopt);

        asset = "atlas.xmat:_private";
        EXPECT_FALSE(TextureRegionAtlas::TryGetRegionName(asset, region));

        asset = "atlas.xmat:7segment";
        EXPECT_TRUE(TextureRegionAtlas::TryGetRegionName(asset, region));
        EXPECT_EQ(asset, "atlas.xmat");
        EXPECT_EQ(region, std::optional<std::string>("7segment"));
    }

    TEST(TextureRegionAtlasTests, OverwritesDuplicateIdsLikeTheUpstreamIndexer)
    {
        const std::string xml =
            "<TextureAtlas Image=\"atlas.png\">"
            "<TextureRegion Id=\"same\" Left=\"0\" Top=\"0\" Width=\"1\" Height=\"1\"/>"
            "<OtherRegion Id=\"same\" Left=\"2\" Top=\"3\" Width=\"4\" Height=\"5\"/>"
            "</TextureAtlas>";
        auto texture = MakeTexture();

        const TextureRegionAtlas atlas = TextureRegionAtlas::FromXml(
            xml, [&](const std::string&) { return texture; });

        ASSERT_EQ(atlas.getRegionsProperty().size(), 1U);
        EXPECT_EQ(atlas.EnsureRegion("same")->getBoundsProperty(), Rectangle(2, 3, 4, 5));
    }

    TEST(TextureRegionAtlasTests, RejectsMissingRegionsNullHandlesAndMalformedAttributes)
    {
        TextureRegionAtlas atlas;
        EXPECT_THROW((void) atlas.EnsureRegion("missing"), std::out_of_range);
        EXPECT_THROW(
            atlas.setItemProperty("null", std::shared_ptr<TextureRegion>{}),
            std::invalid_argument);
        atlas.getRegionsProperty()["null"] = nullptr;
        EXPECT_THROW((void) atlas.ToXml(), std::invalid_argument);

        const auto getter = [](const std::string&) { return MakeTexture(); };
        EXPECT_THROW(
            (void) TextureRegionAtlas::FromXml("<TextureAtlas/>", getter),
            std::invalid_argument);
        EXPECT_THROW(
            (void) TextureRegionAtlas::FromXml(
                "<TextureAtlas Image=\"x\"><TextureRegion Id=\"bad\" Left=\"not-int\" "
                "Top=\"0\" Width=\"1\" Height=\"1\"/></TextureAtlas>", getter),
            std::invalid_argument);
        EXPECT_THROW(
            (void) TextureRegionAtlas::FromXml(
                "<TextureAtlas Image=\"x\"/>", TextureRegionAtlas::TextureGetter{}),
            std::invalid_argument);
        EXPECT_THROW(
            (void) TextureRegionAtlas::FromXml(
                "<TextureAtlas Image=\"x\"/>",
                [](const std::string&) { return std::shared_ptr<Texture2D>{}; }),
            std::invalid_argument);
        EXPECT_ANY_THROW((void) TextureRegionAtlas::FromXml("<TextureAtlas", getter));
    }
}
