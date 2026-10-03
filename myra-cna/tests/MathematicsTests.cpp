// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/Mathematics.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
    using Myra::Utility::Mathematics;

    TEST(MathematicsTests, EpsilonAndZeroFollowTheUpstreamTolerance)
    {
        EXPECT_TRUE(Mathematics::EpsilonEquals(1.0F, 1.0F + 0.5e-6F));
        EXPECT_FALSE(Mathematics::EpsilonEquals(1.0F, 1.0F + 2.0e-6F));
        EXPECT_TRUE(Mathematics::IsZero(0.5e-6F));
        EXPECT_FALSE(Mathematics::IsZero(2.0e-6F));
    }

    TEST(MathematicsTests, ClampKeepsValuesWithinTheSpecifiedBounds)
    {
        EXPECT_EQ(Mathematics::Clamp(-1, 0, 10), 0);
        EXPECT_EQ(Mathematics::Clamp(5, 0, 10), 5);
        EXPECT_EQ(Mathematics::Clamp(11, 0, 10), 10);
        EXPECT_FLOAT_EQ(Mathematics::Clamp(-1.0F, 0.0F, 1.0F), 0.0F);
        EXPECT_FLOAT_EQ(Mathematics::Clamp(0.25F, 0.0F, 1.0F), 0.25F);
        EXPECT_FLOAT_EQ(Mathematics::Clamp(2.0F, 0.0F, 1.0F), 1.0F);
    }

    TEST(MathematicsTests, IntegerConversionsRejectNonFiniteAndOutOfRangeValues)
    {
        EXPECT_EQ(Mathematics::TruncateToInt(3.8F), 3);
        EXPECT_EQ(Mathematics::TruncateToInt(-3.8F), -3);
        EXPECT_THROW(static_cast<void>(Mathematics::TruncateToInt(
            std::numeric_limits<float>::infinity())), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(Mathematics::TruncateToInt(
            std::numeric_limits<float>::quiet_NaN())), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(Mathematics::TruncateToInt(
            std::numeric_limits<float>::max())), std::overflow_error);
    }
}
