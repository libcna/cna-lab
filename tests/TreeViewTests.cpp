// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Misc/TreeView.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::TreeView;
    using Myra::Graphics2D::UI::TreeViewNode;
    using Myra::Graphics2D::UI::Widget;

    TEST(TreeViewTests, RegistersTheCompleteOrderedTreeAndSupportsTraversalAndLookup)
    {
        TreeView tree;
        const auto firstContent = std::make_shared<Widget>();
        const auto secondContent = std::make_shared<Widget>();
        const auto root = tree.AddSubNode(firstContent);
        const auto child = root->AddSubNode(std::make_shared<Widget>());
        const auto nested = child->AddSubNode(std::make_shared<Widget>());
        const auto secondRoot = tree.AddSubNode(secondContent);

        EXPECT_EQ(tree.getSubNodesCountProperty(), 2);
        EXPECT_EQ(tree.getChildNodesCountProperty(), 2);
        EXPECT_EQ(tree.getTotalNodesCountProperty(), 4);
        EXPECT_EQ(tree.GetSubNode(0), root);
        EXPECT_EQ(tree.GetSubNode(1), secondRoot);
        EXPECT_EQ(tree.GetNodeByAbsoluteIndex(0), root);
        EXPECT_EQ(tree.GetNodeByAbsoluteIndex(1), child);
        EXPECT_EQ(tree.GetNodeByAbsoluteIndex(2), nested);
        EXPECT_EQ(tree.GetNodeByAbsoluteIndex(3), secondRoot);
        EXPECT_EQ(
            tree.FindNode([secondContent](TreeViewNode &node) { return node.getContentProperty() == secondContent; }),
            secondRoot);

        std::vector<TreeViewNode *> traversed;
        tree.Iterate(
            [&traversed](TreeViewNode &node)
            {
                traversed.push_back(&node);
                return true;
            });
        EXPECT_EQ(traversed, (std::vector<TreeViewNode *>{root.get(), child.get(), nested.get(), secondRoot.get()}));

        std::vector<TreeViewNode *> partial;
        tree.Iterate(
            [&partial](TreeViewNode &node)
            {
                partial.push_back(&node);
                return partial.size() < 2U;
            });
        EXPECT_EQ(partial, (std::vector<TreeViewNode *>{root.get(), child.get()}));
        EXPECT_THROW(static_cast<void>(tree.GetSubNode(-1)), std::out_of_range);
        EXPECT_THROW(static_cast<void>(tree.GetNodeByAbsoluteIndex(4)), std::out_of_range);
        EXPECT_THROW(tree.Iterate({}), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(tree.FindNode({})), std::invalid_argument);
    }

    TEST(TreeViewTests, SelectsNodesAndNavigatesThePortedParentChildKeyboardPaths)
    {
        TreeView tree;
        const auto root = tree.AddSubNode(std::make_shared<Widget>());
        const auto first = root->AddSubNode(std::make_shared<Widget>());
        const auto second = root->AddSubNode(std::make_shared<Widget>());
        root->setIsExpandedProperty(true);

        int selectionChanges = 0;
        tree.SelectionChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &tree);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectionChanged);
            ++selectionChanges;
        };

        tree.setSelectedNodeProperty(first);
        EXPECT_EQ(selectionChanges, 1);
        tree.setSelectedNodeProperty(first);
        EXPECT_EQ(selectionChanges, 1);
        tree.OnKeyDown(Keys::Up);
        EXPECT_EQ(tree.getSelectedNodeProperty(), root);
        tree.OnKeyDown(Keys::Down);
        EXPECT_EQ(tree.getSelectedNodeProperty(), first);
        tree.OnKeyDown(Keys::Down);
        EXPECT_EQ(tree.getSelectedNodeProperty(), second);
        tree.OnKeyDown(Keys::Enter);
        EXPECT_TRUE(second->getIsExpandedProperty());
        EXPECT_EQ(selectionChanges, 4);
    }

    TEST(TreeViewTests, ExpandsAncestorPathRemovesSubtreesAndClonesAllNodes)
    {
        TreeView tree;
        const auto root = tree.AddSubNode(std::make_shared<Widget>());
        const auto child = root->AddSubNode(std::make_shared<Widget>());
        const auto nested = child->AddSubNode(std::make_shared<Widget>());
        tree.ExpandPath(nested);
        EXPECT_TRUE(root->getIsExpandedProperty());
        EXPECT_TRUE(child->getIsExpandedProperty());
        EXPECT_FALSE(nested->getIsExpandedProperty());

        tree.setSelectedNodeProperty(nested);
        root->RemoveSubNode(child.get());
        EXPECT_EQ(tree.getTotalNodesCountProperty(), 1);
        EXPECT_EQ(tree.getSelectedNodeProperty(), nullptr);
        EXPECT_EQ(child->getParentNodeProperty(), nullptr);

        const auto replacement = root->AddSubNode(std::make_shared<Widget>());
        replacement->AddSubNode(std::make_shared<Widget>());
        const auto clone = std::dynamic_pointer_cast<TreeView>(tree.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getSubNodesCountProperty(), 1);
        EXPECT_EQ(clone->getTotalNodesCountProperty(), 3);
        EXPECT_NE(clone->GetSubNode(0), root);
        EXPECT_EQ(clone->GetSubNode(0)->getChildNodesCountProperty(), 1);
        EXPECT_EQ(clone->GetSubNode(0)->GetSubNode(0)->getChildNodesCountProperty(), 1);

        tree.RemoveAllSubNodes();
        EXPECT_EQ(tree.getSubNodesCountProperty(), 0);
        EXPECT_EQ(tree.getTotalNodesCountProperty(), 0);
    }
} // namespace
