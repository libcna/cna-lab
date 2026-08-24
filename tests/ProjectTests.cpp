// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Project.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlElement.hpp"

namespace
{
    using Myra::Graphics2D::UI::ExportOptions;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::Project;
    using Myra::Graphics2D::UI::Proportion;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::ScrollViewer;
    using Myra::Graphics2D::UI::VerticalStackPanel;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    TEST(ProjectTests, RegistersProjectMetadataAndPreservesUpstreamDefaults)
    {
        Project project;
        EXPECT_EQ(project.getRootProperty(), nullptr);
        EXPECT_EQ(project.getStylesheetPathProperty(), std::nullopt);
        EXPECT_EQ(project.getDesignerRtfAssetsPathProperty(), std::nullopt);
        EXPECT_EQ(project.getExportOptionsProperty().getNamespaceProperty(), std::nullopt);
        EXPECT_TRUE(project.getObjectsNodesProperty().empty());

        EXPECT_TRUE(Project::IsProportionName("Proportion"));
        EXPECT_TRUE(Project::IsProportionName("Grid.DefaultColumnProportion"));
        EXPECT_FALSE(Project::IsProportionName("Proportional"));

        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* projectType = registry.FindByType(typeid(Project));
        const TypeDescriptor* optionsType = registry.FindByType(typeid(ExportOptions));
        ASSERT_NE(projectType, nullptr);
        ASSERT_NE(optionsType, nullptr);
        EXPECT_TRUE(projectType->getCanCreateProperty());
        EXPECT_TRUE(optionsType->getCanCreateProperty());
        ASSERT_NE(registry.FindPropertyByName(typeid(Project), "Root"), nullptr);
        EXPECT_TRUE(registry.FindPropertyByName(typeid(Project), "Root")
            ->getMetadataProperty().Content);
        ASSERT_TRUE(registry.FindPropertyByName(typeid(Project), "DesignerRtfAssetsPath")
            ->getMetadataProperty().FilePath.has_value());
    }

    TEST(ProjectTests, LoadsOwnsAndRoundTripsTheCurrentProjectShape)
    {
        const std::string xml =
            "<Project DesignerRtfAssetsPath=\"assets\">"
            "<Project.ExportOptions Namespace=\"Example.UI\" Class=\"Screen\" "
            "OutputPath=\"generated\" />"
            "<Grid Id=\"root\" ColumnSpacing=\"3\">"
            "<Panel Id=\"child\" Grid.Column=\"1\" />"
            "</Grid>"
            "</Project>";

        std::shared_ptr<Project> project = Project::LoadFromXml(xml);
        ASSERT_NE(project, nullptr);
        EXPECT_EQ(project->getDesignerRtfAssetsPathProperty(), std::optional<std::string>("assets"));
        EXPECT_EQ(project->getExportOptionsProperty().getNamespaceProperty(),
            std::optional<std::string>("Example.UI"));
        EXPECT_EQ(project->getExportOptionsProperty().getClassProperty(),
            std::optional<std::string>("Screen"));
        ASSERT_NE(project->getRootProperty(), nullptr);
        const auto* grid = dynamic_cast<const Grid*>(project->getRootProperty().get());
        ASSERT_NE(grid, nullptr);
        EXPECT_EQ(grid->getColumnSpacingProperty(), 3);
        ASSERT_EQ(grid->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(Grid::GetColumn(*grid->getWidgetsProperty().front()), 1);

        ASSERT_EQ(project->getObjectsNodesProperty().size(), 4U);
        EXPECT_EQ(project->getObjectsNodesProperty()[0].Object, project.get());
        EXPECT_EQ(project->getObjectsNodesProperty()[0].Type, typeid(Project));
        EXPECT_EQ(project->getObjectsNodesProperty()[0].Node->getNameProperty(), "Project");
        EXPECT_EQ(project->getObjectsNodesProperty()[0].RetainedValue, nullptr);
        EXPECT_EQ(project->getObjectsNodesProperty()[1].Object,
            &project->getExportOptionsProperty());
        EXPECT_EQ(project->getObjectsNodesProperty()[1].Type, typeid(ExportOptions));
        EXPECT_EQ(project->getObjectsNodesProperty()[1].Node->getNameProperty(),
            "Project.ExportOptions");
        EXPECT_EQ(project->getObjectsNodesProperty()[2].Object,
            dynamic_cast<void*>(project->getRootProperty().get()));
        EXPECT_EQ(project->getObjectsNodesProperty()[2].Type, typeid(Grid));
        EXPECT_NE(project->getObjectsNodesProperty()[2].RetainedValue, nullptr);

        const std::string saved = project->ToXml();
        EXPECT_NE(saved.find("<Project DesignerRtfAssetsPath=\"assets\">"), std::string::npos);
        EXPECT_NE(saved.find("<Project.ExportOptions Namespace=\"Example.UI\" Class=\"Screen\" "),
            std::string::npos);
        EXPECT_NE(saved.find("<Grid Id=\"root\" ColumnSpacing=\"3\">"), std::string::npos);
        EXPECT_EQ(saved.find("Grid.DefaultColumnProportion"), std::string::npos);
        EXPECT_EQ(saved.find("Grid.DefaultRowProportion"), std::string::npos);

        const std::shared_ptr<Project> roundTrip = Project::LoadFromXml(saved);
        ASSERT_NE(dynamic_cast<Grid*>(roundTrip->getRootProperty().get()), nullptr);
        EXPECT_EQ(roundTrip->getExportOptionsProperty().getOutputPathProperty(),
            std::optional<std::string>("generated"));

        std::weak_ptr<Project> weakProject = project;
        std::weak_ptr<Widget> weakRoot = project->getRootProperty();
        project.reset();
        EXPECT_TRUE(weakProject.expired());
        EXPECT_TRUE(weakRoot.expired());
    }

    TEST(ProjectTests, SupportsExplicitRegistriesAndLegacyContainerNames)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        const std::shared_ptr<Project> project = Project::LoadFromXml(
            "<Project><VerticalBox Spacing=\"7\"><Panel /></VerticalBox></Project>",
            registry, codecs);
        ASSERT_NE(project->getRootProperty(), nullptr);
        EXPECT_EQ(project->getRootProperty()->getChildrenProperty().size(), 1U);
        const std::string saved = project->ToXml(registry, codecs);
        EXPECT_NE(saved.find("<VerticalStackPanel Spacing=\"7\">"), std::string::npos);
        EXPECT_EQ(saved.find("VerticalBox"), std::string::npos);

        const std::shared_ptr<Project> scrollProject = Project::LoadFromXml(
            "<Project><ScrollPane><Panel Width=\"23\" /></ScrollPane></Project>", registry, codecs);
        const auto *scrollViewer = dynamic_cast<const ScrollViewer *>(scrollProject->getRootProperty().get());
        ASSERT_NE(scrollViewer, nullptr);
        const auto scrollContent = std::dynamic_pointer_cast<Panel>(scrollViewer->getContentProperty());
        ASSERT_NE(scrollContent, nullptr);
        EXPECT_EQ(scrollContent->getWidthProperty(), 23);
        const std::string savedScrollProject = scrollProject->ToXml(registry, codecs);
        EXPECT_NE(savedScrollProject.find("<ScrollViewer>"), std::string::npos);
        EXPECT_EQ(savedScrollProject.find("ScrollPane"), std::string::npos);
    }

    TEST(ProjectTests, RejectsANonProjectDocument)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        EXPECT_THROW(
            static_cast<void>(Project::LoadFromXml("<Panel />", registry, codecs)),
            std::invalid_argument);
    }

    TEST(ProjectTests, RejectsDuplicateNonEmptyObjectIds)
    {
        EXPECT_THROW(static_cast<void>(Project::LoadFromXml(
            "<Project><Grid Id=\"root\"><Panel Id=\"duplicate\" />"
            "<Panel Id=\"duplicate\" /></Grid></Project>")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(Project::LoadFromXml(
            "<Project><Grid Id=\"same\"><Panel Id=\"same\" /></Grid></Project>")),
            std::invalid_argument);
    }

    TEST(ProjectTests, AllowsMissingEmptyAndCaseDistinctObjectIds)
    {
        const std::shared_ptr<Project> project = Project::LoadFromXml(
            "<Project><Grid><Panel /><Panel Id=\"\" /><Panel Id=\"\" />"
            "<Panel Id=\"item\" /><Panel Id=\"Item\" /></Grid></Project>");
        const auto* grid = dynamic_cast<const Grid*>(project->getRootProperty().get());
        ASSERT_NE(grid, nullptr);
        EXPECT_EQ(grid->getWidgetsProperty().size(), 5U);
    }

    TEST(ProjectTests, PropagatesMalformedXmlDiagnostics)
    {
        EXPECT_THROW(static_cast<void>(Project::LoadFromXml("<Project>")), std::exception);
        EXPECT_THROW(static_cast<void>(Project::LoadFromXml("")), std::exception);
    }

    TEST(ProjectTests, LoadsStandaloneLegacyWidgetsAndProportionPropertyElements)
    {
        Myra::MML::LoadedDocument proportion = Project::LoadObjectFromXml(
            "<Grid.DefaultColumnProportion Type=\"Pixels\" Value=\"42\" />");
        EXPECT_EQ(proportion.Root.Type, typeid(Proportion));
        const auto* value = static_cast<const Proportion*>(proportion.Root.Value.get());
        EXPECT_EQ(value->getTypeProperty(), ProportionType::Pixels);
        EXPECT_FLOAT_EQ(value->getValueProperty(), 42.0F);
        ASSERT_EQ(proportion.ObjectsNodes.size(), 1U);
        EXPECT_EQ(proportion.ObjectsNodes.front().Node->getNameProperty(),
            "Grid.DefaultColumnProportion");
        EXPECT_EQ(proportion.ObjectsNodes.front().RetainedValue, proportion.Root.Value);

        Myra::MML::LoadedDocument legacy = Project::LoadObjectFromXml(
            "<VerticalBox Spacing=\"8\"><Panel /></VerticalBox>");
        EXPECT_EQ(legacy.Root.Type, typeid(VerticalStackPanel));
        const auto* stack = static_cast<const VerticalStackPanel*>(legacy.Root.Value.get());
        EXPECT_EQ(stack->getSpacingProperty(), 8);
        ASSERT_EQ(stack->getWidgetsProperty().size(), 1U);
    }

    TEST(ProjectTests, SavesStandaloneScalarAndAttachedStateWithoutOwnedChildren)
    {
        Project project;
        Grid grid;
        auto child = std::make_shared<Panel>();
        child->setIdProperty(std::string("selected"));
        Grid::SetColumn(*child, 2);
        grid.AddWidget(child);

        const std::string childXml = project.SaveObjectToXml(
            child.get(), typeid(Panel), std::string("Chosen"), typeid(Grid));
        EXPECT_NE(childXml.find("<Chosen Id=\"selected\" Grid.Column=\"2\""),
            std::string::npos);

        const std::string gridXml = project.SaveObjectToXml(
            &grid, typeid(Grid), std::string("Grid"));
        EXPECT_EQ(gridXml.find("Panel"), std::string::npos);
        EXPECT_EQ(gridXml.find("DefaultColumnProportion"), std::string::npos);
        EXPECT_EQ(gridXml.find("DefaultRowProportion"), std::string::npos);
    }
}
