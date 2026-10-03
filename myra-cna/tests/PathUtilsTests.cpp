// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/PathUtils.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace
{
    TEST(PathUtilsTests, MakesAbsoluteDescendantAndSiblingPathsLexicallyRelative)
    {
        const std::filesystem::path base =
            std::filesystem::temp_directory_path() / "myra-cna-path-utils" / "project";
        const std::filesystem::path descendant = base / "assets" / "skin file.xml";
        const std::filesystem::path sibling = base.parent_path() / "shared" / "font.ttf";

        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo(
            descendant.string(), base.string()), "assets/skin file.xml");
        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo(
            sibling.string(), base.string()), "../shared/font.ttf");
    }

    TEST(PathUtilsTests, PreservesInputWhenEitherPathIsNotAbsolute)
    {
        const std::filesystem::path base =
            std::filesystem::temp_directory_path() / "myra-cna-path-utils";
        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo(
            "assets/skin.xml", base.string()), "assets/skin.xml");
        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo(
            (base / "skin.xml").string(), "relative/base"), (base / "skin.xml").string());
        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo("", base.string()), "");
    }

    TEST(PathUtilsTests, DoesNotRequireThePathsToExist)
    {
        const std::filesystem::path base =
            std::filesystem::temp_directory_path() / "myra-cna-path-utils-never-created";
        const std::filesystem::path target = base / "nested" / "missing.file";
        EXPECT_EQ(Myra::Utility::PathUtils::TryToMakePathRelativeTo(
            target.string(), base.string()), "nested/missing.file");
    }
}
