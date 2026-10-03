// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"

namespace
{
    using Myra::Graphics2D::UI::ButtonBase;
    using Myra::Graphics2D::UI::IInputEventsProcessor;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;

    class TestButton final : public ButtonBase
    {
    public:
        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override
        {
            return content_;
        }

        void setContentProperty(std::shared_ptr<Widget> value) override
        {
            content_ = std::move(value);
        }

        void setCallLog(std::vector<std::string>* calls) noexcept { calls_ = calls; }
        [[nodiscard]] int getDownCalls() const noexcept { return downCalls_; }
        [[nodiscard]] int getUpCalls() const noexcept { return upCalls_; }

    protected:
        void InternalOnTouchUp() override
        {
            ++upCalls_;
            if (calls_ != nullptr)
            {
                calls_->emplace_back("internal:up");
            }
        }

        void InternalOnTouchDown() override
        {
            ++downCalls_;
            if (calls_ != nullptr)
            {
                calls_->emplace_back("internal:down");
            }
        }

        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<TestButton>();
        }

    private:
        std::shared_ptr<Widget> content_;
        std::vector<std::string>* calls_ = nullptr;
        int downCalls_ = 0;
        int upCalls_ = 0;
    };

    TEST(ButtonBaseTests, TouchStateMachineHonorsEnabledAndReadOnly)
    {
        TestButton button;
        int clicks = 0;
        button.Click += [&](void* sender, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(sender, &button);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::TouchUp);
            ++clicks;
        };

        EXPECT_FALSE(button.getReadOnlyProperty());
        button.OnTouchUp();
        EXPECT_EQ(button.getUpCalls(), 1);
        EXPECT_EQ(clicks, 0);

        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_EQ(button.getDownCalls(), 1);
        EXPECT_EQ(button.getUpCalls(), 2);
        EXPECT_EQ(clicks, 1);

        button.setReadOnlyProperty(true);
        button.DoClick();
        EXPECT_EQ(button.getDownCalls(), 1);
        EXPECT_EQ(button.getUpCalls(), 2);
        EXPECT_EQ(clicks, 1);

        button.setReadOnlyProperty(false);
        button.setEnabledProperty(false);
        button.DoClick();
        EXPECT_EQ(button.getDownCalls(), 1);
        EXPECT_EQ(button.getUpCalls(), 2);
        EXPECT_EQ(clicks, 1);
    }

    TEST(ButtonBaseTests, QueuedPointerDeliveryComposesButtonAndWidgetEventOrdering)
    {
        std::vector<std::string> calls;
        auto button = std::make_shared<TestButton>();
        button->setCallLog(&calls);
        button->TouchDown += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back("event:down");
        };
        button->Click += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back("event:click");
        };
        button->TouchUp += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back("event:up");
        };

        const std::shared_ptr<IInputEventsProcessor> processor = button;
        InputEventsManager::Queue(processor, InputEventType::TouchDown);
        InputEventsManager::Queue(processor, InputEventType::TouchUp);
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(calls, (std::vector<std::string>{
                             "internal:down", "event:down", "internal:up",
                             "event:click", "event:up"}));
    }

    TEST(ButtonBaseTests, ClonePreservesConfigurationButNotTransientClickArming)
    {
        TestButton source;
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(13);
        source.setContentProperty(content);
        source.setIsPressedProperty(true);
        source.OnTouchDown();
        source.setReadOnlyProperty(true);

        const std::shared_ptr<TestButton> clone =
            std::dynamic_pointer_cast<TestButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getReadOnlyProperty());
        EXPECT_TRUE(clone->getIsPressedProperty());
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 13);

        int cloneClicks = 0;
        clone->Click += [&](void*, Myra::Events::MyraEventArgs&) {
            ++cloneClicks;
        };
        clone->setReadOnlyProperty(false);
        clone->OnTouchUp();
        EXPECT_EQ(clone->getUpCalls(), 1);
        EXPECT_EQ(cloneClicks, 0);
    }

    TEST(ButtonBaseTests, RegistersAbstractReadOnlyMmlMetadata)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* descriptor = registry.FindByType(typeid(ButtonBase));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "ButtonBase");
        EXPECT_FALSE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::Graphics2D::UI::ContentControl)));

        const PropertyDescriptor* readOnly =
            registry.FindPropertyByName(typeid(ButtonBase), "ReadOnly");
        ASSERT_NE(readOnly, nullptr);
        ASSERT_TRUE(readOnly->getDefaultValueProperty().has_value());
        EXPECT_FALSE(std::any_cast<bool>(*readOnly->getDefaultValueProperty()));

        TestButton button;
        readOnly->Set(static_cast<ButtonBase*>(&button), true);
        EXPECT_TRUE(button.getReadOnlyProperty());
        EXPECT_TRUE(std::any_cast<bool>(readOnly->Get(static_cast<const ButtonBase*>(&button))));
    }
}
