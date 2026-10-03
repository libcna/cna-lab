// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/InputContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

namespace Myra::Graphics2D::UI
{
    class Widget;

    /** @brief Carries per-frame mouse and touch state through the widget tree. */
    class InputContext
    {
    public:
        bool MouseOrTouchHandled = false;
        Widget* MouseWheelWidget = nullptr;
        bool ParentContainsMouse = true;
        bool ParentContainsTouch = true;

        /** @brief Restores the exact upstream default state. */
        void Reset() noexcept;
    };
}
