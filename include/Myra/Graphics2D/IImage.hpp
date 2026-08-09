// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/IImage.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Myra/Graphics2D/IBrush.hpp"

namespace Myra::Graphics2D
{
    /** @brief Represents a brush with a defined pixel size. */
    class IImage : public IBrush
    {
    public:
        ~IImage() override = default;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Point getSizeProperty() const = 0;
    };
}
