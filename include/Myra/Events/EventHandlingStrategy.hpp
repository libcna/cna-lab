// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/EventHandlingStrategy.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

namespace Myra::Events
{
    /**
     * @brief Specifies how input events propagate through the widget hierarchy.
     */
    enum class EventHandlingStrategy
    {
        /** Events are captured at the top level and propagate to child widgets. */
        EventCapturing,

        /** Events start at the widget level and bubble to parent widgets. */
        EventBubbling
    };
}
