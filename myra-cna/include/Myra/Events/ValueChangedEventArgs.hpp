// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/ValueChangedEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <utility>

#include "Myra/Events/MyraEventArgs.hpp"

namespace Myra::Events
{
    /** @brief Provides data for value changed events. */
    template<typename T>
    class ValueChangedEventArgs : public MyraEventArgs
    {
    public:
        ValueChangedEventArgs(T oldValue, T newValue)
            : MyraEventArgs(Graphics2D::UI::InputEventType::ValueChanged),
              oldValue_(std::move(oldValue)),
              newValue_(std::move(newValue))
        {
        }

        [[nodiscard]] const T& getOldValueProperty() const noexcept
        {
            return oldValue_;
        }

        [[nodiscard]] const T& getNewValueProperty() const noexcept
        {
            return newValue_;
        }

    private:
        T oldValue_;
        T newValue_;
    };
}
