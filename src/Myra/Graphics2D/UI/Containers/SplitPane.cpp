// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/SplitPane.cs,
// src/Myra/Graphics2D/UI/Containers/HorizontalSplitPane.cs, and src/Myra/Graphics2D/UI/Containers/VerticalSplitPane.cs
// at 0d79b939310bfe1d00b21803fe15e291caf60aa1. See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/SplitPane.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#ifdef MYRA_CNA_HAS_CNA_TARGET
#include "CNA/Platform/PlatformException.hpp"
#endif
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] std::int64_t CheckedWideAdd(const std::int64_t left, const std::int64_t right,
                                                  const char *const message)
        {
            if ((right > 0 && left > std::numeric_limits<std::int64_t>::max() - right) ||
                (right < 0 && left < std::numeric_limits<std::int64_t>::min() - right))
            {
                throw std::overflow_error(message);
            }
            return left + right;
        }

        [[nodiscard]] std::int64_t CheckedWideSubtract(const std::int64_t left, const std::int64_t right,
                                                       const char *const message)
        {
            if ((right > 0 && left < std::numeric_limits<std::int64_t>::min() + right) ||
                (right < 0 && left > std::numeric_limits<std::int64_t>::max() + right))
            {
                throw std::overflow_error(message);
            }
            return left - right;
        }

        void ApplySplitPaneMouseCursor(const MouseCursorType value)
        {
#ifdef MYRA_CNA_HAS_CNA_TARGET
            try
            {
                MyraEnvironment::setMouseCursorTypeProperty(value);
            }
            catch (const CNA::Platform::PlatformException &)
            {
                // The backing state is assigned before CNA reports that a
                // native cursor is unavailable in a windowless backend.
            }
#else
            MyraEnvironment::setMouseCursorTypeProperty(value);
#endif
        }
    } // namespace

    SplitPane::SplitPane() : callbackState_(std::make_shared<CallbackState>())
    {
        callbackState_->Owner = this;
        setChildrenLayoutProperty(&layout_);
    }

    SplitPane::~SplitPane()
    {
        callbackState_->Owner = nullptr;
        try
        {
            EndHandleDrag();
        }
        catch (...)
        {
            // Destruction must still detach every native callback if a caller
            // installed an invalid cursor enum while a handle was active.
        }
        ClearHandleSubscriptions();
        UnsubscribeDesktopTouchMoved();
    }

    const std::vector<std::shared_ptr<Widget>> &SplitPane::getWidgetsProperty() const noexcept
    {
        return widgets_;
    }

    void SplitPane::AddWidget(std::shared_ptr<Widget> widget)
    {
        InsertWidget(widgets_.size(), std::move(widget));
    }

    void SplitPane::InsertWidget(const std::size_t index, std::shared_ptr<Widget> widget)
    {
        if (!widget)
        {
            throw std::invalid_argument("A SplitPane widget cannot be null.");
        }
        if (index > widgets_.size())
        {
            throw std::out_of_range("SplitPane widget index is outside the collection.");
        }
        if (std::find(widgets_.begin(), widgets_.end(), widget) != widgets_.end())
        {
            throw std::invalid_argument("A SplitPane cannot contain the same widget more than once.");
        }
        widgets_.insert(widgets_.begin() + static_cast<std::ptrdiff_t>(index), std::move(widget));
        Reset();
    }

    bool SplitPane::RemoveWidget(const Widget *const widget)
    {
        const auto iterator =
            std::find_if(widgets_.begin(), widgets_.end(), [widget](const auto &item) { return item.get() == widget; });
        if (iterator == widgets_.end())
        {
            return false;
        }
        widgets_.erase(iterator);
        Reset();
        return true;
    }

    void SplitPane::ClearWidgets()
    {
        widgets_.clear();
        Reset();
    }

    float SplitPane::GetProportion(const int widgetIndex) const noexcept
    {
        if (widgetIndex < 0 || static_cast<std::size_t>(widgetIndex) >= widgets_.size())
        {
            return 0.0F;
        }
        return GetActiveProportions()[widgetIndex * 2]->getValueProperty();
    }

    float SplitPane::GetSplitterPosition(const int leftWidgetIndex) const
    {
        std::shared_ptr<Proportion> left;
        std::shared_ptr<Proportion> right;
        float total = 0.0F;
        GetProportions(leftWidgetIndex, left, right, total);
        if (total == 0.0F)
        {
            return 0.0F;
        }
        return left->getValueProperty() / total;
    }

    void SplitPane::SetSplitterPosition(const int leftWidgetIndex, const float proportion)
    {
        std::shared_ptr<Proportion> left;
        std::shared_ptr<Proportion> right;
        float total = 0.0F;
        GetProportions(leftWidgetIndex, left, right, total);

        const float first = proportion * total;
        const float second = left->getValueProperty() + right->getValueProperty() - first;
        left->setValueProperty(first);
        right->setValueProperty(second);
        InvalidateArrange();
    }

    void SplitPane::Reset()
    {
        EndHandleDrag();
        ClearHandleSubscriptions();
        ClearChildren();
        handles_.clear();
        layout_.getColumnsProportionsProperty().Clear();
        layout_.getRowsProportionsProperty().Clear();

        for (std::size_t index = 0; index < widgets_.size(); ++index)
        {
            if (index > 0)
            {
                const auto handle = std::make_shared<Button>();
                handle->releaseOnTouchLeft_ = false;
                if (getOrientationProperty() == Orientation::Horizontal)
                {
                    handle->setMouseCursorProperty(MouseCursorType::SizeWE);
                    handle->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
                    Grid::SetColumn(*handle, static_cast<int>(index * 2U - 1U));
                    layout_.getColumnsProportionsProperty().Add(std::make_shared<Proportion>(ProportionType::Auto));
                }
                else
                {
                    handle->setMouseCursorProperty(MouseCursorType::SizeNS);
                    handle->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
                    Grid::SetRow(*handle, static_cast<int>(index * 2U - 1U));
                    layout_.getRowsProportionsProperty().Add(std::make_shared<Proportion>(ProportionType::Auto));
                }

                const std::shared_ptr<CallbackState> callbackState = callbackState_;
                const Events::MyraEventHandler::Token token = handle->PressedChanged.Add(
                    [callbackState](void *sender, Events::MyraEventArgs &)
                    {
                        if (callbackState->Owner != nullptr)
                        {
                            callbackState->Owner->HandleOnPressedChanged(sender);
                        }
                    });
                handleSubscriptions_.push_back({handle, token});
                handles_.push_back(handle);
                AddChild(handle);
            }

            const std::shared_ptr<Proportion> proportion = std::make_shared<Proportion>(
                index + 1U < widgets_.size() ? ProportionType::Part : ProportionType::Fill, 1.0F);
            const std::shared_ptr<Widget> &widget = widgets_[index];
            if (getOrientationProperty() == Orientation::Horizontal)
            {
                Grid::SetColumn(*widget, static_cast<int>(index * 2U));
                layout_.getColumnsProportionsProperty().Add(proportion);
            }
            else
            {
                Grid::SetRow(*widget, static_cast<int>(index * 2U));
                layout_.getRowsProportionsProperty().Add(proportion);
            }
            AddChild(widget);
        }

        FireProportionsChanged();
    }

    void SplitPane::OnTouchMoved()
    {
        Widget::OnTouchMoved();
        UpdateHandleDrag();
    }

    void SplitPane::OnPlacedChanged()
    {
        EndHandleDrag();
        UnsubscribeDesktopTouchMoved();
        SubscribeDesktopTouchMoved();
        Widget::OnPlacedChanged();
    }

    void SplitPane::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const splitPane = dynamic_cast<const SplitPane *>(&source);
        if (splitPane == nullptr)
        {
            throw std::invalid_argument("SplitPane copy source must be a SplitPane.");
        }
        for (const std::shared_ptr<Widget> &widget : splitPane->widgets_)
        {
            AddWidget(widget->Clone());
        }
    }

    const ProportionCollection &SplitPane::GetActiveProportions() const noexcept
    {
        return getOrientationProperty() == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                                   : layout_.getRowsProportionsProperty();
    }

    ProportionCollection &SplitPane::GetActiveProportions() noexcept
    {
        return getOrientationProperty() == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                                   : layout_.getRowsProportionsProperty();
    }

    void SplitPane::GetProportions(const int leftWidgetIndex, std::shared_ptr<Proportion> &left,
                                   std::shared_ptr<Proportion> &right, float &total) const
    {
        if (leftWidgetIndex < 0 || static_cast<std::size_t>(leftWidgetIndex) + 1U >= widgets_.size())
        {
            throw std::out_of_range("SplitPane splitter index is outside the collection.");
        }
        const ProportionCollection &proportions = GetActiveProportions();
        total = 0.0F;
        for (int index = 0; index < proportions.getCountProperty(); index += 2)
        {
            total += proportions[index]->getValueProperty();
        }
        const int baseIndex = leftWidgetIndex * 2;
        left = proportions[baseIndex];
        right = proportions[baseIndex + 2];
    }

    void SplitPane::HandleOnPressedChanged(void *const sender)
    {
        const auto iterator = std::find_if(handles_.begin(), handles_.end(),
                                           [sender](const auto &handle) { return handle.get() == sender; });
        if (iterator == handles_.end())
        {
            return;
        }

        if (!(*iterator)->getIsPressedProperty())
        {
            EndHandleDrag();
            return;
        }
        BeginHandleDrag(*iterator);
    }

    void SplitPane::BeginHandleDrag(const std::shared_ptr<Button> &handle)
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr || !desktop->getTouchPositionProperty())
        {
            return;
        }

        EndHandleDrag();
        activeHandle_ = handle;
        const Microsoft::Xna::Framework::Point handlePosition =
            handle->ToGlobal(handle->getBoundsProperty().getLocationProperty());
        mouseCoord_ = getOrientationProperty() == Orientation::Horizontal
                          ? static_cast<std::int64_t>(desktop->getTouchPositionProperty()->X) - handlePosition.X
                          : static_cast<std::int64_t>(desktop->getTouchPositionProperty()->Y) - handlePosition.Y;

        MyraEnvironment::setSetMouseCursorFromWidgetProperty(false);
        ApplySplitPaneMouseCursor(getOrientationProperty() == Orientation::Horizontal ? MouseCursorType::SizeWE
                                                                                      : MouseCursorType::SizeNS);
    }

    void SplitPane::EndHandleDrag()
    {
        if (!activeHandle_ && !mouseCoord_)
        {
            return;
        }

        activeHandle_.reset();
        mouseCoord_.reset();
        MyraEnvironment::setSetMouseCursorFromWidgetProperty(true);
        ApplySplitPaneMouseCursor(
            getMouseCursorProperty().value_or(MyraEnvironment::getDefaultMouseCursorTypeProperty()));
    }

    void SplitPane::UpdateHandleDrag()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr || !desktop->getTouchPositionProperty() || !activeHandle_ || !mouseCoord_)
        {
            return;
        }

        const Microsoft::Xna::Framework::Rectangle bounds = getBoundsProperty();
        if (bounds.Width == 0)
        {
            return;
        }

        const auto handleIterator = std::find_if(getChildrenProperty().begin(), getChildrenProperty().end(),
                                                 [this](const auto &child) { return child == activeHandle_; });
        if (handleIterator == getChildrenProperty().end())
        {
            return;
        }
        const std::size_t handleIndex =
            static_cast<std::size_t>(std::distance(getChildrenProperty().begin(), handleIterator));
        ProportionCollection &proportions = GetActiveProportions();
        if (handleIndex == 0U || handleIndex + 1U >= static_cast<std::size_t>(proportions.getCountProperty()))
        {
            return;
        }

        const Microsoft::Xna::Framework::Point position = ToLocal(*desktop->getTouchPositionProperty());
        std::int64_t firstExtent = getOrientationProperty() == Orientation::Horizontal
                                       ? static_cast<std::int64_t>(position.X) - *mouseCoord_
                                       : static_cast<std::int64_t>(position.Y) - *mouseCoord_;
        for (std::size_t index = 0; index + 1U < handleIndex; ++index)
        {
            const std::int64_t cellExtent =
                getOrientationProperty() == Orientation::Horizontal
                    ? static_cast<std::int64_t>(layout_.GetColumnWidth(static_cast<int>(index)))
                    : static_cast<std::int64_t>(layout_.GetRowHeight(static_cast<int>(index)));
            firstExtent = CheckedWideSubtract(firstExtent, cellExtent,
                                              "Split-pane preceding cell geometry exceeds the widened range.");
        }

        std::int64_t handlesSize = 0;
        for (std::size_t index = 1; index < getChildrenProperty().size(); index += 2U)
        {
            const std::int64_t handleExtent =
                getOrientationProperty() == Orientation::Horizontal
                    ? static_cast<std::int64_t>(layout_.GetColumnWidth(static_cast<int>(index)))
                    : static_cast<std::int64_t>(layout_.GetRowHeight(static_cast<int>(index)));
            handlesSize =
                CheckedWideAdd(handlesSize, handleExtent, "Split-pane handle geometry exceeds the widened range.");
        }
        const std::int64_t paneExtent = getOrientationProperty() == Orientation::Horizontal
                                            ? static_cast<std::int64_t>(bounds.Width)
                                            : static_cast<std::int64_t>(bounds.Height);
        const std::int64_t availableExtent =
            CheckedWideSubtract(paneExtent, handlesSize, "Split-pane available geometry exceeds the widened range.");
        if (availableExtent <= 0)
        {
            InvalidateArrange();
            return;
        }

        const double firstValue = static_cast<double>(widgets_.size()) * static_cast<double>(firstExtent) /
                                  static_cast<double>(availableExtent);
        const std::shared_ptr<Proportion> &first = proportions[static_cast<int>(handleIndex - 1U)];
        const std::shared_ptr<Proportion> &second = proportions[static_cast<int>(handleIndex + 1U)];
        const double secondValue =
            static_cast<double>(first->getValueProperty()) + second->getValueProperty() - firstValue;
        const double floatMaximum = std::numeric_limits<float>::max();
        if (std::isfinite(firstValue) && std::isfinite(secondValue) && firstValue >= 0.0 && secondValue >= 0.0 &&
            firstValue <= floatMaximum && secondValue <= floatMaximum)
        {
            const float newFirst = static_cast<float>(firstValue);
            const float newSecond = static_cast<float>(secondValue);
            if (!Utility::Mathematics::EpsilonEquals(first->getValueProperty(), newFirst) ||
                !Utility::Mathematics::EpsilonEquals(second->getValueProperty(), newSecond))
            {
                first->setValueProperty(newFirst);
                second->setValueProperty(newSecond);
                FireProportionsChanged();
            }
        }
        InvalidateArrange();
    }

    void SplitPane::SubscribeDesktopTouchMoved()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr)
        {
            return;
        }

        const std::shared_ptr<SplitPane> retainedTarget = std::static_pointer_cast<SplitPane>(RetainSelf());
        if (!retainedTarget)
        {
            throw std::logic_error("A placed split pane is missing from its owning collection.");
        }

        touchMovedSubscriptionDesktop_ = desktop;
        try
        {
            touchMovedToken_ = desktop->TouchMoved.Add([retainedTarget](void *, Events::MyraEventArgs &)
                                                       { retainedTarget->DesktopTouchMoved(); });
        }
        catch (...)
        {
            UnsubscribeDesktopTouchMoved();
            throw;
        }
    }

    void SplitPane::UnsubscribeDesktopTouchMoved() noexcept
    {
        try
        {
            if (touchMovedSubscriptionDesktop_ != nullptr)
            {
                static_cast<void>(touchMovedSubscriptionDesktop_->TouchMoved.Remove(touchMovedToken_));
            }
        }
        catch (...)
        {
        }
        touchMovedSubscriptionDesktop_ = nullptr;
        touchMovedToken_ = Events::MyraEventHandler::InvalidToken;
    }

    void SplitPane::DesktopTouchMoved()
    {
        if (touchMovedSubscriptionDesktop_ == nullptr || getDesktopProperty() != touchMovedSubscriptionDesktop_)
        {
            return;
        }
        UpdateHandleDrag();
    }

    void SplitPane::ClearHandleSubscriptions() noexcept
    {
        for (const HandleSubscription &subscription : handleSubscriptions_)
        {
            try
            {
                static_cast<void>(subscription.Handle->PressedChanged.Remove(subscription.Token));
            }
            catch (...)
            {
            }
        }
        handleSubscriptions_.clear();
    }

    void SplitPane::FireProportionsChanged()
    {
        Utility::EventsExtensions::Invoke(ProportionsChanged, this, InputEventType::ProportionChanged);
    }

    Orientation HorizontalSplitPane::getOrientationProperty() const noexcept
    {
        return Orientation::Horizontal;
    }

    std::shared_ptr<Widget> HorizontalSplitPane::CreateCloneInstance() const
    {
        return std::make_shared<HorizontalSplitPane>();
    }

    Orientation VerticalSplitPane::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    std::shared_ptr<Widget> VerticalSplitPane::CreateCloneInstance() const
    {
        return std::make_shared<VerticalSplitPane>();
    }
} // namespace Myra::Graphics2D::UI
