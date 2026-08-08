// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/StringUtils.hpp"

#include <gtest/gtest.h>

TEST(StringUtilsTests, ReturnsZeroForAnEmptyStringAndItsCharacterCountOtherwise)
{
    EXPECT_EQ(Myra::Utility::StringUtils::Length(""), 0U);
    EXPECT_EQ(Myra::Utility::StringUtils::Length("Myra"), 4U);
}
