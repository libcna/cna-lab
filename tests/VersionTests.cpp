// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Version.hpp"

#include <gtest/gtest.h>

#include <string_view>

TEST(VersionTests, ReportsBootstrapVersion)
{
    EXPECT_EQ(Myra::Version::Major, 0);
    EXPECT_EQ(Myra::Version::Minor, 1);
    EXPECT_EQ(Myra::Version::Patch, 0);
    EXPECT_EQ(Myra::Version::GetString(), std::string_view("0.1.0-dev"));
}
