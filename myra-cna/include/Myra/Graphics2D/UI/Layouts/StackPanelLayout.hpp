// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/StackPanelLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <vector>

#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Grid-backed layout engine that stacks widgets on one axis. */
    class StackPanelLayout final : public ILayout
    {
    public:
        explicit StackPanelLayout(Orientation orientation);

        [[nodiscard]] Orientation getOrientationProperty() const noexcept;
        [[nodiscard]] int getSpacingProperty() const noexcept;
        void setSpacingProperty(int value) noexcept;
        [[nodiscard]] const std::shared_ptr<Proportion>& getDefaultProportionProperty() const noexcept;
        void setDefaultProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const ProportionCollection& getProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection& getProportionsProperty() noexcept;
        [[nodiscard]] const std::vector<int>& getGridLinesXProperty() const noexcept;
        [[nodiscard]] const std::vector<int>& getGridLinesYProperty() const noexcept;

        [[nodiscard]] Microsoft::Xna::Framework::Point Measure(
            const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Point availableSize) override;
        void Arrange(const std::vector<std::shared_ptr<Widget>>& widgets,
            Microsoft::Xna::Framework::Rectangle bounds) override;
        [[nodiscard]] int GetCellSize(int index) const noexcept;

    private:
        void UpdateWidgets(const std::vector<std::shared_ptr<Widget>>& widgets);

        GridLayout layout_;
        Orientation orientation_;
    };
}
