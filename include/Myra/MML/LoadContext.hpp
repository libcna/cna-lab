// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/LoadContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Myra/MML/BaseContext.hpp"

namespace System::Xml
{
    class XmlElement;
}

namespace Myra::MML
{
    class BaseAttachedPropertyInfo;

    /** @brief A type-erased object created by the explicit MML type registry. */
    struct LoadedObject final
    {
        std::shared_ptr<void> Value;
        std::type_index Type;
    };

    /** @brief Registry-backed MML loader using explicit scalar, object, collection, and asset adapters. */
    class LoadContext final : public BaseContext
    {
    public:
        using ObjectCreator = std::function<std::shared_ptr<void>(
            const TypeDescriptor&, const System::Xml::XmlElement&)>;
        using ExternalAssetLoader = std::function<std::any(
            const PropertyDescriptor&, const std::string&)>;
        using AttachedExternalAssetLoader = std::function<std::any(
            const BaseAttachedPropertyInfo&, const std::string&)>;

        LoadContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept;

        std::unordered_map<std::string, std::string> LegacyClassNames;
        std::unordered_map<std::string, std::string> LegacyPropertyNames;
        std::unordered_set<std::string> NodesToIgnore;
        ObjectCreator CreateObject;
        ExternalAssetLoader LoadExternalAsset;
        AttachedExternalAssetLoader LoadAttachedExternalAsset;
        bool DemandContentProperty = true;

        /** @brief Pairs each loaded object address with its source element. */
        std::vector<std::pair<void*, const System::Xml::XmlElement*>> ObjectsNodes;

        void Load(void* object, std::type_index type, const System::Xml::XmlElement& element);
        [[nodiscard]] LoadedObject CreateAndLoad(const System::Xml::XmlElement& element);

    private:
        void LoadCore(void* object, std::type_index type, const System::Xml::XmlElement& element);
        [[nodiscard]] std::shared_ptr<void> CreateRegisteredObject(
            const TypeDescriptor& descriptor, const System::Xml::XmlElement& element);
        [[nodiscard]] const TypeDescriptor& ResolveElementType(
            const System::Xml::XmlElement& element, std::type_index expectedBase) const;
        void LoadComplexProperty(void* object, std::type_index objectType, const PropertyDescriptor& property,
            const System::Xml::XmlElement& element);
        [[nodiscard]] std::string ApplyLegacyPropertyName(const std::string& name) const;
        [[nodiscard]] std::string ApplyLegacyClassName(const std::string& name) const;
    };
}
