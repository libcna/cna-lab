// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Misc/TreeView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Misc/TreeView.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    TreeView::TreeView()
    {
        setChildrenLayoutProperty(&layout_);
        setAcceptsKeyboardFocusProperty(true);
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    int TreeView::getSubNodesCountProperty() const noexcept
    {
        return static_cast<int>(getChildrenProperty().size());
    }

    int TreeView::getChildNodesCountProperty() const noexcept
    {
        return getSubNodesCountProperty();
    }

    int TreeView::getTotalNodesCountProperty() const noexcept
    {
        return static_cast<int>(allNodes_.size());
    }

    std::shared_ptr<TreeViewNode> TreeView::getSelectedNodeProperty() const
    {
        return selectedNode_;
    }

    void TreeView::setSelectedNodeProperty(std::shared_ptr<TreeViewNode> value)
    {
        if (value == selectedNode_)
        {
            return;
        }
        selectedNode_ = std::move(value);
        Utility::EventsExtensions::Invoke(SelectionChanged, this, InputEventType::SelectionChanged);
    }

    std::shared_ptr<TreeViewNode> TreeView::AddSubNode(std::shared_ptr<Widget> content)
    {
        auto result = std::make_shared<TreeViewNode>(this);
        result->setContentProperty(std::move(content));
        AddRetainedRootNode(result);
        return result;
    }

    std::shared_ptr<TreeViewNode> TreeView::GetSubNode(const int index) const
    {
        if (index < 0 || index >= getSubNodesCountProperty())
        {
            throw std::out_of_range("TreeView root-node index is outside the collection.");
        }
        return std::dynamic_pointer_cast<TreeViewNode>(getChildrenProperty()[static_cast<std::size_t>(index)]);
    }

    std::shared_ptr<TreeViewNode> TreeView::GetNodeByAbsoluteIndex(const int index) const
    {
        if (index < 0 || index >= getTotalNodesCountProperty())
        {
            throw std::out_of_range("TreeView absolute node index is outside the collection.");
        }
        return allNodes_[static_cast<std::size_t>(index)];
    }

    void TreeView::RemoveAllSubNodes()
    {
        const std::vector<std::shared_ptr<Widget>> roots = getChildrenProperty();
        for (const std::shared_ptr<Widget> &root : roots)
        {
            if (const auto node = std::dynamic_pointer_cast<TreeViewNode>(root))
            {
                UnregisterSubtree(node.get());
            }
        }
        ClearChildren();
        hoverRow_ = nullptr;
        setSelectedNodeProperty(nullptr);
    }

    void TreeView::Iterate(const NodePredicate &action) const
    {
        if (!action)
        {
            throw std::invalid_argument("TreeView iteration requires an action.");
        }
        for (const std::shared_ptr<Widget> &root : getChildrenProperty())
        {
            const auto node = std::dynamic_pointer_cast<TreeViewNode>(root);
            if (node && !IterateNode(node, action))
            {
                return;
            }
        }
    }

    void TreeView::ExpandPath(const std::shared_ptr<TreeViewNode> &node)
    {
        if (!node || node->topTree_ != this)
        {
            return;
        }
        for (TreeViewNode *current = node->parentNode_; current != nullptr; current = current->parentNode_)
        {
            current->setIsExpandedProperty(true);
        }
    }

    std::shared_ptr<TreeViewNode> TreeView::FindNode(const NodePredicate &predicate) const
    {
        if (!predicate)
        {
            throw std::invalid_argument("TreeView node lookup requires a predicate.");
        }
        const auto iterator = std::find_if(allNodes_.begin(), allNodes_.end(),
                                           [&predicate](const auto &node) { return predicate(*node); });
        return iterator == allNodes_.end() ? nullptr : *iterator;
    }

    void TreeView::OnKeyDown(const Keys key)
    {
        Widget::OnKeyDown(key);
        if (!selectedNode_)
        {
            return;
        }

        TreeViewNode *const selected = selectedNode_.get();
        TreeViewNode *const parent = selected->parentNode_;
        int index = 0;
        std::vector<std::shared_ptr<Widget>> parentNodes;
        if (parent != nullptr)
        {
            parentNodes = parent->childNodesStackPanel_->getWidgetsProperty();
            const auto iterator = std::find_if(parentNodes.begin(), parentNodes.end(),
                                               [selected](const auto &node) { return node.get() == selected; });
            if (iterator == parentNodes.end())
            {
                return;
            }
            index = static_cast<int>(std::distance(parentNodes.begin(), iterator));
        }

        switch (key)
        {
        case Keys::Enter:
            selected->setIsExpandedProperty(!selected->getIsExpandedProperty());
            break;
        case Keys::Up:
            if (parent != nullptr)
            {
                if (index == 0)
                {
                    setSelectedNodeProperty(
                        FindNode([parent](TreeViewNode &candidate) { return &candidate == parent; }));
                }
                else
                {
                    const auto previous =
                        std::dynamic_pointer_cast<TreeViewNode>(parentNodes[static_cast<std::size_t>(index - 1)]);
                    if (!previous->getIsExpandedProperty() || previous->getChildNodesCountProperty() == 0)
                    {
                        setSelectedNodeProperty(previous);
                    }
                    else
                    {
                        setSelectedNodeProperty(previous->GetSubNode(previous->getChildNodesCountProperty() - 1));
                    }
                }
            }
            break;
        case Keys::Down:
            if (selected->getIsExpandedProperty() && selected->getChildNodesCountProperty() > 0)
            {
                setSelectedNodeProperty(selected->GetSubNode(0));
            }
            else if (parent != nullptr && index + 1 < static_cast<int>(parentNodes.size()))
            {
                setSelectedNodeProperty(
                    std::dynamic_pointer_cast<TreeViewNode>(parentNodes[static_cast<std::size_t>(index + 1)]));
            }
            else if (parent != nullptr && index + 1 >= static_cast<int>(parentNodes.size()) &&
                     parent->parentNode_ != nullptr)
            {
                TreeViewNode *const grandparent = parent->parentNode_;
                const std::vector<std::shared_ptr<Widget>> &grandparentNodes =
                    grandparent->childNodesStackPanel_->getWidgetsProperty();
                const auto parentIterator = std::find_if(grandparentNodes.begin(), grandparentNodes.end(),
                                                         [parent](const auto &node) { return node.get() == parent; });
                if (parentIterator != grandparentNodes.end() && std::next(parentIterator) != grandparentNodes.end())
                {
                    setSelectedNodeProperty(std::dynamic_pointer_cast<TreeViewNode>(*std::next(parentIterator)));
                }
            }
            break;
        default:
            break;
        }
    }

    void TreeView::InternalArrange()
    {
        Widget::InternalArrange();
        RefreshRowVisibility();
    }

    std::shared_ptr<Widget> TreeView::CreateCloneInstance() const
    {
        return std::make_shared<TreeView>();
    }

    void TreeView::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const treeView = dynamic_cast<const TreeView *>(&source);
        if (treeView == nullptr)
        {
            throw std::invalid_argument("TreeView copy source must be a TreeView.");
        }

        for (const std::shared_ptr<Widget> &root : treeView->getChildrenProperty())
        {
            AddRetainedRootNode(std::dynamic_pointer_cast<TreeViewNode>(root->Clone()));
        }
    }

    bool TreeView::IterateNode(const std::shared_ptr<TreeViewNode> &node, const NodePredicate &action)
    {
        if (!action(*node))
        {
            return false;
        }
        for (const std::shared_ptr<Widget> &child : node->childNodesStackPanel_->getWidgetsProperty())
        {
            const auto childNode = std::dynamic_pointer_cast<TreeViewNode>(child);
            if (childNode && !IterateNode(childNode, action))
            {
                return false;
            }
        }
        return true;
    }

    void TreeView::UpdateRowVisibility(const std::shared_ptr<TreeViewNode> &node)
    {
        node->rowVisible_ = true;
        if (!node->getIsExpandedProperty())
        {
            return;
        }
        for (const std::shared_ptr<Widget> &child : node->childNodesStackPanel_->getWidgetsProperty())
        {
            if (const auto childNode = std::dynamic_pointer_cast<TreeViewNode>(child))
            {
                UpdateRowVisibility(childNode);
            }
        }
    }

    void TreeView::RegisterSubtree(const std::shared_ptr<TreeViewNode> &node)
    {
        if (!node)
        {
            throw std::invalid_argument("A TreeView node cannot be null.");
        }
        node->topTree_ = this;
        const auto existing = std::find_if(allNodes_.begin(), allNodes_.end(),
                                           [&node](const auto &item) { return item.get() == node.get(); });
        if (existing == allNodes_.end())
        {
            allNodes_.push_back(node);
        }
        for (const std::shared_ptr<Widget> &child : node->childNodesStackPanel_->getWidgetsProperty())
        {
            if (const auto childNode = std::dynamic_pointer_cast<TreeViewNode>(child))
            {
                childNode->parentNode_ = node.get();
                RegisterSubtree(childNode);
            }
        }
    }

    void TreeView::UnregisterSubtree(TreeViewNode *const node)
    {
        if (node == nullptr)
        {
            return;
        }
        const std::vector<std::shared_ptr<Widget>> children = node->childNodesStackPanel_->getWidgetsProperty();
        for (const std::shared_ptr<Widget> &child : children)
        {
            if (const auto childNode = std::dynamic_pointer_cast<TreeViewNode>(child))
            {
                UnregisterSubtree(childNode.get());
            }
        }
        if (selectedNode_.get() == node)
        {
            setSelectedNodeProperty(nullptr);
        }
        if (hoverRow_ == node)
        {
            hoverRow_ = nullptr;
        }
        allNodes_.erase(
            std::remove_if(allNodes_.begin(), allNodes_.end(), [node](const auto &item) { return item.get() == node; }),
            allNodes_.end());
        node->topTree_ = nullptr;
    }

    void TreeView::AddRetainedRootNode(std::shared_ptr<TreeViewNode> node)
    {
        if (!node)
        {
            throw std::invalid_argument("A TreeView root node cannot be null.");
        }
        if (node->parentNode_ != nullptr)
        {
            node->parentNode_->RemoveSubNode(node.get());
        }
        Grid::SetRow(*node, getSubNodesCountProperty());
        AddChild(node);
        RegisterSubtree(node);
    }

    void TreeView::RefreshRowVisibility()
    {
        for (const std::shared_ptr<TreeViewNode> &node : allNodes_)
        {
            node->rowVisible_ = false;
        }
        for (const std::shared_ptr<Widget> &root : getChildrenProperty())
        {
            if (const auto node = std::dynamic_pointer_cast<TreeViewNode>(root))
            {
                UpdateRowVisibility(node);
            }
        }
    }
} // namespace Myra::Graphics2D::UI
