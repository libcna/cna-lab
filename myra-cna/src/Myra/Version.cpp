// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Version.hpp"

namespace Myra
{
    std::string_view Version::GetString() noexcept
    {
        return "0.1.0-dev";
    }
}
