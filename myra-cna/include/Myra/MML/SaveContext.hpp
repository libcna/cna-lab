// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/SaveContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <functional>
#include <optional>
#include <string>
#include <typeindex>

#include "Myra/MML/BaseContext.hpp"

namespace System::Xml
{
    class XmlDocument;
    class XmlElement;
}

namespace Myra::MML
{
    class BaseAttachedPropertyInfo;

    /** @brief Registry-backed MML saver using explicit scalar, object, collection, and asset adapters. */
    class SaveContext final : public BaseContext
    {
    public:
        using PropertyFilter = std::function<bool(
            const void*, std::type_index, const PropertyDescriptor&)>;
        using ExternalAssetSaver = std::function<std::string(
            const PropertyDescriptor&, const std::any&)>;
        using AttachedExternalAssetSaver = std::function<std::string(
            const BaseAttachedPropertyInfo&, const std::any&)>;

        SaveContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept;

        PropertyFilter ShouldSerializeProperty;
        ExternalAssetSaver SaveExternalAsset;
        AttachedExternalAssetSaver SaveAttachedExternalAsset;
        bool PrependNamespace = true;

        [[nodiscard]] System::Xml::XmlElement* Save(const void* object, std::type_index type,
            System::Xml::XmlDocument& document, bool skipComplex = false,
            std::optional<std::string> tagName = std::nullopt,
            std::optional<std::type_index> parentType = std::nullopt) const;
        [[nodiscard]] std::string ToXml(const void* object, std::type_index type,
            bool skipComplex = false, std::optional<std::string> tagName = std::nullopt,
            std::optional<std::type_index> parentType = std::nullopt) const;

        [[nodiscard]] bool HasDefaultValue(
            const void* object, std::type_index objectType, const PropertyDescriptor& property) const;

    private:
        void SaveComplexProperty(const void* object, std::type_index ownerType,
            const PropertyDescriptor& property, bool isContent,
            System::Xml::XmlElement& element, System::Xml::XmlDocument& document) const;
        [[nodiscard]] std::string GetSimplePropertyValue(
            const PropertyDescriptor& property, const std::any& value) const;
        [[nodiscard]] std::string GetAttachedPropertyValue(
            const BaseAttachedPropertyInfo& property, const std::any& value) const;
    };
}
