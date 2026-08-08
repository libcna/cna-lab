// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/UIUtils.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <vector>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::Widget;
    using Myra::Utility::UIUtils;

    TEST(UIUtilsTests, TraversesVisibleWidgetsDepthFirstAndStopsImmediately)
    {
        Widget root;
        auto first = std::make_shared<Widget>();
        auto nested = std::make_shared<Widget>();
        auto hidden = std::make_shared<Widget>();
        auto hiddenNested = std::make_shared<Widget>();
        auto last = std::make_shared<Widget>();
        first->AddChild(nested);
        hidden->AddChild(hiddenNested);
        hidden->setVisibleProperty(false);
        root.AddChild(first);
        root.AddChild(hidden);
        root.AddChild(last);

        std::vector<Widget*> visited;
        EXPECT_TRUE(UIUtils::ProcessWidgets(root, [&visited](Widget& widget) {
            visited.push_back(&widget);
            return true;
        }));
        EXPECT_EQ(visited, (std::vector<Widget*>{&root, first.get(), nested.get(), last.get()}));

        visited.clear();
        EXPECT_FALSE(UIUtils::ProcessWidgets(root, [&visited, &nested](Widget& widget) {
            visited.push_back(&widget);
            return &widget != nested.get();
        }));
        EXPECT_EQ(visited, (std::vector<Widget*>{&root, first.get(), nested.get()}));
        EXPECT_THROW(static_cast<void>(UIUtils::ProcessWidgets(root, {})), std::invalid_argument);
    }

    TEST(UIUtilsTests, StableBubbleSortOrdersWidgetsByAscendingZIndex)
    {
        auto firstTwo = std::make_shared<Widget>();
        auto one = std::make_shared<Widget>();
        auto secondTwo = std::make_shared<Widget>();
        auto negative = std::make_shared<Widget>();
        firstTwo->setZIndexProperty(2);
        one->setZIndexProperty(1);
        secondTwo->setZIndexProperty(2);
        negative->setZIndexProperty(-1);

        std::vector<std::shared_ptr<Widget>> widgets{firstTwo, one, secondTwo, negative};
        UIUtils::SortWidgetsByZIndex(widgets);
        EXPECT_EQ(widgets, (std::vector<std::shared_ptr<Widget>>{negative, one, firstTwo, secondTwo}));

        widgets.push_back(nullptr);
        EXPECT_THROW(UIUtils::SortWidgetsByZIndex(widgets), std::invalid_argument);
    }

    TEST(UIUtilsTests, WidgetSnapshotsUseTheSameStableUpstreamOrdering)
    {
        Widget root;
        auto first = std::make_shared<Widget>();
        auto lower = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        first->setZIndexProperty(4);
        lower->setZIndexProperty(1);
        second->setZIndexProperty(4);
        root.AddChild(first);
        root.AddChild(lower);
        root.AddChild(second);

        const auto& sorted = root.getChildrenCopyProperty();
        EXPECT_EQ(sorted, (std::vector<std::shared_ptr<Widget>>{lower, first, second}));
    }

    TEST(UIUtilsTests, TraversalRetainsItsSnapshotDuringReentrantTreeMutation)
    {
        Widget root;
        auto first = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        root.AddChild(first);
        root.AddChild(second);

        std::vector<Widget*> visited;
        EXPECT_TRUE(UIUtils::ProcessWidgets(root, [&](Widget& widget) {
            visited.push_back(&widget);
            if (&widget == first.get())
            {
                EXPECT_TRUE(root.RemoveChild(second.get()));
                static_cast<void>(root.getChildrenCopyProperty());
            }
            return true;
        }));

        EXPECT_EQ(visited, (std::vector<Widget*>{&root, first.get(), second.get()}));
        EXPECT_EQ(second->getParentProperty(), nullptr);
    }
}
