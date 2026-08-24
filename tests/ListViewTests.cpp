// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/ListView.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalSeparator;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::ListView;
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    TEST(ListViewTests, WrapsWidgetsAndUpdatesSingleSelectionFromButtonClicks)
    {
        ListView listView;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        int selectedChanges = 0;
        listView.SelectedIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &listView);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectedIndexChanged);
            ++selectedChanges;
        };

        listView.AddWidget(first);
        listView.AddWidget(second);
        ASSERT_EQ(listView.getWidgetsProperty().size(), 2U);
        EXPECT_EQ(listView.getWidgetsProperty()[0], first);
        EXPECT_EQ(listView.getWidgetsProperty()[1], second);

        auto *const firstButton = dynamic_cast<ListViewButton *>(first->getParentProperty());
        auto *const secondButton = dynamic_cast<ListViewButton *>(second->getParentProperty());
        ASSERT_NE(firstButton, nullptr);
        ASSERT_NE(secondButton, nullptr);
        firstButton->DoClick();
        EXPECT_EQ(listView.getSelectedItemProperty(), first);
        EXPECT_EQ(listView.getSelectedIndexProperty(), 0);
        EXPECT_TRUE(firstButton->getIsPressedProperty());

        secondButton->DoClick();
        EXPECT_EQ(listView.getSelectedItemProperty(), second);
        EXPECT_EQ(listView.getSelectedIndexProperty(), 1);
        EXPECT_FALSE(firstButton->getIsPressedProperty());
        EXPECT_TRUE(secondButton->getIsPressedProperty());
        EXPECT_EQ(selectedChanges, 2);

        EXPECT_TRUE(listView.RemoveWidget(second.get()));
        EXPECT_EQ(listView.getSelectedItemProperty(), nullptr);
        EXPECT_FALSE(listView.getSelectedIndexProperty().has_value());
        EXPECT_EQ(selectedChanges, 3);
    }

    TEST(ListViewTests, SupportsOrderedInsertionMultipleModeAndExactTypeCloning)
    {
        ListView source;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        source.AddWidget(first);
        source.InsertWidget(0, second);
        EXPECT_EQ(source.getWidgetsProperty()[0], second);
        EXPECT_EQ(source.getWidgetsProperty()[1], first);

        source.setSelectionModeProperty(SelectionMode::Multiple);
        auto *const secondButton = dynamic_cast<ListViewButton *>(second->getParentProperty());
        ASSERT_NE(secondButton, nullptr);
        secondButton->DoClick();
        EXPECT_EQ(source.getSelectedItemProperty(), nullptr);
        EXPECT_TRUE(secondButton->getIsPressedProperty());

        const auto clone = std::dynamic_pointer_cast<ListView>(source.Clone());
        ASSERT_NE(clone, nullptr);
        ASSERT_EQ(clone->getWidgetsProperty().size(), 2U);
        EXPECT_NE(clone->getWidgetsProperty()[0], second);
        EXPECT_NE(clone->getWidgetsProperty()[1], first);
        EXPECT_EQ(clone->getSelectionModeProperty(), SelectionMode::Multiple);
        EXPECT_EQ(clone->getSelectedItemProperty(), nullptr);
    }

    TEST(ListViewTests, ExposesTheMutableCollectionAdapterContract)
    {
        ListView listView;
        const auto first = std::make_shared<Widget>();
        const auto selected = std::make_shared<Widget>();
        const auto replacement = std::make_shared<Widget>();
        listView.AddWidget(first);
        listView.AddWidget(selected);

        EXPECT_EQ(listView.IndexOfWidget(first.get()), 0);
        EXPECT_EQ(listView.IndexOfWidget(selected.get()), 1);
        EXPECT_EQ(listView.IndexOfWidget(replacement.get()), -1);
        EXPECT_TRUE(listView.ContainsWidget(selected.get()));
        EXPECT_FALSE(listView.ContainsWidget(replacement.get()));
        EXPECT_FALSE(listView.ContainsWidget(nullptr));

        listView.setSelectedItemProperty(selected);
        listView.InsertWidget(0, replacement);
        EXPECT_EQ(listView.getSelectedIndexProperty(), 2);
        auto *selectedButton = dynamic_cast<ListViewButton *>(selected->getParentProperty());
        ASSERT_NE(selectedButton, nullptr);
        EXPECT_TRUE(selectedButton->getIsPressedProperty());

        const auto setItem = std::make_shared<Widget>();
        listView.SetWidget(1, setItem);
        EXPECT_EQ(listView.getWidgetsProperty()[1], setItem);
        EXPECT_EQ(listView.IndexOfWidget(first.get()), -1);
        EXPECT_EQ(first->getParentProperty(), nullptr);
        selectedButton = dynamic_cast<ListViewButton *>(selected->getParentProperty());
        ASSERT_NE(selectedButton, nullptr);
        EXPECT_TRUE(selectedButton->getIsPressedProperty());

        int selectedChanges = 0;
        listView.SelectedIndexChanged += [&](void *, Myra::Events::MyraEventArgs &)
        {
            ++selectedChanges;
            EXPECT_EQ(listView.IndexOfWidget(selected.get()), -1);
            EXPECT_EQ(listView.getWidgetsProperty().size(), 2U);
        };
        listView.RemoveWidgetAt(2);
        EXPECT_EQ(selectedChanges, 1);
        EXPECT_EQ(listView.getSelectedItemProperty(), nullptr);
        EXPECT_EQ(selected->getParentProperty(), nullptr);

        EXPECT_THROW(listView.SetWidget(2, std::make_shared<Widget>()), std::out_of_range);
        EXPECT_THROW(listView.SetWidget(0, nullptr), std::invalid_argument);
        EXPECT_THROW(listView.RemoveWidgetAt(2), std::out_of_range);
    }

    TEST(ListViewTests, KeyboardAndClicksSkipSeparatorsAndCloseOnlyTheActiveDropdown)
    {
        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 120, 90); });
        const auto listView = std::make_shared<ListView>();
        listView->setWidthProperty(60);
        listView->setHeightProperty(50);
        const auto first = std::make_shared<Widget>();
        const auto separator = std::make_shared<HorizontalSeparator>();
        const auto third = std::make_shared<Widget>();
        listView->AddWidget(first);
        listView->AddWidget(separator);
        listView->AddWidget(third);

        desktop.ShowContextMenu(listView, Point(5, 5));
        desktop.UpdateLayout();
        listView->OnKeyDown(Keys::Down);
        EXPECT_EQ(listView->getSelectedItemProperty(), first);
        listView->OnKeyDown(Keys::Down);
        EXPECT_EQ(listView->getSelectedItemProperty(), third);
        listView->OnKeyDown(Keys::Up);
        EXPECT_EQ(listView->getSelectedItemProperty(), first);

        listView->OnKeyDown(Keys::Enter);
        EXPECT_EQ(desktop.getContextMenuProperty(), nullptr);
        desktop.ShowContextMenu(listView, Point(5, 5));
        desktop.UpdateLayout();
        auto *const thirdButton = dynamic_cast<ListViewButton *>(third->getParentProperty());
        ASSERT_NE(thirdButton, nullptr);
        thirdButton->DoClick();
        EXPECT_EQ(listView->getSelectedItemProperty(), third);
        EXPECT_EQ(desktop.getContextMenuProperty(), nullptr);

        listView->OnKeyDown(Keys::Enter);
        EXPECT_EQ(desktop.getContextMenuProperty(), nullptr);
    }

    TEST(ListViewTests, RetainedWrapperBecomesInertAfterListDestruction)
    {
        auto listView = std::make_shared<ListView>();
        const auto item = std::make_shared<Widget>();
        listView->AddWidget(item);
        const auto box = std::dynamic_pointer_cast<Myra::Graphics2D::UI::VerticalStackPanel>(
            listView->getScrollViewerProperty()->getContentProperty());
        ASSERT_NE(box, nullptr);
        const auto retainedButton = std::dynamic_pointer_cast<ListViewButton>(box->getWidgetsProperty().front());
        ASSERT_NE(retainedButton, nullptr);

        listView.reset();
        retainedButton->DoClick();
        EXPECT_TRUE(retainedButton->getIsPressedProperty());
    }

    TEST(ListViewTests, RegistersAndRoundTripsLogicalWidgetsAndSelectionModeMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(ListView));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Widget)));

        const PropertyDescriptor *widgets = registry.FindPropertyByName(typeid(ListView), "Widgets");
        const PropertyDescriptor *selectionMode = registry.FindPropertyByName(typeid(ListView), "SelectionMode");
        const PropertyDescriptor *scrollViewer = registry.FindPropertyByName(typeid(ListView), "ScrollViewer");
        const PropertyDescriptor *selectedIndex = registry.FindPropertyByName(typeid(ListView), "SelectedIndex");
        const PropertyDescriptor *selectedItem = registry.FindPropertyByName(typeid(ListView), "SelectedItem");
        ASSERT_NE(widgets, nullptr);
        ASSERT_NE(selectionMode, nullptr);
        ASSERT_NE(scrollViewer, nullptr);
        ASSERT_NE(selectedIndex, nullptr);
        ASSERT_NE(selectedItem, nullptr);
        EXPECT_TRUE(widgets->getMetadataProperty().Content);
        EXPECT_EQ(std::any_cast<SelectionMode>(*selectionMode->getDefaultValueProperty()), SelectionMode::Single);
        EXPECT_TRUE(scrollViewer->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedIndex->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedItem->getMetadataProperty().XmlIgnore);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<ListView SelectionMode=\"Multiple\"><Panel Width=\"31\"/><HorizontalSeparator/></ListView>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(ListView));
        const auto *listView = static_cast<const ListView *>(loaded.Value.get());
        EXPECT_EQ(listView->getSelectionModeProperty(), SelectionMode::Multiple);
        ASSERT_EQ(listView->getWidgetsProperty().size(), 2U);
        const auto panel = std::dynamic_pointer_cast<Panel>(listView->getWidgetsProperty()[0]);
        ASSERT_NE(panel, nullptr);
        EXPECT_EQ(panel->getWidthProperty(), 31);
        EXPECT_NE(std::dynamic_pointer_cast<HorizontalSeparator>(listView->getWidgetsProperty()[1]), nullptr);
        EXPECT_NE(dynamic_cast<ListViewButton *>(panel->getParentProperty()), nullptr);
        const auto box = std::dynamic_pointer_cast<Myra::Graphics2D::UI::VerticalStackPanel>(
            listView->getScrollViewerProperty()->getContentProperty());
        ASSERT_NE(box, nullptr);
        ASSERT_EQ(box->getWidgetsProperty().size(), 2U);
        EXPECT_NE(std::dynamic_pointer_cast<ListViewButton>(box->getWidgetsProperty()[0]), nullptr);

        SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(listView, typeid(ListView));
        EXPECT_NE(xml.find("SelectionMode=\"Multiple\""), std::string::npos);
        EXPECT_NE(xml.find("<Panel Width=\"31\""), std::string::npos);
        EXPECT_NE(xml.find("<HorizontalSeparator"), std::string::npos);
        EXPECT_EQ(xml.find("ListViewButton"), std::string::npos);
        EXPECT_EQ(xml.find("ScrollViewer="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedIndex="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedItem="), std::string::npos);
    }
} // namespace
