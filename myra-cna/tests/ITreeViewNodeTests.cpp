// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Misc/ITreeViewNode.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::ITreeViewNode;
    using Myra::Graphics2D::UI::TreeViewNode;
    using Myra::Graphics2D::UI::Widget;

    class TestTreeNode final : public ITreeViewNode
    {
      public:
        [[nodiscard]] int getChildNodesCountProperty() const noexcept override
        {
            return static_cast<int>(children_.size());
        }

        [[nodiscard]] std::shared_ptr<TreeViewNode> AddSubNode(std::shared_ptr<Widget>) override
        {
            children_.push_back(nullptr);
            return nullptr;
        }

        [[nodiscard]] std::shared_ptr<TreeViewNode> GetSubNode(const int index) const override
        {
            return children_.at(static_cast<std::size_t>(index));
        }

        void RemoveAllSubNodes() override { children_.clear(); }

      private:
        std::vector<std::shared_ptr<TreeViewNode>> children_;
    };

    TEST(ITreeViewNodeTests, ExposesTheOrderedChildNodeContract)
    {
        TestTreeNode node;
        EXPECT_EQ(node.getChildNodesCountProperty(), 0);
        EXPECT_EQ(node.AddSubNode(std::make_shared<Widget>()), nullptr);
        EXPECT_EQ(node.getChildNodesCountProperty(), 1);
        EXPECT_EQ(node.GetSubNode(0), nullptr);
        node.RemoveAllSubNodes();
        EXPECT_EQ(node.getChildNodesCountProperty(), 0);
    }
} // namespace
