// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

#include <gtest/gtest.h>

#include <memory>

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
}
