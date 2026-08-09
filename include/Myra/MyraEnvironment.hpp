// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstddef>
#include <optional>

#include "Myra/Events/EventHandlingStrategy.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"

namespace Microsoft::Xna::Framework
{
    class Game;

    namespace Graphics
    {
        class GraphicsDevice;
    }
}

namespace System
{
    class EventArgs;
    class Object;
}

namespace Myra
{
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

        /** @brief Maps and applies a Myra cursor type through CNA Mouse::SetCursor. */
        static void setMouseCursorTypeProperty(Graphics2D::UI::MouseCursorType value);

        [[nodiscard]] static Graphics2D::UI::MouseCursorType
            getDefaultMouseCursorTypeProperty() noexcept;
        static void setDefaultMouseCursorTypeProperty(
            Graphics2D::UI::MouseCursorType value) noexcept;

        /** @brief Returns the configured non-owning Game reference. */
        [[nodiscard]] static Microsoft::Xna::Framework::Game& getGameProperty();

        /**
         * @brief Selects the caller-owned Game used by Myra.
         *
         * Replacing the Game detaches all lifecycle subscriptions from the old
         * instance. Passing a reference makes upstream's non-null requirement
         * explicit in the C++ API.
         */
        static void setGameProperty(Microsoft::Xna::Framework::Game& value);

        /** @brief Returns the live GraphicsDevice belonging to the configured Game. */
        [[nodiscard]] static Microsoft::Xna::Framework::Graphics::GraphicsDevice&
            getGraphicsDeviceProperty();

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
        static Microsoft::Xna::Framework::Game* game_;
        static Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice_;
        static std::optional<std::size_t> gameDisposedToken_;
        static std::optional<std::size_t> graphicsDeviceDisposingToken_;

        static void OnGameDisposed(System::Object* sender, const System::EventArgs& eventArgs);
        static void OnGraphicsDeviceDisposing(System::Object* sender, const System::EventArgs& eventArgs);
    };
}
