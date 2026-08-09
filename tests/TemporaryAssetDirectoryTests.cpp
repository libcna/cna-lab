// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "TemporaryAssetDirectory.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
    using Myra::Tests::TemporaryAssetDirectory;

    TEST(TemporaryAssetDirectoryTests, CreatesNestedAssetsAndCleansItsOwnedTree)
    {
        std::filesystem::path ownedPath;
        {
            TemporaryAssetDirectory assets = TemporaryAssetDirectory::ForCurrentTest();
            ownedPath = assets.getPathProperty();
            EXPECT_TRUE(std::filesystem::is_directory(ownedPath));

            const std::filesystem::path asset = assets.WriteText("nested/skin.xml", "<Project />");
            EXPECT_EQ(asset, ownedPath / "nested" / "skin.xml");
            std::ifstream stream(asset, std::ios::binary);
            const std::string contents{
                std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
            EXPECT_EQ(contents, "<Project />");

            EXPECT_THROW(static_cast<void>(assets.Resolve({})), std::invalid_argument);
            EXPECT_THROW(static_cast<void>(assets.Resolve("/absolute.xml")), std::invalid_argument);
            EXPECT_THROW(static_cast<void>(assets.Resolve("../escape.xml")), std::invalid_argument);
            EXPECT_THROW(static_cast<void>(assets.Resolve("nested/../../escape.xml")), std::invalid_argument);
        }
        EXPECT_FALSE(std::filesystem::exists(ownedPath));
    }

    TEST(TemporaryAssetDirectoryTests, ReusesAStablePathWithoutRetainingStaleAssets)
    {
        std::filesystem::path firstPath;
        {
            TemporaryAssetDirectory first = TemporaryAssetDirectory::ForCurrentTest();
            firstPath = first.getPathProperty();
            static_cast<void>(first.WriteText("stale.txt", "stale"));
        }

        TemporaryAssetDirectory second = TemporaryAssetDirectory::ForCurrentTest();
        EXPECT_EQ(second.getPathProperty(), firstPath);
        EXPECT_FALSE(std::filesystem::exists(second.Resolve("stale.txt")));
    }

    TEST(TemporaryAssetDirectoryTests, MoveTransfersCleanupOwnership)
    {
        std::filesystem::path ownedPath;
        {
            TemporaryAssetDirectory first = TemporaryAssetDirectory::ForCurrentTest();
            ownedPath = first.getPathProperty();
            TemporaryAssetDirectory second = std::move(first);
            EXPECT_EQ(second.getPathProperty(), ownedPath);
            EXPECT_TRUE(first.getPathProperty().empty());
        }
        EXPECT_FALSE(std::filesystem::exists(ownedPath));
    }
}
