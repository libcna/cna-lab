// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Misc/TreeViewNode.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::TreeViewNode;
    using Myra::Graphics2D::UI::Widget;

    TEST(TreeViewNodeTests, OwnsContentAndOrderedChildNodesWithAnExpandMark)
    {
        TreeViewNode root;
        const auto firstContent = std::make_shared<Widget>();
        firstContent->setWidthProperty(30);
        firstContent->setHeightProperty(12);
        root.setContentProperty(firstContent);

        EXPECT_EQ(root.getContentProperty(), firstContent);
        EXPECT_EQ(firstContent->getParentProperty(), &root);
        EXPECT_FALSE(root.getMarkProperty()->getVisibleProperty());
        EXPECT_FALSE(root.getChildNodesStackPanelProperty()->getVisibleProperty());

        const auto first = root.AddSubNode(std::make_shared<Widget>());
        const auto second = root.AddSubNode(nullptr);
        ASSERT_NE(first, nullptr);
        ASSERT_NE(second, nullptr);
        EXPECT_EQ(root.getChildNodesCountProperty(), 2);
        EXPECT_EQ(root.GetSubNode(0), first);
        EXPECT_EQ(root.GetSubNode(1), second);
        EXPECT_EQ(first->getParentNodeProperty(), &root);
        EXPECT_EQ(second->getParentNodeProperty(), &root);
        EXPECT_TRUE(root.getMarkProperty()->getVisibleProperty());

        const Point measured = root.Measure(Point(100, 80));
        EXPECT_EQ(measured, Point(32, 14));
        root.Arrange(Rectangle(0, 0, 100, 80));
        EXPECT_EQ(root.getContentHeightProperty(), 12);

        root.setIsExpandedProperty(true);
        EXPECT_TRUE(root.getIsExpandedProperty());
        EXPECT_TRUE(root.getChildNodesStackPanelProperty()->getVisibleProperty());
        root.getMarkProperty()->DoClick();
        EXPECT_FALSE(root.getIsExpandedProperty());
        EXPECT_FALSE(root.getChildNodesStackPanelProperty()->getVisibleProperty());

        EXPECT_THROW(static_cast<void>(root.GetSubNode(-1)), std::out_of_range);
        EXPECT_THROW(static_cast<void>(root.GetSubNode(2)), std::out_of_range);
    }

    TEST(TreeViewNodeTests, RemovesChildrenAndDetachesTheirHierarchyParent)
    {
        TreeViewNode root;
        const auto first = root.AddSubNode(std::make_shared<Widget>());
        const auto second = root.AddSubNode(std::make_shared<Widget>());
        const auto nested = first->AddSubNode(std::make_shared<Widget>());
        ASSERT_NE(nested, nullptr);

        root.RemoveSubNode(first.get());
        EXPECT_EQ(root.getChildNodesCountProperty(), 1);
        EXPECT_EQ(first->getParentNodeProperty(), nullptr);
        EXPECT_EQ(nested->getParentNodeProperty(), first.get());
        EXPECT_EQ(root.GetSubNode(0), second);

        root.RemoveSubNodeAt(0);
        EXPECT_EQ(root.getChildNodesCountProperty(), 0);
        EXPECT_EQ(second->getParentNodeProperty(), nullptr);
        EXPECT_FALSE(root.getMarkProperty()->getVisibleProperty());
        EXPECT_THROW(root.RemoveSubNodeAt(0), std::out_of_range);

        const auto third = root.AddSubNode(std::make_shared<Widget>());
        const auto fourth = root.AddSubNode(std::make_shared<Widget>());
        root.RemoveAllSubNodes();
        EXPECT_EQ(root.getChildNodesCountProperty(), 0);
        EXPECT_EQ(third->getParentNodeProperty(), nullptr);
        EXPECT_EQ(fourth->getParentNodeProperty(), nullptr);
        EXPECT_FALSE(root.getMarkProperty()->getVisibleProperty());
    }

    TEST(TreeViewNodeTests, ClonesThePortedContentTreeAndExpandState)
    {
        TreeViewNode root;
        const auto content = std::make_shared<Widget>();
        root.setContentProperty(content);
        const auto child = root.AddSubNode(std::make_shared<Widget>());
        const auto nested = child->AddSubNode(std::make_shared<Widget>());
        root.setIsExpandedProperty(true);
        child->setIsExpandedProperty(true);

        const auto clone = std::dynamic_pointer_cast<TreeViewNode>(root.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_TRUE(clone->getIsExpandedProperty());
        ASSERT_EQ(clone->getChildNodesCountProperty(), 1);

        const auto clonedChild = clone->GetSubNode(0);
        ASSERT_NE(clonedChild, child);
        EXPECT_EQ(clonedChild->getParentNodeProperty(), clone.get());
        EXPECT_TRUE(clonedChild->getIsExpandedProperty());
        ASSERT_EQ(clonedChild->getChildNodesCountProperty(), 1);
        EXPECT_NE(clonedChild->GetSubNode(0), nested);
        EXPECT_EQ(clonedChild->GetSubNode(0)->getParentNodeProperty(), clonedChild.get());
    }
} // namespace
