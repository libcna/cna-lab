// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/IItemWithId.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <optional>
#include <string>

namespace Myra::MML
{
    /** @brief Represents an item that has an optional unique identifier. */
    class IItemWithId
    {
    public:
        virtual ~IItemWithId() = default;

        [[nodiscard]] virtual const std::optional<std::string>& getIdProperty() const noexcept = 0;
        virtual void setIdProperty(std::optional<std::string> value) = 0;
    };
}
