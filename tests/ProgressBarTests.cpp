// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Range/HorizontalProgressBar.hpp"
#include "Myra/Graphics2D/UI/Range/VerticalProgressBar.hpp"

#include <gtest/gtest.h>

#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Range/ProgressBar.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalProgressBar;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::ProgressBar;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalProgressBar;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    struct BrushDraw
    {
        Rectangle destination;
        Color color;
    };

    class RecordingBrush final : public IBrush
    {
    public:
        explicit RecordingBrush(std::vector<BrushDraw>& draws) : draws_(draws) {}

        void Draw(RenderContext&, const Rectangle destination, const Color color) const override
        {
            draws_.push_back({destination, color});
        }

    private:
        std::vector<BrushDraw>& draws_;
    };

    class SelfClearingBrush final : public IBrush
    {
    public:
        SelfClearingBrush(ProgressBar& owner, bool& drawCompleted, int& destroyed)
            : owner_(owner), drawCompleted_(drawCompleted), destroyed_(destroyed)
        {
        }

        ~SelfClearingBrush() override { ++destroyed_; }

        void Draw(RenderContext&, Rectangle, Color) const override
        {
            owner_.setFillerProperty(nullptr);
            drawCompleted_ = true;
        }

    private:
        ProgressBar& owner_;
        bool& drawCompleted_;
        int& destroyed_;
    };

    TEST(ProgressBarTests, ConcreteDefaultsAndOrientationsMatchTheSelectedUpstream)
    {
        HorizontalProgressBar horizontal;
        EXPECT_EQ(horizontal.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(horizontal.getMinimumProperty(), 0.0F);
        EXPECT_EQ(horizontal.getMaximumProperty(), 100.0F);
        EXPECT_EQ(horizontal.getValueProperty(), 0.0F);
        EXPECT_EQ(horizontal.getFillerProperty(), nullptr);
        EXPECT_EQ(horizontal.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(horizontal.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        EXPECT_EQ(horizontal.Measure(Point(100, 80)), Point(0, 0));

        VerticalProgressBar vertical;
        EXPECT_EQ(vertical.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(vertical.getHorizontalAlignmentProperty(), HorizontalAlignment::Left);
        EXPECT_EQ(vertical.getVerticalAlignmentProperty(), VerticalAlignment::Stretch);
        EXPECT_EQ(vertical.Measure(Point(100, 80)), Point(0, 0));
    }

    TEST(ProgressBarTests, ValueUsesUpstreamEpsilonAndRaisesTheNonGenericEvent)
    {
        HorizontalProgressBar progressBar;
        int changedCount = 0;
        void* sender = nullptr;
        InputEventType eventType = InputEventType::None;
        progressBar.ValueChanged +=
            [&](void* receivedSender, Myra::Events::MyraEventArgs& arguments) {
                ++changedCount;
                sender = receivedSender;
                eventType = arguments.getEventTypeProperty();
            };

        progressBar.setValueProperty(0.5e-6F);
        EXPECT_EQ(changedCount, 0);
        EXPECT_EQ(progressBar.getValueProperty(), 0.0F);

        progressBar.setValueProperty(2.0e-6F);
        EXPECT_EQ(changedCount, 1);
        progressBar.setValueProperty(2.4e-6F);
        EXPECT_EQ(changedCount, 1);
        EXPECT_EQ(progressBar.getValueProperty(), 2.0e-6F);
        EXPECT_EQ(sender, &progressBar);
        EXPECT_EQ(eventType, InputEventType::ValueChanged);

        const float nan = std::numeric_limits<float>::quiet_NaN();
        progressBar.setValueProperty(nan);
        progressBar.setValueProperty(nan);
        EXPECT_EQ(changedCount, 3);
        EXPECT_TRUE(std::isnan(progressBar.getValueProperty()));
    }

    TEST(ProgressBarTests, HorizontalRenderingClampsAndTruncatesTheFilledExtent)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        std::vector<BrushDraw> draws;
        auto filler = std::make_shared<RecordingBrush>(draws);

        HorizontalProgressBar progressBar;
        progressBar.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        progressBar.setFillerProperty(filler);
        progressBar.setMinimumProperty(10.0F);
        progressBar.setMaximumProperty(30.0F);
        progressBar.setValueProperty(15.0F);
        progressBar.Arrange(Rectangle(0, 0, 101, 20));
        progressBar.InternalRender(context);
        ASSERT_EQ(draws.size(), 1U);
        EXPECT_EQ(draws.back().destination, Rectangle(0, 0, 25, 20));
        EXPECT_EQ(draws.back().color, Color::White);

        progressBar.setValueProperty(-50.0F);
        progressBar.InternalRender(context);
        EXPECT_EQ(draws.size(), 1U);

        progressBar.setValueProperty(50.0F);
        progressBar.InternalRender(context);
        ASSERT_EQ(draws.size(), 2U);
        EXPECT_EQ(draws.back().destination, Rectangle(0, 0, 101, 20));

        progressBar.setMinimumProperty(30.0F);
        progressBar.setMaximumProperty(10.0F);
        progressBar.setValueProperty(20.0F);
        progressBar.InternalRender(context);
        ASSERT_EQ(draws.size(), 3U);
        EXPECT_EQ(draws.back().destination, Rectangle(0, 0, 101, 20));

        progressBar.setMinimumProperty(10.0F);
        progressBar.setMaximumProperty(10.0F);
        progressBar.InternalRender(context);
        EXPECT_EQ(draws.size(), 3U);
    }

    TEST(ProgressBarTests, VerticalRenderingPreservesTheSelectedTopDownGeometry)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        std::vector<BrushDraw> draws;

        VerticalProgressBar progressBar;
        progressBar.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        progressBar.setPaddingProperty(Thickness(1, 2, 3, 4));
        progressBar.setFillerProperty(std::make_shared<RecordingBrush>(draws));
        progressBar.setMinimumProperty(0.0F);
        progressBar.setMaximumProperty(10.0F);
        progressBar.setValueProperty(3.0F);
        progressBar.Arrange(Rectangle(0, 0, 10, 30));
        progressBar.InternalRender(context);

        ASSERT_EQ(draws.size(), 1U);
        EXPECT_EQ(draws.back().destination, Rectangle(1, 2, 6, 7));
        EXPECT_EQ(draws.back().color, Color::White);
    }

    TEST(ProgressBarTests, RejectsUndefinedFillConversionsDeterministically)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        std::vector<BrushDraw> draws;
        auto filler = std::make_shared<RecordingBrush>(draws);

        HorizontalProgressBar nonFinite;
        nonFinite.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        nonFinite.setFillerProperty(filler);
        nonFinite.setValueProperty(std::numeric_limits<float>::quiet_NaN());
        nonFinite.Arrange(Rectangle(0, 0, 20, 10));
        EXPECT_THROW(nonFinite.InternalRender(context), std::invalid_argument);

        nonFinite.setValueProperty(std::numeric_limits<float>::infinity());
        EXPECT_NO_THROW(nonFinite.InternalRender(context));
        ASSERT_EQ(draws.size(), 1U);
        EXPECT_EQ(draws.back().destination, Rectangle(0, 0, 20, 10));

        HorizontalProgressBar overflow;
        overflow.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        overflow.setFillerProperty(filler);
        overflow.setValueProperty(100.0F);
        overflow.Arrange(Rectangle(0, 0, std::numeric_limits<int>::max(), 1));
        EXPECT_THROW(overflow.InternalRender(context), std::overflow_error);
    }

    TEST(ProgressBarTests, RetainsTheFillerAcrossReentrantPropertyMutation)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        HorizontalProgressBar progressBar;
        progressBar.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        progressBar.setValueProperty(100.0F);
        progressBar.Arrange(Rectangle(0, 0, 10, 10));

        bool drawCompleted = false;
        int destroyed = 0;
        auto filler = std::make_shared<SelfClearingBrush>(
            progressBar, drawCompleted, destroyed);
        const std::weak_ptr<SelfClearingBrush> weakFiller = filler;
        progressBar.setFillerProperty(filler);
        filler.reset();

        progressBar.InternalRender(context);

        EXPECT_TRUE(drawCompleted);
        EXPECT_TRUE(weakFiller.expired());
        EXPECT_EQ(destroyed, 1);
        EXPECT_EQ(progressBar.getFillerProperty(), nullptr);
    }

    TEST(ProgressBarTests, ClonePreservesExactConcreteTypeAndSharesTheFiller)
    {
        std::vector<BrushDraw> draws;
        auto filler = std::make_shared<RecordingBrush>(draws);
        HorizontalProgressBar horizontal;
        horizontal.setMinimumProperty(-5.0F);
        horizontal.setMaximumProperty(7.5F);
        horizontal.setValueProperty(2.5F);
        horizontal.setFillerProperty(filler);
        horizontal.setVerticalAlignmentProperty(VerticalAlignment::Bottom);

        const std::shared_ptr<HorizontalProgressBar> horizontalClone =
            std::dynamic_pointer_cast<HorizontalProgressBar>(horizontal.Clone());
        ASSERT_NE(horizontalClone, nullptr);
        EXPECT_EQ(horizontalClone->getMinimumProperty(), -5.0F);
        EXPECT_EQ(horizontalClone->getMaximumProperty(), 7.5F);
        EXPECT_EQ(horizontalClone->getValueProperty(), 2.5F);
        EXPECT_EQ(horizontalClone->getFillerProperty(), filler);
        EXPECT_EQ(horizontalClone->getVerticalAlignmentProperty(), VerticalAlignment::Bottom);
        EXPECT_EQ(horizontalClone->getOrientationProperty(), Orientation::Horizontal);

        VerticalProgressBar vertical;
        vertical.setValueProperty(25.0F);
        const std::shared_ptr<VerticalProgressBar> verticalClone =
            std::dynamic_pointer_cast<VerticalProgressBar>(vertical.Clone());
        ASSERT_NE(verticalClone, nullptr);
        EXPECT_EQ(verticalClone->getValueProperty(), 25.0F);
        EXPECT_EQ(verticalClone->getOrientationProperty(), Orientation::Vertical);
    }

    TEST(ProgressBarTests, RegistersAndRoundTripsExternalFillerMmlMetadata)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* progressBar = registry.FindByType(typeid(ProgressBar));
        const TypeDescriptor* horizontal = registry.FindByType(typeid(HorizontalProgressBar));
        const TypeDescriptor* vertical = registry.FindByType(typeid(VerticalProgressBar));
        ASSERT_NE(progressBar, nullptr);
        ASSERT_NE(horizontal, nullptr);
        ASSERT_NE(vertical, nullptr);
        EXPECT_FALSE(progressBar->getCanCreateProperty());
        EXPECT_TRUE(horizontal->getCanCreateProperty());
        EXPECT_TRUE(vertical->getCanCreateProperty());

        const PropertyDescriptor* orientation =
            registry.FindPropertyByName(typeid(HorizontalProgressBar), "Orientation");
        const PropertyDescriptor* fillerProperty =
            registry.FindPropertyByName(typeid(HorizontalProgressBar), "Filler");
        ASSERT_NE(orientation, nullptr);
        ASSERT_NE(fillerProperty, nullptr);
        EXPECT_TRUE(orientation->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(fillerProperty->getMetadataProperty().ExternalAsset);
        EXPECT_FALSE(fillerProperty->getMetadataProperty().StylePropertyPath.has_value());
        EXPECT_TRUE(fillerProperty->getCanBeNullProperty());

        const PropertyDescriptor* horizontalAlignment = registry.FindPropertyByName(
            typeid(HorizontalProgressBar), "HorizontalAlignment");
        const PropertyDescriptor* verticalAlignment = registry.FindPropertyByName(
            typeid(VerticalProgressBar), "VerticalAlignment");
        ASSERT_NE(horizontalAlignment, nullptr);
        ASSERT_NE(verticalAlignment, nullptr);
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(
                      *horizontalAlignment->getDefaultValueProperty()),
            HorizontalAlignment::Stretch);
        EXPECT_EQ(std::any_cast<VerticalAlignment>(
                      *verticalAlignment->getDefaultValueProperty()),
            VerticalAlignment::Stretch);

        std::vector<BrushDraw> draws;
        const std::shared_ptr<IBrush> filler = std::make_shared<RecordingBrush>(draws);
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        loader.LoadExternalAsset =
            [&](const PropertyDescriptor& property, const std::string& assetName) -> std::any {
                EXPECT_EQ(&property, fillerProperty);
                EXPECT_EQ(assetName, "atlas:progress");
                return filler;
            };

        System::Xml::XmlDocument document;
        document.LoadXml(
            "<HorizontalProgressBar Minimum=\"10\" Maximum=\"30\" Value=\"15\" "
            "Filler=\"atlas:progress\" />");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(HorizontalProgressBar));
        const auto* loadedHorizontal =
            static_cast<const HorizontalProgressBar*>(loaded.Value.get());
        EXPECT_EQ(loadedHorizontal->getMinimumProperty(), 10.0F);
        EXPECT_EQ(loadedHorizontal->getMaximumProperty(), 30.0F);
        EXPECT_EQ(loadedHorizontal->getValueProperty(), 15.0F);
        EXPECT_EQ(loadedHorizontal->getFillerProperty(), filler);

        SaveContext saver(registry, codecs);
        saver.SaveExternalAsset =
            [&](const PropertyDescriptor& property, const std::any& value) {
                EXPECT_EQ(&property, fillerProperty);
                EXPECT_EQ(std::any_cast<const std::shared_ptr<IBrush>&>(value), filler);
                return std::string("atlas:progress");
            };
        const std::string xml =
            saver.ToXml(loadedHorizontal, typeid(HorizontalProgressBar));
        EXPECT_NE(xml.find("Minimum=\"10\""), std::string::npos);
        EXPECT_NE(xml.find("Maximum=\"30\""), std::string::npos);
        EXPECT_NE(xml.find("Value=\"15\""), std::string::npos);
        EXPECT_NE(xml.find("Filler=\"atlas:progress\""), std::string::npos);
        EXPECT_EQ(xml.find("HorizontalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("Orientation="), std::string::npos);
    }
}
