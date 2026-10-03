// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Attributes/RangeAttribute.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <optional>
#include <stdexcept>

namespace Myra::Attributes
{
    /** @brief Specifies the permitted numeric range of a registry property. */
    class RangeAttribute final
    {
    public:
        explicit RangeAttribute(float minimum) noexcept : minimum_(minimum) {}

        RangeAttribute(float minimum, float maximum) : minimum_(minimum), maximum_(maximum)
        {
            if (minimum > maximum)
            {
                throw std::invalid_argument("RangeAttribute minimum must not exceed maximum.");
            }
        }

        [[nodiscard]] const std::optional<float>& getMinimumProperty() const noexcept
        {
            return minimum_;
        }

        [[nodiscard]] const std::optional<float>& getMaximumProperty() const noexcept
        {
            return maximum_;
        }

    private:
        std::optional<float> minimum_;
        std::optional<float> maximum_;
    };
}
