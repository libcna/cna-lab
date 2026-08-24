// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the necessary C++ value-conversion adaptation of Myra MML semantics.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/ValueCodecRegistry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <limits>
#include <system_error>

#include "Myra/Events/EventHandlingStrategy.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/File/FileDialogMode.hpp"
#include "Myra/Graphics2D/UI/Selectors/ISelector.hpp"
#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::MML
{
    namespace
    {
        [[nodiscard]] bool EqualsIgnoringCase(const std::string_view left, const std::string_view right) noexcept
        {
            return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(),
                [](const char a, const char b) {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                        std::tolower(static_cast<unsigned char>(b));
                });
        }

        template<typename T>
        [[nodiscard]] T ParseInteger(std::string_view value)
        {
            value = Detail::TrimCodecValue(value);
            if (value.empty())
            {
                throw std::invalid_argument("An MML integer cannot be empty.");
            }
            if (value.front() == '+')
            {
                value.remove_prefix(1);
                if (value.empty())
                {
                    throw std::invalid_argument("The MML integer has no digits.");
                }
            }

            T result{};
            const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), result);
            if (error != std::errc{} || position != value.data() + value.size())
            {
                throw std::invalid_argument("The MML value is not a valid invariant-culture integer.");
            }
            return result;
        }

        template<typename T>
        [[nodiscard]] T ParseFloating(std::string_view value)
        {
            value = Detail::TrimCodecValue(value);
            if (value == "NaN")
            {
                return std::numeric_limits<T>::quiet_NaN();
            }
            if (value == "Infinity" || value == "+Infinity")
            {
                return std::numeric_limits<T>::infinity();
            }
            if (value == "-Infinity")
            {
                return -std::numeric_limits<T>::infinity();
            }
            if (value.empty())
            {
                throw std::invalid_argument("An MML floating-point value cannot be empty.");
            }
            if (value.front() == '+')
            {
                value.remove_prefix(1);
                if (value.empty())
                {
                    throw std::invalid_argument("The MML floating-point value has no digits.");
                }
            }

            T result{};
            const auto [position, error] = std::from_chars(
                value.data(), value.data() + value.size(), result, std::chars_format::general);
            if (error != std::errc{} || position != value.data() + value.size() || !std::isfinite(result))
            {
                throw std::invalid_argument("The MML value is not a valid invariant-culture floating-point number.");
            }
            return result;
        }

        template<typename T>
        [[nodiscard]] std::string FormatNumber(const T value)
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                if (std::isnan(value))
                {
                    return "NaN";
                }
                if (std::isinf(value))
                {
                    return value < 0 ? "-Infinity" : "Infinity";
                }
            }

            std::array<char, 128> buffer{};
            std::to_chars_result formatted;
            if constexpr (std::is_floating_point_v<T>)
            {
                formatted = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                    std::chars_format::general);
            }
            else
            {
                formatted = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
            }
            if (formatted.ec != std::errc{})
            {
                throw std::runtime_error("Could not format an MML primitive value.");
            }
            return {buffer.data(), formatted.ptr};
        }

        template<typename T>
        class PrimitiveSerializer final : public TypeSerializer<T>
        {
        public:
            [[nodiscard]] T DeserializeT(const std::string& value) const override
            {
                if constexpr (std::same_as<T, std::string>)
                {
                    return value;
                }
                else if constexpr (std::same_as<T, char>)
                {
                    if (value.size() != 1)
                    {
                        throw std::invalid_argument("An MML character must contain exactly one byte.");
                    }
                    return value.front();
                }
                else if constexpr (std::same_as<T, bool>)
                {
                    const std::string_view trimmed = Detail::TrimCodecValue(value);
                    if (EqualsIgnoringCase(trimmed, "True"))
                    {
                        return true;
                    }
                    if (EqualsIgnoringCase(trimmed, "False"))
                    {
                        return false;
                    }
                    throw std::invalid_argument("An MML Boolean must be True or False.");
                }
                else if constexpr (std::is_integral_v<T>)
                {
                    return ParseInteger<T>(value);
                }
                else
                {
                    static_assert(std::is_floating_point_v<T>);
                    return ParseFloating<T>(value);
                }
            }

            [[nodiscard]] std::string SerializeT(const T& value) const override
            {
                if constexpr (std::same_as<T, std::string>)
                {
                    return value;
                }
                else if constexpr (std::same_as<T, char>)
                {
                    return std::string(1, value);
                }
                else if constexpr (std::same_as<T, bool>)
                {
                    return value ? "True" : "False";
                }
                else
                {
                    return FormatNumber(value);
                }
            }
        };

        template<typename T>
        void RegisterPrimitiveAndOptional(ValueCodecRegistry& registry)
        {
            registry.Register(std::make_unique<PrimitiveSerializer<T>>());
            registry.RegisterOptional<T>();
        }

        template<typename T>
        void RegisterEnumAndOptional(ValueCodecRegistry& registry,
            std::initializer_list<EnumValue<T>> values, const bool flags = false)
        {
            registry.RegisterEnum<T>(values, flags);
            registry.RegisterOptional<T>();
        }
    }

    void ValueCodecRegistry::Register(std::unique_ptr<ITypeSerializer> serializer)
    {
        if (!serializer)
        {
            throw std::invalid_argument("An MML value codec cannot be null.");
        }
        const std::type_index type = serializer->getValueTypeProperty();
        if (!serializers_.emplace(type, std::move(serializer)).second)
        {
            throw std::invalid_argument("An MML value codec is already registered for this C++ type.");
        }
    }

    const ITypeSerializer* ValueCodecRegistry::Find(const std::type_index type) const noexcept
    {
        const auto iterator = serializers_.find(type);
        return iterator == serializers_.end() ? nullptr : iterator->second.get();
    }

    std::any ValueCodecRegistry::Deserialize(const std::type_index type, const std::string& value) const
    {
        const ITypeSerializer* serializer = Find(type);
        if (serializer == nullptr)
        {
            throw std::out_of_range("No MML value codec is registered for the requested C++ type.");
        }
        return serializer->Deserialize(value);
    }

    std::string ValueCodecRegistry::Serialize(const std::any& value) const
    {
        const ITypeSerializer* serializer = Find(value.type());
        if (serializer == nullptr)
        {
            throw std::out_of_range("No MML value codec is registered for the supplied C++ value.");
        }
        return serializer->Serialize(value);
    }

    ValueCodecRegistry ValueCodecRegistry::CreateDefault()
    {
        ValueCodecRegistry registry;
        RegisterPrimitiveAndOptional<bool>(registry);
        RegisterPrimitiveAndOptional<char>(registry);
        RegisterPrimitiveAndOptional<signed char>(registry);
        RegisterPrimitiveAndOptional<unsigned char>(registry);
        RegisterPrimitiveAndOptional<short>(registry);
        RegisterPrimitiveAndOptional<unsigned short>(registry);
        RegisterPrimitiveAndOptional<int>(registry);
        RegisterPrimitiveAndOptional<unsigned int>(registry);
        RegisterPrimitiveAndOptional<long>(registry);
        RegisterPrimitiveAndOptional<unsigned long>(registry);
        RegisterPrimitiveAndOptional<long long>(registry);
        RegisterPrimitiveAndOptional<unsigned long long>(registry);
        RegisterPrimitiveAndOptional<float>(registry);
        RegisterPrimitiveAndOptional<double>(registry);
        RegisterPrimitiveAndOptional<std::string>(registry);

#if defined(MYRA_CNA_HAS_CNA_TARGET)
        registry.RegisterAuditedGeometry();
#endif

        RegisterEnumAndOptional<Events::EventHandlingStrategy>(registry,
            {{"EventCapturing", Events::EventHandlingStrategy::EventCapturing},
                {"EventBubbling", Events::EventHandlingStrategy::EventBubbling}});
        RegisterEnumAndOptional<Graphics2D::UI::HorizontalAlignment>(registry,
            {{"Left", Graphics2D::UI::HorizontalAlignment::Left},
                {"Center", Graphics2D::UI::HorizontalAlignment::Center},
                {"Right", Graphics2D::UI::HorizontalAlignment::Right},
                {"Stretch", Graphics2D::UI::HorizontalAlignment::Stretch}});
        RegisterEnumAndOptional<Graphics2D::UI::VerticalAlignment>(registry,
            {{"Top", Graphics2D::UI::VerticalAlignment::Top},
                {"Center", Graphics2D::UI::VerticalAlignment::Center},
                {"Bottom", Graphics2D::UI::VerticalAlignment::Bottom},
                {"Stretch", Graphics2D::UI::VerticalAlignment::Stretch}});
        RegisterEnumAndOptional<Graphics2D::UI::MouseButtons>(registry,
            {{"Left", Graphics2D::UI::MouseButtons::Left},
                {"Middle", Graphics2D::UI::MouseButtons::Middle},
                {"Right", Graphics2D::UI::MouseButtons::Right}});
        RegisterEnumAndOptional<Graphics2D::UI::Orientation>(registry,
            {{"Horizontal", Graphics2D::UI::Orientation::Horizontal},
                {"Vertical", Graphics2D::UI::Orientation::Vertical}});
        RegisterEnumAndOptional<Graphics2D::UI::MouseCursorType>(registry,
            {{"Arrow", Graphics2D::UI::MouseCursorType::Arrow},
                {"IBeam", Graphics2D::UI::MouseCursorType::IBeam},
                {"Wait", Graphics2D::UI::MouseCursorType::Wait},
                {"Crosshair", Graphics2D::UI::MouseCursorType::Crosshair},
                {"WaitArrow", Graphics2D::UI::MouseCursorType::WaitArrow},
                {"SizeNWSE", Graphics2D::UI::MouseCursorType::SizeNWSE},
                {"SizeNESW", Graphics2D::UI::MouseCursorType::SizeNESW},
                {"SizeWE", Graphics2D::UI::MouseCursorType::SizeWE},
                {"SizeNS", Graphics2D::UI::MouseCursorType::SizeNS},
                {"SizeAll", Graphics2D::UI::MouseCursorType::SizeAll},
                {"No", Graphics2D::UI::MouseCursorType::No},
                {"Hand", Graphics2D::UI::MouseCursorType::Hand}});
        RegisterEnumAndOptional<Graphics2D::UI::DragDirection>(registry,
            {{"None", Graphics2D::UI::DragDirection::None},
                {"Vertical", Graphics2D::UI::DragDirection::Vertical},
                {"Horizontal", Graphics2D::UI::DragDirection::Horizontal},
                {"Both", Graphics2D::UI::DragDirection::Both}}, true);
        RegisterEnumAndOptional<Graphics2D::UI::ProportionType>(registry,
            {{"Auto", Graphics2D::UI::ProportionType::Auto},
                {"Part", Graphics2D::UI::ProportionType::Part},
                {"Fill", Graphics2D::UI::ProportionType::Fill},
                {"Pixels", Graphics2D::UI::ProportionType::Pixels}});
        RegisterEnumAndOptional<Graphics2D::UI::ImageResizeMode>(registry,
            {{"Stretch", Graphics2D::UI::ImageResizeMode::Stretch},
                {"KeepAspectRatio", Graphics2D::UI::ImageResizeMode::KeepAspectRatio}});
        RegisterEnumAndOptional<Graphics2D::UI::CheckPosition>(registry,
            {{"Left", Graphics2D::UI::CheckPosition::Left},
                {"Right", Graphics2D::UI::CheckPosition::Right}});
        RegisterEnumAndOptional<Graphics2D::UI::SelectionMode>(registry,
            {{"Single", Graphics2D::UI::SelectionMode::Single},
                {"Multiple", Graphics2D::UI::SelectionMode::Multiple}});
        RegisterEnumAndOptional<Graphics2D::UI::File::FileDialogMode>(registry,
            {{"OpenFile", Graphics2D::UI::File::FileDialogMode::OpenFile},
                {"SaveFile", Graphics2D::UI::File::FileDialogMode::SaveFile},
                {"ChooseFolder", Graphics2D::UI::File::FileDialogMode::ChooseFolder}});

        return registry;
    }
}
