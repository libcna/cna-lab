// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/AttachedPropertiesRegistry.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

#include <limits>
#include <mutex>

namespace Myra::MML
{
    namespace
    {
        struct RegistryState
        {
            std::mutex Mutex;
            std::vector<std::unique_ptr<BaseAttachedPropertyInfo>> Properties;
            int NextId = 0;
        };

        RegistryState& GetRegistryState()
        {
            static RegistryState state;
            return state;
        }
    }

    BaseAttachedPropertyInfo::BaseAttachedPropertyInfo(const int id, std::string name,
        const std::type_index ownerType, const AttachedPropertyOption option, PropertyMetadata metadata)
        : id_(id), name_(std::move(name)), ownerType_(ownerType), option_(option), metadata_(std::move(metadata))
    {
        if (name_.empty())
        {
            throw std::invalid_argument("An attached property name cannot be empty.");
        }
    }

    int BaseAttachedPropertyInfo::getIdProperty() const noexcept { return id_; }

    const std::string& BaseAttachedPropertyInfo::getNameProperty() const noexcept { return name_; }

    std::type_index BaseAttachedPropertyInfo::getOwnerTypeProperty() const noexcept { return ownerType_; }

    AttachedPropertyOption BaseAttachedPropertyInfo::getOptionProperty() const noexcept { return option_; }

    const PropertyMetadata& BaseAttachedPropertyInfo::getMetadataProperty() const noexcept { return metadata_; }

    bool BaseAttachedPropertyInfo::HasValue(const BaseObject& object) const noexcept
    {
        return object.AttachedPropertiesValues.contains(id_);
    }

    int AttachedPropertiesRegistry::ReserveId()
    {
        RegistryState& state = GetRegistryState();
        const std::lock_guard lock(state.Mutex);
        if (state.NextId == std::numeric_limits<int>::max())
        {
            throw std::overflow_error("The attached property registry ran out of identifiers.");
        }
        return state.NextId++;
    }

    void AttachedPropertiesRegistry::Register(std::unique_ptr<BaseAttachedPropertyInfo> property)
    {
        if (!property)
        {
            throw std::invalid_argument("An attached property cannot be null.");
        }
        RegistryState& state = GetRegistryState();
        const std::lock_guard lock(state.Mutex);
        for (const std::unique_ptr<BaseAttachedPropertyInfo>& existing : state.Properties)
        {
            if (existing->getOwnerTypeProperty() == property->getOwnerTypeProperty() &&
                existing->getNameProperty() == property->getNameProperty())
            {
                throw std::invalid_argument(
                    "An attached property with the same owner type and name is already registered.");
            }
        }
        state.Properties.push_back(std::move(property));
    }

    std::vector<const BaseAttachedPropertyInfo*> AttachedPropertiesRegistry::GetPropertiesOfType(
        const std::type_index type, const TypeRegistry& typeRegistry)
    {
        const std::vector<std::type_index> hierarchy = typeRegistry.GetTypesIncludingBase(type);
        RegistryState& state = GetRegistryState();
        const std::lock_guard lock(state.Mutex);

        std::vector<const BaseAttachedPropertyInfo*> result;
        for (const std::type_index ownerType : hierarchy)
        {
            for (const std::unique_ptr<BaseAttachedPropertyInfo>& property : state.Properties)
            {
                if (property->getOwnerTypeProperty() == ownerType)
                {
                    result.push_back(property.get());
                }
            }
        }
        return result;
    }

    const BaseAttachedPropertyInfo* AttachedPropertiesRegistry::FindProperty(
        const std::type_index type, const std::string& name, const TypeRegistry& typeRegistry)
    {
        const std::vector<const BaseAttachedPropertyInfo*> properties = GetPropertiesOfType(type, typeRegistry);
        for (const BaseAttachedPropertyInfo* property : properties)
        {
            if (property->getNameProperty() == name)
            {
                return property;
            }
        }
        return nullptr;
    }
}
