// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/InputEventsManager.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Events/EventHandlingStrategy.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace Myra::Graphics2D::UI
{
    std::deque<InputEventsManager::InputEvent> InputEventsManager::eventsQueue_;
    std::vector<InputEventsManager::InputEvent> InputEventsManager::eventsStack_;

    void InputEventsManager::Queue(
        std::shared_ptr<IInputEventsProcessor> processor, const InputEventType eventType)
    {
        if (!processor)
        {
            throw std::invalid_argument("InputEventsManager cannot queue a null processor");
        }

        InputEvent event{std::move(processor), eventType};
        switch (MyraEnvironment::getEventHandlingModelProperty())
        {
        case Events::EventHandlingStrategy::EventCapturing:
            eventsQueue_.push_back(std::move(event));
            break;
        case Events::EventHandlingStrategy::EventBubbling:
            eventsStack_.push_back(std::move(event));
            break;
        }
    }

    void InputEventsManager::ProcessEvents()
    {
        switch (MyraEnvironment::getEventHandlingModelProperty())
        {
        case Events::EventHandlingStrategy::EventCapturing:
            while (!eventsQueue_.empty())
            {
                InputEvent event = std::move(eventsQueue_.front());
                eventsQueue_.pop_front();
                event.Processor->ProcessEvent(event.Type);
            }
            break;
        case Events::EventHandlingStrategy::EventBubbling:
            while (!eventsStack_.empty())
            {
                InputEvent event = std::move(eventsStack_.back());
                eventsStack_.pop_back();
                event.Processor->ProcessEvent(event.Type);
            }
            break;
        }
    }

    void InputEventsManager::StopPropagation(const InputEventType eventType)
    {
        if (MyraEnvironment::getEventHandlingModelProperty() ==
            Events::EventHandlingStrategy::EventCapturing)
        {
            std::erase_if(eventsQueue_, [eventType](const InputEvent& event) {
                return event.Type == eventType;
            });
            return;
        }

        // Stack enumeration is top-to-bottom in .NET. Rebuilding with that
        // enumeration order intentionally preserves upstream's resulting order.
        std::vector<InputEvent> events;
        events.reserve(eventsStack_.size());
        for (auto event = eventsStack_.rbegin(); event != eventsStack_.rend(); ++event)
        {
            if (event->Type != eventType)
            {
                events.push_back(*event);
            }
        }

        eventsStack_.clear();
        for (const InputEvent& event : events)
        {
            eventsStack_.push_back(event);
        }
    }
}
