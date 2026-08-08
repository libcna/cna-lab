// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/Thickness.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
    using Myra::Graphics2D::Thickness;

    TEST(ThicknessTests, ConstructorsAndComputedPropertiesMatchMyra)
    {
        const Thickness zero;
        EXPECT_EQ(zero, Thickness::Zero);

        const Thickness individual(1, 2, 3, 4);
        EXPECT_EQ(individual.Left, 1);
        EXPECT_EQ(individual.Top, 2);
        EXPECT_EQ(individual.Right, 3);
        EXPECT_EQ(individual.Bottom, 4);
        EXPECT_EQ(individual.getWidthProperty(), 4);
        EXPECT_EQ(individual.getHeightProperty(), 6);
        EXPECT_FALSE(individual.getSameSizeProperty());

        const Thickness twoValues(5, 6);
        EXPECT_EQ(twoValues, Thickness(5, 6, 5, 6));

        const Thickness oneValue(7);
        EXPECT_EQ(oneValue, Thickness(7, 7, 7, 7));
        EXPECT_TRUE(oneValue.getSameSizeProperty());
    }

    TEST(ThicknessTests, UsesUpstreamCompactStringAndParsingFormats)
    {
        EXPECT_EQ(Thickness(5).ToString(), "5");
        EXPECT_EQ(Thickness(5, 6).ToString(), "5, 6");
        EXPECT_EQ(Thickness(1, 2, 3, 4).ToString(), "1, 2, 3, 4");

        EXPECT_EQ(Thickness::FromString(""), Thickness::Zero);
        EXPECT_EQ(Thickness::FromString("5"), Thickness(5));
        EXPECT_EQ(Thickness::FromString(" 5, 6 "), Thickness(5, 6));
        EXPECT_EQ(Thickness::FromString("1, 2, 3, 4"), Thickness(1, 2, 3, 4));
    }

    TEST(ThicknessTests, RejectsUnsupportedOrMalformedText)
    {
        EXPECT_THROW(static_cast<void>(Thickness::FromString("1, 2, 3")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(Thickness::FromString("1px")), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(Thickness::FromString("1,,2,3")), std::invalid_argument);
    }

    TEST(ThicknessTests, EqualityAndHashMatchMyra)
    {
        const Thickness thickness(1, 2, 3, 4);
        EXPECT_TRUE(thickness.Equals(Thickness(1, 2, 3, 4)));
        EXPECT_NE(thickness, Thickness(1, 2, 3, 5));
        EXPECT_EQ(thickness.GetHashCode(), Thickness(1, 2, 3, 4).GetHashCode());
    }
}
