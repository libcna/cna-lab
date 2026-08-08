// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the necessary C++ metadata adaptation of Myra MML semantics.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Myra/Attributes/FilePathAttribute.hpp"
#include "Myra/Attributes/RangeAttribute.hpp"

namespace Myra::MML
{
    /** @brief Explicit metadata that replaces the relevant C# property attributes. */
    struct PropertyMetadata
    {
        bool Content = false;
        bool DesignerFolded = false;
        bool SkipLoad = false;
        bool SkipSave = false;
        std::optional<std::string> XmlName;
        std::optional<std::string> StylePropertyPath;
        std::optional<Attributes::RangeAttribute> Range;
        std::optional<Attributes::FilePathAttribute> FilePath;
    };

    /** @brief Defines a single readable/writable property of a registered C++ type. */
    class PropertyDescriptor final
    {
    public:
        using Value = std::any;
        using Getter = std::function<Value(const void*)>;
        using Setter = std::function<void(void*, const Value&)>;

        PropertyDescriptor(std::string name, std::type_index valueType, Getter getter = {}, Setter setter = {},
            std::optional<Value> defaultValue = std::nullopt, PropertyMetadata metadata = {})
            : name_(std::move(name)), valueType_(valueType), getter_(std::move(getter)), setter_(std::move(setter)),
              defaultValue_(std::move(defaultValue)), metadata_(std::move(metadata))
        {
            if (name_.empty())
            {
                throw std::invalid_argument("A property descriptor name cannot be empty.");
            }
        }

        [[nodiscard]] const std::string& getNameProperty() const noexcept { return name_; }
        [[nodiscard]] const std::string& getXmlNameProperty() const noexcept
        {
            return metadata_.XmlName ? *metadata_.XmlName : name_;
        }
        [[nodiscard]] std::type_index getValueTypeProperty() const noexcept { return valueType_; }
        [[nodiscard]] bool getCanReadProperty() const noexcept { return static_cast<bool>(getter_); }
        [[nodiscard]] bool getCanWriteProperty() const noexcept { return static_cast<bool>(setter_); }
        [[nodiscard]] const std::optional<Value>& getDefaultValueProperty() const noexcept { return defaultValue_; }
        [[nodiscard]] const PropertyMetadata& getMetadataProperty() const noexcept { return metadata_; }

        [[nodiscard]] Value Get(const void* instance) const
        {
            if (!getter_)
            {
                throw std::logic_error("The registered property is not readable.");
            }
            return getter_(instance);
        }

        void Set(void* instance, const Value& value) const
        {
            if (!setter_)
            {
                throw std::logic_error("The registered property is not writable.");
            }
            if (std::type_index(value.type()) != valueType_)
            {
                throw std::invalid_argument("The registered property value has the wrong C++ type.");
            }
            setter_(instance, value);
        }

    private:
        std::string name_;
        std::type_index valueType_;
        Getter getter_;
        Setter setter_;
        std::optional<Value> defaultValue_;
        PropertyMetadata metadata_;
    };

    /** @brief Describes one MML-instantiable C++ type without runtime reflection. */
    class TypeDescriptor final
    {
    public:
        using Factory = std::function<std::shared_ptr<void>()>;

        TypeDescriptor(std::string name, std::type_index type, Factory factory = {},
            std::optional<std::type_index> baseType = std::nullopt, std::optional<std::string> xmlName = std::nullopt)
            : name_(std::move(name)), xmlName_(xmlName ? std::move(*xmlName) : name_), type_(type),
              factory_(std::move(factory)), baseType_(baseType)
        {
            if (name_.empty() || xmlName_.empty())
            {
                throw std::invalid_argument("A type descriptor name cannot be empty.");
            }
        }

        [[nodiscard]] const std::string& getNameProperty() const noexcept { return name_; }
        [[nodiscard]] const std::string& getXmlNameProperty() const noexcept { return xmlName_; }
        [[nodiscard]] std::type_index getTypeProperty() const noexcept { return type_; }
        [[nodiscard]] const std::optional<std::type_index>& getBaseTypeProperty() const noexcept { return baseType_; }
        [[nodiscard]] bool getCanCreateProperty() const noexcept { return static_cast<bool>(factory_); }
        [[nodiscard]] const std::vector<PropertyDescriptor>& getPropertiesProperty() const noexcept { return properties_; }

        void AddProperty(PropertyDescriptor property)
        {
            for (const PropertyDescriptor& existing : properties_)
            {
                if (existing.getNameProperty() == property.getNameProperty() ||
                    existing.getXmlNameProperty() == property.getXmlNameProperty())
                {
                    throw std::invalid_argument("A type descriptor cannot contain duplicate property names.");
                }
            }
            properties_.push_back(std::move(property));
        }

        [[nodiscard]] std::shared_ptr<void> Create() const
        {
            if (!factory_)
            {
                throw std::logic_error("The registered type is abstract and cannot be created.");
            }
            return factory_();
        }

    private:
        std::string name_;
        std::string xmlName_;
        std::type_index type_;
        Factory factory_;
        std::optional<std::type_index> baseType_;
        std::vector<PropertyDescriptor> properties_;
    };

    /** @brief Explicit registry used by MML and PropertyGrid instead of .NET reflection. */
    class TypeRegistry final
    {
    public:
        void Register(TypeDescriptor descriptor)
        {
            const std::type_index type = descriptor.getTypeProperty();
            const std::string xmlName = descriptor.getXmlNameProperty();
            if (byType_.contains(type) || byXmlName_.contains(xmlName))
            {
                throw std::invalid_argument("The type or XML name is already registered.");
            }
            if (descriptor.getBaseTypeProperty() && !byType_.contains(*descriptor.getBaseTypeProperty()))
            {
                throw std::invalid_argument("A registered base type must be registered first.");
            }

            if (!byType_.emplace(type, std::move(descriptor)).second)
            {
                throw std::logic_error("TypeRegistry rejected a duplicate type unexpectedly.");
            }
            byXmlName_.emplace(xmlName, type);
        }

        [[nodiscard]] const TypeDescriptor* FindByType(const std::type_index type) const noexcept
        {
            const auto iterator = byType_.find(type);
            return iterator == byType_.end() ? nullptr : &iterator->second;
        }

        [[nodiscard]] const TypeDescriptor* FindByXmlName(const std::string& xmlName) const noexcept
        {
            const auto iterator = byXmlName_.find(xmlName);
            return iterator == byXmlName_.end() ? nullptr : FindByType(iterator->second);
        }

        [[nodiscard]] std::vector<const PropertyDescriptor*> GetPropertiesIncludingBase(
            const std::type_index type) const
        {
            const TypeDescriptor* descriptor = FindByType(type);
            if (descriptor == nullptr)
            {
                throw std::out_of_range("The requested type is not registered.");
            }

            std::vector<const TypeDescriptor*> hierarchy;
            for (const TypeDescriptor* current = descriptor; current != nullptr;)
            {
                hierarchy.push_back(current);
                current = current->getBaseTypeProperty() ? FindByType(*current->getBaseTypeProperty()) : nullptr;
            }

            std::vector<const PropertyDescriptor*> result;
            for (auto iterator = hierarchy.rbegin(); iterator != hierarchy.rend(); ++iterator)
            {
                for (const PropertyDescriptor& property : (*iterator)->getPropertiesProperty())
                {
                    result.push_back(&property);
                }
            }
            return result;
        }

        [[nodiscard]] std::shared_ptr<void> Create(const std::string& xmlName) const
        {
            const TypeDescriptor* descriptor = FindByXmlName(xmlName);
            if (descriptor == nullptr)
            {
                throw std::out_of_range("The requested XML type is not registered.");
            }
            return descriptor->Create();
        }

    private:
        std::unordered_map<std::type_index, TypeDescriptor> byType_;
        std::unordered_map<std::string, std::type_index> byXmlName_;
    };
}
