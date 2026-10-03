// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/ToggleButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"

#include <utility>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    ToggleButton::ToggleButton()
        : IsToggledChanged(PressedChanged), layout_(*this)
    {
        setChildrenLayoutProperty(&layout_);
    }

    bool ToggleButton::getIsToggledProperty() const noexcept
    {
        return getIsPressedProperty();
    }

    void ToggleButton::setIsToggledProperty(const bool value)
    {
        setIsPressedProperty(value);
    }

    std::shared_ptr<Widget> ToggleButton::getContentProperty() const
    {
        return layout_.getChildProperty();
    }

    void ToggleButton::setContentProperty(std::shared_ptr<Widget> value)
    {
        layout_.setChildProperty(std::move(value));
    }

    void ToggleButton::InternalOnTouchUp()
    {
    }

    void ToggleButton::InternalOnTouchDown()
    {
        SetIsPressedByUser(!getIsPressedProperty());
    }

    void ToggleButton::OnKeyDown(const Keys key)
    {
        ButtonBase::OnKeyDown(key);
        if (!getEnabledProperty())
        {
            return;
        }
        if (key == Keys::Space)
        {
            SetIsPressedByUser(!getIsPressedProperty());
        }
    }

    std::shared_ptr<Widget> ToggleButton::CreateCloneInstance() const
    {
        return std::make_shared<ToggleButton>();
    }
}
