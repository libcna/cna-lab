// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/RadioButton.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Myra::Graphics2D::UI::CheckButtonBase;
    using Myra::Graphics2D::UI::RadioButton;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(RadioButtonTests, SelectingOneSiblingReleasesTheOther)
    {
        Widget parent;
        auto first = std::make_shared<RadioButton>();
        auto second = std::make_shared<RadioButton>();
        parent.AddChild(first);
        parent.AddChild(second);

        first->setIsPressedProperty(true);
        ASSERT_TRUE(first->getIsPressedProperty());
        EXPECT_FALSE(second->getIsPressedProperty());

        second->setIsPressedProperty(true);
        EXPECT_FALSE(first->getIsPressedProperty());
        EXPECT_TRUE(second->getIsPressedProperty());
    }

    TEST(RadioButtonTests, ASelectedGroupedButtonCannotBeTheLastButtonReleased)
    {
        Widget parent;
        auto first = std::make_shared<RadioButton>();
        auto second = std::make_shared<RadioButton>();
        parent.AddChild(first);
        parent.AddChild(second);

        first->setIsPressedProperty(true);
        first->setIsPressedProperty(false);
        EXPECT_TRUE(first->getIsPressedProperty());

        second->setIsPressedProperty(true);
        second->setIsPressedProperty(false);
        EXPECT_TRUE(second->getIsPressedProperty());
        EXPECT_FALSE(first->getIsPressedProperty());

        RadioButton standalone;
        standalone.setIsPressedProperty(true);
        standalone.setIsPressedProperty(false);
        EXPECT_FALSE(standalone.getIsPressedProperty());
    }

    TEST(RadioButtonTests, CloneAndMmlPreserveTheConcreteTypeAndContent)
    {
        RadioButton source;
        source.setIsPressedProperty(true);
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(16);
        source.setContentProperty(content);

        const std::shared_ptr<RadioButton> clone = std::dynamic_pointer_cast<RadioButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getIsPressedProperty());
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 16);

        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(RadioButton));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "RadioButton");
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(CheckButtonBase)));
        EXPECT_EQ(registry.FindPropertyByName(typeid(RadioButton), "IsPressed"), nullptr);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<RadioButton><Widget Width=\"13\" /></RadioButton>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(RadioButton));
        const auto *loadedButton = static_cast<const RadioButton *>(loaded.Value.get());
        EXPECT_FALSE(loadedButton->getIsPressedProperty());
        ASSERT_NE(loadedButton->getContentProperty(), nullptr);
        EXPECT_EQ(loadedButton->getContentProperty()->getWidthProperty(), 13);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedButton, typeid(RadioButton));
        EXPECT_EQ(xml.find("IsPressed="), std::string::npos);
        EXPECT_NE(xml.find("<Widget"), std::string::npos);
    }
} // namespace
