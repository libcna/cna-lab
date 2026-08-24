// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Events/EventHandlingStrategy.hpp"
#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Events::EventHandlingStrategy;
    using Myra::Events::GenericEventArgs;
    using Myra::Graphics2D::UI::IInputEventsProcessor;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::Widget;

    class EventModelGuard final
    {
    public:
        EventModelGuard()
            : previous_(Myra::MyraEnvironment::getEventHandlingModelProperty())
        {
            Myra::MyraEnvironment::setEventHandlingModelProperty(
                EventHandlingStrategy::EventCapturing);
        }

        ~EventModelGuard()
        {
            Myra::MyraEnvironment::setEventHandlingModelProperty(previous_);
        }

    private:
        EventHandlingStrategy previous_;
    };

    class RecordingWidget final : public Widget
    {
    public:
        explicit RecordingWidget(std::vector<std::string>& calls) : calls_(calls) {}

        void OnMouseLeft() override { calls_.emplace_back("hook:MouseLeft"); }
        void OnMouseEntered() override { calls_.emplace_back("hook:MouseEntered"); }
        void OnMouseMoved() override { calls_.emplace_back("hook:MouseMoved"); }
        void OnTouchLeft() override { calls_.emplace_back("hook:TouchLeft"); }
        void OnTouchEntered() override { calls_.emplace_back("hook:TouchEntered"); }
        void OnTouchMoved() override { calls_.emplace_back("hook:TouchMoved"); }
        void OnTouchDown() override { calls_.emplace_back("hook:TouchDown"); }
        void OnTouchUp() override { calls_.emplace_back("hook:TouchUp"); }
        void OnTouchDoubleClick() override { calls_.emplace_back("hook:TouchDoubleClick"); }

    private:
        std::vector<std::string>& calls_;
    };

    class RecordingKeyWidget final : public Widget
    {
    public:
        explicit RecordingKeyWidget(std::vector<std::string>& calls) : calls_(calls) {}

        void OnKeyDown(const Keys key) override
        {
            calls_.emplace_back("hook:KeyDown");
            Widget::OnKeyDown(key);
        }

        void OnKeyUp(const Keys key) override
        {
            calls_.emplace_back("hook:KeyUp");
            Widget::OnKeyUp(key);
        }

        void OnChar(const char16_t character) override
        {
            calls_.emplace_back("hook:Char");
            Widget::OnChar(character);
        }

        void EmitKeyDown(const Keys key) { FireKeyDown(key); }

    private:
        std::vector<std::string>& calls_;
    };

    void AddPointerEventRecorder(Widget& widget, Myra::Events::MyraEventHandler& event,
        std::vector<std::string>& calls, std::string name, const InputEventType expected)
    {
        event += [&widget, &calls, name = std::move(name), expected](
                     void* sender, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(sender, &widget);
            EXPECT_EQ(arguments.getEventTypeProperty(), expected);
            calls.emplace_back("event:" + name);
        };
    }

    TEST(WidgetInputTests, QueuedPointerEventsInvokeVirtualHooksBeforeMatchingEvents)
    {
        EventModelGuard guard;
        std::vector<std::string> calls;
        auto widget = std::make_shared<RecordingWidget>(calls);

        AddPointerEventRecorder(
            *widget, widget->MouseLeft, calls, "MouseLeft", InputEventType::MouseLeft);
        AddPointerEventRecorder(*widget, widget->MouseEntered, calls, "MouseEntered",
            InputEventType::MouseEntered);
        AddPointerEventRecorder(
            *widget, widget->MouseMoved, calls, "MouseMoved", InputEventType::MouseMoved);
        AddPointerEventRecorder(
            *widget, widget->TouchLeft, calls, "TouchLeft", InputEventType::TouchLeft);
        AddPointerEventRecorder(*widget, widget->TouchEntered, calls, "TouchEntered",
            InputEventType::TouchEntered);
        AddPointerEventRecorder(
            *widget, widget->TouchMoved, calls, "TouchMoved", InputEventType::TouchMoved);
        AddPointerEventRecorder(
            *widget, widget->TouchDown, calls, "TouchDown", InputEventType::TouchDown);
        AddPointerEventRecorder(
            *widget, widget->TouchUp, calls, "TouchUp", InputEventType::TouchUp);
        AddPointerEventRecorder(*widget, widget->TouchDoubleClick, calls, "TouchDoubleClick",
            InputEventType::TouchDoubleClick);

        const std::shared_ptr<IInputEventsProcessor> processor = widget;
        InputEventsManager::Queue(processor, InputEventType::MouseLeft);
        InputEventsManager::Queue(processor, InputEventType::MouseEntered);
        InputEventsManager::Queue(processor, InputEventType::MouseMoved);
        InputEventsManager::Queue(processor, InputEventType::TouchLeft);
        InputEventsManager::Queue(processor, InputEventType::TouchEntered);
        InputEventsManager::Queue(processor, InputEventType::TouchMoved);
        InputEventsManager::Queue(processor, InputEventType::TouchDown);
        InputEventsManager::Queue(processor, InputEventType::TouchUp);
        InputEventsManager::Queue(processor, InputEventType::TouchDoubleClick);
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(calls, (std::vector<std::string>{
                             "hook:MouseLeft", "event:MouseLeft",
                             "hook:MouseEntered", "event:MouseEntered",
                             "hook:MouseMoved", "event:MouseMoved",
                             "hook:TouchLeft", "event:TouchLeft",
                             "hook:TouchEntered", "event:TouchEntered",
                             "hook:TouchMoved", "event:TouchMoved",
                             "hook:TouchDown", "event:TouchDown",
                             "hook:TouchUp", "event:TouchUp",
                             "hook:TouchDoubleClick", "event:TouchDoubleClick"}));
    }

    TEST(WidgetInputTests, DirectPointerHooksDoNotRaiseQueuedEvents)
    {
        Widget widget;
        int touchDownEvents = 0;
        widget.TouchDown += [&](void*, Myra::Events::MyraEventArgs&) {
            ++touchDownEvents;
        };

        widget.OnTouchDown();

        EXPECT_EQ(touchDownEvents, 0);
    }

    TEST(WidgetInputTests, KeyCallbacksForwardTheExactCnaKeyAndEventType)
    {
        std::vector<std::string> calls;
        RecordingKeyWidget widget(calls);
        std::vector<Keys> keys;
        widget.KeyDown += [&](void* sender, GenericEventArgs<Keys>& arguments) {
            EXPECT_EQ(sender, &widget);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::KeyDown);
            calls.emplace_back("event:KeyDown");
            keys.push_back(arguments.getDataProperty());
        };
        widget.KeyUp += [&](void* sender, GenericEventArgs<Keys>& arguments) {
            EXPECT_EQ(sender, &widget);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::KeyUp);
            calls.emplace_back("event:KeyUp");
            keys.push_back(arguments.getDataProperty());
        };

        widget.OnKeyDown(Keys::OemPlus);
        widget.OnKeyUp(Keys::Escape);
        widget.EmitKeyDown(Keys::NumPad7);

        EXPECT_EQ(calls, (std::vector<std::string>{
                             "hook:KeyDown", "event:KeyDown",
                             "hook:KeyUp", "event:KeyUp", "event:KeyDown"}));
        EXPECT_EQ(keys, (std::vector<Keys>{Keys::OemPlus, Keys::Escape, Keys::NumPad7}));
    }

    TEST(WidgetInputTests, CharacterCallbackForwardsTheExactUtf16CodeUnitAndEventType)
    {
        std::vector<std::string> calls;
        RecordingKeyWidget widget(calls);
        std::vector<char16_t> characters;
        widget.Char += [&](void *sender, GenericEventArgs<char16_t> &arguments)
        {
            EXPECT_EQ(sender, &widget);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::CharInput);
            calls.emplace_back("event:Char");
            characters.push_back(arguments.getDataProperty());
        };

        widget.OnChar(u'\u00e9');
        widget.OnChar(static_cast<char16_t>(0xd83d));
        widget.OnChar(static_cast<char16_t>(0xde00));

        EXPECT_EQ(calls, (std::vector<std::string>{"hook:Char", "event:Char", "hook:Char", "event:Char", "hook:Char",
                                                   "event:Char"}));
        EXPECT_EQ(characters,
                  (std::vector<char16_t>{u'\u00e9', static_cast<char16_t>(0xd83d), static_cast<char16_t>(0xde00)}));
    }

    TEST(WidgetInputTests, QueuedDispatchRetainsAWidgetUntilItsEventCompletes)
    {
        EventModelGuard guard;
        bool called = false;
        auto widget = std::make_shared<Widget>();
        Widget* const senderAddress = widget.get();
        const std::weak_ptr<Widget> weakWidget = widget;
        widget->TouchUp += [&](void* sender, Myra::Events::MyraEventArgs&) {
            EXPECT_EQ(sender, senderAddress);
            EXPECT_FALSE(weakWidget.expired());
            called = true;
        };

        InputEventsManager::Queue(widget, InputEventType::TouchUp);
        widget.reset();
        EXPECT_FALSE(weakWidget.expired());

        InputEventsManager::ProcessEvents();

        EXPECT_TRUE(called);
        EXPECT_TRUE(weakWidget.expired());
    }
}
