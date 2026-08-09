// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <gtest/gtest.h>

namespace Myra::Tests
{
    /** @brief Owns a deterministic, per-build and per-test temporary asset tree. */
    class TemporaryAssetDirectory final
    {
    public:
        [[nodiscard]] static TemporaryAssetDirectory ForCurrentTest()
        {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            if (testInfo == nullptr)
            {
                throw std::logic_error(
                    "TemporaryAssetDirectory::ForCurrentTest must be called from a running test.");
            }

            const std::filesystem::path root = std::filesystem::temp_directory_path()
                / "myra-cna-tests" / HashPath(std::filesystem::current_path());
            return TemporaryAssetDirectory(root
                / SanitizeName(std::string(testInfo->test_suite_name()) + '.' + testInfo->name()));
        }

        ~TemporaryAssetDirectory() { Cleanup(); }

        TemporaryAssetDirectory(const TemporaryAssetDirectory&) = delete;
        TemporaryAssetDirectory& operator=(const TemporaryAssetDirectory&) = delete;

        TemporaryAssetDirectory(TemporaryAssetDirectory&& other) noexcept
            : path_(std::exchange(other.path_, {}))
        {
        }

        TemporaryAssetDirectory& operator=(TemporaryAssetDirectory&& other) noexcept
        {
            if (this != &other)
            {
                Cleanup();
                path_ = std::exchange(other.path_, {});
            }
            return *this;
        }

        [[nodiscard]] const std::filesystem::path& getPathProperty() const noexcept
        {
            return path_;
        }

        [[nodiscard]] std::filesystem::path Resolve(
            const std::filesystem::path& relativePath) const
        {
            if (relativePath.empty() || relativePath.is_absolute())
            {
                throw std::invalid_argument("Temporary asset paths must be non-empty and relative.");
            }

            const std::filesystem::path normalized = relativePath.lexically_normal();
            for (const std::filesystem::path& component : normalized)
            {
                if (component == "..")
                {
                    throw std::invalid_argument(
                        "Temporary asset paths must remain inside the test directory.");
                }
            }
            return path_ / normalized;
        }

        [[nodiscard]] std::filesystem::path WriteText(
            const std::filesystem::path& relativePath, const std::string_view contents) const
        {
            const std::filesystem::path destination = Resolve(relativePath);
            std::filesystem::create_directories(destination.parent_path());

            std::ofstream stream(destination, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                throw std::runtime_error(
                    "Could not create temporary asset '" + destination.string() + "'.");
            }
            stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
            if (!stream)
            {
                throw std::runtime_error(
                    "Could not write temporary asset '" + destination.string() + "'.");
            }
            return destination;
        }

    private:
        explicit TemporaryAssetDirectory(std::filesystem::path path) : path_(std::move(path))
        {
            std::error_code error;
            std::filesystem::remove_all(path_, error);
            if (error)
            {
                throw std::filesystem::filesystem_error(
                    "Could not clear the temporary asset directory", path_, error);
            }
            std::filesystem::create_directories(path_);
        }

        [[nodiscard]] static std::string SanitizeName(const std::string_view name)
        {
            std::string result;
            result.reserve(name.size());
            for (const unsigned char character : name)
            {
                result.push_back(std::isalnum(character) != 0 || character == '-'
                        || character == '_' || character == '.'
                    ? static_cast<char>(character)
                    : '_');
            }
            return result.empty() || result == "." || result == ".." ? "unnamed-test" : result;
        }

        [[nodiscard]] static std::string HashPath(const std::filesystem::path& path)
        {
            constexpr std::uint64_t offset = 14695981039346656037ULL;
            constexpr std::uint64_t prime = 1099511628211ULL;
            std::uint64_t hash = offset;
            for (const unsigned char character : path.lexically_normal().generic_string())
            {
                hash ^= character;
                hash *= prime;
            }

            std::ostringstream stream;
            stream << std::hex << std::setfill('0') << std::setw(16) << hash;
            return stream.str();
        }

        void Cleanup() noexcept
        {
            if (path_.empty())
            {
                return;
            }
            std::error_code ignored;
            std::filesystem::remove_all(path_, ignored);
            path_.clear();
        }

        std::filesystem::path path_;
    };
}
