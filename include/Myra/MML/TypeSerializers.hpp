// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/TypeSerializers.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Graphics2D/Thickness.hpp"

namespace Myra::MML
{
    /** @brief Type-erased MML conversion between a scalar C++ value and text. */
    class ITypeSerializer
    {
    public:
        virtual ~ITypeSerializer() = default;

        [[nodiscard]] virtual std::type_index getValueTypeProperty() const noexcept = 0;
        [[nodiscard]] virtual std::any Deserialize(const std::string& value) const = 0;
        [[nodiscard]] virtual std::string Serialize(const std::any& value) const = 0;
    };

    /** @brief Typed counterpart of Myra's `TypeSerializer<T>`. */
    template<typename T>
    class TypeSerializer : public ITypeSerializer
    {
    public:
        [[nodiscard]] std::type_index getValueTypeProperty() const noexcept override { return typeid(T); }
        [[nodiscard]] std::any Deserialize(const std::string& value) const override { return DeserializeT(value); }

        [[nodiscard]] std::string Serialize(const std::any& value) const override
        {
            if (std::type_index(value.type()) != typeid(T))
            {
                throw std::invalid_argument("The MML serializer received a value of the wrong C++ type.");
            }
            return SerializeT(std::any_cast<const T&>(value));
        }

        [[nodiscard]] virtual T DeserializeT(const std::string& value) const = 0;
        [[nodiscard]] virtual std::string SerializeT(const T& value) const = 0;
    };

    /** @brief Invariant-culture MML serializer for CNA `Vector2`. */
    class Vector2Serializer final : public TypeSerializer<Microsoft::Xna::Framework::Vector2>
    {
    public:
        [[nodiscard]] Microsoft::Xna::Framework::Vector2 DeserializeT(const std::string& value) const override;
        [[nodiscard]] std::string SerializeT(const Microsoft::Xna::Framework::Vector2& value) const override;
    };

    /** @brief MML serializer for Myra `Thickness`. */
    class ThicknessSerializer final : public TypeSerializer<Graphics2D::Thickness>
    {
    public:
        [[nodiscard]] Graphics2D::Thickness DeserializeT(const std::string& value) const override;
        [[nodiscard]] std::string SerializeT(const Graphics2D::Thickness& value) const override;
    };

    /** @brief Invariant-culture MML serializer for CNA `Rectangle`. */
    class RectangleSerializer final : public TypeSerializer<Microsoft::Xna::Framework::Rectangle>
    {
    public:
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle DeserializeT(const std::string& value) const override;
        [[nodiscard]] std::string SerializeT(const Microsoft::Xna::Framework::Rectangle& value) const override;
    };

    /** @brief Finds the currently audited built-in Myra MML serializers. */
    class TypeSerializers final
    {
    public:
        [[nodiscard]] static const ITypeSerializer* Find(std::type_index type) noexcept;
    };
}
