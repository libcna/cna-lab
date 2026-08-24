// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/TabControl.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Myra::Graphics2D::UI::Button;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::HorizontalSeparator;
    using Myra::Graphics2D::UI::HorizontalStackPanel;
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::TabControl;
    using Myra::Graphics2D::UI::TabItem;
    using Myra::Graphics2D::UI::TabSelectorPosition;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    TEST(TabControlTests, SelectsTheFirstItemThenUpdatesContentFromButtonClicks)
    {
        TabControl tabs;
        const auto firstContent = std::make_shared<Widget>();
        const auto secondContent = std::make_shared<Widget>();
        const auto first = std::make_shared<TabItem>(std::string("One"), firstContent);
        const auto second = std::make_shared<TabItem>(std::string("Two"), secondContent);
        tabs.getItemsProperty().Add(first);
        tabs.getItemsProperty().Add(second);

        EXPECT_EQ(tabs.getSelectedItemProperty(), first);
        EXPECT_TRUE(first->getIsSelectedProperty());
        ASSERT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 2U);
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], firstContent);

        const auto secondButton =
            std::dynamic_pointer_cast<ListViewButton>(tabs.getButtonsGridProperty()->getWidgetsProperty()[1]);
        ASSERT_NE(secondButton, nullptr);
        secondButton->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        EXPECT_FALSE(first->getIsSelectedProperty());
        EXPECT_TRUE(second->getIsSelectedProperty());
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], secondContent);

        const auto replacementContent = std::make_shared<Widget>();
        second->setContentProperty(replacementContent);
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], replacementContent);
    }

    TEST(TabControlTests, RepositionsSelectorButtonsForEverySupportedPlacement)
    {
        TabControl tabs;
        const auto layout = std::dynamic_pointer_cast<Grid>(tabs.getChildrenProperty()[0]);
        ASSERT_NE(layout, nullptr);

        EXPECT_EQ(Grid::GetRow(*tabs.getButtonsGridProperty()), 0);
        EXPECT_EQ(Grid::GetRow(*tabs.getContentPanelProperty()), 1);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Right);
        EXPECT_EQ(Grid::GetColumn(*tabs.getButtonsGridProperty()), 1);
        EXPECT_EQ(Grid::GetColumn(*tabs.getContentPanelProperty()), 0);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Bottom);
        EXPECT_EQ(Grid::GetRow(*tabs.getButtonsGridProperty()), 1);
        EXPECT_EQ(Grid::GetRow(*tabs.getContentPanelProperty()), 0);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Left);
        EXPECT_EQ(Grid::GetColumn(*tabs.getButtonsGridProperty()), 0);
        EXPECT_EQ(Grid::GetColumn(*tabs.getContentPanelProperty()), 1);
    }

    TEST(TabControlTests, RemovesItemsAndClonesThePortedItemAndLayoutState)
    {
        TabControl source;
        const auto first = std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>());
        const auto second = std::make_shared<TabItem>(std::string("Two"), std::make_shared<Widget>());
        source.getItemsProperty().Add(first);
        source.getItemsProperty().Add(second);
        source.setTabSelectorPositionProperty(TabSelectorPosition::Left);
        source.getItemsProperty().Remove(first);
        EXPECT_EQ(source.getSelectedItemProperty(), nullptr);
        ASSERT_EQ(source.getButtonsGridProperty()->getWidgetsProperty().size(), 1U);

        const auto clone = std::dynamic_pointer_cast<TabControl>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getTabSelectorPositionProperty(), TabSelectorPosition::Left);
        ASSERT_EQ(clone->getItemsProperty().getCountProperty(), 1);
        EXPECT_NE(clone->getItemsProperty().getItem(0), second);
        EXPECT_EQ(clone->getItemsProperty().getItem(0)->getTextProperty(), second->getTextProperty());
        EXPECT_EQ(clone->getSelectedIndexProperty(), 0);
    }

    TEST(TabControlTests, CloseableHeadersSelectAndRemoveTheirExactItems)
    {
        TabControl tabs;
        tabs.setCloseableTabsProperty(true);
        const auto first = std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>());
        const auto second = std::make_shared<TabItem>(std::string("Two"), std::make_shared<Widget>());
        tabs.getItemsProperty().Add(first);
        const auto firstHeaderBeforeRebuild =
            std::dynamic_pointer_cast<HorizontalStackPanel>(tabs.getButtonsGridProperty()->getWidgetsProperty()[0]);
        ASSERT_NE(firstHeaderBeforeRebuild, nullptr);
        const auto detachedFirstSelector =
            std::dynamic_pointer_cast<ListViewButton>(firstHeaderBeforeRebuild->getWidgetsProperty()[0]);
        ASSERT_NE(detachedFirstSelector, nullptr);
        tabs.getItemsProperty().Add(second);

        ASSERT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 2U);
        const auto secondHeader =
            std::dynamic_pointer_cast<HorizontalStackPanel>(tabs.getButtonsGridProperty()->getWidgetsProperty()[1]);
        ASSERT_NE(secondHeader, nullptr);
        ASSERT_EQ(secondHeader->getWidgetsProperty().size(), 2U);
        const auto secondSelector = std::dynamic_pointer_cast<ListViewButton>(secondHeader->getWidgetsProperty()[0]);
        const auto secondClose = std::dynamic_pointer_cast<Button>(secondHeader->getWidgetsProperty()[1]);
        ASSERT_NE(secondSelector, nullptr);
        ASSERT_NE(secondClose, nullptr);

        secondSelector->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        EXPECT_EQ(detachedFirstSelector->getButtonsContainerProperty(), nullptr);
        detachedFirstSelector->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        secondClose->DoClick();
        ASSERT_EQ(tabs.getItemsProperty().getCountProperty(), 1);
        EXPECT_EQ(tabs.getItemsProperty().getItem(0), first);
        EXPECT_EQ(tabs.getSelectedItemProperty(), nullptr);
        EXPECT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 1U);
    }

    TEST(TabControlTests, ClonePreservesCloseableStructureAndRetainedButtonsBecomeInert)
    {
        std::shared_ptr<ListViewButton> retainedSelector;
        std::shared_ptr<Button> retainedClose;
        std::weak_ptr<TabControl> releasedTabs;
        {
            auto tabs = std::make_shared<TabControl>();
            tabs->setCloseableTabsProperty(true);
            tabs->getItemsProperty().Add(std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>()));

            const auto clone = std::dynamic_pointer_cast<TabControl>(tabs->Clone());
            ASSERT_NE(clone, nullptr);
            EXPECT_TRUE(clone->getCloseableTabsProperty());
            ASSERT_EQ(clone->getButtonsGridProperty()->getWidgetsProperty().size(), 1U);
            EXPECT_NE(std::dynamic_pointer_cast<HorizontalStackPanel>(
                          clone->getButtonsGridProperty()->getWidgetsProperty()[0]),
                      nullptr);

            const auto header = std::dynamic_pointer_cast<HorizontalStackPanel>(
                tabs->getButtonsGridProperty()->getWidgetsProperty()[0]);
            ASSERT_NE(header, nullptr);
            retainedSelector = std::dynamic_pointer_cast<ListViewButton>(header->getWidgetsProperty()[0]);
            retainedClose = std::dynamic_pointer_cast<Button>(header->getWidgetsProperty()[1]);
            ASSERT_NE(retainedSelector, nullptr);
            ASSERT_NE(retainedClose, nullptr);
            releasedTabs = tabs;
        }

        EXPECT_TRUE(releasedTabs.expired());
        EXPECT_EQ(retainedSelector->getButtonsContainerProperty(), nullptr);
        retainedSelector->DoClick();
        retainedSelector->setIsPressedProperty(true);
        retainedClose->DoClick();
    }

    TEST(TabControlTests, RegistersAndRoundTripsLogicalTabItemsAndConfigurationMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(TabControl));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Widget)));

        const PropertyDescriptor *items = registry.FindPropertyByName(typeid(TabControl), "Items");
        const PropertyDescriptor *selectionMode = registry.FindPropertyByName(typeid(TabControl), "SelectionMode");
        const PropertyDescriptor *selectedIndex = registry.FindPropertyByName(typeid(TabControl), "SelectedIndex");
        const PropertyDescriptor *selectedItem = registry.FindPropertyByName(typeid(TabControl), "SelectedItem");
        const PropertyDescriptor *horizontal = registry.FindPropertyByName(typeid(TabControl), "HorizontalAlignment");
        const PropertyDescriptor *vertical = registry.FindPropertyByName(typeid(TabControl), "VerticalAlignment");
        const PropertyDescriptor *position = registry.FindPropertyByName(typeid(TabControl), "TabSelectorPosition");
        const PropertyDescriptor *closeable = registry.FindPropertyByName(typeid(TabControl), "CloseableTabs");
        const PropertyDescriptor *clip = registry.FindPropertyByName(typeid(TabControl), "ClipToBounds");
        ASSERT_NE(items, nullptr);
        ASSERT_NE(selectionMode, nullptr);
        ASSERT_NE(selectedIndex, nullptr);
        ASSERT_NE(selectedItem, nullptr);
        ASSERT_NE(horizontal, nullptr);
        ASSERT_NE(vertical, nullptr);
        ASSERT_NE(position, nullptr);
        ASSERT_NE(closeable, nullptr);
        ASSERT_NE(clip, nullptr);
        EXPECT_TRUE(items->getMetadataProperty().Content);
        ASSERT_TRUE(items->getComplexAdapterProperty().has_value());
        EXPECT_EQ(items->getComplexAdapterProperty()->getItemTypeProperty(), std::type_index(typeid(TabItem)));
        EXPECT_TRUE(selectionMode->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedIndex->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedItem->getMetadataProperty().XmlIgnore);
        EXPECT_EQ(std::any_cast<Myra::Graphics2D::UI::HorizontalAlignment>(*horizontal->getDefaultValueProperty()),
                  Myra::Graphics2D::UI::HorizontalAlignment::Left);
        EXPECT_EQ(std::any_cast<Myra::Graphics2D::UI::VerticalAlignment>(*vertical->getDefaultValueProperty()),
                  Myra::Graphics2D::UI::VerticalAlignment::Top);
        EXPECT_EQ(std::any_cast<TabSelectorPosition>(*position->getDefaultValueProperty()), TabSelectorPosition::Top);
        EXPECT_FALSE(std::any_cast<bool>(*closeable->getDefaultValueProperty()));
        EXPECT_TRUE(std::any_cast<bool>(*clip->getDefaultValueProperty()));
        EXPECT_EQ(registry.FindPropertyByName(typeid(TabControl), "TabControlStyle"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(TabControl), "ButtonsGrid"), nullptr);
        EXPECT_EQ(registry.FindPropertyByName(typeid(TabControl), "ContentPanel"), nullptr);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        EXPECT_EQ(codecs.Serialize(std::any(TabSelectorPosition::Right)), "Right");
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<TabControl Id=\"tabs\" TabSelectorPosition=\"Left\" CloseableTabs=\"True\">"
                         "<TabItem Id=\"first\" Text=\"One\" Height=\"23\"><Panel Width=\"41\"/></TabItem>"
                         "<TabItem Text=\"Two\"><HorizontalSeparator Thickness=\"3\"/></TabItem>"
                         "</TabControl>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(TabControl));
        const auto *tabs = static_cast<const TabControl *>(loaded.Value.get());
        EXPECT_EQ(tabs->getIdProperty(), std::optional<std::string>("tabs"));
        EXPECT_EQ(tabs->getTabSelectorPositionProperty(), TabSelectorPosition::Left);
        EXPECT_TRUE(tabs->getCloseableTabsProperty());
        EXPECT_TRUE(tabs->getClipToBoundsProperty());
        EXPECT_EQ(tabs->getHorizontalAlignmentProperty(), Myra::Graphics2D::UI::HorizontalAlignment::Left);
        EXPECT_EQ(tabs->getVerticalAlignmentProperty(), Myra::Graphics2D::UI::VerticalAlignment::Top);
        ASSERT_EQ(tabs->getItemsProperty().getCountProperty(), 2);
        const std::shared_ptr<TabItem> first = tabs->getItemsProperty().getItem(0);
        const std::shared_ptr<TabItem> second = tabs->getItemsProperty().getItem(1);
        EXPECT_EQ(tabs->getSelectedItemProperty(), first);
        EXPECT_EQ(first->getIdProperty(), std::optional<std::string>("first"));
        EXPECT_EQ(first->getTextProperty(), std::optional<std::string>("One"));
        EXPECT_EQ(first->getHeightProperty(), std::optional<int>(23));
        const auto firstPanel = std::dynamic_pointer_cast<Panel>(first->getContentProperty());
        ASSERT_NE(firstPanel, nullptr);
        EXPECT_EQ(firstPanel->getWidthProperty(), std::optional<int>(41));
        EXPECT_NE(std::dynamic_pointer_cast<HorizontalSeparator>(second->getContentProperty()), nullptr);
        EXPECT_EQ(tabs->getButtonsGridProperty()->getWidgetsProperty().size(), 2U);

        SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(tabs, typeid(TabControl));
        EXPECT_NE(xml.find("Id=\"tabs\""), std::string::npos);
        EXPECT_NE(xml.find("TabSelectorPosition=\"Left\""), std::string::npos);
        EXPECT_NE(xml.find("CloseableTabs=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("<TabItem Id=\"first\" Text=\"One\" Height=\"23\""), std::string::npos);
        EXPECT_NE(xml.find("<Panel Width=\"41\""), std::string::npos);
        EXPECT_NE(xml.find("<HorizontalSeparator Thickness=\"3\""), std::string::npos);
        EXPECT_EQ(xml.find("HorizontalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("ClipToBounds="), std::string::npos);
        EXPECT_EQ(xml.find("SelectionMode="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedIndex="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedItem="), std::string::npos);
        EXPECT_EQ(xml.find("ListViewButton"), std::string::npos);
        EXPECT_EQ(xml.find("ButtonsGrid"), std::string::npos);
        EXPECT_EQ(xml.find("ContentPanel"), std::string::npos);
    }
} // namespace
