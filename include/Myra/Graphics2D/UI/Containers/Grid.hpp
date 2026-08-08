// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Grid.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Retained-mode container that places children into rows and columns. */
    class Grid : public Container
    {
    public:
        Grid();
        ~Grid() override;

        [[nodiscard]] int getColumnSpacingProperty() const noexcept;
        void setColumnSpacingProperty(int value);
        [[nodiscard]] int getRowSpacingProperty() const noexcept;
        void setRowSpacingProperty(int value);
        [[nodiscard]] const std::shared_ptr<Proportion>& getDefaultColumnProportionProperty() const noexcept;
        void setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const std::shared_ptr<Proportion>& getDefaultRowProportionProperty() const noexcept;
        void setDefaultRowProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const ProportionCollection& getColumnsProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection& getColumnsProportionsProperty() noexcept;
        [[nodiscard]] const ProportionCollection& getRowsProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection& getRowsProportionsProperty() noexcept;

        [[nodiscard]] int GetColumnWidth(int index) const noexcept;
        [[nodiscard]] int GetRowHeight(int index) const noexcept;
        [[nodiscard]] int GetCellLocationX(int column) const noexcept;
        [[nodiscard]] int GetCellLocationY(int row) const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle GetCellRectangle(int column, int row) const noexcept;

        [[nodiscard]] static const MML::AttachedPropertyInfo<int>& getColumnProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int>& getRowProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int>& getColumnSpanProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int>& getRowSpanProperty();
        [[nodiscard]] static int GetColumn(const Widget& widget);
        static void SetColumn(Widget& widget, int value);
        [[nodiscard]] static int GetRow(const Widget& widget);
        static void SetRow(Widget& widget, int value);
        [[nodiscard]] static int GetColumnSpan(const Widget& widget);
        static void SetColumnSpan(Widget& widget, int value);
        [[nodiscard]] static int GetRowSpan(const Widget& widget);
        static void SetRowSpan(Widget& widget, int value);

    private:
        struct ProportionSubscription
        {
            std::shared_ptr<Proportion> proportion;
            Events::MyraEventHandler::Token token = Events::MyraEventHandler::InvalidToken;
        };

        void OnProportionsCollectionChanged();
        void RebuildProportionSubscriptions();
        void ClearProportionSubscriptions();

        GridLayout layout_;
        std::vector<ProportionSubscription> proportionSubscriptions_;
    };
}
