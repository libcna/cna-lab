// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
    TEST(DesktopStubTests, DefaultBoundsFetcherReportsTheMissingLinkedCnaDependency)
    {
        EXPECT_THROW(static_cast<void>(Myra::Graphics2D::UI::Desktop::DefaultBoundsFetcher()), std::logic_error);
    }
} // namespace
