// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/RegisterMyraTypes.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/ContentControl.hpp"
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

namespace
{
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::Button;
    using Myra::Graphics2D::UI::ButtonBase;
    using Myra::Graphics2D::UI::CheckButtonBase;
    using Myra::Graphics2D::UI::Container;
    using Myra::Graphics2D::UI::ContentControl;
    using Myra::Graphics2D::UI::DragDirection;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalStackPanel;
    using Myra::Graphics2D::UI::MouseCursorType;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::Proportion;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::StackPanel;
    using Myra::Graphics2D::UI::ToggleButton;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalStackPanel;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::AttachedPropertiesRegistry;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(MyraTypeRegistryTests, RegistersTheCurrentHierarchyMetadataAndAttachedProperties)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();

        const TypeDescriptor* baseObject = registry.FindByType(typeid(Myra::MML::BaseObject));
        const TypeDescriptor* widget = registry.FindByType(typeid(Widget));
        const TypeDescriptor* contentControl = registry.FindByType(typeid(ContentControl));
        const TypeDescriptor* buttonBase = registry.FindByType(typeid(ButtonBase));
        const TypeDescriptor* button = registry.FindByType(typeid(Button));
        const TypeDescriptor* checkButtonBase = registry.FindByType(typeid(CheckButtonBase));
        const TypeDescriptor* toggleButton = registry.FindByType(typeid(ToggleButton));
        const TypeDescriptor* container = registry.FindByType(typeid(Container));
        const TypeDescriptor* grid = registry.FindByType(typeid(Grid));
        const TypeDescriptor* stack = registry.FindByType(typeid(StackPanel));
        ASSERT_NE(baseObject, nullptr);
        ASSERT_NE(widget, nullptr);
        ASSERT_NE(contentControl, nullptr);
        ASSERT_NE(buttonBase, nullptr);
        ASSERT_NE(button, nullptr);
        ASSERT_NE(checkButtonBase, nullptr);
        ASSERT_NE(toggleButton, nullptr);
        ASSERT_NE(container, nullptr);
        ASSERT_NE(grid, nullptr);
        ASSERT_NE(stack, nullptr);
        EXPECT_TRUE(baseObject->getCanCreateProperty());
        EXPECT_TRUE(widget->getCanCreateProperty());
        EXPECT_FALSE(contentControl->getCanCreateProperty());
        EXPECT_FALSE(buttonBase->getCanCreateProperty());
        EXPECT_TRUE(button->getCanCreateProperty());
        EXPECT_FALSE(checkButtonBase->getCanCreateProperty());
        EXPECT_TRUE(toggleButton->getCanCreateProperty());
        EXPECT_FALSE(container->getCanCreateProperty());
        EXPECT_TRUE(grid->getCanCreateProperty());
        EXPECT_FALSE(stack->getCanCreateProperty());
        EXPECT_TRUE(registry.FindByType(typeid(HorizontalStackPanel))->getCanCreateProperty());
        EXPECT_TRUE(registry.FindByType(typeid(VerticalStackPanel))->getCanCreateProperty());

        const PropertyDescriptor* content =
            registry.FindPropertyByName(typeid(ContentControl), "Content");
        ASSERT_NE(content, nullptr);
        EXPECT_TRUE(content->getMetadataProperty().Content);
        ASSERT_TRUE(content->getComplexAdapterProperty().has_value());
        EXPECT_EQ(content->getComplexAdapterProperty()->getItemTypeProperty(), typeid(Widget));

        const PropertyDescriptor* horizontal =
            registry.FindPropertyByName(typeid(Grid), "HorizontalAlignment");
        const PropertyDescriptor* vertical =
            registry.FindPropertyByName(typeid(Grid), "VerticalAlignment");
        ASSERT_NE(horizontal, nullptr);
        ASSERT_NE(vertical, nullptr);
        ASSERT_TRUE(horizontal->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*horizontal->getDeclaringTypeProperty(), typeid(Container));
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(*horizontal->getDefaultValueProperty()),
            HorizontalAlignment::Stretch);
        EXPECT_EQ(std::any_cast<VerticalAlignment>(*vertical->getDefaultValueProperty()),
            VerticalAlignment::Stretch);

        const PropertyDescriptor* widgets = registry.FindPropertyByName(typeid(Grid), "Widgets");
        ASSERT_NE(widgets, nullptr);
        EXPECT_TRUE(widgets->getMetadataProperty().Content);
        ASSERT_TRUE(widgets->getComplexAdapterProperty().has_value());
        EXPECT_EQ(widgets->getComplexAdapterProperty()->getItemTypeProperty(), typeid(Widget));

        EXPECT_EQ(AttachedPropertiesRegistry::GetPropertiesOfType(typeid(Grid), registry).size(), 4U);
        EXPECT_EQ(AttachedPropertiesRegistry::GetPropertiesOfType(typeid(StackPanel), registry).size(), 2U);
    }

    TEST(MyraTypeRegistryTests, RoundTripsActualGridStackPanelProportionsAndAttachedProperties)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);

        System::Xml::XmlDocument document;
        document.LoadXml(
            "<Grid Id=\"root\" StyleName=\"accent\" Width=\"120\" Height=\"80\" ColumnSpacing=\"2\" "
            "DragDirection=\"Both\" MouseCursor=\"Hand\" Tooltip=\"help\" ClipToBounds=\"True\">"
            "<Grid.ColumnsProportions>"
            "<Proportion Type=\"Pixels\" Value=\"20\" />"
            "<Proportion Type=\"Part\" Value=\"2\" />"
            "</Grid.ColumnsProportions>"
            "<Grid.RowsProportions><Proportion Type=\"Auto\" /></Grid.RowsProportions>"
            "<Panel Id=\"first\" Grid.Column=\"1\" Margin=\"1, 2, 3, 4\" />"
            "<HorizontalStackPanel Id=\"stack\" Grid.Row=\"1\" Spacing=\"3\">"
            "<Panel Id=\"nested\" StackPanel.ProportionType=\"Fill\" "
            "StackPanel.ProportionValue=\"2.5\" />"
            "</HorizontalStackPanel>"
            "</Grid>");

        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        const auto* grid = static_cast<const Grid*>(loaded.Value.get());
        ASSERT_TRUE(grid->getIdProperty().has_value());
        EXPECT_EQ(*grid->getIdProperty(), "root");
        EXPECT_EQ(grid->getStyleNameProperty(), std::optional<std::string>("accent"));
        EXPECT_EQ(grid->getWidthProperty(), std::optional<int>(120));
        EXPECT_EQ(grid->getHeightProperty(), std::optional<int>(80));
        EXPECT_EQ(grid->getColumnSpacingProperty(), 2);
        EXPECT_EQ(grid->getDragDirectionProperty(), DragDirection::Both);
        EXPECT_EQ(grid->getMouseCursorProperty(), MouseCursorType::Hand);
        ASSERT_TRUE(grid->getTooltipProperty().has_value());
        EXPECT_EQ(*grid->getTooltipProperty(), "help");
        EXPECT_TRUE(grid->getClipToBoundsProperty());
        ASSERT_EQ(grid->getColumnsProportionsProperty().getCountProperty(), 2);
        EXPECT_EQ(grid->getColumnsProportionsProperty()[0]->getTypeProperty(), ProportionType::Pixels);
        EXPECT_FLOAT_EQ(grid->getColumnsProportionsProperty()[0]->getValueProperty(), 20.0F);
        EXPECT_EQ(grid->getColumnsProportionsProperty()[1]->getTypeProperty(), ProportionType::Part);
        EXPECT_FLOAT_EQ(grid->getColumnsProportionsProperty()[1]->getValueProperty(), 2.0F);
        ASSERT_EQ(grid->getWidgetsProperty().size(), 2U);

        const auto* first = dynamic_cast<const Panel*>(grid->getWidgetsProperty()[0].get());
        ASSERT_NE(first, nullptr);
        EXPECT_EQ(Grid::GetColumn(*first), 1);
        EXPECT_EQ(first->getMarginProperty(), Thickness(1, 2, 3, 4));
        const auto* stack = dynamic_cast<const HorizontalStackPanel*>(grid->getWidgetsProperty()[1].get());
        ASSERT_NE(stack, nullptr);
        EXPECT_EQ(Grid::GetRow(*stack), 1);
        EXPECT_EQ(stack->getSpacingProperty(), 3);
        ASSERT_EQ(stack->getWidgetsProperty().size(), 1U);
        const Widget& nested = *stack->getWidgetsProperty().front();
        EXPECT_EQ(StackPanel::GetProportionType(nested), ProportionType::Fill);
        EXPECT_FLOAT_EQ(StackPanel::GetProportionValue(nested), 2.5F);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(grid, typeid(Grid));
        System::Xml::XmlDocument savedDocument;
        savedDocument.LoadXml(xml);
        const System::Xml::XmlElement* savedRoot = savedDocument.getDocumentElementProperty();
        ASSERT_NE(savedRoot, nullptr);
        EXPECT_FALSE(savedRoot->HasAttribute("HorizontalAlignment"));
        EXPECT_FALSE(savedRoot->HasAttribute("VerticalAlignment"));
        EXPECT_EQ(savedRoot->GetAttribute("DragDirection"), "Both");
        EXPECT_EQ(savedRoot->GetAttribute("StyleName"), "accent");
        EXPECT_EQ(savedRoot->GetAttribute("MouseCursor"), "Hand");
        EXPECT_TRUE(savedRoot->HasAttribute("Tooltip"));
        EXPECT_EQ(savedRoot->GetAttribute("Tooltip"), "help");
        EXPECT_EQ(savedRoot->GetAttribute("ClipToBounds"), "True");
        EXPECT_NE(xml.find("Grid.Column=\"1\""), std::string::npos);
        EXPECT_NE(xml.find("Grid.Row=\"1\""), std::string::npos);
        EXPECT_NE(xml.find("StackPanel.ProportionType=\"Fill\""), std::string::npos);
        EXPECT_NE(xml.find("Grid.ColumnsProportions"), std::string::npos);

        System::Xml::XmlDocument roundTripDocument;
        roundTripDocument.LoadXml(xml);
        const Myra::MML::LoadedObject roundTrip =
            loader.CreateAndLoad(*roundTripDocument.getDocumentElementProperty());
        const auto* roundTripGrid = static_cast<const Grid*>(roundTrip.Value.get());
        EXPECT_EQ(roundTripGrid->getDragDirectionProperty(), DragDirection::Both);
        EXPECT_EQ(roundTripGrid->getStyleNameProperty(), std::optional<std::string>("accent"));
        EXPECT_EQ(roundTripGrid->getMouseCursorProperty(), MouseCursorType::Hand);
        ASSERT_TRUE(roundTripGrid->getTooltipProperty().has_value());
        EXPECT_EQ(*roundTripGrid->getTooltipProperty(), "help");
        EXPECT_TRUE(roundTripGrid->getClipToBoundsProperty());
        ASSERT_EQ(roundTripGrid->getWidgetsProperty().size(), 2U);
        EXPECT_EQ(roundTripGrid->getColumnsProportionsProperty().getCountProperty(), 2);
        const auto* roundTripStack = dynamic_cast<const HorizontalStackPanel*>(
            roundTripGrid->getWidgetsProperty()[1].get());
        ASSERT_NE(roundTripStack, nullptr);
        ASSERT_EQ(roundTripStack->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(StackPanel::GetProportionType(*roundTripStack->getWidgetsProperty().front()),
            ProportionType::Fill);
        EXPECT_FLOAT_EQ(StackPanel::GetProportionValue(*roundTripStack->getWidgetsProperty().front()), 2.5F);
    }
}
