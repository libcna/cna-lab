// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

#include <gtest/gtest.h>

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/MouseInfo.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
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
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::GridSelectionMode;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Proportion;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;
    using Myra::Tests::RecordingSpriteBatchBackend;

    struct BrushDraw
    {
        std::string name;
        Rectangle destination;
        Color color;
    };

    class RecordingBrush final : public IBrush
    {
      public:
        RecordingBrush(std::string name, std::vector<BrushDraw> &draws) : name_(std::move(name)), draws_(draws) {}

        void Draw(RenderContext &, const Rectangle destination, const Color color) const override
        {
            draws_.push_back({name_, destination, color});
        }

      private:
        std::string name_;
        std::vector<BrushDraw> &draws_;
    };

    class CallbackBrush final : public IBrush
    {
      public:
        explicit CallbackBrush(std::function<void()> callback) : callback_(std::move(callback)) {}

        void Draw(RenderContext &, Rectangle, Color) const override { callback_(); }

      private:
        std::function<void()> callback_;
    };

    class GridTests : public testing::Test
    {
      protected:
        void SetUp() override
        {
            oldMouseInfoGetter_ = MyraEnvironment::getMouseInfoGetterProperty();
            oldDownKeysGetter_ = MyraEnvironment::getDownKeysGetterProperty();
        }

        void TearDown() override
        {
            InputEventsManager::ProcessEvents();
            MyraEnvironment::setMouseInfoGetterProperty(std::move(oldMouseInfoGetter_));
            MyraEnvironment::setDownKeysGetterProperty(std::move(oldDownKeysGetter_));
            MyraEnvironment::ClearGame();
        }

        static void ArrangeTwoByTwo(Grid &grid)
        {
            for (int row = 0; row < 2; ++row)
            {
                for (int column = 0; column < 2; ++column)
                {
                    auto child = std::make_shared<Widget>();
                    Grid::SetColumn(*child, column);
                    Grid::SetRow(*child, row);
                    grid.AddWidget(std::move(child));
                }
            }
            static_cast<void>(grid.Measure(Point(100, 60)));
            grid.Arrange(Rectangle(0, 0, 100, 60));
        }

      private:
        MyraEnvironment::MouseInfoGetter oldMouseInfoGetter_;
        MyraEnvironment::DownKeysGetter oldDownKeysGetter_;
    };

    TEST_F(GridTests, DefaultsEventsAndClonePreserveThePinnedConfigurationBoundary)
    {
        Grid grid;
        EXPECT_FALSE(grid.getShowGridLinesProperty());
        EXPECT_EQ(grid.getGridLinesColorProperty(), Color::White);
        EXPECT_EQ(grid.getSelectionBackgroundProperty(), nullptr);
        EXPECT_EQ(grid.getSelectionHoverBackgroundProperty(), nullptr);
        EXPECT_EQ(grid.getGridSelectionModeProperty(), GridSelectionMode::None);
        EXPECT_TRUE(grid.getHoverIndexCanBeNullProperty());
        EXPECT_FALSE(grid.getCanSelectNothingProperty());
        EXPECT_FALSE(grid.getHoverRowIndexProperty().has_value());
        EXPECT_FALSE(grid.getHoverColumnIndexProperty().has_value());
        EXPECT_FALSE(grid.getSelectedRowIndexProperty().has_value());
        EXPECT_FALSE(grid.getSelectedColumnIndexProperty().has_value());

        int hoverEvents = 0;
        int selectedEvents = 0;
        grid.HoverIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &grid);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::HoverIndexChanged);
            ++hoverEvents;
        };
        grid.SelectedIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &grid);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectedIndexChanged);
            ++selectedEvents;
        };
        grid.setHoverRowIndexProperty(1);
        grid.setHoverRowIndexProperty(1);
        grid.setHoverColumnIndexProperty(2);
        grid.setSelectedRowIndexProperty(3);
        grid.setSelectedRowIndexProperty(3);
        grid.setSelectedColumnIndexProperty(4);
        EXPECT_EQ(hoverEvents, 2);
        EXPECT_EQ(selectedEvents, 2);

        grid.getColumnsProportionsProperty().Add(std::make_shared<Proportion>(ProportionType::Part, 1.0F));
        EXPECT_FALSE(grid.getHoverRowIndexProperty().has_value());
        EXPECT_EQ(grid.getHoverColumnIndexProperty(), std::optional<int>(2));
        EXPECT_FALSE(grid.getSelectedRowIndexProperty().has_value());
        EXPECT_EQ(grid.getSelectedColumnIndexProperty(), std::optional<int>(4));
        EXPECT_EQ(hoverEvents, 3);
        EXPECT_EQ(selectedEvents, 3);

        std::vector<BrushDraw> draws;
        const auto selection = std::make_shared<RecordingBrush>("selection", draws);
        const auto hover = std::make_shared<RecordingBrush>("hover", draws);
        const Color lineColor(12, 34, 56, 78);
        grid.setShowGridLinesProperty(true);
        grid.setGridLinesColorProperty(lineColor);
        grid.setColumnSpacingProperty(5);
        grid.setRowSpacingProperty(3);
        grid.setSelectionBackgroundProperty(selection);
        grid.setSelectionHoverBackgroundProperty(hover);
        grid.setGridSelectionModeProperty(GridSelectionMode::Cell);
        grid.setHoverIndexCanBeNullProperty(false);
        grid.setCanSelectNothingProperty(true);

        const auto clone = std::dynamic_pointer_cast<Grid>(grid.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getShowGridLinesProperty());
        EXPECT_EQ(clone->getGridLinesColorProperty(), lineColor);
        EXPECT_EQ(clone->getColumnSpacingProperty(), 5);
        EXPECT_EQ(clone->getRowSpacingProperty(), 3);
        EXPECT_EQ(clone->getSelectionBackgroundProperty(), selection);
        EXPECT_EQ(clone->getSelectionHoverBackgroundProperty(), hover);
        EXPECT_EQ(clone->getGridSelectionModeProperty(), GridSelectionMode::Cell);
        EXPECT_FALSE(clone->getHoverIndexCanBeNullProperty());
        EXPECT_TRUE(clone->getCanSelectNothingProperty());
        EXPECT_EQ(clone->getColumnsProportionsProperty().getCountProperty(), 1);
        EXPECT_FALSE(clone->getHoverRowIndexProperty().has_value());
        EXPECT_FALSE(clone->getHoverColumnIndexProperty().has_value());
        EXPECT_FALSE(clone->getSelectedRowIndexProperty().has_value());
        EXPECT_FALSE(clone->getSelectedColumnIndexProperty().has_value());
    }

    TEST_F(GridTests, PointerHoverAndTouchSelectionFollowRowColumnAndNullabilityRules)
    {
        MouseInfo snapshot{{0, 0}, false, false, false, 0.0F};
        MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        MyraEnvironment::setDownKeysGetterProperty([](MyraEnvironment::DownKeys &keys) { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 60); });
        auto grid = std::make_shared<Grid>();
        grid->setColumnSpacingProperty(4);
        grid->setRowSpacingProperty(2);
        grid->setGridSelectionModeProperty(GridSelectionMode::Cell);
        for (int row = 0; row < 2; ++row)
        {
            for (int column = 0; column < 2; ++column)
            {
                auto child = std::make_shared<Widget>();
                Grid::SetColumn(*child, column);
                Grid::SetRow(*child, row);
                grid->AddWidget(std::move(child));
            }
        }
        desktop.AddWidget(grid);
        desktop.UpdateLayout();

        const Rectangle bounds = grid->getActualBoundsProperty();
        const Point localTarget(bounds.X + grid->GetCellLocationX(1) + 1, bounds.Y + grid->GetCellLocationY(1) + 1);
        const Point target = grid->ToGlobal(localTarget);
        snapshot.Position = target;
        desktop.UpdateInput();
        grid->OnMouseEntered();
        EXPECT_EQ(grid->getHoverRowIndexProperty(), std::optional<int>(1));
        EXPECT_EQ(grid->getHoverColumnIndexProperty(), std::optional<int>(1));

        snapshot.IsLeftButtonDown = true;
        desktop.UpdateInput();
        grid->OnTouchDown();
        EXPECT_EQ(grid->getSelectedRowIndexProperty(), std::optional<int>(1));
        EXPECT_EQ(grid->getSelectedColumnIndexProperty(), std::optional<int>(1));

        grid->OnTouchDown();
        EXPECT_EQ(grid->getSelectedRowIndexProperty(), std::optional<int>(1));
        EXPECT_EQ(grid->getSelectedColumnIndexProperty(), std::optional<int>(1));
        grid->setCanSelectNothingProperty(true);
        grid->OnTouchDown();
        EXPECT_FALSE(grid->getSelectedRowIndexProperty().has_value());
        EXPECT_FALSE(grid->getSelectedColumnIndexProperty().has_value());

        grid->OnMouseLeft();
        EXPECT_FALSE(grid->getHoverRowIndexProperty().has_value());
        EXPECT_FALSE(grid->getHoverColumnIndexProperty().has_value());
        grid->setHoverRowIndexProperty(0);
        grid->setHoverColumnIndexProperty(0);
        grid->setHoverIndexCanBeNullProperty(false);
        grid->OnMouseLeft();
        EXPECT_EQ(grid->getHoverRowIndexProperty(), std::optional<int>(0));
        EXPECT_EQ(grid->getHoverColumnIndexProperty(), std::optional<int>(0));
        grid->setHoverIndexCanBeNullProperty(true);
        grid->setGridSelectionModeProperty(GridSelectionMode::None);
        grid->OnMouseLeft();
        EXPECT_EQ(grid->getHoverRowIndexProperty(), std::optional<int>(0));
        EXPECT_EQ(grid->getHoverColumnIndexProperty(), std::optional<int>(0));
    }

    TEST_F(GridTests, RendersSelectionModesAndDebugLinesWithExactUpstreamGeometry)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        auto backend = std::make_unique<RecordingSpriteBatchBackend>();
        RecordingSpriteBatchBackend *const recording = backend.get();
        RenderContext context(std::make_unique<SpriteBatch>(std::move(backend)), &game.getGraphicsDeviceProperty());
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        context.setOpacityProperty(1.0F);
        context.Begin();

        Grid grid;
        grid.setColumnSpacingProperty(4);
        grid.setRowSpacingProperty(2);
        ArrangeTwoByTwo(grid);
        std::vector<BrushDraw> draws;
        grid.setSelectionHoverBackgroundProperty(std::make_shared<RecordingBrush>("hover", draws));
        grid.setSelectionBackgroundProperty(std::make_shared<RecordingBrush>("selection", draws));
        grid.setHoverRowIndexProperty(0);
        grid.setHoverColumnIndexProperty(0);
        grid.setSelectedRowIndexProperty(1);
        grid.setSelectedColumnIndexProperty(1);
        const Rectangle bounds = grid.getActualBoundsProperty();

        grid.setGridSelectionModeProperty(GridSelectionMode::Row);
        static_cast<Widget &>(grid).InternalRender(context);
        ASSERT_EQ(draws.size(), 2U);
        EXPECT_EQ(draws[0].name, "hover");
        EXPECT_EQ(draws[0].destination,
                  Rectangle(bounds.X, bounds.Y + grid.GetCellLocationY(0) - grid.getRowSpacingProperty() / 2,
                            bounds.Width, grid.GetRowHeight(0) + grid.getRowSpacingProperty()));
        EXPECT_EQ(draws[1].name, "selection");
        EXPECT_EQ(draws[1].destination,
                  Rectangle(bounds.X, bounds.Y + grid.GetCellLocationY(1) - grid.getRowSpacingProperty() / 2,
                            bounds.Width, grid.GetRowHeight(1) + grid.getRowSpacingProperty()));
        EXPECT_EQ(draws[0].color, Color::White);
        EXPECT_EQ(draws[1].color, Color::White);

        draws.clear();
        grid.setGridSelectionModeProperty(GridSelectionMode::Column);
        static_cast<Widget &>(grid).InternalRender(context);
        ASSERT_EQ(draws.size(), 2U);
        EXPECT_EQ(draws[0].destination,
                  Rectangle(bounds.X + grid.GetCellLocationX(0) - grid.getColumnSpacingProperty() / 2, bounds.Y,
                            grid.GetColumnWidth(0) + grid.getColumnSpacingProperty(), bounds.Height));
        EXPECT_EQ(draws[1].destination,
                  Rectangle(bounds.X + grid.GetCellLocationX(1) - grid.getColumnSpacingProperty() / 2, bounds.Y,
                            grid.GetColumnWidth(1) + grid.getColumnSpacingProperty(), bounds.Height));

        draws.clear();
        grid.setGridSelectionModeProperty(GridSelectionMode::Cell);
        static_cast<Widget &>(grid).InternalRender(context);
        ASSERT_EQ(draws.size(), 2U);
        EXPECT_EQ(draws[0].destination,
                  Rectangle(bounds.X + grid.GetCellLocationX(0) - grid.getColumnSpacingProperty() / 2,
                            bounds.Y + grid.GetCellLocationY(0) - grid.getRowSpacingProperty() / 2,
                            grid.GetColumnWidth(0) + grid.getColumnSpacingProperty(),
                            grid.GetRowHeight(0) + grid.getRowSpacingProperty()));
        EXPECT_EQ(draws[1].destination,
                  Rectangle(bounds.X + grid.GetCellLocationX(1) - grid.getColumnSpacingProperty() / 2,
                            bounds.Y + grid.GetCellLocationY(1) - grid.getRowSpacingProperty() / 2,
                            grid.GetColumnWidth(1) + grid.getColumnSpacingProperty(),
                            grid.GetRowHeight(1) + grid.getRowSpacingProperty()));

        draws.clear();
        const Color lineColor(10, 20, 30, 40);
        grid.setGridSelectionModeProperty(GridSelectionMode::None);
        grid.setShowGridLinesProperty(true);
        grid.setGridLinesColorProperty(lineColor);
        static_cast<Widget &>(grid).InternalRender(context);
        context.End();
        ASSERT_EQ(recording->draws.size(), grid.getGridLinesXProperty().size() + grid.getGridLinesYProperty().size());
        ASSERT_EQ(grid.getGridLinesXProperty().size(), 1U);
        ASSERT_EQ(grid.getGridLinesYProperty().size(), 1U);
        EXPECT_EQ(recording->draws[0].destination,
                  Rectangle(bounds.X + grid.getGridLinesXProperty()[0], bounds.Y, 1, bounds.Height));
        EXPECT_EQ(recording->draws[1].destination,
                  Rectangle(bounds.X, bounds.Y + grid.getGridLinesYProperty()[0], bounds.Width, 1));
        EXPECT_EQ(recording->draws[0].color, lineColor);
        EXPECT_EQ(recording->draws[1].color, lineColor);
    }

    TEST_F(GridTests, RetainsBrushesAcrossReentrantMutationAndRejectsInvalidRuntimeIndices)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        Grid grid;
        ArrangeTwoByTwo(grid);
        grid.setGridSelectionModeProperty(GridSelectionMode::Cell);
        grid.setHoverRowIndexProperty(0);
        grid.setHoverColumnIndexProperty(0);
        grid.setSelectedRowIndexProperty(1);
        grid.setSelectedColumnIndexProperty(1);

        std::vector<BrushDraw> draws;
        grid.setSelectionBackgroundProperty(std::make_shared<RecordingBrush>("selection", draws));
        std::weak_ptr<IBrush> weakHover;
        auto hover = std::make_shared<CallbackBrush>(
            [&]
            {
                grid.setSelectionHoverBackgroundProperty(nullptr);
                grid.setSelectionBackgroundProperty(nullptr);
                grid.setSelectedRowIndexProperty(std::nullopt);
                grid.setSelectedColumnIndexProperty(std::nullopt);
                EXPECT_FALSE(weakHover.expired());
            });
        weakHover = hover;
        grid.setSelectionHoverBackgroundProperty(hover);
        hover.reset();

        static_cast<Widget &>(grid).InternalRender(context);
        EXPECT_TRUE(draws.empty());
        EXPECT_TRUE(weakHover.expired());

        grid.setGridSelectionModeProperty(GridSelectionMode::Row);
        grid.setSelectedRowIndexProperty(9);
        grid.setSelectionBackgroundProperty(std::make_shared<RecordingBrush>("selection", draws));
        EXPECT_THROW(static_cast<Widget &>(grid).InternalRender(context), std::out_of_range);
    }

    TEST_F(GridTests, RegistersAndRoundTripsSelectionMmlWhileIgnoringRuntimeIndices)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *gridType = registry.FindByType(typeid(Grid));
        ASSERT_NE(gridType, nullptr);
        const PropertyDescriptor *selection = registry.FindPropertyByName(typeid(Grid), "SelectionBackground");
        const PropertyDescriptor *hover = registry.FindPropertyByName(typeid(Grid), "SelectionHoverBackground");
        const PropertyDescriptor *mode = registry.FindPropertyByName(typeid(Grid), "GridSelectionMode");
        const PropertyDescriptor *hoverRow = registry.FindPropertyByName(typeid(Grid), "HoverRowIndex");
        const PropertyDescriptor *selectedColumn = registry.FindPropertyByName(typeid(Grid), "SelectedColumnIndex");
        ASSERT_NE(selection, nullptr);
        ASSERT_NE(hover, nullptr);
        ASSERT_NE(mode, nullptr);
        ASSERT_NE(hoverRow, nullptr);
        ASSERT_NE(selectedColumn, nullptr);
        EXPECT_TRUE(selection->getMetadataProperty().ExternalAsset);
        EXPECT_TRUE(hover->getMetadataProperty().ExternalAsset);
        EXPECT_TRUE(hoverRow->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedColumn->getMetadataProperty().XmlIgnore);
        EXPECT_EQ(registry.FindPropertyByName(typeid(Grid), "GridLinesColor"), nullptr);

        std::vector<BrushDraw> draws;
        const std::shared_ptr<IBrush> selectionBrush = std::make_shared<RecordingBrush>("selection", draws);
        const std::shared_ptr<IBrush> hoverBrush = std::make_shared<RecordingBrush>("hover", draws);
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        loader.LoadExternalAsset = [&](const PropertyDescriptor &property, const std::string &name) -> std::any
        {
            if (&property == selection)
            {
                EXPECT_EQ(name, "selection");
                return selectionBrush;
            }
            EXPECT_EQ(&property, hover);
            EXPECT_EQ(name, "hover");
            return hoverBrush;
        };
        System::Xml::XmlDocument document;
        document.LoadXml("<Grid ShowGridLines=\"True\" GridSelectionMode=\"Cell\" HoverIndexCanBeNull=\"False\" "
                         "CanSelectNothing=\"True\" SelectionBackground=\"selection\" "
                         "SelectionHoverBackground=\"hover\" />");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(Grid));
        auto *const loadedGrid = static_cast<Grid *>(loaded.Value.get());
        EXPECT_TRUE(loadedGrid->getShowGridLinesProperty());
        EXPECT_EQ(loadedGrid->getGridSelectionModeProperty(), GridSelectionMode::Cell);
        EXPECT_FALSE(loadedGrid->getHoverIndexCanBeNullProperty());
        EXPECT_TRUE(loadedGrid->getCanSelectNothingProperty());
        EXPECT_EQ(loadedGrid->getSelectionBackgroundProperty(), selectionBrush);
        EXPECT_EQ(loadedGrid->getSelectionHoverBackgroundProperty(), hoverBrush);
        loadedGrid->setHoverRowIndexProperty(2);
        loadedGrid->setSelectedColumnIndexProperty(3);

        SaveContext saver(registry, codecs);
        saver.SaveExternalAsset = [&](const PropertyDescriptor &property, const std::any &value)
        {
            const auto &brush = std::any_cast<const std::shared_ptr<IBrush> &>(value);
            if (&property == selection)
            {
                EXPECT_EQ(brush, selectionBrush);
                return std::string("selection");
            }
            EXPECT_EQ(&property, hover);
            EXPECT_EQ(brush, hoverBrush);
            return std::string("hover");
        };
        const std::string xml = saver.ToXml(loadedGrid, typeid(Grid));
        EXPECT_NE(xml.find("ShowGridLines=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("GridSelectionMode=\"Cell\""), std::string::npos);
        EXPECT_NE(xml.find("HoverIndexCanBeNull=\"False\""), std::string::npos);
        EXPECT_NE(xml.find("CanSelectNothing=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("SelectionBackground=\"selection\""), std::string::npos);
        EXPECT_NE(xml.find("SelectionHoverBackground=\"hover\""), std::string::npos);
        EXPECT_EQ(xml.find("HoverRowIndex="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedColumnIndex="), std::string::npos);
    }
} // namespace
