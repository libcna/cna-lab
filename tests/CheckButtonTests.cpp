// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/CheckButton.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Myra::Graphics2D::UI::CheckButton;
    using Myra::Graphics2D::UI::CheckButtonBase;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(CheckButtonTests, AliasesCheckedStateAndPressedChangedEvent)
    {
        CheckButton button;
        EXPECT_FALSE(button.getIsCheckedProperty());
        EXPECT_EQ(&button.IsCheckedChanged, &button.PressedChanged);

        int changes = 0;
        InputEventType eventType = InputEventType::None;
        const auto token = button.IsCheckedChanged.Add(
            [&](void *sender, Myra::Events::MyraEventArgs &arguments)
            {
                EXPECT_EQ(sender, &button);
                ++changes;
                eventType = arguments.getEventTypeProperty();
            });

        button.setIsCheckedProperty(true);
        button.setIsCheckedProperty(true);
        EXPECT_TRUE(button.getIsPressedProperty());
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(eventType, InputEventType::PressedChanged);
        EXPECT_TRUE(button.PressedChanged.Remove(token));
        button.setIsCheckedProperty(false);
        EXPECT_EQ(changes, 1);
    }

    TEST(CheckButtonTests, ClonePreservesExactTypeStateAndASeparateEventAlias)
    {
        CheckButton source;
        source.setIsCheckedProperty(true);
        source.setReadOnlyProperty(true);
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(14);
        source.setContentProperty(content);

        int sourceEvents = 0;
        source.IsCheckedChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++sourceEvents; };

        const std::shared_ptr<CheckButton> clone = std::dynamic_pointer_cast<CheckButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getIsCheckedProperty());
        EXPECT_TRUE(clone->getReadOnlyProperty());
        EXPECT_EQ(&clone->IsCheckedChanged, &clone->PressedChanged);
        EXPECT_NE(&clone->IsCheckedChanged, &source.IsCheckedChanged);
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 14);
        EXPECT_EQ(clone->getContentProperty()->getParentProperty(), clone.get());

        clone->setIsCheckedProperty(false);
        EXPECT_EQ(sourceEvents, 0);
    }

    TEST(CheckButtonTests, RegistersAndRoundTripsCheckedContentMml)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(CheckButton));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "CheckButton");
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(CheckButtonBase)));

        const PropertyDescriptor *checked = registry.FindPropertyByName(typeid(CheckButton), "IsChecked");
        const PropertyDescriptor *content = registry.FindPropertyByName(typeid(CheckButton), "Content");
        ASSERT_NE(checked, nullptr);
        ASSERT_NE(content, nullptr);
        ASSERT_TRUE(checked->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*checked->getDeclaringTypeProperty(), typeid(CheckButton));
        ASSERT_TRUE(checked->getDefaultValueProperty().has_value());
        EXPECT_FALSE(std::any_cast<bool>(*checked->getDefaultValueProperty()));
        EXPECT_TRUE(content->getMetadataProperty().Content);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<CheckButton IsChecked=\"True\" ReadOnly=\"True\">"
                         "<Widget Width=\"12\" /></CheckButton>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(CheckButton));
        const auto *loadedButton = static_cast<const CheckButton *>(loaded.Value.get());
        EXPECT_TRUE(loadedButton->getIsCheckedProperty());
        EXPECT_TRUE(loadedButton->getReadOnlyProperty());
        ASSERT_NE(loadedButton->getContentProperty(), nullptr);
        EXPECT_EQ(loadedButton->getContentProperty()->getWidthProperty(), 12);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedButton, typeid(CheckButton));
        EXPECT_NE(xml.find("IsChecked=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("ReadOnly=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("<Widget"), std::string::npos);

        const CheckButton defaults;
        const std::string defaultXml = saver.ToXml(&defaults, typeid(CheckButton));
        EXPECT_EQ(defaultXml.find("IsChecked="), std::string::npos);
    }
} // namespace
