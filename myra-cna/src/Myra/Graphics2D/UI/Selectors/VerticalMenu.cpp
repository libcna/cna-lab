// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/VerticalMenu.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"

namespace Myra::Graphics2D::UI
{
    VerticalMenu::VerticalMenu()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        setVerticalAlignmentProperty(VerticalAlignment::Top);
    }

    Orientation VerticalMenu::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    void VerticalMenu::OnKeyDown(const Microsoft::Xna::Framework::Input::Keys key)
    {
        Menu::OnKeyDown(key);
        using Microsoft::Xna::Framework::Input::Keys;
        if (key == Keys::Up)
        {
            MoveHover(-1);
        }
        else if (key == Keys::Down)
        {
            MoveHover(1);
        }
    }
} // namespace Myra::Graphics2D::UI
