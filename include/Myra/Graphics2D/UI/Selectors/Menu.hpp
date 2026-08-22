// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/Menu.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Myra/Graphics2D/UI/Selectors/IMenuItem.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D::UI
{
    class MenuItem;

    /** @brief Base retained menu state with item ownership and keyboard navigation. */
    class Menu : public Widget
    {
      public:
        using ItemCollection = MenuItemCollection;

        Menu();
        ~Menu() override;

        [[nodiscard]] virtual Orientation getOrientationProperty() const noexcept = 0;
        [[nodiscard]] const ItemCollection &getItemsProperty() const noexcept;
        [[nodiscard]] ItemCollection &getItemsProperty() noexcept;
        [[nodiscard]] bool getHoverIndexCanBeNullProperty() const noexcept;
        void setHoverIndexCanBeNullProperty(bool value) noexcept;
        [[nodiscard]] const std::optional<int> &getHoverIndexProperty() const noexcept;
        void setHoverIndexProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int> &getSelectedIndexProperty() const noexcept;
        void setSelectedIndexProperty(std::optional<int> value);
        [[nodiscard]] MenuItem *getOpenMenuItemProperty() const noexcept;
        [[nodiscard]] bool getIsOpenProperty() const noexcept;

        /** @brief Closes retained submenu state and clears the logical hover/selection indices. */
        void Close();
        /** @brief Executes a logical menu item or opens its nested submenu. */
        void Click(std::optional<int> index);
        /** @brief Searches this menu and all retained submenus by optional item identifier. */
        [[nodiscard]] MenuItem *FindMenuItemById(const std::string &id) noexcept;
        [[nodiscard]] const MenuItem *FindMenuItemById(const std::string &id) const noexcept;
        /** @brief Advances logical hover by @p delta, wrapping and skipping separators. */
        void MoveHover(int delta);
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

      private:
        void OnItemsCollectionChanged();
        void SynchronizeItemState();
        [[nodiscard]] std::optional<int> NormalizeIndex(std::optional<int> value) const noexcept;
        [[nodiscard]] std::shared_ptr<MenuItem> GetMenuItem(std::optional<int> index) const;

        ItemCollection items_;
        std::vector<std::shared_ptr<IMenuItem>> attachedItems_;
        std::optional<int> hoverIndex_;
        std::optional<int> selectedIndex_;
        MenuItem *openMenuItem_ = nullptr;
        bool hoverIndexCanBeNull_ = true;
    };
} // namespace Myra::Graphics2D::UI
