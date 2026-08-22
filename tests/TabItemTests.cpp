// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/TabItem.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>

#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::TabItem;
    using Myra::Graphics2D::UI::Widget;

    TEST(TabItemTests, RaisesChangedForTextContentAndIdentifierMutations)
    {
        TabItem item;
        int changed = 0;
        item.Changed += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };

        item.setTextProperty(std::string("First"));
        item.setTextProperty(std::string("First"));
        const auto content = std::make_shared<Widget>();
        item.setContentProperty(content);
        item.setContentProperty(content);
        item.setIdProperty(std::string("tab-1"));
        EXPECT_EQ(changed, 3);
        EXPECT_EQ(item.getTextProperty(), std::optional<std::string>("First"));
        EXPECT_EQ(item.getContentProperty(), content);
        EXPECT_EQ(item.ToString(), "First (#tab-1)");

        item.setTextProperty(std::nullopt);
        EXPECT_EQ(item.ToString(), "(#tab-1)");
        item.setIdProperty(std::nullopt);
        EXPECT_EQ(item.ToString(), "");
    }

    TEST(TabItemTests, KeepsSelectionAndNonVisualMetadataInTheDataCore)
    {
        TabItem item;
        int selectionChanges = 0;
        item.SelectedChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectionChanged);
            ++selectionChanges;
        };

        item.setIsSelectedProperty(true);
        item.setIsSelectedProperty(true);
        item.setIsSelectedProperty(false);
        EXPECT_FALSE(item.getIsSelectedProperty());
        EXPECT_EQ(selectionChanges, 1);

        item.setTagProperty(std::string("designer value"));
        item.setImageTextSpacingProperty(7);
        item.setHeightProperty(24);
        EXPECT_EQ(std::any_cast<const std::string &>(item.getTagProperty()), "designer value");
        EXPECT_EQ(item.getImageTextSpacingProperty(), 7);
        EXPECT_EQ(item.getHeightProperty(), std::optional<int>(24));
    }

    TEST(TabItemTests, ClonesPortedStateButPreservesTheSelectedUpstreamFreshIdentity)
    {
        const auto content = std::make_shared<Widget>();
        TabItem item(std::string("Settings"), content);
        item.setIdProperty(std::string("settings-tab"));
        item.setTagProperty(42);
        item.setImageTextSpacingProperty(3);
        item.setHeightProperty(32);
        item.setIsSelectedProperty(true);

        const auto clone = item.Clone();
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getTextProperty(), std::optional<std::string>("Settings"));
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(std::any_cast<int>(clone->getTagProperty()), 42);
        EXPECT_EQ(clone->getImageTextSpacingProperty(), 3);
        EXPECT_EQ(clone->getHeightProperty(), std::optional<int>(32));
        EXPECT_FALSE(clone->getIdProperty().has_value());
        EXPECT_FALSE(clone->getIsSelectedProperty());
        EXPECT_EQ(clone->ToString(), "Settings ");
    }
} // namespace
