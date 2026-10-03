// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::Widget;

    class FixedWidget final : public Widget
    {
    public:
        explicit FixedWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

    protected:
        [[nodiscard]] Point InternalMeasure(Point) override { return desiredSize_; }

    private:
        Point desiredSize_;
    };

    TEST(GridLayoutTests, MeasuresPartColumnsAndArrangesChildrenIntoCells)
    {
        Grid grid;
        auto first = std::make_shared<FixedWidget>(Point(20, 10));
        auto second = std::make_shared<FixedWidget>(Point(30, 15));
        Grid::SetColumn(*second, 1);
        grid.AddWidget(first);
        grid.AddWidget(second);

        EXPECT_EQ(grid.Measure(Point(200, 100)), Point(60, 15));
        grid.Arrange(Rectangle(10, 20, 100, 40));

        EXPECT_EQ(grid.GetColumnWidth(0), 50);
        EXPECT_EQ(grid.GetColumnWidth(1), 50);
        EXPECT_EQ(grid.GetRowHeight(0), 40);
        EXPECT_EQ(grid.GetCellLocationX(0), 0);
        EXPECT_EQ(grid.GetCellLocationX(1), 50);
        EXPECT_EQ(grid.GetCellRectangle(1, 0), Rectangle(50, 0, 50, 40));
        EXPECT_EQ(first->getContainerBoundsProperty(), Rectangle(0, 0, 50, 40));
        EXPECT_EQ(second->getContainerBoundsProperty(), Rectangle(50, 0, 50, 40));
    }

    TEST(GridLayoutTests, SupportsSpansAndAttachedPropertyMetadata)
    {
        Grid grid;
        auto child = std::make_shared<FixedWidget>(Point(80, 10));
        Grid::SetColumnSpan(*child, 2);
        Grid::SetRowSpan(*child, 2);
        grid.AddWidget(child);

        EXPECT_EQ(Grid::GetColumn(*child), 0);
        EXPECT_EQ(Grid::GetRow(*child), 0);
        EXPECT_EQ(Grid::GetColumnSpan(*child), 2);
        EXPECT_EQ(Grid::GetRowSpan(*child), 2);
        ASSERT_TRUE(Grid::getColumnProperty().getMetadataProperty().Range.has_value());
        EXPECT_EQ(Grid::getColumnProperty().getMetadataProperty().Range->getMinimumProperty(), 0.0F);

        grid.Arrange(Rectangle(0, 0, 100, 60));
        EXPECT_EQ(grid.GetCellRectangle(0, 0), Rectangle(0, 0, 50, 30));
        EXPECT_EQ(child->getContainerBoundsProperty(), Rectangle(0, 0, 100, 60));
    }

    TEST(GridLayoutTests, ObservableProportionsInvalidateAndObserveRetainedObjects)
    {
        Grid grid;
        auto first = std::make_shared<FixedWidget>(Point(20, 10));
        auto second = std::make_shared<FixedWidget>(Point(30, 10));
        Grid::SetColumn(*second, 1);
        grid.AddWidget(first);
        grid.AddWidget(second);

        auto proportion = std::make_shared<Myra::Graphics2D::UI::Proportion>(
            Myra::Graphics2D::UI::ProportionType::Pixels, 20.0F);
        auto& columns = grid.getColumnsProportionsProperty();
        columns.Add(proportion);
        EXPECT_EQ(columns.getCountProperty(), 1);
        EXPECT_EQ(columns[0], proportion);

        grid.Arrange(Rectangle(0, 0, 100, 30));
        EXPECT_EQ(grid.GetColumnWidth(0), 20);
        EXPECT_EQ(grid.GetColumnWidth(1), 80);

        proportion->setValueProperty(35.0F);
        grid.Arrange(Rectangle(0, 0, 100, 30));
        EXPECT_EQ(grid.GetColumnWidth(0), 35);
        EXPECT_EQ(grid.GetColumnWidth(1), 65);

        ASSERT_TRUE(columns.Remove(proportion));
        grid.Arrange(Rectangle(0, 0, 100, 30));
        EXPECT_EQ(grid.GetColumnWidth(0), 50);
        EXPECT_EQ(grid.GetColumnWidth(1), 50);
    }

    TEST(GridLayoutTests, DetachesProportionSubscriptionsWhenDestroyed)
    {
        auto proportion = std::make_shared<Myra::Graphics2D::UI::Proportion>(
            Myra::Graphics2D::UI::ProportionType::Pixels, 10.0F);
        {
            auto grid = std::make_unique<Grid>();
            grid->getColumnsProportionsProperty().Add(proportion);
        }

        proportion->setValueProperty(20.0F);
    }

    TEST(GridLayoutTests, RejectsInvalidCoordinatesAndNullProportionsDeterministically)
    {
        Myra::Graphics2D::UI::GridLayout layout;
        layout.setDefaultColumnProportionProperty(nullptr);
        layout.setDefaultRowProportionProperty(nullptr);
        EXPECT_EQ(layout.getDefaultColumnProportionProperty(), nullptr);
        EXPECT_EQ(layout.getDefaultRowProportionProperty(), nullptr);
        EXPECT_THROW(static_cast<void>(layout.GetColumnProportion(-1)), std::logic_error);
        EXPECT_THROW(static_cast<void>(layout.GetRowProportion(-1)), std::logic_error);
        layout.getColumnsProportionsProperty().Add(nullptr);
        EXPECT_THROW(static_cast<void>(layout.GetColumnProportion(0)), std::logic_error);

        Grid grid;
        auto child = std::make_shared<FixedWidget>(Point(10, 10));
        Grid::SetColumn(*child, -1);
        grid.AddWidget(child);
        EXPECT_THROW(static_cast<void>(grid.Measure(Point(100, 100))), std::invalid_argument);

        Grid invalidSpanGrid;
        auto invalidSpanChild = std::make_shared<FixedWidget>(Point(10, 10));
        Grid::SetColumnSpan(*invalidSpanChild, 0);
        invalidSpanGrid.AddWidget(invalidSpanChild);
        EXPECT_THROW(static_cast<void>(invalidSpanGrid.Measure(Point(100, 100))), std::invalid_argument);
    }

    TEST(GridLayoutTests, UsesUpstreamEpsilonForEffectivelyZeroPartTotals)
    {
        Grid grid;
        auto first = std::make_shared<FixedWidget>(Point(20, 10));
        auto second = std::make_shared<FixedWidget>(Point(20, 10));
        Grid::SetColumn(*second, 1);
        grid.AddWidget(first);
        grid.AddWidget(second);
        grid.getColumnsProportionsProperty().Add(std::make_shared<Myra::Graphics2D::UI::Proportion>(
            Myra::Graphics2D::UI::ProportionType::Part, 0.25e-6F));
        grid.getColumnsProportionsProperty().Add(std::make_shared<Myra::Graphics2D::UI::Proportion>(
            Myra::Graphics2D::UI::ProportionType::Part, 0.25e-6F));

        grid.Arrange(Rectangle(0, 0, 100, 30));

        EXPECT_EQ(grid.GetColumnWidth(0), 0);
        EXPECT_EQ(grid.GetColumnWidth(1), 0);
    }

    TEST(GridLayoutTests, RejectsNonFiniteProportionsAndIntegerOverflow)
    {
        Grid nonFiniteGrid;
        nonFiniteGrid.setDefaultColumnProportionProperty(
            std::make_shared<Myra::Graphics2D::UI::Proportion>(
                Myra::Graphics2D::UI::ProportionType::Pixels,
                std::numeric_limits<float>::infinity()));
        nonFiniteGrid.AddWidget(std::make_shared<FixedWidget>(Point(10, 10)));
        EXPECT_THROW(static_cast<void>(nonFiniteGrid.Measure(Point(100, 100))), std::invalid_argument);

        Grid overflowingSpacingGrid;
        overflowingSpacingGrid.setColumnSpacingProperty(std::numeric_limits<int>::max());
        for (int column = 0; column < 3; ++column)
        {
            auto child = std::make_shared<FixedWidget>(Point(1, 1));
            Grid::SetColumn(*child, column);
            overflowingSpacingGrid.AddWidget(child);
        }
        EXPECT_THROW(static_cast<void>(overflowingSpacingGrid.Measure(Point(100, 100))), std::overflow_error);
    }
}
