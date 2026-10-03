// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/ScrollViewer.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    namespace
    {
        [[nodiscard]] int CheckedScrollInteger(const std::int64_t value, const char *const message)
        {
            if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(value);
        }

        [[nodiscard]] int CheckedScrollAdd(const int left, const int right, const char *const message)
        {
            return CheckedScrollInteger(static_cast<std::int64_t>(left) + right, message);
        }

        [[nodiscard]] int CheckedScrollSubtract(const int left, const int right, const char *const message)
        {
            return CheckedScrollInteger(static_cast<std::int64_t>(left) - right, message);
        }

        [[nodiscard]] int CheckedScrollNegate(const int value, const char *const message)
        {
            return CheckedScrollInteger(-static_cast<std::int64_t>(value), message);
        }

        [[nodiscard]] int CheckedScrollScale(const std::int64_t value, const int scale, const int divisor,
                                             const char *const message)
        {
            if (divisor == 0)
            {
                throw std::logic_error("A ScrollViewer thumb travel divisor cannot be zero.");
            }
            return CheckedScrollInteger(value * scale / divisor, message);
        }

        [[nodiscard]] int CheckedNonNegativeScrollExtent(const std::int64_t value, const char *const message)
        {
            return value <= 0 ? 0 : CheckedScrollInteger(value, message);
        }

        [[nodiscard]] bool ContainsScrollPoint(const Rectangle &rectangle, const Point point) noexcept
        {
            const std::int64_t right = static_cast<std::int64_t>(rectangle.X) + rectangle.Width;
            const std::int64_t bottom = static_cast<std::int64_t>(rectangle.Y) + rectangle.Height;
            return rectangle.X <= point.X && point.X < right && rectangle.Y <= point.Y && point.Y < bottom;
        }
    } // namespace

    ScrollViewer::ScrollViewer() : layout_(*this)
    {
        setChildrenLayoutProperty(&layout_);
        setClipToBoundsProperty(true);
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    ScrollViewer::~ScrollViewer()
    {
        UnsubscribeDesktopInput();
    }

    std::shared_ptr<Widget> ScrollViewer::getContentProperty() const
    {
        return layout_.getChildProperty();
    }

    void ScrollViewer::setContentProperty(std::shared_ptr<Widget> value)
    {
        layout_.setChildProperty(std::move(value));
        ResetScroll();
    }

    Point ScrollViewer::getScrollMaximumProperty() const
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return Point(0, 0);
        }

        const Rectangle contentBounds = content->getBoundsProperty();
        const Rectangle bounds = getActualBoundsProperty();
        return Point(CheckedNonNegativeScrollExtent(
                         static_cast<std::int64_t>(contentBounds.Width) - bounds.Width + getVerticalThumbWidth(),
                         "Horizontal ScrollViewer extent exceeds the supported integer range."),
                     CheckedNonNegativeScrollExtent(
                         static_cast<std::int64_t>(contentBounds.Height) - bounds.Height + getHorizontalThumbHeight(),
                         "Vertical ScrollViewer extent exceeds the supported integer range."));
    }

    Point ScrollViewer::getScrollPositionProperty() const
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return Point(0, 0);
        }
        return Point(CheckedScrollNegate(content->getLeftProperty(),
                                         "Horizontal ScrollViewer position exceeds the supported integer range."),
                     CheckedScrollNegate(content->getTopProperty(),
                                         "Vertical ScrollViewer position exceeds the supported integer range."));
    }

    void ScrollViewer::setScrollPositionProperty(const Point value)
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return;
        }
        const int left =
            CheckedScrollNegate(value.X, "Horizontal ScrollViewer offset exceeds the supported integer range.");
        const int top =
            CheckedScrollNegate(value.Y, "Vertical ScrollViewer offset exceeds the supported integer range.");
        content->setLeftProperty(left);
        content->setTopProperty(top);
    }

    void ScrollViewer::ResetScroll()
    {
        setScrollPositionProperty(Point(0, 0));
    }

    std::shared_ptr<Graphics2D::IImage> ScrollViewer::getHorizontalScrollBackgroundProperty() const
    {
        return horizontalScrollBackground_;
    }

    void ScrollViewer::setHorizontalScrollBackgroundProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        horizontalScrollBackground_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IImage> ScrollViewer::getHorizontalScrollKnobProperty() const
    {
        return horizontalScrollKnob_;
    }

    void ScrollViewer::setHorizontalScrollKnobProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        horizontalScrollKnob_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IImage> ScrollViewer::getVerticalScrollBackgroundProperty() const
    {
        return verticalScrollBackground_;
    }

    void ScrollViewer::setVerticalScrollBackgroundProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        verticalScrollBackground_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IImage> ScrollViewer::getVerticalScrollKnobProperty() const
    {
        return verticalScrollKnob_;
    }

    void ScrollViewer::setVerticalScrollKnobProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        verticalScrollKnob_ = std::move(value);
    }

    int ScrollViewer::getScrollMultiplierProperty() const noexcept
    {
        return scrollMultiplier_;
    }

    void ScrollViewer::setScrollMultiplierProperty(const int value) noexcept
    {
        scrollMultiplier_ = value;
    }

    bool ScrollViewer::getShowHorizontalScrollBarProperty() const noexcept
    {
        return showHorizontalScrollBar_;
    }

    void ScrollViewer::setShowHorizontalScrollBarProperty(const bool value)
    {
        if (value == showHorizontalScrollBar_)
        {
            return;
        }
        showHorizontalScrollBar_ = value;
        InvalidateMeasure();
    }

    bool ScrollViewer::getShowVerticalScrollBarProperty() const noexcept
    {
        return showVerticalScrollBar_;
    }

    void ScrollViewer::setShowVerticalScrollBarProperty(const bool value)
    {
        if (value == showVerticalScrollBar_)
        {
            return;
        }
        showVerticalScrollBar_ = value;
        InvalidateMeasure();
    }

    bool ScrollViewer::getHorizontalScrollingOnProperty() const noexcept
    {
        return horizontalScrollingOn_;
    }

    bool ScrollViewer::getVerticalScrollingOnProperty() const noexcept
    {
        return verticalScrollingOn_;
    }

    bool ScrollViewer::getAcceptsMouseWheelProperty() const noexcept
    {
        return verticalScrollingOn_;
    }

    void ScrollViewer::OnMouseWheel(const float delta)
    {
        Widget::OnMouseWheel(delta);
        if (!verticalScrollingOn_)
        {
            return;
        }

        const int step = CheckedScrollScale(scrollMultiplier_, getScrollMaximumProperty().Y, thumbMaximumY_,
                                            "ScrollViewer wheel step exceeds the supported integer range.");
        if (delta < 0.0F)
        {
            scrollbarOrientation_ = Orientation::Vertical;
            MoveThumb(step);
        }
        else if (delta > 0.0F)
        {
            scrollbarOrientation_ = Orientation::Vertical;
            MoveThumb(-step);
        }
    }

    void ScrollViewer::OnTouchDown()
    {
        Widget::OnTouchDown();
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr || !desktop->getTouchPositionProperty())
        {
            return;
        }

        const Point &globalTouchPosition = *desktop->getTouchPositionProperty();
        const Point localTouchPosition = ToLocal(globalTouchPosition);
        const Point thumbPosition = getThumbPosition();

        Rectangle rectangle = verticalScrollbarThumb_;
        rectangle.Y = CheckedScrollAdd(rectangle.Y, thumbPosition.Y,
                                       "Vertical ScrollViewer thumb coordinate exceeds the supported integer range.");
        if (showVerticalScrollBar_ && verticalScrollingOn_ && ContainsScrollPoint(rectangle, localTouchPosition))
        {
            startBoundsPosition_ = globalTouchPosition.Y;
            scrollbarOrientation_ = Orientation::Vertical;
        }

        rectangle = horizontalScrollbarThumb_;
        rectangle.X = CheckedScrollAdd(rectangle.X, thumbPosition.X,
                                       "Horizontal ScrollViewer thumb coordinate exceeds the supported integer range.");
        if (showHorizontalScrollBar_ && horizontalScrollingOn_ && ContainsScrollPoint(rectangle, localTouchPosition))
        {
            startBoundsPosition_ = globalTouchPosition.X;
            scrollbarOrientation_ = Orientation::Horizontal;
        }
    }

    void ScrollViewer::OnTouchUp()
    {
        Widget::OnTouchUp();
        startBoundsPosition_.reset();
    }

    bool ScrollViewer::InputFallsThrough(const Point localPosition)
    {
        if (getBackgroundProperty())
        {
            return false;
        }
        if ((horizontalScrollingOn_ && ContainsScrollPoint(horizontalScrollbarFrame_, localPosition)) ||
            (verticalScrollingOn_ && ContainsScrollPoint(verticalScrollbarFrame_, localPosition)))
        {
            return false;
        }
        return true;
    }

    void ScrollViewer::InternalRender(Graphics2D::RenderContext &context)
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content || !content->getVisibleProperty())
        {
            return;
        }

        Widget::InternalRender(context);
        const Point thumbPosition = getThumbPosition();
        if (horizontalScrollingOn_ && showHorizontalScrollBar_)
        {
            if (horizontalScrollBackground_)
            {
                Graphics2D::IBrushExtensions::Draw(*horizontalScrollBackground_, context, horizontalScrollbarFrame_);
            }
            if (horizontalScrollKnob_)
            {
                Rectangle rectangle = horizontalScrollbarThumb_;
                rectangle.X =
                    CheckedScrollAdd(rectangle.X, thumbPosition.X,
                                     "Horizontal ScrollViewer render coordinate exceeds the supported integer range.");
                Graphics2D::IBrushExtensions::Draw(*horizontalScrollKnob_, context, rectangle);
            }
        }
        if (verticalScrollingOn_ && showVerticalScrollBar_)
        {
            if (verticalScrollBackground_)
            {
                Graphics2D::IBrushExtensions::Draw(*verticalScrollBackground_, context, verticalScrollbarFrame_);
            }
            if (verticalScrollKnob_)
            {
                Rectangle rectangle = verticalScrollbarThumb_;
                rectangle.Y =
                    CheckedScrollAdd(rectangle.Y, thumbPosition.Y,
                                     "Vertical ScrollViewer render coordinate exceeds the supported integer range.");
                Graphics2D::IBrushExtensions::Draw(*verticalScrollKnob_, context, rectangle);
            }
        }
    }

    Point ScrollViewer::InternalMeasure(const Point availableSize)
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return Point(0, 0);
        }

        Point measureSize = content->Measure(availableSize);
        const bool horizontalScrollbarVisible = showHorizontalScrollBar_ && measureSize.X > availableSize.X;
        const bool verticalScrollbarVisible = showVerticalScrollBar_ && measureSize.Y > availableSize.Y;
        if (horizontalScrollbarVisible || verticalScrollbarVisible)
        {
            if (horizontalScrollbarVisible)
            {
                measureSize.Y = CheckedScrollAdd(measureSize.Y, getHorizontalScrollbarHeight(),
                                                 "Measured ScrollViewer height exceeds the supported integer range.");
            }
            if (verticalScrollbarVisible)
            {
                measureSize.X = CheckedScrollAdd(measureSize.X, getVerticalScrollbarWidth(),
                                                 "Measured ScrollViewer width exceeds the supported integer range.");
            }
        }
        return measureSize;
    }

    void ScrollViewer::InternalArrange()
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return;
        }

        Rectangle bounds = getActualBoundsProperty();
        Point availableSize(bounds.Width, bounds.Height);
        const Point oldMeasureSize = content->Measure(availableSize);
        horizontalScrollingOn_ = oldMeasureSize.X > bounds.Width;
        verticalScrollingOn_ = oldMeasureSize.Y > bounds.Height;

        if (horizontalScrollingOn_ || verticalScrollingOn_)
        {
            const int verticalScrollbarWidth = getVerticalScrollbarWidth();
            const int horizontalScrollbarHeight = getHorizontalScrollbarHeight();
            if (horizontalScrollingOn_ && showHorizontalScrollBar_)
            {
                availableSize.Y = std::max(
                    0, CheckedScrollSubtract(availableSize.Y, horizontalScrollbarHeight,
                                             "ScrollViewer available height exceeds the supported integer range."));
            }
            if (verticalScrollingOn_ && showVerticalScrollBar_)
            {
                availableSize.X = std::max(
                    0, CheckedScrollSubtract(availableSize.X, verticalScrollbarWidth,
                                             "ScrollViewer available width exceeds the supported integer range."));
            }

            const Point measureSize = content->Measure(availableSize);
            const int scrollbarWidth = CheckedScrollSubtract(
                bounds.Width, verticalScrollingOn_ && showVerticalScrollBar_ ? verticalScrollbarWidth : 0,
                "Horizontal ScrollViewer frame width exceeds the supported integer range.");
            const int boundsBottom = CheckedScrollAdd(bounds.Y, bounds.Height,
                                                      "ScrollViewer bottom edge exceeds the supported integer range.");
            horizontalScrollbarFrame_ = Rectangle(
                bounds.X,
                CheckedScrollSubtract(boundsBottom, horizontalScrollbarHeight,
                                      "Horizontal ScrollViewer frame coordinate exceeds the supported integer range."),
                scrollbarWidth, horizontalScrollbarHeight);
            const int measuredWidth = std::max(1, measureSize.X);
            const int horizontalKnobWidth = horizontalScrollKnob_ ? horizontalScrollKnob_->getSizeProperty().X : 0;
            const int horizontalKnobHeight = horizontalScrollKnob_ ? horizontalScrollKnob_->getSizeProperty().Y : 0;
            horizontalScrollbarThumb_ = Rectangle(
                bounds.X,
                CheckedScrollSubtract(boundsBottom, horizontalScrollbarHeight,
                                      "Horizontal ScrollViewer thumb coordinate exceeds the supported integer range."),
                std::max(
                    horizontalKnobWidth,
                    CheckedScrollScale(scrollbarWidth, scrollbarWidth, measuredWidth,
                                       "Horizontal ScrollViewer thumb width exceeds the supported integer range.")),
                horizontalKnobHeight);

            const int scrollbarHeight = CheckedScrollSubtract(
                bounds.Height, horizontalScrollingOn_ && showHorizontalScrollBar_ ? horizontalScrollbarHeight : 0,
                "Vertical ScrollViewer frame height exceeds the supported integer range.");
            const int verticalScrollbarX = CheckedScrollSubtract(
                CheckedScrollAdd(bounds.X, bounds.Width,
                                 "ScrollViewer right edge exceeds the supported integer range."),
                verticalScrollbarWidth, "Vertical ScrollViewer frame coordinate exceeds the supported integer range.");
            verticalScrollbarFrame_ = Rectangle(verticalScrollbarX, bounds.Y, verticalScrollbarWidth, scrollbarHeight);
            const int measuredHeight = std::max(1, measureSize.Y);
            const int verticalKnobWidth = verticalScrollKnob_ ? verticalScrollKnob_->getSizeProperty().X : 0;
            const int verticalKnobHeight = verticalScrollKnob_ ? verticalScrollKnob_->getSizeProperty().Y : 0;
            verticalScrollbarThumb_ =
                Rectangle(verticalScrollbarX, bounds.Y, verticalKnobWidth,
                          std::max(verticalKnobHeight,
                                   CheckedScrollScale(
                                       scrollbarHeight, scrollbarHeight, measuredHeight,
                                       "Vertical ScrollViewer thumb height exceeds the supported integer range.")));
            thumbMaximumX_ =
                CheckedScrollSubtract(scrollbarWidth, horizontalScrollbarThumb_.Width,
                                      "Horizontal ScrollViewer thumb travel exceeds the supported integer range.");
            thumbMaximumY_ =
                CheckedScrollSubtract(scrollbarHeight, verticalScrollbarThumb_.Height,
                                      "Vertical ScrollViewer thumb travel exceeds the supported integer range.");
            if (thumbMaximumX_ == 0)
            {
                thumbMaximumX_ = 1;
            }
            if (thumbMaximumY_ == 0)
            {
                thumbMaximumY_ = 1;
            }

            bounds.Width = horizontalScrollingOn_ && showHorizontalScrollBar_ ? measureSize.X : availableSize.X;
            bounds.Height = verticalScrollingOn_ && showVerticalScrollBar_ ? measureSize.Y : availableSize.Y;
        }

        content->Arrange(bounds);
        Point scrollPosition = getScrollPositionProperty();
        const Point maximum = getScrollMaximumProperty();
        if (scrollPosition.X > maximum.X)
        {
            scrollPosition.X = maximum.X;
        }
        if (scrollPosition.Y > maximum.Y)
        {
            scrollPosition.Y = maximum.Y;
        }
        setScrollPositionProperty(scrollPosition);
    }

    void ScrollViewer::OnPlacedChanged()
    {
        UnsubscribeDesktopInput();
        SubscribeDesktopInput();
        ContentControl::OnPlacedChanged();
    }

    std::shared_ptr<Widget> ScrollViewer::CreateCloneInstance() const
    {
        return std::make_shared<ScrollViewer>();
    }

    void ScrollViewer::CopyFrom(const Widget &source)
    {
        ContentControl::CopyFrom(source);
        const auto *const scrollViewer = dynamic_cast<const ScrollViewer *>(&source);
        if (scrollViewer == nullptr)
        {
            throw std::invalid_argument("ScrollViewer copy source must be a ScrollViewer.");
        }
        horizontalScrollBackground_ = scrollViewer->horizontalScrollBackground_;
        horizontalScrollKnob_ = scrollViewer->horizontalScrollKnob_;
        verticalScrollBackground_ = scrollViewer->verticalScrollBackground_;
        verticalScrollKnob_ = scrollViewer->verticalScrollKnob_;
        setShowHorizontalScrollBarProperty(scrollViewer->showHorizontalScrollBar_);
        setShowVerticalScrollBarProperty(scrollViewer->showVerticalScrollBar_);
        setScrollMultiplierProperty(scrollViewer->scrollMultiplier_);
    }

    int ScrollViewer::getHorizontalScrollbarHeight() const
    {
        int result = 0;
        if (horizontalScrollBackground_)
        {
            result = horizontalScrollBackground_->getSizeProperty().Y;
        }
        if (horizontalScrollKnob_)
        {
            result = std::max(result, horizontalScrollKnob_->getSizeProperty().Y);
        }
        return result;
    }

    int ScrollViewer::getVerticalScrollbarWidth() const
    {
        int result = 0;
        if (verticalScrollBackground_)
        {
            result = verticalScrollBackground_->getSizeProperty().X;
        }
        if (verticalScrollKnob_)
        {
            result = std::max(result, verticalScrollKnob_->getSizeProperty().X);
        }
        return result;
    }

    int ScrollViewer::getVerticalThumbWidth() const noexcept
    {
        return verticalScrollingOn_ && showVerticalScrollBar_ ? verticalScrollbarThumb_.Width : 0;
    }

    int ScrollViewer::getHorizontalThumbHeight() const noexcept
    {
        return horizontalScrollingOn_ && showHorizontalScrollBar_ ? horizontalScrollbarThumb_.Height : 0;
    }

    Point ScrollViewer::getThumbPosition() const
    {
        const Point scrollPosition = getScrollPositionProperty();
        const Point maximum = getScrollMaximumProperty();
        Point result(0, 0);
        if (maximum.X > 0)
        {
            result.X =
                CheckedScrollScale(scrollPosition.X, thumbMaximumX_, maximum.X,
                                   "Horizontal ScrollViewer thumb position exceeds the supported integer range.");
        }
        if (maximum.Y > 0)
        {
            result.Y = CheckedScrollScale(scrollPosition.Y, thumbMaximumY_, maximum.Y,
                                          "Vertical ScrollViewer thumb position exceeds the supported integer range.");
        }
        return result;
    }

    void ScrollViewer::MoveThumb(const int delta)
    {
        Point scrollPosition = getScrollPositionProperty();
        const Point maximum = getScrollMaximumProperty();
        int &position = scrollbarOrientation_ == Orientation::Horizontal ? scrollPosition.X : scrollPosition.Y;
        const int limit = scrollbarOrientation_ == Orientation::Horizontal ? maximum.X : maximum.Y;
        const std::int64_t candidate = static_cast<std::int64_t>(position) + delta;
        if (candidate < 0)
        {
            position = 0;
        }
        else if (candidate > limit)
        {
            position = limit;
        }
        else
        {
            position = static_cast<int>(candidate);
        }
        setScrollPositionProperty(scrollPosition);
    }

    void ScrollViewer::SubscribeDesktopInput()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr)
        {
            return;
        }

        const std::shared_ptr<ScrollViewer> retainedTarget = std::static_pointer_cast<ScrollViewer>(RetainSelf());
        if (!retainedTarget)
        {
            throw std::logic_error("A placed ScrollViewer is missing from its owning collection.");
        }

        inputSubscriptionDesktop_ = desktop;
        try
        {
            touchMovedToken_ = desktop->TouchMoved.Add([retainedTarget](void *, Events::MyraEventArgs &)
                                                       { retainedTarget->DesktopTouchMoved(); });
            touchUpToken_ = desktop->TouchUp.Add([retainedTarget](void *, Events::MyraEventArgs &)
                                                 { retainedTarget->DesktopTouchUp(); });
        }
        catch (...)
        {
            UnsubscribeDesktopInput();
            throw;
        }
    }

    void ScrollViewer::UnsubscribeDesktopInput() noexcept
    {
        try
        {
            if (inputSubscriptionDesktop_ != nullptr)
            {
                static_cast<void>(inputSubscriptionDesktop_->TouchMoved.Remove(touchMovedToken_));
                static_cast<void>(inputSubscriptionDesktop_->TouchUp.Remove(touchUpToken_));
            }
        }
        catch (...)
        {
        }
        inputSubscriptionDesktop_ = nullptr;
        touchMovedToken_ = Events::MyraEventHandler::InvalidToken;
        touchUpToken_ = Events::MyraEventHandler::InvalidToken;
        startBoundsPosition_.reset();
    }

    void ScrollViewer::DesktopTouchMoved()
    {
        if (!startBoundsPosition_ || inputSubscriptionDesktop_ == nullptr ||
            getDesktopProperty() != inputSubscriptionDesktop_ || !inputSubscriptionDesktop_->getTouchPositionProperty())
        {
            return;
        }

        const Point &touchPosition = *inputSubscriptionDesktop_->getTouchPositionProperty();
        int delta;
        if (scrollbarOrientation_ == Orientation::Horizontal)
        {
            delta = CheckedScrollScale(static_cast<std::int64_t>(touchPosition.X) - *startBoundsPosition_,
                                       getScrollMaximumProperty().X, thumbMaximumX_,
                                       "Horizontal ScrollViewer drag delta exceeds the supported integer range.");
            startBoundsPosition_ = touchPosition.X;
        }
        else
        {
            delta = CheckedScrollScale(static_cast<std::int64_t>(touchPosition.Y) - *startBoundsPosition_,
                                       getScrollMaximumProperty().Y, thumbMaximumY_,
                                       "Vertical ScrollViewer drag delta exceeds the supported integer range.");
            startBoundsPosition_ = touchPosition.Y;
        }
        MoveThumb(delta);
    }

    void ScrollViewer::DesktopTouchUp() noexcept
    {
        if (inputSubscriptionDesktop_ != nullptr && getDesktopProperty() == inputSubscriptionDesktop_)
        {
            startBoundsPosition_.reset();
        }
    }
} // namespace Myra::Graphics2D::UI
