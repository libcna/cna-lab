// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Project.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <typeindex>
#include <vector>

#include "Myra/MML/LoadContext.hpp"

namespace Myra::MML
{
    class TypeRegistry;
    class ValueCodecRegistry;
}

namespace Myra::Graphics2D::UI
{
    class Widget;

    /** @brief Settings persisted with a project for future source-code export. */
    class ExportOptions final
    {
    public:
        [[nodiscard]] const std::optional<std::string>& getNamespaceProperty() const noexcept;
        void setNamespaceProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getClassProperty() const noexcept;
        void setClassProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getOutputPathProperty() const noexcept;
        void setOutputPathProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getTemplateDesignerProperty() const noexcept;
        void setTemplateDesignerProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getTemplateMainProperty() const noexcept;
        void setTemplateMainProperty(std::optional<std::string> value);

    private:
        std::optional<std::string> namespace_;
        std::optional<std::string> class_;
        std::optional<std::string> outputPath_;
        std::optional<std::string> templateDesigner_;
        std::optional<std::string> templateMain_;
    };

    /** @brief Owns one serializable UI tree and the source nodes produced while loading it. */
    class Project final
    {
    public:
        inline static constexpr std::string_view ProportionName = "Proportion";
        inline static constexpr std::string_view DefaultProportionName = "DefaultProportion";
        inline static constexpr std::string_view DefaultColumnProportionName = "DefaultColumnProportion";
        inline static constexpr std::string_view DefaultRowProportionName = "DefaultRowProportion";

        Project() = default;
        ~Project() = default;

        Project(const Project&) = delete;
        Project& operator=(const Project&) = delete;
        Project(Project&&) = delete;
        Project& operator=(Project&&) = delete;

        [[nodiscard]] ExportOptions& getExportOptionsProperty() noexcept;
        [[nodiscard]] const ExportOptions& getExportOptionsProperty() const noexcept;
        [[nodiscard]] const std::shared_ptr<Widget>& getRootProperty() const noexcept;
        void setRootProperty(std::shared_ptr<Widget> value) noexcept;
        [[nodiscard]] const std::optional<std::string>& getStylesheetPathProperty() const noexcept;
        void setStylesheetPathProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getDesignerRtfAssetsPathProperty() const noexcept;
        void setDesignerRtfAssetsPathProperty(std::optional<std::string> value);

        /** @brief Loaded object/node mappings whose nodes remain valid for this Project's lifetime. */
        [[nodiscard]] const std::vector<MML::LoadedObjectNode>& getObjectsNodesProperty() const noexcept;

        [[nodiscard]] static bool IsProportionName(std::string_view value) noexcept;

        /** @brief Serializes using the registry/codecs supplied by an integrating application. */
        [[nodiscard]] std::string ToXml(
            const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs) const;

        /** @brief Serializes all currently registered built-in Myra-CNA project types. */
        [[nodiscard]] std::string ToXml() const;

        /** @brief Loads a Project and makes it own its parsed document and mapping handles. */
        [[nodiscard]] static std::shared_ptr<Project> LoadFromXml(const std::string& xml,
            const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs);

        /** @brief Loads all currently registered built-in Myra-CNA project types. */
        [[nodiscard]] static std::shared_ptr<Project> LoadFromXml(const std::string& xml);

        /** @brief Loads one designer object while owning its source DOM and mappings. */
        [[nodiscard]] static MML::LoadedDocument LoadObjectFromXml(const std::string& xml,
            const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs);

        /** @brief Loads one currently registered built-in designer object. */
        [[nodiscard]] static MML::LoadedDocument LoadObjectFromXml(const std::string& xml);

        /** @brief Saves only one object's scalar/attached state for designer source replacement. */
        [[nodiscard]] std::string SaveObjectToXml(const void* object, std::type_index objectType,
            const MML::TypeRegistry& typeRegistry, const MML::ValueCodecRegistry& valueCodecs,
            std::optional<std::string> tagName = std::nullopt,
            std::optional<std::type_index> parentType = std::nullopt) const;

        /** @brief Saves one currently registered built-in designer object. */
        [[nodiscard]] std::string SaveObjectToXml(const void* object, std::type_index objectType,
            std::optional<std::string> tagName = std::nullopt,
            std::optional<std::type_index> parentType = std::nullopt) const;

    private:
        void AdoptLoadedDocument(MML::LoadedDocument loadedDocument);

        ExportOptions exportOptions_;
        std::shared_ptr<Widget> root_;
        std::optional<std::string> stylesheetPath_;
        std::optional<std::string> designerRtfAssetsPath_;
        std::shared_ptr<System::Xml::XmlDocument> sourceDocument_;
        std::vector<MML::LoadedObjectNode> objectsNodes_;
    };
}
