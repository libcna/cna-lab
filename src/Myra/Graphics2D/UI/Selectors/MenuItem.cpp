// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/MenuItem.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"

#include <cctype>
#include <utility>

#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    MenuItem::MenuItem() : MenuItem(std::string()) {}

    MenuItem::MenuItem(std::string id) : MenuItem(std::move(id), std::string()) {}

    MenuItem::MenuItem(std::string id, std::string text) : MenuItem(std::move(id), std::move(text), std::any()) {}

    MenuItem::MenuItem(std::string id, std::string text, std::any tag)
        : subMenu_(std::make_shared<VerticalMenu>()), tag_(std::move(tag))
    {
        setIdProperty(std::move(id));
        setTextProperty(std::move(text));
    }

    const std::optional<std::string> &MenuItem::getIdProperty() const noexcept
    {
        return MML::BaseObject::getIdProperty();
    }

    void MenuItem::setIdProperty(std::optional<std::string> value)
    {
        MML::BaseObject::setIdProperty(std::move(value));
    }

    const std::optional<std::string> &MenuItem::getTextProperty() const noexcept
    {
        return text_;
    }

    void MenuItem::setTextProperty(std::optional<std::string> value)
    {
        if (value == text_)
        {
            return;
        }

        text_ = std::move(value);
        displayTextDirty_ = true;
        underscoreChar_.reset();
        if (text_)
        {
            const std::size_t underscoreIndex = text_->find('&');
            if (underscoreIndex != std::string::npos && underscoreIndex + 1 < text_->size())
            {
                const unsigned char character = static_cast<unsigned char>((*text_)[underscoreIndex + 1]);
                underscoreChar_ = static_cast<char>(std::tolower(character));
            }
        }

        FireChanged();
    }

    const std::optional<std::string> &MenuItem::getDisplayTextProperty() const
    {
        UpdateDisplayText();
        return displayText_;
    }

    const std::optional<std::string> &MenuItem::getDisabledDisplayTextProperty() const
    {
        UpdateDisplayText();
        return disabledDisplayText_;
    }

    const std::any &MenuItem::getTagProperty() const noexcept
    {
        return tag_;
    }

    void MenuItem::setTagProperty(std::any value)
    {
        tag_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IImage> MenuItem::getImageProperty() const
    {
        return image_;
    }

    void MenuItem::setImageProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == image_)
        {
            return;
        }

        image_ = std::move(value);
        FireChanged();
    }

    const std::optional<std::string> &MenuItem::getShortcutTextProperty() const noexcept
    {
        return shortcutText_;
    }

    void MenuItem::setShortcutTextProperty(std::optional<std::string> value)
    {
        if (value == shortcutText_)
        {
            return;
        }

        shortcutText_ = std::move(value);
        FireChanged();
    }

    bool MenuItem::getEnabledProperty() const noexcept
    {
        return enabled_;
    }

    void MenuItem::setEnabledProperty(const bool value) noexcept
    {
        enabled_ = value;
    }

    std::shared_ptr<VerticalMenu> MenuItem::getSubMenuProperty() const
    {
        return subMenu_;
    }

    const MenuItemCollection &MenuItem::getItemsProperty() const noexcept
    {
        return subMenu_->getItemsProperty();
    }

    MenuItemCollection &MenuItem::getItemsProperty() noexcept
    {
        return subMenu_->getItemsProperty();
    }

    bool MenuItem::getCanOpenProperty() const noexcept
    {
        return subMenu_->getItemsProperty().getCountProperty() > 0;
    }

    MenuItem *MenuItem::FindMenuItemById(const std::string &id) noexcept
    {
        if (getIdProperty() == id)
        {
            return this;
        }
        return subMenu_->FindMenuItemById(id);
    }

    const MenuItem *MenuItem::FindMenuItemById(const std::string &id) const noexcept
    {
        if (getIdProperty() == id)
        {
            return this;
        }
        return subMenu_->FindMenuItemById(id);
    }

    Menu *MenuItem::getMenuProperty() const noexcept
    {
        return menu_;
    }

    void MenuItem::setMenuProperty(Menu *const value) noexcept
    {
        menu_ = value;
    }

    std::optional<char> MenuItem::getUnderscoreCharProperty() const noexcept
    {
        return underscoreChar_;
    }

    int MenuItem::getIndexProperty() const noexcept
    {
        return index_;
    }

    void MenuItem::setIndexProperty(const int value) noexcept
    {
        index_ = value;
    }

    void MenuItem::FireSelected()
    {
        Utility::EventsExtensions::Invoke(Selected, this, InputEventType::SelectionChanged);
    }

    void MenuItem::OnIdChanged()
    {
        MML::BaseObject::OnIdChanged();
        FireChanged();
    }

    void MenuItem::UpdateDisplayText() const
    {
        if (!displayTextDirty_)
        {
            return;
        }

        if (!text_ || !underscoreChar_)
        {
            displayText_ = text_;
            disabledDisplayText_ = text_;
        }
        else
        {
            const std::size_t underscoreIndex = text_->find('&');
            const std::string plainText = text_->substr(0, underscoreIndex) + text_->substr(underscoreIndex + 1);
            disabledDisplayText_ = plainText;
            // RichText highlighting is deferred until the font/style core exists. Its no-style upstream
            // fallback is this same marker-free string.
            displayText_ = plainText;
        }

        displayTextDirty_ = false;
    }

    void MenuItem::FireChanged()
    {
        Utility::EventsExtensions::Invoke(Changed, this, InputEventType::ValueChanged);
    }
} // namespace Myra::Graphics2D::UI
