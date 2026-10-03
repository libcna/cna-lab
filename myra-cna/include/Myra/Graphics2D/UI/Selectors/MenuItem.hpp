// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/MenuItem.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <memory>
#include <optional>
#include <string>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Selectors/IMenuItem.hpp"
#include "Myra/MML/BaseObject.hpp"

namespace Myra::Graphics2D
{
    class IImage;
}

namespace Myra::Graphics2D::UI
{
    class VerticalMenu;

    /** @brief Style-independent data and command state for one menu entry. */
    class MenuItem final : public MML::BaseObject, public IMenuItem
    {
      public:
        MenuItem();
        explicit MenuItem(std::string id);
        MenuItem(std::string id, std::string text);
        MenuItem(std::string id, std::string text, std::any tag);
        ~MenuItem() override = default;

        Events::MyraEventHandler Selected;
        Events::MyraEventHandler Changed;

        using MML::BaseObject::setIdProperty;

        [[nodiscard]] const std::optional<std::string> &getIdProperty() const noexcept override;
        void setIdProperty(std::optional<std::string> value) override;

        [[nodiscard]] const std::optional<std::string> &getTextProperty() const noexcept;
        void setTextProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string> &getDisplayTextProperty() const;
        [[nodiscard]] const std::optional<std::string> &getDisabledDisplayTextProperty() const;
        [[nodiscard]] const std::any &getTagProperty() const noexcept;
        void setTagProperty(std::any value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getImageProperty() const;
        void setImageProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] const std::optional<std::string> &getShortcutTextProperty() const noexcept;
        void setShortcutTextProperty(std::optional<std::string> value);
        [[nodiscard]] bool getEnabledProperty() const noexcept;
        void setEnabledProperty(bool value) noexcept;
        [[nodiscard]] std::shared_ptr<VerticalMenu> getSubMenuProperty() const;
        [[nodiscard]] const MenuItemCollection &getItemsProperty() const noexcept;
        [[nodiscard]] MenuItemCollection &getItemsProperty() noexcept;
        [[nodiscard]] bool getCanOpenProperty() const noexcept;
        [[nodiscard]] MenuItem *FindMenuItemById(const std::string &id) noexcept;
        [[nodiscard]] const MenuItem *FindMenuItemById(const std::string &id) const noexcept;

        [[nodiscard]] Menu *getMenuProperty() const noexcept override;
        void setMenuProperty(Menu *value) noexcept override;
        [[nodiscard]] std::optional<char> getUnderscoreCharProperty() const noexcept override;
        [[nodiscard]] int getIndexProperty() const noexcept override;
        void setIndexProperty(int value) noexcept override;

        /** @brief Raises the command-selection event used by the future Menu visual layer. */
        void FireSelected();

      protected:
        void OnIdChanged() override;

      private:
        void UpdateDisplayText() const;
        void FireChanged();

        std::optional<std::string> text_;
        std::optional<std::string> shortcutText_;
        std::shared_ptr<Graphics2D::IImage> image_;
        std::shared_ptr<VerticalMenu> subMenu_;
        std::any tag_;
        Menu *menu_ = nullptr;
        std::optional<char> underscoreChar_;
        int index_ = 0;
        bool enabled_ = true;
        mutable bool displayTextDirty_ = true;
        mutable std::optional<std::string> displayText_;
        mutable std::optional<std::string> disabledDisplayText_;
    };
} // namespace Myra::Graphics2D::UI
