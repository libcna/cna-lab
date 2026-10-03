// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/MyraEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/InputEventType.hpp"

namespace Myra::Events
{
    /** @brief Provides data for Myra UI events. */
    class MyraEventArgs
    {
    public:
        static const MyraEventArgs Empty;

        explicit MyraEventArgs(Graphics2D::UI::InputEventType inputEventType) noexcept
            : eventType_(inputEventType)
        {
        }

        virtual ~MyraEventArgs() = default;

        [[nodiscard]] constexpr Graphics2D::UI::InputEventType getEventTypeProperty() const noexcept
        {
            return eventType_;
        }

        /** Stops further queued events having this event's type. */
        void StopPropagation() const;

    private:
        Graphics2D::UI::InputEventType eventType_;
    };
}
