// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ISelector.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <concepts>
#include <memory>
#include <optional>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Selectors/ISelectorItem.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Specifies whether a selector maintains one or multiple selected items. */
    enum class SelectionMode
    {
        Single,
        Multiple
    };

    /** @brief Non-generic selector selection contract. */
    class ISelector
    {
      public:
        virtual ~ISelector() = default;

        [[nodiscard]] virtual SelectionMode getSelectionModeProperty() const noexcept = 0;
        virtual void setSelectionModeProperty(SelectionMode value) noexcept = 0;
        [[nodiscard]] virtual std::optional<int> getSelectedIndexProperty() const = 0;
        virtual void setSelectedIndexProperty(std::optional<int> value) = 0;
        virtual Events::MyraEventHandler &getSelectedIndexChangedEvent() noexcept = 0;
    };

    /** @brief Typed selector contract retaining selectable item references. */
    template <std::derived_from<ISelectorItem> ItemT> class ISelectorT : public ISelector
    {
      public:
        using ItemCollection = System::Collections::ObjectModel::ObservableCollection<std::shared_ptr<ItemT>>;

        ~ISelectorT() override = default;

        [[nodiscard]] virtual const ItemCollection &getItemsProperty() const noexcept = 0;
        [[nodiscard]] virtual ItemCollection &getItemsProperty() noexcept = 0;
        [[nodiscard]] virtual std::shared_ptr<ItemT> getSelectedItemProperty() const = 0;
        virtual void setSelectedItemProperty(std::shared_ptr<ItemT> value) = 0;
    };
} // namespace Myra::Graphics2D::UI
