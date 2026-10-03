// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Misc/ITreeViewNode.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

namespace Myra::Graphics2D::UI
{
    class TreeViewNode;
    class Widget;

    /** @brief Contract for a tree item that owns an ordered collection of child nodes. */
    class ITreeViewNode
    {
      public:
        virtual ~ITreeViewNode() = default;

        [[nodiscard]] virtual int getChildNodesCountProperty() const noexcept = 0;
        [[nodiscard]] virtual std::shared_ptr<TreeViewNode> AddSubNode(std::shared_ptr<Widget> content) = 0;
        [[nodiscard]] virtual std::shared_ptr<TreeViewNode> GetSubNode(int index) const = 0;
        virtual void RemoveAllSubNodes() = 0;
    };
} // namespace Myra::Graphics2D::UI
