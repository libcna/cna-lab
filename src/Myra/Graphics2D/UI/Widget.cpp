// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Children.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>

#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"
#include "Myra/Graphics2D/UI/LayoutUtils.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    Widget::Widget() = default;

    Widget::~Widget()
    {
        ClearChildren();
    }

    int Widget::getLeftProperty() const noexcept { return left_; }

    void Widget::setLeftProperty(const int value)
    {
        if (value == left_)
        {
            return;
        }
        left_ = value;
        InvalidateTransform();
        FireLocationChanged();
    }

    int Widget::getTopProperty() const noexcept { return top_; }

    void Widget::setTopProperty(const int value)
    {
        if (value == top_)
        {
            return;
        }
        top_ = value;
        InvalidateTransform();
        FireLocationChanged();
    }

    const std::optional<int>& Widget::getMinWidthProperty() const noexcept { return minWidth_; }

    void Widget::setMinWidthProperty(std::optional<int> value)
    {
        if (value == minWidth_)
        {
            return;
        }
        minWidth_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const std::optional<int>& Widget::getMaxWidthProperty() const noexcept { return maxWidth_; }

    void Widget::setMaxWidthProperty(std::optional<int> value)
    {
        if (value == maxWidth_)
        {
            return;
        }
        maxWidth_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const std::optional<int>& Widget::getWidthProperty() const noexcept { return width_; }

    void Widget::setWidthProperty(std::optional<int> value)
    {
        if (value == width_)
        {
            return;
        }
        width_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const std::optional<int>& Widget::getMinHeightProperty() const noexcept { return minHeight_; }

    void Widget::setMinHeightProperty(std::optional<int> value)
    {
        if (value == minHeight_)
        {
            return;
        }
        minHeight_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const std::optional<int>& Widget::getMaxHeightProperty() const noexcept { return maxHeight_; }

    void Widget::setMaxHeightProperty(std::optional<int> value)
    {
        if (value == maxHeight_)
        {
            return;
        }
        maxHeight_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const std::optional<int>& Widget::getHeightProperty() const noexcept { return height_; }

    void Widget::setHeightProperty(std::optional<int> value)
    {
        if (value == height_)
        {
            return;
        }
        height_ = value;
        InvalidateMeasure();
        FireSizeChanged();
    }

    const Thickness& Widget::getMarginProperty() const noexcept { return margin_; }

    void Widget::setMarginProperty(const Thickness value)
    {
        if (margin_ == value)
        {
            return;
        }
        margin_ = value;
        InvalidateMeasure();
    }

    const Thickness& Widget::getBorderThicknessProperty() const noexcept { return borderThickness_; }

    void Widget::setBorderThicknessProperty(const Thickness value)
    {
        if (borderThickness_ == value)
        {
            return;
        }
        borderThickness_ = value;
        InvalidateMeasure();
    }

    const Thickness& Widget::getPaddingProperty() const noexcept { return padding_; }

    void Widget::setPaddingProperty(const Thickness value)
    {
        if (padding_ == value)
        {
            return;
        }
        padding_ = value;
        InvalidateMeasure();
    }

    HorizontalAlignment Widget::getHorizontalAlignmentProperty() const noexcept { return horizontalAlignment_; }

    void Widget::setHorizontalAlignmentProperty(const HorizontalAlignment value)
    {
        if (value == horizontalAlignment_)
        {
            return;
        }
        horizontalAlignment_ = value;
        InvalidateMeasure();
    }

    VerticalAlignment Widget::getVerticalAlignmentProperty() const noexcept { return verticalAlignment_; }

    void Widget::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        if (value == verticalAlignment_)
        {
            return;
        }
        verticalAlignment_ = value;
        InvalidateMeasure();
    }

    bool Widget::getEnabledProperty() const noexcept { return enabled_; }

    void Widget::setEnabledProperty(const bool value)
    {
        if (value == enabled_)
        {
            return;
        }
        enabled_ = value;
        for (const std::shared_ptr<Widget>& child : getChildrenCopyProperty())
        {
            child->setEnabledProperty(value);
        }
        Utility::EventsExtensions::Invoke(EnabledChanged, this, InputEventType::EnabledChanged);
    }

    bool Widget::getVisibleProperty() const noexcept { return visible_; }

    void Widget::setVisibleProperty(const bool value)
    {
        if (value == visible_)
        {
            return;
        }
        visible_ = value;
        OnVisibleChanged();
    }

    int Widget::getZIndexProperty() const noexcept { return zIndex_; }

    void Widget::setZIndexProperty(const int value)
    {
        if (value == zIndex_)
        {
            return;
        }
        zIndex_ = value;
        InvalidateMeasure();
    }

    float Widget::getOpacityProperty() const noexcept { return opacity_; }

    void Widget::setOpacityProperty(const float value)
    {
        if (value < 0.0F || value > 1.0F)
        {
            throw std::out_of_range("Widget opacity must be in the range [0, 1].");
        }
        opacity_ = value;
    }

    const Vector2& Widget::getScaleProperty() const noexcept { return scale_; }

    void Widget::setScaleProperty(const Vector2 value)
    {
        if (value == scale_)
        {
            return;
        }
        scale_ = value;
        InvalidateTransform();
    }

    const Vector2& Widget::getTransformOriginProperty() const noexcept { return transformOrigin_; }

    void Widget::setTransformOriginProperty(const Vector2 value)
    {
        if (value == transformOrigin_)
        {
            return;
        }
        transformOrigin_ = value;
        InvalidateTransform();
    }

    float Widget::getRotationProperty() const noexcept { return rotation_; }

    void Widget::setRotationProperty(const float value)
    {
        if (value == rotation_)
        {
            return;
        }
        rotation_ = value;
        InvalidateTransform();
    }

    const Rectangle& Widget::getBoundsProperty() const noexcept { return layoutBounds_; }

    Rectangle Widget::getActualBoundsProperty() const noexcept
    {
        return (Rectangle(0, 0, layoutBounds_.Width, layoutBounds_.Height) - margin_) - borderThickness_ - padding_;
    }

    const Rectangle& Widget::getContainerBoundsProperty() const noexcept { return containerBounds_; }

    int Widget::getMBPWidthProperty() const noexcept
    {
        return margin_.Left + margin_.Right + borderThickness_.Left + borderThickness_.Right + padding_.Left + padding_.Right;
    }

    int Widget::getMBPHeightProperty() const noexcept
    {
        return margin_.Top + margin_.Bottom + borderThickness_.Top + borderThickness_.Bottom + padding_.Top + padding_.Bottom;
    }

    Widget* Widget::getParentProperty() const noexcept { return parent_; }

    ILayout* Widget::getChildrenLayoutProperty() const noexcept { return childrenLayout_; }

    void Widget::setChildrenLayoutProperty(ILayout* const value) noexcept { childrenLayout_ = value; }

    const std::vector<std::shared_ptr<Widget>>& Widget::getChildrenProperty() const noexcept { return children_; }

    const std::vector<std::shared_ptr<Widget>>& Widget::getChildrenCopyProperty()
    {
        UpdateChildren();
        return childrenCopy_;
    }

    void Widget::AddChild(std::shared_ptr<Widget> child)
    {
        if (!child)
        {
            throw std::invalid_argument("A widget child cannot be null.");
        }
        if (child.get() == this)
        {
            throw std::invalid_argument("A widget cannot be its own child.");
        }
        if (child->parent_ == this)
        {
            return;
        }
        if (child->parent_ != nullptr)
        {
            static_cast<void>(child->parent_->RemoveChild(child.get()));
        }

        children_.push_back(std::move(child));
        OnChildAdded(*children_.back());
        childrenDirty_ = true;
        InvalidateMeasure();
    }

    bool Widget::RemoveChild(const Widget* const child)
    {
        const auto iterator = std::find_if(children_.begin(), children_.end(), [child](const auto& item) {
            return item.get() == child;
        });
        if (iterator == children_.end())
        {
            return false;
        }

        std::shared_ptr<Widget> removed = *iterator;
        children_.erase(iterator);
        OnChildRemoved(*removed);
        childrenDirty_ = true;
        InvalidateMeasure();
        return true;
    }

    void Widget::ClearChildren()
    {
        for (const std::shared_ptr<Widget>& child : children_)
        {
            OnChildRemoved(*child);
        }
        children_.clear();
        childrenCopy_.clear();
        childrenDirty_ = true;
        InvalidateMeasure();
    }

    void Widget::RemoveFromParent()
    {
        if (parent_ != nullptr)
        {
            static_cast<void>(parent_->RemoveChild(this));
        }
    }

    Point Widget::Measure(Point availableSize)
    {
        if (!measureDirty_ && lastMeasureAvailableSize_ == availableSize)
        {
            return lastMeasureSize_;
        }

        if (width_.has_value() && availableSize.X > *width_)
        {
            availableSize.X = *width_;
        }
        else if (maxWidth_.has_value() && availableSize.X > *maxWidth_)
        {
            availableSize.X = *maxWidth_;
        }
        if (height_.has_value() && availableSize.Y > *height_)
        {
            availableSize.Y = *height_;
        }
        else if (maxHeight_.has_value() && availableSize.Y > *maxHeight_)
        {
            availableSize.Y = *maxHeight_;
        }

        availableSize.X -= getMBPWidthProperty();
        availableSize.Y -= getMBPHeightProperty();
        Point result = InternalMeasure(availableSize);
        result.X += getMBPWidthProperty();
        result.Y += getMBPHeightProperty();

        if (width_.has_value())
        {
            result.X = *width_;
        }
        else
        {
            if (minWidth_.has_value() && result.X < *minWidth_)
            {
                result.X = *minWidth_;
            }
            if (maxWidth_.has_value() && result.X > *maxWidth_)
            {
                result.X = *maxWidth_;
            }
        }
        if (height_.has_value())
        {
            result.Y = *height_;
        }
        else
        {
            if (minHeight_.has_value() && result.Y < *minHeight_)
            {
                result.Y = *minHeight_;
            }
            if (maxHeight_.has_value() && result.Y > *maxHeight_)
            {
                result.Y = *maxHeight_;
            }
        }

        lastMeasureSize_ = result;
        lastMeasureAvailableSize_ = availableSize;
        measureDirty_ = false;
        return result;
    }

    void Widget::Arrange(const Rectangle containerBounds)
    {
        if (!arrangeDirty_ && containerBounds_ == containerBounds)
        {
            return;
        }
        arrangeDirty_ = true;
        containerBounds_ = containerBounds;
        UpdateArrange();
    }

    void Widget::UpdateArrange()
    {
        if (!arrangeDirty_)
        {
            return;
        }

        Point size;
        const Point containerSize(containerBounds_.Width, containerBounds_.Height);
        if (horizontalAlignment_ != HorizontalAlignment::Stretch || verticalAlignment_ != VerticalAlignment::Stretch)
        {
            size = Measure(containerSize);
        }
        else
        {
            size = containerSize;
        }
        size.X = std::min(size.X, containerBounds_.Width);
        size.Y = std::min(size.Y, containerBounds_.Height);

        Point alignedContainerSize = containerSize;
        if (horizontalAlignment_ == HorizontalAlignment::Stretch && width_.has_value() && *width_ < alignedContainerSize.X)
        {
            alignedContainerSize.X = *width_;
        }
        if (verticalAlignment_ == VerticalAlignment::Stretch && height_.has_value() && *height_ < alignedContainerSize.Y)
        {
            alignedContainerSize.Y = *height_;
        }

        layoutBounds_ = LayoutUtils::Align(alignedContainerSize, size, horizontalAlignment_, verticalAlignment_);
        layoutBounds_.Offset(containerBounds_.X, containerBounds_.Y);
        InvalidateTransform();
        InternalArrange();
        Utility::EventsExtensions::Invoke(ArrangeUpdated, this, InputEventType::ArrangeUpdated);
        arrangeDirty_ = false;
    }

    void Widget::InvalidateMeasure()
    {
        measureDirty_ = true;
        InvalidateArrange();
        if (parent_ != nullptr)
        {
            parent_->InvalidateMeasure();
        }
    }

    void Widget::InvalidateArrange() noexcept { arrangeDirty_ = true; }

    void Widget::OnAttachedPropertyLayoutChanged(const MML::AttachedPropertyOption option)
    {
        switch (option)
        {
            case MML::AttachedPropertyOption::None:
                break;
            case MML::AttachedPropertyOption::AffectsArrange:
                InvalidateArrange();
                break;
            case MML::AttachedPropertyOption::AffectsMeasure:
                InvalidateMeasure();
                break;
        }
    }

    Vector2 Widget::ToLocal(const Vector2 source)
    {
        return getTransformProperty().InverseApply(source);
    }

    Vector2 Widget::ToGlobal(const Vector2 position)
    {
        return getTransformProperty().Apply(position);
    }

    Point Widget::ToLocal(const Point source)
    {
        return Utility::Mathematics::ToPoint(ToLocal(Vector2(static_cast<float>(source.X), static_cast<float>(source.Y))));
    }

    Point Widget::ToGlobal(const Point position)
    {
        return Utility::Mathematics::ToPoint(ToGlobal(Vector2(static_cast<float>(position.X), static_cast<float>(position.Y))));
    }

    bool Widget::ContainsGlobalPoint(const Point globalPosition)
    {
        const Point localPosition = ToLocal(globalPosition);
        const Rectangle borderBounds = Rectangle(0, 0, layoutBounds_.Width, layoutBounds_.Height) - margin_;
        return borderBounds.Contains(localPosition);
    }

    Point Widget::InternalMeasure(const Point availableSize)
    {
        if (childrenLayout_ == nullptr)
        {
            return Point(0, 0);
        }
        return childrenLayout_->Measure(getChildrenCopyProperty(), availableSize);
    }

    void Widget::InternalArrange()
    {
        if (childrenLayout_ != nullptr)
        {
            childrenLayout_->Arrange(getChildrenCopyProperty(), getActualBoundsProperty());
        }
    }

    void Widget::OnVisibleChanged()
    {
        InvalidateMeasure();
        Utility::EventsExtensions::Invoke(VisibleChanged, this, InputEventType::VisibleChanged);
    }

    void Widget::OnChildAdded(Widget& child)
    {
        child.parent_ = this;
        child.InvalidateTransform();
    }

    void Widget::OnChildRemoved(Widget& child)
    {
        if (child.parent_ == this)
        {
            child.parent_ = nullptr;
            child.InvalidateTransform();
        }
    }

    void Widget::InvalidateTransform()
    {
        transformDirty_ = true;
        for (const std::shared_ptr<Widget>& child : getChildrenCopyProperty())
        {
            child->InvalidateTransform();
        }
    }

    const Graphics2D::Transform& Widget::getTransformProperty()
    {
        UpdateTransform();
        return *transform_;
    }

    void Widget::UpdateTransform()
    {
        if (!transformDirty_)
        {
            return;
        }

        const Point point(layoutBounds_.X + left_, layoutBounds_.Y + top_);
        Graphics2D::Transform localTransform(
            Vector2(static_cast<float>(point.X), static_cast<float>(point.Y)),
            Vector2(transformOrigin_.X * static_cast<float>(layoutBounds_.Width),
                transformOrigin_.Y * static_cast<float>(layoutBounds_.Height)),
            scale_, rotation_ * std::numbers::pi_v<float> / 180.0F);
        if (parent_ != nullptr)
        {
            Graphics2D::Transform parentTransform = parent_->getTransformProperty();
            parentTransform.AddTransform(localTransform);
            transform_ = std::move(parentTransform);
        }
        else
        {
            transform_ = std::move(localTransform);
        }
        transformDirty_ = false;
    }

    void Widget::UpdateChildren()
    {
        if (!childrenDirty_)
        {
            return;
        }
        childrenCopy_ = children_;
        std::stable_sort(childrenCopy_.begin(), childrenCopy_.end(), [](const auto& left, const auto& right) {
            return left->getZIndexProperty() < right->getZIndexProperty();
        });
        childrenDirty_ = false;
    }

    void Widget::FireLocationChanged()
    {
        Utility::EventsExtensions::Invoke(LocationChanged, this, InputEventType::LocationChanged);
    }

    void Widget::FireSizeChanged()
    {
        Utility::EventsExtensions::Invoke(SizeChanged, this, InputEventType::SizeChanged);
    }
}
