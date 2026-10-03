// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/MyraEventHandler.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <concepts>

#include "Myra/Events/MyraEventArgs.hpp"
#include "System/MulticastAction.hpp"

namespace Myra::Events
{
    /**
     * @brief Multicast counterpart of the non-generic Myra event delegate.
     *
     * `void*` preserves C#'s unconstrained `object sender`; C++ widgets do
     * not need to inherit sharp-runtime's abstract `System::Object`.
     */
    using MyraEventHandler = System::MulticastAction<void*, MyraEventArgs&>;

    /** @brief Multicast counterpart of MyraEventHandler<T>. */
    template<std::derived_from<MyraEventArgs> T>
    using MyraEventHandlerT = System::MulticastAction<void*, T&>;
}
