// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/PathUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/PathUtils.hpp"

#include <filesystem>

namespace Myra::Utility
{
    std::string PathUtils::TryToMakePathRelativeTo(
        const std::string& path, const std::string& pathRelativeTo) noexcept
    {
        try
        {
            const std::filesystem::path target(path);
            const std::filesystem::path base(pathRelativeTo);
            if (!target.is_absolute() || !base.is_absolute())
            {
                return path;
            }

            const std::filesystem::path relative =
                target.lexically_normal().lexically_relative(base.lexically_normal());
            if (relative.empty())
            {
                return path;
            }
            return relative.generic_string();
        }
        catch (...)
        {
            return path;
        }
    }
}
