// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

#include <gtest/gtest.h>

#include <string>

namespace
{
    TEST(EventsExtensionsTests, InvokesNonGenericEventWithTheSpecifiedSenderAndType)
    {
        Myra::Events::MyraEventHandler eventHandler;
        int senderValue = 0;
        void* sender = nullptr;
        Myra::Graphics2D::UI::InputEventType eventType = Myra::Graphics2D::UI::InputEventType::None;

        eventHandler += [&](void* receivedSender, Myra::Events::MyraEventArgs& arguments) {
            sender = receivedSender;
            eventType = arguments.getEventTypeProperty();
        };

        Myra::Utility::EventsExtensions::Invoke(
            eventHandler, &senderValue, Myra::Graphics2D::UI::InputEventType::VisibleChanged);

        EXPECT_EQ(sender, &senderValue);
        EXPECT_EQ(eventType, Myra::Graphics2D::UI::InputEventType::VisibleChanged);
    }

    TEST(EventsExtensionsTests, InvokesGenericEventWithPayloadAndOptionalNullSender)
    {
        Myra::Events::MyraEventHandlerT<Myra::Events::GenericEventArgs<std::string>> eventHandler;
        void* sender = reinterpret_cast<void*>(1);
        std::string payload;
        Myra::Graphics2D::UI::InputEventType eventType = Myra::Graphics2D::UI::InputEventType::None;

        eventHandler += [&](void* receivedSender, Myra::Events::GenericEventArgs<std::string>& arguments) {
            sender = receivedSender;
            payload = arguments.getDataProperty();
            eventType = arguments.getEventTypeProperty();
        };

        Myra::Utility::EventsExtensions::Invoke(
            eventHandler, std::string("payload"), Myra::Graphics2D::UI::InputEventType::SelectedIndexChanged);

        EXPECT_EQ(sender, nullptr);
        EXPECT_EQ(payload, "payload");
        EXPECT_EQ(eventType, Myra::Graphics2D::UI::InputEventType::SelectedIndexChanged);
    }
}
