// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/TabItem.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <memory>
#include <optional>
#include <string>

#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/IContent.hpp"
#include "Myra/Graphics2D/UI/Selectors/ISelectorItem.hpp"
#include "Myra/MML/BaseObject.hpp"

namespace Myra::Graphics2D
{
    class IImage;
}

namespace Myra::Graphics2D::UI
{
    /** @brief Data and retained content for one TabControl selector item. */
    class TabItem final : public MML::BaseObject, public ISelectorItem, public Myra::Graphics2D::IContent
    {
      public:
        TabItem() = default;
        explicit TabItem(std::optional<std::string> text, std::shared_ptr<Widget> content = nullptr);
        ~TabItem() override = default;

        Events::MyraEventHandler Changed;
        Events::MyraEventHandler SelectedChanged;

        [[nodiscard]] const std::optional<std::string> &getTextProperty() const noexcept;
        void setTextProperty(std::optional<std::string> value);
        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;
        [[nodiscard]] const std::any &getTagProperty() const noexcept;
        void setTagProperty(std::any value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getImageProperty() const;
        void setImageProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] int getImageTextSpacingProperty() const noexcept;
        void setImageTextSpacingProperty(int value) noexcept;
        [[nodiscard]] const std::optional<int> &getHeightProperty() const noexcept;
        void setHeightProperty(std::optional<int> value) noexcept;

        [[nodiscard]] bool getIsSelectedProperty() const noexcept override;
        void setIsSelectedProperty(bool value) noexcept override;
        [[nodiscard]] std::string ToString() const;
        [[nodiscard]] std::shared_ptr<TabItem> Clone() const;

      protected:
        void OnIdChanged() override;

      private:
        void FireChanged();

        std::shared_ptr<Widget> content_;
        std::optional<std::string> text_;
        std::any tag_;
        std::shared_ptr<Graphics2D::IImage> image_;
        std::optional<int> height_;
        int imageTextSpacing_ = 0;
        bool isSelected_ = false;
    };
} // namespace Myra::Graphics2D::UI
