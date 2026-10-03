// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/Slider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/Slider.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] int CheckedHintDifference(const int first, const int second, const int third,
                                                const char *const message)
        {
            const std::int64_t result = static_cast<std::int64_t>(first) - second - third;
            if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(result);
        }
    } // namespace

    Slider::Slider() : layout_(*this)
    {
        auto imageButton = std::make_shared<Button>();
        imageButton->setContentProperty(std::make_shared<Image>());
        imageButton->releaseOnTouchLeft_ = false;
        setChildrenLayoutProperty(&layout_);
        layout_.setChildProperty(std::move(imageButton));
    }

    Slider::~Slider()
    {
        UnsubscribeDesktopTouchMoved();
    }

    float Slider::getMinimumProperty() const noexcept
    {
        return minimum_;
    }

    void Slider::setMinimumProperty(const float value) noexcept
    {
        minimum_ = value;
    }

    float Slider::getMaximumProperty() const noexcept
    {
        return maximum_;
    }

    void Slider::setMaximumProperty(const float value) noexcept
    {
        maximum_ = value;
    }

    float Slider::getValueProperty() const noexcept
    {
        return value_;
    }

    void Slider::setValueProperty(float value)
    {
        if (value > maximum_)
        {
            value = maximum_;
        }
        if (value < minimum_)
        {
            value = minimum_;
        }
        if (value_ == value)
        {
            return;
        }

        const float oldValue = value_;
        value_ = value;
        SyncHintWithValue();
        Events::ValueChangedEventArgs<float> arguments(oldValue, value_);
        ValueChanged.Invoke(this, arguments);
    }

    bool Slider::getWheelAdjustmentProperty() const noexcept
    {
        return wheelAdjustment_;
    }

    void Slider::setWheelAdjustmentProperty(const bool value) noexcept
    {
        wheelAdjustment_ = value;
    }

    float Slider::getWheelStepProperty() const noexcept
    {
        return wheelStep_;
    }

    void Slider::setWheelStepProperty(const float value) noexcept
    {
        wheelStep_ = value;
    }

    std::shared_ptr<Button> Slider::getImageButtonProperty() const
    {
        return layout_.getChildProperty();
    }

    bool Slider::getAcceptsMouseWheelProperty() const noexcept
    {
        return wheelAdjustment_;
    }

    void Slider::OnTouchDown()
    {
        Widget::OnTouchDown();
        UpdateHint();
        getImageButtonProperty()->setIsPressedProperty(true);
    }

    void Slider::OnMouseWheel(const float delta)
    {
        Widget::OnMouseWheel(delta);
        if (!wheelAdjustment_)
        {
            return;
        }

        const float previousValue = value_;
        setValueProperty(delta < 0.0F ? value_ - wheelStep_ : value_ + wheelStep_);
        if (value_ == previousValue)
        {
            return;
        }

        Events::ValueChangedEventArgs<float> generalArguments(previousValue, value_);
        ValueChanged.Invoke(this, generalArguments);
        Events::ValueChangedEventArgs<float> userArguments(previousValue, value_);
        ValueChangedByUser.Invoke(this, userArguments);
    }

    void Slider::InternalArrange()
    {
        Widget::InternalArrange();
        SyncHintWithValue();
    }

    void Slider::OnPlacedChanged()
    {
        UnsubscribeDesktopTouchMoved();
        SubscribeDesktopTouchMoved();
        Widget::OnPlacedChanged();
    }

    void Slider::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const slider = dynamic_cast<const Slider *>(&source);
        if (slider == nullptr)
        {
            throw std::invalid_argument("Slider copy source must be a Slider.");
        }

        setMinimumProperty(slider->minimum_);
        setMaximumProperty(slider->maximum_);
        setValueProperty(slider->value_);
        setWheelAdjustmentProperty(slider->wheelAdjustment_);
        setWheelStepProperty(slider->wheelStep_);
        std::shared_ptr<Button> clonedKnob =
            std::dynamic_pointer_cast<Button>(slider->getImageButtonProperty()->Clone());
        if (!clonedKnob)
        {
            throw std::logic_error("A cloned slider knob must remain a Button.");
        }
        clonedKnob->releaseOnTouchLeft_ = false;
        layout_.setChildProperty(std::move(clonedKnob));
    }

    int Slider::getHint() const
    {
        const std::shared_ptr<Button> imageButton = getImageButtonProperty();
        return getOrientationProperty() == Orientation::Horizontal ? imageButton->getLeftProperty()
                                                                   : imageButton->getTopProperty();
    }

    void Slider::setHint(const int value)
    {
        const std::shared_ptr<Button> imageButton = getImageButtonProperty();
        if (getOrientationProperty() == Orientation::Horizontal)
        {
            imageButton->setLeftProperty(value);
        }
        else
        {
            imageButton->setTopProperty(value);
        }
    }

    int Slider::getMaxHint() const
    {
        const std::shared_ptr<Button> imageButton = getImageButtonProperty();
        return getOrientationProperty() == Orientation::Horizontal
                   ? CheckedHintDifference(getBoundsProperty().Width, imageButton->getBoundsProperty().Width,
                                           getMarginProperty().getWidthProperty(),
                                           "Horizontal slider hint range exceeds the supported integer range.")
                   : CheckedHintDifference(getBoundsProperty().Height, imageButton->getBoundsProperty().Height,
                                           getMarginProperty().getHeightProperty(),
                                           "Vertical slider hint range exceeds the supported integer range.");
    }

    void Slider::SyncHintWithValue()
    {
        const float range = maximum_ - minimum_;
        if (range == 0.0F || !std::isfinite(range) || !std::isfinite(value_))
        {
            setHint(0);
            return;
        }
        const float hint = static_cast<float>(getMaxHint()) * ((value_ - minimum_) / range);
        if (!std::isfinite(hint))
        {
            setHint(0);
            return;
        }
        setHint(Utility::Mathematics::TruncateToInt(hint));
    }

    void Slider::UpdateHint()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr || !desktop->getTouchPositionProperty())
        {
            return;
        }

        const Microsoft::Xna::Framework::Point position = ToLocal(*desktop->getTouchPositionProperty());
        const Microsoft::Xna::Framework::Rectangle knobBounds = getImageButtonProperty()->getActualBoundsProperty();
        int hint = getOrientationProperty() == Orientation::Horizontal
                       ? CheckedHintDifference(position.X, knobBounds.Width / 2, getMarginProperty().Left,
                                               "Horizontal slider pointer hint exceeds the supported integer range.")
                       : CheckedHintDifference(position.Y, knobBounds.Height / 2, getMarginProperty().Top,
                                               "Vertical slider pointer hint exceeds the supported integer range.");
        const int maxHint = getMaxHint();
        if (hint < 0)
        {
            hint = 0;
        }
        if (hint > maxHint)
        {
            hint = maxHint;
        }

        const float oldValue = value_;
        bool valueChanged = false;
        if (maxHint != 0)
        {
            const float newValue =
                minimum_ + static_cast<float>(hint) * (maximum_ - minimum_) / static_cast<float>(maxHint);
            if (value_ != newValue)
            {
                value_ = newValue;
                valueChanged = true;
            }
        }
        setHint(hint);

        if (valueChanged)
        {
            Events::ValueChangedEventArgs<float> generalArguments(oldValue, value_);
            ValueChanged.Invoke(this, generalArguments);
            Events::ValueChangedEventArgs<float> userArguments(oldValue, value_);
            ValueChangedByUser.Invoke(this, userArguments);
        }
    }

    void Slider::SubscribeDesktopTouchMoved()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr)
        {
            return;
        }

        const std::shared_ptr<Slider> retainedTarget = std::static_pointer_cast<Slider>(RetainSelf());
        if (!retainedTarget)
        {
            throw std::logic_error("A placed slider is missing from its owning collection.");
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

    void Slider::UnsubscribeDesktopTouchMoved() noexcept
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

    void Slider::DesktopTouchMoved()
    {
        if (touchMovedSubscriptionDesktop_ == nullptr || getDesktopProperty() != touchMovedSubscriptionDesktop_ ||
            !getImageButtonProperty()->getIsPressedProperty())
        {
            return;
        }
        UpdateHint();
    }
} // namespace Myra::Graphics2D::UI
