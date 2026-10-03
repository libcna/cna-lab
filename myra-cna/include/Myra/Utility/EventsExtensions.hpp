// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/EventsExtensions.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <utility>

#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"

namespace Myra::Utility::EventsExtensions
{
    /** @brief Invokes a non-generic Myra event without a sender. */
    inline void Invoke(Events::MyraEventHandler& eventHandler,
        const Graphics2D::UI::InputEventType eventType)
    {
        Events::MyraEventArgs arguments(eventType);
        eventHandler.Invoke(nullptr, arguments);
    }

    /** @brief Invokes a non-generic Myra event with its source object. */
    inline void Invoke(Events::MyraEventHandler& eventHandler, void* sender,
        const Graphics2D::UI::InputEventType eventType)
    {
        Events::MyraEventArgs arguments(eventType);
        eventHandler.Invoke(sender, arguments);
    }

    /** @brief Invokes a generic Myra event without a sender. */
    template<typename T>
    void Invoke(Events::MyraEventHandlerT<Events::GenericEventArgs<T>>& eventHandler, T data,
        const Graphics2D::UI::InputEventType eventType)
    {
        Events::GenericEventArgs<T> arguments(std::move(data), eventType);
        eventHandler.Invoke(nullptr, arguments);
    }

    /** @brief Invokes a generic Myra event with its source object. */
    template<typename T>
    void Invoke(Events::MyraEventHandlerT<Events::GenericEventArgs<T>>& eventHandler, void* sender, T data,
        const Graphics2D::UI::InputEventType eventType)
    {
        Events::GenericEventArgs<T> arguments(std::move(data), eventType);
        eventHandler.Invoke(sender, arguments);
    }
}
