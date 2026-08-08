// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Enums.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

namespace Myra::Graphics2D::UI
{
    enum class HorizontalAlignment { Left, Center, Right, Stretch };
    enum class VerticalAlignment { Top, Center, Bottom, Stretch };
    enum class MouseButtons { Left, Middle, Right };
    enum class Orientation { Horizontal, Vertical };
    enum class MouseCursorType {
        Arrow, IBeam, Wait, Crosshair, WaitArrow, SizeNWSE, SizeNESW,
        SizeWE, SizeNS, SizeAll, No, Hand
    };
}
