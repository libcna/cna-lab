// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/Slider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/Slider.hpp"

#include <cmath>
#include <stdexcept>

#include "Myra/Graphics2D/UI/Simple/Image.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    Slider::Slider() : layout_(*this)
    {
        auto imageButton = std::make_shared<Button>();
        imageButton->setContentProperty(std::make_shared<Image>());
        imageButton->releaseOnTouchLeft_ = false;
        setChildrenLayoutProperty(&layout_);
        layout_.setChildProperty(std::move(imageButton));
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

    void Slider::InternalArrange()
    {
        Widget::InternalArrange();
        SyncHintWithValue();
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
        layout_.setChildProperty(std::dynamic_pointer_cast<Button>(slider->getImageButtonProperty()->Clone()));
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
                   ? getBoundsProperty().Width - imageButton->getBoundsProperty().Width -
                         getMarginProperty().getWidthProperty()
                   : getBoundsProperty().Height - imageButton->getBoundsProperty().Height -
                         getMarginProperty().getHeightProperty();
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
} // namespace Myra::Graphics2D::UI
