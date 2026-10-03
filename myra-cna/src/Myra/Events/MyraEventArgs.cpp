// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Events/MyraEventArgs.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Events/MyraEventArgs.hpp"

#include "Myra/Graphics2D/UI/InputEventsManager.hpp"

namespace Myra::Events
{
    const MyraEventArgs MyraEventArgs::Empty(Graphics2D::UI::InputEventType::None);

    void MyraEventArgs::StopPropagation() const
    {
        Graphics2D::UI::InputEventsManager::StopPropagation(getEventTypeProperty());
    }
}
