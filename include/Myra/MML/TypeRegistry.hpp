// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the necessary C++ metadata adaptation of Myra MML semantics.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <algorithm>
#include <any>
#include <concepts>
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
#include "Myra/MML/BaseObject.hpp"

namespace Myra::MML
{
    /** @brief Explicit metadata that replaces the relevant C# property attributes. */
    struct PropertyMetadata
    {
        bool Content = false;
        bool DesignerFolded = false;
        bool SkipLoad = false;
        bool SkipSave = false;
        bool XmlIgnore = false;
        bool Obsolete = false;
        bool ExternalAsset = false;
        std::optional<std::string> XmlName;
        std::optional<std::string> StylePropertyPath;
        std::optional<Attributes::RangeAttribute> Range;
        std::optional<Attributes::FilePathAttribute> FilePath;
    };

    /** @brief Shape of an explicitly adapted non-scalar MML property. */
    enum class ComplexPropertyKind
    {
        Single,
        Sequence,
        Dictionary
    };

    /**
     * @brief Non-owning registered object passed from a complex-property adapter to MML saving.
     *
     * `Value` must address the exact C++ object named by `Type`, not merely a base
     * subobject. Polymorphic base-pointer collections can obtain it with
     * `dynamic_cast<const void*>(pointer)` and report `typeid(*pointer)`.
     */
    struct RegisteredObjectView final
    {
        RegisteredObjectView(const void* value, std::type_index type) : Value(value), Type(type) {}

        const void* Value;
        std::type_index Type;
    };

    /**
     * @brief Explicit replacement for the IList/IDictionary/property reflection used by MML.
     *
     * Objects supplied to assign/append/insert callbacks are aliasing shared
     * pointers adjusted to `itemType`; callbacks may safely cast `value.get()`
     * to that declared item type while retaining the shared ownership handle.
     */
    class ComplexPropertyAdapter final
    {
    public:
        using ExistingObjectGetter = std::function<void*(void*)>;
        using ObjectAssigner = std::function<void(void*, const std::shared_ptr<void>&)>;
        using ObjectAppender = std::function<void(void*, const std::shared_ptr<void>&)>;
        using DictionaryInserter = std::function<void(
            void*, const std::string&, const std::shared_ptr<void>&)>;
        using ObjectEnumerator = std::function<std::vector<RegisteredObjectView>(const void*)>;

        [[nodiscard]] static ComplexPropertyAdapter SingleReadOnly(std::type_index itemType,
            ExistingObjectGetter existingObject, ObjectEnumerator enumerate)
        {
            if (!existingObject || !enumerate)
            {
                throw std::invalid_argument("A read-only single-object MML adapter needs get/enumerate callbacks.");
            }
            return ComplexPropertyAdapter(ComplexPropertyKind::Single, itemType,
                std::move(existingObject), {}, {}, {}, std::move(enumerate));
        }

        [[nodiscard]] static ComplexPropertyAdapter SingleWritable(std::type_index itemType,
            ObjectAssigner assign, ObjectEnumerator enumerate)
        {
            if (!assign || !enumerate)
            {
                throw std::invalid_argument("A writable single-object MML adapter needs assign/enumerate callbacks.");
            }
            return ComplexPropertyAdapter(ComplexPropertyKind::Single, itemType,
                {}, std::move(assign), {}, {}, std::move(enumerate));
        }

        [[nodiscard]] static ComplexPropertyAdapter Sequence(std::type_index itemType,
            ObjectAppender append, ObjectEnumerator enumerate)
        {
            if (!append || !enumerate)
            {
                throw std::invalid_argument("A sequence MML adapter needs append/enumerate callbacks.");
            }
            return ComplexPropertyAdapter(ComplexPropertyKind::Sequence, itemType,
                {}, {}, std::move(append), {}, std::move(enumerate));
        }

        [[nodiscard]] static ComplexPropertyAdapter Dictionary(std::type_index itemType,
            DictionaryInserter insert, ObjectEnumerator enumerate)
        {
            if (!insert || !enumerate)
            {
                throw std::invalid_argument("A dictionary MML adapter needs insert/enumerate callbacks.");
            }
            return ComplexPropertyAdapter(ComplexPropertyKind::Dictionary, itemType,
                {}, {}, {}, std::move(insert), std::move(enumerate));
        }

        [[nodiscard]] ComplexPropertyKind getKindProperty() const noexcept { return kind_; }
        [[nodiscard]] std::type_index getItemTypeProperty() const noexcept { return itemType_; }
        [[nodiscard]] bool getCanAssignSingleProperty() const noexcept { return static_cast<bool>(assign_); }

        [[nodiscard]] void* GetExistingObject(void* owner) const
        {
            if (!existingObject_)
            {
                throw std::logic_error("The complex MML property has no existing-object callback.");
            }
            if (owner == nullptr)
            {
                throw std::invalid_argument("A complex MML property owner cannot be null.");
            }
            return existingObject_(owner);
        }

        void AssignObject(void* owner, const std::shared_ptr<void>& value) const
        {
            if (!assign_)
            {
                throw std::logic_error("The complex MML property has no object-assignment callback.");
            }
            if (owner == nullptr)
            {
                throw std::invalid_argument("A complex MML property owner cannot be null.");
            }
            assign_(owner, value);
        }

        void AppendObject(void* owner, const std::shared_ptr<void>& value) const
        {
            if (!append_)
            {
                throw std::logic_error("The complex MML property has no sequence-append callback.");
            }
            if (owner == nullptr)
            {
                throw std::invalid_argument("A complex MML property owner cannot be null.");
            }
            append_(owner, value);
        }

        void InsertObject(void* owner, const std::string& key, const std::shared_ptr<void>& value) const
        {
            if (!insert_)
            {
                throw std::logic_error("The complex MML property has no dictionary-insert callback.");
            }
            if (owner == nullptr)
            {
                throw std::invalid_argument("A complex MML property owner cannot be null.");
            }
            insert_(owner, key, value);
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateObjects(const void* owner) const
        {
            if (owner == nullptr)
            {
                throw std::invalid_argument("A complex MML property owner cannot be null.");
            }
            return enumerate_(owner);
        }

    private:
        ComplexPropertyAdapter(ComplexPropertyKind kind, std::type_index itemType,
            ExistingObjectGetter existingObject, ObjectAssigner assign, ObjectAppender append,
            DictionaryInserter insert, ObjectEnumerator enumerate)
            : kind_(kind), itemType_(itemType), existingObject_(std::move(existingObject)),
              assign_(std::move(assign)), append_(std::move(append)), insert_(std::move(insert)),
              enumerate_(std::move(enumerate))
        {
        }

        ComplexPropertyKind kind_;
        std::type_index itemType_;
        ExistingObjectGetter existingObject_;
        ObjectAssigner assign_;
        ObjectAppender append_;
        DictionaryInserter insert_;
        ObjectEnumerator enumerate_;
    };

    /** @brief Defines a single readable/writable property of a registered C++ type. */
    class PropertyDescriptor final
    {
    public:
        using Value = std::any;
        using Getter = std::function<Value(const void*)>;
        using Setter = std::function<void(void*, const Value&)>;
        using Equality = std::function<bool(const Value&, const Value&)>;
        using NullCheck = std::function<bool(const Value&)>;

        PropertyDescriptor(std::string name, std::type_index valueType, Getter getter = {}, Setter setter = {},
            std::optional<Value> defaultValue = std::nullopt, PropertyMetadata metadata = {}, Equality equality = {},
            NullCheck nullCheck = {}, std::optional<ComplexPropertyAdapter> complexAdapter = std::nullopt)
            : name_(std::move(name)), valueType_(valueType), getter_(std::move(getter)), setter_(std::move(setter)),
              defaultValue_(std::move(defaultValue)), metadata_(std::move(metadata)), equality_(std::move(equality)),
              nullCheck_(std::move(nullCheck)), complexAdapter_(std::move(complexAdapter))
        {
            if (name_.empty())
            {
                throw std::invalid_argument("A property descriptor name cannot be empty.");
            }
            if (metadata_.XmlName && metadata_.XmlName->empty())
            {
                throw std::invalid_argument("A property descriptor XML name cannot be empty.");
            }
            if (defaultValue_ && std::type_index(defaultValue_->type()) != valueType_)
            {
                throw std::invalid_argument("A registered property default has the wrong C++ type.");
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
        [[nodiscard]] const std::optional<std::type_index>& getDeclaringTypeProperty() const noexcept
        {
            return declaringType_;
        }
        [[nodiscard]] bool getCanCompareDefaultProperty() const noexcept { return static_cast<bool>(equality_); }
        [[nodiscard]] bool getCanBeNullProperty() const noexcept { return static_cast<bool>(nullCheck_); }
        [[nodiscard]] const std::optional<ComplexPropertyAdapter>& getComplexAdapterProperty() const noexcept
        {
            return complexAdapter_;
        }

        [[nodiscard]] Value Get(const void* instance) const
        {
            if (!getter_)
            {
                throw std::logic_error("The registered property is not readable.");
            }
            if (instance == nullptr)
            {
                throw std::invalid_argument("A registered property instance cannot be null.");
            }
            Value value = getter_(instance);
            ValidateValueType(value);
            return value;
        }

        void Set(void* instance, const Value& value) const
        {
            if (!setter_)
            {
                throw std::logic_error("The registered property is not writable.");
            }
            if (instance == nullptr)
            {
                throw std::invalid_argument("A registered property instance cannot be null.");
            }
            if (std::type_index(value.type()) != valueType_)
            {
                throw std::invalid_argument("The registered property value has the wrong C++ type.");
            }
            setter_(instance, value);
        }

        [[nodiscard]] bool IsNull(const Value& value) const
        {
            if (!nullCheck_)
            {
                return false;
            }
            ValidateValueType(value);
            return nullCheck_(value);
        }

        [[nodiscard]] bool IsDefaultValue(const Value& value) const
        {
            if (!defaultValue_ || !equality_)
            {
                return false;
            }
            ValidateValueType(value);
            return equality_(value, *defaultValue_);
        }

    private:
        friend class TypeDescriptor;

        void SetDeclaringType(const std::type_index type)
        {
            if (declaringType_)
            {
                throw std::logic_error("A property descriptor is already attached to a type descriptor.");
            }
            declaringType_ = type;
        }

        void ValidateValueType(const Value& value) const
        {
            if (std::type_index(value.type()) != valueType_)
            {
                throw std::invalid_argument("The registered property value has the wrong C++ type.");
            }
        }

        std::string name_;
        std::type_index valueType_;
        Getter getter_;
        Setter setter_;
        std::optional<Value> defaultValue_;
        PropertyMetadata metadata_;
        Equality equality_;
        NullCheck nullCheck_;
        std::optional<ComplexPropertyAdapter> complexAdapter_;
        std::optional<std::type_index> declaringType_;
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
        [[nodiscard]] bool getCanCastToBaseProperty() const noexcept
        {
            return static_cast<bool>(getDirectBase_) && static_cast<bool>(getConstDirectBase_);
        }
        [[nodiscard]] const std::vector<PropertyDescriptor>& getPropertiesProperty() const noexcept { return properties_; }

        /** @brief Enables BaseObject-only MML behavior for this exact registered C++ type. */
        template<typename T>
            requires std::derived_from<T, BaseObject>
        void EnableBaseObjectAccess()
        {
            if (type_ != typeid(T))
            {
                throw std::invalid_argument("The BaseObject adapter type must match the type descriptor.");
            }
            getBaseObject_ = [](void* object) { return static_cast<BaseObject*>(static_cast<T*>(object)); };
            getConstBaseObject_ = [](const void* object) {
                return static_cast<const BaseObject*>(static_cast<const T*>(object));
            };
        }

        /** @brief Registers the pointer adjustment from this exact type to its declared direct base. */
        template<typename T, typename TBase>
            requires std::derived_from<T, TBase>
        void EnableBaseTypeAccess()
        {
            if (type_ != typeid(T) || !baseType_ || *baseType_ != typeid(TBase))
            {
                throw std::invalid_argument("The direct-base adapter must match the type descriptor hierarchy.");
            }
            getDirectBase_ = [](void* object) { return static_cast<TBase*>(static_cast<T*>(object)); };
            getConstDirectBase_ = [](const void* object) {
                return static_cast<const TBase*>(static_cast<const T*>(object));
            };
        }

        void AddProperty(PropertyDescriptor property)
        {
            for (const PropertyDescriptor& existing : properties_)
            {
                if (existing.getNameProperty() == property.getNameProperty() ||
                    existing.getNameProperty() == property.getXmlNameProperty() ||
                    existing.getXmlNameProperty() == property.getNameProperty() ||
                    existing.getXmlNameProperty() == property.getXmlNameProperty())
                {
                    throw std::invalid_argument(
                        "A type descriptor cannot contain duplicate or cross-colliding property names.");
                }
            }
            property.SetDeclaringType(type_);
            properties_.push_back(std::move(property));
        }

        [[nodiscard]] std::shared_ptr<void> Create() const
        {
            if (!factory_)
            {
                throw std::logic_error("The registered type is abstract and cannot be created.");
            }
            std::shared_ptr<void> result = factory_();
            if (!result)
            {
                throw std::runtime_error("The registered type factory returned null.");
            }
            return result;
        }

        [[nodiscard]] BaseObject* GetBaseObject(void* object) const noexcept
        {
            return getBaseObject_ ? getBaseObject_(object) : nullptr;
        }

        [[nodiscard]] const BaseObject* GetBaseObject(const void* object) const noexcept
        {
            return getConstBaseObject_ ? getConstBaseObject_(object) : nullptr;
        }

        [[nodiscard]] void* GetDirectBase(void* object) const
        {
            if (!getDirectBase_)
            {
                throw std::logic_error("The registered derived type has no direct-base pointer adapter.");
            }
            return getDirectBase_(object);
        }

        [[nodiscard]] const void* GetDirectBase(const void* object) const
        {
            if (!getConstDirectBase_)
            {
                throw std::logic_error("The registered derived type has no direct-base pointer adapter.");
            }
            return getConstDirectBase_(object);
        }

    private:
        std::string name_;
        std::string xmlName_;
        std::type_index type_;
        Factory factory_;
        std::optional<std::type_index> baseType_;
        std::vector<PropertyDescriptor> properties_;
        std::function<BaseObject*(void*)> getBaseObject_;
        std::function<const BaseObject*(const void*)> getConstBaseObject_;
        std::function<void*(void*)> getDirectBase_;
        std::function<const void*(const void*)> getConstDirectBase_;
    };

    /** @brief Explicit registry used by MML and PropertyGrid instead of .NET reflection. */
    class TypeRegistry final
    {
    public:
        void Register(TypeDescriptor descriptor)
        {
            const std::type_index type = descriptor.getTypeProperty();
            const std::string name = descriptor.getNameProperty();
            const std::string xmlName = descriptor.getXmlNameProperty();
            if (byType_.contains(type) || byName_.contains(name) || byXmlName_.contains(xmlName) ||
                (name != xmlName && (byXmlName_.contains(name) || byName_.contains(xmlName))))
            {
                throw std::invalid_argument("The type, C++ name, or XML name is already registered.");
            }
            if (descriptor.getBaseTypeProperty() && !byType_.contains(*descriptor.getBaseTypeProperty()))
            {
                throw std::invalid_argument("A registered base type must be registered first.");
            }
            if (descriptor.getBaseTypeProperty() && !descriptor.getCanCastToBaseProperty())
            {
                throw std::invalid_argument("A registered derived type needs a direct-base pointer adapter.");
            }
            if (descriptor.getBaseTypeProperty())
            {
                std::vector<const PropertyDescriptor*> effectiveProperties =
                    GetPropertiesIncludingBase(*descriptor.getBaseTypeProperty());
                for (const PropertyDescriptor& property : descriptor.getPropertiesProperty())
                {
                    std::optional<size_t> overriddenIndex;
                    for (size_t index = 0; index < effectiveProperties.size(); ++index)
                    {
                        const PropertyDescriptor* inheritedProperty = effectiveProperties[index];
                        const bool overridesSameIdentity =
                            property.getNameProperty() == inheritedProperty->getNameProperty() ||
                            property.getXmlNameProperty() == inheritedProperty->getXmlNameProperty();
                        if (!overridesSameIdentity)
                        {
                            continue;
                        }
                        if (overriddenIndex)
                        {
                            throw std::invalid_argument(
                                "A derived property cannot override multiple inherited MML properties.");
                        }
                        overriddenIndex = index;
                    }

                    for (size_t index = 0; index < effectiveProperties.size(); ++index)
                    {
                        if (overriddenIndex && index == *overriddenIndex)
                        {
                            continue;
                        }
                        const PropertyDescriptor* inheritedProperty = effectiveProperties[index];
                        if (property.getNameProperty() == inheritedProperty->getNameProperty() ||
                            property.getNameProperty() == inheritedProperty->getXmlNameProperty() ||
                            property.getXmlNameProperty() == inheritedProperty->getNameProperty() ||
                            property.getXmlNameProperty() == inheritedProperty->getXmlNameProperty())
                        {
                            throw std::invalid_argument(
                                "A derived property collides with the effective inherited MML namespace.");
                        }
                    }

                    if (overriddenIndex)
                    {
                        effectiveProperties[*overriddenIndex] = &property;
                    }
                    else
                    {
                        effectiveProperties.push_back(&property);
                    }
                }
            }
            for (const PropertyDescriptor& property : descriptor.getPropertiesProperty())
            {
                if (property.getComplexAdapterProperty() &&
                    !byType_.contains(property.getComplexAdapterProperty()->getItemTypeProperty()) &&
                    property.getComplexAdapterProperty()->getItemTypeProperty() != type)
                {
                    throw std::invalid_argument(
                        "A complex-property item type must be registered before its owner type.");
                }
            }

            if (!byType_.emplace(type, std::move(descriptor)).second)
            {
                throw std::logic_error("TypeRegistry rejected a duplicate type unexpectedly.");
            }
            byName_.emplace(name, type);
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

        [[nodiscard]] const TypeDescriptor* FindByName(const std::string& name) const noexcept
        {
            const auto iterator = byName_.find(name);
            return iterator == byName_.end() ? nullptr : FindByType(iterator->second);
        }

        [[nodiscard]] bool IsTypeOrDerivedFrom(
            const std::type_index type, const std::type_index expectedBase) const noexcept
        {
            for (const TypeDescriptor* current = FindByType(type); current != nullptr;)
            {
                if (current->getTypeProperty() == expectedBase)
                {
                    return true;
                }
                current = current->getBaseTypeProperty() ? FindByType(*current->getBaseTypeProperty()) : nullptr;
            }
            return false;
        }

        [[nodiscard]] void* CastObject(
            void* object, const std::type_index type, const std::type_index expectedBase) const
        {
            if (object == nullptr || type == expectedBase)
            {
                return object;
            }
            void* result = object;
            for (const TypeDescriptor* current = FindByType(type); current != nullptr;)
            {
                if (!current->getBaseTypeProperty())
                {
                    break;
                }
                result = current->GetDirectBase(result);
                current = FindByType(*current->getBaseTypeProperty());
                if (current != nullptr && current->getTypeProperty() == expectedBase)
                {
                    return result;
                }
            }
            throw std::invalid_argument("The registered object type is not derived from the requested base type.");
        }

        [[nodiscard]] const void* CastObject(
            const void* object, const std::type_index type, const std::type_index expectedBase) const
        {
            if (object == nullptr || type == expectedBase)
            {
                return object;
            }
            const void* result = object;
            for (const TypeDescriptor* current = FindByType(type); current != nullptr;)
            {
                if (!current->getBaseTypeProperty())
                {
                    break;
                }
                result = current->GetDirectBase(result);
                current = FindByType(*current->getBaseTypeProperty());
                if (current != nullptr && current->getTypeProperty() == expectedBase)
                {
                    return result;
                }
            }
            throw std::invalid_argument("The registered object type is not derived from the requested base type.");
        }

        [[nodiscard]] std::shared_ptr<void> CastObject(const std::shared_ptr<void>& object,
            const std::type_index type, const std::type_index expectedBase) const
        {
            return std::shared_ptr<void>(object, CastObject(object.get(), type, expectedBase));
        }

        [[nodiscard]] PropertyDescriptor::Value GetPropertyValue(const void* object,
            const std::type_index objectType, const PropertyDescriptor& property) const
        {
            if (!property.getDeclaringTypeProperty())
            {
                throw std::logic_error("The property descriptor has no declaring type.");
            }
            return property.Get(CastObject(object, objectType, *property.getDeclaringTypeProperty()));
        }

        void SetPropertyValue(void* object, const std::type_index objectType,
            const PropertyDescriptor& property, const PropertyDescriptor::Value& value) const
        {
            if (!property.getDeclaringTypeProperty())
            {
                throw std::logic_error("The property descriptor has no declaring type.");
            }
            property.Set(CastObject(object, objectType, *property.getDeclaringTypeProperty()), value);
        }

        [[nodiscard]] void* GetPropertyOwner(void* object, const std::type_index objectType,
            const PropertyDescriptor& property) const
        {
            if (object == nullptr)
            {
                throw std::invalid_argument("A registered property owner cannot be null.");
            }
            if (!property.getDeclaringTypeProperty())
            {
                throw std::logic_error("The property descriptor has no declaring type.");
            }
            return CastObject(object, objectType, *property.getDeclaringTypeProperty());
        }

        [[nodiscard]] const void* GetPropertyOwner(const void* object, const std::type_index objectType,
            const PropertyDescriptor& property) const
        {
            if (object == nullptr)
            {
                throw std::invalid_argument("A registered property owner cannot be null.");
            }
            if (!property.getDeclaringTypeProperty())
            {
                throw std::logic_error("The property descriptor has no declaring type.");
            }
            return CastObject(object, objectType, *property.getDeclaringTypeProperty());
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
                    const auto existing = std::find_if(result.begin(), result.end(),
                        [&property](const PropertyDescriptor* candidate) {
                            return candidate->getNameProperty() == property.getNameProperty() ||
                                candidate->getXmlNameProperty() == property.getXmlNameProperty();
                        });
                    if (existing == result.end())
                    {
                        result.push_back(&property);
                    }
                    else
                    {
                        *existing = &property;
                    }
                }
            }
            return result;
        }

        [[nodiscard]] const PropertyDescriptor* FindPropertyByName(
            const std::type_index type, const std::string& name) const
        {
            const std::vector<const PropertyDescriptor*> properties = GetPropertiesIncludingBase(type);
            for (auto iterator = properties.rbegin(); iterator != properties.rend(); ++iterator)
            {
                if ((*iterator)->getNameProperty() == name)
                {
                    return *iterator;
                }
            }
            return nullptr;
        }

        [[nodiscard]] const PropertyDescriptor* FindPropertyByXmlName(
            const std::type_index type, const std::string& xmlName) const
        {
            const std::vector<const PropertyDescriptor*> properties = GetPropertiesIncludingBase(type);
            for (auto iterator = properties.rbegin(); iterator != properties.rend(); ++iterator)
            {
                if ((*iterator)->getXmlNameProperty() == xmlName)
                {
                    return *iterator;
                }
            }
            return nullptr;
        }

        /** @brief Returns the registered type first, followed by each registered base type. */
        [[nodiscard]] std::vector<std::type_index> GetTypesIncludingBase(const std::type_index type) const
        {
            const TypeDescriptor* descriptor = FindByType(type);
            if (descriptor == nullptr)
            {
                throw std::out_of_range("The requested type is not registered.");
            }

            std::vector<std::type_index> result;
            for (const TypeDescriptor* current = descriptor; current != nullptr;)
            {
                result.push_back(current->getTypeProperty());
                current = current->getBaseTypeProperty() ? FindByType(*current->getBaseTypeProperty()) : nullptr;
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
        std::unordered_map<std::string, std::type_index> byName_;
        std::unordered_map<std::string, std::type_index> byXmlName_;
    };
}
