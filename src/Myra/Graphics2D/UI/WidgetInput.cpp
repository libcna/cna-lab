// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Input.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1. See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#ifdef MYRA_CNA_HAS_CNA_TARGET
#include "CNA/Platform/PlatformException.hpp"
#endif
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputContext.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] int CheckedDragAdd(const int left, const int right)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) + right;
            if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("Widget drag position is outside the integer range.");
            }
            return static_cast<int>(result);
        }

        [[nodiscard]] int CheckedDragSubtract(const int left, const int right)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) - right;
            if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("Widget drag bounds are outside the integer range.");
            }
            return static_cast<int>(result);
        }

        [[nodiscard]] bool HasDragDirection(const DragDirection value, const DragDirection direction) noexcept
        {
            return (static_cast<int>(value) & static_cast<int>(direction)) == static_cast<int>(direction);
        }

        void ApplyWidgetMouseCursor(const MouseCursorType value)
        {
#ifdef MYRA_CNA_HAS_CNA_TARGET
            try
            {
                MyraEnvironment::setMouseCursorTypeProperty(value);
            }
            catch (const CNA::Platform::PlatformException &)
            {
                // The backing state is assigned before CNA reports an unavailable
                // native cursor capability (for example in a windowless backend).
            }
#else
            MyraEnvironment::setMouseCursorTypeProperty(value);
#endif
        }
    } // namespace

    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
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
            if (desktop_->getPreviousTouchPositionProperty())
            {
                QueueInputEvent(InputEventType::TouchEntered);
            }
            else
            {
                QueueInputEvent(InputEventType::TouchDown);
                ProcessDoubleClick(*localTouchPosition_);
            }
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

    void Widget::ProcessDoubleClick(const Point touchPosition)
    {
        const auto now = std::chrono::steady_clock::now();
        if (lastTouchDown_)
        {
            const std::int64_t deltaX =
                static_cast<std::int64_t>(touchPosition.X) - static_cast<std::int64_t>(lastLocalTouchPosition_.X);
            const std::int64_t deltaY =
                static_cast<std::int64_t>(touchPosition.Y) - static_cast<std::int64_t>(lastLocalTouchPosition_.Y);
            const std::int64_t distanceX = deltaX < 0 ? -deltaX : deltaX;
            const std::int64_t distanceY = deltaY < 0 ? -deltaY : deltaY;
            const int radius = MyraEnvironment::getDoubleClickRadiusProperty();

            if (now - *lastTouchDown_ <
                    std::chrono::milliseconds(MyraEnvironment::getDoubleClickIntervalInMsProperty()) &&
                radius >= 0 && distanceX <= radius && distanceY <= radius)
            {
                lastTouchDown_.reset();
                QueueInputEvent(InputEventType::TouchDoubleClick);
                return;
            }
        }

        lastTouchDown_ = now;
        lastLocalTouchPosition_ = touchPosition;
    }

    void Widget::SubscribeDragEvents()
    {
        UnsubscribeDragEvents();
        if (desktop_ == nullptr || !getIsDraggableProperty())
        {
            return;
        }

        const std::shared_ptr<Widget> retainedTarget = RetainSelf();
        if (!retainedTarget)
        {
            throw std::logic_error("A placed draggable widget is missing from its owning collection.");
        }

        const auto moved = [retainedTarget](void *, Events::MyraEventArgs &) { retainedTarget->ProcessDragMoved(); };
        const auto released = [retainedTarget](void *, Events::MyraEventArgs &) { retainedTarget->EndDrag(); };

        try
        {
            if (parent_ != nullptr)
            {
                dragSubscriptionParent_ = parent_;
                dragMovedToken_ = parent_->TouchMoved.Add(moved);
                dragUpToken_ = parent_->TouchUp.Add(released);
            }
            else
            {
                dragSubscriptionDesktop_ = desktop_;
                dragMovedToken_ = desktop_->TouchMoved.Add(moved);
                dragUpToken_ = desktop_->TouchUp.Add(released);
            }
        }
        catch (...)
        {
            UnsubscribeDragEvents();
            throw;
        }
    }

    void Widget::UnsubscribeDragEvents() noexcept
    {
        try
        {
            if (dragSubscriptionParent_ != nullptr)
            {
                static_cast<void>(dragSubscriptionParent_->TouchMoved.Remove(dragMovedToken_));
                static_cast<void>(dragSubscriptionParent_->TouchUp.Remove(dragUpToken_));
            }
            else if (dragSubscriptionDesktop_ != nullptr)
            {
                static_cast<void>(dragSubscriptionDesktop_->TouchMoved.Remove(dragMovedToken_));
                static_cast<void>(dragSubscriptionDesktop_->TouchUp.Remove(dragUpToken_));
            }
        }
        catch (...)
        {
        }

        dragSubscriptionParent_ = nullptr;
        dragSubscriptionDesktop_ = nullptr;
        dragMovedToken_ = Events::MyraEventHandler::InvalidToken;
        dragUpToken_ = Events::MyraEventHandler::InvalidToken;
        EndDrag();
    }

    void Widget::BeginDrag()
    {
        EndDrag();
        if (desktop_ == nullptr || dragHandle_ == nullptr || !desktop_->getTouchPositionProperty())
        {
            return;
        }

        const std::shared_ptr<Widget> retainedHandle = desktop_->RetainWidget(dragHandle_);
        if (!retainedHandle || !retainedHandle->getIsTouchInsideProperty())
        {
            return;
        }

        ITransformable *const parentTransform =
            parent_ != nullptr ? static_cast<ITransformable *>(parent_) : static_cast<ITransformable *>(desktop_);
        const Point &touchPosition = *desktop_->getTouchPositionProperty();
        dragStartPosition_ =
            parentTransform->ToLocal(Vector2(static_cast<float>(touchPosition.X), static_cast<float>(touchPosition.Y)));
        dragStartLeftTop_ = Point(left_, top_);
    }

    void Widget::ProcessDragMoved()
    {
        if (!dragStartPosition_ || !getIsDraggableProperty() || desktop_ == nullptr ||
            !desktop_->getTouchPositionProperty())
        {
            return;
        }

        ITransformable *const parentTransform =
            parent_ != nullptr ? static_cast<ITransformable *>(parent_) : static_cast<ITransformable *>(desktop_);
        const Point &touchPosition = *desktop_->getTouchPositionProperty();
        const Vector2 newPosition =
            parentTransform->ToLocal(Vector2(static_cast<float>(touchPosition.X), static_cast<float>(touchPosition.Y)));
        const Vector2 delta = newPosition - *dragStartPosition_;

        int newLeft = left_;
        int newTop = top_;
        if (HasDragDirection(dragDirection_, DragDirection::Horizontal))
        {
            newLeft = CheckedDragAdd(dragStartLeftTop_.X, Utility::Mathematics::TruncateToInt(delta.X));
        }
        if (HasDragDirection(dragDirection_, DragDirection::Vertical))
        {
            newTop = CheckedDragAdd(dragStartLeftTop_.Y, Utility::Mathematics::TruncateToInt(delta.Y));
        }

        const Rectangle parentBounds =
            parent_ != nullptr ? parent_->getActualBoundsProperty() : desktop_->getInternalBoundsProperty();
        const Rectangle bounds = getBoundsProperty();
        if (newLeft < 0)
        {
            newLeft = 0;
        }
        if (static_cast<std::int64_t>(newLeft) + bounds.Width > parentBounds.Width)
        {
            newLeft = CheckedDragSubtract(parentBounds.Width, bounds.Width);
        }
        if (newTop < 0)
        {
            newTop = 0;
        }
        if (static_cast<std::int64_t>(newTop) + bounds.Height > parentBounds.Height)
        {
            newTop = CheckedDragSubtract(parentBounds.Height, bounds.Height);
        }

        setLeftProperty(newLeft);
        setTopProperty(newTop);
    }

    void Widget::EndDrag() noexcept
    {
        dragStartPosition_.reset();
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
            if (desktop_ != nullptr && desktop_->tooltipOwner_.lock().get() == this)
            {
                desktop_->HideTooltip();
            }
            lastMouseMovement_.reset();
            if (MyraEnvironment::getSetMouseCursorFromWidgetProperty() && mouseCursor_)
            {
                Widget *ancestor = parent_;
                while (ancestor != nullptr && !ancestor->getIsMouseInsideProperty())
                {
                    ancestor = ancestor->parent_;
                }

                const MouseCursorType cursor = ancestor != nullptr && ancestor->mouseCursor_
                                                   ? *ancestor->mouseCursor_
                                                   : MyraEnvironment::getDefaultMouseCursorTypeProperty();
                ApplyWidgetMouseCursor(cursor);
            }
            OnMouseLeft();
            Utility::EventsExtensions::Invoke(MouseLeft, this, eventType);
            break;
        case InputEventType::MouseEntered:
            lastMouseMovement_ = std::chrono::steady_clock::now();
            if (MyraEnvironment::getSetMouseCursorFromWidgetProperty() && mouseCursor_)
            {
                ApplyWidgetMouseCursor(*mouseCursor_);
            }
            OnMouseEntered();
            Utility::EventsExtensions::Invoke(MouseEntered, this, eventType);
            break;
        case InputEventType::MouseMoved:
            lastMouseMovement_ = std::chrono::steady_clock::now();
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
            BeginDrag();
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
