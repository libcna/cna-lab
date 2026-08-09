// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <stdexcept>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseCursor.hpp"
#include "Myra/DefaultAssets.hpp"
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

namespace Myra
{
    namespace
    {
        using Myra::Graphics2D::UI::MouseCursorType;
        using Microsoft::Xna::Framework::Input::MouseCursor;

        constexpr const char* MissingGameMessage =
            "MyraEnvironment.Game is not set. Set it to the caller-owned Game instance before using Myra.";
        constexpr const char* DisposedDeviceMessage =
            "MyraEnvironment.GraphicsDevice is disposed. Configure a live Game before using graphics features.";

        [[nodiscard]] MouseCursor& ResolveMouseCursor(const MouseCursorType value)
        {
            switch (value)
            {
            case MouseCursorType::Arrow:
                return MouseCursor::getArrowProperty();
            case MouseCursorType::IBeam:
                return MouseCursor::getIBeamProperty();
            case MouseCursorType::Wait:
                return MouseCursor::getWaitProperty();
            case MouseCursorType::Crosshair:
                return MouseCursor::getCrosshairProperty();
            case MouseCursorType::WaitArrow:
                return MouseCursor::getWaitArrowProperty();
            case MouseCursorType::SizeNWSE:
                return MouseCursor::getSizeNWSEProperty();
            case MouseCursorType::SizeNESW:
                return MouseCursor::getSizeNESWProperty();
            case MouseCursorType::SizeWE:
                return MouseCursor::getSizeWEProperty();
            case MouseCursorType::SizeNS:
                return MouseCursor::getSizeNSProperty();
            case MouseCursorType::SizeAll:
                return MouseCursor::getSizeAllProperty();
            case MouseCursorType::No:
                return MouseCursor::getNoProperty();
            case MouseCursorType::Hand:
                return MouseCursor::getHandProperty();
            }

            throw std::invalid_argument("Could not map the requested Myra mouse cursor type.");
        }
    }

    Microsoft::Xna::Framework::Game* MyraEnvironment::game_ = nullptr;
    Microsoft::Xna::Framework::Graphics::GraphicsDevice* MyraEnvironment::graphicsDevice_ = nullptr;
    std::optional<std::size_t> MyraEnvironment::gameDisposedToken_;
    std::optional<std::size_t> MyraEnvironment::graphicsDeviceDisposingToken_;

    void MyraEnvironment::setMouseCursorTypeProperty(
        const Graphics2D::UI::MouseCursorType value)
    {
        if (mouseCursorType_ == value)
        {
            return;
        }

        // Preserve upstream assignment ordering: an invalid cast is retained in
        // the backing property before cursor lookup reports that it is unmapped.
        mouseCursorType_ = value;
        Microsoft::Xna::Framework::Input::Mouse::SetCursor(ResolveMouseCursor(value));
    }

    Microsoft::Xna::Framework::Game& MyraEnvironment::getGameProperty()
    {
        if (game_ == nullptr)
        {
            throw std::logic_error(MissingGameMessage);
        }
        return *game_;
    }

    void MyraEnvironment::setGameProperty(Microsoft::Xna::Framework::Game& value)
    {
        Microsoft::Xna::Framework::Graphics::GraphicsDevice& device = value.getGraphicsDeviceProperty();
        if (device.getIsDisposedProperty())
        {
            throw std::invalid_argument(
                "MyraEnvironment.Game must expose a GraphicsDevice that has not been disposed.");
        }

        if (game_ == &value && graphicsDevice_ == &device)
        {
            return;
        }

        ClearGame();

        const std::size_t gameToken = value.Disposed.Add(OnGameDisposed);
        try
        {
            const std::size_t deviceToken = device.Disposing.Add(OnGraphicsDeviceDisposing);
            game_ = &value;
            graphicsDevice_ = &device;
            gameDisposedToken_ = gameToken;
            graphicsDeviceDisposingToken_ = deviceToken;
        }
        catch (...)
        {
            value.Disposed.Remove(gameToken);
            throw;
        }
    }

    Microsoft::Xna::Framework::Graphics::GraphicsDevice&
        MyraEnvironment::getGraphicsDeviceProperty()
    {
        if (game_ == nullptr || graphicsDevice_ == nullptr)
        {
            throw std::logic_error(MissingGameMessage);
        }
        if (graphicsDevice_->getIsDisposedProperty())
        {
            ClearGame();
            throw std::logic_error(DisposedDeviceMessage);
        }
        return *graphicsDevice_;
    }

    void MyraEnvironment::ClearGame() noexcept
    {
        DefaultAssets::Dispose();

        Microsoft::Xna::Framework::Game* const oldGame = game_;
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* const oldDevice = graphicsDevice_;
        const std::optional<std::size_t> oldGameToken = gameDisposedToken_;
        const std::optional<std::size_t> oldDeviceToken = graphicsDeviceDisposingToken_;

        game_ = nullptr;
        graphicsDevice_ = nullptr;
        gameDisposedToken_.reset();
        graphicsDeviceDisposingToken_.reset();

        if (oldGame != nullptr && oldGameToken)
        {
            oldGame->Disposed.Remove(*oldGameToken);
        }
        if (oldDevice != nullptr && oldDeviceToken)
        {
            oldDevice->Disposing.Remove(*oldDeviceToken);
        }
    }

    void MyraEnvironment::OnGameDisposed(System::Object* sender, const System::EventArgs& eventArgs)
    {
        (void) eventArgs;
        if (game_ != nullptr && sender == static_cast<System::Object*>(game_))
        {
            ClearGame();
        }
    }

    void MyraEnvironment::OnGraphicsDeviceDisposing(
        System::Object* sender, const System::EventArgs& eventArgs)
    {
        (void) eventArgs;
        if (graphicsDevice_ != nullptr && sender == static_cast<System::Object*>(graphicsDevice_))
        {
            ClearGame();
        }
    }
}
