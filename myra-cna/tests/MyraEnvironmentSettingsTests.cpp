// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <utility>

namespace
{
    using Myra::MyraEnvironment;
    using Myra::Events::EventHandlingStrategy;
    using Myra::Graphics2D::UI::MouseCursorType;

    class MyraEnvironmentSettingsTests : public testing::Test
    {
      protected:
        void SetUp() override
        {
            mouseInfoGetter_ = MyraEnvironment::getMouseInfoGetterProperty();
            downKeysGetter_ = MyraEnvironment::getDownKeysGetterProperty();
            tooltipCreator_ = MyraEnvironment::getTooltipCreatorProperty();
        }

        void TearDown() override
        {
            MyraEnvironment::setEventHandlingModelProperty(EventHandlingStrategy::EventCapturing);
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDrawTextGlyphsFramesProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::setSetMouseCursorFromWidgetProperty(true);
            MyraEnvironment::setDefaultMouseCursorTypeProperty(MouseCursorType::Arrow);
            MyraEnvironment::setDoubleClickIntervalInMsProperty(500);
            MyraEnvironment::setDoubleClickRadiusProperty(2);
            MyraEnvironment::setTooltipDelayInMsProperty(500);
            MyraEnvironment::setTooltipOffsetProperty({0, 20});
            MyraEnvironment::setTooltipCreatorProperty(std::move(tooltipCreator_));
            MyraEnvironment::setEnableModalDarkeningProperty(false);
            MyraEnvironment::setMouseInfoGetterProperty(std::move(mouseInfoGetter_));
            MyraEnvironment::setDownKeysGetterProperty(std::move(downKeysGetter_));
        }

      private:
        MyraEnvironment::MouseInfoGetter mouseInfoGetter_;
        MyraEnvironment::DownKeysGetter downKeysGetter_;
        MyraEnvironment::TooltipCreator tooltipCreator_;
    };

    TEST_F(MyraEnvironmentSettingsTests, UsesTheUpstreamConfigurationDefaults)
    {
        EXPECT_EQ(MyraEnvironment::getEventHandlingModelProperty(), EventHandlingStrategy::EventCapturing);
        EXPECT_FALSE(MyraEnvironment::getDrawWidgetsFramesProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawMouseHoveredWidgetFrameProperty());
        EXPECT_FALSE(MyraEnvironment::getDrawTextGlyphsFramesProperty());
        EXPECT_FALSE(MyraEnvironment::getDisableClippingProperty());
        EXPECT_TRUE(MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::Arrow);
        EXPECT_EQ(MyraEnvironment::getDefaultMouseCursorTypeProperty(), MouseCursorType::Arrow);
        EXPECT_EQ(MyraEnvironment::getDoubleClickIntervalInMsProperty(), 500);
        EXPECT_EQ(MyraEnvironment::getDoubleClickRadiusProperty(), 2);
        EXPECT_EQ(MyraEnvironment::getTooltipDelayInMsProperty(), 500);
        EXPECT_EQ(MyraEnvironment::getTooltipOffsetProperty(), Microsoft::Xna::Framework::Point(0, 20));
        EXPECT_FALSE(MyraEnvironment::getTooltipCreatorProperty());
        EXPECT_FALSE(MyraEnvironment::getEnableModalDarkeningProperty());
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
        MyraEnvironment::setDoubleClickIntervalInMsProperty(275);
        MyraEnvironment::setDoubleClickRadiusProperty(7);
        MyraEnvironment::setTooltipDelayInMsProperty(125);
        MyraEnvironment::setTooltipOffsetProperty({4, 9});
        MyraEnvironment::setTooltipCreatorProperty([](Myra::Graphics2D::UI::Widget &) { return nullptr; });
        MyraEnvironment::setEnableModalDarkeningProperty(true);

        EXPECT_EQ(MyraEnvironment::getEventHandlingModelProperty(), EventHandlingStrategy::EventBubbling);
        EXPECT_TRUE(MyraEnvironment::getDrawWidgetsFramesProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawMouseHoveredWidgetFrameProperty());
        EXPECT_TRUE(MyraEnvironment::getDrawTextGlyphsFramesProperty());
        EXPECT_TRUE(MyraEnvironment::getDisableClippingProperty());
        EXPECT_FALSE(MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(MyraEnvironment::getDefaultMouseCursorTypeProperty(), MouseCursorType::Hand);
        EXPECT_EQ(MyraEnvironment::getDoubleClickIntervalInMsProperty(), 275);
        EXPECT_EQ(MyraEnvironment::getDoubleClickRadiusProperty(), 7);
        EXPECT_EQ(MyraEnvironment::getTooltipDelayInMsProperty(), 125);
        EXPECT_EQ(MyraEnvironment::getTooltipOffsetProperty(), Microsoft::Xna::Framework::Point(4, 9));
        EXPECT_TRUE(MyraEnvironment::getTooltipCreatorProperty());
        EXPECT_TRUE(MyraEnvironment::getEnableModalDarkeningProperty());
    }

    TEST_F(MyraEnvironmentSettingsTests, StoresInjectableMouseAndKeyboardSnapshotProviders)
    {
        MyraEnvironment::setMouseInfoGetterProperty(
            [] { return Myra::Graphics2D::UI::MouseInfo{{3, 5}, true, false, true, 12.0F}; });
        MyraEnvironment::setDownKeysGetterProperty(
            [](MyraEnvironment::DownKeys &keys)
            {
                keys.fill(false);
                keys[17] = true;
            });

        const Myra::Graphics2D::UI::MouseInfo mouseInfo = MyraEnvironment::getMouseInfoGetterProperty()();
        EXPECT_EQ(mouseInfo.Position, Microsoft::Xna::Framework::Point(3, 5));
        EXPECT_TRUE(mouseInfo.IsLeftButtonDown);
        EXPECT_TRUE(mouseInfo.IsRightButtonDown);
        EXPECT_FLOAT_EQ(mouseInfo.Wheel, 12.0F);

        MyraEnvironment::DownKeys keys{};
        MyraEnvironment::getDownKeysGetterProperty()(keys);
        EXPECT_TRUE(keys[17]);

        MyraEnvironment::setMouseInfoGetterProperty({});
        MyraEnvironment::setDownKeysGetterProperty({});
        EXPECT_FALSE(MyraEnvironment::getMouseInfoGetterProperty());
        EXPECT_FALSE(MyraEnvironment::getDownKeysGetterProperty());
    }

#ifndef MYRA_CNA_HAS_CNA_TARGET
    TEST_F(MyraEnvironmentSettingsTests, DeferredCursorSetterValidatesAndStoresWithoutANativeBackend)
    {
        MyraEnvironment::setMouseCursorTypeProperty(MouseCursorType::Hand);
        EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::Hand);

        const auto invalid = static_cast<MouseCursorType>(1000);
        EXPECT_THROW(MyraEnvironment::setMouseCursorTypeProperty(invalid), std::invalid_argument);
        EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), invalid);

        MyraEnvironment::setMouseCursorTypeProperty(MouseCursorType::Arrow);
    }
#endif
} // namespace
