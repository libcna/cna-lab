// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Proportion.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"

#include <iomanip>
#include <sstream>

#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    const Proportion Proportion::Auto(ProportionType::Auto);
    const Proportion Proportion::Fill(ProportionType::Fill);
    const Proportion Proportion::GridDefault(ProportionType::Part, 1.0F);
    const Proportion Proportion::StackPanelDefault(ProportionType::Auto);

    Proportion::Proportion(const ProportionType type) noexcept : type_(type) {}

    Proportion::Proportion(const ProportionType type, const float value) noexcept : type_(type), value_(value) {}

    ProportionType Proportion::getTypeProperty() const noexcept { return type_; }

    void Proportion::setTypeProperty(const ProportionType value)
    {
        if (value == type_)
        {
            return;
        }
        type_ = value;
        FireChanged();
    }

    float Proportion::getValueProperty() const noexcept { return value_; }

    void Proportion::setValueProperty(const float value)
    {
        if (Utility::Mathematics::EpsilonEquals(value, value_))
        {
            return;
        }
        value_ = value;
        FireChanged();
    }

    std::string Proportion::ToString() const
    {
        switch (type_)
        {
        case ProportionType::Auto: return "Auto";
        case ProportionType::Fill: return "Fill";
        case ProportionType::Part:
        {
            std::ostringstream stream;
            stream << "Part: " << std::fixed << std::setprecision(2) << value_;
            return stream.str();
        }
        case ProportionType::Pixels:
            return "Pixels: " + std::to_string(static_cast<int>(value_));
        }
        return "";
    }

    void Proportion::FireChanged()
    {
        Utility::EventsExtensions::Invoke(Changed, this, InputEventType::ProportionChanged);
    }
}
