// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalMenu;
    using Myra::Graphics2D::UI::MenuItem;
    using Myra::Graphics2D::UI::MenuSeparator;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalMenu;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    TEST(MenuTests, RetainsItemsUpdatesOwnershipAndSearchesNestedSubmenus)
    {
        HorizontalMenu menu;
        const auto file = std::make_shared<MenuItem>("file", "&File");
        const auto separator = std::make_shared<MenuSeparator>();
        const auto edit = std::make_shared<MenuItem>("edit", "&Edit");
        const auto nested = std::make_shared<MenuItem>("new", "&New");

        menu.getItemsProperty().Add(file);
        menu.getItemsProperty().Add(separator);
        menu.getItemsProperty().Add(edit);
        file->getItemsProperty().Add(nested);

        EXPECT_EQ(menu.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(menu.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(menu.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        EXPECT_TRUE(menu.getAcceptsKeyboardFocusProperty());
        EXPECT_EQ(file->getMenuProperty(), &menu);
        EXPECT_EQ(separator->getMenuProperty(), &menu);
        EXPECT_EQ(edit->getMenuProperty(), &menu);
        EXPECT_EQ(file->getIndexProperty(), 0);
        EXPECT_EQ(separator->getIndexProperty(), 1);
        EXPECT_EQ(edit->getIndexProperty(), 2);
        EXPECT_TRUE(file->getCanOpenProperty());
        EXPECT_EQ(nested->getMenuProperty(), file->getSubMenuProperty().get());
        EXPECT_EQ(menu.FindMenuItemById("new"), nested.get());
        EXPECT_EQ(file->FindMenuItemById("new"), nested.get());
        EXPECT_EQ(menu.FindMenuItemById("missing"), nullptr);

        EXPECT_TRUE(menu.getItemsProperty().Remove(file));
        EXPECT_EQ(file->getMenuProperty(), nullptr);
        EXPECT_EQ(separator->getIndexProperty(), 0);
        EXPECT_EQ(edit->getIndexProperty(), 1);
    }

    TEST(MenuTests, NavigatesSkipsSeparatorsExecutesMnemonicsAndOpensSubmenuState)
    {
        HorizontalMenu menu;
        const auto file = std::make_shared<MenuItem>("file", "&File");
        const auto separator = std::make_shared<MenuSeparator>();
        const auto edit = std::make_shared<MenuItem>("edit", "&Edit");
        const auto nested = std::make_shared<MenuItem>("new", "&New");
        int editSelections = 0;
        int nestedSelections = 0;
        edit->Selected += [&](void *, Myra::Events::MyraEventArgs &) { ++editSelections; };
        nested->Selected += [&](void *, Myra::Events::MyraEventArgs &) { ++nestedSelections; };

        menu.getItemsProperty().Add(file);
        menu.getItemsProperty().Add(separator);
        menu.getItemsProperty().Add(edit);
        file->getItemsProperty().Add(nested);

        menu.MoveHover(1);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);
        menu.OnKeyDown(Keys::Right);
        EXPECT_EQ(menu.getHoverIndexProperty(), 2);
        menu.OnKeyDown(Keys::Left);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);

        menu.OnKeyDown(Keys::E);
        EXPECT_EQ(editSelections, 1);
        EXPECT_FALSE(menu.getHoverIndexProperty().has_value());
        EXPECT_FALSE(menu.getSelectedIndexProperty().has_value());

        menu.Click(0);
        EXPECT_TRUE(menu.getIsOpenProperty());
        EXPECT_EQ(menu.getOpenMenuItemProperty(), file.get());
        EXPECT_EQ(menu.getSelectedIndexProperty(), 0);
        menu.OnKeyDown(Keys::N);
        EXPECT_EQ(nestedSelections, 1);

        menu.Close();
        EXPECT_FALSE(menu.getIsOpenProperty());
        EXPECT_FALSE(menu.getHoverIndexProperty().has_value());
        EXPECT_FALSE(menu.getSelectedIndexProperty().has_value());
    }

    TEST(MenuTests, VerticalMenuUsesUpAndDownAndItsOwnAlignmentDefaults)
    {
        VerticalMenu menu;
        const auto first = std::make_shared<MenuItem>("first", "&First");
        const auto second = std::make_shared<MenuItem>("second", "&Second");
        menu.getItemsProperty().Add(first);
        menu.getItemsProperty().Add(second);

        EXPECT_EQ(menu.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(menu.getHorizontalAlignmentProperty(), HorizontalAlignment::Left);
        EXPECT_EQ(menu.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        menu.OnKeyDown(Keys::Down);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);
        menu.OnKeyDown(Keys::Up);
        EXPECT_EQ(menu.getHoverIndexProperty(), 1);
    }

    TEST(MenuTests, RegistersAndRoundTripsLogicalItemsAndConcreteMenuDefaultsMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *menuDescriptor = registry.FindByType(typeid(Myra::Graphics2D::UI::Menu));
        const TypeDescriptor *horizontalDescriptor = registry.FindByType(typeid(HorizontalMenu));
        const TypeDescriptor *verticalDescriptor = registry.FindByType(typeid(VerticalMenu));
        ASSERT_NE(menuDescriptor, nullptr);
        ASSERT_NE(horizontalDescriptor, nullptr);
        ASSERT_NE(verticalDescriptor, nullptr);
        EXPECT_FALSE(menuDescriptor->getCanCreateProperty());
        EXPECT_TRUE(horizontalDescriptor->getCanCreateProperty());
        EXPECT_TRUE(verticalDescriptor->getCanCreateProperty());
        ASSERT_TRUE(menuDescriptor->getBaseTypeProperty().has_value());
        ASSERT_TRUE(horizontalDescriptor->getBaseTypeProperty().has_value());
        ASSERT_TRUE(verticalDescriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*menuDescriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::Graphics2D::UI::Widget)));
        EXPECT_EQ(*horizontalDescriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::Graphics2D::UI::Menu)));
        EXPECT_EQ(*verticalDescriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::Graphics2D::UI::Menu)));

        const PropertyDescriptor *items = registry.FindPropertyByName(typeid(HorizontalMenu), "Items");
        const PropertyDescriptor *nullableHover =
            registry.FindPropertyByName(typeid(HorizontalMenu), "HoverIndexCanBeNull");
        const PropertyDescriptor *orientation = registry.FindPropertyByName(typeid(HorizontalMenu), "Orientation");
        const PropertyDescriptor *isOpen = registry.FindPropertyByName(typeid(HorizontalMenu), "IsOpen");
        const PropertyDescriptor *hoverIndex = registry.FindPropertyByName(typeid(HorizontalMenu), "HoverIndex");
        const PropertyDescriptor *selectedIndex = registry.FindPropertyByName(typeid(HorizontalMenu), "SelectedIndex");
        const PropertyDescriptor *horizontal =
            registry.FindPropertyByName(typeid(HorizontalMenu), "HorizontalAlignment");
        const PropertyDescriptor *vertical = registry.FindPropertyByName(typeid(HorizontalMenu), "VerticalAlignment");
        ASSERT_NE(items, nullptr);
        ASSERT_NE(nullableHover, nullptr);
        ASSERT_NE(orientation, nullptr);
        ASSERT_NE(isOpen, nullptr);
        ASSERT_NE(hoverIndex, nullptr);
        ASSERT_NE(selectedIndex, nullptr);
        ASSERT_NE(horizontal, nullptr);
        ASSERT_NE(vertical, nullptr);
        EXPECT_TRUE(items->getMetadataProperty().Content);
        EXPECT_TRUE(std::any_cast<bool>(*nullableHover->getDefaultValueProperty()));
        EXPECT_TRUE(orientation->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(isOpen->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(hoverIndex->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedIndex->getMetadataProperty().XmlIgnore);
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(*horizontal->getDefaultValueProperty()),
                  HorizontalAlignment::Stretch);
        EXPECT_EQ(std::any_cast<VerticalAlignment>(*vertical->getDefaultValueProperty()), VerticalAlignment::Top);
        const PropertyDescriptor *verticalHorizontal =
            registry.FindPropertyByName(typeid(VerticalMenu), "HorizontalAlignment");
        ASSERT_NE(verticalHorizontal, nullptr);
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(*verticalHorizontal->getDefaultValueProperty()),
                  HorizontalAlignment::Left);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "MenuStyle"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "LabelFont"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "LabelColor"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "SelectionHoverBackground"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "SelectionBackground"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(HorizontalMenu), "LabelHorizontalAlignment"), nullptr);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<HorizontalMenu Id=\"main\" HoverIndexCanBeNull=\"False\">"
                         "<MenuItem Id=\"file\" Text=\"&amp;File\"><MenuItem Id=\"new\" Text=\"&amp;New\"/></MenuItem>"
                         "<MenuSeparator/>"
                         "<MenuItem Id=\"edit\" Text=\"&amp;Edit\"/>"
                         "</HorizontalMenu>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(HorizontalMenu));
        auto *menu = static_cast<HorizontalMenu *>(loaded.Value.get());
        EXPECT_EQ(menu->getIdProperty(), std::optional<std::string>("main"));
        EXPECT_FALSE(menu->getHoverIndexCanBeNullProperty());
        EXPECT_EQ(menu->getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(menu->getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(menu->getVerticalAlignmentProperty(), VerticalAlignment::Top);
        ASSERT_EQ(menu->getItemsProperty().getCountProperty(), 3);
        const auto file = std::dynamic_pointer_cast<MenuItem>(menu->getItemsProperty().getItem(0));
        const auto separator = std::dynamic_pointer_cast<MenuSeparator>(menu->getItemsProperty().getItem(1));
        const auto edit = std::dynamic_pointer_cast<MenuItem>(menu->getItemsProperty().getItem(2));
        ASSERT_NE(file, nullptr);
        ASSERT_NE(separator, nullptr);
        ASSERT_NE(edit, nullptr);
        EXPECT_EQ(file->getMenuProperty(), menu);
        EXPECT_EQ(separator->getMenuProperty(), menu);
        EXPECT_EQ(edit->getMenuProperty(), menu);
        EXPECT_EQ(file->getIndexProperty(), 0);
        EXPECT_EQ(separator->getIndexProperty(), 1);
        EXPECT_EQ(edit->getIndexProperty(), 2);
        ASSERT_EQ(file->getItemsProperty().getCountProperty(), 1);
        EXPECT_EQ(file->getItemsProperty().getItem(0)->getIdProperty(), std::optional<std::string>("new"));

        menu->setHoverIndexProperty(1);
        menu->setSelectedIndexProperty(0);
        EXPECT_EQ(menu->getHoverIndexProperty(), std::optional<int>(1));
        EXPECT_TRUE(menu->getIsOpenProperty());
        SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(menu, typeid(HorizontalMenu));
        EXPECT_NE(xml.find("Id=\"main\""), std::string::npos);
        EXPECT_NE(xml.find("HoverIndexCanBeNull=\"False\""), std::string::npos);
        EXPECT_NE(xml.find("Id=\"file\""), std::string::npos);
        EXPECT_NE(xml.find("Id=\"new\""), std::string::npos);
        EXPECT_NE(xml.find("<MenuSeparator"), std::string::npos);
        EXPECT_NE(xml.find("Id=\"edit\""), std::string::npos);
        EXPECT_EQ(xml.find("HorizontalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("Orientation="), std::string::npos);
        EXPECT_EQ(xml.find("IsOpen="), std::string::npos);
        EXPECT_EQ(xml.find("HoverIndex="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedIndex="), std::string::npos);
        EXPECT_EQ(xml.find("VerticalMenu"), std::string::npos);
        EXPECT_EQ(xml.find("MenuStyle="), std::string::npos);
        EXPECT_EQ(xml.find("SelectionBackground="), std::string::npos);
    }
} // namespace
