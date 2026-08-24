// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ListView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Selectors/ISelector.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Scrollable widget collection with single-selection ListViewButton wrappers. */
    class ListView final : public Widget, public IContainer
    {
      public:
        ListView();
        ~ListView() override;

        Events::MyraEventHandler SelectedIndexChanged;

        [[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getWidgetsProperty() const noexcept override;
        void AddWidget(std::shared_ptr<Widget> widget) override;
        void InsertWidget(std::size_t index, std::shared_ptr<Widget> widget);
        [[nodiscard]] bool RemoveWidget(const Widget *widget) override;
        void ClearWidgets();

        [[nodiscard]] std::shared_ptr<ScrollViewer> getScrollViewerProperty() const;
        [[nodiscard]] SelectionMode getSelectionModeProperty() const noexcept;
        void setSelectionModeProperty(SelectionMode value) noexcept;
        [[nodiscard]] std::optional<int> getSelectedIndexProperty() const;
        void setSelectedIndexProperty(std::optional<int> value);
        [[nodiscard]] std::shared_ptr<Widget> getSelectedItemProperty() const;
        void setSelectedItemProperty(std::shared_ptr<Widget> value);

        /** @brief Forwards a wheel delta to the owned scroll viewer. */
        void OnMouseWheel(float delta) override;
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        struct CallbackState
        {
            ListView *owner = nullptr;
        };

        struct ButtonSubscription
        {
            std::shared_ptr<ListViewButton> button;
            Events::MyraEventHandler::Token token = Events::MyraEventHandler::InvalidToken;
        };

        [[nodiscard]] std::shared_ptr<Widget> Wrap(std::shared_ptr<Widget> widget);
        void RebuildDisplay();
        void ClearButtonSubscriptions() noexcept;
        void ButtonOnClick(void *sender, Events::MyraEventArgs &arguments);
        void HideComboDropdown();
        void UpdateScrolling();

        std::shared_ptr<CallbackState> callbackState_;
        SingleItemLayout<ScrollViewer> layout_;
        std::shared_ptr<ScrollViewer> scrollViewer_;
        std::shared_ptr<VerticalStackPanel> box_;
        std::vector<ButtonSubscription> buttonSubscriptions_;
        std::vector<std::shared_ptr<Widget>> widgets_;
        std::shared_ptr<Widget> selectedItem_;
        SelectionMode selectionMode_ = SelectionMode::Single;
    };
} // namespace Myra::Graphics2D::UI
