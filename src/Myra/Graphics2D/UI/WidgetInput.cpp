// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Input.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    void Widget::ProcessEvent(const InputEventType eventType)
    {
        switch (eventType)
        {
        case InputEventType::MouseLeft:
            OnMouseLeft();
            Utility::EventsExtensions::Invoke(MouseLeft, this, eventType);
            break;
        case InputEventType::MouseEntered:
            OnMouseEntered();
            Utility::EventsExtensions::Invoke(MouseEntered, this, eventType);
            break;
        case InputEventType::MouseMoved:
            OnMouseMoved();
            Utility::EventsExtensions::Invoke(MouseMoved, this, eventType);
            break;
        case InputEventType::TouchLeft:
            OnTouchLeft();
            Utility::EventsExtensions::Invoke(TouchLeft, this, eventType);
            break;
        case InputEventType::TouchEntered:
            OnTouchEntered();
            Utility::EventsExtensions::Invoke(TouchEntered, this, eventType);
            break;
        case InputEventType::TouchMoved:
            OnTouchMoved();
            Utility::EventsExtensions::Invoke(TouchMoved, this, eventType);
            break;
        case InputEventType::TouchDown:
            OnTouchDown();
            Utility::EventsExtensions::Invoke(TouchDown, this, eventType);
            break;
        case InputEventType::TouchUp:
            OnTouchUp();
            Utility::EventsExtensions::Invoke(TouchUp, this, eventType);
            break;
        case InputEventType::TouchDoubleClick:
            OnTouchDoubleClick();
            Utility::EventsExtensions::Invoke(TouchDoubleClick, this, eventType);
            break;
        default:
            break;
        }
    }

    void Widget::OnMouseLeft() {}

    void Widget::OnMouseEntered() {}

    void Widget::OnMouseMoved() {}

    void Widget::OnTouchLeft() {}

    void Widget::OnTouchEntered() {}

    void Widget::OnTouchMoved() {}

    void Widget::OnTouchDown() {}

    void Widget::OnTouchUp() {}

    void Widget::OnTouchDoubleClick() {}

    void Widget::FireKeyDown(const Keys key)
    {
        Utility::EventsExtensions::Invoke(KeyDown, this, key, InputEventType::KeyDown);
    }

    void Widget::OnKeyDown(const Keys key)
    {
        FireKeyDown(key);
    }

    void Widget::OnKeyUp(const Keys key)
    {
        Utility::EventsExtensions::Invoke(KeyUp, this, key, InputEventType::KeyUp);
    }
}
