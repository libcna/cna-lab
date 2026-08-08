// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/SaveContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/SaveContext.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/MML/AttachedPropertiesRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

namespace Myra::MML
{
    SaveContext::SaveContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept
        : BaseContext(typeRegistry, valueCodecs)
    {
    }

    System::Xml::XmlElement* SaveContext::Save(const void* object, const std::type_index type,
        System::Xml::XmlDocument& document, const bool skipComplex, std::optional<std::string> tagName,
        const std::optional<std::type_index> parentType) const
    {
        if (object == nullptr)
        {
            throw std::invalid_argument("Cannot save a null object as MML.");
        }
        const TypeDescriptor* descriptor = getTypeRegistryProperty().FindByType(type);
        if (descriptor == nullptr)
        {
            throw std::out_of_range("Cannot save an unregistered C++ type as MML.");
        }

        System::Xml::XmlElement* element = document.CreateElement(
            tagName ? *tagName : descriptor->getXmlNameProperty());
        const ParsedProperties properties = ParseProperties(type, true);
        for (const PropertyDescriptor* property : properties.Simple)
        {
            const bool shouldSerialize = ShouldSerializeProperty
                ? ShouldSerializeProperty(object, type, *property)
                : !HasDefaultValue(object, type, *property);
            if (!shouldSerialize)
            {
                continue;
            }

            const std::any value = getTypeRegistryProperty().GetPropertyValue(object, type, *property);
            if (property->IsNull(value))
            {
                continue;
            }

            const std::string text = GetSimplePropertyValue(*property, value);
            if (!text.empty())
            {
                element->SetAttribute(property->getXmlNameProperty(), text);
                continue;
            }

            const std::optional<std::any>& defaultValue = property->getDefaultValueProperty();
            if (defaultValue && !property->IsNull(*defaultValue) &&
                !GetSimplePropertyValue(*property, *defaultValue).empty())
            {
                element->SetAttribute(property->getXmlNameProperty(), "");
            }
        }

        const BaseObject* baseObject = descriptor->GetBaseObject(object);
        if (baseObject != nullptr && parentType)
        {
            const std::vector<const BaseAttachedPropertyInfo*> attachedProperties =
                AttachedPropertiesRegistry::GetPropertiesOfType(*parentType, getTypeRegistryProperty());
            for (const BaseAttachedPropertyInfo* property : attachedProperties)
            {
                const PropertyMetadata& metadata = property->getMetadataProperty();
                if (metadata.XmlIgnore || metadata.SkipSave || metadata.Obsolete)
                {
                    continue;
                }

                const std::any value = property->GetValueObject(*baseObject);
                if (property->IsDefaultValueObject(value))
                {
                    continue;
                }

                const std::string text = GetAttachedPropertyValue(*property, value);
                if (text.empty())
                {
                    continue;
                }
                const TypeDescriptor* owner = getTypeRegistryProperty().FindByType(property->getOwnerTypeProperty());
                if (owner == nullptr)
                {
                    throw std::logic_error("An attached-property owner is missing from TypeRegistry.");
                }
                const std::string propertyName = owner->getNameProperty() + "." + property->getNameProperty();
                element->SetAttribute(propertyName, text);
            }
        }

        if (!skipComplex)
        {
            const PropertyDescriptor* contentProperty = nullptr;
            for (const PropertyDescriptor* property : properties.Complex)
            {
                if (!property->getMetadataProperty().Content)
                {
                    continue;
                }
                if (contentProperty != nullptr)
                {
                    throw std::logic_error("An MML type cannot have multiple content properties.");
                }
                contentProperty = property;
            }
            for (const PropertyDescriptor* property : properties.Complex)
            {
                SaveComplexProperty(object, type, *property, property == contentProperty, *element, document);
            }
        }
        return element;
    }

    std::string SaveContext::ToXml(const void* object, const std::type_index type,
        const bool skipComplex, std::optional<std::string> tagName,
        const std::optional<std::type_index> parentType) const
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = Save(
            object, type, document, skipComplex, std::move(tagName), parentType);
        document.AppendChild(root);
        return document.getOuterXmlProperty();
    }

    bool SaveContext::HasDefaultValue(const void* object, const std::type_index objectType,
        const PropertyDescriptor& property) const
    {
        if (!property.getDefaultValueProperty() || !property.getCanCompareDefaultProperty())
        {
            return false;
        }
        return property.IsDefaultValue(
            getTypeRegistryProperty().GetPropertyValue(object, objectType, property));
    }

    void SaveContext::SaveComplexProperty(const void* object, const std::type_index ownerType,
        const PropertyDescriptor& property, const bool isContent,
        System::Xml::XmlElement& element, System::Xml::XmlDocument& document) const
    {
        const bool shouldSerialize = ShouldSerializeProperty
            ? ShouldSerializeProperty(object, ownerType, property)
            : !HasDefaultValue(object, ownerType, property);
        if (!shouldSerialize)
        {
            return;
        }

        const std::optional<ComplexPropertyAdapter>& adapterValue = property.getComplexAdapterProperty();
        if (!adapterValue)
        {
            throw std::logic_error("The complex MML property has no explicit adapter.");
        }
        const ComplexPropertyAdapter& adapter = *adapterValue;
        const void* propertyOwner = getTypeRegistryProperty().GetPropertyOwner(object, ownerType, property);
        const std::vector<RegisteredObjectView> values = adapter.EnumerateObjects(propertyOwner);
        if (adapter.getKindProperty() == ComplexPropertyKind::Single && values.size() > 1)
        {
            throw std::logic_error("A single-object MML adapter enumerated more than one object.");
        }
        if (values.empty())
        {
            return;
        }

        const TypeDescriptor* owner = getTypeRegistryProperty().FindByType(ownerType);
        if (owner == nullptr)
        {
            throw std::logic_error("The complex MML property owner is missing from TypeRegistry.");
        }
        const std::string propertyName = PrependNamespace
            ? owner->getNameProperty() + "." + property.getXmlNameProperty()
            : property.getXmlNameProperty();

        if (adapter.getKindProperty() == ComplexPropertyKind::Single)
        {
            const RegisteredObjectView& value = values.front();
            if (value.Value == nullptr ||
                !getTypeRegistryProperty().IsTypeOrDerivedFrom(value.Type, adapter.getItemTypeProperty()))
            {
                throw std::invalid_argument("A complex MML adapter enumerated an invalid object type.");
            }
            System::Xml::XmlElement* child = isContent
                ? Save(value.Value, value.Type, document)
                : Save(value.Value, value.Type, document, false, propertyName);
            element.AppendChild(child);
            return;
        }

        System::Xml::XmlElement* collectionRoot = &element;
        if (!isContent)
        {
            collectionRoot = document.CreateElement(propertyName);
            element.AppendChild(collectionRoot);
        }
        for (const RegisteredObjectView& value : values)
        {
            if (value.Value == nullptr ||
                !getTypeRegistryProperty().IsTypeOrDerivedFrom(value.Type, adapter.getItemTypeProperty()))
            {
                throw std::invalid_argument("A complex MML adapter enumerated an invalid object type.");
            }
            const std::optional<std::type_index> parentType =
                adapter.getKindProperty() == ComplexPropertyKind::Sequence
                    ? std::optional<std::type_index>(ownerType)
                    : std::nullopt;
            System::Xml::XmlElement* child = Save(
                value.Value, value.Type, document, false, std::nullopt, parentType);
            collectionRoot->AppendChild(child);
        }
    }

    std::string SaveContext::GetSimplePropertyValue(
        const PropertyDescriptor& property, const std::any& value) const
    {
        if (IsPropertyExternalAsset(property))
        {
            if (!SaveExternalAsset)
            {
                throw std::logic_error("The MML property requires an external-asset saver.");
            }
            return SaveExternalAsset(property, value);
        }
        return getValueCodecsProperty().Serialize(value);
    }

    std::string SaveContext::GetAttachedPropertyValue(
        const BaseAttachedPropertyInfo& property, const std::any& value) const
    {
        if (property.getMetadataProperty().ExternalAsset)
        {
            if (!SaveAttachedExternalAsset)
            {
                throw std::logic_error("The attached MML property requires an external-asset saver.");
            }
            return SaveAttachedExternalAsset(property, value);
        }
        return getValueCodecsProperty().Serialize(value);
    }
}
