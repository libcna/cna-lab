// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/ILayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <vector>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace Myra::Graphics2D::UI
{
    class Widget;

    /** @brief Defines a layout that measures and arranges widgets. */
    class ILayout
    {
    public:
        virtual ~ILayout() = default;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Point Measure(
            const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Point availableSize) = 0;

        virtual void Arrange(
            const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Rectangle bounds) = 0;
    };
}
