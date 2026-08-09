// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Project.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Project.hpp"

#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] std::string ApplyLegacyClassName(const std::string& value)
        {
            if (value == "VerticalBox")
            {
                return "VerticalStackPanel";
            }
            if (value == "HorizontalBox")
            {
                return "HorizontalStackPanel";
            }
            if (value == "TextField")
            {
                return "TextBox";
            }
            if (value == "TextBlock")
            {
                return "Label";
            }
            if (value == "ScrollPane")
            {
                return "ScrollViewer";
            }
            return value;
        }

        void ConfigureLoadContext(MML::LoadContext& context)
        {
            context.LegacyClassNames.emplace("VerticalBox", "VerticalStackPanel");
            context.LegacyClassNames.emplace("HorizontalBox", "HorizontalStackPanel");
            context.LegacyClassNames.emplace("TextField", "TextBox");
            context.LegacyClassNames.emplace("TextBlock", "Label");
            context.LegacyClassNames.emplace("ScrollPane", "ScrollViewer");
        }

        void ConfigureSaveContext(
            MML::SaveContext& context, const MML::TypeRegistry& typeRegistry)
        {
            context.ShouldSerializeProperty = [&context, &typeRegistry](const void* object,
                const std::type_index objectType, const MML::PropertyDescriptor& property) {
                if (context.HasDefaultValue(object, objectType, property))
                {
                    return false;
                }

                const std::string& name = property.getNameProperty();
                if ((name == Project::DefaultColumnProportionName ||
                        name == Project::DefaultRowProportionName) &&
                    typeRegistry.IsTypeOrDerivedFrom(objectType, typeid(Grid)))
                {
                    const auto* grid = static_cast<const Grid*>(
                        typeRegistry.CastObject(object, objectType, typeid(Grid)));
                    return name == Project::DefaultColumnProportionName
                        ? grid->getDefaultColumnProportionProperty() != Proportion::GridDefault
                        : grid->getDefaultRowProportionProperty() != Proportion::GridDefault;
                }
                if (name == Project::DefaultProportionName &&
                    typeRegistry.IsTypeOrDerivedFrom(objectType, typeid(StackPanel)))
                {
                    const auto* stack = static_cast<const StackPanel*>(
                        typeRegistry.CastObject(object, objectType, typeid(StackPanel)));
                    return stack->getDefaultProportionProperty() != Proportion::StackPanelDefault;
                }
                return true;
            };
        }

        void ValidateUniqueObjectIds(
            const MML::LoadedDocument& loadedDocument, const MML::TypeRegistry& typeRegistry)
        {
            std::unordered_map<std::string, const MML::BaseObject*> objectsById;
            for (const MML::LoadedObjectNode& mapping : loadedDocument.ObjectsNodes)
            {
                const MML::TypeDescriptor* descriptor = typeRegistry.FindByType(mapping.Type);
                if (descriptor == nullptr)
                {
                    throw std::logic_error("A loaded Project mapping has an unregistered C++ type.");
                }
                const MML::BaseObject* object = descriptor->GetBaseObject(mapping.Object);
                if (object == nullptr || !object->getIdProperty() || object->getIdProperty()->empty())
                {
                    continue;
                }

                const auto [iterator, inserted] = objectsById.emplace(*object->getIdProperty(), object);
                if (!inserted && iterator->second != object)
                {
                    throw std::invalid_argument(
                        "A Myra project cannot contain duplicate non-empty object Id '" +
                        *object->getIdProperty() + "'.");
                }
            }
        }
    }

    const std::optional<std::string>& ExportOptions::getNamespaceProperty() const noexcept
    {
        return namespace_;
    }

    void ExportOptions::setNamespaceProperty(std::optional<std::string> value)
    {
        namespace_ = std::move(value);
    }

    const std::optional<std::string>& ExportOptions::getClassProperty() const noexcept
    {
        return class_;
    }

    void ExportOptions::setClassProperty(std::optional<std::string> value)
    {
        class_ = std::move(value);
    }

    const std::optional<std::string>& ExportOptions::getOutputPathProperty() const noexcept
    {
        return outputPath_;
    }

    void ExportOptions::setOutputPathProperty(std::optional<std::string> value)
    {
        outputPath_ = std::move(value);
    }

    const std::optional<std::string>& ExportOptions::getTemplateDesignerProperty() const noexcept
    {
        return templateDesigner_;
    }

    void ExportOptions::setTemplateDesignerProperty(std::optional<std::string> value)
    {
        templateDesigner_ = std::move(value);
    }

    const std::optional<std::string>& ExportOptions::getTemplateMainProperty() const noexcept
    {
        return templateMain_;
    }

    void ExportOptions::setTemplateMainProperty(std::optional<std::string> value)
    {
        templateMain_ = std::move(value);
    }

    ExportOptions& Project::getExportOptionsProperty() noexcept
    {
        return exportOptions_;
    }

    const ExportOptions& Project::getExportOptionsProperty() const noexcept
    {
        return exportOptions_;
    }

    const std::shared_ptr<Widget>& Project::getRootProperty() const noexcept
    {
        return root_;
    }

    void Project::setRootProperty(std::shared_ptr<Widget> value) noexcept
    {
        root_ = std::move(value);
    }

    const std::optional<std::string>& Project::getStylesheetPathProperty() const noexcept
    {
        return stylesheetPath_;
    }

    void Project::setStylesheetPathProperty(std::optional<std::string> value)
    {
        stylesheetPath_ = std::move(value);
    }

    const std::optional<std::string>& Project::getDesignerRtfAssetsPathProperty() const noexcept
    {
        return designerRtfAssetsPath_;
    }

    void Project::setDesignerRtfAssetsPathProperty(std::optional<std::string> value)
    {
        designerRtfAssetsPath_ = std::move(value);
    }

    const std::vector<MML::LoadedObjectNode>& Project::getObjectsNodesProperty() const noexcept
    {
        return objectsNodes_;
    }

    bool Project::IsProportionName(const std::string_view value) noexcept
    {
        return value.ends_with(ProportionName) || value.ends_with(DefaultProportionName) ||
            value.ends_with(DefaultColumnProportionName) || value.ends_with(DefaultRowProportionName);
    }

    std::string Project::ToXml(
        const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs) const
    {
        MML::SaveContext context(typeRegistry, valueCodecs);
        ConfigureSaveContext(context, typeRegistry);
        return context.ToXml(this, typeid(Project));
    }

    std::string Project::ToXml() const
    {
        const MML::TypeRegistry typeRegistry = MML::CreateMyraTypeRegistry();
        const MML::ValueCodecRegistry valueCodecs = MML::ValueCodecRegistry::CreateDefault();
        return ToXml(typeRegistry, valueCodecs);
    }

    std::shared_ptr<Project> Project::LoadFromXml(const std::string& xml,
        const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs)
    {
        MML::LoadContext context(typeRegistry, valueCodecs);
        ConfigureLoadContext(context);

        MML::LoadedDocument loadedDocument = context.CreateAndLoadDocument(xml);
        if (loadedDocument.Root.Type != typeid(Project))
        {
            throw std::invalid_argument("A Myra project document must have a Project root element.");
        }
        ValidateUniqueObjectIds(loadedDocument, typeRegistry);
        auto result = std::shared_ptr<Project>(loadedDocument.Root.Value,
            static_cast<Project*>(loadedDocument.Root.Value.get()));
        result->AdoptLoadedDocument(std::move(loadedDocument));
        return result;
    }

    std::shared_ptr<Project> Project::LoadFromXml(const std::string& xml)
    {
        const MML::TypeRegistry typeRegistry = MML::CreateMyraTypeRegistry();
        const MML::ValueCodecRegistry valueCodecs = MML::ValueCodecRegistry::CreateDefault();
        return LoadFromXml(xml, typeRegistry, valueCodecs);
    }

    MML::LoadedDocument Project::LoadObjectFromXml(const std::string& xml,
        const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs)
    {
        auto document = std::make_shared<System::Xml::XmlDocument>();
        document->LoadXml(xml);
        System::Xml::XmlElement* const rootElement = document->getDocumentElementProperty();
        if (rootElement == nullptr)
        {
            throw std::invalid_argument("An MML object document must have a root element.");
        }

        const MML::TypeDescriptor* descriptor = nullptr;
        if (IsProportionName(rootElement->getNameProperty()))
        {
            descriptor = typeRegistry.FindByType(typeid(Proportion));
        }
        else
        {
            descriptor = typeRegistry.FindByXmlName(
                ApplyLegacyClassName(rootElement->getNameProperty()));
        }
        if (descriptor == nullptr)
        {
            throw std::out_of_range("Could not resolve designer object element '" +
                rootElement->getNameProperty() + "' through TypeRegistry.");
        }

        std::shared_ptr<void> object = descriptor->Create();
        MML::LoadContext context(typeRegistry, valueCodecs);
        ConfigureLoadContext(context);
        context.Load(object.get(), descriptor->getTypeProperty(), *rootElement);
        std::vector<MML::LoadedObjectNode> objectsNodes = std::move(context.ObjectsNodes);
        for (MML::LoadedObjectNode& mapping : objectsNodes)
        {
            if (mapping.Object == object.get() && mapping.Type == descriptor->getTypeProperty())
            {
                mapping.RetainedValue = object;
                break;
            }
        }
        return {std::move(document),
            {std::move(object), descriptor->getTypeProperty()}, std::move(objectsNodes)};
    }

    MML::LoadedDocument Project::LoadObjectFromXml(const std::string& xml)
    {
        const MML::TypeRegistry typeRegistry = MML::CreateMyraTypeRegistry();
        const MML::ValueCodecRegistry valueCodecs = MML::ValueCodecRegistry::CreateDefault();
        return LoadObjectFromXml(xml, typeRegistry, valueCodecs);
    }

    std::string Project::SaveObjectToXml(const void* object, const std::type_index objectType,
        const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs,
        std::optional<std::string> tagName, const std::optional<std::type_index> parentType) const
    {
        MML::SaveContext context(typeRegistry, valueCodecs);
        ConfigureSaveContext(context, typeRegistry);
        return context.ToXml(object, objectType, true, std::move(tagName), parentType);
    }

    std::string Project::SaveObjectToXml(const void* object, const std::type_index objectType,
        std::optional<std::string> tagName, const std::optional<std::type_index> parentType) const
    {
        const MML::TypeRegistry typeRegistry = MML::CreateMyraTypeRegistry();
        const MML::ValueCodecRegistry valueCodecs = MML::ValueCodecRegistry::CreateDefault();
        return SaveObjectToXml(object, objectType, typeRegistry, valueCodecs,
            std::move(tagName), parentType);
    }

    void Project::AdoptLoadedDocument(MML::LoadedDocument loadedDocument)
    {
        sourceDocument_ = std::move(loadedDocument.Document);
        objectsNodes_ = std::move(loadedDocument.ObjectsNodes);
        for (MML::LoadedObjectNode& mapping : objectsNodes_)
        {
            if (mapping.Object == this)
            {
                // The externally returned shared_ptr owns the Project. Keeping
                // its root factory handle here would create a self-cycle.
                mapping.RetainedValue.reset();
                break;
            }
        }
    }
}
