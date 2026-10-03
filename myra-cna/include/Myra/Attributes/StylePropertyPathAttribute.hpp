// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Attributes/StylePropertyPathAttribute.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>
#include <utility>

namespace Myra::Attributes
{
    /** @brief Binds a registry property to a named property inside its style. */
    class StylePropertyPathAttribute final
    {
    public:
        explicit StylePropertyPathAttribute(std::string name) : name_(std::move(name)) {}

        [[nodiscard]] const std::string& getNameProperty() const noexcept
        {
            return name_;
        }

    private:
        std::string name_;
    };
}
