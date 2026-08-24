// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

#include "Myra/Events/EventHandlingStrategy.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/MouseInfo.hpp"

namespace Microsoft::Xna::Framework
{
    class Game;

    namespace Graphics
    {
        class GraphicsDevice;
    }
} // namespace Microsoft::Xna::Framework

namespace System
{
    class EventArgs;
    class Object;
} // namespace System

namespace Myra
{
    namespace Graphics2D::UI
    {
        class Widget;
    }

    /**
     * @brief Provides global configuration for Myra UI.
     *
     * The Game and GraphicsDevice properties are checked non-owning references.
     * The caller owns the Game and must either destroy/dispose it while the CNA
     * lifecycle events are intact or call ClearGame() before invalidating it.
     * Like upstream Myra, the environment is intended for the UI/game thread.
     */
    class MyraEnvironment final
    {
      public:
        static constexpr std::size_t KeyStateCount = 0xff;
        using DownKeys = std::array<bool, KeyStateCount>;
        using MouseInfoGetter = std::function<Graphics2D::UI::MouseInfo()>;
        using DownKeysGetter = std::function<void(DownKeys &)>;
        using TooltipCreator = std::function<std::shared_ptr<Graphics2D::UI::Widget>(Graphics2D::UI::Widget &)>;

        MyraEnvironment() = delete;

        [[nodiscard]] static Events::EventHandlingStrategy getEventHandlingModelProperty() noexcept;
        static void setEventHandlingModelProperty(Events::EventHandlingStrategy value) noexcept;

        [[nodiscard]] static bool getDrawWidgetsFramesProperty() noexcept;
        static void setDrawWidgetsFramesProperty(bool value) noexcept;

        [[nodiscard]] static bool getDrawKeyboardFocusedWidgetFrameProperty() noexcept;
        static void setDrawKeyboardFocusedWidgetFrameProperty(bool value) noexcept;

        [[nodiscard]] static bool getDrawMouseHoveredWidgetFrameProperty() noexcept;
        static void setDrawMouseHoveredWidgetFrameProperty(bool value) noexcept;

        [[nodiscard]] static bool getDrawTextGlyphsFramesProperty() noexcept;
        static void setDrawTextGlyphsFramesProperty(bool value) noexcept;

        [[nodiscard]] static bool getDisableClippingProperty() noexcept;
        static void setDisableClippingProperty(bool value) noexcept;

        [[nodiscard]] static bool getSetMouseCursorFromWidgetProperty() noexcept;
        static void setSetMouseCursorFromWidgetProperty(bool value) noexcept;

        /** @brief Returns the current Myra cursor type. */
        [[nodiscard]] static Graphics2D::UI::MouseCursorType getMouseCursorTypeProperty() noexcept;

        /** @brief Maps a cursor type and applies it through CNA when linked. */
        static void setMouseCursorTypeProperty(Graphics2D::UI::MouseCursorType value);

        [[nodiscard]] static Graphics2D::UI::MouseCursorType getDefaultMouseCursorTypeProperty() noexcept;
        static void setDefaultMouseCursorTypeProperty(Graphics2D::UI::MouseCursorType value) noexcept;

        [[nodiscard]] static const MouseInfoGetter &getMouseInfoGetterProperty() noexcept;
        static void setMouseInfoGetterProperty(MouseInfoGetter value);
        [[nodiscard]] static const DownKeysGetter &getDownKeysGetterProperty() noexcept;
        static void setDownKeysGetterProperty(DownKeysGetter value);

        [[nodiscard]] static int getDoubleClickIntervalInMsProperty() noexcept;
        static void setDoubleClickIntervalInMsProperty(int value) noexcept;
        [[nodiscard]] static int getDoubleClickRadiusProperty() noexcept;
        static void setDoubleClickRadiusProperty(int value) noexcept;

        [[nodiscard]] static int getTooltipDelayInMsProperty() noexcept;
        static void setTooltipDelayInMsProperty(int value) noexcept;
        [[nodiscard]] static const Microsoft::Xna::Framework::Point &getTooltipOffsetProperty() noexcept;
        static void setTooltipOffsetProperty(Microsoft::Xna::Framework::Point value) noexcept;
        /** @brief Gets the injected tooltip factory; empty until Label/style support is ported. */
        [[nodiscard]] static const TooltipCreator &getTooltipCreatorProperty() noexcept;
        static void setTooltipCreatorProperty(TooltipCreator value);

        /** @brief Reads the current CNA mouse snapshot relative to the active viewport. */
        [[nodiscard]] static Graphics2D::UI::MouseInfo DefaultMouseInfoGetter();
        /** @brief Reads the current CNA keyboard snapshot into the fixed upstream key domain. */
        static void DefaultDownKeysGetter(DownKeys &keys);

        /** @brief Returns the configured non-owning Game reference. */
        [[nodiscard]] static Microsoft::Xna::Framework::Game &getGameProperty();

        /**
         * @brief Selects the caller-owned Game used by Myra.
         *
         * Replacing the Game detaches all lifecycle subscriptions from the old
         * instance. Passing a reference makes upstream's non-null requirement
         * explicit in the C++ API.
         */
        static void setGameProperty(Microsoft::Xna::Framework::Game &value);

        /** @brief Returns the live GraphicsDevice belonging to the configured Game. */
        [[nodiscard]] static Microsoft::Xna::Framework::Graphics::GraphicsDevice &getGraphicsDeviceProperty();

        /**
         * @brief Detaches lifecycle subscriptions and clears the non-owning Game reference.
         *
         * Calling this before destroying a custom Game/device-service arrangement
         * is the explicit fallback when that arrangement does not raise CNA's
         * normal Game::Disposed or GraphicsDevice::Disposing event.
         */
        static void ClearGame() noexcept;

      private:
        static Events::EventHandlingStrategy eventHandlingModel_;
        static bool drawWidgetsFrames_;
        static bool drawKeyboardFocusedWidgetFrame_;
        static bool drawMouseHoveredWidgetFrame_;
        static bool drawTextGlyphsFrames_;
        static bool disableClipping_;
        static bool setMouseCursorFromWidget_;
        static Graphics2D::UI::MouseCursorType mouseCursorType_;
        static Graphics2D::UI::MouseCursorType defaultMouseCursorType_;
        static MouseInfoGetter mouseInfoGetter_;
        static DownKeysGetter downKeysGetter_;
        static int doubleClickIntervalInMs_;
        static int doubleClickRadius_;
        static int tooltipDelayInMs_;
        static Microsoft::Xna::Framework::Point tooltipOffset_;
        static TooltipCreator tooltipCreator_;
        static Microsoft::Xna::Framework::Game *game_;
        static Microsoft::Xna::Framework::Graphics::GraphicsDevice *graphicsDevice_;
        static std::optional<std::size_t> gameDisposedToken_;
        static std::optional<std::size_t> graphicsDeviceDisposingToken_;

        static void ApplyMouseCursorType(Graphics2D::UI::MouseCursorType value);
        static void OnGameDisposed(System::Object *sender, const System::EventArgs &eventArgs);
        static void OnGraphicsDeviceDisposing(System::Object *sender, const System::EventArgs &eventArgs);
    };
} // namespace Myra
