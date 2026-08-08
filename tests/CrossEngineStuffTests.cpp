// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/CrossEngineStuff.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/CrossEngineStuff.hpp"

#include <gtest/gtest.h>

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Myra::Utility::CrossEngineStuff;

    TEST(CrossEngineStuffTests, MultiplyColorUsesCnaChannelScaling)
    {
        const Color source(200, 100, 50, 240);
        const Color result = CrossEngineStuff::MultiplyColor(source, 0.5F);

        EXPECT_EQ(result.getRProperty(), 100);
        EXPECT_EQ(result.getGProperty(), 50);
        EXPECT_EQ(result.getBProperty(), 25);
        EXPECT_EQ(result.getAProperty(), 120);
    }

    TEST(CrossEngineStuffTests, MultiplyColorPreservesCnaClampingEndpoints)
    {
        const Color source(200, 100, 50, 240);

        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 0.0F), Color(0, 0, 0, 0));
        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 1.0F), source);
        EXPECT_EQ(CrossEngineStuff::MultiplyColor(source, 2.0F), Color(255, 200, 100, 255));
    }
}
