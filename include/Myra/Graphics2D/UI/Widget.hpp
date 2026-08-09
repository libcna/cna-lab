// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs and src/Myra/Graphics2D/UI/Widget.Children.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <any>
#include <array>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Events/ValueChangingEventArgs.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/Transform.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/ILayout.hpp"
#include "Myra/Graphics2D/UI/ITransformable.hpp"
#include "Myra/MML/BaseObject.hpp"

namespace Myra::Graphics2D
{
    class IBrush;
    class RenderContext;
}

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
        Events::MyraEventHandler KeyboardFocusChanged;
        Events::MyraEventHandler PressedChanged;
        Events::MyraEventHandlerT<Events::ValueChangingEventArgs<bool>> PressedChangingByUser;

        using RenderCallback = std::function<void(Graphics2D::RenderContext&)>;
        RenderCallback BeforeRender;
        RenderCallback AfterRender;

        [[nodiscard]] const std::optional<std::string>& getStyleNameProperty() const noexcept;
        void setStyleNameProperty(std::optional<std::string> value);

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
        [[nodiscard]] virtual DragDirection getDragDirectionProperty() const noexcept;
        virtual void setDragDirectionProperty(DragDirection value) noexcept;
        [[nodiscard]] bool getIsDraggableProperty() const noexcept;
        [[nodiscard]] int getZIndexProperty() const noexcept;
        void setZIndexProperty(int value);
        [[nodiscard]] virtual const std::optional<MouseCursorType>& getMouseCursorProperty() const noexcept;
        virtual void setMouseCursorProperty(std::optional<MouseCursorType> value);
        [[nodiscard]] const std::optional<std::string>& getTooltipProperty() const noexcept;
        void setTooltipProperty(std::optional<std::string> value);
        [[nodiscard]] bool getIsModalProperty() const noexcept;
        void setIsModalProperty(bool value) noexcept;
        [[nodiscard]] float getOpacityProperty() const noexcept;
        void setOpacityProperty(float value);

        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getBackgroundProperty() const;
        void setBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getOverBackgroundProperty() const;
        void setOverBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getDisabledBackgroundProperty() const;
        void setDisabledBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getFocusedBackgroundProperty() const;
        void setFocusedBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getPressedBackgroundProperty() const;
        void setPressedBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);

        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getBorderProperty() const;
        void setBorderProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getOverBorderProperty() const;
        void setOverBorderProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getDisabledBorderProperty() const;
        void setDisabledBorderProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getFocusedBorderProperty() const;
        void setFocusedBorderProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getPressedBorderProperty() const;
        void setPressedBorderProperty(std::shared_ptr<Graphics2D::IBrush> value);

        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> GetCurrentBackground() const;
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> GetCurrentBorder() const;
        [[nodiscard]] virtual bool getIsPressedProperty() const noexcept;
        virtual void setIsPressedProperty(bool value);
        [[nodiscard]] virtual bool getClipToBoundsProperty() const noexcept;
        virtual void setClipToBoundsProperty(bool value) noexcept;
        [[nodiscard]] bool getAcceptsKeyboardFocusProperty() const noexcept;
        void setAcceptsKeyboardFocusProperty(bool value) noexcept;
        [[nodiscard]] bool getIsKeyboardFocusedProperty() const noexcept;

        [[nodiscard]] const Microsoft::Xna::Framework::Vector2& getScaleProperty() const noexcept;
        void setScaleProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] const Microsoft::Xna::Framework::Vector2& getTransformOriginProperty() const noexcept;
        void setTransformOriginProperty(Microsoft::Xna::Framework::Vector2 value);
        [[nodiscard]] float getRotationProperty() const noexcept;
        void setRotationProperty(float value);
        [[nodiscard]] Widget* getDragHandleProperty() const noexcept;
        void setDragHandleProperty(Widget* value) noexcept;

        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getBoundsProperty() const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getActualBoundsProperty() const;
        [[nodiscard]] const Microsoft::Xna::Framework::Rectangle& getContainerBoundsProperty() const noexcept;
        [[nodiscard]] int getMBPWidthProperty() const;
        [[nodiscard]] int getMBPHeightProperty() const;
        [[nodiscard]] Widget* getParentProperty() const noexcept;
        [[nodiscard]] const std::any& getTagProperty() const noexcept;
        void setTagProperty(std::any value);

        [[nodiscard]] ILayout* getChildrenLayoutProperty() const noexcept;
        void setChildrenLayoutProperty(ILayout* value) noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<Widget>>& getChildrenProperty() const noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<Widget>>& getChildrenCopyProperty();
        void AddChild(std::shared_ptr<Widget> child);
        [[nodiscard]] bool RemoveChild(const Widget* child);
        void ClearChildren();
        void RemoveFromParent();

        /** @brief Creates a deep copy using the dynamic type's virtual factory. */
        [[nodiscard]] std::shared_ptr<Widget> Clone() const;

        using WidgetPredicate = std::function<bool(Widget&)>;

        /** @brief Counts descendants, optionally excluding invisible subtrees. */
        [[nodiscard]] std::size_t CalculateTotalChildCount(bool visibleOnly);

        /** @brief Finds the first descendant with the requested identifier in Z-order. */
        [[nodiscard]] Widget* FindChildById(const std::string& id);

        /** @brief Returns the requested descendant or throws when it is absent. */
        [[nodiscard]] Widget& EnsureWidgetById(const std::string& id);

        /** @brief Finds the first descendant matching a predicate in depth-first Z-order. */
        [[nodiscard]] Widget* FindChild(const WidgetPredicate& predicate = {});

        /** @brief Returns matching direct children or descendants in depth-first Z-order. */
        [[nodiscard]] std::vector<Widget*> GetChildren(
            bool recursive = false, const WidgetPredicate& predicate = {});

        template<typename WidgetT>
            requires std::derived_from<WidgetT, Widget>
        [[nodiscard]] WidgetT* FindChildById(const std::string& id)
        {
            return FindChild<WidgetT>([&id](WidgetT& widget) {
                const std::optional<std::string>& widgetId = widget.getIdProperty();
                return widgetId.has_value() && *widgetId == id;
            });
        }

        template<typename WidgetT>
            requires std::derived_from<WidgetT, Widget>
        [[nodiscard]] WidgetT* FindChild(
            const std::function<bool(WidgetT&)>& predicate = {})
        {
            const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
            for (const std::shared_ptr<Widget>& widget : snapshot)
            {
                WidgetT* const casted = dynamic_cast<WidgetT*>(widget.get());
                if (casted != nullptr && (!predicate || predicate(*casted)))
                {
                    return casted;
                }
                if (WidgetT* const descendant = widget->FindChild<WidgetT>(predicate))
                {
                    return descendant;
                }
            }
            return nullptr;
        }

        [[nodiscard]] Microsoft::Xna::Framework::Point Measure(
            Microsoft::Xna::Framework::Point availableSize);
        void Arrange(Microsoft::Xna::Framework::Rectangle containerBounds);
        void UpdateArrange();
        virtual void InvalidateMeasure();
        void InvalidateArrange() noexcept;

        /** @brief Renders this widget, its decoration, and its retained child snapshot. */
        void Render(Graphics2D::RenderContext& context);

        /** @brief Renders widget-specific content; the default traverses children in Z-order. */
        virtual void InternalRender(Graphics2D::RenderContext& context);

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
        virtual void OnLostKeyboardFocus();
        virtual void OnGotKeyboardFocus();
        virtual void OnPressedChanged();

    protected:
        static constexpr std::size_t WidgetVisualStateNormal = 0;
        static constexpr std::size_t WidgetVisualStateDisabled = 1;
        static constexpr std::size_t WidgetVisualStateOver = 2;
        static constexpr std::size_t WidgetVisualStateFocused = 3;
        static constexpr std::size_t WidgetVisualStatePressed = 4;
        static constexpr std::size_t WidgetVisualStateTotal = 5;

        template<typename VisualT>
        [[nodiscard]] std::shared_ptr<VisualT> GetCurrentVisual(
            const std::array<std::shared_ptr<VisualT>, WidgetVisualStateTotal>& values) const
        {
            std::shared_ptr<VisualT> result = values[WidgetVisualStateNormal];
            if (enabled_)
            {
                if (isPressed_ && values[WidgetVisualStatePressed])
                {
                    return values[WidgetVisualStatePressed];
                }
                if (isKeyboardFocused_ && values[WidgetVisualStateFocused])
                {
                    return values[WidgetVisualStateFocused];
                }
                if (UseOverBackground() && values[WidgetVisualStateOver])
                {
                    return values[WidgetVisualStateOver];
                }
            }
            else if (values[WidgetVisualStateDisabled])
            {
                return values[WidgetVisualStateDisabled];
            }
            else if (values[WidgetVisualStateOver])
            {
                return values[WidgetVisualStateOver];
            }
            return result;
        }

        /** @brief Supplies the hover-state hook completed by P5-010 input tracking. */
        [[nodiscard]] virtual bool UseOverBackground() const noexcept;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize);
        virtual void InternalArrange();
        virtual void OnVisibleChanged();
        virtual void OnChildAdded(Widget& child);
        virtual void OnChildRemoved(Widget& child);
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getBorderBoundsProperty() const;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getBackgroundBoundsProperty() const;
        void InvalidateTransform();
        void SetIsPressedByUser(bool value);
        [[nodiscard]] bool getSuppressInvalidateMeasureProperty() const noexcept;
        void setSuppressInvalidateMeasureProperty(bool value) noexcept;

        /** @brief Constructs an empty instance of this widget's exact dynamic type. */
        [[nodiscard]] virtual std::shared_ptr<Widget> CreateCloneInstance() const;

        /** @brief Copies this type's ported state from @p source. */
        virtual void CopyFrom(const Widget& source);

    private:
        friend class Desktop;

        [[nodiscard]] const Graphics2D::Transform& getTransformProperty();
        void setIsKeyboardFocusedProperty(bool value);
        void UpdateTransform();
        void UpdateChildren();
        void FireLocationChanged();
        void FireSizeChanged();

        std::optional<std::string> styleName_;
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
        bool suppressInvalidateMeasure_ = false;
        std::uint64_t measureInvalidationVersion_ = 0;
        std::uint64_t arrangeInvalidationVersion_ = 0;
        Microsoft::Xna::Framework::Point lastMeasureSize_;
        Microsoft::Xna::Framework::Point lastMeasureAvailableSize_;
        Microsoft::Xna::Framework::Rectangle containerBounds_;
        Microsoft::Xna::Framework::Rectangle layoutBounds_;
        bool visible_ = true;
        bool enabled_ = true;
        DragDirection dragDirection_ = DragDirection::None;
        std::optional<MouseCursorType> mouseCursor_;
        std::optional<std::string> tooltip_;
        bool isModal_ = false;
        float opacity_ = 1.0F;
        std::array<std::shared_ptr<Graphics2D::IBrush>, WidgetVisualStateTotal> backgrounds_;
        std::array<std::shared_ptr<Graphics2D::IBrush>, WidgetVisualStateTotal> borders_;
        bool isPressed_ = false;
        bool clipToBounds_ = false;
        bool acceptsKeyboardFocus_ = false;
        bool isKeyboardFocused_ = false;
        Microsoft::Xna::Framework::Vector2 scale_{1.0F, 1.0F};
        Microsoft::Xna::Framework::Vector2 transformOrigin_{0.5F, 0.5F};
        float rotation_ = 0.0F;
        Widget* dragHandle_ = nullptr;
        bool transformDirty_ = true;
        std::optional<Graphics2D::Transform> transform_;
        Widget* parent_ = nullptr;
        std::any tag_;
        Desktop* desktop_ = nullptr;
        ILayout* childrenLayout_ = nullptr;
        bool childrenDirty_ = true;
        std::vector<std::shared_ptr<Widget>> children_;
        std::vector<std::shared_ptr<Widget>> childrenCopy_;
    };
}
