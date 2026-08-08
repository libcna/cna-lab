// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::HorizontalStackPanel;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::StackPanel;
    using Myra::Graphics2D::UI::VerticalStackPanel;
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
        auto& proportions = panel.getProportionsProperty();
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
}
