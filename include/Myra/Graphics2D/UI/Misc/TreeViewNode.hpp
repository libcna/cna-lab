// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Misc/TreeViewNode.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"
#include "Myra/Graphics2D/UI/Misc/ITreeViewNode.hpp"
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"

namespace Myra::Graphics2D::UI
{
    class TreeView;

    /** @brief One expandable tree row with retained content and an ordered node subtree. */
    class TreeViewNode : public ContentControl, public ITreeViewNode
    {
      public:
        explicit TreeViewNode(TreeView *topTree = nullptr);
        ~TreeViewNode() override = default;

        [[nodiscard]] bool getIsExpandedProperty() const noexcept;
        void setIsExpandedProperty(bool value);
        [[nodiscard]] std::shared_ptr<ToggleButton> getMarkProperty() const;
        [[nodiscard]] std::shared_ptr<VerticalStackPanel> getChildNodesStackPanelProperty() const;
        [[nodiscard]] int getContentHeightProperty() const noexcept;
        [[nodiscard]] int getChildNodesCountProperty() const noexcept override;
        [[nodiscard]] TreeViewNode *getParentNodeProperty() const noexcept;

        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;

        [[nodiscard]] std::shared_ptr<TreeViewNode> AddSubNode(std::shared_ptr<Widget> content) override;
        [[nodiscard]] std::shared_ptr<TreeViewNode> GetSubNode(int index) const override;
        void RemoveAllSubNodes() override;
        void RemoveSubNode(TreeViewNode *subNode);
        void RemoveSubNodeAt(int index);

      protected:
        /** @brief Synchronises mark visibility after a child collection mutation. */
        virtual void UpdateMark();
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        friend class TreeView;

        void AddRetainedSubNode(std::shared_ptr<TreeViewNode> subNode);

        GridLayout layout_;
        TreeView *topTree_ = nullptr;
        TreeViewNode *parentNode_ = nullptr;
        std::shared_ptr<ToggleButton> mark_;
        std::shared_ptr<VerticalStackPanel> childNodesStackPanel_;
        std::shared_ptr<Widget> content_;
        bool rowVisible_ = false;
    };
} // namespace Myra::Graphics2D::UI
