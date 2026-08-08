// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the necessary C++ value-conversion adaptation of Myra MML semantics.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <any>
#include <charconv>
#include <concepts>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Myra/MML/TypeSerializers.hpp"

namespace Myra::MML
{
    namespace Detail
    {
        [[nodiscard]] inline std::string_view TrimCodecValue(const std::string_view value) noexcept
        {
            const size_t first = value.find_first_not_of(" \t\r\n");
            if (first == std::string_view::npos)
            {
                return {};
            }
            return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }

        template<typename Integer>
        [[nodiscard]] Integer ParseEnumInteger(std::string_view value)
        {
            static_assert(std::is_integral_v<Integer>);
            value = TrimCodecValue(value);
            if (value.empty())
            {
                throw std::invalid_argument("An MML enum value cannot be empty.");
            }

            bool hasLeadingPlus = value.front() == '+';
            if (hasLeadingPlus)
            {
                value.remove_prefix(1);
                if (value.empty())
                {
                    throw std::invalid_argument("The MML enum value has no digits.");
                }
            }

            if constexpr (std::is_signed_v<Integer>)
            {
                long long parsed = 0;
                const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                if (error != std::errc{} || position != value.data() + value.size() ||
                    parsed < static_cast<long long>(std::numeric_limits<Integer>::min()) ||
                    parsed > static_cast<long long>(std::numeric_limits<Integer>::max()))
                {
                    throw std::invalid_argument("The MML enum value is not a valid underlying integer.");
                }
                return static_cast<Integer>(parsed);
            }
            else
            {
                if (value.front() == '-')
                {
                    throw std::invalid_argument("A negative MML enum value cannot use an unsigned underlying type.");
                }
                unsigned long long parsed = 0;
                const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                if (error != std::errc{} || position != value.data() + value.size() ||
                    parsed > static_cast<unsigned long long>(std::numeric_limits<Integer>::max()))
                {
                    throw std::invalid_argument("The MML enum value is not a valid underlying integer.");
                }
                return static_cast<Integer>(parsed);
            }
        }

        template<typename Integer>
        [[nodiscard]] std::string FormatEnumInteger(const Integer value)
        {
            static_assert(std::is_integral_v<Integer>);
            char buffer[std::numeric_limits<Integer>::digits10 + 4]{};
            const auto [position, error] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            if (error != std::errc{})
            {
                throw std::runtime_error("Could not format an MML enum value.");
            }
            return {buffer, position};
        }
    }

    /** @brief One explicit textual name for a C++ enum value. */
    template<typename T>
        requires std::is_enum_v<T>
    struct EnumValue final
    {
        std::string Name;
        T Value;
    };

    /** @brief Explicit, non-reflective counterpart of Myra's enum conversion. */
    template<typename T>
        requires std::is_enum_v<T>
    class EnumSerializer final : public TypeSerializer<T>
    {
    public:
        EnumSerializer(std::initializer_list<EnumValue<T>> values, const bool flags = false)
            : values_(values), flags_(flags)
        {
            if (values_.empty())
            {
                throw std::invalid_argument("An MML enum serializer needs at least one named value.");
            }
            for (size_t index = 0; index < values_.size(); ++index)
            {
                if (values_[index].Name.empty() || values_[index].Name.find(',') != std::string::npos)
                {
                    throw std::invalid_argument("An MML enum name cannot be empty or contain a comma.");
                }
                for (size_t other = 0; other < index; ++other)
                {
                    if (values_[other].Name == values_[index].Name)
                    {
                        throw std::invalid_argument("An MML enum serializer cannot contain duplicate names.");
                    }
                }
            }
        }

        [[nodiscard]] T DeserializeT(const std::string& text) const override
        {
            const std::string_view value = Detail::TrimCodecValue(text);
            for (const EnumValue<T>& item : values_)
            {
                if (value == item.Name)
                {
                    return item.Value;
                }
            }

            if (flags_ && value.find(',') != std::string_view::npos)
            {
                using Underlying = std::underlying_type_t<T>;
                using Unsigned = std::make_unsigned_t<Underlying>;
                Unsigned combined = 0;
                std::string_view remaining = value;
                while (!remaining.empty())
                {
                    const size_t separator = remaining.find(',');
                    const std::string_view name = Detail::TrimCodecValue(
                        separator == std::string_view::npos ? remaining : remaining.substr(0, separator));
                    bool found = false;
                    for (const EnumValue<T>& item : values_)
                    {
                        if (name == item.Name)
                        {
                            combined |= static_cast<Unsigned>(static_cast<Underlying>(item.Value));
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        throw std::invalid_argument("The MML flags value contains an unknown enum name.");
                    }
                    if (separator == std::string_view::npos)
                    {
                        break;
                    }
                    remaining.remove_prefix(separator + 1);
                    if (remaining.empty())
                    {
                        throw std::invalid_argument("The MML flags value ends with an empty name.");
                    }
                }
                return static_cast<T>(static_cast<Underlying>(combined));
            }

            using Underlying = std::underlying_type_t<T>;
            return static_cast<T>(Detail::ParseEnumInteger<Underlying>(value));
        }

        [[nodiscard]] std::string SerializeT(const T& value) const override
        {
            for (const EnumValue<T>& item : values_)
            {
                if (item.Value == value)
                {
                    return item.Name;
                }
            }

            using Underlying = std::underlying_type_t<T>;
            if (flags_)
            {
                using Unsigned = std::make_unsigned_t<Underlying>;
                Unsigned remaining = static_cast<Unsigned>(static_cast<Underlying>(value));
                std::string result;
                for (const EnumValue<T>& item : values_)
                {
                    const Unsigned bits = static_cast<Unsigned>(static_cast<Underlying>(item.Value));
                    if (bits != 0 && (remaining & bits) == bits)
                    {
                        if (!result.empty())
                        {
                            result += ", ";
                        }
                        result += item.Name;
                        remaining &= ~bits;
                    }
                }
                if (remaining == 0 && !result.empty())
                {
                    return result;
                }
            }
            return Detail::FormatEnumInteger(static_cast<Underlying>(value));
        }

    private:
        std::vector<EnumValue<T>> values_;
        bool flags_;
    };

    /** @brief Adapts an underlying value codec to a C++ nullable value type. */
    template<typename T>
    class OptionalSerializer final : public TypeSerializer<std::optional<T>>
    {
    public:
        explicit OptionalSerializer(const TypeSerializer<T>& valueSerializer) noexcept
            : valueSerializer_(valueSerializer)
        {
        }

        [[nodiscard]] std::optional<T> DeserializeT(const std::string& value) const override
        {
            return std::any_cast<T>(valueSerializer_.Deserialize(value));
        }

        [[nodiscard]] std::string SerializeT(const std::optional<T>& value) const override
        {
            if (!value)
            {
                throw std::invalid_argument(
                    "A null optional has no MML text; represent it by omitting the XML property.");
            }
            return valueSerializer_.Serialize(std::any(*value));
        }

    private:
        const TypeSerializer<T>& valueSerializer_;
    };

    /** @brief Registry for all scalar text conversions consumed by MML contexts. */
    class ValueCodecRegistry final
    {
    public:
        ValueCodecRegistry() = default;
        ValueCodecRegistry(const ValueCodecRegistry&) = delete;
        ValueCodecRegistry& operator=(const ValueCodecRegistry&) = delete;
        ValueCodecRegistry(ValueCodecRegistry&&) noexcept = default;
        ValueCodecRegistry& operator=(ValueCodecRegistry&&) noexcept = default;

        void Register(std::unique_ptr<ITypeSerializer> serializer);

        template<typename T>
        void RegisterOptional()
        {
            const ITypeSerializer* serializer = Find(typeid(T));
            const auto* typed = dynamic_cast<const TypeSerializer<T>*>(serializer);
            if (typed == nullptr)
            {
                throw std::invalid_argument("The underlying MML value codec must be registered first.");
            }
            Register(std::make_unique<OptionalSerializer<T>>(*typed));
        }

        template<typename T>
            requires std::is_enum_v<T>
        void RegisterEnum(std::initializer_list<EnumValue<T>> values, const bool flags = false)
        {
            Register(std::make_unique<EnumSerializer<T>>(values, flags));
        }

        [[nodiscard]] const ITypeSerializer* Find(std::type_index type) const noexcept;
        [[nodiscard]] std::any Deserialize(std::type_index type, const std::string& value) const;
        [[nodiscard]] std::string Serialize(const std::any& value) const;

        /**
         * @brief Adds the Vector2, Thickness, and Rectangle codecs that require CNA linkage.
         *
         * `CreateDefault` calls this automatically when Myra-CNA is built against a CNA
         * target. A deferred-linkage consumer may call it after supplying the CNA symbols.
         */
        void RegisterAuditedGeometry();

        /**
         * @brief Creates all scalar codecs and, when CNA is linked by CMake, audited geometry codecs.
         */
        [[nodiscard]] static ValueCodecRegistry CreateDefault();

    private:
        std::unordered_map<std::type_index, std::unique_ptr<ITypeSerializer>> serializers_;
    };
}
