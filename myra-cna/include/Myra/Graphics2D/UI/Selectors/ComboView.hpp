// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ComboView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListView.hpp"
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Combo selector core that owns a toggle facade and a detached ListView dropdown. */
    class ComboView final : public Widget, public IContainer
    {
      public:
        ComboView();
        ~ComboView() override;

        /** @brief Raised when the retained ListView selection changes. */
        Events::MyraEventHandler SelectedIndexChanged;

        [[nodiscard]] std::optional<int> getDropdownMaximumHeightProperty() const noexcept;
        void setDropdownMaximumHeightProperty(std::optional<int> value);
        [[nodiscard]] bool getIsExpandedProperty() const noexcept;
        [[nodiscard]] std::shared_ptr<ToggleButton> getButtonProperty() const;
        [[nodiscard]] std::shared_ptr<ListView> getListViewProperty() const;

        [[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getWidgetsProperty() const noexcept override;
        void AddWidget(std::shared_ptr<Widget> widget) override;
        [[nodiscard]] bool RemoveWidget(const Widget *widget) override;
        [[nodiscard]] std::shared_ptr<Widget> getSelectedItemProperty() const;
        void setSelectedItemProperty(std::shared_ptr<Widget> value);
        [[nodiscard]] SelectionMode getSelectionModeProperty() const noexcept;
        void setSelectionModeProperty(SelectionMode value) noexcept;
        [[nodiscard]] std::optional<int> getSelectedIndexProperty() const;
        void setSelectedIndexProperty(std::optional<int> value);
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

      protected:
        [[nodiscard]] Microsoft::Xna::Framework::Point
        InternalMeasure(Microsoft::Xna::Framework::Point availableSize) override;
        void InternalArrange() override;
        void OnPlacedChanged() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        struct CallbackState
        {
            ComboView *owner = nullptr;
        };

        void OnButtonPressedChanged(void *sender, Events::MyraEventArgs &arguments);
        void DesktopOnContextMenuClosed();
        void SubscribeDesktopContextMenuClosed();
        void UnsubscribeDesktopContextMenuClosed() noexcept;
        void UpdateSelectedItem();

        std::shared_ptr<CallbackState> callbackState_;
        SingleItemLayout<ToggleButton> layout_;
        std::shared_ptr<ToggleButton> button_;
        std::shared_ptr<ListView> listView_;
        Events::MyraEventHandler::Token buttonPressedToken_ = Events::MyraEventHandler::InvalidToken;
        Events::MyraEventHandler::Token listSelectionToken_ = Events::MyraEventHandler::InvalidToken;
        Desktop *contextMenuSubscriptionDesktop_ = nullptr;
        Events::MyraEventHandlerT<Events::GenericEventArgs<Widget *>>::Token contextMenuClosedToken_ =
            Events::MyraEventHandlerT<Events::GenericEventArgs<Widget *>>::InvalidToken;
    };
} // namespace Myra::Graphics2D::UI
