// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/StringUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstddef>
#include <string>

namespace Myra::Utility
{
    /** @brief String helpers retaining Myra's empty-string behavior. */
    class StringUtils final
    {
    public:
        StringUtils() = delete;

        [[nodiscard]] static std::size_t Length(const std::string& value) noexcept
        {
            return value.size();
        }
    };
}
