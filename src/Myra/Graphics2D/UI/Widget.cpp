// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs, src/Myra/Graphics2D/UI/Widget.Children.cs,
// and src/Myra/Graphics2D/UI/Widget.Input.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"
#include "Myra/Utility/UIUtils.hpp"
#include "Myra/Graphics2D/UI/LayoutUtils.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        int CheckedAdd(const int left, const int right, const char* const message)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) + right;
            if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(result);
        }

        int CheckedSubtract(const int left, const int right, const char* const message)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) - right;
            if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(result);
        }
    }

    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    Widget::Widget() : dragHandle_(this) {}

    Widget::~Widget()
    {
        ClearChildren();
    }

    const std::optional<std::string>& Widget::getStyleNameProperty() const noexcept { return styleName_; }

    void Widget::setStyleNameProperty(std::optional<std::string> value)
    {
        styleName_ = std::move(value);
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
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
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

    DragDirection Widget::getDragDirectionProperty() const noexcept { return dragDirection_; }

    void Widget::setDragDirectionProperty(const DragDirection value) noexcept { dragDirection_ = value; }

    bool Widget::getIsDraggableProperty() const noexcept { return dragDirection_ != DragDirection::None; }

    int Widget::getZIndexProperty() const noexcept { return zIndex_; }

    void Widget::setZIndexProperty(const int value)
    {
        if (value == zIndex_)
        {
            return;
        }
        zIndex_ = value;
        if (parent_ != nullptr)
        {
            parent_->childrenDirty_ = true;
        }
        else if (desktop_ != nullptr)
        {
            desktop_->InvalidateWidgetsOrder();
        }
        InvalidateMeasure();
    }

    const std::optional<MouseCursorType>& Widget::getMouseCursorProperty() const noexcept
    {
        return mouseCursor_;
    }

    void Widget::setMouseCursorProperty(std::optional<MouseCursorType> value)
    {
        if (value == mouseCursor_)
        {
            return;
        }
        mouseCursor_ = value;
        const std::vector<std::shared_ptr<Widget>> snapshot = children_;
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            child->setMouseCursorProperty(value);
        }
    }

    const std::optional<std::string>& Widget::getTooltipProperty() const noexcept { return tooltip_; }

    void Widget::setTooltipProperty(std::optional<std::string> value)
    {
        tooltip_ = std::move(value);
    }

    bool Widget::getIsModalProperty() const noexcept { return isModal_; }

    void Widget::setIsModalProperty(const bool value) noexcept { isModal_ = value; }

    float Widget::getOpacityProperty() const noexcept { return opacity_; }

    void Widget::setOpacityProperty(const float value)
    {
        if (!std::isfinite(value) || value < 0.0F || value > 1.0F)
        {
            throw std::out_of_range("Widget opacity must be in the range [0, 1].");
        }
        opacity_ = value;
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getBackgroundProperty() const
    {
        return backgrounds_[WidgetVisualStateNormal];
    }

    void Widget::setBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        backgrounds_[WidgetVisualStateNormal] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getOverBackgroundProperty() const
    {
        return backgrounds_[WidgetVisualStateOver];
    }

    void Widget::setOverBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        backgrounds_[WidgetVisualStateOver] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getDisabledBackgroundProperty() const
    {
        return backgrounds_[WidgetVisualStateDisabled];
    }

    void Widget::setDisabledBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        backgrounds_[WidgetVisualStateDisabled] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getFocusedBackgroundProperty() const
    {
        return backgrounds_[WidgetVisualStateFocused];
    }

    void Widget::setFocusedBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        backgrounds_[WidgetVisualStateFocused] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getPressedBackgroundProperty() const
    {
        return backgrounds_[WidgetVisualStatePressed];
    }

    void Widget::setPressedBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        backgrounds_[WidgetVisualStatePressed] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getBorderProperty() const
    {
        return borders_[WidgetVisualStateNormal];
    }

    void Widget::setBorderProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        borders_[WidgetVisualStateNormal] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getOverBorderProperty() const
    {
        return borders_[WidgetVisualStateOver];
    }

    void Widget::setOverBorderProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        borders_[WidgetVisualStateOver] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getDisabledBorderProperty() const
    {
        return borders_[WidgetVisualStateDisabled];
    }

    void Widget::setDisabledBorderProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        borders_[WidgetVisualStateDisabled] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getFocusedBorderProperty() const
    {
        return borders_[WidgetVisualStateFocused];
    }

    void Widget::setFocusedBorderProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        borders_[WidgetVisualStateFocused] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::getPressedBorderProperty() const
    {
        return borders_[WidgetVisualStatePressed];
    }

    void Widget::setPressedBorderProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        borders_[WidgetVisualStatePressed] = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::GetCurrentBackground() const
    {
        return GetCurrentVisual(backgrounds_);
    }

    std::shared_ptr<Graphics2D::IBrush> Widget::GetCurrentBorder() const
    {
        return GetCurrentVisual(borders_);
    }

    bool Widget::getIsPressedProperty() const noexcept { return isPressed_; }

    void Widget::setIsPressedProperty(const bool value)
    {
        if (value == isPressed_)
        {
            return;
        }
        isPressed_ = value;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            child->setIsPressedProperty(value);
        }
        OnPressedChanged();
    }

    bool Widget::getClipToBoundsProperty() const noexcept { return clipToBounds_; }

    void Widget::setClipToBoundsProperty(const bool value) noexcept { clipToBounds_ = value; }

    bool Widget::getAcceptsKeyboardFocusProperty() const noexcept { return acceptsKeyboardFocus_; }

    void Widget::setAcceptsKeyboardFocusProperty(const bool value) noexcept
    {
        acceptsKeyboardFocus_ = value;
    }

    bool Widget::getIsKeyboardFocusedProperty() const noexcept { return isKeyboardFocused_; }

    bool Widget::getIsPlacedProperty() const noexcept { return desktop_ != nullptr; }

    Desktop* Widget::getDesktopProperty() const noexcept { return desktop_; }

    void Widget::setIsKeyboardFocusedProperty(const bool value)
    {
        if (value == isKeyboardFocused_)
        {
            return;
        }
        isKeyboardFocused_ = value;
        Utility::EventsExtensions::Invoke(
            KeyboardFocusChanged, this, InputEventType::KeyboardFocusChanged);
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

    Widget* Widget::getDragHandleProperty() const noexcept { return dragHandle_; }

    void Widget::setDragHandleProperty(Widget* const value) noexcept { dragHandle_ = value; }

    Rectangle Widget::getBoundsProperty() const noexcept
    {
        return Rectangle(0, 0, layoutBounds_.Width, layoutBounds_.Height);
    }

    Rectangle Widget::getActualBoundsProperty() const
    {
        return (getBoundsProperty() - margin_) - borderThickness_ - padding_;
    }

    const Rectangle& Widget::getContainerBoundsProperty() const noexcept { return containerBounds_; }

    int Widget::getMBPWidthProperty() const
    {
        return CheckedAdd(CheckedAdd(margin_.getWidthProperty(), borderThickness_.getWidthProperty(),
                              "Widget horizontal border geometry exceeds the supported integer range."),
            padding_.getWidthProperty(), "Widget horizontal box geometry exceeds the supported integer range.");
    }

    int Widget::getMBPHeightProperty() const
    {
        return CheckedAdd(CheckedAdd(margin_.getHeightProperty(), borderThickness_.getHeightProperty(),
                              "Widget vertical border geometry exceeds the supported integer range."),
            padding_.getHeightProperty(), "Widget vertical box geometry exceeds the supported integer range.");
    }

    Widget* Widget::getParentProperty() const noexcept { return parent_; }

    const std::any& Widget::getTagProperty() const noexcept { return tag_; }

    void Widget::setTagProperty(std::any value)
    {
        tag_ = std::move(value);
    }

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
        for (const Widget* ancestor = this; ancestor != nullptr; ancestor = ancestor->parent_)
        {
            if (ancestor == child.get())
            {
                throw std::invalid_argument("Adding an ancestor would create a widget ownership cycle.");
            }
        }
        if (child->parent_ == this)
        {
            return;
        }
        while (child->parent_ != nullptr || child->desktop_ != nullptr)
        {
            if (child->parent_ == this)
            {
                return;
            }
            if (child->parent_ != nullptr)
            {
                Widget* const oldParent = child->parent_;
                if (!oldParent->RemoveChild(child.get()))
                {
                    throw std::logic_error("A widget parent did not retain its reported child.");
                }
            }
            else
            {
                Desktop* const oldDesktop = child->desktop_;
                if (!oldDesktop->RemoveWidget(child.get()))
                {
                    throw std::logic_error("A widget desktop did not retain its reported root.");
                }
            }
        }
        std::shared_ptr<Widget> retained = std::move(child);
        children_.push_back(retained);
        childrenDirty_ = true;
        OnChildAdded(*retained);
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
        childrenDirty_ = true;
        OnChildRemoved(*removed);
        InvalidateMeasure();
        return true;
    }

    void Widget::ClearChildren()
    {
        const std::vector<std::shared_ptr<Widget>> snapshot = children_;
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            const auto iterator = std::find(children_.begin(), children_.end(), child);
            if (iterator == children_.end())
            {
                continue;
            }

            std::shared_ptr<Widget> removed = *iterator;
            children_.erase(iterator);
            childrenDirty_ = true;
            OnChildRemoved(*removed);
        }
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

    void Widget::RemoveFromDesktop()
    {
        if (desktop_ != nullptr && parent_ == nullptr)
        {
            static_cast<void>(desktop_->RemoveWidget(this));
        }
    }

    void Widget::SetKeyboardFocus()
    {
        if (desktop_ == nullptr)
        {
            throw std::logic_error("Keyboard focus requires a widget attached to a desktop.");
        }
        desktop_->setFocusedKeyboardWidgetProperty(this);
    }

    std::shared_ptr<Widget> Widget::Clone() const
    {
        std::shared_ptr<Widget> result = CreateCloneInstance();
        if (!result)
        {
            throw std::logic_error("A widget clone factory returned null.");
        }
        if (result.get() == this)
        {
            throw std::logic_error("A widget clone factory must create a new instance.");
        }
        if (typeid(*result) != typeid(*this))
        {
            throw std::logic_error("A widget clone factory must preserve the exact dynamic type.");
        }

        result->CopyFrom(*this);
        for (const auto& [id, value] : AttachedPropertiesValues)
        {
            result->AttachedPropertiesValues.insert_or_assign(id, value);
        }
        return result;
    }

    std::shared_ptr<Widget> Widget::CreateCloneInstance() const
    {
        return std::make_shared<Widget>();
    }

    void Widget::CopyFrom(const Widget& source)
    {
        setStyleNameProperty(source.styleName_);
        setLeftProperty(source.left_);
        setTopProperty(source.top_);
        setMinWidthProperty(source.minWidth_);
        setMaxWidthProperty(source.maxWidth_);
        setWidthProperty(source.width_);
        setMinHeightProperty(source.minHeight_);
        setMaxHeightProperty(source.maxHeight_);
        setHeightProperty(source.height_);
        setMarginProperty(source.margin_);
        setBorderThicknessProperty(source.borderThickness_);
        setPaddingProperty(source.padding_);
        setHorizontalAlignmentProperty(source.horizontalAlignment_);
        setVerticalAlignmentProperty(source.verticalAlignment_);
        setEnabledProperty(source.enabled_);
        setVisibleProperty(source.visible_);
        setDragDirectionProperty(source.dragDirection_);
        setZIndexProperty(source.zIndex_);
        setMouseCursorProperty(source.mouseCursor_);
        setTooltipProperty(source.tooltip_);
        setScaleProperty(source.scale_);
        setTransformOriginProperty(source.transformOrigin_);
        setRotationProperty(source.rotation_);
        setDragHandleProperty(source.dragHandle_ == &source ? this : source.dragHandle_);
        setIsModalProperty(source.isModal_);
        setOpacityProperty(source.opacity_);
        setClipToBoundsProperty(source.clipToBounds_);
        setTagProperty(source.tag_);
        setAcceptsKeyboardFocusProperty(source.acceptsKeyboardFocus_);
        BeforeRender = source.BeforeRender;
        AfterRender = source.AfterRender;
        backgrounds_ = source.backgrounds_;
        borders_ = source.borders_;
    }

    std::size_t Widget::CalculateTotalChildCount(const bool visibleOnly)
    {
        std::size_t result = 0;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            if (visibleOnly && !child->getVisibleProperty())
            {
                continue;
            }
            ++result;
            result += child->CalculateTotalChildCount(visibleOnly);
        }
        return result;
    }

    Widget* Widget::FindChildById(const std::string& id)
    {
        return FindChild([&id](Widget& widget) {
            const std::optional<std::string>& widgetId = widget.getIdProperty();
            return widgetId.has_value() && *widgetId == id;
        });
    }

    Widget& Widget::EnsureWidgetById(const std::string& id)
    {
        Widget* const result = FindChildById(id);
        if (result == nullptr)
        {
            throw std::out_of_range("Could not find a descendant widget with id '" + id + "'.");
        }
        return *result;
    }

    Widget* Widget::FindChild(const WidgetPredicate& predicate)
    {
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& widget : snapshot)
        {
            if (!predicate || predicate(*widget))
            {
                return widget.get();
            }
            if (Widget* const descendant = widget->FindChild(predicate))
            {
                return descendant;
            }
        }
        return nullptr;
    }

    std::vector<Widget*> Widget::GetChildren(
        const bool recursive, const WidgetPredicate& predicate)
    {
        std::vector<Widget*> result;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& widget : snapshot)
        {
            if (!predicate || predicate(*widget))
            {
                result.push_back(widget.get());
            }
            if (recursive)
            {
                std::vector<Widget*> descendants = widget->GetChildren(true, predicate);
                result.insert(result.end(), descendants.begin(), descendants.end());
            }
        }
        return result;
    }

    Point Widget::Measure(Point availableSize)
    {
        if (!measureDirty_ && lastMeasureAvailableSize_ == availableSize)
        {
            return lastMeasureSize_;
        }
        const std::uint64_t invalidationVersion = measureInvalidationVersion_;
        const Point requestedAvailableSize = availableSize;

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

        availableSize.X = CheckedSubtract(availableSize.X, getMBPWidthProperty(),
            "Widget measure width exceeds the supported integer range.");
        availableSize.Y = CheckedSubtract(availableSize.Y, getMBPHeightProperty(),
            "Widget measure height exceeds the supported integer range.");
        Point result = InternalMeasure(availableSize);
        result.X = CheckedAdd(result.X, getMBPWidthProperty(),
            "Measured widget width exceeds the supported integer range.");
        result.Y = CheckedAdd(result.Y, getMBPHeightProperty(),
            "Measured widget height exceeds the supported integer range.");

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
        lastMeasureAvailableSize_ = requestedAvailableSize;
        measureDirty_ = measureInvalidationVersion_ != invalidationVersion;
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
        const std::uint64_t invalidationVersion = arrangeInvalidationVersion_;

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
        layoutBounds_.X = CheckedAdd(layoutBounds_.X, containerBounds_.X,
            "Arranged widget X coordinate exceeds the supported integer range.");
        layoutBounds_.Y = CheckedAdd(layoutBounds_.Y, containerBounds_.Y,
            "Arranged widget Y coordinate exceeds the supported integer range.");
        InvalidateTransform();
        InternalArrange();
        Utility::EventsExtensions::Invoke(ArrangeUpdated, this, InputEventType::ArrangeUpdated);
        arrangeDirty_ = arrangeInvalidationVersion_ != invalidationVersion;
    }

    void Widget::InvalidateMeasure()
    {
        if (suppressInvalidateMeasure_)
        {
            return;
        }
        ++measureInvalidationVersion_;
        measureDirty_ = true;
        InvalidateArrange();
        if (parent_ != nullptr)
        {
            parent_->InvalidateMeasure();
        }
        else if (desktop_ != nullptr)
        {
            desktop_->InvalidateLayout();
        }
    }

    void Widget::InvalidateArrange() noexcept
    {
        ++arrangeInvalidationVersion_;
        arrangeDirty_ = true;
    }

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

    void Widget::OnLostKeyboardFocus()
    {
        setIsKeyboardFocusedProperty(false);
    }

    void Widget::OnGotKeyboardFocus()
    {
        setIsKeyboardFocusedProperty(true);
    }

    void Widget::OnPressedChanged()
    {
        Utility::EventsExtensions::Invoke(PressedChanged, this, InputEventType::PressedChanged);
    }

    void Widget::OnPlacedChanged()
    {
        Utility::EventsExtensions::Invoke(PlacedChanged, this, InputEventType::PlacedChanged);
    }

    bool Widget::UseOverBackground() const noexcept
    {
        return false;
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
        const std::int64_t right = static_cast<std::int64_t>(borderBounds.X) + borderBounds.Width;
        const std::int64_t bottom = static_cast<std::int64_t>(borderBounds.Y) + borderBounds.Height;
        return borderBounds.X <= localPosition.X && localPosition.X < right &&
            borderBounds.Y <= localPosition.Y && localPosition.Y < bottom;
    }

    bool Widget::InputFallsThrough(const Point)
    {
        return false;
    }

    Point Widget::InternalMeasure(const Point availableSize)
    {
        if (childrenLayout_ == nullptr)
        {
            return Point(0, 0);
        }
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        return childrenLayout_->Measure(snapshot, availableSize);
    }

    void Widget::InternalArrange()
    {
        if (childrenLayout_ != nullptr)
        {
            const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
            childrenLayout_->Arrange(snapshot, getActualBoundsProperty());
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
        child.SetDesktop(desktop_);
        child.InvalidateTransform();
    }

    void Widget::OnChildRemoved(Widget& child)
    {
        if (child.parent_ == this)
        {
            try
            {
                child.SetDesktop(nullptr);
            }
            catch (...)
            {
                child.parent_ = nullptr;
                child.InvalidateTransform();
                throw;
            }
            child.parent_ = nullptr;
            child.InvalidateTransform();
        }
    }

    Rectangle Widget::getBorderBoundsProperty() const
    {
        return getBoundsProperty() - margin_;
    }

    Rectangle Widget::getBackgroundBoundsProperty() const
    {
        return getBorderBoundsProperty() - borderThickness_;
    }

    bool Widget::getSuppressInvalidateMeasureProperty() const noexcept
    {
        return suppressInvalidateMeasure_;
    }

    void Widget::setSuppressInvalidateMeasureProperty(const bool value) noexcept
    {
        suppressInvalidateMeasure_ = value;
    }

    void Widget::SetIsPressedByUser(const bool value)
    {
        if (value != isPressed_ && PressedChangingByUser)
        {
            Events::ValueChangingEventArgs<bool> arguments(isPressed_, value);
            PressedChangingByUser.Invoke(this, arguments);
            if (arguments.Cancel)
            {
                return;
            }
        }
        setIsPressedProperty(value);
    }

    void Widget::InvalidateTransform()
    {
        transformDirty_ = true;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            child->InvalidateTransform();
        }
    }

    void Widget::SetDesktop(Desktop* const value)
    {
        if (desktop_ == value)
        {
            return;
        }
        std::exception_ptr pendingException;
        if (desktop_ != nullptr)
        {
            try
            {
                desktop_->ClearFocusForDetaching(*this);
            }
            catch (...)
            {
                pendingException = std::current_exception();
            }
        }
        desktop_ = value;
        InvalidateTransform();
        if (desktop_ != nullptr)
        {
            InvalidateMeasure();
        }

        const std::vector<std::shared_ptr<Widget>> snapshot = children_;
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            if (child->parent_ != this)
            {
                continue;
            }
            try
            {
                child->SetDesktop(value);
            }
            catch (...)
            {
                if (!pendingException)
                {
                    pendingException = std::current_exception();
                }
            }
        }
        try
        {
            OnPlacedChanged();
        }
        catch (...)
        {
            if (!pendingException)
            {
                pendingException = std::current_exception();
            }
        }
        if (pendingException)
        {
            std::rethrow_exception(pendingException);
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

        const Point point(CheckedAdd(layoutBounds_.X, left_,
                              "Widget transform X coordinate exceeds the supported integer range."),
            CheckedAdd(layoutBounds_.Y, top_,
                "Widget transform Y coordinate exceeds the supported integer range."));
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
        else if (desktop_ != nullptr)
        {
            Graphics2D::Transform desktopTransform = desktop_->getTransformProperty();
            desktopTransform.AddTransform(localTransform);
            transform_ = std::move(desktopTransform);
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
        Utility::UIUtils::SortWidgetsByZIndex(childrenCopy_);
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
