// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/Button.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/Button.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Desktop.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    Button::Button() : layout_(*this)
    {
        setChildrenLayoutProperty(&layout_);
    }

    Button::~Button()
    {
        UnsubscribeDesktopTouchUp();
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

    void Button::OnPlacedChanged()
    {
        UnsubscribeDesktopTouchUp();
        SubscribeDesktopTouchUp();
        ButtonBase::OnPlacedChanged();
    }

    void Button::SubscribeDesktopTouchUp()
    {
        Desktop *const desktop = getDesktopProperty();
        if (releaseOnTouchLeft_ || desktop == nullptr)
        {
            return;
        }

        const std::shared_ptr<Button> retainedTarget = std::static_pointer_cast<Button>(RetainSelf());
        if (!retainedTarget)
        {
            throw std::logic_error("A placed non-releasing button is missing from its owning collection.");
        }

        touchUpSubscriptionDesktop_ = desktop;
        try
        {
            touchUpToken_ = desktop->TouchUp.Add([retainedTarget](void *, Events::MyraEventArgs &)
                                                 { retainedTarget->DesktopTouchUp(); });
        }
        catch (...)
        {
            UnsubscribeDesktopTouchUp();
            throw;
        }
    }

    void Button::UnsubscribeDesktopTouchUp() noexcept
    {
        try
        {
            if (touchUpSubscriptionDesktop_ != nullptr)
            {
                static_cast<void>(touchUpSubscriptionDesktop_->TouchUp.Remove(touchUpToken_));
            }
        }
        catch (...)
        {
        }
        touchUpSubscriptionDesktop_ = nullptr;
        touchUpToken_ = Events::MyraEventHandler::InvalidToken;
    }

    void Button::DesktopTouchUp()
    {
        if (touchUpSubscriptionDesktop_ == nullptr || getDesktopProperty() != touchUpSubscriptionDesktop_)
        {
            return;
        }
        setIsPressedProperty(false);
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
