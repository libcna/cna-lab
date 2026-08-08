// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/GridLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <vector>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/ILayout.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Layout engine that arranges widgets in proportioned grid cells. */
    class GridLayout final : public ILayout
    {
    public:
        [[nodiscard]] int getColumnSpacingProperty() const noexcept;
        void setColumnSpacingProperty(int value) noexcept;
        [[nodiscard]] int getRowSpacingProperty() const noexcept;
        void setRowSpacingProperty(int value) noexcept;

        [[nodiscard]] const Proportion& getDefaultColumnProportionProperty() const noexcept;
        void setDefaultColumnProportionProperty(Proportion value);
        [[nodiscard]] const Proportion& getDefaultRowProportionProperty() const noexcept;
        void setDefaultRowProportionProperty(Proportion value);
        [[nodiscard]] const std::vector<Proportion>& getColumnsProportionsProperty() const noexcept;
        [[nodiscard]] std::vector<Proportion>& getColumnsProportionsProperty() noexcept;
        [[nodiscard]] const std::vector<Proportion>& getRowsProportionsProperty() const noexcept;
        [[nodiscard]] std::vector<Proportion>& getRowsProportionsProperty() noexcept;

        [[nodiscard]] const std::vector<int>& getGridLinesXProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getGridLinesYProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getColWidthsProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getRowHeightsProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getCellLocationsXProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getCellLocationsYProperty() const noexcept;

        [[nodiscard]] int GetColumnWidth(int index) const noexcept;
        [[nodiscard]] int GetRowHeight(int index) const noexcept;
        [[nodiscard]] int GetCellLocationX(int column) const noexcept;
        [[nodiscard]] int GetCellLocationY(int row) const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle GetCellRectangle(int column, int row) const noexcept;
        [[nodiscard]] const Proportion& GetColumnProportion(int column) const noexcept;
        [[nodiscard]] const Proportion& GetRowProportion(int row) const noexcept;

        [[nodiscard]] Microsoft::Xna::Framework::Point Measure(
            const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Point availableSize) override;
        void Arrange(const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Rectangle bounds) override;

    private:
        [[nodiscard]] Microsoft::Xna::Framework::Point GetActualGridPosition(const Widget& child) const;
        void LayoutProcessFixedPart();
        void LayoutControl(Widget& control, Microsoft::Xna::Framework::Rectangle bounds);

        int columnSpacing_ = 0;
        int rowSpacing_ = 0;
        Proportion defaultColumnProportion_ = Proportion::GridDefault;
        Proportion defaultRowProportion_ = Proportion::GridDefault;
        std::vector<Proportion> columnsProportions_;
        std::vector<Proportion> rowsProportions_;
        std::vector<int> measureColumnWidths_;
        std::vector<int> measureRowHeights_;
        std::vector<std::shared_ptr<Widget>> visibleWidgets_;
        std::vector<std::vector<std::vector<std::shared_ptr<Widget>>>> widgetsByGridPosition_;
        std::vector<int> gridLinesX_;
        std::vector<int> gridLinesY_;
        std::vector<int> columnWidths_;
        std::vector<int> rowHeights_;
        std::vector<int> cellLocationsX_;
        std::vector<int> cellLocationsY_;
    };
}
