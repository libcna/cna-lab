// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Microsoft::Xna::Framework::Point;
    using Myra::Graphics2D::UI::ButtonBase;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::ToggleButton;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(ToggleButtonTests, AliasesToggledStateEventAndLaysOutContent)
    {
        ToggleButton button;
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(&button.IsToggledChanged, &button.PressedChanged);

        int changes = 0;
        InputEventType eventType = InputEventType::None;
        const auto token = button.IsToggledChanged.Add(
            [&](void* sender, Myra::Events::MyraEventArgs& arguments) {
                EXPECT_EQ(sender, &button);
                ++changes;
                eventType = arguments.getEventTypeProperty();
            });
        button.setIsToggledProperty(true);
        button.setIsToggledProperty(true);
        EXPECT_TRUE(button.getIsPressedProperty());
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(eventType, InputEventType::PressedChanged);
        EXPECT_TRUE(button.PressedChanged.Remove(token));
        button.setIsToggledProperty(false);
        EXPECT_EQ(changes, 1);

        auto content = std::make_shared<Widget>();
        content->setWidthProperty(21);
        content->setHeightProperty(9);
        button.setContentProperty(content);
        EXPECT_EQ(button.getContentProperty(), content);
        ASSERT_EQ(button.getChildrenProperty().size(), 1U);
        EXPECT_EQ(button.getChildrenProperty().front(), content);
        EXPECT_EQ(content->getParentProperty(), &button);
        EXPECT_EQ(button.Measure(Point(100, 100)), Point(21, 9));

        button.setContentProperty(nullptr);
        EXPECT_EQ(content->getParentProperty(), nullptr);
        EXPECT_TRUE(button.getChildrenProperty().empty());
    }

    TEST(ToggleButtonTests, TouchDownTogglesAndTouchUpClicksWithoutChangingState)
    {
        ToggleButton button;
        int changes = 0;
        int clicks = 0;
        button.IsToggledChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            ++changes;
        };
        button.Click += [&](void*, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::TouchUp);
            ++clicks;
        };

        button.OnTouchDown();
        EXPECT_TRUE(button.getIsToggledProperty());
        EXPECT_EQ(changes, 1);
        button.OnTouchLeft();
        EXPECT_TRUE(button.getIsToggledProperty());
        button.OnTouchUp();
        EXPECT_TRUE(button.getIsToggledProperty());
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(clicks, 1);

        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(changes, 2);
        EXPECT_EQ(clicks, 2);

        button.setReadOnlyProperty(true);
        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(changes, 2);
        EXPECT_EQ(clicks, 2);

        button.setReadOnlyProperty(false);
        button.setEnabledProperty(false);
        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(changes, 2);
        EXPECT_EQ(clicks, 2);
    }

    TEST(ToggleButtonTests, CancelledUserToggleKeepsStateButRetainsTouchClickArming)
    {
        ToggleButton button;
        int changing = 0;
        int clicks = 0;
        const auto token = button.PressedChangingByUser.Add(
            [&](void*, Myra::Events::ValueChangingEventArgs<bool>& arguments) {
                EXPECT_FALSE(arguments.getOldValueProperty());
                EXPECT_TRUE(arguments.getNewValueProperty());
                ++changing;
                arguments.Cancel = true;
            });
        button.Click += [&](void*, Myra::Events::MyraEventArgs&) {
            ++clicks;
        };

        button.OnTouchDown();
        EXPECT_FALSE(button.getIsToggledProperty());
        button.OnTouchUp();
        EXPECT_EQ(changing, 1);
        EXPECT_EQ(clicks, 1);

        button.OnKeyDown(Keys::Space);
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(changing, 2);
        EXPECT_EQ(clicks, 1);

        EXPECT_TRUE(button.PressedChangingByUser.Remove(token));
        button.OnKeyDown(Keys::Space);
        EXPECT_TRUE(button.getIsToggledProperty());
    }

    TEST(ToggleButtonTests, SpaceRaisesKeyEventThenTogglesAndPreservesUpstreamReadOnlyBehavior)
    {
        ToggleButton button;
        std::vector<std::string> calls;
        int clicks = 0;
        button.KeyDown += [&](void*, Myra::Events::GenericEventArgs<Keys>& arguments) {
            EXPECT_EQ(arguments.getDataProperty(), Keys::Space);
            calls.emplace_back("key");
        };
        button.IsToggledChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back(button.getIsToggledProperty() ? "on" : "off");
        };
        button.Click += [&](void*, Myra::Events::MyraEventArgs&) {
            ++clicks;
        };

        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key", "on"}));
        EXPECT_EQ(clicks, 0);

        calls.clear();
        button.setEnabledProperty(false);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key"}));
        EXPECT_TRUE(button.getIsToggledProperty());

        calls.clear();
        button.setEnabledProperty(true);
        button.setReadOnlyProperty(true);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key", "off"}));
        EXPECT_FALSE(button.getIsToggledProperty());
        EXPECT_EQ(clicks, 0);
    }

    TEST(ToggleButtonTests, ClonePreservesExactTypeDeepContentStateAndLocalEventAlias)
    {
        ToggleButton source;
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(14);
        source.setContentProperty(content);
        source.setIsToggledProperty(true);
        source.setReadOnlyProperty(true);
        int sourceEvents = 0;
        source.IsToggledChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            ++sourceEvents;
        };

        const std::shared_ptr<ToggleButton> clone =
            std::dynamic_pointer_cast<ToggleButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getIsToggledProperty());
        EXPECT_TRUE(clone->getReadOnlyProperty());
        EXPECT_EQ(&clone->IsToggledChanged, &clone->PressedChanged);
        EXPECT_NE(&clone->IsToggledChanged, &source.IsToggledChanged);
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 14);
        EXPECT_EQ(clone->getContentProperty()->getParentProperty(), clone.get());

        clone->setIsToggledProperty(false);
        EXPECT_EQ(sourceEvents, 0);
    }

    TEST(ToggleButtonTests, RegistersAndRoundTripsToggledContentMml)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* descriptor = registry.FindByType(typeid(ToggleButton));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "ToggleButton");
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(ButtonBase)));

        const PropertyDescriptor* toggled =
            registry.FindPropertyByName(typeid(ToggleButton), "IsToggled");
        const PropertyDescriptor* content =
            registry.FindPropertyByName(typeid(ToggleButton), "Content");
        ASSERT_NE(toggled, nullptr);
        ASSERT_NE(content, nullptr);
        ASSERT_TRUE(toggled->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*toggled->getDeclaringTypeProperty(), typeid(ToggleButton));
        ASSERT_TRUE(toggled->getDefaultValueProperty().has_value());
        EXPECT_FALSE(std::any_cast<bool>(*toggled->getDefaultValueProperty()));
        EXPECT_TRUE(content->getMetadataProperty().Content);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml(
            "<ToggleButton IsToggled=\"True\" ReadOnly=\"True\">"
            "<Widget Width=\"12\" /></ToggleButton>");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(ToggleButton));
        const auto* loadedButton = static_cast<const ToggleButton*>(loaded.Value.get());
        EXPECT_TRUE(loadedButton->getIsToggledProperty());
        EXPECT_TRUE(loadedButton->getReadOnlyProperty());
        ASSERT_NE(loadedButton->getContentProperty(), nullptr);
        EXPECT_EQ(loadedButton->getContentProperty()->getWidthProperty(), 12);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedButton, typeid(ToggleButton));
        EXPECT_NE(xml.find("IsToggled=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("ReadOnly=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("<Widget"), std::string::npos);

        const ToggleButton defaults;
        const std::string defaultXml = saver.ToXml(&defaults, typeid(ToggleButton));
        EXPECT_EQ(defaultXml.find("IsToggled="), std::string::npos);
    }
}
