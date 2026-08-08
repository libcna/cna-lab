// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Grid.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

#include <algorithm>
#include <utility>

#include "Myra/Attributes/RangeAttribute.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        MML::PropertyMetadata RangeMetadata(const float minimum)
        {
            MML::PropertyMetadata metadata;
            metadata.Range = Attributes::RangeAttribute(minimum);
            return metadata;
        }
    }

    Grid::Grid()
    {
        setChildrenLayoutProperty(&layout_);
        layout_.getColumnsProportionsProperty().CollectionChanged.emplace_back(
            [this](void*, const auto&) { OnProportionsCollectionChanged(); });
        layout_.getRowsProportionsProperty().CollectionChanged.emplace_back(
            [this](void*, const auto&) { OnProportionsCollectionChanged(); });
    }

    Grid::~Grid()
    {
        ClearProportionSubscriptions();
    }

    int Grid::getColumnSpacingProperty() const noexcept { return layout_.getColumnSpacingProperty(); }

    void Grid::setColumnSpacingProperty(const int value)
    {
        if (value == layout_.getColumnSpacingProperty())
        {
            return;
        }
        layout_.setColumnSpacingProperty(value);
        InvalidateMeasure();
    }

    int Grid::getRowSpacingProperty() const noexcept { return layout_.getRowSpacingProperty(); }

    void Grid::setRowSpacingProperty(const int value)
    {
        if (value == layout_.getRowSpacingProperty())
        {
            return;
        }
        layout_.setRowSpacingProperty(value);
        InvalidateMeasure();
    }

    const std::shared_ptr<Proportion>& Grid::getDefaultColumnProportionProperty() const noexcept
    {
        return layout_.getDefaultColumnProportionProperty();
    }

    void Grid::setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value)
    {
        layout_.setDefaultColumnProportionProperty(std::move(value));
        InvalidateMeasure();
    }

    const std::shared_ptr<Proportion>& Grid::getDefaultRowProportionProperty() const noexcept
    {
        return layout_.getDefaultRowProportionProperty();
    }

    void Grid::setDefaultRowProportionProperty(std::shared_ptr<Proportion> value)
    {
        layout_.setDefaultRowProportionProperty(std::move(value));
        InvalidateMeasure();
    }

    const ProportionCollection& Grid::getColumnsProportionsProperty() const noexcept
    {
        return layout_.getColumnsProportionsProperty();
    }

    ProportionCollection& Grid::getColumnsProportionsProperty() noexcept
    {
        return layout_.getColumnsProportionsProperty();
    }

    const ProportionCollection& Grid::getRowsProportionsProperty() const noexcept
    {
        return layout_.getRowsProportionsProperty();
    }

    ProportionCollection& Grid::getRowsProportionsProperty() noexcept
    {
        return layout_.getRowsProportionsProperty();
    }

    void Grid::OnProportionsCollectionChanged()
    {
        RebuildProportionSubscriptions();
        InvalidateMeasure();
    }

    void Grid::RebuildProportionSubscriptions()
    {
        ClearProportionSubscriptions();

        const auto subscribe = [this](const std::shared_ptr<Proportion>& proportion) {
            if (!proportion)
            {
                return;
            }
            const Events::MyraEventHandler::Token token = proportion->Changed.Add(
                [this](void*, Events::MyraEventArgs&) { InvalidateMeasure(); });
            proportionSubscriptions_.push_back({proportion, token});
        };
        for (const std::shared_ptr<Proportion>& proportion : layout_.getColumnsProportionsProperty())
        {
            subscribe(proportion);
        }
        for (const std::shared_ptr<Proportion>& proportion : layout_.getRowsProportionsProperty())
        {
            subscribe(proportion);
        }
    }

    void Grid::ClearProportionSubscriptions()
    {
        for (ProportionSubscription& subscription : proportionSubscriptions_)
        {
            if (subscription.proportion)
            {
                static_cast<void>(subscription.proportion->Changed.Remove(subscription.token));
            }
        }
        proportionSubscriptions_.clear();
    }

    int Grid::GetColumnWidth(const int index) const noexcept { return layout_.GetColumnWidth(index); }
    int Grid::GetRowHeight(const int index) const noexcept { return layout_.GetRowHeight(index); }
    int Grid::GetCellLocationX(const int column) const noexcept { return layout_.GetCellLocationX(column); }
    int Grid::GetCellLocationY(const int row) const noexcept { return layout_.GetCellLocationY(row); }

    Microsoft::Xna::Framework::Rectangle Grid::GetCellRectangle(const int column, const int row) const noexcept
    {
        return layout_.GetCellRectangle(column, row);
    }

    const MML::AttachedPropertyInfo<int>& Grid::getColumnProperty()
    {
        static const MML::AttachedPropertyInfo<int>* property = [] {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "Column", 0, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(0.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int>& Grid::getRowProperty()
    {
        static const MML::AttachedPropertyInfo<int>* property = [] {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "Row", 0, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(0.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int>& Grid::getColumnSpanProperty()
    {
        static const MML::AttachedPropertyInfo<int>* property = [] {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "ColumnSpan", 1, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(1.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int>& Grid::getRowSpanProperty()
    {
        static const MML::AttachedPropertyInfo<int>* property = [] {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "RowSpan", 1, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(1.0F));
        }();
        return *property;
    }

    int Grid::GetColumn(const Widget& widget) { return getColumnProperty().GetValue(widget); }
    void Grid::SetColumn(Widget& widget, const int value) { getColumnProperty().SetValue(widget, value); }
    int Grid::GetRow(const Widget& widget) { return getRowProperty().GetValue(widget); }
    void Grid::SetRow(Widget& widget, const int value) { getRowProperty().SetValue(widget, value); }
    int Grid::GetColumnSpan(const Widget& widget) { return getColumnSpanProperty().GetValue(widget); }
    void Grid::SetColumnSpan(Widget& widget, const int value) { getColumnSpanProperty().SetValue(widget, value); }
    int Grid::GetRowSpan(const Widget& widget) { return getRowSpanProperty().GetValue(widget); }
    void Grid::SetRowSpan(Widget& widget, const int value) { getRowSpanProperty().SetValue(widget, value); }
}
