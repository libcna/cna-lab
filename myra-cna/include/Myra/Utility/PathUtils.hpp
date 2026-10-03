// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/PathUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>

namespace Myra::Utility
{
    /** @brief Filesystem path helpers shared by project and property-grid serialization. */
    class PathUtils final
    {
    public:
        /**
         * @brief Returns a lexical relative path when both inputs are absolute and compatible.
         *
         * Like upstream, conversion failures preserve the original path.
         */
        [[nodiscard]] static std::string TryToMakePathRelativeTo(
            const std::string& path, const std::string& pathRelativeTo) noexcept;
    };
}
