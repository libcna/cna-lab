// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/StackPanel.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Layouts/StackPanelLayout.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Base container that arranges children along one fixed axis. */
    class StackPanel : public Container
    {
    public:
        ~StackPanel() override = default;

        [[nodiscard]] Orientation getOrientationProperty() const noexcept;
        [[nodiscard]] int getSpacingProperty() const noexcept;
        void setSpacingProperty(int value);
        [[nodiscard]] const std::shared_ptr<Proportion>& getDefaultProportionProperty() const noexcept;
        void setDefaultProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const ProportionCollection& getProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection& getProportionsProperty() noexcept;
        [[nodiscard]] int GetCellSize(int index) const noexcept;

        [[nodiscard]] static const MML::AttachedPropertyInfo<ProportionType>& getProportionTypeProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<float>& getProportionValueProperty();
        [[nodiscard]] static ProportionType GetProportionType(const Widget& widget);
        static void SetProportionType(Widget& widget, ProportionType value);
        [[nodiscard]] static float GetProportionValue(const Widget& widget);
        static void SetProportionValue(Widget& widget, float value);

    protected:
        explicit StackPanel(Orientation orientation);
        void CopyFrom(const Widget& source) override;
        [[nodiscard]] Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize) override;
        void InternalArrange() override;

    private:
        void UpdateChildren();
        void InvalidateProportions() noexcept;

        StackPanelLayout layout_;
        ProportionCollection proportions_;
        bool childrenDirty_ = true;
    };

    /** @brief A stack panel with horizontal orientation. */
    class HorizontalStackPanel final : public StackPanel
    {
    public:
        HorizontalStackPanel();

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };

    /** @brief A stack panel with vertical orientation. */
    class VerticalStackPanel final : public StackPanel
    {
    public:
        VerticalStackPanel();

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
}
