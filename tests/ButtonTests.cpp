// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/Button.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
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
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::Button;
    using Myra::Graphics2D::UI::ButtonBase;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(ButtonTests, ContentUsesSingleItemLayoutAndReparentsReplacement)
    {
        Button button;
        EXPECT_EQ(button.getContentProperty(), nullptr);
        EXPECT_TRUE(button.getChildrenProperty().empty());

        auto first = std::make_shared<Widget>();
        first->setWidthProperty(20);
        first->setHeightProperty(10);
        button.setContentProperty(first);

        EXPECT_EQ(button.getContentProperty(), first);
        ASSERT_EQ(button.getChildrenProperty().size(), 1U);
        EXPECT_EQ(button.getChildrenProperty().front(), first);
        EXPECT_EQ(first->getParentProperty(), &button);
        EXPECT_EQ(button.Measure(Point(100, 100)), Point(20, 10));
        button.Arrange(Rectangle(2, 3, 40, 30));
        EXPECT_EQ(first->getContainerBoundsProperty(), Rectangle(0, 0, 20, 10));

        auto replacement = std::make_shared<Widget>();
        button.setContentProperty(replacement);
        EXPECT_EQ(first->getParentProperty(), nullptr);
        EXPECT_EQ(replacement->getParentProperty(), &button);
        ASSERT_EQ(button.getChildrenProperty().size(), 1U);
        EXPECT_EQ(button.getChildrenProperty().front(), replacement);

        button.setContentProperty(nullptr);
        EXPECT_EQ(replacement->getParentProperty(), nullptr);
        EXPECT_TRUE(button.getChildrenProperty().empty());
    }

    TEST(ButtonTests, TouchAndTouchLeftDrivePressedStateAndClick)
    {
        Button button;
        int pressedChanges = 0;
        int clicks = 0;
        InputEventType clickType = InputEventType::None;
        button.PressedChanged += [&](void* sender, Myra::Events::MyraEventArgs&) {
            EXPECT_EQ(sender, &button);
            ++pressedChanges;
        };
        button.Click += [&](void* sender, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(sender, &button);
            ++clicks;
            clickType = arguments.getEventTypeProperty();
        };

        button.OnTouchDown();
        EXPECT_TRUE(button.getIsPressedProperty());
        button.OnTouchLeft();
        EXPECT_FALSE(button.getIsPressedProperty());
        EXPECT_EQ(pressedChanges, 2);
        EXPECT_EQ(clicks, 0);

        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_FALSE(button.getIsPressedProperty());
        EXPECT_EQ(pressedChanges, 4);
        EXPECT_EQ(clicks, 1);
        EXPECT_EQ(clickType, InputEventType::TouchUp);

        button.setReadOnlyProperty(true);
        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_EQ(pressedChanges, 4);
        EXPECT_EQ(clicks, 1);

        button.setReadOnlyProperty(false);
        button.setEnabledProperty(false);
        button.OnTouchDown();
        button.OnTouchUp();
        EXPECT_EQ(pressedChanges, 4);
        EXPECT_EQ(clicks, 1);
    }

    TEST(ButtonTests, SpaceRaisesKeyEventBeforeClickAndHonorsStateGuards)
    {
        Button button;
        std::vector<std::string> calls;
        button.KeyDown += [&](void*, Myra::Events::GenericEventArgs<Keys>& arguments) {
            EXPECT_EQ(arguments.getDataProperty(), Keys::Space);
            calls.emplace_back("key");
        };
        button.PressedChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back(button.getIsPressedProperty() ? "pressed" : "released");
        };
        button.Click += [&](void*, Myra::Events::MyraEventArgs&) {
            calls.emplace_back("click");
        };

        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key", "pressed", "released", "click"}));

        calls.clear();
        button.setEnabledProperty(false);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key"}));

        calls.clear();
        button.setEnabledProperty(true);
        button.setReadOnlyProperty(true);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key"}));
    }

    TEST(ButtonTests, ClonePreservesExactTypeDeepContentAndButtonBaseState)
    {
        Button source;
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(13);
        source.setContentProperty(content);
        source.setReadOnlyProperty(true);
        source.setIsPressedProperty(true);

        const std::shared_ptr<Button> clone =
            std::dynamic_pointer_cast<Button>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getReadOnlyProperty());
        EXPECT_TRUE(clone->getIsPressedProperty());
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 13);
        EXPECT_EQ(clone->getContentProperty()->getParentProperty(), clone.get());

        clone->OnTouchLeft();
        EXPECT_FALSE(clone->getIsPressedProperty());
    }

    TEST(ButtonTests, RegistersAndRoundTripsContentAndReadOnlyMml)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* descriptor = registry.FindByType(typeid(Button));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "Button");
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(ButtonBase)));

        const PropertyDescriptor* readOnly =
            registry.FindPropertyByName(typeid(Button), "ReadOnly");
        const PropertyDescriptor* content =
            registry.FindPropertyByName(typeid(Button), "Content");
        ASSERT_NE(readOnly, nullptr);
        ASSERT_NE(content, nullptr);
        ASSERT_TRUE(readOnly->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*readOnly->getDeclaringTypeProperty(), typeid(ButtonBase));
        EXPECT_TRUE(content->getMetadataProperty().Content);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml(
            "<Button ReadOnly=\"True\" Width=\"30\"><Widget Width=\"12\" /></Button>");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(Button));
        const auto* loadedButton = static_cast<const Button*>(loaded.Value.get());
        EXPECT_TRUE(loadedButton->getReadOnlyProperty());
        EXPECT_EQ(loadedButton->getWidthProperty(), 30);
        ASSERT_NE(loadedButton->getContentProperty(), nullptr);
        EXPECT_EQ(loadedButton->getContentProperty()->getWidthProperty(), 12);
        ASSERT_EQ(loadedButton->getChildrenProperty().size(), 1U);
        EXPECT_EQ(loadedButton->getChildrenProperty().front(), loadedButton->getContentProperty());

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedButton, typeid(Button));
        EXPECT_NE(xml.find("ReadOnly=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("Width=\"30\""), std::string::npos);
        EXPECT_NE(xml.find("<Widget"), std::string::npos);

        const Button defaults;
        const std::string defaultXml = saver.ToXml(&defaults, typeid(Button));
        EXPECT_EQ(defaultXml.find("ReadOnly="), std::string::npos);
    }
}
