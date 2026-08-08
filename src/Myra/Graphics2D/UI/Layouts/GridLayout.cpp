// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/GridLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        int CheckedIntegerResult(const long long value, const char* const message)
        {
            if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(value);
        }

        int CheckedAdd(const int left, const int right, const char* const message)
        {
            return CheckedIntegerResult(static_cast<long long>(left) + right, message);
        }

        int CheckedSubtract(const int left, const int right, const char* const message)
        {
            return CheckedIntegerResult(static_cast<long long>(left) - right, message);
        }

        int CheckedMultiply(const int left, const int right, const char* const message)
        {
            return CheckedIntegerResult(static_cast<long long>(left) * right, message);
        }
    }

    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    int GridLayout::getColumnSpacingProperty() const noexcept { return columnSpacing_; }
    void GridLayout::setColumnSpacingProperty(const int value) noexcept { columnSpacing_ = value; }
    int GridLayout::getRowSpacingProperty() const noexcept { return rowSpacing_; }
    void GridLayout::setRowSpacingProperty(const int value) noexcept { rowSpacing_ = value; }
    const std::shared_ptr<Proportion>& GridLayout::getDefaultColumnProportionProperty() const noexcept
    {
        return defaultColumnProportion_;
    }

    void GridLayout::setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value)
    {
        defaultColumnProportion_ = std::move(value);
    }

    const std::shared_ptr<Proportion>& GridLayout::getDefaultRowProportionProperty() const noexcept
    {
        return defaultRowProportion_;
    }

    void GridLayout::setDefaultRowProportionProperty(std::shared_ptr<Proportion> value)
    {
        defaultRowProportion_ = std::move(value);
    }

    const ProportionCollection& GridLayout::getColumnsProportionsProperty() const noexcept { return columnsProportions_; }
    ProportionCollection& GridLayout::getColumnsProportionsProperty() noexcept { return columnsProportions_; }
    const ProportionCollection& GridLayout::getRowsProportionsProperty() const noexcept { return rowsProportions_; }
    ProportionCollection& GridLayout::getRowsProportionsProperty() noexcept { return rowsProportions_; }
    const std::vector<int>& GridLayout::getGridLinesXProperty() const noexcept { return gridLinesX_; }
    const std::vector<int>& GridLayout::getGridLinesYProperty() const noexcept { return gridLinesY_; }
    const std::vector<int>& GridLayout::getColWidthsProperty() const noexcept { return columnWidths_; }
    const std::vector<int>& GridLayout::getRowHeightsProperty() const noexcept { return rowHeights_; }
    const std::vector<int>& GridLayout::getCellLocationsXProperty() const noexcept { return cellLocationsX_; }
    const std::vector<int>& GridLayout::getCellLocationsYProperty() const noexcept { return cellLocationsY_; }

    int GridLayout::GetColumnWidth(const int index) const noexcept
    {
        return index < 0 || static_cast<size_t>(index) >= columnWidths_.size() ? 0 : columnWidths_[index];
    }

    int GridLayout::GetRowHeight(const int index) const noexcept
    {
        return index < 0 || static_cast<size_t>(index) >= rowHeights_.size() ? 0 : rowHeights_[index];
    }

    int GridLayout::GetCellLocationX(const int column) const noexcept
    {
        return column < 0 || static_cast<size_t>(column) >= cellLocationsX_.size() ? 0 : cellLocationsX_[column];
    }

    int GridLayout::GetCellLocationY(const int row) const noexcept
    {
        return row < 0 || static_cast<size_t>(row) >= cellLocationsY_.size() ? 0 : cellLocationsY_[row];
    }

    Rectangle GridLayout::GetCellRectangle(const int column, const int row) const noexcept
    {
        if (column < 0 || row < 0 || static_cast<size_t>(column) >= cellLocationsX_.size() ||
            static_cast<size_t>(row) >= cellLocationsY_.size())
        {
            return Rectangle(0, 0, 0, 0);
        }
        return Rectangle(cellLocationsX_[column], cellLocationsY_[row], columnWidths_[column], rowHeights_[row]);
    }

    const std::shared_ptr<Proportion>& GridLayout::GetColumnProportion(const int column) const
    {
        const std::shared_ptr<Proportion>& result = column < 0 || column >= columnsProportions_.getCountProperty()
            ? defaultColumnProportion_ : columnsProportions_[column];
        if (!result)
        {
            throw std::logic_error("A grid column proportion cannot be null.");
        }
        return result;
    }

    const std::shared_ptr<Proportion>& GridLayout::GetRowProportion(const int row) const
    {
        const std::shared_ptr<Proportion>& result = row < 0 || row >= rowsProportions_.getCountProperty()
            ? defaultRowProportion_ : rowsProportions_[row];
        if (!result)
        {
            throw std::logic_error("A grid row proportion cannot be null.");
        }
        return result;
    }

    Point GridLayout::GetActualGridPosition(const Widget& child) const
    {
        return Point(Grid::GetColumn(child), Grid::GetRow(child));
    }

    void GridLayout::LayoutProcessFixedPart()
    {
        int size = 0;
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            if (GetColumnProportion(static_cast<int>(index))->getTypeProperty() == ProportionType::Part)
            {
                size = std::max(size, measureColumnWidths_[index]);
            }
        }
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(index));
            if (proportion->getTypeProperty() == ProportionType::Part)
            {
                measureColumnWidths_[index] = Utility::Mathematics::TruncateToInt(
                    static_cast<float>(size) * proportion->getValueProperty());
            }
        }

        size = 0;
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            if (GetRowProportion(static_cast<int>(index))->getTypeProperty() == ProportionType::Part)
            {
                size = std::max(size, measureRowHeights_[index]);
            }
        }
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(index));
            if (proportion->getTypeProperty() == ProportionType::Part)
            {
                measureRowHeights_[index] = Utility::Mathematics::TruncateToInt(
                    static_cast<float>(size) * proportion->getValueProperty());
            }
        }
    }

    Point GridLayout::Measure(const std::vector<std::shared_ptr<Widget>>& widgets, Point availableSize)
    {
        int rows = 0;
        int columns = 0;
        visibleWidgets_.clear();
        for (const std::shared_ptr<Widget>& child : widgets)
        {
            if (!child || !child->getVisibleProperty())
            {
                continue;
            }
            visibleWidgets_.push_back(child);
            const Point position = GetActualGridPosition(*child);
            if (position.X < 0 || position.Y < 0)
            {
                throw std::invalid_argument("Grid row and column coordinates cannot be negative.");
            }
            const int columnSpan = Grid::GetColumnSpan(*child);
            const int rowSpan = Grid::GetRowSpan(*child);
            if (columnSpan <= 0 || rowSpan <= 0)
            {
                throw std::invalid_argument("Grid row and column spans must be positive.");
            }
            if (position.X > std::numeric_limits<int>::max() - columnSpan ||
                position.Y > std::numeric_limits<int>::max() - rowSpan)
            {
                throw std::overflow_error("A grid coordinate and span exceed the supported integer range.");
            }
            columns = std::max(columns, position.X + columnSpan);
            rows = std::max(rows, position.Y + rowSpan);
        }
        columns = std::max(columns, static_cast<int>(columnsProportions_.getCountProperty()));
        rows = std::max(rows, static_cast<int>(rowsProportions_.getCountProperty()));

        measureColumnWidths_.assign(static_cast<size_t>(columns), 0);
        measureRowHeights_.assign(static_cast<size_t>(rows), 0);
        widgetsByGridPosition_.assign(static_cast<size_t>(rows),
            std::vector<std::vector<std::shared_ptr<Widget>>>(static_cast<size_t>(columns)));
        for (const std::shared_ptr<Widget>& widget : visibleWidgets_)
        {
            const Point position = GetActualGridPosition(*widget);
            widgetsByGridPosition_[position.Y][position.X].push_back(widget);
        }

        const int columnSpacingTotal = CheckedMultiply(
            columns - 1, columnSpacing_, "Grid column spacing exceeds the supported integer range.");
        const int rowSpacingTotal = CheckedMultiply(
            rows - 1, rowSpacing_, "Grid row spacing exceeds the supported integer range.");
        availableSize.X = CheckedSubtract(availableSize.X, columnSpacingTotal,
            "Grid available width exceeds the supported integer range.");
        availableSize.Y = CheckedSubtract(availableSize.Y, rowSpacingTotal,
            "Grid available height exceeds the supported integer range.");
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < columns; ++column)
            {
                const std::shared_ptr<Proportion>& rowProportion = GetRowProportion(row);
                const std::shared_ptr<Proportion>& columnProportion = GetColumnProportion(column);
                if (columnProportion->getTypeProperty() == ProportionType::Pixels)
                {
                    measureColumnWidths_[column] = Utility::Mathematics::TruncateToInt(
                        columnProportion->getValueProperty());
                }
                if (rowProportion->getTypeProperty() == ProportionType::Pixels)
                {
                    measureRowHeights_[row] = Utility::Mathematics::TruncateToInt(
                        rowProportion->getValueProperty());
                }
                for (const std::shared_ptr<Widget>& widget : widgetsByGridPosition_[row][column])
                {
                    Point measuredSize(0, 0);
                    if (rowProportion->getTypeProperty() != ProportionType::Pixels ||
                        columnProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measuredSize = widget->Measure(availableSize);
                    }
                    if (Grid::GetColumnSpan(*widget) != 1)
                    {
                        measuredSize.X = 0;
                    }
                    if (Grid::GetRowSpan(*widget) != 1)
                    {
                        measuredSize.Y = 0;
                    }
                    if (columnProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measureColumnWidths_[column] = std::max(measureColumnWidths_[column], measuredSize.X);
                    }
                    if (rowProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measureRowHeights_[row] = std::max(measureRowHeights_[row], measuredSize.Y);
                    }
                }
            }
        }
        LayoutProcessFixedPart();

        Point result(0, 0);
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            result.X = CheckedAdd(result.X, measureColumnWidths_[index],
                "Measured grid width exceeds the supported integer range.");
            if (index + 1 < measureColumnWidths_.size())
            {
                result.X = CheckedAdd(result.X, columnSpacing_,
                    "Measured grid width exceeds the supported integer range.");
            }
        }
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            result.Y = CheckedAdd(result.Y, measureRowHeights_[index],
                "Measured grid height exceeds the supported integer range.");
            if (index + 1 < measureRowHeights_.size())
            {
                result.Y = CheckedAdd(result.Y, rowSpacing_,
                    "Measured grid height exceeds the supported integer range.");
            }
        }
        return result;
    }

    void GridLayout::Arrange(const std::vector<std::shared_ptr<Widget>>& widgets, const Rectangle bounds)
    {
        static_cast<void>(Measure(widgets, Point(bounds.Width, bounds.Height)));
        columnWidths_ = measureColumnWidths_;
        rowHeights_ = measureRowHeights_;

        const int arrangedColumnSpacing = CheckedMultiply(
            static_cast<int>(columnWidths_.size()) - 1, columnSpacing_,
            "Arranged grid column spacing exceeds the supported integer range.");
        float availableWidth = static_cast<float>(CheckedSubtract(
            bounds.Width, arrangedColumnSpacing, "Arranged grid width exceeds the supported integer range."));
        float totalPart = 0.0F;
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(column));
            if (proportion->getTypeProperty() == ProportionType::Auto || proportion->getTypeProperty() == ProportionType::Pixels)
            {
                availableWidth -= static_cast<float>(columnWidths_[column]);
            }
            else
            {
                totalPart += proportion->getValueProperty();
            }
        }
        if (!Utility::Mathematics::IsZero(totalPart))
        {
            float takenSpace = 0.0F;
            for (size_t column = 0; column < columnWidths_.size(); ++column)
            {
                const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(column));
                if (proportion->getTypeProperty() == ProportionType::Part)
                {
                    columnWidths_[column] = Utility::Mathematics::TruncateToInt(
                        proportion->getValueProperty() * availableWidth / totalPart);
                    takenSpace += static_cast<float>(columnWidths_[column]);
                }
            }
            availableWidth -= takenSpace;
        }
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            if (GetColumnProportion(static_cast<int>(column))->getTypeProperty() == ProportionType::Fill)
            {
                columnWidths_[column] = Utility::Mathematics::TruncateToInt(availableWidth);
                break;
            }
        }

        const int arrangedRowSpacing = CheckedMultiply(
            static_cast<int>(rowHeights_.size()) - 1, rowSpacing_,
            "Arranged grid row spacing exceeds the supported integer range.");
        float availableHeight = static_cast<float>(CheckedSubtract(
            bounds.Height, arrangedRowSpacing, "Arranged grid height exceeds the supported integer range."));
        totalPart = 0.0F;
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(row));
            if (proportion->getTypeProperty() == ProportionType::Auto || proportion->getTypeProperty() == ProportionType::Pixels)
            {
                availableHeight -= static_cast<float>(rowHeights_[row]);
            }
            else
            {
                totalPart += proportion->getValueProperty();
            }
        }
        if (!Utility::Mathematics::IsZero(totalPart))
        {
            float takenSpace = 0.0F;
            for (size_t row = 0; row < rowHeights_.size(); ++row)
            {
                const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(row));
                if (proportion->getTypeProperty() == ProportionType::Part)
                {
                    rowHeights_[row] = Utility::Mathematics::TruncateToInt(
                        proportion->getValueProperty() * availableHeight / totalPart);
                    takenSpace += static_cast<float>(rowHeights_[row]);
                }
            }
            availableHeight -= takenSpace;
        }
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            if (GetRowProportion(static_cast<int>(row))->getTypeProperty() == ProportionType::Fill)
            {
                rowHeights_[row] = Utility::Mathematics::TruncateToInt(availableHeight);
                break;
            }
        }

        gridLinesX_.clear();
        cellLocationsX_.clear();
        int position = 0;
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            cellLocationsX_.push_back(position);
            position = CheckedAdd(position, columnWidths_[column],
                "A grid column location exceeds the supported integer range.");
            if (column + 1 < columnWidths_.size())
            {
                gridLinesX_.push_back(CheckedAdd(position, columnSpacing_ / 2,
                    "A vertical grid line exceeds the supported integer range."));
            }
            position = CheckedAdd(position, columnSpacing_,
                "A grid column location exceeds the supported integer range.");
        }
        gridLinesY_.clear();
        cellLocationsY_.clear();
        position = 0;
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            cellLocationsY_.push_back(position);
            position = CheckedAdd(position, rowHeights_[row],
                "A grid row location exceeds the supported integer range.");
            if (row + 1 < rowHeights_.size())
            {
                gridLinesY_.push_back(CheckedAdd(position, rowSpacing_ / 2,
                    "A horizontal grid line exceeds the supported integer range."));
            }
            position = CheckedAdd(position, rowSpacing_,
                "A grid row location exceeds the supported integer range.");
        }
        for (const std::shared_ptr<Widget>& control : visibleWidgets_)
        {
            LayoutControl(*control, bounds);
        }
    }

    void GridLayout::LayoutControl(Widget& control, const Rectangle bounds)
    {
        const Point position = GetActualGridPosition(control);
        const int columnSpan = Grid::GetColumnSpan(control);
        const int rowSpan = Grid::GetRowSpan(control);
        if (position.X < 0 || position.Y < 0 || columnSpan <= 0 || rowSpan <= 0 ||
            static_cast<size_t>(position.X) >= columnWidths_.size() ||
            static_cast<size_t>(position.Y) >= rowHeights_.size() ||
            (columnSpan > 0 && static_cast<long long>(position.X) + columnSpan >
                static_cast<long long>(columnWidths_.size())) ||
            (rowSpan > 0 && static_cast<long long>(position.Y) + rowSpan >
                static_cast<long long>(rowHeights_.size())))
        {
            throw std::invalid_argument(
                "A grid child's coordinates or spans changed outside the measured grid bounds.");
        }

        const int columnEnd = CheckedAdd(position.X, columnSpan,
            "A grid column and span exceed the supported integer range.");
        const int rowEnd = CheckedAdd(position.Y, rowSpan,
            "A grid row and span exceed the supported integer range.");

        Point cellSize(0, 0);
        for (int column = position.X; column < columnEnd; ++column)
        {
            cellSize.X = CheckedAdd(cellSize.X, columnWidths_[column],
                "A spanned grid cell width exceeds the supported integer range.");
            if (column + 1 < columnEnd)
            {
                cellSize.X = CheckedAdd(cellSize.X, columnSpacing_,
                    "A spanned grid cell width exceeds the supported integer range.");
            }
        }
        for (int row = position.Y; row < rowEnd; ++row)
        {
            cellSize.Y = CheckedAdd(cellSize.Y, rowHeights_[row],
                "A spanned grid cell height exceeds the supported integer range.");
            if (row + 1 < rowEnd)
            {
                cellSize.Y = CheckedAdd(cellSize.Y, rowSpacing_,
                    "A spanned grid cell height exceeds the supported integer range.");
            }
        }
        Rectangle rectangle(CheckedAdd(bounds.X, cellLocationsX_[position.X],
                                "A grid child X coordinate exceeds the supported integer range."),
            CheckedAdd(bounds.Y, cellLocationsY_[position.Y],
                "A grid child Y coordinate exceeds the supported integer range."),
            cellSize.X, cellSize.Y);
        const long long boundsRight = static_cast<long long>(bounds.X) + bounds.Width;
        const long long boundsBottom = static_cast<long long>(bounds.Y) + bounds.Height;
        if (static_cast<long long>(rectangle.X) + rectangle.Width > boundsRight)
        {
            rectangle.Width = CheckedIntegerResult(boundsRight - rectangle.X,
                "A clipped grid child width exceeds the supported integer range.");
        }
        if (rectangle.Width < 0)
        {
            rectangle.Width = 0;
        }
        if (static_cast<long long>(rectangle.Y) + rectangle.Height > boundsBottom)
        {
            rectangle.Height = CheckedIntegerResult(boundsBottom - rectangle.Y,
                "A clipped grid child height exceeds the supported integer range.");
        }
        if (rectangle.Height < 0)
        {
            rectangle.Height = 0;
        }
        control.Arrange(rectangle);
    }
}
