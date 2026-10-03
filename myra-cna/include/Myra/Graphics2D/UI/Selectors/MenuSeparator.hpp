// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/MenuSeparator.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "Myra/Graphics2D/UI/Selectors/IMenuItem.hpp"
#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Non-interactive menu item that owns the separator visual assigned by Menu. */
    class MenuSeparator final : public IMenuItem
    {
      public:
        /** @brief Internal separator visual. It remains unset until the owning Menu creates it. */
        std::shared_ptr<SeparatorWidget> Separator;

        ~MenuSeparator() override = default;

        [[nodiscard]] const std::optional<std::string> &getIdProperty() const noexcept override { return id_; }
        void setIdProperty(std::optional<std::string> value) override { id_ = std::move(value); }

        [[nodiscard]] Menu *getMenuProperty() const noexcept override { return menu_; }
        void setMenuProperty(Menu *const value) noexcept override { menu_ = value; }
        [[nodiscard]] std::optional<char> getUnderscoreCharProperty() const noexcept override { return std::nullopt; }
        [[nodiscard]] int getIndexProperty() const noexcept override { return index_; }
        void setIndexProperty(const int value) noexcept override { index_ = value; }

      private:
        std::optional<std::string> id_;
        Menu *menu_ = nullptr;
        int index_ = 0;
    };
} // namespace Myra::Graphics2D::UI
