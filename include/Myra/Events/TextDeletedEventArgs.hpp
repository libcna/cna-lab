// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/TextDeletedEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>
#include <utility>

#include "Myra/Events/MyraEventArgs.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Myra::Events
{
    /** @brief Provides data for text deletion events. */
    class TextDeletedEventArgs : public MyraEventArgs
    {
    public:
        TextDeletedEventArgs(const SharpRuntime::intcs startPosition, std::string value)
            : MyraEventArgs(Graphics2D::UI::InputEventType::TextDeleted),
              startPosition_(startPosition),
              value_(std::move(value))
        {
        }

        [[nodiscard]] constexpr SharpRuntime::intcs getStartPositionProperty() const noexcept
        {
            return startPosition_;
        }

        [[nodiscard]] const std::string& getValueProperty() const noexcept
        {
            return value_;
        }

    private:
        SharpRuntime::intcs startPosition_;
        std::string value_;
    };
}
