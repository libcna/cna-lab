// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "System/Random.hpp"

namespace CnaKiller
{
    /**
     * @brief Owns a churn pool of GPU/audio/etc. resources for the chaos engine.
     *
     * Every "create X" chaos action pushes a freshly constructed @p T into a pool; every
     * "destroy X" action removes one -- picked either at random (to simulate an application
     * that frees resources in an arbitrary, non-LIFO order) or from the front (to simulate
     * steady turnover of the oldest resources). Removing an entry runs @p T's destructor
     * immediately via unique_ptr, so this is real resource destruction, not just bookkeeping.
     *
     * All "which index" decisions are made by the caller via the supplied System::Random, so
     * the pool itself introduces no extra nondeterminism.
     */
    template <typename T>
    class ResourcePool
    {
    public:
        void Add(std::unique_ptr<T> item) { items_.push_back(std::move(item)); }

        [[nodiscard]] bool Empty() const { return items_.empty(); }
        [[nodiscard]] std::size_t Size() const { return items_.size(); }

        [[nodiscard]] T& At(std::size_t index) { return *items_[index]; }
        [[nodiscard]] const T& At(std::size_t index) const { return *items_[index]; }

        /** @brief Returns a reference to a pool entry chosen uniformly at random. Pool must be non-empty. */
        [[nodiscard]] T& RandomItem(System::Random& rng)
        {
            const auto index = static_cast<std::size_t>(rng.Next(0, static_cast<int>(items_.size())));
            return *items_[index];
        }

        /** @brief Destroys the entry at a uniformly random index. No-op if the pool is empty. */
        void DestroyRandom(System::Random& rng)
        {
            if (items_.empty())
                return;
            const auto index = static_cast<std::size_t>(rng.Next(0, static_cast<int>(items_.size())));
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
        }

        /** @brief Destroys the entry at @p index. */
        void DestroyAt(std::size_t index)
        {
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
        }

        /** @brief Removes the entry at @p index from the pool and hands it to the caller. */
        [[nodiscard]] std::unique_ptr<T> Take(std::size_t index)
        {
            std::unique_ptr<T> item = std::move(items_[index]);
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
            return item;
        }

        /** @brief Destroys the oldest surviving entry (front of the pool). No-op if empty. */
        void DestroyOldest()
        {
            if (!items_.empty())
                items_.erase(items_.begin());
        }

        /** @brief Destroys every entry immediately. */
        void Clear() { items_.clear(); }

        [[nodiscard]] auto begin() { return items_.begin(); }
        [[nodiscard]] auto end() { return items_.end(); }
        [[nodiscard]] auto begin() const { return items_.begin(); }
        [[nodiscard]] auto end() const { return items_.end(); }

    private:
        std::vector<std::unique_ptr<T>> items_;
    };
}
