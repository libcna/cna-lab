// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra and MonoGame.Extended, MIT Licenses,
// Copyright (c) 2017-2020 The Myra Team and Copyright (c) 2015 Dylan Wilson.
// Ported from: src/Myra/Utility/InputExtension.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md, THIRD_PARTY_NOTICES.md, and UPSTREAM_MANIFEST.md.
#pragma once

#include <optional>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"

namespace Myra::Utility
{
    /** @brief Converts CNA key values to the selected upstream US-keyboard characters. */
    class InputExtension final
    {
    public:
        [[nodiscard]] static std::optional<char> ToChar(
            Microsoft::Xna::Framework::Input::Keys key, bool isShiftDown) noexcept;
    };
}
