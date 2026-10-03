// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/ProgressBar.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D
{
    class IBrush;
}

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract progress indicator rendered along one fixed orientation. */
    class ProgressBar : public Widget
    {
    public:
        ~ProgressBar() override;

        Events::MyraEventHandler ValueChanged;

        [[nodiscard]] virtual Orientation getOrientationProperty() const noexcept = 0;
        [[nodiscard]] float getMinimumProperty() const noexcept;
        void setMinimumProperty(float value) noexcept;
        [[nodiscard]] float getMaximumProperty() const noexcept;
        void setMaximumProperty(float value) noexcept;
        [[nodiscard]] float getValueProperty() const noexcept;
        void setValueProperty(float value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getFillerProperty() const;
        void setFillerProperty(std::shared_ptr<Graphics2D::IBrush> value);

        void InternalRender(Graphics2D::RenderContext& context) override;

    protected:
        ProgressBar();
        void CopyFrom(const Widget& source) override;

    private:
        float minimum_ = 0.0F;
        float maximum_ = 100.0F;
        float value_ = 0.0F;
        std::shared_ptr<Graphics2D::IBrush> filler_;
    };
}
