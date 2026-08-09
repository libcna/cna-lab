// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/ButtonBase.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"

#include <stdexcept>

#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    bool ButtonBase::getReadOnlyProperty() const noexcept
    {
        return readOnly_;
    }

    void ButtonBase::setReadOnlyProperty(const bool value) noexcept
    {
        readOnly_ = value;
    }

    void ButtonBase::DoClick()
    {
        OnTouchDown();
        OnTouchUp();
    }

    void ButtonBase::OnTouchUp()
    {
        ContentControl::OnTouchUp();
        if (!getEnabledProperty() || readOnly_)
        {
            return;
        }

        InternalOnTouchUp();
        if (isClicked_)
        {
            Utility::EventsExtensions::Invoke(Click, this, InputEventType::TouchUp);
            isClicked_ = false;
        }
    }

    void ButtonBase::OnTouchDown()
    {
        ContentControl::OnTouchDown();
        if (!getEnabledProperty() || readOnly_)
        {
            return;
        }

        InternalOnTouchDown();
        isClicked_ = true;
    }

    void ButtonBase::CopyFrom(const Widget& source)
    {
        ContentControl::CopyFrom(source);
        const auto* const buttonBase = dynamic_cast<const ButtonBase*>(&source);
        if (buttonBase == nullptr)
        {
            throw std::invalid_argument("ButtonBase copy source must be a ButtonBase.");
        }

        setPressedBackgroundProperty(buttonBase->getPressedBackgroundProperty());
        setIsPressedProperty(buttonBase->getIsPressedProperty());
        setReadOnlyProperty(buttonBase->readOnly_);
    }
}
