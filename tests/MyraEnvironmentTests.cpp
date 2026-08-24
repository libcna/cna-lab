// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <stdexcept>

#include "CNA/Platform/PlatformException.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::UI::MouseCursorType;

    void SetCursorWhenSupported(const MouseCursorType cursorType)
    {
        try
        {
            MyraEnvironment::setMouseCursorTypeProperty(cursorType);
        }
        catch (const CNA::Platform::PlatformException &exception)
        {
            // The SOFTWARE renderer's windowless test platform correctly
            // rejects native cursor creation. Mapping occurs before this CNA
            // platform operation, and SDL-backed display tests cover applying
            // the cursor to a real window.
            EXPECT_EQ(exception.GetOperation(), "Mouse::SetCursor");
        }
    }

    class MyraEnvironmentTests : public testing::Test
    {
      protected:
        void SetUp() override { MyraEnvironment::ClearGame(); }

        void TearDown() override
        {
            MyraEnvironment::setEnableModalDarkeningProperty(false);
            MyraEnvironment::setDarkeningColorProperty(Color(0, 0, 0, 192));
            MyraEnvironment::ClearGame();
        }
    };

    TEST_F(MyraEnvironmentTests, RejectsGraphicsAccessBeforeAGameIsConfigured)
    {
        EXPECT_THROW((void)MyraEnvironment::getGameProperty(), std::logic_error);
        EXPECT_THROW((void)MyraEnvironment::getGraphicsDeviceProperty(), std::logic_error);
    }

    TEST_F(MyraEnvironmentTests, ExposesTheConfiguredGameAndItsLiveGraphicsDevice)
    {
        Game game;

        MyraEnvironment::setGameProperty(game);

        EXPECT_EQ(&MyraEnvironment::getGameProperty(), &game);
        EXPECT_EQ(&MyraEnvironment::getGraphicsDeviceProperty(), &game.getGraphicsDeviceProperty());
        EXPECT_FALSE(MyraEnvironment::getEnableModalDarkeningProperty());
        EXPECT_EQ(MyraEnvironment::getDarkeningColorProperty(), Color(0, 0, 0, 192));

        MyraEnvironment::setEnableModalDarkeningProperty(true);
        MyraEnvironment::setDarkeningColorProperty(Color(7, 11, 13, 17));
        EXPECT_TRUE(MyraEnvironment::getEnableModalDarkeningProperty());
        EXPECT_EQ(MyraEnvironment::getDarkeningColorProperty(), Color(7, 11, 13, 17));

        const std::size_t disposedSubscriptions = game.Disposed.Size();
        const std::size_t deviceSubscriptions = game.getGraphicsDeviceProperty().Disposing.Size();
        MyraEnvironment::setGameProperty(game);
        EXPECT_EQ(game.Disposed.Size(), disposedSubscriptions);
        EXPECT_EQ(game.getGraphicsDeviceProperty().Disposing.Size(), deviceSubscriptions);
    }

    TEST_F(MyraEnvironmentTests, ExplicitGameDisposalClearsTheNonOwningEnvironmentReference)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);

        game.Dispose();

        EXPECT_THROW((void)MyraEnvironment::getGameProperty(), std::logic_error);
        EXPECT_THROW((void)MyraEnvironment::getGraphicsDeviceProperty(), std::logic_error);
    }

    TEST_F(MyraEnvironmentTests, GameDestructionClearsTheReferenceThroughDeviceDisposal)
    {
        auto game = std::make_unique<Game>();
        MyraEnvironment::setGameProperty(*game);

        game.reset();

        EXPECT_THROW((void)MyraEnvironment::getGameProperty(), std::logic_error);
        EXPECT_THROW((void)MyraEnvironment::getGraphicsDeviceProperty(), std::logic_error);
    }

    TEST_F(MyraEnvironmentTests, ReplacingTheGameDetachesTheOldLifecycleSubscriptions)
    {
        Game first;
        Game second;
        MyraEnvironment::setGameProperty(first);
        MyraEnvironment::setGameProperty(second);

        first.Dispose();

        EXPECT_EQ(&MyraEnvironment::getGameProperty(), &second);
        EXPECT_EQ(&MyraEnvironment::getGraphicsDeviceProperty(), &second.getGraphicsDeviceProperty());
    }

    TEST_F(MyraEnvironmentTests, MapsEverySupportedMouseCursorTypeThroughCna)
    {
        constexpr std::array cursorTypes{
            MouseCursorType::IBeam,     MouseCursorType::Wait,     MouseCursorType::Crosshair,
            MouseCursorType::WaitArrow, MouseCursorType::SizeNWSE, MouseCursorType::SizeNESW,
            MouseCursorType::SizeWE,    MouseCursorType::SizeNS,   MouseCursorType::SizeAll,
            MouseCursorType::No,        MouseCursorType::Hand,     MouseCursorType::Arrow,
        };

        for (const MouseCursorType cursorType : cursorTypes)
        {
            SetCursorWhenSupported(cursorType);
            EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), cursorType);
        }
    }

    TEST_F(MyraEnvironmentTests, RejectsAnUnmappedMouseCursorValue)
    {
        const auto invalid = static_cast<MouseCursorType>(1000);

        EXPECT_THROW(MyraEnvironment::setMouseCursorTypeProperty(invalid), std::invalid_argument);
        EXPECT_EQ(MyraEnvironment::getMouseCursorTypeProperty(), invalid);

        SetCursorWhenSupported(MouseCursorType::Arrow);
    }
} // namespace
