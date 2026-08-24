// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::MyraEnvironment;
    using Myra::Events::GenericEventArgs;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalMenu;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::MenuItem;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::Widget;

    class KeyProbeWidget final : public Widget
    {
      public:
        std::vector<Keys> keysDown;
        std::vector<Keys> keysUp;

        void OnKeyDown(const Keys key) override
        {
            keysDown.push_back(key);
            Widget::OnKeyDown(key);
        }

        void OnKeyUp(const Keys key) override
        {
            keysUp.push_back(key);
            Widget::OnKeyUp(key);
        }
    };

    class DesktopInputTests : public testing::Test
    {
      protected:
        void SetUp() override
        {
            oldMouseInfoGetter_ = MyraEnvironment::getMouseInfoGetterProperty();
            oldDownKeysGetter_ = MyraEnvironment::getDownKeysGetterProperty();
            MyraEnvironment::setEventHandlingModelProperty(Myra::Events::EventHandlingStrategy::EventCapturing);
        }

        void TearDown() override
        {
            InputEventsManager::ProcessEvents();
            MyraEnvironment::setMouseInfoGetterProperty(std::move(oldMouseInfoGetter_));
            MyraEnvironment::setDownKeysGetterProperty(std::move(oldDownKeysGetter_));
        }

        static void UseFixedBounds(Desktop &desktop)
        {
            desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 320, 200); });
        }

      private:
        MyraEnvironment::MouseInfoGetter oldMouseInfoGetter_;
        MyraEnvironment::DownKeysGetter oldDownKeysGetter_;
    };

    TEST_F(DesktopInputTests, MouseSnapshotsQueueTouchMovementAndWheelEventsInUpstreamOrder)
    {
        MouseInfo snapshot{{10, 20}, true, false, false, 120.0F};
        MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        MyraEnvironment::setDownKeysGetterProperty([](MyraEnvironment::DownKeys &keys) { keys.fill(false); });

        Desktop desktop;
        std::vector<InputEventType> events;
        std::vector<float> wheelDeltas;
        desktop.MouseMoved += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &desktop);
            events.push_back(arguments.getEventTypeProperty());
        };
        desktop.TouchDown +=
            [&](void *, Myra::Events::MyraEventArgs &arguments) { events.push_back(arguments.getEventTypeProperty()); };
        desktop.TouchMoved +=
            [&](void *, Myra::Events::MyraEventArgs &arguments) { events.push_back(arguments.getEventTypeProperty()); };
        desktop.TouchUp +=
            [&](void *, Myra::Events::MyraEventArgs &arguments) { events.push_back(arguments.getEventTypeProperty()); };
        desktop.MouseWheelChanged += [&](void *, GenericEventArgs<float> &arguments)
        {
            events.push_back(arguments.getEventTypeProperty());
            wheelDeltas.push_back(arguments.getDataProperty());
        };

        desktop.UpdateInput();
        EXPECT_EQ(desktop.getPreviousMousePositionProperty(), Point(0, 0));
        EXPECT_FALSE(desktop.getPreviousTouchPositionProperty().has_value());
        EXPECT_EQ(desktop.getMousePositionProperty(), Point(10, 20));
        EXPECT_EQ(desktop.getTouchPositionProperty(), Point(10, 20));
        EXPECT_TRUE(desktop.getIsTouchDownProperty());
        EXPECT_FLOAT_EQ(desktop.getMouseWheelDeltaProperty(), 120.0F);
        InputEventsManager::ProcessEvents();
        EXPECT_EQ(events, (std::vector<InputEventType>{InputEventType::MouseMoved, InputEventType::TouchDown,
                                                       InputEventType::MouseWheel}));
        EXPECT_EQ(wheelDeltas, (std::vector<float>{120.0F}));

        events.clear();
        snapshot = {{15, 25}, true, false, false, 240.0F};
        desktop.UpdateInput();
        EXPECT_EQ(desktop.getPreviousMousePositionProperty(), Point(10, 20));
        EXPECT_EQ(desktop.getPreviousTouchPositionProperty(), Point(10, 20));
        EXPECT_EQ(desktop.getTouchPositionProperty(), Point(15, 25));
        InputEventsManager::ProcessEvents();
        EXPECT_EQ(events, (std::vector<InputEventType>{InputEventType::MouseMoved, InputEventType::TouchMoved,
                                                       InputEventType::MouseWheel}));
        ASSERT_EQ(wheelDeltas.size(), 2U);
        EXPECT_FLOAT_EQ(wheelDeltas.back(), 120.0F);

        events.clear();
        snapshot = {{15, 25}, false, false, false, 240.0F};
        desktop.UpdateInput();
        EXPECT_FALSE(desktop.getTouchPositionProperty().has_value());
        EXPECT_FLOAT_EQ(desktop.getMouseWheelDeltaProperty(), 0.0F);
        InputEventsManager::ProcessEvents();
        EXPECT_EQ(events, (std::vector<InputEventType>{InputEventType::TouchUp}));
    }

    TEST_F(DesktopInputTests, QueuedDesktopEventsBecomeHarmlessAfterDesktopDestruction)
    {
        MyraEnvironment::setMouseInfoGetterProperty([] { return MouseInfo{{4, 7}, false, false, false, 0.0F}; });
        int mouseMoveCalls = 0;
        {
            Desktop desktop;
            desktop.MouseMoved += [&](void *, Myra::Events::MyraEventArgs &) { ++mouseMoveCalls; };
            desktop.UpdateMouseInput();
        }

        InputEventsManager::ProcessEvents();
        EXPECT_EQ(mouseMoveCalls, 0);
    }

    TEST_F(DesktopInputTests, TabNavigationRoutesKeysAndWrapsAcrossFocusableWidgets)
    {
        MyraEnvironment::DownKeys keys{};
        MyraEnvironment::setDownKeysGetterProperty([&](MyraEnvironment::DownKeys &destination) { destination = keys; });

        Desktop desktop;
        UseFixedBounds(desktop);
        auto root = std::make_shared<Panel>();
        auto first = std::make_shared<KeyProbeWidget>();
        auto skipped = std::make_shared<KeyProbeWidget>();
        auto second = std::make_shared<KeyProbeWidget>();
        first->setAcceptsKeyboardFocusProperty(true);
        skipped->setAcceptsKeyboardFocusProperty(true);
        skipped->setEnabledProperty(false);
        second->setAcceptsKeyboardFocusProperty(true);
        root->AddWidget(first);
        root->AddWidget(skipped);
        root->AddWidget(second);
        desktop.AddWidget(root);

        keys[static_cast<std::size_t>(Keys::Tab)] = true;
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), first.get());
        EXPECT_EQ(first->keysDown, (std::vector<Keys>{Keys::Tab}));

        keys[static_cast<std::size_t>(Keys::Tab)] = false;
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(first->keysUp, (std::vector<Keys>{Keys::Tab}));

        keys[static_cast<std::size_t>(Keys::Tab)] = true;
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), second.get());
        EXPECT_EQ(second->keysDown, (std::vector<Keys>{Keys::Tab}));

        keys[static_cast<std::size_t>(Keys::Tab)] = false;
        desktop.UpdateKeyboardInput();
        keys[static_cast<std::size_t>(Keys::Tab)] = true;
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), first.get());
    }

    TEST_F(DesktopInputTests, KeyboardTransitionsRepeatAndMenuPriorityMatchTheSelectedUpstream)
    {
        MyraEnvironment::DownKeys keys{};
        MyraEnvironment::setDownKeysGetterProperty([&](MyraEnvironment::DownKeys &destination) { destination = keys; });

        Desktop desktop;
        UseFixedBounds(desktop);
        auto root = std::make_shared<Panel>();
        auto focused = std::make_shared<KeyProbeWidget>();
        focused->setAcceptsKeyboardFocusProperty(true);
        auto menu = std::make_shared<HorizontalMenu>();
        menu->getItemsProperty().Add(std::make_shared<MenuItem>("file", "&File"));
        menu->getItemsProperty().Add(std::make_shared<MenuItem>("edit", "&Edit"));
        root->AddWidget(focused);
        root->AddWidget(menu);
        desktop.AddWidget(root);
        desktop.UpdateLayout();
        focused->SetKeyboardFocus();
        desktop.setRepeatKeyDownStartInMsProperty(-1);
        desktop.setRepeatKeyDownIntervalInMsProperty(-1);

        std::vector<Keys> globalDown;
        std::vector<Keys> globalUp;
        desktop.KeyDown +=
            [&](void *, GenericEventArgs<Keys> &arguments) { globalDown.push_back(arguments.getDataProperty()); };
        desktop.KeyUp +=
            [&](void *, GenericEventArgs<Keys> &arguments) { globalUp.push_back(arguments.getDataProperty()); };

        keys[static_cast<std::size_t>(Keys::A)] = true;
        desktop.UpdateKeyboardInput();
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(globalDown, (std::vector<Keys>{Keys::A, Keys::A}));
        EXPECT_EQ(focused->keysDown, (std::vector<Keys>{Keys::A, Keys::A}));
        EXPECT_TRUE(desktop.IsKeyDown(Keys::A));

        keys[static_cast<std::size_t>(Keys::A)] = false;
        desktop.UpdateKeyboardInput();
        EXPECT_EQ(globalUp, (std::vector<Keys>{Keys::A}));
        EXPECT_EQ(focused->keysUp, (std::vector<Keys>{Keys::A}));
        EXPECT_THROW(static_cast<void>(desktop.IsKeyDown(static_cast<Keys>(0xff))), std::out_of_range);

        keys[static_cast<std::size_t>(Keys::LeftAlt)] = true;
        desktop.UpdateKeyboardInput();
        desktop.OnKeyDown(Keys::Right);
        EXPECT_EQ(menu->getHoverIndexProperty(), 0);
        EXPECT_EQ(focused->keysDown, (std::vector<Keys>{Keys::A, Keys::A}));
    }
} // namespace
