// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Misc/TreeView.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D::UI
{
    struct TreeViewTestAccess
    {
        static void SetHoverRow(TreeView &tree, TreeViewNode *const node) { tree.hoverRow_ = node; }

        static void RefreshRowVisibility(TreeView &tree) { tree.RefreshRowVisibility(); }
    };
} // namespace Myra::Graphics2D::UI

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::TreeView;
    using Myra::Graphics2D::UI::TreeViewNode;
    using Myra::Graphics2D::UI::TreeViewTestAccess;
    using Myra::Graphics2D::UI::Widget;

    struct RowBrushObservation
    {
        std::string name;
        Rectangle destination;
        Color color;
    };

    class RowRecordingBrush final : public IBrush
    {
      public:
        RowRecordingBrush(std::string name, std::vector<RowBrushObservation> &observations)
            : name_(std::move(name)), observations_(observations)
        {
        }

        void Draw(RenderContext &, const Rectangle destination, const Color color) const override
        {
            observations_.push_back({name_, destination, color});
        }

      private:
        std::string name_;
        std::vector<RowBrushObservation> &observations_;
    };

    class CallbackBrush final : public IBrush
    {
      public:
        explicit CallbackBrush(std::function<void()> callback) : callback_(std::move(callback)) {}

        void Draw(RenderContext &, Rectangle, Color) const override { callback_(); }

      private:
        std::function<void()> callback_;
    };

    RenderContext MakeContext(Game &game)
    {
        auto &device = game.getGraphicsDeviceProperty();
        device.setViewportProperty(Viewport(0, 0, 400, 300));
        return RenderContext(device);
    }

    Rectangle ExpectedRowRectangle(TreeView &tree, TreeViewNode &node)
    {
        const Point rowPosition = tree.ToLocal(node.ToGlobal(node.getActualBoundsProperty().getLocationProperty()));
        const Rectangle bounds = tree.getActualBoundsProperty();
        return Rectangle(bounds.X, rowPosition.Y, bounds.Width, node.getContentHeightProperty());
    }

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

    TEST(TreeViewTests, RetainedMarkBecomesInertAfterItsNodeIsDestroyed)
    {
        TreeView tree;
        auto root = tree.AddSubNode(std::make_shared<Widget>());
        static_cast<void>(root->AddSubNode(std::make_shared<Widget>()));
        const auto mark = root->getMarkProperty();
        const auto childPanel = root->getChildNodesStackPanelProperty();
        root->setIsExpandedProperty(true);
        ASSERT_TRUE(childPanel->getVisibleProperty());
        const std::weak_ptr<TreeViewNode> weakRoot = root;

        tree.RemoveAllSubNodes();
        root.reset();
        ASSERT_TRUE(weakRoot.expired());
        mark->DoClick();
        EXPECT_FALSE(mark->getIsPressedProperty());
        EXPECT_TRUE(childPanel->getVisibleProperty());
    }

    TEST(TreeViewTests, RendersVisibleHoverAndSelectionRowsInThePinnedPriorityOrder)
    {
        Game game;
        RenderContext context = MakeContext(game);
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        TreeView tree;
        const auto rootContent = std::make_shared<Widget>();
        rootContent->setWidthProperty(80);
        rootContent->setHeightProperty(20);
        const auto root = tree.AddSubNode(rootContent);
        const auto childContent = std::make_shared<Widget>();
        childContent->setWidthProperty(80);
        childContent->setHeightProperty(12);
        const auto child = root->AddSubNode(childContent);
        const auto secondContent = std::make_shared<Widget>();
        secondContent->setWidthProperty(80);
        secondContent->setHeightProperty(16);
        const auto second = tree.AddSubNode(secondContent);
        root->setIsExpandedProperty(true);
        static_cast<void>(tree.Measure(Point(120, 100)));
        tree.Arrange(Rectangle(10, 20, 120, 100));

        std::vector<RowBrushObservation> observations;
        const auto selection = std::make_shared<RowRecordingBrush>("selection", observations);
        const auto hover = std::make_shared<RowRecordingBrush>("hover", observations);
        tree.setSelectionBackgroundProperty(selection);
        tree.setSelectionHoverBackgroundProperty(hover);
        tree.setSelectedNodeProperty(root);
        TreeViewTestAccess::SetHoverRow(tree, second.get());

        tree.Render(context);

        ASSERT_EQ(observations.size(), 2U);
        EXPECT_EQ(observations[0].name, "hover");
        EXPECT_EQ(observations[0].destination, ExpectedRowRectangle(tree, *second));
        EXPECT_EQ(observations[0].color, Color::White);
        EXPECT_EQ(observations[1].name, "selection");
        EXPECT_EQ(observations[1].destination, ExpectedRowRectangle(tree, *root));
        EXPECT_EQ(observations[1].color, Color::White);
        EXPECT_EQ(tree.getSelectionBackgroundProperty(), selection);
        EXPECT_EQ(tree.getSelectionHoverBackgroundProperty(), hover);

        const auto clone = std::dynamic_pointer_cast<TreeView>(tree.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getSelectionBackgroundProperty(), selection);
        EXPECT_EQ(clone->getSelectionHoverBackgroundProperty(), hover);

        observations.clear();
        tree.setSelectionBackgroundProperty(nullptr);
        tree.Render(context);
        ASSERT_EQ(observations.size(), 1U);
        EXPECT_EQ(observations[0].name, "hover");
        EXPECT_EQ(observations[0].destination, ExpectedRowRectangle(tree, *second));

        observations.clear();
        tree.setSelectionBackgroundProperty(selection);
        TreeViewTestAccess::SetHoverRow(tree, root.get());
        tree.Render(context);
        ASSERT_EQ(observations.size(), 1U);
        EXPECT_EQ(observations[0].name, "selection");

        observations.clear();
        root->setIsExpandedProperty(false);
        TreeViewTestAccess::RefreshRowVisibility(tree);
        tree.setSelectedNodeProperty(child);
        TreeViewTestAccess::SetHoverRow(tree, nullptr);
        tree.Render(context);
        EXPECT_TRUE(observations.empty());
    }

    TEST(TreeViewTests, RetainsRenderTargetsAcrossReentrantBrushAndTreeRemoval)
    {
        Game game;
        RenderContext context = MakeContext(game);
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        TreeView tree;
        auto root = tree.AddSubNode(std::make_shared<Widget>());
        auto second = tree.AddSubNode(std::make_shared<Widget>());
        static_cast<void>(tree.Measure(Point(100, 60)));
        tree.Arrange(Rectangle(0, 0, 100, 60));
        tree.setSelectedNodeProperty(root);
        TreeViewTestAccess::SetHoverRow(tree, second.get());

        std::vector<RowBrushObservation> observations;
        tree.setSelectionBackgroundProperty(std::make_shared<RowRecordingBrush>("selection", observations));
        std::weak_ptr<IBrush> weakBrush;
        const std::weak_ptr<TreeViewNode> weakRoot = root;
        const std::weak_ptr<TreeViewNode> weakHover = second;
        auto callbackBrush = std::make_shared<CallbackBrush>(
            [&]
            {
                tree.setSelectionHoverBackgroundProperty(nullptr);
                tree.setSelectionBackgroundProperty(nullptr);
                tree.RemoveAllSubNodes();
                EXPECT_FALSE(weakBrush.expired());
                EXPECT_FALSE(weakRoot.expired());
                EXPECT_FALSE(weakHover.expired());
            });
        weakBrush = callbackBrush;
        tree.setSelectionHoverBackgroundProperty(callbackBrush);
        callbackBrush.reset();
        root.reset();
        second.reset();

        tree.Render(context);

        EXPECT_TRUE(observations.empty());
        EXPECT_TRUE(weakBrush.expired());
        EXPECT_EQ(tree.getTotalNodesCountProperty(), 0);
        tree.UpdateArrange();
        EXPECT_TRUE(weakRoot.expired());
        EXPECT_TRUE(weakHover.expired());
        EXPECT_EQ(tree.getSubNodesCountProperty(), 0);
        EXPECT_EQ(tree.getSelectedNodeProperty(), nullptr);
        EXPECT_EQ(tree.getHoverRowProperty(), nullptr);
    }
} // namespace
