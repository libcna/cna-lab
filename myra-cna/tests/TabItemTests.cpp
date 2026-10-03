// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/TabItem.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::TabItem;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    TEST(TabItemTests, RaisesChangedForTextContentAndIdentifierMutations)
    {
        TabItem item;
        int changed = 0;
        item.Changed += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };

        item.setTextProperty(std::string("First"));
        item.setTextProperty(std::string("First"));
        const auto content = std::make_shared<Widget>();
        item.setContentProperty(content);
        item.setContentProperty(content);
        item.setIdProperty(std::string("tab-1"));
        EXPECT_EQ(changed, 3);
        EXPECT_EQ(item.getTextProperty(), std::optional<std::string>("First"));
        EXPECT_EQ(item.getContentProperty(), content);
        EXPECT_EQ(item.ToString(), "First (#tab-1)");

        item.setTextProperty(std::nullopt);
        EXPECT_EQ(item.ToString(), "(#tab-1)");
        item.setIdProperty(std::nullopt);
        EXPECT_EQ(item.ToString(), "");
    }

    TEST(TabItemTests, KeepsSelectionAndNonVisualMetadataInTheDataCore)
    {
        TabItem item;
        int selectionChanges = 0;
        item.SelectedChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectionChanged);
            ++selectionChanges;
        };

        item.setIsSelectedProperty(true);
        item.setIsSelectedProperty(true);
        item.setIsSelectedProperty(false);
        EXPECT_FALSE(item.getIsSelectedProperty());
        EXPECT_EQ(selectionChanges, 1);

        item.setTagProperty(std::string("designer value"));
        item.setImageTextSpacingProperty(7);
        item.setHeightProperty(24);
        EXPECT_EQ(std::any_cast<const std::string &>(item.getTagProperty()), "designer value");
        EXPECT_EQ(item.getImageTextSpacingProperty(), 7);
        EXPECT_EQ(item.getHeightProperty(), std::optional<int>(24));
    }

    TEST(TabItemTests, ClonesPortedStateButPreservesTheSelectedUpstreamFreshIdentity)
    {
        const auto content = std::make_shared<Widget>();
        TabItem item(std::string("Settings"), content);
        item.setIdProperty(std::string("settings-tab"));
        item.setTagProperty(42);
        item.setImageTextSpacingProperty(3);
        item.setHeightProperty(32);
        item.setIsSelectedProperty(true);

        const auto clone = item.Clone();
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getTextProperty(), std::optional<std::string>("Settings"));
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(std::any_cast<int>(clone->getTagProperty()), 42);
        EXPECT_EQ(clone->getImageTextSpacingProperty(), 3);
        EXPECT_EQ(clone->getHeightProperty(), std::optional<int>(32));
        EXPECT_FALSE(clone->getIdProperty().has_value());
        EXPECT_FALSE(clone->getIsSelectedProperty());
        EXPECT_EQ(clone->ToString(), "Settings ");
    }

    TEST(TabItemTests, RegistersAndRoundTripsBaseIdentityAndImplicitContentMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(TabItem));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::MML::BaseObject)));

        const PropertyDescriptor *content = registry.FindPropertyByName(typeid(TabItem), "Content");
        const PropertyDescriptor *text = registry.FindPropertyByName(typeid(TabItem), "Text");
        const PropertyDescriptor *height = registry.FindPropertyByName(typeid(TabItem), "Height");
        const PropertyDescriptor *tag = registry.FindPropertyByName(typeid(TabItem), "Tag");
        const PropertyDescriptor *image = registry.FindPropertyByName(typeid(TabItem), "Image");
        const PropertyDescriptor *spacing = registry.FindPropertyByName(typeid(TabItem), "ImageTextSpacing");
        const PropertyDescriptor *selected = registry.FindPropertyByName(typeid(TabItem), "IsSelected");
        ASSERT_NE(content, nullptr);
        ASSERT_NE(text, nullptr);
        ASSERT_NE(height, nullptr);
        ASSERT_NE(tag, nullptr);
        ASSERT_NE(image, nullptr);
        ASSERT_NE(spacing, nullptr);
        ASSERT_NE(selected, nullptr);
        EXPECT_TRUE(content->getMetadataProperty().Content);
        EXPECT_FALSE(std::any_cast<std::optional<std::string>>(*text->getDefaultValueProperty()).has_value());
        EXPECT_FALSE(std::any_cast<std::optional<int>>(*height->getDefaultValueProperty()).has_value());
        EXPECT_TRUE(tag->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(image->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(spacing->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selected->getMetadataProperty().XmlIgnore);
        EXPECT_EQ(registry.FindPropertyByName(typeid(TabItem), "Color"), nullptr);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<TabItem Id=\"overview\" Text=\"Overview\" Height=\"42\"><Panel Width=\"73\"/></TabItem>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(TabItem));
        auto *item = static_cast<TabItem *>(loaded.Value.get());
        EXPECT_EQ(item->getIdProperty(), std::optional<std::string>("overview"));
        EXPECT_EQ(item->getTextProperty(), std::optional<std::string>("Overview"));
        EXPECT_EQ(item->getHeightProperty(), std::optional<int>(42));
        const auto panel = std::dynamic_pointer_cast<Panel>(item->getContentProperty());
        ASSERT_NE(panel, nullptr);
        EXPECT_EQ(panel->getWidthProperty(), std::optional<int>(73));

        item->setTagProperty(std::string("runtime-only"));
        item->setImageTextSpacingProperty(9);
        item->setIsSelectedProperty(true);
        const std::any tagValue = tag->Get(item);
        EXPECT_EQ(std::any_cast<const std::string &>(std::any_cast<const std::any &>(tagValue)), "runtime-only");
        SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(item, typeid(TabItem));
        EXPECT_NE(xml.find("Id=\"overview\""), std::string::npos);
        EXPECT_NE(xml.find("Text=\"Overview\""), std::string::npos);
        EXPECT_NE(xml.find("Height=\"42\""), std::string::npos);
        EXPECT_NE(xml.find("<Panel Width=\"73\""), std::string::npos);
        EXPECT_EQ(xml.find("Tag="), std::string::npos);
        EXPECT_EQ(xml.find("Image="), std::string::npos);
        EXPECT_EQ(xml.find("ImageTextSpacing="), std::string::npos);
        EXPECT_EQ(xml.find("IsSelected="), std::string::npos);
        EXPECT_EQ(xml.find("Color="), std::string::npos);
    }
} // namespace
