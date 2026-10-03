// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/ProgressBar.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/ProgressBar.hpp"

#include <stdexcept>
#include <utility>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;

    ProgressBar::ProgressBar() = default;

    ProgressBar::~ProgressBar() = default;

    float ProgressBar::getMinimumProperty() const noexcept
    {
        return minimum_;
    }

    void ProgressBar::setMinimumProperty(const float value) noexcept
    {
        minimum_ = value;
    }

    float ProgressBar::getMaximumProperty() const noexcept
    {
        return maximum_;
    }

    void ProgressBar::setMaximumProperty(const float value) noexcept
    {
        maximum_ = value;
    }

    float ProgressBar::getValueProperty() const noexcept
    {
        return value_;
    }

    void ProgressBar::setValueProperty(const float value)
    {
        if (Utility::Mathematics::EpsilonEquals(value_, value))
        {
            return;
        }
        value_ = value;
        Utility::EventsExtensions::Invoke(
            ValueChanged, this, InputEventType::ValueChanged);
    }

    std::shared_ptr<Graphics2D::IBrush> ProgressBar::getFillerProperty() const
    {
        return filler_;
    }

    void ProgressBar::setFillerProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        filler_ = std::move(value);
    }

    void ProgressBar::InternalRender(Graphics2D::RenderContext& context)
    {
        Widget::InternalRender(context);

        const std::shared_ptr<Graphics2D::IBrush> filler = filler_;
        if (!filler)
        {
            return;
        }

        float value = value_;
        if (value < minimum_)
        {
            value = minimum_;
        }
        if (value > maximum_)
        {
            value = maximum_;
        }

        const float delta = maximum_ - minimum_;
        if (Utility::Mathematics::IsZero(delta))
        {
            return;
        }

        const float filledPart = (value - minimum_) / delta;
        if (Utility::Mathematics::EpsilonEquals(filledPart, 0.0F))
        {
            return;
        }

        const Rectangle bounds = getActualBoundsProperty();
        if (getOrientationProperty() == Orientation::Horizontal)
        {
            const int width = Utility::Mathematics::TruncateToInt(
                filledPart * static_cast<float>(bounds.Width));
            filler->Draw(context,
                Rectangle(bounds.X, bounds.Y, width, bounds.Height), Color::White);
        }
        else
        {
            const int height = Utility::Mathematics::TruncateToInt(
                filledPart * static_cast<float>(bounds.Height));
            filler->Draw(context,
                Rectangle(bounds.X, bounds.Y, bounds.Width, height), Color::White);
        }
    }

    void ProgressBar::CopyFrom(const Widget& source)
    {
        Widget::CopyFrom(source);
        const auto* const progressBar = dynamic_cast<const ProgressBar*>(&source);
        if (progressBar == nullptr)
        {
            throw std::invalid_argument("ProgressBar copy source must be a ProgressBar.");
        }
        setMinimumProperty(progressBar->minimum_);
        setMaximumProperty(progressBar->maximum_);
        setValueProperty(progressBar->value_);
        setFillerProperty(progressBar->filler_);
    }
}
