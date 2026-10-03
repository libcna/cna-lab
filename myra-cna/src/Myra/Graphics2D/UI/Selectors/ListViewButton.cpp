// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ListViewButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"

namespace Myra::Graphics2D::UI
{
    Widget *ListViewButton::getButtonsContainerProperty() const noexcept
    {
        return buttonsContainer_;
    }

    void ListViewButton::setButtonsContainerProperty(Widget *const value) noexcept
    {
        buttonsContainer_ = value;
    }

    bool ListViewButton::getIsPressedProperty() const noexcept
    {
        return ToggleButton::getIsPressedProperty();
    }

    void ListViewButton::setIsPressedProperty(const bool value)
    {
        Widget *const parent = getParentProperty();
        if (getIsPressedProperty() && parent != nullptr)
        {
            bool anotherPressed = false;
            for (const std::shared_ptr<Widget> &child : GetTopParent()->getChildrenCopyProperty())
            {
                ListViewButton *listViewButton = dynamic_cast<ListViewButton *>(child.get());
                if (listViewButton == this)
                {
                    continue;
                }
                if (listViewButton == nullptr)
                {
                    listViewButton = child->FindChild<ListViewButton>();
                    if (listViewButton == nullptr || listViewButton == this)
                    {
                        continue;
                    }
                }
                if (listViewButton->getIsPressedProperty())
                {
                    anotherPressed = true;
                    break;
                }
            }
            if (!anotherPressed)
            {
                return;
            }
        }

        ToggleButton::setIsPressedProperty(value);
    }

    void ListViewButton::OnPressedChanged()
    {
        ToggleButton::OnPressedChanged();

        if (getParentProperty() == nullptr || !getIsPressedProperty())
        {
            return;
        }

        for (const std::shared_ptr<Widget> &child : GetTopParent()->getChildrenCopyProperty())
        {
            ListViewButton *listViewButton = dynamic_cast<ListViewButton *>(child.get());
            if (listViewButton == this)
            {
                continue;
            }
            if (listViewButton == nullptr)
            {
                listViewButton = child->FindChild<ListViewButton>();
                if (listViewButton == nullptr || listViewButton == this)
                {
                    continue;
                }
            }
            listViewButton->setIsPressedProperty(false);
        }
    }

    std::shared_ptr<Widget> ListViewButton::CreateCloneInstance() const
    {
        return std::make_shared<ListViewButton>();
    }

    Widget *ListViewButton::GetTopParent() const noexcept
    {
        return buttonsContainer_ != nullptr ? buttonsContainer_ : getParentProperty();
    }
} // namespace Myra::Graphics2D::UI
