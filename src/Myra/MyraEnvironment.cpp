// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

namespace Myra
{
    Events::EventHandlingStrategy MyraEnvironment::eventHandlingModel_ =
        Events::EventHandlingStrategy::EventCapturing;

    Events::EventHandlingStrategy MyraEnvironment::getEventHandlingModelProperty() noexcept
    {
        return eventHandlingModel_;
    }

    void MyraEnvironment::setEventHandlingModelProperty(const Events::EventHandlingStrategy value) noexcept
    {
        eventHandlingModel_ = value;
    }
}
