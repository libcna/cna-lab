// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Misc/TreeView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Layouts/StackPanelLayout.hpp"
#include "Myra/Graphics2D/UI/Misc/ITreeViewNode.hpp"
#include "Myra/Graphics2D/UI/Misc/TreeViewNode.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Hierarchical owner of expandable TreeViewNode roots and selection state. */
    class TreeView : public Widget, public ITreeViewNode
    {
      public:
        using NodePredicate = std::function<bool(TreeViewNode &)>;

        TreeView();
        ~TreeView() override = default;

        Events::MyraEventHandler SelectionChanged;

        [[nodiscard]] int getSubNodesCountProperty() const noexcept;
        [[nodiscard]] int getChildNodesCountProperty() const noexcept override;
        [[nodiscard]] int getTotalNodesCountProperty() const noexcept;
        [[nodiscard]] std::shared_ptr<TreeViewNode> getSelectedNodeProperty() const;
        void setSelectedNodeProperty(std::shared_ptr<TreeViewNode> value);
        [[nodiscard]] TreeViewNode *getHoverRowProperty() const noexcept;

        [[nodiscard]] std::shared_ptr<TreeViewNode> AddSubNode(std::shared_ptr<Widget> content) override;
        [[nodiscard]] std::shared_ptr<TreeViewNode> GetSubNode(int index) const override;
        [[nodiscard]] std::shared_ptr<TreeViewNode> GetNodeByAbsoluteIndex(int index) const;
        void RemoveAllSubNodes() override;
        void Iterate(const NodePredicate &action) const;
        void ExpandPath(const std::shared_ptr<TreeViewNode> &node);
        [[nodiscard]] std::shared_ptr<TreeViewNode> FindNode(const NodePredicate &predicate) const;

        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;
        void OnMouseMoved() override;
        void OnMouseLeft() override;
        void OnTouchDown() override;
        void OnTouchDoubleClick() override;

      protected:
        void InternalArrange() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        friend class TreeViewNode;

        static bool IterateNode(const std::shared_ptr<TreeViewNode> &node, const NodePredicate &action);
        static void UpdateRowVisibility(const std::shared_ptr<TreeViewNode> &node);
        void RegisterSubtree(const std::shared_ptr<TreeViewNode> &node);
        void UnregisterSubtree(TreeViewNode *node);
        void AddRetainedRootNode(std::shared_ptr<TreeViewNode> node);
        void RefreshRowVisibility();
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle BuildRowRect(TreeViewNode &node);
        void SetHoverRow(Microsoft::Xna::Framework::Point position);

        StackPanelLayout layout_{Orientation::Vertical};
        std::vector<std::shared_ptr<TreeViewNode>> allNodes_;
        std::shared_ptr<TreeViewNode> selectedNode_;
        TreeViewNode *hoverRow_ = nullptr;
    };
} // namespace Myra::Graphics2D::UI
