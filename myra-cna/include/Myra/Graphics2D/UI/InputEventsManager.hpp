// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/InputEventsManager.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "Myra/Graphics2D/UI/InputEventType.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Processes a queued Myra input event. */
    class IInputEventsProcessor
    {
    public:
        virtual ~IInputEventsProcessor() = default;
        virtual void ProcessEvent(InputEventType eventType) = 0;
    };

    /** @brief Queues and propagates input events using Myra's capture/bubble modes. */
    class InputEventsManager final
    {
    public:
        InputEventsManager() = delete;

        /**
         * @brief Queues an event while retaining its processor through dispatch.
         * @throws std::invalid_argument if @p processor is null.
         */
        static void Queue(
            std::shared_ptr<IInputEventsProcessor> processor, InputEventType eventType);
        static void ProcessEvents();
        static void StopPropagation(InputEventType eventType);

    private:
        struct InputEvent
        {
            std::shared_ptr<IInputEventsProcessor> Processor;
            InputEventType Type;
        };

        static std::deque<InputEvent> eventsQueue_;
        static std::vector<InputEvent> eventsStack_;
    };
}
