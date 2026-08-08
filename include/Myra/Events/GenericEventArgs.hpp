// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/GenericEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <utility>

#include "Myra/Events/MyraEventArgs.hpp"

namespace Myra::Events
{
    /** @brief Generic event arguments for passing arbitrary data with an event. */
    template<typename T>
    class GenericEventArgs final : public MyraEventArgs
    {
    public:
        GenericEventArgs(T value, const Graphics2D::UI::InputEventType eventType)
            : MyraEventArgs(eventType), data_(std::move(value))
        {
        }

        [[nodiscard]] const T& getDataProperty() const noexcept
        {
            return data_;
        }

    private:
        T data_;
    };
}
