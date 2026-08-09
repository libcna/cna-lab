// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <gtest/gtest.h>

namespace
{
    using Myra::Events::EventHandlingStrategy;
    using Myra::Graphics2D::UI::MouseCursorType;
    using Myra::MyraEnvironment;

    class MyraEnvironmentSettingsTests : public testing::Test
    {
    protected:
        void TearDown() override
        {
            MyraEnvironment::setEventHandlingModelProperty(
                EventHandlingStrategy::EventCapturing);
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDrawTextGlyphsFramesProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::setSetMouseCursorFromWidgetProperty(true);
            MyraEnvironment::setDefaultMouseCursorTypeProperty(MouseCursorType::Arrow);
        }
    };

    TEST_F(MyraEnvironmentSettingsTests, UsesTheUpstreamConfigurationDefaults)
    {
        EXPECT_EQ(
            MyraEnvironment::getEventHandlingModelProperty(),
            EventHandlingStrategy::EventCapturing);
        EXPECT_FALSE(MyraEnvironment::getDrawWidgetsFramesProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawMouseHoveredWidgetFrameProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawTextGlyphsFramesProperty());
        EXPECT_FALSE(MyraEnvironment::getDisableClippingProperty());
        EXPECT_TRUE(MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::Arrow);
        EXPECT_EQ(MyraEnvironment::getDefaultMouseCursorTypeProperty(), MouseCursorType::Arrow);
    }

    TEST_F(MyraEnvironmentSettingsTests, StoresEachConfigurationSettingIndependently)
    {
        MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventBubbling);
        MyraEnvironment::setDrawWidgetsFramesProperty(true);
        MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(true);
        MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(true);
        MyraEnvironment::setDrawTextGlyphsFramesProperty(true);
        MyraEnvironment::setDisableClippingProperty(true);
        MyraEnvironment::setSetMouseCursorFromWidgetProperty(false);
        MyraEnvironment::setDefaultMouseCursorTypeProperty(MouseCursorType::Hand);

        EXPECT_EQ(
            MyraEnvironment::getEventHandlingModelProperty(),
            EventHandlingStrategy::EventBubbling);
        EXPECT_TRUE(MyraEnvironment::getDrawWidgetsFramesProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawMouseHoveredWidgetFrameProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawTextGlyphsFramesProperty());
        EXPECT_TRUE(MyraEnvironment::getDisableClippingProperty());
        EXPECT_FALSE(MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(MyraEnvironment::getDefaultMouseCursorTypeProperty(), MouseCursorType::Hand);
    }
}
