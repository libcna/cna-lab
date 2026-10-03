// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Container.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <vector>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Contract for a widget that exposes a collection of child widgets. */
    class IContainer
    {
    public:
        virtual ~IContainer() = default;

        [[nodiscard]] virtual const std::vector<std::shared_ptr<Widget>>& getWidgetsProperty() const noexcept = 0;
        virtual void AddWidget(std::shared_ptr<Widget> widget) = 0;
        [[nodiscard]] virtual bool RemoveWidget(const Widget* widget) = 0;
    };

    /** @brief Base widget whose default alignment fills its parent. */
    class Container : public Widget, public IContainer
    {
    public:
        Container();
        ~Container() override = default;

        [[nodiscard]] const std::vector<std::shared_ptr<Widget>>& getWidgetsProperty() const noexcept override;
        void AddWidget(std::shared_ptr<Widget> widget) override;
        [[nodiscard]] bool RemoveWidget(const Widget* widget) override;
        [[nodiscard]] bool InputFallsThrough(
            Microsoft::Xna::Framework::Point localPosition) override;

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget& source) override;
    };
}
