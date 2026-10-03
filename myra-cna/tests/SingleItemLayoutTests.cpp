// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::SingleItemLayout;
    using Myra::Graphics2D::UI::Widget;

    class FixedWidget final : public Widget
    {
    public:
        explicit FixedWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

    protected:
        [[nodiscard]] Point InternalMeasure(Point) override
        {
            return desiredSize_;
        }

    private:
        Point desiredSize_;
    };

    TEST(SingleItemLayoutTests, ReplacesTheContainerContentAndMeasuresTheFirstChild)
    {
        Widget container;
        SingleItemLayout<FixedWidget> layout(container);
        const auto first = std::make_shared<FixedWidget>(Point(20, 30));
        const auto second = std::make_shared<FixedWidget>(Point(40, 50));

        EXPECT_EQ(layout.Measure(container.getChildrenProperty(), Point(100, 100)), Point(0, 0));
        layout.setChildProperty(first);
        EXPECT_EQ(layout.getChildProperty(), first);
        EXPECT_EQ(layout.Measure(container.getChildrenProperty(), Point(100, 100)), Point(20, 30));

        layout.setChildProperty(second);
        ASSERT_EQ(container.getChildrenProperty().size(), 1U);
        EXPECT_EQ(layout.getChildProperty(), second);
        EXPECT_EQ(second->getParentProperty(), &container);
        EXPECT_EQ(first->getParentProperty(), nullptr);
    }

    TEST(SingleItemLayoutTests, ArrangesOnlyAVisibleChild)
    {
        Widget container;
        SingleItemLayout<FixedWidget> layout(container);
        const auto child = std::make_shared<FixedWidget>(Point(20, 30));
        layout.setChildProperty(child);

        layout.Arrange(container.getChildrenProperty(), Rectangle(1, 2, 100, 80));
        EXPECT_EQ(child->getBoundsProperty(), Rectangle(0, 0, 20, 30));

        child->setVisibleProperty(false);
        layout.Arrange(container.getChildrenProperty(), Rectangle(10, 20, 30, 40));
        EXPECT_EQ(child->getBoundsProperty(), Rectangle(0, 0, 20, 30));
    }
}
