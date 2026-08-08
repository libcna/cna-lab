// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/IHasColor.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

namespace Myra::MML
{
    /** @brief Represents an object that exposes a CNA color. */
    class IHasColor
    {
    public:
        virtual ~IHasColor() = default;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Color getColorProperty() const = 0;
    };
}
