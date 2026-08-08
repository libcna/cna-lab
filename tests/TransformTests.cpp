// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/Transform.hpp"
#include "Myra/Utility/Mathematics.hpp"

#include <gtest/gtest.h>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Graphics2D::Transform;
    using Myra::Utility::Mathematics;

    TEST(TransformTests, AppliesScaleOffsetAndInverseUsingCnaMatrices)
    {
        const Transform transform(Vector2(10.0F, 20.0F), Vector2(0.0F, 0.0F), Vector2(2.0F, 3.0F), 0.0F);

        const Vector2 applied = transform.Apply(Vector2(5.0F, 5.0F));
        EXPECT_FLOAT_EQ(applied.X, 20.0F);
        EXPECT_FLOAT_EQ(applied.Y, 35.0F);

        const Vector2 inverse = transform.InverseApply(applied);
        EXPECT_TRUE(Mathematics::EpsilonEquals(inverse.X, 5.0F));
        EXPECT_TRUE(Mathematics::EpsilonEquals(inverse.Y, 5.0F));

        const Point point = transform.Apply(Point(5, 5));
        EXPECT_EQ(point.X, 20);
        EXPECT_EQ(point.Y, 35);

        const Rectangle rectangle = transform.Apply(Rectangle(0, 0, 10, 5));
        EXPECT_EQ(rectangle.X, 10);
        EXPECT_EQ(rectangle.Y, 20);
        EXPECT_EQ(rectangle.Width, 20);
        EXPECT_EQ(rectangle.Height, 15);
    }

    TEST(TransformTests, ToPointUsesDotNetMidpointToEvenRounding)
    {
        const Point roundedEvenDown = Mathematics::ToPoint(Vector2(2.5F, -2.5F));
        EXPECT_EQ(roundedEvenDown.X, 2);
        EXPECT_EQ(roundedEvenDown.Y, -2);

        const Point roundedEvenUp = Mathematics::ToPoint(Vector2(3.5F, -3.5F));
        EXPECT_EQ(roundedEvenUp.X, 4);
        EXPECT_EQ(roundedEvenUp.Y, -4);
    }
}
