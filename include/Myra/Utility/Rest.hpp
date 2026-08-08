// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/Rest.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Myra::Utility
{
    /** @brief Small rectangular/jagged-table helpers used by upstream data widgets. */
    class Rest final
    {
    public:
        Rest() = delete;

        /**
         * @brief Copies the outer table and each row while retaining element copy semantics.
         *
         * For pointer-like elements this deliberately remains a shallow element copy, just
         * like the selected upstream's copy of its object references.
         */
        template<typename T>
        [[nodiscard]] static std::vector<std::vector<T>> CloneArray(
            const std::vector<std::vector<T>>& source)
        {
            return source;
        }

        /**
         * @brief Sorts rows by one column, using an optional strict weak ordering.
         * @throws std::out_of_range if the column is negative or absent from any row.
         */
        template<typename T, typename Compare = std::less<T>>
        static void SortByColumn(
            std::vector<std::vector<T>>& source,
            std::ptrdiff_t columnIndex,
            bool ascending,
            Compare compare = {})
        {
            if (columnIndex < 0)
            {
                throw std::out_of_range("Rest::SortByColumn column index cannot be negative");
            }

            const auto column = static_cast<std::size_t>(columnIndex);
            for (const auto& row : source)
            {
                if (column >= row.size())
                {
                    throw std::out_of_range(
                        "Rest::SortByColumn column index is outside at least one row");
                }
            }

            std::sort(source.begin(), source.end(),
                [column, ascending, compare = std::move(compare)](
                    const auto& left, const auto& right) mutable
                {
                    return ascending
                        ? std::invoke(compare, left[column], right[column])
                        : std::invoke(compare, right[column], left[column]);
                });
        }
    };
}
