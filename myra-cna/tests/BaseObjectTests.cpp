// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/BaseObject.hpp"

#include <gtest/gtest.h>

#include <any>
#include <optional>
#include <string>
#include <utility>

namespace
{
    TEST(BaseObjectTests, IdPreservesNullEmptyAndValueChanges)
    {
        Myra::MML::BaseObject object;
        int notifications = 0;
        void* sender = nullptr;
        Myra::Graphics2D::UI::InputEventType eventType = Myra::Graphics2D::UI::InputEventType::None;

        object.IdChanged += [&](void* receivedSender, Myra::Events::MyraEventArgs& arguments) {
            ++notifications;
            sender = receivedSender;
            eventType = arguments.getEventTypeProperty();
        };

        EXPECT_FALSE(object.getIdProperty().has_value());
        object.setIdProperty(std::optional<std::string>());
        EXPECT_EQ(notifications, 0);

        object.setIdProperty(std::string());
        ASSERT_TRUE(object.getIdProperty().has_value());
        EXPECT_EQ(*object.getIdProperty(), "");
        EXPECT_EQ(notifications, 1);
        EXPECT_EQ(sender, &object);
        EXPECT_EQ(eventType, Myra::Graphics2D::UI::InputEventType::ValueChanged);

        object.setIdProperty(std::string());
        EXPECT_EQ(notifications, 1);
        object.setIdProperty(std::string("dialog"));
        ASSERT_TRUE(object.getIdProperty().has_value());
        EXPECT_EQ(*object.getIdProperty(), "dialog");
        EXPECT_EQ(notifications, 2);

        object.setIdProperty(std::optional<std::string>());
        EXPECT_FALSE(object.getIdProperty().has_value());
        EXPECT_EQ(notifications, 3);
    }

    TEST(BaseObjectTests, ExposesFaithfulAttachedPropertyAndUserDataStores)
    {
        Myra::MML::BaseObject object;

        object.AttachedPropertiesValues.emplace(7, 42);
        ASSERT_TRUE(object.AttachedPropertiesValues.contains(7));
        EXPECT_EQ(std::any_cast<int>(object.AttachedPropertiesValues.at(7)), 42);

        object.getUserDataProperty().emplace("_custom", "value");
        const auto& userData = std::as_const(object).getUserDataProperty();
        ASSERT_TRUE(userData.contains("_custom"));
        EXPECT_EQ(userData.at("_custom"), "value");
    }
}
