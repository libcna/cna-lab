// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/Slider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Events/ValueChangedEventArgs.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract range selector with a retained button knob. */
    class Slider : public Widget
    {
      public:
        ~Slider() override = default;

        Events::MyraEventHandlerT<Events::ValueChangedEventArgs<float>> ValueChanged;
        Events::MyraEventHandlerT<Events::ValueChangedEventArgs<float>> ValueChangedByUser;

        [[nodiscard]] virtual Orientation getOrientationProperty() const noexcept = 0;
        [[nodiscard]] float getMinimumProperty() const noexcept;
        void setMinimumProperty(float value) noexcept;
        [[nodiscard]] float getMaximumProperty() const noexcept;
        void setMaximumProperty(float value) noexcept;
        [[nodiscard]] float getValueProperty() const noexcept;
        void setValueProperty(float value);
        [[nodiscard]] bool getWheelAdjustmentProperty() const noexcept;
        void setWheelAdjustmentProperty(bool value) noexcept;
        [[nodiscard]] float getWheelStepProperty() const noexcept;
        void setWheelStepProperty(float value) noexcept;
        [[nodiscard]] std::shared_ptr<Button> getImageButtonProperty() const;

      protected:
        Slider();
        void InternalArrange() override;
        void CopyFrom(const Widget &source) override;

      private:
        [[nodiscard]] int getHint() const;
        void setHint(int value);
        [[nodiscard]] int getMaxHint() const;
        void SyncHintWithValue();

        SingleItemLayout<Button> layout_;
        float minimum_ = 0.0F;
        float maximum_ = 100.0F;
        float value_ = 0.0F;
        float wheelStep_ = 1.0F;
        bool wheelAdjustment_ = false;
    };
} // namespace Myra::Graphics2D::UI
