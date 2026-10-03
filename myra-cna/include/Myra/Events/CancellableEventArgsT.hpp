// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/CancellableEventArgs{T}.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <utility>

#include "Myra/Events/MyraEventArgs.hpp"

namespace Myra::Events
{
    /** @brief Provides data for cancellable events with associated data. */
    template<typename T>
    class CancellableEventArgsT : public MyraEventArgs
    {
    public:
        CancellableEventArgsT(T data, const Graphics2D::UI::InputEventType inputEventType)
            : MyraEventArgs(inputEventType), data_(std::move(data))
        {
        }

        [[nodiscard]] const T& getDataProperty() const noexcept
        {
            return data_;
        }

        /** Gets or sets whether the event should be cancelled. */
        bool Cancel = false;

    private:
        T data_;
    };
}
