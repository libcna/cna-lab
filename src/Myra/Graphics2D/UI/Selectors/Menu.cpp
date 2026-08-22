// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/Menu.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/Menu.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"
#include "Myra/Utility/InputExtension.hpp"

namespace Myra::Graphics2D::UI
{
    Menu::Menu()
    {
        items_.CollectionChanged.emplace_back([this](void *, const auto &) { OnItemsCollectionChanged(); });
        setAcceptsKeyboardFocusProperty(true);
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    Menu::~Menu()
    {
        for (const std::shared_ptr<IMenuItem> &item : attachedItems_)
        {
            if (item && item->getMenuProperty() == this)
            {
                item->setMenuProperty(nullptr);
            }
        }
    }

    const Menu::ItemCollection &Menu::getItemsProperty() const noexcept
    {
        return items_;
    }

    Menu::ItemCollection &Menu::getItemsProperty() noexcept
    {
        return items_;
    }

    bool Menu::getHoverIndexCanBeNullProperty() const noexcept
    {
        return hoverIndexCanBeNull_;
    }

    void Menu::setHoverIndexCanBeNullProperty(const bool value) noexcept
    {
        hoverIndexCanBeNull_ = value;
    }

    const std::optional<int> &Menu::getHoverIndexProperty() const noexcept
    {
        return hoverIndex_;
    }

    void Menu::setHoverIndexProperty(std::optional<int> value)
    {
        value = NormalizeIndex(value);
        if (value && !GetMenuItem(value) && hoverIndexCanBeNull_)
        {
            value.reset();
        }
        if (value == hoverIndex_)
        {
            return;
        }

        hoverIndex_ = value;
        const std::shared_ptr<MenuItem> item = GetMenuItem(hoverIndex_);
        if (getIsOpenProperty() && item && item.get() != openMenuItem_ && item->getCanOpenProperty())
        {
            setSelectedIndexProperty(hoverIndex_);
        }
    }

    const std::optional<int> &Menu::getSelectedIndexProperty() const noexcept
    {
        return selectedIndex_;
    }

    void Menu::setSelectedIndexProperty(std::optional<int> value)
    {
        value = NormalizeIndex(value);
        if (value == selectedIndex_)
        {
            return;
        }

        if (openMenuItem_)
        {
            openMenuItem_->getSubMenuProperty()->Close();
            openMenuItem_ = nullptr;
        }
        selectedIndex_ = value;
        const std::shared_ptr<MenuItem> item = GetMenuItem(selectedIndex_);
        if (item && item->getCanOpenProperty())
        {
            openMenuItem_ = item.get();
        }
    }

    MenuItem *Menu::getOpenMenuItemProperty() const noexcept
    {
        return openMenuItem_;
    }

    bool Menu::getIsOpenProperty() const noexcept
    {
        return openMenuItem_ != nullptr;
    }

    void Menu::Close()
    {
        if (openMenuItem_)
        {
            openMenuItem_->getSubMenuProperty()->Close();
            openMenuItem_ = nullptr;
        }
        hoverIndex_.reset();
        selectedIndex_.reset();
    }

    void Menu::Click(const std::optional<int> index)
    {
        const std::shared_ptr<MenuItem> item = GetMenuItem(NormalizeIndex(index));
        if (!item)
        {
            return;
        }

        if (item->getCanOpenProperty())
        {
            setHoverIndexProperty(index);
            setSelectedIndexProperty(index);
            return;
        }

        Close();
        item->FireSelected();
    }

    MenuItem *Menu::FindMenuItemById(const std::string &id) noexcept
    {
        for (int index = 0; index < items_.getCountProperty(); ++index)
        {
            const std::shared_ptr<MenuItem> item = GetMenuItem(index);
            if (item)
            {
                if (MenuItem *const result = item->FindMenuItemById(id))
                {
                    return result;
                }
            }
        }
        return nullptr;
    }

    const MenuItem *Menu::FindMenuItemById(const std::string &id) const noexcept
    {
        for (int index = 0; index < items_.getCountProperty(); ++index)
        {
            const std::shared_ptr<MenuItem> item = GetMenuItem(index);
            if (item)
            {
                if (const MenuItem *const result = item->FindMenuItemById(id))
                {
                    return result;
                }
            }
        }
        return nullptr;
    }

    void Menu::MoveHover(const int delta)
    {
        const int count = items_.getCountProperty();
        if (count == 0)
        {
            return;
        }

        int hoverIndex = selectedIndex_.value_or(hoverIndex_.value_or(-1));
        int iterations = 0;
        while (iterations <= count)
        {
            hoverIndex += delta;
            if (hoverIndex < 0)
            {
                hoverIndex = count - 1;
            }
            if (hoverIndex >= count)
            {
                hoverIndex = 0;
            }
            if (GetMenuItem(hoverIndex))
            {
                setHoverIndexProperty(hoverIndex);
                return;
            }
            ++iterations;
        }
    }

    void Menu::OnKeyDown(const Microsoft::Xna::Framework::Input::Keys key)
    {
        Widget::OnKeyDown(key);

        using Microsoft::Xna::Framework::Input::Keys;
        if (key == Keys::Enter || key == Keys::Space)
        {
            if (const std::shared_ptr<MenuItem> item = GetMenuItem(hoverIndex_); item && !item->getCanOpenProperty())
            {
                Click(hoverIndex_);
                return;
            }
        }

        if (const std::optional<char> character = Utility::InputExtension::ToChar(key, false))
        {
            const char lowerCharacter = static_cast<char>(std::tolower(static_cast<unsigned char>(*character)));
            for (int index = 0; index < items_.getCountProperty(); ++index)
            {
                const std::shared_ptr<MenuItem> item = GetMenuItem(index);
                if (item && item->getUnderscoreCharProperty() == lowerCharacter)
                {
                    Click(index);
                    return;
                }
            }
        }

        if (openMenuItem_)
        {
            openMenuItem_->getSubMenuProperty()->OnKeyDown(key);
        }
    }

    void Menu::OnItemsCollectionChanged()
    {
        SynchronizeItemState();
        InvalidateMeasure();
    }

    void Menu::SynchronizeItemState()
    {
        for (const std::shared_ptr<IMenuItem> &item : attachedItems_)
        {
            if (item && !items_.Contains(item) && item->getMenuProperty() == this)
            {
                item->setMenuProperty(nullptr);
            }
        }

        attachedItems_.clear();
        for (int index = 0; index < items_.getCountProperty(); ++index)
        {
            const std::shared_ptr<IMenuItem> &item = items_[index];
            if (!item)
            {
                throw std::invalid_argument("A Menu cannot retain a null IMenuItem.");
            }
            item->setMenuProperty(this);
            item->setIndexProperty(index);
            attachedItems_.push_back(item);
        }

        hoverIndex_ = NormalizeIndex(hoverIndex_);
        selectedIndex_ = NormalizeIndex(selectedIndex_);
        if (openMenuItem_ && std::none_of(attachedItems_.begin(), attachedItems_.end(),
                                          [this](const auto &item) { return item.get() == openMenuItem_; }))
        {
            openMenuItem_ = nullptr;
        }
    }

    std::optional<int> Menu::NormalizeIndex(const std::optional<int> value) const noexcept
    {
        if (!value || *value < 0 || *value >= items_.getCountProperty())
        {
            return std::nullopt;
        }
        return value;
    }

    std::shared_ptr<MenuItem> Menu::GetMenuItem(const std::optional<int> index) const
    {
        if (!index || *index < 0 || *index >= items_.getCountProperty())
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<MenuItem>(items_[*index]);
    }
} // namespace Myra::Graphics2D::UI
