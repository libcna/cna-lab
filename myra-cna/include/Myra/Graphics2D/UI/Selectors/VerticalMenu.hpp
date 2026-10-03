// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/VerticalMenu.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/Selectors/Menu.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Menu with vertical logical navigation. */
    class VerticalMenu final : public Menu
    {
      public:
        VerticalMenu();
        ~VerticalMenu() override = default;

        [[nodiscard]] Orientation getOrientationProperty() const noexcept override;
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;
    };
} // namespace Myra::Graphics2D::UI
