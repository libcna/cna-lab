// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Events/MyraEventArgs.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MyraEnvironment.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
    using Myra::Events::EventHandlingStrategy;
    using Myra::Events::MyraEventArgs;
    using Myra::Graphics2D::UI::IInputEventsProcessor;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::InputEventsManager;

    class RecordingProcessor final : public IInputEventsProcessor
    {
    public:
        explicit RecordingProcessor(std::vector<int>& calls, const int identifier)
            : calls_(calls), identifier_(identifier)
        {
        }

        void ProcessEvent(const InputEventType) override
        {
            calls_.push_back(identifier_);
        }

    private:
        std::vector<int>& calls_;
        int identifier_;
    };

    class StopPropagationProcessor final : public IInputEventsProcessor
    {
    public:
        explicit StopPropagationProcessor(std::vector<int>& calls)
            : calls_(calls)
        {
        }

        void ProcessEvent(const InputEventType eventType) override
        {
            calls_.push_back(1);
            MyraEventArgs(eventType).StopPropagation();
        }

    private:
        std::vector<int>& calls_;
    };

    TEST(InputEventsManagerTests, CapturingProcessesEventsInQueueOrder)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
        std::vector<int> calls;
        auto first = std::make_shared<RecordingProcessor>(calls, 1);
        auto second = std::make_shared<RecordingProcessor>(calls, 2);
        auto third = std::make_shared<RecordingProcessor>(calls, 3);

        InputEventsManager::Queue(first, InputEventType::MouseEntered);
        InputEventsManager::Queue(second, InputEventType::MouseMoved);
        InputEventsManager::Queue(third, InputEventType::MouseLeft);
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(calls, (std::vector<int>{1, 2, 3}));
    }

    TEST(InputEventsManagerTests, BubblingProcessesEventsInStackOrder)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventBubbling);
        std::vector<int> calls;
        auto first = std::make_shared<RecordingProcessor>(calls, 1);
        auto second = std::make_shared<RecordingProcessor>(calls, 2);
        auto third = std::make_shared<RecordingProcessor>(calls, 3);

        InputEventsManager::Queue(first, InputEventType::MouseEntered);
        InputEventsManager::Queue(second, InputEventType::MouseMoved);
        InputEventsManager::Queue(third, InputEventType::MouseLeft);
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(calls, (std::vector<int>{3, 2, 1}));
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
    }

    TEST(InputEventsManagerTests, StopPropagationRemovesPendingEventsOfTheSameType)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
        std::vector<int> calls;
        auto stopping = std::make_shared<StopPropagationProcessor>(calls);
        auto removed = std::make_shared<RecordingProcessor>(calls, 2);
        auto retained = std::make_shared<RecordingProcessor>(calls, 3);

        InputEventsManager::Queue(stopping, InputEventType::MouseMoved);
        InputEventsManager::Queue(removed, InputEventType::MouseMoved);
        InputEventsManager::Queue(retained, InputEventType::KeyDown);
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(calls, (std::vector<int>{1, 3}));
    }

    TEST(InputEventsManagerTests, BubblingStopPropagationPreservesTheUpstreamRebuildOrder)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventBubbling);
        std::vector<int> calls;
        auto oldest = std::make_shared<RecordingProcessor>(calls, 1);
        auto removed = std::make_shared<RecordingProcessor>(calls, 2);
        auto newest = std::make_shared<RecordingProcessor>(calls, 3);
        auto stopping = std::make_shared<StopPropagationProcessor>(calls);

        InputEventsManager::Queue(oldest, InputEventType::KeyDown);
        InputEventsManager::Queue(removed, InputEventType::MouseMoved);
        InputEventsManager::Queue(newest, InputEventType::KeyUp);
        InputEventsManager::Queue(stopping, InputEventType::MouseMoved);
        InputEventsManager::ProcessEvents();

        // Upstream enumerates the stack top-to-bottom and pushes the filtered
        // sequence into a fresh stack, reversing the surviving events.
        EXPECT_EQ(calls, (std::vector<int>{1, 1, 3}));
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
    }

    TEST(InputEventsManagerTests, RetainsProcessorUntilCapturingEventIsDispatched)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
        std::vector<int> calls;
        std::weak_ptr<RecordingProcessor> observer;

        {
            auto processor = std::make_shared<RecordingProcessor>(calls, 1);
            observer = processor;
            InputEventsManager::Queue(processor, InputEventType::MouseMoved);
        }

        ASSERT_FALSE(observer.expired());
        InputEventsManager::ProcessEvents();

        EXPECT_TRUE(observer.expired());
        EXPECT_EQ(calls, (std::vector<int>{1}));
    }

    TEST(InputEventsManagerTests, StopPropagationReleasesFilteredProcessor)
    {
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventBubbling);
        std::vector<int> calls;
        std::weak_ptr<RecordingProcessor> observer;

        {
            auto processor = std::make_shared<RecordingProcessor>(calls, 1);
            observer = processor;
            InputEventsManager::Queue(processor, InputEventType::MouseMoved);
        }

        ASSERT_FALSE(observer.expired());
        InputEventsManager::StopPropagation(InputEventType::MouseMoved);
        EXPECT_TRUE(observer.expired());

        InputEventsManager::ProcessEvents();
        EXPECT_TRUE(calls.empty());
        Myra::MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
    }

    TEST(InputEventsManagerTests, RejectsNullProcessor)
    {
        EXPECT_THROW(InputEventsManager::Queue({}, InputEventType::MouseMoved), std::invalid_argument);
    }
}
