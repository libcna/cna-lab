// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"

#include <gtest/gtest.h>

namespace
{
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Proportion;
    using Myra::Graphics2D::UI::ProportionType;

    TEST(ProportionTests, ExposesTheUpstreamStaticDefaults)
    {
        EXPECT_EQ(Proportion::Auto.getTypeProperty(), ProportionType::Auto);
        EXPECT_EQ(Proportion::Fill.getTypeProperty(), ProportionType::Fill);
        EXPECT_EQ(Proportion::GridDefault.getTypeProperty(), ProportionType::Part);
        EXPECT_FLOAT_EQ(Proportion::GridDefault.getValueProperty(), 1.0F);
        EXPECT_EQ(Proportion::StackPanelDefault.getTypeProperty(), ProportionType::Auto);
    }

    TEST(ProportionTests, ChangesOnlyOutsideTheUpstreamEpsilonAndRaisesEvents)
    {
        Proportion proportion;
        int changedCount = 0;
        void* sender = nullptr;
        InputEventType eventType = InputEventType::None;
        proportion.Changed += [&](void* receivedSender, Myra::Events::MyraEventArgs& arguments) {
            ++changedCount;
            sender = receivedSender;
            eventType = arguments.getEventTypeProperty();
        };

        proportion.setTypeProperty(ProportionType::Auto);
        proportion.setValueProperty(1.0F + 0.0000001F);
        EXPECT_EQ(changedCount, 0);

        proportion.setTypeProperty(ProportionType::Pixels);
        proportion.setValueProperty(12.5F);
        EXPECT_EQ(changedCount, 2);
        EXPECT_EQ(sender, &proportion);
        EXPECT_EQ(eventType, InputEventType::ProportionChanged);
    }

    TEST(ProportionTests, UsesTheUpstreamDisplayFormats)
    {
        EXPECT_EQ(Proportion(ProportionType::Auto).ToString(), "Auto");
        EXPECT_EQ(Proportion(ProportionType::Fill).ToString(), "Fill");
        EXPECT_EQ(Proportion(ProportionType::Part, 2.5F).ToString(), "Part: 2.50");
        EXPECT_EQ(Proportion(ProportionType::Pixels, 3.8F).ToString(), "Pixels: 3");
    }
}
