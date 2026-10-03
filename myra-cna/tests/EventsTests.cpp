// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Events/CancellableEventArgs.hpp"
#include "Myra/Events/CancellableEventArgsT.hpp"
#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Events/TextDeletedEventArgs.hpp"
#include "Myra/Events/ValueChangedEventArgs.hpp"
#include "Myra/Events/ValueChangingEventArgs.hpp"

#include <gtest/gtest.h>

#include <string>

namespace
{
    using Myra::Events::CancellableEventArgs;
    using Myra::Events::CancellableEventArgsT;
    using Myra::Events::GenericEventArgs;
    using Myra::Events::MyraEventHandlerT;
    using Myra::Events::TextDeletedEventArgs;
    using Myra::Events::ValueChangedEventArgs;
    using Myra::Events::ValueChangingEventArgs;
    using Myra::Graphics2D::UI::InputEventType;

    TEST(EventsTests, ArgumentsPreserveUpstreamValuesAndMutability)
    {
        CancellableEventArgs cancellable(InputEventType::Closing);
        EXPECT_EQ(cancellable.getEventTypeProperty(), InputEventType::Closing);
        EXPECT_FALSE(cancellable.Cancel);
        cancellable.Cancel = true;
        EXPECT_TRUE(cancellable.Cancel);

        CancellableEventArgsT<std::string> cancellableWithData("payload", InputEventType::MouseMoved);
        EXPECT_EQ(cancellableWithData.getDataProperty(), "payload");
        EXPECT_EQ(cancellableWithData.getEventTypeProperty(), InputEventType::MouseMoved);

        GenericEventArgs<int> generic(42, InputEventType::SelectedIndexChanged);
        EXPECT_EQ(generic.getDataProperty(), 42);

        const TextDeletedEventArgs textDeleted(7, "removed");
        EXPECT_EQ(textDeleted.getEventTypeProperty(), InputEventType::TextDeleted);
        EXPECT_EQ(textDeleted.getStartPositionProperty(), 7);
        EXPECT_EQ(textDeleted.getValueProperty(), "removed");

        const ValueChangedEventArgs<int> changed(1, 2);
        EXPECT_EQ(changed.getEventTypeProperty(), InputEventType::ValueChanged);
        EXPECT_EQ(changed.getOldValueProperty(), 1);
        EXPECT_EQ(changed.getNewValueProperty(), 2);

        ValueChangingEventArgs<int> changing(3, 4);
        changing.Cancel = true;
        changing.setNewValueProperty(5);
        EXPECT_TRUE(changing.Cancel);
        EXPECT_EQ(changing.getOldValueProperty(), 3);
        EXPECT_EQ(changing.getNewValueProperty(), 5);
    }

    TEST(EventsTests, GenericHandlerSupportsMutationAndTokenRemoval)
    {
        MyraEventHandlerT<ValueChangingEventArgs<int>> handlers;
        int sender = 0;
        void* receivedSender = nullptr;
        int invocationCount = 0;

        const auto token = handlers.Add([&](void* source, ValueChangingEventArgs<int>& args) {
            receivedSender = source;
            ++invocationCount;
            args.Cancel = true;
            args.setNewValueProperty(99);
        });

        ValueChangingEventArgs<int> args(10, 20);
        handlers.Invoke(&sender, args);
        EXPECT_EQ(receivedSender, &sender);
        EXPECT_EQ(invocationCount, 1);
        EXPECT_TRUE(args.Cancel);
        EXPECT_EQ(args.getNewValueProperty(), 99);

        EXPECT_TRUE(handlers.Remove(token));
        handlers.Invoke(&sender, args);
        EXPECT_EQ(invocationCount, 1);
    }
}
