// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/IMenuItem.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <optional>

#include "Myra/MML/IItemWithId.hpp"

namespace Myra::Graphics2D::UI
{
    class Menu;

    /** @brief Contract shared by items owned by a menu. */
    class IMenuItem : public MML::IItemWithId
    {
      public:
        ~IMenuItem() override = default;

        [[nodiscard]] virtual Menu *getMenuProperty() const noexcept = 0;
        virtual void setMenuProperty(Menu *value) noexcept = 0;
        [[nodiscard]] virtual std::optional<char> getUnderscoreCharProperty() const noexcept = 0;
        [[nodiscard]] virtual int getIndexProperty() const noexcept = 0;
        virtual void setIndexProperty(int value) noexcept = 0;
    };
} // namespace Myra::Graphics2D::UI
