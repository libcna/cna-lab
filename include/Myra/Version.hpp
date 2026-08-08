// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <string_view>

namespace Myra
{
    /**
     * @brief Version information for the Myra-CNA library.
     *
     * This bootstrap API is original Myra-CNA code. It deliberately does not
     * claim an upstream Myra API equivalent.
     */
    class Version final
    {
    public:
        Version() = delete;

        static constexpr int Major = 0;
        static constexpr int Minor = 1;
        static constexpr int Patch = 0;

        [[nodiscard]] static std::string_view GetString() noexcept;
    };
}
