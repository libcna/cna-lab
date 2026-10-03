// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Proportion.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <string>

#include "Myra/Events/MyraEventHandler.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Specifies how a grid or stack cell divides available space. */
    enum class ProportionType
    {
        Auto,
        Part,
        Fill,
        Pixels
    };

    /** @brief Defines the size rule for one grid or stack cell. */
    class Proportion
    {
    public:
        // C# exposes these as readonly references to mutable objects.  shared_ptr
        // preserves that reference identity in the C++ object model.
        static const std::shared_ptr<Proportion> Auto;
        static const std::shared_ptr<Proportion> Fill;
        static const std::shared_ptr<Proportion> GridDefault;
        static const std::shared_ptr<Proportion> StackPanelDefault;

        Events::MyraEventHandler Changed;

        Proportion() = default;
        explicit Proportion(ProportionType type) noexcept;
        Proportion(ProportionType type, float value) noexcept;

        [[nodiscard]] ProportionType getTypeProperty() const noexcept;
        void setTypeProperty(ProportionType value);
        [[nodiscard]] float getValueProperty() const noexcept;
        void setValueProperty(float value);
        [[nodiscard]] std::string ToString() const;

    private:
        void FireChanged();

        ProportionType type_ = ProportionType::Auto;
        float value_ = 1.0F;
    };

    /**
     * @brief C++ counterpart of ObservableCollection&lt;Proportion&gt;.
     *
     * A C# collection stores references to mutable Proportion objects.  The
     * shared pointers preserve that identity and let owners remove their
     * subscriptions safely when a proportion leaves a collection.
     */
    using ProportionCollection = System::Collections::ObjectModel::ObservableCollection<std::shared_ptr<Proportion>>;
}
