// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Events/EventHandlingStrategy.hpp"

#include <gtest/gtest.h>

TEST(EventHandlingStrategyTests, ProvidesBothUpstreamPropagationModes)
{
    using Myra::Events::EventHandlingStrategy;

    EXPECT_NE(EventHandlingStrategy::EventCapturing, EventHandlingStrategy::EventBubbling);
}
