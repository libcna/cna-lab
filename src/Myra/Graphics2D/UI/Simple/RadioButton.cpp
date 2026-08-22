// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/RadioButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/RadioButton.hpp"

namespace Myra::Graphics2D::UI
{
    bool RadioButton::getIsPressedProperty() const noexcept
    {
        return CheckButtonBase::getIsPressedProperty();
    }

    void RadioButton::setIsPressedProperty(const bool value)
    {
        Widget *const parent = getParentProperty();
        if (getIsPressedProperty() && parent != nullptr)
        {
            bool anotherPressed = false;
            for (const std::shared_ptr<Widget> &child : parent->getChildrenCopyProperty())
            {
                const auto *const radioButton = dynamic_cast<const RadioButton *>(child.get());
                if (radioButton != nullptr && radioButton != this && radioButton->getIsPressedProperty())
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

        CheckButtonBase::setIsPressedProperty(value);
    }

    void RadioButton::OnPressedChanged()
    {
        CheckButtonBase::OnPressedChanged();
        Widget *const parent = getParentProperty();
        if (parent == nullptr || !getIsPressedProperty())
        {
            return;
        }

        for (const std::shared_ptr<Widget> &child : parent->getChildrenCopyProperty())
        {
            auto *const radioButton = dynamic_cast<RadioButton *>(child.get());
            if (radioButton != nullptr && radioButton != this)
            {
                radioButton->setIsPressedProperty(false);
            }
        }
    }

    std::shared_ptr<Widget> RadioButton::CreateCloneInstance() const
    {
        return std::make_shared<RadioButton>();
    }
} // namespace Myra::Graphics2D::UI
