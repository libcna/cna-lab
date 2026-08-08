// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/LoadContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/LoadContext.hpp"

#include <stdexcept>

#include "Myra/MML/AttachedPropertiesRegistry.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlAttributeCollection.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlNodeList.hpp"
#include "System/Xml/XmlNodeType.hpp"

namespace Myra::MML
{
    namespace
    {
        [[nodiscard]] std::string TrimName(const std::string& value)
        {
            const size_t first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
            {
                return {};
            }
            return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }
    }

    LoadContext::LoadContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept
        : BaseContext(typeRegistry, valueCodecs)
    {
    }

    void LoadContext::Load(void* object, const std::type_index type, const System::Xml::XmlElement& element)
    {
        const std::size_t objectsNodesStart = ObjectsNodes.size();
        try
        {
            LoadCore(object, type, element);
        }
        catch (...)
        {
            ObjectsNodes.resize(objectsNodesStart);
            throw;
        }
    }

    void LoadContext::LoadCore(void* object, const std::type_index type,
        const System::Xml::XmlElement& element)
    {
        if (object == nullptr)
        {
            throw std::invalid_argument("Cannot load MML into a null object.");
        }

        const TypeDescriptor* descriptor = getTypeRegistryProperty().FindByType(type);
        if (descriptor == nullptr)
        {
            throw std::out_of_range("Cannot load MML into an unregistered C++ type.");
        }
        BaseObject* baseObject = descriptor->GetBaseObject(object);

        const ParsedProperties properties = ParseProperties(type, false);
        ObjectsNodes.emplace_back(object, &element);

        const System::Xml::XmlAttributeCollection* attributes = element.getAttributesProperty();
        for (SharpRuntime::intcs index = 0; index < attributes->getCountProperty(); ++index)
        {
            const System::Xml::XmlAttribute* attribute = (*attributes)[index];
            const std::string rawName = attribute->getNameProperty();
            const std::string propertyName = ApplyLegacyPropertyName(rawName);
            if (propertyName.find('.') != std::string::npos)
            {
                const size_t separator = propertyName.find('.');
                if (separator != propertyName.rfind('.'))
                {
                    throw std::invalid_argument("An attached-property MML name must contain exactly one dot.");
                }

                const std::string ownerName = TrimName(propertyName.substr(0, separator));
                const std::string attachedName = TrimName(propertyName.substr(separator + 1));
                if (ownerName.empty() || attachedName.empty())
                {
                    throw std::invalid_argument("An attached-property MML owner and name cannot be empty.");
                }

                const TypeDescriptor* owner = getTypeRegistryProperty().FindByName(ownerName);
                if (owner == nullptr)
                {
                    owner = getTypeRegistryProperty().FindByXmlName(ownerName);
                }
                if (owner == nullptr)
                {
                    throw std::out_of_range("Could not resolve attached-property owner '" + ownerName +
                        "' while loading MML attribute '" + rawName + "'.");
                }

                const BaseAttachedPropertyInfo* attachedProperty = AttachedPropertiesRegistry::FindProperty(
                    owner->getTypeProperty(), attachedName, getTypeRegistryProperty());
                if (attachedProperty == nullptr)
                {
                    throw std::out_of_range("Could not resolve attached MML property '" + rawName + "'.");
                }
                const PropertyMetadata& metadata = attachedProperty->getMetadataProperty();
                if (metadata.XmlIgnore || metadata.SkipLoad)
                {
                    continue;
                }
                if (baseObject == nullptr)
                {
                    throw std::logic_error(
                        "The MML target type has no registered BaseObject adapter for attached properties.");
                }

                std::any attachedValue;
                if (metadata.ExternalAsset)
                {
                    if (!LoadAttachedExternalAsset)
                    {
                        throw std::logic_error("The attached MML property requires an external-asset loader.");
                    }
                    attachedValue = LoadAttachedExternalAsset(*attachedProperty, attribute->getValueProperty());
                }
                else
                {
                    try
                    {
                        attachedValue = getValueCodecsProperty().Deserialize(
                            attachedProperty->getPropertyTypeProperty(), attribute->getValueProperty());
                    }
                    catch (const std::invalid_argument& error)
                    {
                        throw std::invalid_argument("Could not parse attached MML attribute '" + rawName +
                            "' on element '" + element.getNameProperty() + "': " + error.what());
                    }
                }
                attachedProperty->SetValueObject(*baseObject, attachedValue);
                continue;
            }

            const PropertyDescriptor* property = nullptr;
            for (auto iterator = properties.Simple.rbegin(); iterator != properties.Simple.rend(); ++iterator)
            {
                if ((*iterator)->getNameProperty() == propertyName ||
                    (*iterator)->getXmlNameProperty() == propertyName)
                {
                    property = *iterator;
                    break;
                }
            }

            if (property == nullptr)
            {
                if (propertyName.starts_with('_'))
                {
                    if (baseObject != nullptr)
                    {
                        const bool inserted = baseObject->getUserDataProperty().emplace(
                            propertyName, attribute->getValueProperty()).second;
                        if (!inserted)
                        {
                            throw std::invalid_argument("The BaseObject user-data key is already present.");
                        }
                    }
                }
                continue;
            }

            std::any value;
            if (IsPropertyExternalAsset(*property))
            {
                if (!LoadExternalAsset)
                {
                    throw std::logic_error("The MML property requires an external-asset loader.");
                }
                value = LoadExternalAsset(*property, attribute->getValueProperty());
            }
            else
            {
                try
                {
                    value = getValueCodecsProperty().Deserialize(
                        property->getValueTypeProperty(), attribute->getValueProperty());
                }
                catch (const std::invalid_argument& error)
                {
                    throw std::invalid_argument("Could not parse MML attribute '" + rawName +
                        "' on element '" + element.getNameProperty() + "': " + error.what());
                }
            }
            getTypeRegistryProperty().SetPropertyValue(object, type, *property, value);
        }

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

        System::Xml::XmlNodeList* children = element.getChildNodesProperty();
        for (SharpRuntime::intcs index = 0; index < children->getCountProperty(); ++index)
        {
            System::Xml::XmlNode* node = children->Item(index);
            if (node->getNodeTypeProperty() != System::Xml::XmlNodeType::Element)
            {
                continue;
            }
            if (NodesToIgnore.contains(node->getNameProperty()))
            {
                continue;
            }

            const auto* child = static_cast<const System::Xml::XmlElement*>(node);
            std::string childName = child->getNameProperty();
            bool isPropertyElement = false;
            const size_t separator = childName.find('.');
            if (separator != std::string::npos)
            {
                if (separator != childName.rfind('.'))
                {
                    throw std::invalid_argument("An MML property element name must contain at most one dot.");
                }
                childName = TrimName(childName.substr(separator + 1));
                if (childName.empty())
                {
                    throw std::invalid_argument("An MML property element name cannot be empty.");
                }
                isPropertyElement = true;
            }
            childName = ApplyLegacyPropertyName(childName);

            const PropertyDescriptor* property = nullptr;
            for (auto iterator = properties.Complex.rbegin(); iterator != properties.Complex.rend(); ++iterator)
            {
                if ((*iterator)->getNameProperty() == childName ||
                    (*iterator)->getXmlNameProperty() == childName)
                {
                    property = *iterator;
                    break;
                }
            }
            if (property != nullptr)
            {
                LoadComplexProperty(object, type, *property, *child);
                continue;
            }
            if (isPropertyElement)
            {
                throw std::out_of_range("The registered MML type has no complex property matching element '" +
                    child->getNameProperty() + "'.");
            }

            const TypeDescriptor& itemDescriptor = ResolveElementType(*child,
                contentProperty && contentProperty->getComplexAdapterProperty()
                    ? contentProperty->getComplexAdapterProperty()->getItemTypeProperty()
                    : typeid(void));
            std::shared_ptr<void> item = CreateRegisteredObject(itemDescriptor, *child);
            Load(item.get(), itemDescriptor.getTypeProperty(), *child);

            if (contentProperty == nullptr)
            {
                if (DemandContentProperty)
                {
                    throw std::logic_error("The registered MML type has no content property.");
                }
                continue;
            }
            const std::optional<ComplexPropertyAdapter>& adapter = contentProperty->getComplexAdapterProperty();
            if (!adapter)
            {
                throw std::logic_error("The MML content property has no complex-property adapter.");
            }
            void* contentOwner = getTypeRegistryProperty().GetPropertyOwner(object, type, *contentProperty);
            if (adapter->getKindProperty() == ComplexPropertyKind::Sequence)
            {
                adapter->AppendObject(contentOwner, getTypeRegistryProperty().CastObject(
                    item, itemDescriptor.getTypeProperty(), adapter->getItemTypeProperty()));
            }
            else if (adapter->getKindProperty() == ComplexPropertyKind::Single &&
                adapter->getCanAssignSingleProperty())
            {
                adapter->AssignObject(contentOwner, getTypeRegistryProperty().CastObject(
                    item, itemDescriptor.getTypeProperty(), adapter->getItemTypeProperty()));
            }
            else
            {
                throw std::logic_error("An implicit MML content property must be a writable single or sequence.");
            }
        }
    }

    LoadedObject LoadContext::CreateAndLoad(const System::Xml::XmlElement& element)
    {
        const std::string typeName = ApplyLegacyClassName(element.getNameProperty());
        const TypeDescriptor* descriptor = getTypeRegistryProperty().FindByXmlName(typeName);
        if (descriptor == nullptr)
        {
            throw std::out_of_range("Could not resolve MML root element '" + typeName +
                "' through TypeRegistry.");
        }

        std::shared_ptr<void> object = CreateRegisteredObject(*descriptor, element);
        Load(object.get(), descriptor->getTypeProperty(), element);
        return {std::move(object), descriptor->getTypeProperty()};
    }

    std::shared_ptr<void> LoadContext::CreateRegisteredObject(
        const TypeDescriptor& descriptor, const System::Xml::XmlElement& element)
    {
        std::shared_ptr<void> object = CreateObject ? CreateObject(descriptor, element) : descriptor.Create();
        if (!object)
        {
            throw std::runtime_error("The MML object creator returned null.");
        }
        return object;
    }

    const TypeDescriptor& LoadContext::ResolveElementType(
        const System::Xml::XmlElement& element, const std::type_index expectedBase) const
    {
        const std::string typeName = ApplyLegacyClassName(element.getNameProperty());
        const TypeDescriptor* descriptor = getTypeRegistryProperty().FindByXmlName(typeName);
        if (descriptor == nullptr)
        {
            throw std::out_of_range("Could not resolve nested MML element '" + typeName +
                "' through TypeRegistry.");
        }
        if (expectedBase != typeid(void) &&
            !getTypeRegistryProperty().IsTypeOrDerivedFrom(descriptor->getTypeProperty(), expectedBase))
        {
            const TypeDescriptor* expected = getTypeRegistryProperty().FindByType(expectedBase);
            const std::string expectedName = expected == nullptr ? expectedBase.name() : expected->getNameProperty();
            throw std::invalid_argument("Nested MML element '" + typeName +
                "' is incompatible with expected base type '" + expectedName + "'.");
        }
        return *descriptor;
    }

    void LoadContext::LoadComplexProperty(void* object, const std::type_index objectType,
        const PropertyDescriptor& property, const System::Xml::XmlElement& element)
    {
        const std::optional<ComplexPropertyAdapter>& adapterValue = property.getComplexAdapterProperty();
        if (!adapterValue)
        {
            throw std::logic_error("The complex MML property has no explicit adapter.");
        }
        const ComplexPropertyAdapter& adapter = *adapterValue;
        void* propertyOwner = getTypeRegistryProperty().GetPropertyOwner(object, objectType, property);
        const TypeDescriptor* itemDescriptor = getTypeRegistryProperty().FindByType(adapter.getItemTypeProperty());
        if (itemDescriptor == nullptr)
        {
            throw std::logic_error("The complex MML property item type is not registered.");
        }

        if (adapter.getKindProperty() == ComplexPropertyKind::Single)
        {
            if (!adapter.getCanAssignSingleProperty())
            {
                void* existing = adapter.GetExistingObject(propertyOwner);
                if (existing == nullptr)
                {
                    throw std::runtime_error("A read-only complex MML property returned a null object.");
                }
                Load(existing, itemDescriptor->getTypeProperty(), element);
                return;
            }

            std::shared_ptr<void> value = CreateRegisteredObject(*itemDescriptor, element);
            Load(value.get(), itemDescriptor->getTypeProperty(), element);
            adapter.AssignObject(propertyOwner, value);
            return;
        }

        System::Xml::XmlNodeList* children = element.getChildNodesProperty();
        for (SharpRuntime::intcs index = 0; index < children->getCountProperty(); ++index)
        {
            System::Xml::XmlNode* node = children->Item(index);
            if (node->getNodeTypeProperty() != System::Xml::XmlNodeType::Element)
            {
                continue;
            }
            const auto* child = static_cast<const System::Xml::XmlElement*>(node);
            const TypeDescriptor& concreteDescriptor = ResolveElementType(*child, adapter.getItemTypeProperty());
            std::shared_ptr<void> value = CreateRegisteredObject(concreteDescriptor, *child);
            Load(value.get(), concreteDescriptor.getTypeProperty(), *child);

            if (adapter.getKindProperty() == ComplexPropertyKind::Sequence)
            {
                adapter.AppendObject(propertyOwner, getTypeRegistryProperty().CastObject(
                    value, concreteDescriptor.getTypeProperty(), adapter.getItemTypeProperty()));
            }
            else
            {
                const std::string key = child->HasAttribute(std::string(IdName))
                    ? child->GetAttribute(std::string(IdName))
                    : std::string();
                adapter.InsertObject(propertyOwner, key, getTypeRegistryProperty().CastObject(
                    value, concreteDescriptor.getTypeProperty(), adapter.getItemTypeProperty()));
            }
        }
    }

    std::string LoadContext::ApplyLegacyPropertyName(const std::string& name) const
    {
        const auto iterator = LegacyPropertyNames.find(name);
        return iterator == LegacyPropertyNames.end() ? name : iterator->second;
    }

    std::string LoadContext::ApplyLegacyClassName(const std::string& name) const
    {
        const auto iterator = LegacyClassNames.find(name);
        return iterator == LegacyClassNames.end() ? name : iterator->second;
    }
}
