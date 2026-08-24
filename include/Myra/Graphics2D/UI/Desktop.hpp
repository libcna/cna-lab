// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Events/CancellableEventArgsT.hpp"
#include "Myra/Events/GenericEventArgs.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/Transform.hpp"
#include "Myra/Graphics2D/UI/InputContext.hpp"
#include "Myra/Graphics2D/UI/ITransformable.hpp"
#include "Myra/Graphics2D/UI/MouseInfo.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "System/IDisposable.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"

namespace Myra::Graphics2D
{
    class IBrush;
    class RenderContext;
}

namespace Myra::Graphics2D::UI
{
    class HorizontalMenu;

    /** @brief Owns, lays out, transforms, and focuses the root widgets of a Myra UI tree. */
    class Desktop final : public ITransformable, public System::IDisposable
    {
      public:
        using WidgetCollection = System::Collections::ObjectModel::ObservableCollection<std::shared_ptr<Widget>>;
        using BoundsFetcher = std::function<Microsoft::Xna::Framework::Rectangle()>;
        using WidgetPredicate = std::function<bool(Widget &)>;
        using WidgetOperation = std::function<bool(Widget &)>;

        Desktop();
        ~Desktop() override;

        Desktop(const Desktop &) = delete;
        Desktop &operator=(const Desktop &) = delete;
        Desktop(Desktop &&) = delete;
        Desktop &operator=(Desktop &&) = delete;

        Events::MyraEventHandlerT<Events::CancellableEventArgsT<Widget *>> WidgetLosingKeyboardFocus;
        Events::MyraEventHandlerT<Events::GenericEventArgs<Widget *>> WidgetGotKeyboardFocus;
        Events::MyraEventHandlerT<Events::CancellableEventArgsT<Widget *>> ContextMenuClosing;
        Events::MyraEventHandlerT<Events::GenericEventArgs<Widget *>> ContextMenuClosed;
        Events::MyraEventHandler MouseMoved;
        Events::MyraEventHandler TouchMoved;
        Events::MyraEventHandler TouchDown;
        Events::MyraEventHandler TouchUp;
        Events::MyraEventHandler TouchDoubleClick;
        Events::MyraEventHandlerT<Events::GenericEventArgs<float>> MouseWheelChanged;
        Events::MyraEventHandlerT<Events::GenericEventArgs<Microsoft::Xna::Framework::Input::Keys>> KeyUp;
        Events::MyraEventHandlerT<Events::GenericEventArgs<Microsoft::Xna::Framework::Input::Keys>> KeyDown;
        Events::MyraEventHandlerT<Events::GenericEventArgs<char16_t>> Char;
        std::function<void(Microsoft::Xna::Framework::Input::Keys)> KeyDownHandler;

        [[nodiscard]] const BoundsFetcher &getBoundsFetcherProperty() const noexcept;
        void setBoundsFetcherProperty(BoundsFetcher value);

        [[nodiscard]] std::shared_ptr<Widget> getRootProperty() const;
        void setRootProperty(std::shared_ptr<Widget> value);

        [[nodiscard]] WidgetCollection &getWidgetsProperty() noexcept;
        [[nodiscard]] const WidgetCollection &getWidgetsProperty() const noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildrenCopyProperty();
        [[nodiscard]] std::shared_ptr<Widget> GetChild(std::size_t index);
        void AddWidget(std::shared_ptr<Widget> widget);
        [[nodiscard]] bool RemoveWidget(const Widget *widget);
        void ClearWidgets();

        [[nodiscard]] HorizontalMenu *getMenuBarProperty() const noexcept;
        [[nodiscard]] std::shared_ptr<Widget> getContextMenuProperty() const;
        void ShowContextMenu(std::shared_ptr<Widget> menu, Microsoft::Xna::Framework::Point position);
        void HideContextMenu();
        /** @brief Returns the currently retained tooltip overlay, if any. */
        [[nodiscard]] std::shared_ptr<Widget> getTooltipProperty() const;
        /** @brief Creates and fits an owner's tooltip through MyraEnvironment's injected factory. */
        void ShowTooltip(Widget &owner, Microsoft::Xna::Framework::Point position);
        void HideTooltip();
        [[nodiscard]] const Microsoft::Xna::Framework::Rectangle &getInternalBoundsProperty() const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getLayoutBoundsProperty() const;

        [[nodiscard]] Widget *getFocusedKeyboardWidgetProperty() const noexcept;
        void setFocusedKeyboardWidgetProperty(Widget *value);

        [[nodiscard]] const Microsoft::Xna::Framework::Point &getPreviousMousePositionProperty() const noexcept;
        [[nodiscard]] const std::optional<Microsoft::Xna::Framework::Point> &
        getPreviousTouchPositionProperty() const noexcept;
        [[nodiscard]] const Microsoft::Xna::Framework::Point &getMousePositionProperty() const noexcept;
        [[nodiscard]] const std::optional<Microsoft::Xna::Framework::Point> &getTouchPositionProperty() const noexcept;
        [[nodiscard]] bool getIsTouchDownProperty() const noexcept;
        [[nodiscard]] float getMouseWheelDeltaProperty() const noexcept;
        [[nodiscard]] const std::array<bool, 0xff> &getDownKeysProperty() const noexcept;
        [[nodiscard]] int getRepeatKeyDownStartInMsProperty() const noexcept;
        void setRepeatKeyDownStartInMsProperty(int value) noexcept;
        [[nodiscard]] int getRepeatKeyDownIntervalInMsProperty() const noexcept;
        void setRepeatKeyDownIntervalInMsProperty(int value) noexcept;
        [[nodiscard]] bool IsKeyDown(Microsoft::Xna::Framework::Input::Keys key) const;
        [[nodiscard]] static bool getIsMobileProperty() noexcept;

        void UpdateMouseInput();
        void UpdateKeyboardInput();
        void UpdateInput();
        /** @brief Hit-tests the current pointer snapshot and queues widget input transitions. */
        void ProcessWidgetInput();
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key);
        void OnChar(char16_t character);

        /** @brief Runs the selected upstream layout/input/event/layout/visual frame pipeline. */
        void Render();
        /** @brief Renders the background and visible roots through the retained graphics context. */
        void RenderVisual();
        [[nodiscard]] bool getIsDisposedProperty() const noexcept;
        void Dispose() override;

        [[nodiscard]] float getOpacityProperty() const noexcept;
        void setOpacityProperty(float value) noexcept;
        [[nodiscard]] const Microsoft::Xna::Framework::Vector2 &getScaleProperty() const noexcept;
        void setScaleProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] const Microsoft::Xna::Framework::Vector2 &getTransformOriginProperty() const noexcept;
        void setTransformOriginProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] float getRotationProperty() const noexcept;
        void setRotationProperty(float value);

        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getBackgroundProperty() const;
        void setBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] bool getHasModalWidgetProperty();

        void InvalidateLayout() noexcept;
        void UpdateLayout();

        /** @brief Visits visible roots and descendants in ascending Z-order. */
        void ProcessWidgets(const WidgetOperation &operation);
        [[nodiscard]] Widget *FindChild(const WidgetPredicate &predicate);
        [[nodiscard]] Widget *FindChildById(const std::string &id);
        [[nodiscard]] std::size_t CalculateTotalWidgets(bool visibleOnly);

        [[nodiscard]] Microsoft::Xna::Framework::Vector2 ToLocal(Microsoft::Xna::Framework::Vector2 source) override;
        [[nodiscard]] Microsoft::Xna::Framework::Vector2 ToGlobal(Microsoft::Xna::Framework::Vector2 position) override;
        [[nodiscard]] Microsoft::Xna::Framework::Point ToLocal(Microsoft::Xna::Framework::Point source);
        [[nodiscard]] Microsoft::Xna::Framework::Point ToGlobal(Microsoft::Xna::Framework::Point position);

        [[nodiscard]] static Microsoft::Xna::Framework::Rectangle DefaultBoundsFetcher();

      private:
        friend class Widget;
        friend struct DesktopTestAccess;

        class ValidatedWidgetCollection final : public WidgetCollection
        {
          protected:
            void InsertItem(SharpRuntime::intcs index, const std::shared_ptr<Widget> &item) override;
            void SetItem(SharpRuntime::intcs index, const std::shared_ptr<Widget> &item) override;
        };

        class InputProcessor final : public IInputEventsProcessor
        {
          public:
            explicit InputProcessor(Desktop &owner) noexcept;
            void Detach() noexcept;
            void ProcessEvent(InputEventType eventType) override;
            void ProcessChar(char16_t character);

          private:
            Desktop *owner_;
        };

        void OnWidgetsChanged();
        void SynchronizeRoots();
        void SynchronizeRootsOnce();
        void ReconcileContextMenuOwnership();
        void ReconcileTooltipOwnership();
        void InitializeTextInput();
        void DisposeTextInput() noexcept;
        void DisposeGraphicsResources();
        void RemoveWidgetFromPreviousOwner(const std::shared_ptr<Widget> &widget);
        [[nodiscard]] std::shared_ptr<Widget> RetainWidget(const Widget *widget) const;
        [[nodiscard]] bool ContainsWidget(const Widget &root, const Widget *target) const;
        void ClearFocusForDetaching(Widget &root);
        void ForceDetachForDestruction(Widget &root) noexcept;
        void ChangeFocus(Widget *value, bool allowCancellation);
        void FocusNextWidget();
        [[nodiscard]] static bool CanFocusWidget(const Widget &widget) noexcept;
        [[nodiscard]] bool IsMenuBarActive() const noexcept;
        void InputOnTouchDown();
        void FixOverWidgetPosition(Widget &widget, Microsoft::Xna::Framework::Point position);
        void setMousePositionProperty(Microsoft::Xna::Framework::Point value);
        void setTouchPositionProperty(std::optional<Microsoft::Xna::Framework::Point> value);
        void setMouseWheelDeltaProperty(float value);
        void QueueInputEvent(InputEventType eventType);
        void ProcessInputEvent(InputEventType eventType);
        void InvalidateTransform();
        void InvalidateWidgetsOrder() noexcept;
        void UpdateTransform();
        void UpdateWidgetsCopy();
        [[nodiscard]] const Graphics2D::Transform &getTransformProperty();
        void setInternalBoundsProperty(Microsoft::Xna::Framework::Rectangle value);

        ValidatedWidgetCollection widgets_;
        std::vector<std::shared_ptr<Widget>> attachedRoots_;
        std::vector<std::shared_ptr<Widget>> widgetsCopy_;
        BoundsFetcher boundsFetcher_;
        Microsoft::Xna::Framework::Rectangle internalBounds_;
        HorizontalMenu *menuBar_ = nullptr;
        Widget *focusedKeyboardWidget_ = nullptr;
        std::shared_ptr<Widget> contextMenu_;
        std::shared_ptr<Widget> tooltip_;
        std::weak_ptr<Widget> tooltipOwner_;
        std::weak_ptr<Widget> previousKeyboardFocus_;
        std::shared_ptr<InputProcessor> inputProcessor_;
        std::shared_ptr<Graphics2D::IBrush> background_;
        std::shared_ptr<Graphics2D::RenderContext> renderContext_;
        float opacity_ = 1.0F;
        Microsoft::Xna::Framework::Vector2 scale_{1.0F, 1.0F};
        Microsoft::Xna::Framework::Vector2 transformOrigin_{0.5F, 0.5F};
        float rotation_ = 0.0F;
        bool layoutDirty_ = true;
        bool widgetsDirty_ = true;
        bool transformDirty_ = true;
        bool synchronizingRoots_ = false;
        bool rootsResyncRequested_ = false;
        bool destroying_ = false;
        bool disposed_ = false;
        bool focusChanging_ = false;
        bool focusClearedDuringCallback_ = false;
        bool pendingFocusChange_ = false;
        Widget *pendingFocus_ = nullptr;
        std::uint64_t layoutInvalidationVersion_ = 0;
        std::optional<Graphics2D::Transform> transform_;
        MouseInfo lastMouseInfo_;
        InputContext inputContext_;
        std::array<bool, 0xff> downKeys_{};
        std::array<bool, 0xff> lastDownKeys_{};
        Microsoft::Xna::Framework::Point previousMousePosition_;
        std::optional<Microsoft::Xna::Framework::Point> previousTouchPosition_;
        Microsoft::Xna::Framework::Point mousePosition_;
        std::optional<Microsoft::Xna::Framework::Point> touchPosition_;
        float mouseWheelDelta_ = 0.0F;
        int repeatKeyDownStartInMs_ = 500;
        int repeatKeyDownIntervalInMs_ = 50;
        std::optional<std::chrono::steady_clock::time_point> lastKeyDown_;
        int keyDownCount_ = 0;
        std::optional<std::uint64_t> textInputToken_;
    };
} // namespace Myra::Graphics2D::UI
