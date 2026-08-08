// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/AttachedPropertiesRegistry.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "Myra/MML/BaseObject.hpp"
#include "Myra/MML/TypeRegistry.hpp"

namespace Myra::MML
{
    /** @brief Specifies how an attached property change affects widget layout. */
    enum class AttachedPropertyOption : int
    {
        None,
        AffectsArrange,
        AffectsMeasure
    };

    /** @brief Non-generic metadata and object access for an attached property. */
    class BaseAttachedPropertyInfo
    {
    public:
        BaseAttachedPropertyInfo(int id, std::string name, std::type_index ownerType,
            AttachedPropertyOption option, PropertyMetadata metadata = {});
        virtual ~BaseAttachedPropertyInfo() = default;

        [[nodiscard]] int getIdProperty() const noexcept;
        [[nodiscard]] const std::string& getNameProperty() const noexcept;
        [[nodiscard]] std::type_index getOwnerTypeProperty() const noexcept;
        [[nodiscard]] AttachedPropertyOption getOptionProperty() const noexcept;
        [[nodiscard]] const PropertyMetadata& getMetadataProperty() const noexcept;
        [[nodiscard]] virtual std::type_index getPropertyTypeProperty() const noexcept = 0;
        [[nodiscard]] virtual std::any getDefaultValueObjectProperty() const = 0;
        [[nodiscard]] virtual bool IsDefaultValueObject(const std::any& value) const = 0;

        [[nodiscard]] bool HasValue(const BaseObject& object) const noexcept;
        [[nodiscard]] virtual std::any GetValueObject(const BaseObject& object) const = 0;
        virtual void SetValueObject(BaseObject& object, const std::any& value) const = 0;

    private:
        int id_;
        std::string name_;
        std::type_index ownerType_;
        AttachedPropertyOption option_;
        PropertyMetadata metadata_;
    };

    /** @brief Typed metadata and accessor for an attached Myra property. */
    template <std::equality_comparable T>
    class AttachedPropertyInfo final : public BaseAttachedPropertyInfo
    {
    public:
        AttachedPropertyInfo(int id, std::string name, std::type_index ownerType, T defaultValue,
            AttachedPropertyOption option, PropertyMetadata metadata = {})
            : BaseAttachedPropertyInfo(id, std::move(name), ownerType, option, std::move(metadata)),
              defaultValue_(std::move(defaultValue))
        {
        }

        [[nodiscard]] const T& getDefaultValueProperty() const noexcept { return defaultValue_; }
        [[nodiscard]] std::type_index getPropertyTypeProperty() const noexcept override { return typeid(T); }
        [[nodiscard]] std::any getDefaultValueObjectProperty() const override { return defaultValue_; }

        [[nodiscard]] bool IsDefaultValueObject(const std::any& value) const override
        {
            if (std::type_index(value.type()) != typeid(T))
            {
                throw std::invalid_argument("The attached property value has the wrong C++ type.");
            }
            return std::any_cast<const T&>(value) == defaultValue_;
        }

        [[nodiscard]] T GetValue(const BaseObject& object) const
        {
            const auto iterator = object.AttachedPropertiesValues.find(getIdProperty());
            return iterator == object.AttachedPropertiesValues.end() ? defaultValue_ : std::any_cast<T>(iterator->second);
        }

        void SetValue(BaseObject& object, const T& value) const
        {
            if (GetValue(object) == value)
            {
                return;
            }

            object.AttachedPropertiesValues[getIdProperty()] = value;
            object.OnAttachedPropertyLayoutChanged(getOptionProperty());
            object.OnAttachedPropertyChanged(*this);
        }

        [[nodiscard]] std::any GetValueObject(const BaseObject& object) const override { return GetValue(object); }

        void SetValueObject(BaseObject& object, const std::any& value) const override
        {
            if (std::type_index(value.type()) != typeid(T))
            {
                throw std::invalid_argument("The attached property value has the wrong C++ type.");
            }
            SetValue(object, std::any_cast<const T&>(value));
        }

    private:
        T defaultValue_;
    };

    /** @brief Global Myra registry for statically declared attached properties. */
    class AttachedPropertiesRegistry final
    {
    public:
        template <std::equality_comparable T>
        [[nodiscard]] static const AttachedPropertyInfo<T>& Create(std::type_index ownerType, std::string name,
            T defaultValue, AttachedPropertyOption option, PropertyMetadata metadata = {})
        {
            const int id = ReserveId();
            auto property = std::make_unique<AttachedPropertyInfo<T>>(
                id, std::move(name), ownerType, std::move(defaultValue), option, std::move(metadata));
            const AttachedPropertyInfo<T>& result = *property;
            Register(std::move(property));
            return result;
        }

        [[nodiscard]] static std::vector<const BaseAttachedPropertyInfo*> GetPropertiesOfType(
            std::type_index type, const TypeRegistry& typeRegistry);
        [[nodiscard]] static const BaseAttachedPropertyInfo* FindProperty(
            std::type_index type, const std::string& name, const TypeRegistry& typeRegistry);

    private:
        [[nodiscard]] static int ReserveId();
        static void Register(std::unique_ptr<BaseAttachedPropertyInfo> property);
    };
}
