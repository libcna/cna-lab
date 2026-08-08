// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/TypeSerializers.hpp"

#include <gtest/gtest.h>

#include <any>
#include <string>
#include <typeindex>

namespace
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Graphics2D::Thickness;
    using Myra::MML::RectangleSerializer;
    using Myra::MML::ThicknessSerializer;
    using Myra::MML::TypeSerializers;
    using Myra::MML::Vector2Serializer;

    TEST(TypeSerializersTests, RoundTripsTheMyraVectorThicknessAndRectangleForms)
    {
        Vector2Serializer vector;
        EXPECT_EQ(vector.DeserializeT("1.5, -2.25"), Vector2(1.5F, -2.25F));
        EXPECT_EQ(vector.SerializeT(Vector2(1.5F, -2.25F)), "1.5, -2.25");

        ThicknessSerializer thickness;
        EXPECT_EQ(thickness.DeserializeT("1, 2, 3, 4"), Thickness(1, 2, 3, 4));
        EXPECT_EQ(thickness.SerializeT(Thickness(2, 3)), "2, 3");

        RectangleSerializer rectangle;
        EXPECT_EQ(rectangle.DeserializeT("1, -2, 30, 40"), Rectangle(1, -2, 30, 40));
        EXPECT_EQ(rectangle.SerializeT(Rectangle(1, -2, 30, 40)), "1, -2, 30, 40");
    }

    TEST(TypeSerializersTests, FindsTypeErasedSerializersAndRejectsMalformedOrWrongValues)
    {
        const auto* vector = TypeSerializers::Find(typeid(Vector2));
        ASSERT_NE(vector, nullptr);
        EXPECT_EQ(std::any_cast<Vector2>(vector->Deserialize("3, 4")), Vector2(3.0F, 4.0F));
        EXPECT_EQ(vector->Serialize(std::any(Vector2(3.0F, 4.0F))), "3, 4");
        EXPECT_EQ(TypeSerializers::Find(typeid(std::string)), nullptr);

        RectangleSerializer rectangle;
        EXPECT_THROW(static_cast<void>(rectangle.DeserializeT("1, 2, 3")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(rectangle.DeserializeT("1, 2, three, 4")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(vector->Serialize(std::any(3))), std::invalid_argument);
    }
}
