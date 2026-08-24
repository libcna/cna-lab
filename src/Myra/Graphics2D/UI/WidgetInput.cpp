// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Input.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1. See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <utility>

#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputContext.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::Keys;

    bool Widget::getIsMouseInsideProperty() const noexcept
    {
        return localMousePosition_.has_value();
    }

    const std::optional<Point> &Widget::getLocalMousePositionProperty() const noexcept
    {
        return localMousePosition_;
    }

    bool Widget::getIsTouchInsideProperty() const noexcept
    {
        return localTouchPosition_.has_value();
    }

    const std::optional<Point> &Widget::getLocalTouchPositionProperty() const noexcept
    {
        return localTouchPosition_;
    }

    void Widget::QueueInputEvent(const InputEventType eventType)
    {
        if (desktop_ == nullptr)
        {
            return;
        }
        std::shared_ptr<Widget> retainedTarget = desktop_->RetainWidget(this);
        if (retainedTarget)
        {
            InputEventsManager::Queue(std::move(retainedTarget), eventType);
        }
    }

    void Widget::setLocalMousePositionProperty(std::optional<Point> value)
    {
        if (value == localMousePosition_)
        {
            return;
        }

        const std::optional<Point> oldValue = localMousePosition_;
        localMousePosition_ = std::move(value);
        if (desktop_ == nullptr)
        {
            return;
        }

        if (localMousePosition_ && !oldValue)
        {
            QueueInputEvent(InputEventType::MouseEntered);
        }
        else if (!localMousePosition_ && oldValue)
        {
            QueueInputEvent(InputEventType::MouseLeft);
        }
        else if (localMousePosition_ != oldValue)
        {
            QueueInputEvent(InputEventType::MouseMoved);
        }
    }

    void Widget::setLocalTouchPositionProperty(std::optional<Point> value)
    {
        if (value == localTouchPosition_)
        {
            return;
        }

        const std::optional<Point> oldValue = localTouchPosition_;
        localTouchPosition_ = std::move(value);
        if (desktop_ == nullptr)
        {
            return;
        }

        if (localTouchPosition_ && !oldValue)
        {
            QueueInputEvent(desktop_->getPreviousTouchPositionProperty() ? InputEventType::TouchEntered
                                                                         : InputEventType::TouchDown);
        }
        else if (!localTouchPosition_ && oldValue)
        {
            QueueInputEvent(desktop_->getTouchPositionProperty() ? InputEventType::TouchLeft : InputEventType::TouchUp);
        }
        else if (localTouchPosition_ != oldValue)
        {
            QueueInputEvent(InputEventType::TouchMoved);
        }
    }

    bool Widget::getAcceptsMouseWheelProperty() const noexcept
    {
        return false;
    }

    void Widget::ProcessInput(InputContext &inputContext)
    {
        Desktop *const desktop = desktop_;
        if (!visible_ || desktop == nullptr)
        {
            return;
        }

        const std::vector<std::shared_ptr<Widget>> children = getChildrenCopyProperty();
        if (!inputContext.MouseOrTouchHandled)
        {
            const bool oldContainsMouse = inputContext.ParentContainsMouse;
            const bool oldContainsTouch = inputContext.ParentContainsTouch;

            if (!Desktop::getIsMobileProperty())
            {
                if (inputContext.ParentContainsMouse)
                {
                    const Point globalMousePosition = desktop->getMousePositionProperty();
                    if (ContainsGlobalPoint(globalMousePosition))
                    {
                        setLocalMousePositionProperty(ToLocal(globalMousePosition));
                    }
                    else
                    {
                        setLocalMousePositionProperty(std::nullopt);
                        inputContext.ParentContainsMouse = false;
                    }
                }
                else
                {
                    setLocalMousePositionProperty(std::nullopt);
                }
            }

            const std::optional<Point> &globalTouchPosition = desktop->getTouchPositionProperty();
            if (globalTouchPosition && inputContext.ParentContainsTouch)
            {
                if (ContainsGlobalPoint(*globalTouchPosition))
                {
                    setLocalTouchPositionProperty(ToLocal(*globalTouchPosition));
                }
                else
                {
                    setLocalTouchPositionProperty(std::nullopt);
                    inputContext.ParentContainsTouch = false;
                }
            }
            else
            {
                setLocalTouchPositionProperty(std::nullopt);
            }

            if (getIsMouseInsideProperty() && !Utility::Mathematics::IsZero(desktop->getMouseWheelDeltaProperty()) &&
                getAcceptsMouseWheelProperty())
            {
                inputContext.MouseWheelWidget = this;
            }

            for (auto iterator = children.rbegin(); iterator != children.rend(); ++iterator)
            {
                (*iterator)->ProcessInput(inputContext);
            }

            if (isModal_)
            {
                inputContext.MouseOrTouchHandled = true;
            }
            else if (!Desktop::getIsMobileProperty())
            {
                if (getIsMouseInsideProperty() && InputFallsThrough(*localMousePosition_) == false)
                {
                    inputContext.MouseOrTouchHandled = true;
                }
            }
            else if (getIsTouchInsideProperty() && InputFallsThrough(*localTouchPosition_) == false)
            {
                inputContext.MouseOrTouchHandled = true;
            }

            inputContext.ParentContainsMouse = oldContainsMouse;
            inputContext.ParentContainsTouch = oldContainsTouch;
            return;
        }

        if (!Desktop::getIsMobileProperty())
        {
            setLocalMousePositionProperty(std::nullopt);
        }
        setLocalTouchPositionProperty(std::nullopt);
        for (auto iterator = children.rbegin(); iterator != children.rend(); ++iterator)
        {
            (*iterator)->ProcessInput(inputContext);
        }
    }

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
        case InputEventType::MouseWheel:
            if (desktop_ != nullptr)
            {
                OnMouseWheel(desktop_->getMouseWheelDeltaProperty());
                if (desktop_ != nullptr)
                {
                    Utility::EventsExtensions::Invoke(MouseWheelChanged, this, desktop_->getMouseWheelDeltaProperty(),
                                                      eventType);
                }
            }
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
            if (desktop_ != nullptr && enabled_ && acceptsKeyboardFocus_)
            {
                desktop_->setFocusedKeyboardWidgetProperty(this);
            }
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

    void Widget::OnMouseWheel(const float) {}

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
} // namespace Myra::Graphics2D::UI
