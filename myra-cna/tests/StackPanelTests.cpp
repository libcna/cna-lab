// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <stdexcept>
#include <string>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::HorizontalStackPanel;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::StackPanel;
    using Myra::Graphics2D::UI::VerticalStackPanel;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;
    using Myra::Tests::RecordingSpriteBatchBackend;

    class FixedWidget final : public Widget
    {
      public:
        explicit FixedWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

      protected:
        [[nodiscard]] Point InternalMeasure(Point) override { return desiredSize_; }

      private:
        Point desiredSize_;
    };

    TEST(StackPanelTests, HorizontalLayoutUsesSpacingAndAutoChildSizes)
    {
        HorizontalStackPanel panel;
        panel.setSpacingProperty(5);
        auto first = std::make_shared<FixedWidget>(Point(20, 10));
        auto second = std::make_shared<FixedWidget>(Point(30, 15));
        panel.AddWidget(first);
        panel.AddWidget(second);

        EXPECT_EQ(panel.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(panel.Measure(Point(200, 100)), Point(55, 15));
        panel.Arrange(Rectangle(0, 0, 100, 40));

        EXPECT_EQ(panel.GetCellSize(0), 20);
        EXPECT_EQ(panel.GetCellSize(1), 30);
        EXPECT_EQ(first->getContainerBoundsProperty(), Rectangle(0, 0, 20, 40));
        EXPECT_EQ(second->getContainerBoundsProperty(), Rectangle(25, 0, 30, 40));
    }

    TEST(StackPanelTests, AttachedProportionsDivideTheAvailableStackAxis)
    {
        VerticalStackPanel panel;
        auto first = std::make_shared<FixedWidget>(Point(10, 10));
        auto second = std::make_shared<FixedWidget>(Point(10, 10));
        StackPanel::SetProportionType(*first, ProportionType::Part);
        StackPanel::SetProportionValue(*first, 1.0F);
        StackPanel::SetProportionType(*second, ProportionType::Part);
        StackPanel::SetProportionValue(*second, 2.0F);
        panel.AddWidget(first);
        panel.AddWidget(second);

        panel.Arrange(Rectangle(0, 0, 40, 90));
        EXPECT_EQ(panel.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(panel.GetCellSize(0), 30);
        EXPECT_EQ(panel.GetCellSize(1), 60);
        EXPECT_EQ(first->getContainerBoundsProperty(), Rectangle(0, 0, 40, 30));
        EXPECT_EQ(second->getContainerBoundsProperty(), Rectangle(0, 30, 40, 60));
    }

    TEST(StackPanelTests, UsesObservableExplicitProportionsDuringLayout)
    {
        HorizontalStackPanel panel;
        auto first = std::make_shared<FixedWidget>(Point(10, 10));
        auto second = std::make_shared<FixedWidget>(Point(10, 10));
        panel.AddWidget(first);
        panel.AddWidget(second);

        auto proportion = std::make_shared<Myra::Graphics2D::UI::Proportion>(ProportionType::Pixels, 25.0F);
        auto &proportions = panel.getProportionsProperty();
        proportions.Add(proportion);
        EXPECT_EQ(proportions.getCountProperty(), 1);
        EXPECT_EQ(proportions[0], proportion);

        panel.Arrange(Rectangle(0, 0, 100, 20));
        EXPECT_EQ(panel.GetCellSize(0), 25);
        EXPECT_EQ(panel.GetCellSize(1), 10);
    }

    TEST(StackPanelTests, SupportsNullDefaultButRejectsNullExplicitProportions)
    {
        HorizontalStackPanel panel;
        auto child = std::make_shared<FixedWidget>(Point(10, 10));
        panel.AddWidget(child);
        panel.setDefaultProportionProperty(nullptr);

        EXPECT_NO_THROW(static_cast<void>(panel.Measure(Point(100, 20))));
        panel.getProportionsProperty().Add(nullptr);
        EXPECT_THROW(static_cast<void>(panel.Measure(Point(101, 20))), std::logic_error);
    }

    TEST(StackPanelTests, DefaultsCloneAndMmlPreserveTheDependencySafeDebugState)
    {
        HorizontalStackPanel panel;
        EXPECT_FALSE(panel.getShowGridLinesProperty());
        EXPECT_EQ(panel.getGridLinesColorProperty(), Color::White);

        const Color lineColor(12, 34, 56, 78);
        panel.setShowGridLinesProperty(true);
        panel.setGridLinesColorProperty(lineColor);
        const auto clone = std::dynamic_pointer_cast<HorizontalStackPanel>(panel.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getShowGridLinesProperty());
        EXPECT_EQ(clone->getGridLinesColorProperty(), lineColor);

        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const PropertyDescriptor *show = registry.FindPropertyByName(typeid(HorizontalStackPanel), "ShowGridLines");
        ASSERT_NE(show, nullptr);
        ASSERT_TRUE(show->getDefaultValueProperty().has_value());
        EXPECT_FALSE(std::any_cast<bool>(*show->getDefaultValueProperty()));
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalStackPanel), "GridLinesColor"), nullptr);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<HorizontalStackPanel ShowGridLines=\"True\" />");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(HorizontalStackPanel));
        const auto *loadedPanel = static_cast<const HorizontalStackPanel *>(loaded.Value.get());
        EXPECT_TRUE(loadedPanel->getShowGridLinesProperty());

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedPanel, typeid(HorizontalStackPanel));
        EXPECT_NE(xml.find("ShowGridLines=\"True\""), std::string::npos);
        EXPECT_EQ(xml.find("GridLinesColor="), std::string::npos);
    }

    TEST(StackPanelTests, RendersHorizontalAndVerticalDebugLinesAfterChildren)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend *const recording = backend.get();
        RenderContext context(std::make_unique<SpriteBatch>(std::move(backend)), &game.getGraphicsDeviceProperty());
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        context.setOpacityProperty(1.0F);
        context.Begin();

        HorizontalStackPanel horizontal;
        horizontal.setSpacingProperty(5);
        horizontal.setShowGridLinesProperty(true);
        horizontal.setGridLinesColorProperty(Color::Red);
        horizontal.AddWidget(std::make_shared<FixedWidget>(Point(20, 10)));
        horizontal.AddWidget(std::make_shared<FixedWidget>(Point(30, 15)));
        static_cast<void>(horizontal.Measure(Point(100, 40)));
        horizontal.Arrange(Rectangle(0, 0, 100, 40));
        static_cast<Widget &>(horizontal).InternalRender(context);

        VerticalStackPanel vertical;
        vertical.setSpacingProperty(4);
        vertical.setShowGridLinesProperty(true);
        vertical.setGridLinesColorProperty(Color::Blue);
        vertical.AddWidget(std::make_shared<FixedWidget>(Point(10, 10)));
        vertical.AddWidget(std::make_shared<FixedWidget>(Point(10, 10)));
        static_cast<void>(vertical.Measure(Point(40, 90)));
        vertical.Arrange(Rectangle(0, 0, 40, 90));
        static_cast<Widget &>(vertical).InternalRender(context);
        context.End();

        ASSERT_EQ(recording->draws.size(), 2U);
        EXPECT_EQ(recording->draws[0].destination,
                  Rectangle(horizontal.GetCellSize(0) + horizontal.getSpacingProperty() / 2, 0, 1, 40));
        EXPECT_EQ(recording->draws[0].color, Color::Red);
        EXPECT_EQ(recording->draws[1].destination,
                  Rectangle(0, vertical.GetCellSize(0) + vertical.getSpacingProperty() / 2, 40, 1));
        EXPECT_EQ(recording->draws[1].color, Color::Blue);

        MyraEnvironment::ClearGame();
    }
} // namespace
