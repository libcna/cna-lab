// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/SplitPane.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract container that divides its logical widget collection with splitter handles. */
    class SplitPane : public Container
    {
      public:
        ~SplitPane() override;

        Events::MyraEventHandler ProportionsChanged;

        [[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getWidgetsProperty() const noexcept override;
        void AddWidget(std::shared_ptr<Widget> widget) override;
        void InsertWidget(std::size_t index, std::shared_ptr<Widget> widget);
        [[nodiscard]] bool RemoveWidget(const Widget *widget) override;
        void ClearWidgets();

        [[nodiscard]] virtual Orientation getOrientationProperty() const noexcept = 0;
        [[nodiscard]] float GetProportion(int widgetIndex) const noexcept;
        [[nodiscard]] float GetSplitterPosition(int leftWidgetIndex) const;
        void SetSplitterPosition(int leftWidgetIndex, float proportion);
        void Reset();
        void OnTouchMoved() override;

      protected:
        SplitPane();
        void OnPlacedChanged() override;
        void CopyFrom(const Widget &source) override;

      private:
        struct CallbackState final
        {
            SplitPane *Owner = nullptr;
        };

        struct HandleSubscription final
        {
            std::shared_ptr<Button> Handle;
            Events::MyraEventHandler::Token Token = Events::MyraEventHandler::InvalidToken;
        };

        [[nodiscard]] const ProportionCollection &GetActiveProportions() const noexcept;
        [[nodiscard]] ProportionCollection &GetActiveProportions() noexcept;
        void GetProportions(int leftWidgetIndex, std::shared_ptr<Proportion> &left, std::shared_ptr<Proportion> &right,
                            float &total) const;
        void HandleOnPressedChanged(void *sender);
        void BeginHandleDrag(const std::shared_ptr<Button> &handle);
        void EndHandleDrag();
        void UpdateHandleDrag();
        void SubscribeDesktopTouchMoved();
        void UnsubscribeDesktopTouchMoved() noexcept;
        void DesktopTouchMoved();
        void ClearHandleSubscriptions() noexcept;
        void FireProportionsChanged();

        GridLayout layout_;
        std::vector<std::shared_ptr<Widget>> widgets_;
        std::vector<std::shared_ptr<Button>> handles_;
        std::shared_ptr<CallbackState> callbackState_;
        std::vector<HandleSubscription> handleSubscriptions_;
        std::shared_ptr<Button> activeHandle_;
        std::optional<std::int64_t> mouseCoord_;
        Desktop *touchMovedSubscriptionDesktop_ = nullptr;
        Events::MyraEventHandler::Token touchMovedToken_ = Events::MyraEventHandler::InvalidToken;
    };

    /** @brief Split pane whose logical widgets are arranged left-to-right. */
    class HorizontalSplitPane final : public SplitPane
    {
      public:
        HorizontalSplitPane() = default;
        ~HorizontalSplitPane() override = default;

        [[nodiscard]] Orientation getOrientationProperty() const noexcept override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };

    /** @brief Split pane whose logical widgets are arranged top-to-bottom. */
    class VerticalSplitPane final : public SplitPane
    {
      public:
        VerticalSplitPane() = default;
        ~VerticalSplitPane() override = default;

        [[nodiscard]] Orientation getOrientationProperty() const noexcept override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
} // namespace Myra::Graphics2D::UI
