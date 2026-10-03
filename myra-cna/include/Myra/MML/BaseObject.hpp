// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/BaseObject.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <optional>
#include <string>
#include <unordered_map>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/MML/INotifyAttachedPropertyChanged.hpp"
#include "Myra/MML/IItemWithId.hpp"

namespace Myra::MML
{
    enum class AttachedPropertyOption : int;

    /** @brief Base class for objects that support identifiers and attached properties. */
    class BaseObject : public IItemWithId, public INotifyAttachedPropertyChanged
    {
    public:
        /**
         * @brief Values for all attached properties on this object.
         *
         * This remains public to preserve the upstream C# field's role as the
         * backing store consumed by AttachedPropertiesRegistry.
         */
        std::unordered_map<int, std::any> AttachedPropertiesValues;

        /** @brief Raised after the optional identifier changes. */
        Events::MyraEventHandler IdChanged;

        virtual ~BaseObject() = default;

        [[nodiscard]] const std::optional<std::string>& getIdProperty() const noexcept override;
        void setIdProperty(std::optional<std::string> value) override;
        void setIdProperty(std::string value);

        /** @brief Returns custom MML attributes that do not map to a property. */
        [[nodiscard]] const std::unordered_map<std::string, std::string>& getUserDataProperty() const noexcept;
        [[nodiscard]] std::unordered_map<std::string, std::string>& getUserDataProperty() noexcept;

        /** @brief Called after an attached property value changes. */
        void OnAttachedPropertyChanged(const BaseAttachedPropertyInfo& propertyInfo) override;

        /** @brief Applies the layout invalidation represented by an attached property. */
        virtual void OnAttachedPropertyLayoutChanged(AttachedPropertyOption option);

    protected:
        /** @brief Raises IdChanged using the upstream ValueChanged event kind. */
        virtual void OnIdChanged();

    private:
        std::optional<std::string> id_;
        std::unordered_map<std::string, std::string> userData_;
    };
}
