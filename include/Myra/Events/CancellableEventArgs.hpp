// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/CancellableEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Events/MyraEventArgs.hpp"

namespace Myra::Events
{
    /** @brief Provides data for events that can be cancelled. */
    class CancellableEventArgs : public MyraEventArgs
    {
    public:
        explicit CancellableEventArgs(const Graphics2D::UI::InputEventType inputEventType) noexcept
            : MyraEventArgs(inputEventType)
        {
        }

        /** Gets or sets whether the event should be cancelled. */
        bool Cancel = false;
    };
}
