// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Selectors/IMenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::IMenuItem;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Menu;
    using Myra::Graphics2D::UI::MenuItem;
    using Myra::Graphics2D::UI::MenuSeparator;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    class TestImage final : public IImage
    {
      public:
        void Draw(RenderContext &, const Rectangle, const Color) const override {}
        [[nodiscard]] Point getSizeProperty() const override { return Point(1, 1); }
    };

    TEST(MenuItemTests, ExtractsMnemonicAndDefersRichTextStylingToTheFutureMenuLayer)
    {
        MenuItem item;
        int changed = 0;
        item.Changed += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };

        EXPECT_EQ(item.getTextProperty(), std::optional<std::string>(""));
        item.setTextProperty(std::string("E&xit"));
        item.setTextProperty(std::string("E&xit"));
        EXPECT_EQ(item.getUnderscoreCharProperty(), std::optional<char>('x'));
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Exit"));
        EXPECT_EQ(item.getDisabledDisplayTextProperty(), std::optional<std::string>("Exit"));
        EXPECT_EQ(changed, 1);

        item.setTextProperty(std::string("Fish && chips"));
        EXPECT_EQ(item.getUnderscoreCharProperty(), std::optional<char>('&'));
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Fish & chips"));

        item.setTextProperty(std::string("Trailing &"));
        EXPECT_FALSE(item.getUnderscoreCharProperty().has_value());
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Trailing &"));
    }

    TEST(MenuItemTests, RetainsDataCoreStateAndReportsChangedAndSelectedEvents)
    {
        MenuItem item("file", "&File", std::string("designer tag"));
        const auto image = std::make_shared<TestImage>();
        int changed = 0;
        int selected = 0;
        item.Changed += [&](void *, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };
        item.Selected += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectionChanged);
            ++selected;
        };

        item.setImageProperty(image);
        item.setImageProperty(image);
        item.setShortcutTextProperty(std::string("Ctrl+F"));
        item.setShortcutTextProperty(std::string("Ctrl+F"));
        item.setIdProperty(std::string("file-menu"));
        item.setEnabledProperty(false);
        item.setIndexProperty(4);
        item.FireSelected();

        EXPECT_EQ(changed, 3);
        EXPECT_EQ(std::any_cast<const std::string &>(item.getTagProperty()), "designer tag");
        EXPECT_EQ(item.getImageProperty(), image);
        EXPECT_EQ(item.getShortcutTextProperty(), std::optional<std::string>("Ctrl+F"));
        EXPECT_FALSE(item.getEnabledProperty());
        EXPECT_EQ(item.getIndexProperty(), 4);
        EXPECT_EQ(item.getIdProperty(), std::optional<std::string>("file-menu"));
        EXPECT_EQ(selected, 1);
    }

    TEST(MenuItemTests, RegistersAndRoundTripsNestedLogicalMenuItemsMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *interfaceDescriptor = registry.FindByType(typeid(IMenuItem));
        const TypeDescriptor *itemDescriptor = registry.FindByType(typeid(MenuItem));
        const TypeDescriptor *separatorDescriptor = registry.FindByType(typeid(MenuSeparator));
        ASSERT_NE(interfaceDescriptor, nullptr);
        ASSERT_NE(itemDescriptor, nullptr);
        ASSERT_NE(separatorDescriptor, nullptr);
        EXPECT_FALSE(interfaceDescriptor->getCanCreateProperty());
        EXPECT_TRUE(itemDescriptor->getCanCreateProperty());
        EXPECT_TRUE(separatorDescriptor->getCanCreateProperty());
        ASSERT_TRUE(itemDescriptor->getBaseTypeProperty().has_value());
        ASSERT_TRUE(separatorDescriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*itemDescriptor->getBaseTypeProperty(), std::type_index(typeid(IMenuItem)));
        EXPECT_EQ(*separatorDescriptor->getBaseTypeProperty(), std::type_index(typeid(IMenuItem)));

        const PropertyDescriptor *text = registry.FindPropertyByName(typeid(MenuItem), "Text");
        const PropertyDescriptor *image = registry.FindPropertyByName(typeid(MenuItem), "Image");
        const PropertyDescriptor *shortcut = registry.FindPropertyByName(typeid(MenuItem), "ShortcutText");
        const PropertyDescriptor *items = registry.FindPropertyByName(typeid(MenuItem), "Items");
        const PropertyDescriptor *tag = registry.FindPropertyByName(typeid(MenuItem), "Tag");
        const PropertyDescriptor *menu = registry.FindPropertyByName(typeid(MenuItem), "Menu");
        const PropertyDescriptor *underscore = registry.FindPropertyByName(typeid(MenuItem), "UnderscoreChar");
        const PropertyDescriptor *enabled = registry.FindPropertyByName(typeid(MenuItem), "Enabled");
        const PropertyDescriptor *canOpen = registry.FindPropertyByName(typeid(MenuItem), "CanOpen");
        const PropertyDescriptor *index = registry.FindPropertyByName(typeid(MenuItem), "Index");
        ASSERT_NE(text, nullptr);
        ASSERT_NE(image, nullptr);
        ASSERT_NE(shortcut, nullptr);
        ASSERT_NE(items, nullptr);
        ASSERT_NE(tag, nullptr);
        ASSERT_NE(menu, nullptr);
        ASSERT_NE(underscore, nullptr);
        ASSERT_NE(enabled, nullptr);
        ASSERT_NE(canOpen, nullptr);
        ASSERT_NE(index, nullptr);
        EXPECT_FALSE(std::any_cast<std::optional<std::string>>(*text->getDefaultValueProperty()).has_value());
        EXPECT_FALSE(std::any_cast<std::optional<std::string>>(*shortcut->getDefaultValueProperty()).has_value());
        EXPECT_TRUE(image->getMetadataProperty().ExternalAsset);
        EXPECT_TRUE(items->getMetadataProperty().Content);
        ASSERT_TRUE(items->getComplexAdapterProperty().has_value());
        EXPECT_EQ(items->getComplexAdapterProperty()->getItemTypeProperty(), std::type_index(typeid(IMenuItem)));
        EXPECT_TRUE(tag->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(menu->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(underscore->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(enabled->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(canOpen->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(index->getMetadataProperty().XmlIgnore);
        const PropertyDescriptor *separatorId = registry.FindPropertyByName(typeid(MenuSeparator), "Id");
        ASSERT_NE(separatorId, nullptr);
        EXPECT_TRUE(separatorId->getMetadataProperty().XmlIgnore);
        EXPECT_EQ(registry.FindPropertyByName(typeid(MenuItem), "Color"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(MenuItem), "ShortcutColor"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(MenuItem), "SubMenu"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(MenuSeparator), "Separator"), nullptr);

        const std::shared_ptr<IImage> fileImage = std::make_shared<TestImage>();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        loader.LoadExternalAsset = [&](const PropertyDescriptor &, const std::string &assetName) -> std::any
        {
            EXPECT_EQ(assetName, "atlas:file");
            return fileImage;
        };
        System::Xml::XmlDocument document;
        document.LoadXml("<MenuItem Id=\"file\" Text=\"&amp;File\" ShortcutText=\"Alt+F\" Image=\"atlas:file\">"
                         "<MenuItem Id=\"new\" Text=\"&amp;New\"/>"
                         "<MenuSeparator Id=\"ignored\"/>"
                         "<MenuItem Id=\"exit\" Text=\"E&amp;xit\"/>"
                         "</MenuItem>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(MenuItem));
        auto *item = static_cast<MenuItem *>(loaded.Value.get());
        EXPECT_EQ(item->getIdProperty(), std::optional<std::string>("file"));
        EXPECT_EQ(item->getTextProperty(), std::optional<std::string>("&File"));
        EXPECT_EQ(item->getShortcutTextProperty(), std::optional<std::string>("Alt+F"));
        EXPECT_EQ(item->getImageProperty(), fileImage);
        EXPECT_EQ(item->getUnderscoreCharProperty(), std::optional<char>('f'));
        ASSERT_EQ(item->getItemsProperty().getCountProperty(), 3);
        const auto nested = std::dynamic_pointer_cast<MenuItem>(item->getItemsProperty().getItem(0));
        const auto separator = std::dynamic_pointer_cast<MenuSeparator>(item->getItemsProperty().getItem(1));
        const auto exit = std::dynamic_pointer_cast<MenuItem>(item->getItemsProperty().getItem(2));
        ASSERT_NE(nested, nullptr);
        ASSERT_NE(separator, nullptr);
        ASSERT_NE(exit, nullptr);
        EXPECT_EQ(nested->getIdProperty(), std::optional<std::string>("new"));
        EXPECT_FALSE(separator->getIdProperty().has_value());
        EXPECT_EQ(exit->getIdProperty(), std::optional<std::string>("exit"));
        Menu *const subMenu = item->getSubMenuProperty().get();
        EXPECT_EQ(nested->getMenuProperty(), subMenu);
        EXPECT_EQ(separator->getMenuProperty(), subMenu);
        EXPECT_EQ(exit->getMenuProperty(), subMenu);
        EXPECT_EQ(nested->getIndexProperty(), 0);
        EXPECT_EQ(separator->getIndexProperty(), 1);
        EXPECT_EQ(exit->getIndexProperty(), 2);

        item->setTagProperty(std::string("runtime-only"));
        item->setEnabledProperty(false);
        SaveContext saver(registry, codecs);
        saver.SaveExternalAsset = [&](const PropertyDescriptor &, const std::any &value)
        {
            EXPECT_EQ(std::any_cast<const std::shared_ptr<IImage> &>(value), fileImage);
            return std::string("atlas:file");
        };
        const std::string xml = saver.ToXml(item, typeid(MenuItem));
        EXPECT_NE(xml.find("Id=\"file\""), std::string::npos);
        EXPECT_NE(xml.find("Text=\"&amp;File\""), std::string::npos);
        EXPECT_NE(xml.find("ShortcutText=\"Alt+F\""), std::string::npos);
        EXPECT_NE(xml.find("Image=\"atlas:file\""), std::string::npos);
        EXPECT_NE(xml.find("Id=\"new\""), std::string::npos);
        EXPECT_NE(xml.find("<MenuSeparator"), std::string::npos);
        EXPECT_NE(xml.find("Id=\"exit\""), std::string::npos);
        EXPECT_EQ(xml.find("Id=\"ignored\""), std::string::npos);
        EXPECT_EQ(xml.find("Tag="), std::string::npos);
        EXPECT_EQ(xml.find("Menu="), std::string::npos);
        EXPECT_EQ(xml.find("UnderscoreChar="), std::string::npos);
        EXPECT_EQ(xml.find("Enabled="), std::string::npos);
        EXPECT_EQ(xml.find("CanOpen="), std::string::npos);
        EXPECT_EQ(xml.find("Index="), std::string::npos);
        EXPECT_EQ(xml.find("Color="), std::string::npos);
        EXPECT_EQ(xml.find("ShortcutColor="), std::string::npos);
        EXPECT_EQ(xml.find("Separator="), std::string::npos);
    }
} // namespace
