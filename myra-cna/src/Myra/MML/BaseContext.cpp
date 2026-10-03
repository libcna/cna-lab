// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/BaseContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/BaseContext.hpp"

namespace Myra::MML
{
    BaseContext::BaseContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept
        : typeRegistry_(typeRegistry), valueCodecs_(valueCodecs)
    {
    }

    const ITypeSerializer* BaseContext::FindSerializer(const std::type_index type) const noexcept
    {
        return valueCodecs_.Find(type);
    }

    bool BaseContext::IsPropertyExternalAsset(const PropertyDescriptor& property) noexcept
    {
        return property.getMetadataProperty().ExternalAsset;
    }

    ParsedProperties BaseContext::ParseProperties(const std::type_index type, const bool isSave) const
    {
        ParsedProperties result;
        for (const PropertyDescriptor* property : typeRegistry_.GetPropertiesIncludingBase(type))
        {
            if (!property->getCanReadProperty() && !property->getComplexAdapterProperty())
            {
                continue;
            }

            const PropertyMetadata& metadata = property->getMetadataProperty();
            if (metadata.XmlIgnore || (isSave && (metadata.SkipSave || metadata.Obsolete)) ||
                (!isSave && metadata.SkipLoad))
            {
                continue;
            }

            if (FindSerializer(property->getValueTypeProperty()) != nullptr || IsPropertyExternalAsset(*property))
            {
                result.Simple.push_back(property);
            }
            else
            {
                result.Complex.push_back(property);
            }
        }
        return result;
    }

    const TypeRegistry& BaseContext::getTypeRegistryProperty() const noexcept { return typeRegistry_; }

    const ValueCodecRegistry& BaseContext::getValueCodecsProperty() const noexcept { return valueCodecs_; }
}
