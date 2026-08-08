// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/ITransformable.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Maps points between a widget's local and global coordinate spaces. */
    class ITransformable
    {
    public:
        virtual ~ITransformable() = default;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Vector2 ToLocal(
            Microsoft::Xna::Framework::Vector2 source) const = 0;
        [[nodiscard]] virtual Microsoft::Xna::Framework::Vector2 ToGlobal(
            Microsoft::Xna::Framework::Vector2 position) const = 0;
    };
}
