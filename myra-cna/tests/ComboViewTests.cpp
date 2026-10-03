// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/ComboView.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
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
    using Myra::Graphics2D::UI::ComboView;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalSeparator;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::ValueCodecRegistry;

    TEST(ComboViewTests, SelectsTheFirstItemWhenExpandedAndForwardsTheListEvent)
    {
        ComboView comboView;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        int selectedChanges = 0;
        comboView.SelectedIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, comboView.getListViewProperty().get());
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectedIndexChanged);
            ++selectedChanges;
        };

        EXPECT_EQ(comboView.getDropdownMaximumHeightProperty(), 300);
        comboView.AddWidget(first);
        comboView.AddWidget(second);
        comboView.getButtonProperty()->DoClick();
        EXPECT_TRUE(comboView.getIsExpandedProperty());
        EXPECT_EQ(comboView.getSelectedItemProperty(), first);
        EXPECT_EQ(comboView.getSelectedIndexProperty(), 0);
        ASSERT_NE(comboView.getButtonProperty()->getContentProperty(), nullptr);
        EXPECT_NE(comboView.getButtonProperty()->getContentProperty(), first);

        comboView.setSelectedIndexProperty(1);
        EXPECT_EQ(comboView.getSelectedItemProperty(), second);
        EXPECT_EQ(comboView.getSelectedIndexProperty(), 1);
        EXPECT_EQ(selectedChanges, 2);
    }

    TEST(ComboViewTests, DelegatesCollectionAndConfigurationAndClonesItsItems)
    {
        ComboView source;
        const auto child = std::make_shared<Widget>();
        source.AddWidget(child);
        source.setDropdownMaximumHeightProperty(73);
        source.setSelectionModeProperty(SelectionMode::Multiple);

        const auto clone = std::dynamic_pointer_cast<ComboView>(source.Clone());
        ASSERT_NE(clone, nullptr);
        ASSERT_EQ(clone->getWidgetsProperty().size(), 1U);
        EXPECT_NE(clone->getWidgetsProperty().front(), child);
        EXPECT_EQ(clone->getDropdownMaximumHeightProperty(), 73);
        EXPECT_EQ(clone->getSelectionModeProperty(), SelectionMode::Multiple);
        EXPECT_TRUE(clone->RemoveWidget(clone->getWidgetsProperty().front().get()));
        EXPECT_TRUE(clone->getWidgetsProperty().empty());
    }

    TEST(ComboViewTests, ShowsDropdownBelowTheControlAndClosesThroughKeyboardAndClicks)
    {
        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 200, 120); });
        auto comboView = std::make_shared<ComboView>();
        comboView->setWidthProperty(60);
        comboView->setHeightProperty(20);
        comboView->setLeftProperty(10);
        comboView->setTopProperty(15);
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        comboView->AddWidget(first);
        comboView->AddWidget(second);
        desktop.AddWidget(comboView);
        desktop.UpdateLayout();
        comboView->SetKeyboardFocus();

        comboView->getButtonProperty()->DoClick();
        const auto listView = comboView->getListViewProperty();
        EXPECT_EQ(desktop.getContextMenuProperty(), listView);
        EXPECT_TRUE(comboView->getIsExpandedProperty());
        EXPECT_EQ(listView->getLeftProperty(), 10);
        EXPECT_EQ(listView->getTopProperty(), 35);
        EXPECT_EQ(listView->getWidthProperty(), 60);
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), listView.get());

        desktop.OnKeyDown(Keys::Down);
        EXPECT_EQ(comboView->getSelectedItemProperty(), second);
        desktop.OnKeyDown(Keys::Enter);
        EXPECT_EQ(desktop.getContextMenuProperty(), nullptr);
        EXPECT_FALSE(comboView->getIsExpandedProperty());
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), comboView.get());

        comboView->getButtonProperty()->DoClick();
        desktop.UpdateLayout();
        auto *const firstButton = dynamic_cast<Myra::Graphics2D::UI::ListViewButton *>(first->getParentProperty());
        ASSERT_NE(firstButton, nullptr);
        firstButton->DoClick();
        EXPECT_EQ(comboView->getSelectedItemProperty(), first);
        EXPECT_EQ(desktop.getContextMenuProperty(), nullptr);
        EXPECT_FALSE(comboView->getIsExpandedProperty());
    }

    TEST(ComboViewTests, ReentrantSelectionRemovalInvalidatesDetachedChildCallbacks)
    {
        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 160, 100); });
        auto comboView = std::make_shared<ComboView>();
        comboView->setWidthProperty(60);
        comboView->setHeightProperty(20);
        comboView->AddWidget(std::make_shared<Widget>());
        comboView->AddWidget(std::make_shared<Widget>());
        comboView->setSelectedIndexProperty(0);
        const auto listView = comboView->getListViewProperty();
        const auto retainedButton = comboView->getButtonProperty();
        desktop.AddWidget(comboView);
        desktop.UpdateLayout();
        retainedButton->DoClick();
        ASSERT_EQ(desktop.getContextMenuProperty(), listView);

        const std::weak_ptr<ComboView> weakCombo = comboView;
        int selectionCalls = 0;
        comboView->SelectedIndexChanged += [&](void *, Myra::Events::MyraEventArgs &)
        {
            ++selectionCalls;
            if (!comboView)
            {
                return;
            }
            EXPECT_TRUE(desktop.RemoveWidget(comboView.get()));
            static_cast<void>(desktop.getChildrenCopyProperty());
            EXPECT_EQ(comboView.use_count(), 1);
            comboView.reset();
        };
        desktop.OnKeyDown(Keys::Down);
        EXPECT_EQ(comboView, nullptr);
        EXPECT_TRUE(weakCombo.expired());
        EXPECT_EQ(selectionCalls, 1);
        EXPECT_EQ(listView->getSelectedIndexProperty(), 1);
        EXPECT_EQ(desktop.getContextMenuProperty(), listView);

        desktop.HideContextMenu();
        listView->setSelectedIndexProperty(0);
        retainedButton->DoClick();
        EXPECT_EQ(selectionCalls, 1);
    }

    TEST(ComboViewTests, RegistersAndRoundTripsLogicalWidgetsAndConfigurationMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(ComboView));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Widget)));

        const PropertyDescriptor *widgets = registry.FindPropertyByName(typeid(ComboView), "Widgets");
        const PropertyDescriptor *maximumHeight =
            registry.FindPropertyByName(typeid(ComboView), "DropdownMaximumHeight");
        const PropertyDescriptor *selectionMode = registry.FindPropertyByName(typeid(ComboView), "SelectionMode");
        const PropertyDescriptor *expanded = registry.FindPropertyByName(typeid(ComboView), "IsExpanded");
        const PropertyDescriptor *listView = registry.FindPropertyByName(typeid(ComboView), "ListView");
        const PropertyDescriptor *selectedIndex = registry.FindPropertyByName(typeid(ComboView), "SelectedIndex");
        const PropertyDescriptor *selectedItem = registry.FindPropertyByName(typeid(ComboView), "SelectedItem");
        ASSERT_NE(widgets, nullptr);
        ASSERT_NE(maximumHeight, nullptr);
        ASSERT_NE(selectionMode, nullptr);
        ASSERT_NE(expanded, nullptr);
        ASSERT_NE(listView, nullptr);
        ASSERT_NE(selectedIndex, nullptr);
        ASSERT_NE(selectedItem, nullptr);
        EXPECT_TRUE(widgets->getMetadataProperty().Content);
        EXPECT_EQ(std::any_cast<std::optional<int>>(*maximumHeight->getDefaultValueProperty()), 300);
        EXPECT_EQ(std::any_cast<SelectionMode>(*selectionMode->getDefaultValueProperty()), SelectionMode::Single);
        EXPECT_TRUE(expanded->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(listView->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedIndex->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(selectedItem->getMetadataProperty().XmlIgnore);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<ComboView DropdownMaximumHeight=\"87\" SelectionMode=\"Multiple\">"
                         "<Panel Height=\"29\"/><HorizontalSeparator/></ComboView>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        ASSERT_EQ(loaded.Type, typeid(ComboView));
        const auto *comboView = static_cast<const ComboView *>(loaded.Value.get());
        EXPECT_EQ(comboView->getDropdownMaximumHeightProperty(), 87);
        EXPECT_EQ(comboView->getSelectionModeProperty(), SelectionMode::Multiple);
        ASSERT_EQ(comboView->getWidgetsProperty().size(), 2U);
        const auto panel = std::dynamic_pointer_cast<Panel>(comboView->getWidgetsProperty()[0]);
        ASSERT_NE(panel, nullptr);
        EXPECT_EQ(panel->getHeightProperty(), 29);
        EXPECT_NE(std::dynamic_pointer_cast<HorizontalSeparator>(comboView->getWidgetsProperty()[1]), nullptr);
        EXPECT_NE(dynamic_cast<ListViewButton *>(panel->getParentProperty()), nullptr);

        SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(comboView, typeid(ComboView));
        EXPECT_NE(xml.find("DropdownMaximumHeight=\"87\""), std::string::npos);
        EXPECT_NE(xml.find("SelectionMode=\"Multiple\""), std::string::npos);
        EXPECT_NE(xml.find("<Panel Height=\"29\""), std::string::npos);
        EXPECT_NE(xml.find("<HorizontalSeparator"), std::string::npos);
        EXPECT_EQ(xml.find("ListViewButton"), std::string::npos);
        EXPECT_EQ(xml.find("IsExpanded="), std::string::npos);
        EXPECT_EQ(xml.find("ListView="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedIndex="), std::string::npos);
        EXPECT_EQ(xml.find("SelectedItem="), std::string::npos);
    }
} // namespace
