// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/ValueCodecRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::DragDirection;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::MML::EnumValue;
    using Myra::MML::ValueCodecRegistry;

    enum class LocalMode
    {
        First = 1,
        Second = 2
    };

    TEST(ValueCodecRegistryTests, ConvertsInvariantPrimitivesAndPreservesStrings)
    {
        const ValueCodecRegistry registry = ValueCodecRegistry::CreateDefault();

        EXPECT_EQ(std::any_cast<int>(registry.Deserialize(typeid(int), " +42 ")), 42);
        EXPECT_EQ(std::any_cast<unsigned int>(registry.Deserialize(typeid(unsigned int), "17")), 17U);
        EXPECT_FLOAT_EQ(std::any_cast<float>(registry.Deserialize(typeid(float), " -2.5 ")), -2.5F);
        EXPECT_TRUE(std::any_cast<bool>(registry.Deserialize(typeid(bool), " tRuE ")));
        EXPECT_EQ(std::any_cast<std::string>(registry.Deserialize(typeid(std::string), "  text  ")), "  text  ");

        EXPECT_EQ(registry.Serialize(std::any(false)), "False");
        EXPECT_EQ(registry.Serialize(std::any(-27)), "-27");
        EXPECT_EQ(registry.Serialize(std::any(1.25F)), "1.25");
        EXPECT_EQ(registry.Serialize(std::any(std::numeric_limits<double>::infinity())), "Infinity");
    }

    TEST(ValueCodecRegistryTests, RejectsMalformedPrimitiveAndMissingCodecValues)
    {
        const ValueCodecRegistry registry = ValueCodecRegistry::CreateDefault();

        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(bool), "1")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(unsigned int), "-1")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(int), "12px")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(float), "inf")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(LocalMode), "First")), std::out_of_range);
        EXPECT_THROW(static_cast<void>(registry.Serialize(std::any(LocalMode::First))), std::out_of_range);
    }

    TEST(ValueCodecRegistryTests, WrapsRegisteredValuesInOptionalWithoutInventingANullText)
    {
        const ValueCodecRegistry registry = ValueCodecRegistry::CreateDefault();

        const auto parsed = std::any_cast<std::optional<int>>(
            registry.Deserialize(typeid(std::optional<int>), "9"));
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, 9);
        EXPECT_EQ(registry.Serialize(std::any(std::optional<int>(11))), "11");
        EXPECT_THROW(static_cast<void>(registry.Serialize(std::any(std::optional<int>()))), std::invalid_argument);
    }

    TEST(ValueCodecRegistryTests, UsesExplicitCaseSensitiveEnumNamesAndFlagCombinations)
    {
        const ValueCodecRegistry registry = ValueCodecRegistry::CreateDefault();

        EXPECT_EQ(std::any_cast<HorizontalAlignment>(
            registry.Deserialize(typeid(HorizontalAlignment), "Stretch")), HorizontalAlignment::Stretch);
        EXPECT_THROW(static_cast<void>(registry.Deserialize(typeid(HorizontalAlignment), "stretch")),
            std::invalid_argument);
        EXPECT_EQ(registry.Serialize(std::any(HorizontalAlignment::Center)), "Center");
        EXPECT_EQ(registry.Serialize(std::any(static_cast<HorizontalAlignment>(19))), "19");

        EXPECT_EQ(std::any_cast<DragDirection>(
            registry.Deserialize(typeid(DragDirection), "Vertical, Horizontal")), DragDirection::Both);
        EXPECT_EQ(registry.Serialize(std::any(DragDirection::Both)), "Both");

        const auto optional = std::any_cast<std::optional<HorizontalAlignment>>(
            registry.Deserialize(typeid(std::optional<HorizontalAlignment>), "Right"));
        ASSERT_TRUE(optional.has_value());
        EXPECT_EQ(*optional, HorizontalAlignment::Right);
    }

    TEST(ValueCodecRegistryTests, SupportsFiniteApplicationEnumRegistrationAndRejectsDuplicates)
    {
        ValueCodecRegistry registry;
        registry.RegisterEnum<LocalMode>({{"First", LocalMode::First}, {"Second", LocalMode::Second}});
        registry.RegisterOptional<LocalMode>();
        EXPECT_EQ(std::any_cast<LocalMode>(registry.Deserialize(typeid(LocalMode), "First")), LocalMode::First);
        EXPECT_EQ(registry.Serialize(std::any(std::optional<LocalMode>(LocalMode::Second))), "Second");

        EXPECT_THROW(registry.RegisterEnum<LocalMode>({{"Again", LocalMode::First}}), std::invalid_argument);
        EXPECT_THROW((Myra::MML::EnumSerializer<LocalMode>({})), std::invalid_argument);
        EXPECT_THROW((Myra::MML::EnumSerializer<LocalMode>(
            {{"First", LocalMode::First}, {"First", LocalMode::Second}})), std::invalid_argument);
        EXPECT_THROW(registry.Register(std::unique_ptr<Myra::MML::ITypeSerializer>()), std::invalid_argument);
    }

#if defined(MYRA_CNA_HAS_CNA_TARGET)
    TEST(ValueCodecRegistryTests, IncludesAuditedGeometryWhenCnaIsLinked)
    {
        using Microsoft::Xna::Framework::Vector2;

        const ValueCodecRegistry registry = ValueCodecRegistry::CreateDefault();
        const auto vector = std::any_cast<Vector2>(registry.Deserialize(typeid(Vector2), "1.5, -2"));
        EXPECT_EQ(vector, Vector2(1.5F, -2.0F));
        EXPECT_EQ(registry.Serialize(std::any(vector)), "1.5, -2");
        EXPECT_NE(registry.Find(typeid(std::optional<Vector2>)), nullptr);
    }
#endif
}
