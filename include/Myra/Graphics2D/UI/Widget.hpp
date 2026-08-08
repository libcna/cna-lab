// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Children.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/Transform.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/ILayout.hpp"
#include "Myra/Graphics2D/UI/ITransformable.hpp"
#include "Myra/MML/BaseObject.hpp"

namespace Myra::Graphics2D::UI
{
    class Desktop;

    /** @brief Specifies the directions in which a widget may be dragged. */
    enum class DragDirection
    {
        None = 0,
        Vertical = 1,
        Horizontal = 2,
        Both = Vertical | Horizontal
    };

    /** @brief Base class for retained-mode Myra UI widgets. */
    class Widget : public MML::BaseObject, public ITransformable
    {
    public:
        Widget();
        ~Widget() override;

        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&) = delete;
        Widget(Widget&&) = delete;
        Widget& operator=(Widget&&) = delete;

        Events::MyraEventHandler VisibleChanged;
        Events::MyraEventHandler EnabledChanged;
        Events::MyraEventHandler LocationChanged;
        Events::MyraEventHandler SizeChanged;
        Events::MyraEventHandler ArrangeUpdated;

        [[nodiscard]] int getLeftProperty() const noexcept;
        void setLeftProperty(int value);
        [[nodiscard]] int getTopProperty() const noexcept;
        void setTopProperty(int value);

        [[nodiscard]] const std::optional<int>& getMinWidthProperty() const noexcept;
        void setMinWidthProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int>& getMaxWidthProperty() const noexcept;
        void setMaxWidthProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int>& getWidthProperty() const noexcept;
        void setWidthProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int>& getMinHeightProperty() const noexcept;
        void setMinHeightProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int>& getMaxHeightProperty() const noexcept;
        void setMaxHeightProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int>& getHeightProperty() const noexcept;
        void setHeightProperty(std::optional<int> value);

        [[nodiscard]] const Thickness& getMarginProperty() const noexcept;
        void setMarginProperty(Thickness value);
        [[nodiscard]] const Thickness& getBorderThicknessProperty() const noexcept;
        void setBorderThicknessProperty(Thickness value);
        [[nodiscard]] const Thickness& getPaddingProperty() const noexcept;
        void setPaddingProperty(Thickness value);

        [[nodiscard]] virtual HorizontalAlignment getHorizontalAlignmentProperty() const noexcept;
        virtual void setHorizontalAlignmentProperty(HorizontalAlignment value);
        [[nodiscard]] virtual VerticalAlignment getVerticalAlignmentProperty() const noexcept;
        virtual void setVerticalAlignmentProperty(VerticalAlignment value);

        [[nodiscard]] bool getEnabledProperty() const noexcept;
        void setEnabledProperty(bool value);
        [[nodiscard]] bool getVisibleProperty() const noexcept;
        void setVisibleProperty(bool value);
        [[nodiscard]] int getZIndexProperty() const noexcept;
        void setZIndexProperty(int value);
        [[nodiscard]] float getOpacityProperty() const noexcept;
        void setOpacityProperty(float value);

        [[nodiscard]] const Microsoft::Xna::Framework::Vector2& getScaleProperty() const noexcept;
        void setScaleProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] const Microsoft::Xna::Framework::Vector2& getTransformOriginProperty() const noexcept;
        void setTransformOriginProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] float getRotationProperty() const noexcept;
        void setRotationProperty(float value);

        [[nodiscard]] const Microsoft::Xna::Framework::Rectangle& getBoundsProperty() const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getActualBoundsProperty() const noexcept;
        [[nodiscard]] const Microsoft::Xna::Framework::Rectangle& getContainerBoundsProperty() const noexcept;
        [[nodiscard]] int getMBPWidthProperty() const noexcept;
        [[nodiscard]] int getMBPHeightProperty() const noexcept;
        [[nodiscard]] Widget* getParentProperty() const noexcept;

        [[nodiscard]] ILayout* getChildrenLayoutProperty() const noexcept;
        void setChildrenLayoutProperty(ILayout* value) noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<Widget>>& getChildrenProperty() const noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<Widget>>& getChildrenCopyProperty();
        void AddChild(std::shared_ptr<Widget> child);
        [[nodiscard]] bool RemoveChild(const Widget* child);
        void ClearChildren();
        void RemoveFromParent();

        [[nodiscard]] Microsoft::Xna::Framework::Point Measure(
            Microsoft::Xna::Framework::Point availableSize);
        void Arrange(Microsoft::Xna::Framework::Rectangle containerBounds);
        void UpdateArrange();
        virtual void InvalidateMeasure();
        void InvalidateArrange() noexcept;

        [[nodiscard]] Microsoft::Xna::Framework::Vector2 ToLocal(
            Microsoft::Xna::Framework::Vector2 source) override;
        [[nodiscard]] Microsoft::Xna::Framework::Vector2 ToGlobal(
            Microsoft::Xna::Framework::Vector2 position) override;
        [[nodiscard]] Microsoft::Xna::Framework::Point ToLocal(
            Microsoft::Xna::Framework::Point source);
        [[nodiscard]] Microsoft::Xna::Framework::Point ToGlobal(
            Microsoft::Xna::Framework::Point position);
        [[nodiscard]] bool ContainsGlobalPoint(Microsoft::Xna::Framework::Point globalPosition);

        void OnAttachedPropertyLayoutChanged(MML::AttachedPropertyOption option) override;

    protected:
        [[nodiscard]] virtual Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize);
        virtual void InternalArrange();
        virtual void OnVisibleChanged();
        virtual void OnChildAdded(Widget& child);
        virtual void OnChildRemoved(Widget& child);
        void InvalidateTransform();

    private:
        [[nodiscard]] const Graphics2D::Transform& getTransformProperty();
        void UpdateTransform();
        void UpdateChildren();
        void FireLocationChanged();
        void FireSizeChanged();

        Thickness margin_;
        Thickness borderThickness_;
        Thickness padding_;
        int left_ = 0;
        int top_ = 0;
        std::optional<int> minWidth_;
        std::optional<int> minHeight_;
        std::optional<int> maxWidth_;
        std::optional<int> maxHeight_;
        std::optional<int> width_;
        std::optional<int> height_;
        int zIndex_ = 0;
        HorizontalAlignment horizontalAlignment_ = HorizontalAlignment::Left;
        VerticalAlignment verticalAlignment_ = VerticalAlignment::Top;
        bool measureDirty_ = true;
        bool arrangeDirty_ = true;
        Microsoft::Xna::Framework::Point lastMeasureSize_;
        Microsoft::Xna::Framework::Point lastMeasureAvailableSize_;
        Microsoft::Xna::Framework::Rectangle containerBounds_;
        Microsoft::Xna::Framework::Rectangle layoutBounds_;
        bool visible_ = true;
        bool enabled_ = true;
        float opacity_ = 1.0F;
        Microsoft::Xna::Framework::Vector2 scale_{1.0F, 1.0F};
        Microsoft::Xna::Framework::Vector2 transformOrigin_{0.5F, 0.5F};
        float rotation_ = 0.0F;
        bool transformDirty_ = true;
        std::optional<Graphics2D::Transform> transform_;
        Widget* parent_ = nullptr;
        Desktop* desktop_ = nullptr;
        ILayout* childrenLayout_ = nullptr;
        bool childrenDirty_ = true;
        std::vector<std::shared_ptr<Widget>> children_;
        std::vector<std::shared_ptr<Widget>> childrenCopy_;
    };
}
