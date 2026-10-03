// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.Input.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Mouse position, button state, and cumulative wheel value for one input snapshot. */
    struct MouseInfo
    {
        Microsoft::Xna::Framework::Point Position{};
        bool IsLeftButtonDown = false;
        bool IsMiddleButtonDown = false;
        bool IsRightButtonDown = false;
        float Wheel = 0.0F;
    };
} // namespace Myra::Graphics2D::UI
