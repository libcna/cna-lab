// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/InputContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/InputContext.hpp"

namespace Myra::Graphics2D::UI
{
    void InputContext::Reset() noexcept
    {
        MouseOrTouchHandled = false;
        MouseWheelWidget = nullptr;
        ParentContainsMouse = true;
        ParentContainsTouch = true;
    }
}
