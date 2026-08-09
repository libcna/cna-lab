// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/IBrush.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace Myra::Graphics2D
{
    class RenderContext;

    /** @brief Represents content that can fill a destination rectangle. */
    class IBrush
    {
    public:
        virtual ~IBrush() = default;

        /** @brief Draws this brush through a caller-owned render context. */
        virtual void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const = 0;
    };

    /** @brief C++ equivalents of the selected upstream IBrush extension methods. */
    namespace IBrushExtensions
    {
        inline void Draw(
            const IBrush& brush,
            RenderContext& context,
            const Microsoft::Xna::Framework::Rectangle destination)
        {
            brush.Draw(context, destination, Microsoft::Xna::Framework::Color::White);
        }
    }
}
