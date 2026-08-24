// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/ScrollViewer.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/IImage.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    ScrollViewer::ScrollViewer() : layout_(*this)
    {
        setChildrenLayoutProperty(&layout_);
        setClipToBoundsProperty(true);
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
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
        return Point(std::max(0, contentBounds.Width - bounds.Width + getVerticalThumbWidth()),
                     std::max(0, contentBounds.Height - bounds.Height + getHorizontalThumbHeight()));
    }

    Point ScrollViewer::getScrollPositionProperty() const
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return Point(0, 0);
        }
        return Point(-content->getLeftProperty(), -content->getTopProperty());
    }

    void ScrollViewer::setScrollPositionProperty(const Point value)
    {
        const std::shared_ptr<Widget> content = getContentProperty();
        if (!content)
        {
            return;
        }
        content->setLeftProperty(-value.X);
        content->setTopProperty(-value.Y);
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

        const int step = scrollMultiplier_ * getScrollMaximumProperty().Y / thumbMaximumY_;
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
                rectangle.X += thumbPosition.X;
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
                rectangle.Y += thumbPosition.Y;
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
                measureSize.Y += getHorizontalScrollbarHeight();
            }
            if (verticalScrollbarVisible)
            {
                measureSize.X += getVerticalScrollbarWidth();
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
                availableSize.Y = std::max(0, availableSize.Y - horizontalScrollbarHeight);
            }
            if (verticalScrollingOn_ && showVerticalScrollBar_)
            {
                availableSize.X = std::max(0, availableSize.X - verticalScrollbarWidth);
            }

            const Point measureSize = content->Measure(availableSize);
            const int scrollbarWidth =
                bounds.Width - (verticalScrollingOn_ && showVerticalScrollBar_ ? verticalScrollbarWidth : 0);
            const int boundsBottom = bounds.Y + bounds.Height;
            horizontalScrollbarFrame_ = Rectangle(bounds.X, boundsBottom - horizontalScrollbarHeight, scrollbarWidth,
                                                  horizontalScrollbarHeight);
            const int measuredWidth = std::max(1, measureSize.X);
            const int horizontalKnobWidth = horizontalScrollKnob_ ? horizontalScrollKnob_->getSizeProperty().X : 0;
            const int horizontalKnobHeight = horizontalScrollKnob_ ? horizontalScrollKnob_->getSizeProperty().Y : 0;
            horizontalScrollbarThumb_ = Rectangle(
                bounds.X, boundsBottom - horizontalScrollbarHeight,
                std::max(horizontalKnobWidth, scrollbarWidth * scrollbarWidth / measuredWidth), horizontalKnobHeight);

            const int scrollbarHeight =
                bounds.Height - (horizontalScrollingOn_ && showHorizontalScrollBar_ ? horizontalScrollbarHeight : 0);
            verticalScrollbarFrame_ = Rectangle(bounds.X + bounds.Width - verticalScrollbarWidth, bounds.Y,
                                                verticalScrollbarWidth, scrollbarHeight);
            const int measuredHeight = std::max(1, measureSize.Y);
            const int verticalKnobWidth = verticalScrollKnob_ ? verticalScrollKnob_->getSizeProperty().X : 0;
            const int verticalKnobHeight = verticalScrollKnob_ ? verticalScrollKnob_->getSizeProperty().Y : 0;
            verticalScrollbarThumb_ =
                Rectangle(bounds.X + bounds.Width - verticalScrollbarWidth, bounds.Y, verticalKnobWidth,
                          std::max(verticalKnobHeight, scrollbarHeight * scrollbarHeight / measuredHeight));
            thumbMaximumX_ = std::max(1, scrollbarWidth - horizontalScrollbarThumb_.Width);
            thumbMaximumY_ = std::max(1, scrollbarHeight - verticalScrollbarThumb_.Height);

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
            result.X = scrollPosition.X * thumbMaximumX_ / maximum.X;
        }
        if (maximum.Y > 0)
        {
            result.Y = scrollPosition.Y * thumbMaximumY_ / maximum.Y;
        }
        return result;
    }

    void ScrollViewer::MoveThumb(const int delta)
    {
        Point scrollPosition = getScrollPositionProperty();
        const Point maximum = getScrollMaximumProperty();
        int &position = scrollbarOrientation_ == Orientation::Horizontal ? scrollPosition.X : scrollPosition.Y;
        const int limit = scrollbarOrientation_ == Orientation::Horizontal ? maximum.X : maximum.Y;
        position = std::clamp(position + delta, 0, limit);
        setScrollPositionProperty(scrollPosition);
    }
} // namespace Myra::Graphics2D::UI
