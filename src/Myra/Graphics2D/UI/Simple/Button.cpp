// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/Button.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/Button.hpp"

#include <utility>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    Button::Button() : layout_(*this)
    {
        setChildrenLayoutProperty(&layout_);
    }

    std::shared_ptr<Widget> Button::getContentProperty() const
    {
        return layout_.getChildProperty();
    }

    void Button::setContentProperty(std::shared_ptr<Widget> value)
    {
        layout_.setChildProperty(std::move(value));
    }

    void Button::OnTouchLeft()
    {
        ButtonBase::OnTouchLeft();
        if (releaseOnTouchLeft_)
        {
            SetIsPressedByUser(false);
        }
    }

    void Button::InternalOnTouchUp()
    {
        SetIsPressedByUser(false);
    }

    void Button::InternalOnTouchDown()
    {
        SetIsPressedByUser(true);
    }

    void Button::OnKeyDown(const Keys key)
    {
        ButtonBase::OnKeyDown(key);
        if (!getEnabledProperty())
        {
            return;
        }
        if (key == Keys::Space)
        {
            DoClick();
        }
    }

    std::shared_ptr<Widget> Button::CreateCloneInstance() const
    {
        return std::make_shared<Button>();
    }
}
