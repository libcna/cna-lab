// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/LayoutUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"

namespace Myra::Graphics2D::UI
{
    class LayoutUtils final
    {
    public:
        LayoutUtils() = delete;

        [[nodiscard]] static Microsoft::Xna::Framework::Rectangle Align(
            const Microsoft::Xna::Framework::Point& containerSize,
            const Microsoft::Xna::Framework::Point& controlSize,
            const HorizontalAlignment horizontalAlignment,
            const VerticalAlignment verticalAlignment)
        {
            Microsoft::Xna::Framework::Rectangle result(0, 0, controlSize.X, controlSize.Y);
            switch (horizontalAlignment)
            {
            case HorizontalAlignment::Center: result.X = (containerSize.X - controlSize.X) / 2; break;
            case HorizontalAlignment::Right: result.X = containerSize.X - controlSize.X; break;
            case HorizontalAlignment::Stretch: result.Width = containerSize.X; break;
            case HorizontalAlignment::Left: break;
            }
            switch (verticalAlignment)
            {
            case VerticalAlignment::Center: result.Y = (containerSize.Y - controlSize.Y) / 2; break;
            case VerticalAlignment::Bottom: result.Y = containerSize.Y - controlSize.Y; break;
            case VerticalAlignment::Stretch: result.Height = containerSize.Y; break;
            case VerticalAlignment::Top: break;
            }
            return result;
        }
    };
}
