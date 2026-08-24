// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the explicit C++ metadata table required in place of .NET reflection.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/RegisterMyraTypes.hpp"

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <type_traits>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/ContentControl.hpp"
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Containers/Proportion.hpp"
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"
#include "Myra/Graphics2D/UI/Containers/SplitPane.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Project.hpp"
#include "Myra/Graphics2D/UI/Range/HorizontalProgressBar.hpp"
#include "Myra/Graphics2D/UI/Range/HorizontalSlider.hpp"
#include "Myra/Graphics2D/UI/Range/ProgressBar.hpp"
#include "Myra/Graphics2D/UI/Range/Slider.hpp"
#include "Myra/Graphics2D/UI/Range/VerticalProgressBar.hpp"
#include "Myra/Graphics2D/UI/Range/VerticalSlider.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/CheckButton.hpp"
#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"
#include "Myra/Graphics2D/UI/Simple/RadioButton.hpp"
#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"
#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"
#include "Myra/Graphics2D/UI/Simple/VerticalSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/ComboView.hpp"
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"
#include "Myra/Graphics2D/UI/Selectors/IMenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListView.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/TabControl.hpp"
#include "Myra/Graphics2D/UI/Selectors/TabItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::MML
{
    namespace
    {
        using Graphics2D::Thickness;
        using Graphics2D::UI::Button;
        using Graphics2D::UI::ButtonBase;
        using Graphics2D::UI::CheckButton;
        using Graphics2D::UI::CheckButtonBase;
        using Graphics2D::UI::CheckPosition;
        using Graphics2D::UI::ComboView;
        using Graphics2D::UI::Container;
        using Graphics2D::UI::ContentControl;
        using Graphics2D::UI::DragDirection;
        using Graphics2D::UI::ExportOptions;
        using Graphics2D::UI::Grid;
        using Graphics2D::UI::HorizontalAlignment;
        using Graphics2D::UI::HorizontalMenu;
        using Graphics2D::UI::HorizontalProgressBar;
        using Graphics2D::UI::HorizontalSeparator;
        using Graphics2D::UI::HorizontalSlider;
        using Graphics2D::UI::HorizontalSplitPane;
        using Graphics2D::UI::HorizontalStackPanel;
        using Graphics2D::UI::Image;
        using Graphics2D::UI::ImageResizeMode;
        using Graphics2D::UI::IMenuItem;
        using Graphics2D::UI::ListView;
        using Graphics2D::UI::Menu;
        using Graphics2D::UI::MenuItem;
        using Graphics2D::UI::MenuItemCollection;
        using Graphics2D::UI::MenuSeparator;
        using Graphics2D::UI::MouseCursorType;
        using Graphics2D::UI::Orientation;
        using Graphics2D::UI::Panel;
        using Graphics2D::UI::ProgressBar;
        using Graphics2D::UI::Project;
        using Graphics2D::UI::Proportion;
        using Graphics2D::UI::ProportionCollection;
        using Graphics2D::UI::ProportionType;
        using Graphics2D::UI::RadioButton;
        using Graphics2D::UI::ScrollViewer;
        using Graphics2D::UI::SelectionMode;
        using Graphics2D::UI::SeparatorWidget;
        using Graphics2D::UI::Slider;
        using Graphics2D::UI::SplitPane;
        using Graphics2D::UI::StackPanel;
        using Graphics2D::UI::TabControl;
        using Graphics2D::UI::TabItem;
        using Graphics2D::UI::TabSelectorPosition;
        using Graphics2D::UI::ToggleButton;
        using Graphics2D::UI::VerticalAlignment;
        using Graphics2D::UI::VerticalMenu;
        using Graphics2D::UI::VerticalProgressBar;
        using Graphics2D::UI::VerticalSeparator;
        using Graphics2D::UI::VerticalSlider;
        using Graphics2D::UI::VerticalSplitPane;
        using Graphics2D::UI::VerticalStackPanel;
        using Graphics2D::UI::Widget;
        using Microsoft::Xna::Framework::Point;
        using Microsoft::Xna::Framework::Vector2;

        template <typename T> struct IsOptional : std::false_type
        {
        };

        template <typename T> struct IsOptional<std::optional<T>> : std::true_type
        {
        };

        template <typename T> struct IsSharedPtr : std::false_type
        {
        };

        template <typename T> struct IsSharedPtr<std::shared_ptr<T>> : std::true_type
        {
        };

        template <typename Owner, typename Value, typename Getter, typename Setter>
        [[nodiscard]] PropertyDescriptor MakeScalarProperty(std::string name, Getter getter, Setter setter,
                                                            Value defaultValue, PropertyMetadata metadata = {})
        {
            PropertyDescriptor::NullCheck nullCheck;
            if constexpr (IsOptional<Value>::value || IsSharedPtr<Value>::value)
            {
                nullCheck = [](const std::any &value) { return !std::any_cast<const Value &>(value); };
            }
            return PropertyDescriptor(
                std::move(name), typeid(Value), [getter = std::move(getter)](const void *object)
                { return std::any(Value(getter(*static_cast<const Owner *>(object)))); },
                [setter = std::move(setter)](void *object, const std::any &value)
                { setter(*static_cast<Owner *>(object), std::any_cast<const Value &>(value)); },
                std::any(std::move(defaultValue)), std::move(metadata), [](const std::any &left, const std::any &right)
                { return std::any_cast<const Value &>(left) == std::any_cast<const Value &>(right); },
                std::move(nullCheck));
        }

        [[nodiscard]] std::shared_ptr<Widget> AsWidget(const std::shared_ptr<void> &value)
        {
            return std::shared_ptr<Widget>(value, static_cast<Widget *>(value.get()));
        }

        [[nodiscard]] std::shared_ptr<Proportion> AsProportion(const std::shared_ptr<void> &value)
        {
            return std::shared_ptr<Proportion>(value, static_cast<Proportion *>(value.get()));
        }

        [[nodiscard]] std::shared_ptr<IMenuItem> AsMenuItemInterface(const std::shared_ptr<void> &value)
        {
            return std::shared_ptr<IMenuItem>(value, static_cast<IMenuItem *>(value.get()));
        }

        [[nodiscard]] std::shared_ptr<TabItem> AsTabItem(const std::shared_ptr<void> &value)
        {
            return std::shared_ptr<TabItem>(value, static_cast<TabItem *>(value.get()));
        }

        [[nodiscard]] std::vector<RegisteredObjectView>
        EnumerateWidgets(const std::vector<std::shared_ptr<Widget>> &values)
        {
            std::vector<RegisteredObjectView> result;
            result.reserve(values.size());
            for (const std::shared_ptr<Widget> &value : values)
            {
                if (value)
                {
                    result.emplace_back(dynamic_cast<const void *>(value.get()), typeid(*value));
                }
                else
                {
                    result.emplace_back(nullptr, typeid(Widget));
                }
            }
            return result;
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateWidget(const std::shared_ptr<Widget> &value)
        {
            return value ? std::vector<RegisteredObjectView>{{dynamic_cast<const void *>(value.get()), typeid(*value)}}
                         : std::vector<RegisteredObjectView>();
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateTabItems(const TabControl::ItemCollection &values)
        {
            std::vector<RegisteredObjectView> result;
            result.reserve(static_cast<std::size_t>(values.getCountProperty()));
            for (const std::shared_ptr<TabItem> &value : values)
            {
                if (value)
                {
                    result.emplace_back(dynamic_cast<const void *>(value.get()), typeid(*value));
                }
                else
                {
                    result.emplace_back(nullptr, typeid(TabItem));
                }
            }
            return result;
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateMenuItems(const MenuItemCollection &values)
        {
            std::vector<RegisteredObjectView> result;
            result.reserve(static_cast<std::size_t>(values.getCountProperty()));
            for (const std::shared_ptr<IMenuItem> &value : values)
            {
                if (value)
                {
                    result.emplace_back(dynamic_cast<const void *>(value.get()), typeid(*value));
                }
                else
                {
                    result.emplace_back(nullptr, typeid(IMenuItem));
                }
            }
            return result;
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateExportOptions(const ExportOptions &value)
        {
            return {{&value, typeid(ExportOptions)}};
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateProportions(const ProportionCollection &values)
        {
            std::vector<RegisteredObjectView> result;
            for (const std::shared_ptr<Proportion> &value : values)
            {
                result.emplace_back(value.get(), typeid(Proportion));
            }
            return result;
        }

        [[nodiscard]] std::vector<RegisteredObjectView> EnumerateProportion(const std::shared_ptr<Proportion> &value)
        {
            return value ? std::vector<RegisteredObjectView>{{value.get(), typeid(Proportion)}}
                         : std::vector<RegisteredObjectView>();
        }

        PropertyDescriptor
        MakeProportionProperty(std::string name,
                               std::function<const std::shared_ptr<Proportion> &(const void *)> getter,
                               std::function<void(void *, std::shared_ptr<Proportion>)> setter)
        {
            return PropertyDescriptor(
                std::move(name), typeid(std::shared_ptr<Proportion>), {}, {}, std::nullopt, {}, {}, {},
                ComplexPropertyAdapter::SingleWritable(
                    typeid(Proportion), [setter = std::move(setter)](void *object, const std::shared_ptr<void> &value)
                    { setter(object, AsProportion(value)); },
                    [getter = std::move(getter)](const void *object) { return EnumerateProportion(getter(object)); }));
        }

        PropertyDescriptor
        MakeProportionCollectionProperty(std::string name, std::function<ProportionCollection &(void *)> mutableGetter,
                                         std::function<const ProportionCollection &(const void *)> getter)
        {
            return PropertyDescriptor(
                std::move(name), typeid(ProportionCollection), {}, {}, std::nullopt, {}, {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(Proportion), [mutableGetter](void *object, const std::shared_ptr<void> &value)
                    { mutableGetter(object).Add(AsProportion(value)); },
                    [getter](const void *object) { return EnumerateProportions(getter(object)); }));
        }

        TypeDescriptor MakeBaseObjectDescriptor()
        {
            TypeDescriptor descriptor("BaseObject", typeid(BaseObject),
                                      [] { return std::static_pointer_cast<void>(std::make_shared<BaseObject>()); });
            descriptor.EnableBaseObjectAccess<BaseObject>();
            descriptor.AddProperty(MakeScalarProperty<BaseObject, std::optional<std::string>>(
                "Id", [](const BaseObject &object) { return object.getIdProperty(); },
                [](BaseObject &object, const std::optional<std::string> &value) { object.setIdProperty(value); },
                std::nullopt));
            return descriptor;
        }

        TypeDescriptor MakeWidgetDescriptor()
        {
            TypeDescriptor descriptor(
                "Widget", typeid(Widget), [] { return std::static_pointer_cast<void>(std::make_shared<Widget>()); },
                typeid(BaseObject));
            descriptor.EnableBaseTypeAccess<Widget, BaseObject>();
            descriptor.EnableBaseObjectAccess<Widget>();

            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<std::string>>(
                "StyleName", [](const Widget &object) { return object.getStyleNameProperty(); },
                [](Widget &object, const std::optional<std::string> &value) { object.setStyleNameProperty(value); },
                std::optional<std::string>(std::string())));
            descriptor.AddProperty(MakeScalarProperty<Widget, int>(
                "Left", [](const Widget &object) { return object.getLeftProperty(); },
                [](Widget &object, const int value) { object.setLeftProperty(value); }, 0));
            descriptor.AddProperty(MakeScalarProperty<Widget, int>(
                "Top", [](const Widget &object) { return object.getTopProperty(); },
                [](Widget &object, const int value) { object.setTopProperty(value); }, 0));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "MinWidth", [](const Widget &object) { return object.getMinWidthProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setMinWidthProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "MaxWidth", [](const Widget &object) { return object.getMaxWidthProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setMaxWidthProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "Width", [](const Widget &object) { return object.getWidthProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setWidthProperty(value); }, std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "MinHeight", [](const Widget &object) { return object.getMinHeightProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setMinHeightProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "MaxHeight", [](const Widget &object) { return object.getMaxHeightProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setMaxHeightProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<int>>(
                "Height", [](const Widget &object) { return object.getHeightProperty(); },
                [](Widget &object, const std::optional<int> &value) { object.setHeightProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, Thickness>(
                "Margin", [](const Widget &object) { return object.getMarginProperty(); },
                [](Widget &object, const Thickness &value) { object.setMarginProperty(value); }, Thickness()));
            descriptor.AddProperty(MakeScalarProperty<Widget, Thickness>(
                "BorderThickness", [](const Widget &object) { return object.getBorderThicknessProperty(); },
                [](Widget &object, const Thickness &value) { object.setBorderThicknessProperty(value); }, Thickness()));
            descriptor.AddProperty(MakeScalarProperty<Widget, Thickness>(
                "Padding", [](const Widget &object) { return object.getPaddingProperty(); },
                [](Widget &object, const Thickness &value) { object.setPaddingProperty(value); }, Thickness()));
            descriptor.AddProperty(MakeScalarProperty<Widget, HorizontalAlignment>(
                "HorizontalAlignment", [](const Widget &object) { return object.getHorizontalAlignmentProperty(); },
                [](Widget &object, const HorizontalAlignment value) { object.setHorizontalAlignmentProperty(value); },
                HorizontalAlignment::Left));
            descriptor.AddProperty(MakeScalarProperty<Widget, VerticalAlignment>(
                "VerticalAlignment", [](const Widget &object) { return object.getVerticalAlignmentProperty(); },
                [](Widget &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                VerticalAlignment::Top));
            descriptor.AddProperty(MakeScalarProperty<Widget, bool>(
                "Enabled", [](const Widget &object) { return object.getEnabledProperty(); },
                [](Widget &object, const bool value) { object.setEnabledProperty(value); }, true));
            descriptor.AddProperty(MakeScalarProperty<Widget, bool>(
                "Visible", [](const Widget &object) { return object.getVisibleProperty(); },
                [](Widget &object, const bool value) { object.setVisibleProperty(value); }, true));
            descriptor.AddProperty(MakeScalarProperty<Widget, DragDirection>(
                "DragDirection", [](const Widget &object) { return object.getDragDirectionProperty(); },
                [](Widget &object, const DragDirection value) { object.setDragDirectionProperty(value); },
                DragDirection::None));
            descriptor.AddProperty(MakeScalarProperty<Widget, int>(
                "ZIndex", [](const Widget &object) { return object.getZIndexProperty(); },
                [](Widget &object, const int value) { object.setZIndexProperty(value); }, 0));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<MouseCursorType>>(
                "MouseCursor", [](const Widget &object) { return object.getMouseCursorProperty(); },
                [](Widget &object, const std::optional<MouseCursorType> &value)
                { object.setMouseCursorProperty(value); }, std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<Widget, std::optional<std::string>>(
                "Tooltip", [](const Widget &object) { return object.getTooltipProperty(); },
                [](Widget &object, const std::optional<std::string> &value) { object.setTooltipProperty(value); },
                std::nullopt));
            PropertyMetadata opacityMetadata;
            opacityMetadata.Range = Attributes::RangeAttribute(0.0F, 1.0F);
            descriptor.AddProperty(MakeScalarProperty<Widget, float>(
                "Opacity", [](const Widget &object) { return object.getOpacityProperty(); },
                [](Widget &object, const float value) { object.setOpacityProperty(value); }, 1.0F,
                std::move(opacityMetadata)));
            descriptor.AddProperty(MakeScalarProperty<Widget, Vector2>(
                "Scale", [](const Widget &object) { return object.getScaleProperty(); },
                [](Widget &object, const Vector2 &value) { object.setScaleProperty(value); }, Vector2(1.0F, 1.0F)));
            descriptor.AddProperty(MakeScalarProperty<Widget, Vector2>(
                "TransformOrigin", [](const Widget &object) { return object.getTransformOriginProperty(); },
                [](Widget &object, const Vector2 &value) { object.setTransformOriginProperty(value); },
                Vector2(0.5F, 0.5F)));
            descriptor.AddProperty(MakeScalarProperty<Widget, float>(
                "Rotation", [](const Widget &object) { return object.getRotationProperty(); },
                [](Widget &object, const float value) { object.setRotationProperty(value); }, 0.0F));
            descriptor.AddProperty(MakeScalarProperty<Widget, bool>(
                "ClipToBounds", [](const Widget &object) { return object.getClipToBoundsProperty(); },
                [](Widget &object, const bool value) { object.setClipToBoundsProperty(value); }, false));
            return descriptor;
        }

        TypeDescriptor MakeContainerDescriptor()
        {
            TypeDescriptor descriptor("Container", typeid(Container), {}, typeid(Widget));
            descriptor.EnableBaseTypeAccess<Container, Widget>();
            descriptor.EnableBaseObjectAccess<Container>();
            descriptor.AddProperty(MakeScalarProperty<Container, HorizontalAlignment>(
                "HorizontalAlignment", [](const Container &object) { return object.getHorizontalAlignmentProperty(); },
                [](Container &object, const HorizontalAlignment value)
                { object.setHorizontalAlignmentProperty(value); }, HorizontalAlignment::Stretch));
            descriptor.AddProperty(MakeScalarProperty<Container, VerticalAlignment>(
                "VerticalAlignment", [](const Container &object) { return object.getVerticalAlignmentProperty(); },
                [](Container &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                VerticalAlignment::Stretch));
            PropertyMetadata widgetsMetadata;
            widgetsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Widgets", typeid(std::vector<std::shared_ptr<Widget>>), {}, {}, std::nullopt,
                std::move(widgetsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<Container *>(object)->AddWidget(AsWidget(value)); }, [](const void *object)
                    { return EnumerateWidgets(static_cast<const Container *>(object)->getWidgetsProperty()); })));
            return descriptor;
        }

        TypeDescriptor MakeImageDescriptor()
        {
            TypeDescriptor descriptor(
                "Image", typeid(Image), [] { return std::static_pointer_cast<void>(std::make_shared<Image>()); },
                typeid(Widget));
            descriptor.EnableBaseTypeAccess<Image, Widget>();
            descriptor.EnableBaseObjectAccess<Image>();

            const auto addRenderable =
                [&descriptor](std::string name, std::string stylePropertyPath, auto getter, auto setter)
            {
                PropertyMetadata metadata;
                metadata.ExternalAsset = true;
                metadata.StylePropertyPath = std::move(stylePropertyPath);
                descriptor.AddProperty(MakeScalarProperty<Image, std::shared_ptr<Graphics2D::IImage>>(
                    std::move(name), std::move(getter), std::move(setter), nullptr, std::move(metadata)));
            };
            addRenderable(
                "Renderable", "Image", [](const Image &object) { return object.getRenderableProperty(); },
                [](Image &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setRenderableProperty(value); });
            addRenderable(
                "DisabledRenderable", "DisabledImage",
                [](const Image &object) { return object.getDisabledRenderableProperty(); },
                [](Image &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setDisabledRenderableProperty(value); });
            addRenderable(
                "OverRenderable", "OverImage", [](const Image &object) { return object.getOverRenderableProperty(); },
                [](Image &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setOverRenderableProperty(value); });
            addRenderable(
                "FocusedRenderable", "FocusedImage",
                [](const Image &object) { return object.getFocusedRenderableProperty(); },
                [](Image &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setFocusedRenderableProperty(value); });
            addRenderable(
                "PressedRenderable", "PressedImage",
                [](const Image &object) { return object.getPressedRenderableProperty(); },
                [](Image &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setPressedRenderableProperty(value); });
            descriptor.AddProperty(MakeScalarProperty<Image, ImageResizeMode>(
                "ResizeMode", [](const Image &object) { return object.getResizeModeProperty(); },
                [](Image &object, const ImageResizeMode value) { object.setResizeModeProperty(value); },
                ImageResizeMode::Stretch));
            return descriptor;
        }

        TypeDescriptor MakeSeparatorWidgetDescriptor()
        {
            TypeDescriptor descriptor("SeparatorWidget", typeid(SeparatorWidget), {}, typeid(Image));
            descriptor.EnableBaseTypeAccess<SeparatorWidget, Image>();
            descriptor.EnableBaseObjectAccess<SeparatorWidget>();
            descriptor.AddProperty(MakeScalarProperty<SeparatorWidget, int>(
                "Thickness", [](const SeparatorWidget &object) { return object.getThicknessProperty(); },
                [](SeparatorWidget &object, const int value) { object.setThicknessProperty(value); }, 0));
            PropertyMetadata orientationMetadata;
            orientationMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Orientation", typeid(Orientation), [](const void *object)
                { return std::any(static_cast<const SeparatorWidget *>(object)->getOrientationProperty()); }, {},
                std::nullopt, std::move(orientationMetadata)));
            return descriptor;
        }

        template <typename T>
        TypeDescriptor MakeConcreteSeparatorDescriptor(std::string name, const HorizontalAlignment horizontalDefault,
                                                       const VerticalAlignment verticalDefault)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(SeparatorWidget));
            descriptor.template EnableBaseTypeAccess<T, SeparatorWidget>();
            descriptor.template EnableBaseObjectAccess<T>();
            descriptor.AddProperty(MakeScalarProperty<T, HorizontalAlignment>(
                "HorizontalAlignment", [](const T &object) { return object.getHorizontalAlignmentProperty(); },
                [](T &object, const HorizontalAlignment value) { object.setHorizontalAlignmentProperty(value); },
                horizontalDefault));
            descriptor.AddProperty(MakeScalarProperty<T, VerticalAlignment>(
                "VerticalAlignment", [](const T &object) { return object.getVerticalAlignmentProperty(); },
                [](T &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                verticalDefault));
            return descriptor;
        }

        TypeDescriptor MakeProgressBarDescriptor()
        {
            TypeDescriptor descriptor("ProgressBar", typeid(ProgressBar), {}, typeid(Widget));
            descriptor.EnableBaseTypeAccess<ProgressBar, Widget>();
            descriptor.EnableBaseObjectAccess<ProgressBar>();
            PropertyMetadata orientationMetadata;
            orientationMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Orientation", typeid(Orientation), [](const void *object)
                { return std::any(static_cast<const ProgressBar *>(object)->getOrientationProperty()); }, {},
                std::nullopt, std::move(orientationMetadata)));
            descriptor.AddProperty(MakeScalarProperty<ProgressBar, float>(
                "Minimum", [](const ProgressBar &object) { return object.getMinimumProperty(); },
                [](ProgressBar &object, const float value) { object.setMinimumProperty(value); }, 0.0F));
            descriptor.AddProperty(MakeScalarProperty<ProgressBar, float>(
                "Maximum", [](const ProgressBar &object) { return object.getMaximumProperty(); },
                [](ProgressBar &object, const float value) { object.setMaximumProperty(value); }, 100.0F));
            descriptor.AddProperty(MakeScalarProperty<ProgressBar, float>(
                "Value", [](const ProgressBar &object) { return object.getValueProperty(); },
                [](ProgressBar &object, const float value) { object.setValueProperty(value); }, 0.0F));
            PropertyMetadata fillerMetadata;
            fillerMetadata.ExternalAsset = true;
            descriptor.AddProperty(MakeScalarProperty<ProgressBar, std::shared_ptr<Graphics2D::IBrush>>(
                "Filler", [](const ProgressBar &object) { return object.getFillerProperty(); },
                [](ProgressBar &object, const std::shared_ptr<Graphics2D::IBrush> &value)
                { object.setFillerProperty(value); }, nullptr, std::move(fillerMetadata)));
            return descriptor;
        }

        template <typename T>
        TypeDescriptor MakeConcreteProgressBarDescriptor(std::string name, const HorizontalAlignment horizontalDefault,
                                                         const VerticalAlignment verticalDefault)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(ProgressBar));
            descriptor.template EnableBaseTypeAccess<T, ProgressBar>();
            descriptor.template EnableBaseObjectAccess<T>();
            descriptor.AddProperty(MakeScalarProperty<T, HorizontalAlignment>(
                "HorizontalAlignment", [](const T &object) { return object.getHorizontalAlignmentProperty(); },
                [](T &object, const HorizontalAlignment value) { object.setHorizontalAlignmentProperty(value); },
                horizontalDefault));
            descriptor.AddProperty(MakeScalarProperty<T, VerticalAlignment>(
                "VerticalAlignment", [](const T &object) { return object.getVerticalAlignmentProperty(); },
                [](T &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                verticalDefault));
            return descriptor;
        }

        TypeDescriptor MakeSliderDescriptor()
        {
            TypeDescriptor descriptor("Slider", typeid(Slider), {}, typeid(Widget));
            descriptor.EnableBaseTypeAccess<Slider, Widget>();
            descriptor.EnableBaseObjectAccess<Slider>();
            PropertyMetadata orientationMetadata;
            orientationMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Orientation", typeid(Orientation), [](const void *object)
                { return std::any(static_cast<const Slider *>(object)->getOrientationProperty()); }, {}, std::nullopt,
                std::move(orientationMetadata)));
            descriptor.AddProperty(MakeScalarProperty<Slider, float>(
                "Minimum", [](const Slider &object) { return object.getMinimumProperty(); },
                [](Slider &object, const float value) { object.setMinimumProperty(value); }, 0.0F));
            descriptor.AddProperty(MakeScalarProperty<Slider, float>(
                "Maximum", [](const Slider &object) { return object.getMaximumProperty(); },
                [](Slider &object, const float value) { object.setMaximumProperty(value); }, 100.0F));
            descriptor.AddProperty(MakeScalarProperty<Slider, float>(
                "Value", [](const Slider &object) { return object.getValueProperty(); },
                [](Slider &object, const float value) { object.setValueProperty(value); }, 0.0F));
            descriptor.AddProperty(MakeScalarProperty<Slider, bool>(
                "WheelAdjustment", [](const Slider &object) { return object.getWheelAdjustmentProperty(); },
                [](Slider &object, const bool value) { object.setWheelAdjustmentProperty(value); }, false));
            descriptor.AddProperty(MakeScalarProperty<Slider, float>(
                "WheelStep", [](const Slider &object) { return object.getWheelStepProperty(); },
                [](Slider &object, const float value) { object.setWheelStepProperty(value); }, 1.0F));
            PropertyMetadata imageButtonMetadata;
            imageButtonMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "ImageButton", typeid(std::shared_ptr<Button>), [](const void *object)
                { return std::any(static_cast<const Slider *>(object)->getImageButtonProperty()); }, {}, std::nullopt,
                std::move(imageButtonMetadata)));
            return descriptor;
        }

        template <typename T>
        TypeDescriptor MakeConcreteSliderDescriptor(std::string name, const HorizontalAlignment horizontalDefault,
                                                    const VerticalAlignment verticalDefault)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(Slider));
            descriptor.template EnableBaseTypeAccess<T, Slider>();
            descriptor.template EnableBaseObjectAccess<T>();
            descriptor.AddProperty(MakeScalarProperty<T, HorizontalAlignment>(
                "HorizontalAlignment", [](const T &object) { return object.getHorizontalAlignmentProperty(); },
                [](T &object, const HorizontalAlignment value) { object.setHorizontalAlignmentProperty(value); },
                horizontalDefault));
            descriptor.AddProperty(MakeScalarProperty<T, VerticalAlignment>(
                "VerticalAlignment", [](const T &object) { return object.getVerticalAlignmentProperty(); },
                [](T &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                verticalDefault));
            return descriptor;
        }

        TypeDescriptor MakeContentControlDescriptor()
        {
            TypeDescriptor descriptor("ContentControl", typeid(ContentControl), {}, typeid(Widget));
            descriptor.EnableBaseTypeAccess<ContentControl, Widget>();
            descriptor.EnableBaseObjectAccess<ContentControl>();
            PropertyMetadata contentMetadata;
            contentMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Content", typeid(std::shared_ptr<Widget>), {}, {}, std::nullopt, std::move(contentMetadata), {}, {},
                ComplexPropertyAdapter::SingleWritable(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<ContentControl *>(object)->setContentProperty(AsWidget(value)); },
                    [](const void *object)
                    { return EnumerateWidget(static_cast<const ContentControl *>(object)->getContentProperty()); })));
            return descriptor;
        }

        TypeDescriptor MakeScrollViewerDescriptor()
        {
            TypeDescriptor descriptor(
                "ScrollViewer", typeid(ScrollViewer), []
                { return std::static_pointer_cast<void>(std::make_shared<ScrollViewer>()); }, typeid(ContentControl));
            descriptor.EnableBaseTypeAccess<ScrollViewer, ContentControl>();
            descriptor.EnableBaseObjectAccess<ScrollViewer>();

            PropertyMetadata scrollMaximumMetadata;
            scrollMaximumMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "ScrollMaximum", typeid(Point), [](const void *object)
                { return std::any(static_cast<const ScrollViewer *>(object)->getScrollMaximumProperty()); }, {},
                std::nullopt, std::move(scrollMaximumMetadata)));
            PropertyMetadata scrollPositionMetadata;
            scrollPositionMetadata.XmlIgnore = true;
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, Point>(
                "ScrollPosition", [](const ScrollViewer &object) { return object.getScrollPositionProperty(); },
                [](ScrollViewer &object, const Point value) { object.setScrollPositionProperty(value); }, Point(),
                std::move(scrollPositionMetadata)));

            const auto addImage = [&descriptor](std::string name, auto getter, auto setter)
            {
                PropertyMetadata metadata;
                metadata.ExternalAsset = true;
                descriptor.AddProperty(MakeScalarProperty<ScrollViewer, std::shared_ptr<Graphics2D::IImage>>(
                    std::move(name), std::move(getter), std::move(setter), nullptr, std::move(metadata)));
            };
            addImage(
                "HorizontalScrollBackground",
                [](const ScrollViewer &object) { return object.getHorizontalScrollBackgroundProperty(); },
                [](ScrollViewer &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setHorizontalScrollBackgroundProperty(value); });
            addImage(
                "HorizontalScrollKnob",
                [](const ScrollViewer &object) { return object.getHorizontalScrollKnobProperty(); },
                [](ScrollViewer &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setHorizontalScrollKnobProperty(value); });
            addImage(
                "VerticalScrollBackground",
                [](const ScrollViewer &object) { return object.getVerticalScrollBackgroundProperty(); },
                [](ScrollViewer &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setVerticalScrollBackgroundProperty(value); });
            addImage(
                "VerticalScrollKnob", [](const ScrollViewer &object) { return object.getVerticalScrollKnobProperty(); },
                [](ScrollViewer &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setVerticalScrollKnobProperty(value); });

            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, int>(
                "ScrollMultiplier", [](const ScrollViewer &object) { return object.getScrollMultiplierProperty(); },
                [](ScrollViewer &object, const int value) { object.setScrollMultiplierProperty(value); }, 10));
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, bool>(
                "ShowHorizontalScrollBar", [](const ScrollViewer &object)
                { return object.getShowHorizontalScrollBarProperty(); }, [](ScrollViewer &object, const bool value)
                { object.setShowHorizontalScrollBarProperty(value); }, true));
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, bool>(
                "ShowVerticalScrollBar",
                [](const ScrollViewer &object) { return object.getShowVerticalScrollBarProperty(); },
                [](ScrollViewer &object, const bool value) { object.setShowVerticalScrollBarProperty(value); }, true));
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, HorizontalAlignment>(
                "HorizontalAlignment",
                [](const ScrollViewer &object) { return object.getHorizontalAlignmentProperty(); },
                [](ScrollViewer &object, const HorizontalAlignment value)
                { object.setHorizontalAlignmentProperty(value); }, HorizontalAlignment::Stretch));
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, VerticalAlignment>(
                "VerticalAlignment", [](const ScrollViewer &object) { return object.getVerticalAlignmentProperty(); },
                [](ScrollViewer &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                VerticalAlignment::Stretch));
            descriptor.AddProperty(MakeScalarProperty<ScrollViewer, bool>(
                "ClipToBounds", [](const ScrollViewer &object) { return object.getClipToBoundsProperty(); },
                [](ScrollViewer &object, const bool value) { object.setClipToBoundsProperty(value); }, true));
            return descriptor;
        }

        TypeDescriptor MakeListViewDescriptor()
        {
            TypeDescriptor descriptor(
                "ListView", typeid(ListView),
                [] { return std::static_pointer_cast<void>(std::make_shared<ListView>()); }, typeid(Widget));
            descriptor.EnableBaseTypeAccess<ListView, Widget>();
            descriptor.EnableBaseObjectAccess<ListView>();

            descriptor.AddProperty(MakeScalarProperty<ListView, SelectionMode>(
                "SelectionMode", [](const ListView &object) { return object.getSelectionModeProperty(); },
                [](ListView &object, const SelectionMode value) { object.setSelectionModeProperty(value); },
                SelectionMode::Single));

            PropertyMetadata widgetsMetadata;
            widgetsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Widgets", typeid(std::vector<std::shared_ptr<Widget>>), {}, {}, std::nullopt,
                std::move(widgetsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<ListView *>(object)->AddWidget(AsWidget(value)); }, [](const void *object)
                    { return EnumerateWidgets(static_cast<const ListView *>(object)->getWidgetsProperty()); })));

            PropertyMetadata scrollViewerMetadata;
            scrollViewerMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "ScrollViewer", typeid(std::shared_ptr<ScrollViewer>), [](const void *object)
                { return std::any(static_cast<const ListView *>(object)->getScrollViewerProperty()); }, {},
                std::nullopt, std::move(scrollViewerMetadata)));

            PropertyMetadata selectedIndexMetadata;
            selectedIndexMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "SelectedIndex", typeid(std::optional<int>), [](const void *object)
                { return std::any(static_cast<const ListView *>(object)->getSelectedIndexProperty()); }, {},
                std::nullopt, std::move(selectedIndexMetadata)));

            PropertyMetadata selectedItemMetadata;
            selectedItemMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "SelectedItem", typeid(std::shared_ptr<Widget>), [](const void *object)
                { return std::any(static_cast<const ListView *>(object)->getSelectedItemProperty()); }, {},
                std::nullopt, std::move(selectedItemMetadata)));
            return descriptor;
        }

        TypeDescriptor MakeComboViewDescriptor()
        {
            TypeDescriptor descriptor(
                "ComboView", typeid(ComboView),
                [] { return std::static_pointer_cast<void>(std::make_shared<ComboView>()); }, typeid(Widget));
            descriptor.EnableBaseTypeAccess<ComboView, Widget>();
            descriptor.EnableBaseObjectAccess<ComboView>();

            descriptor.AddProperty(MakeScalarProperty<ComboView, std::optional<int>>(
                "DropdownMaximumHeight",
                [](const ComboView &object) { return object.getDropdownMaximumHeightProperty(); },
                [](ComboView &object, const std::optional<int> &value)
                { object.setDropdownMaximumHeightProperty(value); }, std::optional<int>(300)));
            descriptor.AddProperty(MakeScalarProperty<ComboView, SelectionMode>(
                "SelectionMode", [](const ComboView &object) { return object.getSelectionModeProperty(); },
                [](ComboView &object, const SelectionMode value) { object.setSelectionModeProperty(value); },
                SelectionMode::Single));

            PropertyMetadata widgetsMetadata;
            widgetsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Widgets", typeid(std::vector<std::shared_ptr<Widget>>), {}, {}, std::nullopt,
                std::move(widgetsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<ComboView *>(object)->AddWidget(AsWidget(value)); }, [](const void *object)
                    { return EnumerateWidgets(static_cast<const ComboView *>(object)->getWidgetsProperty()); })));

            PropertyMetadata expandedMetadata;
            expandedMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "IsExpanded", typeid(bool), [](const void *object)
                { return std::any(static_cast<const ComboView *>(object)->getIsExpandedProperty()); }, {}, std::nullopt,
                std::move(expandedMetadata)));

            PropertyMetadata listViewMetadata;
            listViewMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "ListView", typeid(std::shared_ptr<ListView>), [](const void *object)
                { return std::any(static_cast<const ComboView *>(object)->getListViewProperty()); }, {}, std::nullopt,
                std::move(listViewMetadata)));

            PropertyMetadata selectedIndexMetadata;
            selectedIndexMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "SelectedIndex", typeid(std::optional<int>), [](const void *object)
                { return std::any(static_cast<const ComboView *>(object)->getSelectedIndexProperty()); }, {},
                std::nullopt, std::move(selectedIndexMetadata)));

            PropertyMetadata selectedItemMetadata;
            selectedItemMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "SelectedItem", typeid(std::shared_ptr<Widget>), [](const void *object)
                { return std::any(static_cast<const ComboView *>(object)->getSelectedItemProperty()); }, {},
                std::nullopt, std::move(selectedItemMetadata)));
            return descriptor;
        }

        TypeDescriptor MakeTabItemDescriptor()
        {
            TypeDescriptor descriptor(
                "TabItem", typeid(TabItem), [] { return std::static_pointer_cast<void>(std::make_shared<TabItem>()); },
                typeid(BaseObject));
            descriptor.EnableBaseTypeAccess<TabItem, BaseObject>();
            descriptor.EnableBaseObjectAccess<TabItem>();

            descriptor.AddProperty(MakeScalarProperty<TabItem, std::optional<std::string>>(
                "Text", [](const TabItem &object) { return object.getTextProperty(); },
                [](TabItem &object, const std::optional<std::string> &value) { object.setTextProperty(value); },
                std::nullopt));
            PropertyMetadata contentMetadata;
            contentMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Content", typeid(std::shared_ptr<Widget>), {}, {}, std::nullopt, std::move(contentMetadata), {}, {},
                ComplexPropertyAdapter::SingleWritable(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<TabItem *>(object)->setContentProperty(AsWidget(value)); }, [](const void *object)
                    { return EnumerateWidget(static_cast<const TabItem *>(object)->getContentProperty()); })));
            descriptor.AddProperty(MakeScalarProperty<TabItem, std::optional<int>>(
                "Height", [](const TabItem &object) { return object.getHeightProperty(); },
                [](TabItem &object, const std::optional<int> &value) { object.setHeightProperty(value); },
                std::nullopt));

            const auto addIgnored = [&descriptor](std::string name, std::type_index type, auto getter)
            {
                PropertyMetadata metadata;
                metadata.XmlIgnore = true;
                descriptor.AddProperty(PropertyDescriptor(std::move(name), type, std::move(getter), {}, std::nullopt,
                                                          std::move(metadata)));
            };
            addIgnored("Tag", typeid(std::any),
                       [](const void *object)
                       {
                           return std::any(std::in_place_type<std::any>,
                                           static_cast<const TabItem *>(object)->getTagProperty());
                       });
            addIgnored("Image", typeid(std::shared_ptr<Graphics2D::IImage>), [](const void *object)
                       { return std::any(static_cast<const TabItem *>(object)->getImageProperty()); });
            addIgnored("ImageTextSpacing", typeid(int), [](const void *object)
                       { return std::any(static_cast<const TabItem *>(object)->getImageTextSpacingProperty()); });
            addIgnored("IsSelected", typeid(bool), [](const void *object)
                       { return std::any(static_cast<const TabItem *>(object)->getIsSelectedProperty()); });
            return descriptor;
        }

        TypeDescriptor MakeTabControlDescriptor()
        {
            TypeDescriptor descriptor(
                "TabControl", typeid(TabControl),
                [] { return std::static_pointer_cast<void>(std::make_shared<TabControl>()); }, typeid(Widget));
            descriptor.EnableBaseTypeAccess<TabControl, Widget>();
            descriptor.EnableBaseObjectAccess<TabControl>();

            PropertyMetadata itemsMetadata;
            itemsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Items", typeid(TabControl::ItemCollection), {}, {}, std::nullopt, std::move(itemsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(TabItem), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<TabControl *>(object)->getItemsProperty().Add(AsTabItem(value)); },
                    [](const void *object)
                    { return EnumerateTabItems(static_cast<const TabControl *>(object)->getItemsProperty()); })));

            const auto addIgnored = [&descriptor](std::string name, std::type_index type, auto getter)
            {
                PropertyMetadata metadata;
                metadata.XmlIgnore = true;
                descriptor.AddProperty(PropertyDescriptor(std::move(name), type, std::move(getter), {}, std::nullopt,
                                                          std::move(metadata)));
            };
            addIgnored("SelectionMode", typeid(SelectionMode), [](const void *object)
                       { return std::any(static_cast<const TabControl *>(object)->getSelectionModeProperty()); });
            addIgnored("SelectedIndex", typeid(std::optional<int>), [](const void *object)
                       { return std::any(static_cast<const TabControl *>(object)->getSelectedIndexProperty()); });
            addIgnored("SelectedItem", typeid(std::shared_ptr<TabItem>), [](const void *object)
                       { return std::any(static_cast<const TabControl *>(object)->getSelectedItemProperty()); });

            descriptor.AddProperty(MakeScalarProperty<TabControl, HorizontalAlignment>(
                "HorizontalAlignment", [](const TabControl &object) { return object.getHorizontalAlignmentProperty(); },
                [](TabControl &object, const HorizontalAlignment value)
                { object.setHorizontalAlignmentProperty(value); }, HorizontalAlignment::Left));
            descriptor.AddProperty(MakeScalarProperty<TabControl, VerticalAlignment>(
                "VerticalAlignment", [](const TabControl &object) { return object.getVerticalAlignmentProperty(); },
                [](TabControl &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                VerticalAlignment::Top));
            descriptor.AddProperty(MakeScalarProperty<TabControl, TabSelectorPosition>(
                "TabSelectorPosition", [](const TabControl &object) { return object.getTabSelectorPositionProperty(); },
                [](TabControl &object, const TabSelectorPosition value)
                { object.setTabSelectorPositionProperty(value); }, TabSelectorPosition::Top));
            descriptor.AddProperty(MakeScalarProperty<TabControl, bool>(
                "CloseableTabs", [](const TabControl &object) { return object.getCloseableTabsProperty(); },
                [](TabControl &object, const bool value) { object.setCloseableTabsProperty(value); }, false));
            descriptor.AddProperty(MakeScalarProperty<TabControl, bool>(
                "ClipToBounds", [](const TabControl &object) { return object.getClipToBoundsProperty(); },
                [](TabControl &object, const bool value) { object.setClipToBoundsProperty(value); }, true));
            return descriptor;
        }

        TypeDescriptor MakeIMenuItemDescriptor()
        {
            TypeDescriptor descriptor("IMenuItem", typeid(IMenuItem));
            descriptor.AddProperty(MakeScalarProperty<IMenuItem, std::optional<std::string>>(
                "Id", [](const IMenuItem &object) { return object.getIdProperty(); },
                [](IMenuItem &object, const std::optional<std::string> &value) { object.setIdProperty(value); },
                std::nullopt));

            const auto addIgnored = [&descriptor](std::string name, std::type_index type, auto getter)
            {
                PropertyMetadata metadata;
                metadata.XmlIgnore = true;
                descriptor.AddProperty(PropertyDescriptor(std::move(name), type, std::move(getter), {}, std::nullopt,
                                                          std::move(metadata)));
            };
            addIgnored("Menu", typeid(Menu *), [](const void *object)
                       { return std::any(static_cast<const IMenuItem *>(object)->getMenuProperty()); });
            addIgnored("UnderscoreChar", typeid(std::optional<char>), [](const void *object)
                       { return std::any(static_cast<const IMenuItem *>(object)->getUnderscoreCharProperty()); });
            addIgnored("Index", typeid(int), [](const void *object)
                       { return std::any(static_cast<const IMenuItem *>(object)->getIndexProperty()); });
            return descriptor;
        }

        TypeDescriptor MakeMenuItemDescriptor()
        {
            TypeDescriptor descriptor(
                "MenuItem", typeid(MenuItem),
                [] { return std::static_pointer_cast<void>(std::make_shared<MenuItem>()); }, typeid(IMenuItem));
            descriptor.EnableBaseTypeAccess<MenuItem, IMenuItem>();
            descriptor.EnableBaseObjectAccess<MenuItem>();

            descriptor.AddProperty(MakeScalarProperty<MenuItem, std::optional<std::string>>(
                "Text", [](const MenuItem &object) { return object.getTextProperty(); },
                [](MenuItem &object, const std::optional<std::string> &value) { object.setTextProperty(value); },
                std::nullopt));
            PropertyMetadata imageMetadata;
            imageMetadata.ExternalAsset = true;
            descriptor.AddProperty(MakeScalarProperty<MenuItem, std::shared_ptr<Graphics2D::IImage>>(
                "Image", [](const MenuItem &object) { return object.getImageProperty(); },
                [](MenuItem &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setImageProperty(value); }, nullptr, std::move(imageMetadata)));
            descriptor.AddProperty(MakeScalarProperty<MenuItem, std::optional<std::string>>(
                "ShortcutText", [](const MenuItem &object) { return object.getShortcutTextProperty(); },
                [](MenuItem &object, const std::optional<std::string> &value)
                { object.setShortcutTextProperty(value); }, std::nullopt));

            PropertyMetadata itemsMetadata;
            itemsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Items", typeid(MenuItemCollection), {}, {}, std::nullopt, std::move(itemsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(IMenuItem), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<MenuItem *>(object)->getItemsProperty().Add(AsMenuItemInterface(value)); },
                    [](const void *object)
                    { return EnumerateMenuItems(static_cast<const MenuItem *>(object)->getItemsProperty()); })));

            const auto addIgnored = [&descriptor](std::string name, std::type_index type, auto getter)
            {
                PropertyMetadata metadata;
                metadata.XmlIgnore = true;
                descriptor.AddProperty(PropertyDescriptor(std::move(name), type, std::move(getter), {}, std::nullopt,
                                                          std::move(metadata)));
            };
            addIgnored("Tag", typeid(std::any),
                       [](const void *object)
                       {
                           return std::any(std::in_place_type<std::any>,
                                           static_cast<const MenuItem *>(object)->getTagProperty());
                       });
            addIgnored("Enabled", typeid(bool), [](const void *object)
                       { return std::any(static_cast<const MenuItem *>(object)->getEnabledProperty()); });
            addIgnored("CanOpen", typeid(bool), [](const void *object)
                       { return std::any(static_cast<const MenuItem *>(object)->getCanOpenProperty()); });
            return descriptor;
        }

        TypeDescriptor MakeMenuSeparatorDescriptor()
        {
            TypeDescriptor descriptor(
                "MenuSeparator", typeid(MenuSeparator),
                [] { return std::static_pointer_cast<void>(std::make_shared<MenuSeparator>()); }, typeid(IMenuItem));
            descriptor.EnableBaseTypeAccess<MenuSeparator, IMenuItem>();

            PropertyMetadata idMetadata;
            idMetadata.XmlIgnore = true;
            descriptor.AddProperty(MakeScalarProperty<MenuSeparator, std::optional<std::string>>(
                "Id", [](const MenuSeparator &object) { return object.getIdProperty(); },
                [](MenuSeparator &object, const std::optional<std::string> &value) { object.setIdProperty(value); },
                std::nullopt, std::move(idMetadata)));
            return descriptor;
        }

        TypeDescriptor MakeMenuDescriptor()
        {
            TypeDescriptor descriptor("Menu", typeid(Menu), {}, typeid(Widget));
            descriptor.EnableBaseTypeAccess<Menu, Widget>();
            descriptor.EnableBaseObjectAccess<Menu>();

            PropertyMetadata itemsMetadata;
            itemsMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Items", typeid(MenuItemCollection), {}, {}, std::nullopt, std::move(itemsMetadata), {}, {},
                ComplexPropertyAdapter::Sequence(
                    typeid(IMenuItem), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<Menu *>(object)->getItemsProperty().Add(AsMenuItemInterface(value)); },
                    [](const void *object)
                    { return EnumerateMenuItems(static_cast<const Menu *>(object)->getItemsProperty()); })));
            descriptor.AddProperty(MakeScalarProperty<Menu, bool>(
                "HoverIndexCanBeNull", [](const Menu &object) { return object.getHoverIndexCanBeNullProperty(); },
                [](Menu &object, const bool value) { object.setHoverIndexCanBeNullProperty(value); }, true));

            const auto addIgnored = [&descriptor](std::string name, std::type_index type, auto getter)
            {
                PropertyMetadata metadata;
                metadata.XmlIgnore = true;
                descriptor.AddProperty(PropertyDescriptor(std::move(name), type, std::move(getter), {}, std::nullopt,
                                                          std::move(metadata)));
            };
            addIgnored("Orientation", typeid(Orientation), [](const void *object)
                       { return std::any(static_cast<const Menu *>(object)->getOrientationProperty()); });
            addIgnored("IsOpen", typeid(bool), [](const void *object)
                       { return std::any(static_cast<const Menu *>(object)->getIsOpenProperty()); });
            addIgnored("HoverIndex", typeid(std::optional<int>), [](const void *object)
                       { return std::any(static_cast<const Menu *>(object)->getHoverIndexProperty()); });
            addIgnored("SelectedIndex", typeid(std::optional<int>), [](const void *object)
                       { return std::any(static_cast<const Menu *>(object)->getSelectedIndexProperty()); });
            return descriptor;
        }

        template <typename T>
        TypeDescriptor MakeConcreteMenuDescriptor(std::string name, const HorizontalAlignment horizontalDefault,
                                                  const VerticalAlignment verticalDefault)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(Menu));
            descriptor.template EnableBaseTypeAccess<T, Menu>();
            descriptor.template EnableBaseObjectAccess<T>();
            descriptor.AddProperty(MakeScalarProperty<T, HorizontalAlignment>(
                "HorizontalAlignment", [](const T &object) { return object.getHorizontalAlignmentProperty(); },
                [](T &object, const HorizontalAlignment value) { object.setHorizontalAlignmentProperty(value); },
                horizontalDefault));
            descriptor.AddProperty(MakeScalarProperty<T, VerticalAlignment>(
                "VerticalAlignment", [](const T &object) { return object.getVerticalAlignmentProperty(); },
                [](T &object, const VerticalAlignment value) { object.setVerticalAlignmentProperty(value); },
                verticalDefault));
            return descriptor;
        }

        TypeDescriptor MakeButtonBaseDescriptor()
        {
            TypeDescriptor descriptor("ButtonBase", typeid(ButtonBase), {}, typeid(ContentControl));
            descriptor.EnableBaseTypeAccess<ButtonBase, ContentControl>();
            descriptor.EnableBaseObjectAccess<ButtonBase>();
            descriptor.AddProperty(MakeScalarProperty<ButtonBase, bool>(
                "ReadOnly", [](const ButtonBase &object) { return object.getReadOnlyProperty(); },
                [](ButtonBase &object, const bool value) { object.setReadOnlyProperty(value); }, false));
            return descriptor;
        }

        TypeDescriptor MakeButtonDescriptor()
        {
            TypeDescriptor descriptor(
                "Button", typeid(Button), [] { return std::static_pointer_cast<void>(std::make_shared<Button>()); },
                typeid(ButtonBase));
            descriptor.EnableBaseTypeAccess<Button, ButtonBase>();
            descriptor.EnableBaseObjectAccess<Button>();
            return descriptor;
        }

        TypeDescriptor MakeCheckButtonBaseDescriptor()
        {
            TypeDescriptor descriptor("CheckButtonBase", typeid(CheckButtonBase), {}, typeid(ButtonBase));
            descriptor.EnableBaseTypeAccess<CheckButtonBase, ButtonBase>();
            descriptor.EnableBaseObjectAccess<CheckButtonBase>();
            descriptor.AddProperty(MakeScalarProperty<CheckButtonBase, CheckPosition>(
                "CheckPosition", [](const CheckButtonBase &object) { return object.getCheckPositionProperty(); },
                [](CheckButtonBase &object, const CheckPosition value) { object.setCheckPositionProperty(value); },
                CheckPosition::Left));

            PropertyMetadata spacingMetadata;
            spacingMetadata.StylePropertyPath = "ImageTextSpacing";
            descriptor.AddProperty(MakeScalarProperty<CheckButtonBase, int>(
                "CheckContentSpacing", [](const CheckButtonBase &object)
                { return object.getCheckContentSpacingProperty(); }, [](CheckButtonBase &object, const int value)
                { object.setCheckContentSpacingProperty(value); }, 0, std::move(spacingMetadata)));

            const auto addImage =
                [&descriptor](std::string name, std::string stylePropertyPath, auto getter, auto setter)
            {
                PropertyMetadata metadata;
                metadata.ExternalAsset = true;
                metadata.StylePropertyPath = std::move(stylePropertyPath);
                descriptor.AddProperty(MakeScalarProperty<CheckButtonBase, std::shared_ptr<Graphics2D::IImage>>(
                    std::move(name), std::move(getter), std::move(setter), nullptr, std::move(metadata)));
            };
            addImage(
                "UncheckedImage", "ImageStyle/Image",
                [](const CheckButtonBase &object) { return object.getUncheckedImageProperty(); },
                [](CheckButtonBase &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setUncheckedImageProperty(value); });
            addImage(
                "CheckedImage", "ImageStyle/PressedImage",
                [](const CheckButtonBase &object) { return object.getCheckedImageProperty(); },
                [](CheckButtonBase &object, const std::shared_ptr<Graphics2D::IImage> &value)
                { object.setCheckedImageProperty(value); });

            PropertyMetadata checkImageMetadata;
            checkImageMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "CheckImage", typeid(std::shared_ptr<Image>), [](const void *object)
                { return std::any(static_cast<const CheckButtonBase *>(object)->getCheckImageProperty()); }, {},
                std::nullopt, std::move(checkImageMetadata)));
            return descriptor;
        }

        TypeDescriptor MakeCheckButtonDescriptor()
        {
            TypeDescriptor descriptor(
                "CheckButton", typeid(CheckButton), []
                { return std::static_pointer_cast<void>(std::make_shared<CheckButton>()); }, typeid(CheckButtonBase));
            descriptor.EnableBaseTypeAccess<CheckButton, CheckButtonBase>();
            descriptor.EnableBaseObjectAccess<CheckButton>();
            descriptor.AddProperty(MakeScalarProperty<CheckButton, bool>(
                "IsChecked", [](const CheckButton &object) { return object.getIsCheckedProperty(); },
                [](CheckButton &object, const bool value) { object.setIsCheckedProperty(value); }, false));
            return descriptor;
        }

        TypeDescriptor MakeRadioButtonDescriptor()
        {
            TypeDescriptor descriptor(
                "RadioButton", typeid(RadioButton), []
                { return std::static_pointer_cast<void>(std::make_shared<RadioButton>()); }, typeid(CheckButtonBase));
            descriptor.EnableBaseTypeAccess<RadioButton, CheckButtonBase>();
            descriptor.EnableBaseObjectAccess<RadioButton>();
            return descriptor;
        }

        TypeDescriptor MakeToggleButtonDescriptor()
        {
            TypeDescriptor descriptor(
                "ToggleButton", typeid(ToggleButton),
                [] { return std::static_pointer_cast<void>(std::make_shared<ToggleButton>()); }, typeid(ButtonBase));
            descriptor.EnableBaseTypeAccess<ToggleButton, ButtonBase>();
            descriptor.EnableBaseObjectAccess<ToggleButton>();
            descriptor.AddProperty(MakeScalarProperty<ToggleButton, bool>(
                "IsToggled", [](const ToggleButton &object) { return object.getIsToggledProperty(); },
                [](ToggleButton &object, const bool value) { object.setIsToggledProperty(value); }, false));
            return descriptor;
        }

        TypeDescriptor MakeProportionDescriptor()
        {
            TypeDescriptor descriptor("Proportion", typeid(Proportion),
                                      [] { return std::static_pointer_cast<void>(std::make_shared<Proportion>()); });
            descriptor.AddProperty(MakeScalarProperty<Proportion, ProportionType>(
                "Type", [](const Proportion &object) { return object.getTypeProperty(); },
                [](Proportion &object, const ProportionType value) { object.setTypeProperty(value); },
                ProportionType::Auto));
            descriptor.AddProperty(MakeScalarProperty<Proportion, float>(
                "Value", [](const Proportion &object) { return object.getValueProperty(); },
                [](Proportion &object, const float value) { object.setValueProperty(value); }, 1.0F));
            return descriptor;
        }

        TypeDescriptor MakePanelDescriptor()
        {
            TypeDescriptor descriptor(
                "Panel", typeid(Panel), [] { return std::static_pointer_cast<void>(std::make_shared<Panel>()); },
                typeid(Container));
            descriptor.EnableBaseTypeAccess<Panel, Container>();
            descriptor.EnableBaseObjectAccess<Panel>();
            return descriptor;
        }

        TypeDescriptor MakeGridDescriptor()
        {
            TypeDescriptor descriptor(
                "Grid", typeid(Grid), [] { return std::static_pointer_cast<void>(std::make_shared<Grid>()); },
                typeid(Container));
            descriptor.EnableBaseTypeAccess<Grid, Container>();
            descriptor.EnableBaseObjectAccess<Grid>();
            descriptor.AddProperty(MakeScalarProperty<Grid, int>(
                "ColumnSpacing", [](const Grid &object) { return object.getColumnSpacingProperty(); },
                [](Grid &object, const int value) { object.setColumnSpacingProperty(value); }, 0));
            descriptor.AddProperty(MakeScalarProperty<Grid, int>(
                "RowSpacing", [](const Grid &object) { return object.getRowSpacingProperty(); },
                [](Grid &object, const int value) { object.setRowSpacingProperty(value); }, 0));
            descriptor.AddProperty(MakeProportionProperty(
                "DefaultColumnProportion", [](const void *object) -> const std::shared_ptr<Proportion> &
                { return static_cast<const Grid *>(object)->getDefaultColumnProportionProperty(); },
                [](void *object, std::shared_ptr<Proportion> value)
                { static_cast<Grid *>(object)->setDefaultColumnProportionProperty(std::move(value)); }));
            descriptor.AddProperty(MakeProportionProperty(
                "DefaultRowProportion", [](const void *object) -> const std::shared_ptr<Proportion> &
                { return static_cast<const Grid *>(object)->getDefaultRowProportionProperty(); },
                [](void *object, std::shared_ptr<Proportion> value)
                { static_cast<Grid *>(object)->setDefaultRowProportionProperty(std::move(value)); }));
            descriptor.AddProperty(MakeProportionCollectionProperty(
                "ColumnsProportions", [](void *object) -> ProportionCollection &
                { return static_cast<Grid *>(object)->getColumnsProportionsProperty(); },
                [](const void *object) -> const ProportionCollection &
                { return static_cast<const Grid *>(object)->getColumnsProportionsProperty(); }));
            descriptor.AddProperty(MakeProportionCollectionProperty(
                "RowsProportions", [](void *object) -> ProportionCollection &
                { return static_cast<Grid *>(object)->getRowsProportionsProperty(); },
                [](const void *object) -> const ProportionCollection &
                { return static_cast<const Grid *>(object)->getRowsProportionsProperty(); }));
            return descriptor;
        }

        TypeDescriptor MakeSplitPaneDescriptor()
        {
            TypeDescriptor descriptor("SplitPane", typeid(SplitPane), {}, typeid(Container));
            descriptor.EnableBaseTypeAccess<SplitPane, Container>();
            descriptor.EnableBaseObjectAccess<SplitPane>();
            PropertyMetadata orientationMetadata;
            orientationMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Orientation", typeid(Orientation), [](const void *object)
                { return std::any(static_cast<const SplitPane *>(object)->getOrientationProperty()); }, {},
                std::nullopt, std::move(orientationMetadata)));
            return descriptor;
        }

        template <typename T> TypeDescriptor MakeConcreteSplitPaneDescriptor(std::string name)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(SplitPane));
            descriptor.template EnableBaseTypeAccess<T, SplitPane>();
            descriptor.template EnableBaseObjectAccess<T>();
            return descriptor;
        }

        TypeDescriptor MakeStackPanelDescriptor()
        {
            TypeDescriptor descriptor("StackPanel", typeid(StackPanel), {}, typeid(Container));
            descriptor.EnableBaseTypeAccess<StackPanel, Container>();
            descriptor.EnableBaseObjectAccess<StackPanel>();
            PropertyMetadata orientationMetadata;
            orientationMetadata.XmlIgnore = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Orientation", typeid(Orientation), [](const void *object)
                { return std::any(static_cast<const StackPanel *>(object)->getOrientationProperty()); }, {},
                std::nullopt, std::move(orientationMetadata)));
            descriptor.AddProperty(MakeScalarProperty<StackPanel, int>(
                "Spacing", [](const StackPanel &object) { return object.getSpacingProperty(); },
                [](StackPanel &object, const int value) { object.setSpacingProperty(value); }, 0));
            descriptor.AddProperty(MakeProportionProperty(
                "DefaultProportion", [](const void *object) -> const std::shared_ptr<Proportion> &
                { return static_cast<const StackPanel *>(object)->getDefaultProportionProperty(); },
                [](void *object, std::shared_ptr<Proportion> value)
                { static_cast<StackPanel *>(object)->setDefaultProportionProperty(std::move(value)); }));
            return descriptor;
        }

        template <typename T> TypeDescriptor MakeConcreteStackPanelDescriptor(std::string name)
        {
            TypeDescriptor descriptor(
                std::move(name), typeid(T), [] { return std::static_pointer_cast<void>(std::make_shared<T>()); },
                typeid(StackPanel));
            descriptor.template EnableBaseTypeAccess<T, StackPanel>();
            descriptor.template EnableBaseObjectAccess<T>();
            return descriptor;
        }

        TypeDescriptor MakeExportOptionsDescriptor()
        {
            TypeDescriptor descriptor("ExportOptions", typeid(ExportOptions),
                                      [] { return std::static_pointer_cast<void>(std::make_shared<ExportOptions>()); });
            descriptor.AddProperty(MakeScalarProperty<ExportOptions, std::optional<std::string>>(
                "Namespace", [](const ExportOptions &object) { return object.getNamespaceProperty(); },
                [](ExportOptions &object, const std::optional<std::string> &value)
                { object.setNamespaceProperty(value); }, std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<ExportOptions, std::optional<std::string>>(
                "Class", [](const ExportOptions &object) { return object.getClassProperty(); },
                [](ExportOptions &object, const std::optional<std::string> &value) { object.setClassProperty(value); },
                std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<ExportOptions, std::optional<std::string>>(
                "OutputPath", [](const ExportOptions &object) { return object.getOutputPathProperty(); },
                [](ExportOptions &object, const std::optional<std::string> &value)
                { object.setOutputPathProperty(value); }, std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<ExportOptions, std::optional<std::string>>(
                "TemplateDesigner", [](const ExportOptions &object) { return object.getTemplateDesignerProperty(); },
                [](ExportOptions &object, const std::optional<std::string> &value)
                { object.setTemplateDesignerProperty(value); }, std::nullopt));
            descriptor.AddProperty(MakeScalarProperty<ExportOptions, std::optional<std::string>>(
                "TemplateMain", [](const ExportOptions &object) { return object.getTemplateMainProperty(); },
                [](ExportOptions &object, const std::optional<std::string> &value)
                { object.setTemplateMainProperty(value); }, std::nullopt));
            return descriptor;
        }

        TypeDescriptor MakeProjectDescriptor()
        {
            TypeDescriptor descriptor("Project", typeid(Project),
                                      [] { return std::static_pointer_cast<void>(std::make_shared<Project>()); });
            descriptor.AddProperty(MakeScalarProperty<Project, std::optional<std::string>>(
                "StylesheetPath", [](const Project &object) { return object.getStylesheetPathProperty(); },
                [](Project &object, const std::optional<std::string> &value)
                { object.setStylesheetPathProperty(value); }, std::nullopt));
            PropertyMetadata assetsPathMetadata;
            assetsPathMetadata.FilePath =
                Attributes::FilePathAttribute(Graphics2D::UI::File::FileDialogMode::ChooseFolder);
            descriptor.AddProperty(MakeScalarProperty<Project, std::optional<std::string>>(
                "DesignerRtfAssetsPath",
                [](const Project &object) { return object.getDesignerRtfAssetsPathProperty(); },
                [](Project &object, const std::optional<std::string> &value)
                { object.setDesignerRtfAssetsPathProperty(value); }, std::nullopt, std::move(assetsPathMetadata)));
            descriptor.AddProperty(PropertyDescriptor(
                "ExportOptions", typeid(ExportOptions), {}, {}, std::nullopt, {}, {}, {},
                ComplexPropertyAdapter::SingleReadOnly(
                    typeid(ExportOptions),
                    [](void *object) { return &static_cast<Project *>(object)->getExportOptionsProperty(); },
                    [](const void *object)
                    {
                        return EnumerateExportOptions(static_cast<const Project *>(object)->getExportOptionsProperty());
                    })));
            PropertyMetadata rootMetadata;
            rootMetadata.Content = true;
            descriptor.AddProperty(PropertyDescriptor(
                "Root", typeid(std::shared_ptr<Widget>), {}, {}, std::nullopt, std::move(rootMetadata), {}, {},
                ComplexPropertyAdapter::SingleWritable(
                    typeid(Widget), [](void *object, const std::shared_ptr<void> &value)
                    { static_cast<Project *>(object)->setRootProperty(AsWidget(value)); }, [](const void *object)
                    { return EnumerateWidget(static_cast<const Project *>(object)->getRootProperty()); })));
            return descriptor;
        }
    } // namespace

    void RegisterMyraTypes(TypeRegistry &registry)
    {
        registry.Register(MakeBaseObjectDescriptor());
        registry.Register(MakeIMenuItemDescriptor());
        registry.Register(MakeMenuItemDescriptor());
        registry.Register(MakeMenuSeparatorDescriptor());
        registry.Register(MakeWidgetDescriptor());
        registry.Register(MakeMenuDescriptor());
        registry.Register(MakeConcreteMenuDescriptor<HorizontalMenu>("HorizontalMenu", HorizontalAlignment::Stretch,
                                                                     VerticalAlignment::Top));
        registry.Register(MakeConcreteMenuDescriptor<VerticalMenu>("VerticalMenu", HorizontalAlignment::Left,
                                                                   VerticalAlignment::Top));
        registry.Register(MakeImageDescriptor());
        registry.Register(MakeSeparatorWidgetDescriptor());
        registry.Register(MakeConcreteSeparatorDescriptor<HorizontalSeparator>(
            "HorizontalSeparator", HorizontalAlignment::Stretch, VerticalAlignment::Center));
        registry.Register(MakeConcreteSeparatorDescriptor<VerticalSeparator>(
            "VerticalSeparator", HorizontalAlignment::Center, VerticalAlignment::Stretch));
        registry.Register(MakeProgressBarDescriptor());
        registry.Register(MakeConcreteProgressBarDescriptor<HorizontalProgressBar>(
            "HorizontalProgressBar", HorizontalAlignment::Stretch, VerticalAlignment::Top));
        registry.Register(MakeConcreteProgressBarDescriptor<VerticalProgressBar>(
            "VerticalProgressBar", HorizontalAlignment::Left, VerticalAlignment::Stretch));
        registry.Register(MakeSliderDescriptor());
        registry.Register(MakeConcreteSliderDescriptor<HorizontalSlider>(
            "HorizontalSlider", HorizontalAlignment::Stretch, VerticalAlignment::Top));
        registry.Register(MakeConcreteSliderDescriptor<VerticalSlider>("VerticalSlider", HorizontalAlignment::Left,
                                                                       VerticalAlignment::Stretch));
        registry.Register(MakeContentControlDescriptor());
        registry.Register(MakeScrollViewerDescriptor());
        registry.Register(MakeListViewDescriptor());
        registry.Register(MakeComboViewDescriptor());
        registry.Register(MakeTabItemDescriptor());
        registry.Register(MakeTabControlDescriptor());
        registry.Register(MakeButtonBaseDescriptor());
        registry.Register(MakeButtonDescriptor());
        registry.Register(MakeToggleButtonDescriptor());
        registry.Register(MakeCheckButtonBaseDescriptor());
        registry.Register(MakeCheckButtonDescriptor());
        registry.Register(MakeRadioButtonDescriptor());
        registry.Register(MakeContainerDescriptor());
        registry.Register(MakeProportionDescriptor());
        registry.Register(MakePanelDescriptor());
        registry.Register(MakeGridDescriptor());
        registry.Register(MakeSplitPaneDescriptor());
        registry.Register(MakeConcreteSplitPaneDescriptor<HorizontalSplitPane>("HorizontalSplitPane"));
        registry.Register(MakeConcreteSplitPaneDescriptor<VerticalSplitPane>("VerticalSplitPane"));
        registry.Register(MakeStackPanelDescriptor());
        registry.Register(MakeConcreteStackPanelDescriptor<HorizontalStackPanel>("HorizontalStackPanel"));
        registry.Register(MakeConcreteStackPanelDescriptor<VerticalStackPanel>("VerticalStackPanel"));
        registry.Register(MakeExportOptionsDescriptor());
        registry.Register(MakeProjectDescriptor());

        static_cast<void>(Grid::getColumnProperty());
        static_cast<void>(Grid::getRowProperty());
        static_cast<void>(Grid::getColumnSpanProperty());
        static_cast<void>(Grid::getRowSpanProperty());
        static_cast<void>(StackPanel::getProportionTypeProperty());
        static_cast<void>(StackPanel::getProportionValueProperty());
    }

    TypeRegistry CreateMyraTypeRegistry()
    {
        TypeRegistry registry;
        RegisterMyraTypes(registry);
        return registry;
    }
} // namespace Myra::MML
