// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs and src/Myra/Graphics2D/UI/Desktop.Input.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1. See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <chrono>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::Keys;

    Desktop::InputProcessor::InputProcessor(Desktop &owner) noexcept : owner_(&owner) {}

    void Desktop::InputProcessor::Detach() noexcept
    {
        owner_ = nullptr;
    }

    void Desktop::InputProcessor::ProcessEvent(const InputEventType eventType)
    {
        if (owner_ != nullptr)
        {
            owner_->ProcessInputEvent(eventType);
        }
    }

    const Point &Desktop::getPreviousMousePositionProperty() const noexcept
    {
        return previousMousePosition_;
    }

    const std::optional<Point> &Desktop::getPreviousTouchPositionProperty() const noexcept
    {
        return previousTouchPosition_;
    }

    const Point &Desktop::getMousePositionProperty() const noexcept
    {
        return mousePosition_;
    }

    const std::optional<Point> &Desktop::getTouchPositionProperty() const noexcept
    {
        return touchPosition_;
    }

    bool Desktop::getIsTouchDownProperty() const noexcept
    {
        return touchPosition_.has_value();
    }

    float Desktop::getMouseWheelDeltaProperty() const noexcept
    {
        return mouseWheelDelta_;
    }

    const std::array<bool, 0xff> &Desktop::getDownKeysProperty() const noexcept
    {
        return downKeys_;
    }

    int Desktop::getRepeatKeyDownStartInMsProperty() const noexcept
    {
        return repeatKeyDownStartInMs_;
    }

    void Desktop::setRepeatKeyDownStartInMsProperty(const int value) noexcept
    {
        repeatKeyDownStartInMs_ = value;
    }

    int Desktop::getRepeatKeyDownIntervalInMsProperty() const noexcept
    {
        return repeatKeyDownIntervalInMs_;
    }

    void Desktop::setRepeatKeyDownIntervalInMsProperty(const int value) noexcept
    {
        repeatKeyDownIntervalInMs_ = value;
    }

    bool Desktop::IsKeyDown(const Keys key) const
    {
        const int index = static_cast<int>(key);
        if (index < 0 || index >= static_cast<int>(downKeys_.size()))
        {
            throw std::out_of_range("The key is outside Desktop's fixed upstream key-state domain.");
        }
        return downKeys_[static_cast<std::size_t>(index)];
    }

    bool Desktop::getIsMobileProperty() noexcept
    {
        return false;
    }

    void Desktop::UpdateMouseInput()
    {
        const MyraEnvironment::MouseInfoGetter getter = MyraEnvironment::getMouseInfoGetterProperty();
        if (!getter)
        {
            return;
        }

        const MouseInfo mouseInfo = getter();
        setMousePositionProperty(mouseInfo.Position);

        std::optional<Point> touchPosition;
        if (mouseInfo.IsLeftButtonDown || mouseInfo.IsMiddleButtonDown || mouseInfo.IsRightButtonDown)
        {
            touchPosition = mousePosition_;
        }
        setTouchPositionProperty(touchPosition);

        if (!Utility::Mathematics::EpsilonEquals(mouseInfo.Wheel, lastMouseInfo_.Wheel))
        {
            setMouseWheelDeltaProperty(mouseInfo.Wheel - lastMouseInfo_.Wheel);
        }
        else
        {
            setMouseWheelDeltaProperty(0.0F);
        }
        lastMouseInfo_ = mouseInfo;
    }

    void Desktop::UpdateKeyboardInput()
    {
        const MyraEnvironment::DownKeysGetter getter = MyraEnvironment::getDownKeysGetterProperty();
        if (!getter)
        {
            return;
        }

        getter(downKeys_);
        const auto now = std::chrono::steady_clock::now();
        for (std::size_t index = 0; index < downKeys_.size(); ++index)
        {
            const Keys key = static_cast<Keys>(index);
            if (downKeys_[index] && !lastDownKeys_[index])
            {
                if (key == Keys::Tab)
                {
                    FocusNextWidget();
                }
                if (KeyDownHandler)
                {
                    KeyDownHandler(key);
                }
                lastKeyDown_ = now;
                keyDownCount_ = 0;
            }
            else if (!downKeys_[index] && lastDownKeys_[index])
            {
                Utility::EventsExtensions::Invoke(KeyUp, key, InputEventType::KeyUp);
                Widget *const target = focusedKeyboardWidget_;
                const std::shared_ptr<Widget> retainedTarget = RetainWidget(target);
                if (target != nullptr && target == focusedKeyboardWidget_)
                {
                    target->OnKeyUp(key);
                }
                static_cast<void>(retainedTarget);
                lastKeyDown_.reset();
                keyDownCount_ = 0;
            }
            else if (downKeys_[index] && lastDownKeys_[index] && lastKeyDown_)
            {
                const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - *lastKeyDown_).count();
                const int threshold = keyDownCount_ == 0 ? repeatKeyDownStartInMs_ : repeatKeyDownIntervalInMs_;
                if (elapsed > threshold)
                {
                    if (KeyDownHandler)
                    {
                        KeyDownHandler(key);
                    }
                    lastKeyDown_ = now;
                    ++keyDownCount_;
                }
            }
        }
        lastDownKeys_ = downKeys_;
    }

    void Desktop::UpdateInput()
    {
        UpdateKeyboardInput();
        previousMousePosition_ = mousePosition_;
        previousTouchPosition_ = touchPosition_;
        UpdateMouseInput();
    }

    void Desktop::OnKeyDown(const Keys key)
    {
        Utility::EventsExtensions::Invoke(KeyDown, key, InputEventType::KeyDown);

        if (IsMenuBarActive())
        {
            HorizontalMenu *const menu = menuBar_;
            const std::shared_ptr<Widget> retainedMenu = RetainWidget(menu);
            if (menu != nullptr && menu == menuBar_)
            {
                menu->OnKeyDown(key);
            }
            static_cast<void>(retainedMenu);
            return;
        }

        Widget *const target = focusedKeyboardWidget_;
        const std::shared_ptr<Widget> retainedTarget = RetainWidget(target);
        if (target != nullptr && target == focusedKeyboardWidget_)
        {
            target->OnKeyDown(key);
        }
        static_cast<void>(retainedTarget);
    }

    void Desktop::FocusNextWidget()
    {
        if (widgets_.getCountProperty() == 0)
        {
            return;
        }

        bool searchForFocusable = focusedKeyboardWidget_ == nullptr;
        bool focusChanged = false;
        ProcessWidgets(
            [&](Widget &widget)
            {
                if (searchForFocusable)
                {
                    if (CanFocusWidget(widget))
                    {
                        widget.SetKeyboardFocus();
                        focusChanged = true;
                        return false;
                    }
                }
                else if (&widget == focusedKeyboardWidget_)
                {
                    searchForFocusable = true;
                }
                return true;
            });

        if (focusChanged || focusedKeyboardWidget_ == nullptr)
        {
            return;
        }

        ProcessWidgets(
            [](Widget &widget)
            {
                if (CanFocusWidget(widget))
                {
                    widget.SetKeyboardFocus();
                    return false;
                }
                return true;
            });
    }

    bool Desktop::CanFocusWidget(const Widget &widget) noexcept
    {
        return widget.getVisibleProperty() && widget.getEnabledProperty() && widget.getAcceptsKeyboardFocusProperty();
    }

    bool Desktop::IsMenuBarActive() const noexcept
    {
        return menuBar_ != nullptr && (menuBar_->getOpenMenuItemProperty() != nullptr || IsKeyDown(Keys::LeftAlt) ||
                                       IsKeyDown(Keys::RightAlt));
    }

    void Desktop::setMousePositionProperty(const Point value)
    {
        if (mousePosition_ == value)
        {
            return;
        }
        mousePosition_ = value;
        QueueInputEvent(InputEventType::MouseMoved);
    }

    void Desktop::setTouchPositionProperty(std::optional<Point> value)
    {
        if (touchPosition_ == value)
        {
            return;
        }

        const std::optional<Point> oldValue = touchPosition_;
        touchPosition_ = std::move(value);
        if (touchPosition_ && !oldValue)
        {
            QueueInputEvent(InputEventType::TouchDown);
        }
        else if (!touchPosition_ && oldValue)
        {
            QueueInputEvent(InputEventType::TouchUp);
        }
        else if (touchPosition_ != oldValue)
        {
            QueueInputEvent(InputEventType::TouchMoved);
        }
    }

    void Desktop::setMouseWheelDeltaProperty(const float value)
    {
        mouseWheelDelta_ = value;
        if (!Utility::Mathematics::IsZero(value))
        {
            QueueInputEvent(InputEventType::MouseWheel);
        }
    }

    void Desktop::QueueInputEvent(const InputEventType eventType)
    {
        InputEventsManager::Queue(inputProcessor_, eventType);
    }

    void Desktop::ProcessInputEvent(const InputEventType eventType)
    {
        switch (eventType)
        {
        case InputEventType::MouseMoved:
            Utility::EventsExtensions::Invoke(MouseMoved, this, eventType);
            break;
        case InputEventType::MouseWheel:
            Utility::EventsExtensions::Invoke(MouseWheelChanged, this, mouseWheelDelta_, eventType);
            break;
        case InputEventType::TouchMoved:
            Utility::EventsExtensions::Invoke(TouchMoved, this, eventType);
            break;
        case InputEventType::TouchDown:
            Utility::EventsExtensions::Invoke(TouchDown, this, eventType);
            break;
        case InputEventType::TouchUp:
            Utility::EventsExtensions::Invoke(TouchUp, this, eventType);
            break;
        case InputEventType::TouchDoubleClick:
            Utility::EventsExtensions::Invoke(TouchDoubleClick, this, eventType);
            break;
        default:
            break;
        }
    }
} // namespace Myra::Graphics2D::UI
