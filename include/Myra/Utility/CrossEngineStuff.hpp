// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/CrossEngineStuff.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

namespace Myra::Utility
{
    /** @brief CNA-specific helpers shared by rendering and platform integration. */
    class CrossEngineStuff final
    {
    public:
        CrossEngineStuff() = delete;

        /** @brief Multiplies every color channel by the supplied scalar. */
        [[nodiscard]] static Microsoft::Xna::Framework::Color MultiplyColor(
            const Microsoft::Xna::Framework::Color& color,
            float value)
        {
            return Microsoft::Xna::Framework::Color::Multiply(color, value);
        }
    };
}
