// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Misc/TreeViewNode.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Misc/TreeViewNode.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"

namespace Myra::Graphics2D::UI
{
    TreeViewNode::TreeViewNode(TreeView *const topTree)
        : topTree_(topTree), mark_(std::make_shared<ToggleButton>()),
          childNodesStackPanel_(std::make_shared<VerticalStackPanel>())
    {
        layout_.setColumnSpacingProperty(2);
        layout_.setRowSpacingProperty(2);
        setChildrenLayoutProperty(&layout_);
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);

        layout_.getColumnsProportionsProperty().Add(Proportion::Auto);
        layout_.getColumnsProportionsProperty().Add(Proportion::Fill);
        layout_.getRowsProportionsProperty().Add(Proportion::Auto);
        layout_.getRowsProportionsProperty().Add(Proportion::Auto);

        mark_->setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        mark_->setVerticalAlignmentProperty(VerticalAlignment::Center);
        mark_->setContentProperty(std::make_shared<Image>());
        mark_->PressedChanged += [this](void *, Events::MyraEventArgs &)
        { childNodesStackPanel_->setVisibleProperty(mark_->getIsPressedProperty()); };
        AddChild(mark_);

        childNodesStackPanel_->setVisibleProperty(false);
        Grid::SetColumn(*childNodesStackPanel_, 1);
        Grid::SetRow(*childNodesStackPanel_, 1);
        AddChild(childNodesStackPanel_);
        UpdateMark();
    }

    bool TreeViewNode::getIsExpandedProperty() const noexcept
    {
        return mark_->getIsToggledProperty();
    }

    void TreeViewNode::setIsExpandedProperty(const bool value)
    {
        mark_->setIsToggledProperty(value);
    }

    std::shared_ptr<ToggleButton> TreeViewNode::getMarkProperty() const
    {
        return mark_;
    }

    std::shared_ptr<VerticalStackPanel> TreeViewNode::getChildNodesStackPanelProperty() const
    {
        return childNodesStackPanel_;
    }

    int TreeViewNode::getContentHeightProperty() const noexcept
    {
        return layout_.GetRowHeight(0);
    }

    int TreeViewNode::getChildNodesCountProperty() const noexcept
    {
        return static_cast<int>(childNodesStackPanel_->getWidgetsProperty().size());
    }

    TreeViewNode *TreeViewNode::getParentNodeProperty() const noexcept
    {
        return parentNode_;
    }

    std::shared_ptr<Widget> TreeViewNode::getContentProperty() const
    {
        return content_;
    }

    void TreeViewNode::setContentProperty(std::shared_ptr<Widget> value)
    {
        if (content_ == value)
        {
            return;
        }
        if (content_)
        {
            static_cast<void>(RemoveChild(content_.get()));
        }

        content_ = std::move(value);
        if (content_)
        {
            Grid::SetColumn(*content_, 1);
            AddChild(content_);
        }
    }

    std::shared_ptr<TreeViewNode> TreeViewNode::AddSubNode(std::shared_ptr<Widget> content)
    {
        auto result = std::make_shared<TreeViewNode>(topTree_);
        result->setContentProperty(std::move(content));
        AddRetainedSubNode(result);
        return result;
    }

    std::shared_ptr<TreeViewNode> TreeViewNode::GetSubNode(const int index) const
    {
        if (index < 0 || index >= getChildNodesCountProperty())
        {
            throw std::out_of_range("TreeViewNode child index is outside the collection.");
        }
        return std::dynamic_pointer_cast<TreeViewNode>(
            childNodesStackPanel_->getWidgetsProperty()[static_cast<std::size_t>(index)]);
    }

    void TreeViewNode::RemoveAllSubNodes()
    {
        for (const std::shared_ptr<Widget> &widget : childNodesStackPanel_->getWidgetsProperty())
        {
            const auto node = std::dynamic_pointer_cast<TreeViewNode>(widget);
            if (node)
            {
                node->parentNode_ = nullptr;
            }
        }
        childNodesStackPanel_->ClearChildren();
        UpdateMark();
    }

    void TreeViewNode::RemoveSubNode(TreeViewNode *const subNode)
    {
        if (subNode == nullptr || !childNodesStackPanel_->RemoveWidget(subNode))
        {
            return;
        }
        subNode->parentNode_ = nullptr;
        UpdateMark();
    }

    void TreeViewNode::RemoveSubNodeAt(const int index)
    {
        const std::shared_ptr<TreeViewNode> subNode = GetSubNode(index);
        RemoveSubNode(subNode.get());
    }

    void TreeViewNode::UpdateMark()
    {
        mark_->setVisibleProperty(getChildNodesCountProperty() > 0);
    }

    std::shared_ptr<Widget> TreeViewNode::CreateCloneInstance() const
    {
        return std::make_shared<TreeViewNode>();
    }

    void TreeViewNode::CopyFrom(const Widget &source)
    {
        ContentControl::CopyFrom(source);
        const auto *const node = dynamic_cast<const TreeViewNode *>(&source);
        if (node == nullptr)
        {
            throw std::invalid_argument("TreeViewNode copy source must be a TreeViewNode.");
        }

        for (int index = 0; index < node->getChildNodesCountProperty(); ++index)
        {
            const std::shared_ptr<TreeViewNode> child = node->GetSubNode(index);
            AddRetainedSubNode(std::dynamic_pointer_cast<TreeViewNode>(child->Clone()));
        }
        setIsExpandedProperty(node->getIsExpandedProperty());
    }

    void TreeViewNode::AddRetainedSubNode(std::shared_ptr<TreeViewNode> subNode)
    {
        if (!subNode)
        {
            throw std::invalid_argument("A TreeViewNode child cannot be null.");
        }
        if (subNode.get() == this)
        {
            throw std::invalid_argument("A TreeViewNode cannot be its own child.");
        }
        for (const TreeViewNode *ancestor = this; ancestor != nullptr; ancestor = ancestor->parentNode_)
        {
            if (ancestor == subNode.get())
            {
                throw std::invalid_argument("Adding an ancestor would create a tree-node ownership cycle.");
            }
        }

        if (subNode->parentNode_ != nullptr && subNode->parentNode_ != this)
        {
            subNode->parentNode_->RemoveSubNode(subNode.get());
        }
        if (subNode->parentNode_ == this)
        {
            return;
        }

        Grid::SetRow(*subNode, getChildNodesCountProperty());
        subNode->parentNode_ = this;
        childNodesStackPanel_->AddWidget(std::move(subNode));
        UpdateMark();
    }
} // namespace Myra::Graphics2D::UI
