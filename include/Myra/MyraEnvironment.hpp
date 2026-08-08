// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Events/EventHandlingStrategy.hpp"

namespace Myra
{
    /**
     * @brief Provides global configuration for Myra UI.
     *
     * This initial port exposes the event-propagation property required by the
     * input-event runtime. Additional upstream environment properties are
     * added here as their dependencies are ported.
     */
    class MyraEnvironment final
    {
    public:
        MyraEnvironment() = delete;

        [[nodiscard]] static Events::EventHandlingStrategy getEventHandlingModelProperty() noexcept;
        static void setEventHandlingModelProperty(Events::EventHandlingStrategy value) noexcept;

    private:
        static Events::EventHandlingStrategy eventHandlingModel_;
    };
}
