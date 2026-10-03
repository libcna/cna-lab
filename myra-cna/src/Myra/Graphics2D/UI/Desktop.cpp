// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <algorithm>
#include <exception>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/EventsExtensions.hpp"
#include "Myra/Utility/Mathematics.hpp"
#include "Myra/Utility/UIUtils.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] int CheckedContextPosition(const std::int64_t value)
        {
            if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("Context-menu position is outside the supported integer range.");
            }
            return static_cast<int>(value);
        }
    } // namespace

    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    void Desktop::ValidatedWidgetCollection::InsertItem(const SharpRuntime::intcs index,
                                                        const std::shared_ptr<Widget> &item)
    {
        if (!item)
        {
            throw std::invalid_argument("A desktop root widget cannot be null.");
        }
        if (Contains(item))
        {
            throw std::invalid_argument("A desktop cannot contain the same root widget more than once.");
        }
        WidgetCollection::InsertItem(index, item);
    }

    void Desktop::ValidatedWidgetCollection::SetItem(const SharpRuntime::intcs index,
                                                     const std::shared_ptr<Widget> &item)
    {
        if (!item)
        {
            throw std::invalid_argument("A desktop root widget cannot be null.");
        }
        const std::shared_ptr<Widget> &oldItem = getItem(index);
        if (oldItem != item && Contains(item))
        {
            throw std::invalid_argument("A desktop cannot contain the same root widget more than once.");
        }
        WidgetCollection::SetItem(index, item);
    }

    Desktop::Desktop() : boundsFetcher_(DefaultBoundsFetcher), inputProcessor_(std::make_shared<InputProcessor>(*this))
    {
        widgets_.CollectionChanged.push_back([this](auto *, const auto &) { OnWidgetsChanged(); });
        KeyDownHandler = [this](const Microsoft::Xna::Framework::Input::Keys key) { OnKeyDown(key); };
        InitializeTextInput();
    }

    Desktop::~Desktop()
    {
        destroying_ = true;
        Dispose();
        contextMenu_.reset();
        tooltip_.reset();
        tooltipOwner_.reset();
        previousKeyboardFocus_.reset();
        inputProcessor_->Detach();
        try
        {
            ChangeFocus(nullptr, false);
        }
        catch (...)
        {
            if (focusedKeyboardWidget_ != nullptr)
            {
                focusedKeyboardWidget_->isKeyboardFocused_ = false;
            }
            focusedKeyboardWidget_ = nullptr;
            focusChanging_ = false;
        }
        const std::vector<std::shared_ptr<Widget>> roots = attachedRoots_;
        attachedRoots_.clear();
        for (const std::shared_ptr<Widget> &root : roots)
        {
            if (root && root->desktop_ == this)
            {
                try
                {
                    root->SetDesktop(nullptr);
                }
                catch (...)
                {
                    ForceDetachForDestruction(*root);
                }
            }
        }
    }

    bool Desktop::getIsDisposedProperty() const noexcept
    {
        return disposed_;
    }

    void Desktop::Dispose()
    {
        if (disposed_)
        {
            return;
        }
        disposed_ = true;
        DisposeTextInput();
        DisposeGraphicsResources();
    }

    const Desktop::BoundsFetcher &Desktop::getBoundsFetcherProperty() const noexcept
    {
        return boundsFetcher_;
    }

    void Desktop::setBoundsFetcherProperty(BoundsFetcher value)
    {
        if (!value)
        {
            throw std::invalid_argument("A desktop bounds fetcher cannot be empty.");
        }
        boundsFetcher_ = std::move(value);
        InvalidateLayout();
    }

    std::shared_ptr<Widget> Desktop::getRootProperty() const
    {
        if (widgets_.getCountProperty() == 0)
        {
            return nullptr;
        }
        return widgets_.getItem(0);
    }

    void Desktop::setRootProperty(std::shared_ptr<Widget> value)
    {
        if (getRootProperty() == value)
        {
            return;
        }
        HideContextMenu();
        HideTooltip();
        widgets_.Clear();
        if (value)
        {
            widgets_.Add(value);
        }
    }

    Desktop::WidgetCollection &Desktop::getWidgetsProperty() noexcept
    {
        return widgets_;
    }

    const Desktop::WidgetCollection &Desktop::getWidgetsProperty() const noexcept
    {
        return widgets_;
    }

    const std::vector<std::shared_ptr<Widget>> &Desktop::getChildrenCopyProperty()
    {
        UpdateWidgetsCopy();
        return widgetsCopy_;
    }

    std::shared_ptr<Widget> Desktop::GetChild(const std::size_t index)
    {
        UpdateWidgetsCopy();
        if (index >= widgetsCopy_.size())
        {
            throw std::out_of_range("Desktop child index is out of range.");
        }
        return widgetsCopy_[index];
    }

    void Desktop::AddWidget(std::shared_ptr<Widget> widget)
    {
        if (!widget)
        {
            throw std::invalid_argument("A desktop root widget cannot be null.");
        }
        if (!widgets_.Contains(widget))
        {
            widgets_.Add(widget);
        }
    }

    bool Desktop::RemoveWidget(const Widget *const widget)
    {
        for (SharpRuntime::intcs index = 0; index < widgets_.getCountProperty(); ++index)
        {
            const std::shared_ptr<Widget> &candidate = widgets_.getItem(index);
            if (candidate.get() == widget)
            {
                return widgets_.Remove(candidate);
            }
        }
        return false;
    }

    void Desktop::ClearWidgets()
    {
        HideContextMenu();
        HideTooltip();
        widgets_.Clear();
    }

    HorizontalMenu *Desktop::getMenuBarProperty() const noexcept
    {
        return menuBar_;
    }

    std::shared_ptr<Widget> Desktop::getContextMenuProperty() const
    {
        return contextMenu_;
    }

    void Desktop::ShowContextMenu(std::shared_ptr<Widget> menu, Point position)
    {
        HideContextMenu();
        if (contextMenu_ || !menu)
        {
            return;
        }

        position = ToLocal(position);
        FixOverWidgetPosition(*menu, position);
        if (contextMenu_)
        {
            return;
        }

        contextMenu_ = menu;
        menu->setVisibleProperty(true);
        if (contextMenu_ != menu)
        {
            return;
        }
        AddWidget(menu);
        if (contextMenu_ != menu || menu->desktop_ != this)
        {
            return;
        }

        if (menu->getAcceptsKeyboardFocusProperty())
        {
            if (const std::shared_ptr<Widget> previous = RetainWidget(focusedKeyboardWidget_))
            {
                previousKeyboardFocus_ = previous;
            }
            setFocusedKeyboardWidgetProperty(menu.get());
        }
    }

    void Desktop::HideContextMenu()
    {
        const std::shared_ptr<Widget> menu = contextMenu_;
        if (!menu)
        {
            return;
        }

        // Clear the public state before callbacks so a callback can safely show a replacement.
        contextMenu_.reset();
        std::exception_ptr pendingException;
        try
        {
            static_cast<void>(RemoveWidget(menu.get()));
        }
        catch (...)
        {
            pendingException = std::current_exception();
        }
        try
        {
            menu->setVisibleProperty(false);
        }
        catch (...)
        {
            if (!pendingException)
            {
                pendingException = std::current_exception();
            }
        }
        try
        {
            Utility::EventsExtensions::Invoke(ContextMenuClosed, menu.get(), InputEventType::ContextMenuClosing);
        }
        catch (...)
        {
            if (!pendingException)
            {
                pendingException = std::current_exception();
            }
        }

        if (!contextMenu_)
        {
            const std::shared_ptr<Widget> previous = previousKeyboardFocus_.lock();
            previousKeyboardFocus_.reset();
            if (previous && previous->desktop_ == this)
            {
                try
                {
                    setFocusedKeyboardWidgetProperty(previous.get());
                }
                catch (...)
                {
                    if (!pendingException)
                    {
                        pendingException = std::current_exception();
                    }
                }
            }
        }

        if (pendingException)
        {
            std::rethrow_exception(pendingException);
        }
    }

    std::shared_ptr<Widget> Desktop::getTooltipProperty() const
    {
        return tooltip_;
    }

    void Desktop::ShowTooltip(Widget &owner, Point position)
    {
        const std::optional<std::string> &text = owner.getTooltipProperty();
        if (!text || text->empty())
        {
            return;
        }

        const std::shared_ptr<Widget> retainedOwner = RetainWidget(&owner);
        if (!retainedOwner)
        {
            return;
        }

        HideTooltip();
        const MyraEnvironment::TooltipCreator creator = MyraEnvironment::getTooltipCreatorProperty();
        if (!creator || retainedOwner->desktop_ != this)
        {
            return;
        }

        std::shared_ptr<Widget> tooltip = creator(*retainedOwner);
        if (!tooltip || tooltip == retainedOwner || retainedOwner->desktop_ != this || tooltip_)
        {
            return;
        }

        position = ToLocal(position);
        FixOverWidgetPosition(*tooltip, position);
        if (retainedOwner->desktop_ != this || tooltip_)
        {
            return;
        }

        tooltip_ = tooltip;
        tooltipOwner_ = retainedOwner;
        try
        {
            tooltip->setVisibleProperty(true);
            if (tooltip_ == tooltip)
            {
                AddWidget(tooltip);
            }
        }
        catch (...)
        {
            if (tooltip_ == tooltip)
            {
                tooltip_.reset();
                tooltipOwner_.reset();
            }
            throw;
        }
    }

    void Desktop::HideTooltip()
    {
        const std::shared_ptr<Widget> tooltip = tooltip_;
        if (!tooltip)
        {
            tooltipOwner_.reset();
            return;
        }

        tooltip_.reset();
        tooltipOwner_.reset();
        std::exception_ptr pendingException;
        try
        {
            static_cast<void>(RemoveWidget(tooltip.get()));
        }
        catch (...)
        {
            pendingException = std::current_exception();
        }
        try
        {
            tooltip->setVisibleProperty(false);
        }
        catch (...)
        {
            if (!pendingException)
            {
                pendingException = std::current_exception();
            }
        }
        if (pendingException)
        {
            std::rethrow_exception(pendingException);
        }
    }

    void Desktop::FixOverWidgetPosition(Widget &widget, Point position)
    {
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Top);

        const Rectangle layoutBounds = getLayoutBoundsProperty();
        const Point measure = widget.Measure(Point(layoutBounds.Width, layoutBounds.Height));
        const int right = layoutBounds.getRightProperty();
        const int bottom = layoutBounds.getBottomProperty();
        if (static_cast<std::int64_t>(position.X) + measure.X > right)
        {
            position.X = CheckedContextPosition(static_cast<std::int64_t>(right) - measure.X);
        }
        if (static_cast<std::int64_t>(position.Y) + measure.Y > bottom)
        {
            position.Y = CheckedContextPosition(static_cast<std::int64_t>(bottom) - measure.Y);
        }
        widget.setLeftProperty(position.X);
        widget.setTopProperty(position.Y);
    }

    const Rectangle &Desktop::getInternalBoundsProperty() const noexcept
    {
        return internalBounds_;
    }

    Rectangle Desktop::getLayoutBoundsProperty() const
    {
        return {0, 0, internalBounds_.Width, internalBounds_.Height};
    }

    Widget *Desktop::getFocusedKeyboardWidgetProperty() const noexcept
    {
        return focusedKeyboardWidget_;
    }

    void Desktop::setFocusedKeyboardWidgetProperty(Widget *const value)
    {
        if (value != nullptr && value->desktop_ != this)
        {
            throw std::invalid_argument("Keyboard focus can only be assigned to a widget on this desktop.");
        }
        if (focusChanging_)
        {
            pendingFocus_ = value;
            pendingFocusChange_ = true;
            return;
        }

        Widget *requested = value;
        do
        {
            pendingFocusChange_ = false;
            ChangeFocus(requested, true);
            requested = pendingFocus_;
        } while (pendingFocusChange_);
    }

    float Desktop::getOpacityProperty() const noexcept
    {
        return opacity_;
    }

    void Desktop::setOpacityProperty(const float value) noexcept
    {
        opacity_ = value;
    }

    const Vector2 &Desktop::getScaleProperty() const noexcept
    {
        return scale_;
    }

    void Desktop::setScaleProperty(const Vector2 value)
    {
        if (Utility::Mathematics::EpsilonEquals(value, scale_))
        {
            return;
        }
        scale_ = value;
        InvalidateTransform();
    }

    const Vector2 &Desktop::getTransformOriginProperty() const noexcept
    {
        return transformOrigin_;
    }

    void Desktop::setTransformOriginProperty(const Vector2 value)
    {
        if (Utility::Mathematics::EpsilonEquals(value, transformOrigin_))
        {
            return;
        }
        transformOrigin_ = value;
        InvalidateTransform();
    }

    float Desktop::getRotationProperty() const noexcept
    {
        return rotation_;
    }

    void Desktop::setRotationProperty(const float value)
    {
        if (Utility::Mathematics::EpsilonEquals(value, rotation_))
        {
            return;
        }
        rotation_ = value;
        InvalidateTransform();
    }

    std::shared_ptr<Graphics2D::IBrush> Desktop::getBackgroundProperty() const
    {
        return background_;
    }

    void Desktop::setBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        background_ = std::move(value);
    }

    bool Desktop::getHasModalWidgetProperty()
    {
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (auto iterator = snapshot.rbegin(); iterator != snapshot.rend(); ++iterator)
        {
            const std::shared_ptr<Widget> &widget = *iterator;
            if (widget->getVisibleProperty() && widget->getEnabledProperty() && widget->getIsModalProperty())
            {
                return true;
            }
        }
        return false;
    }

    void Desktop::InvalidateLayout() noexcept
    {
        ++layoutInvalidationVersion_;
        layoutDirty_ = true;
        menuBar_ = nullptr;
    }

    void Desktop::UpdateLayout()
    {
        setInternalBoundsProperty(boundsFetcher_());
        if (internalBounds_.getIsEmptyProperty() || !layoutDirty_)
        {
            return;
        }

        const std::uint64_t invalidationVersion = layoutInvalidationVersion_;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget> &child : snapshot)
        {
            if (child->getVisibleProperty())
            {
                child->Arrange(getLayoutBoundsProperty());
            }
        }

        menuBar_ = nullptr;
        const std::vector<std::shared_ptr<Widget>> menuSnapshot = getChildrenCopyProperty();
        for (auto iterator = menuSnapshot.rbegin(); iterator != menuSnapshot.rend(); ++iterator)
        {
            const std::shared_ptr<Widget> &widget = *iterator;
            if (!widget->getVisibleProperty())
            {
                continue;
            }
            menuBar_ = widget->FindChild<HorizontalMenu>();
            if (menuBar_ != nullptr)
            {
                break;
            }
        }
        layoutDirty_ = layoutInvalidationVersion_ != invalidationVersion;
    }

    void Desktop::ProcessWidgets(const WidgetOperation &operation)
    {
        if (!operation)
        {
            throw std::invalid_argument("A desktop widget operation cannot be empty.");
        }
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget> &widget : snapshot)
        {
            if (!Utility::UIUtils::ProcessWidgets(*widget, operation))
            {
                return;
            }
        }
    }

    Widget *Desktop::FindChild(const WidgetPredicate &predicate)
    {
        if (!predicate)
        {
            throw std::invalid_argument("A desktop widget predicate cannot be empty.");
        }
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget> &widget : snapshot)
        {
            if (predicate(*widget))
            {
                return widget.get();
            }
            if (Widget *const result = widget->FindChild(predicate))
            {
                return result;
            }
        }
        return nullptr;
    }

    Widget *Desktop::FindChildById(const std::string &id)
    {
        return FindChild(
            [&id](Widget &widget)
            {
                const std::optional<std::string> &widgetId = widget.getIdProperty();
                return widgetId.has_value() && *widgetId == id;
            });
    }

    std::size_t Desktop::CalculateTotalWidgets(const bool visibleOnly)
    {
        std::size_t result = 0;
        for (SharpRuntime::intcs index = 0; index < widgets_.getCountProperty(); ++index)
        {
            const std::shared_ptr<Widget> &widget = widgets_.getItem(index);
            if (visibleOnly && !widget->getVisibleProperty())
            {
                continue;
            }
            ++result;
            result += widget->CalculateTotalChildCount(visibleOnly);
        }
        return result;
    }

    Vector2 Desktop::ToLocal(const Vector2 source)
    {
        return getTransformProperty().InverseApply(source);
    }

    Vector2 Desktop::ToGlobal(const Vector2 position)
    {
        return getTransformProperty().Apply(position);
    }

    Point Desktop::ToLocal(const Point source)
    {
        return getTransformProperty().InverseApply(source);
    }

    Point Desktop::ToGlobal(const Point position)
    {
        return getTransformProperty().Apply(position);
    }

    void Desktop::OnWidgetsChanged()
    {
        if (destroying_)
        {
            return;
        }
        widgetsDirty_ = true;
        InvalidateLayout();
        if (synchronizingRoots_)
        {
            rootsResyncRequested_ = true;
            return;
        }
        SynchronizeRoots();
        ReconcileContextMenuOwnership();
        ReconcileTooltipOwnership();
    }

    void Desktop::SynchronizeRoots()
    {
        std::exception_ptr pendingException;
        synchronizingRoots_ = true;
        do
        {
            rootsResyncRequested_ = false;
            try
            {
                SynchronizeRootsOnce();
            }
            catch (...)
            {
                if (!pendingException)
                {
                    pendingException = std::current_exception();
                }
            }
        } while (rootsResyncRequested_);
        synchronizingRoots_ = false;
        if (pendingException)
        {
            std::rethrow_exception(pendingException);
        }
    }

    void Desktop::SynchronizeRootsOnce()
    {
        std::exception_ptr pendingException;
        std::vector<std::shared_ptr<Widget>> current;
        current.reserve(static_cast<std::size_t>(widgets_.getCountProperty()));
        for (SharpRuntime::intcs index = 0; index < widgets_.getCountProperty(); ++index)
        {
            current.push_back(widgets_.getItem(index));
        }

        const std::vector<std::shared_ptr<Widget>> previous = attachedRoots_;
        for (const std::shared_ptr<Widget> &root : previous)
        {
            if (std::find(current.begin(), current.end(), root) == current.end() && root->desktop_ == this)
            {
                try
                {
                    root->SetDesktop(nullptr);
                }
                catch (...)
                {
                    if (!pendingException)
                    {
                        pendingException = std::current_exception();
                    }
                }
            }
        }

        for (const std::shared_ptr<Widget> &root : current)
        {
            try
            {
                RemoveWidgetFromPreviousOwner(root);
            }
            catch (...)
            {
                if (!pendingException)
                {
                    pendingException = std::current_exception();
                }
            }
            if (root->desktop_ != this)
            {
                try
                {
                    root->SetDesktop(this);
                }
                catch (...)
                {
                    if (!pendingException)
                    {
                        pendingException = std::current_exception();
                    }
                }
            }
        }
        attachedRoots_ = std::move(current);
        if (pendingException)
        {
            std::rethrow_exception(pendingException);
        }
    }

    void Desktop::ReconcileContextMenuOwnership()
    {
        const std::shared_ptr<Widget> menu = contextMenu_;
        if (!menu || (menu->parent_ == nullptr && menu->desktop_ == this))
        {
            return;
        }

        contextMenu_.reset();
        const std::shared_ptr<Widget> previous = previousKeyboardFocus_.lock();
        previousKeyboardFocus_.reset();
        std::exception_ptr pendingException;
        try
        {
            menu->setVisibleProperty(false);
        }
        catch (...)
        {
            pendingException = std::current_exception();
        }
        if (previous && previous->desktop_ == this && focusedKeyboardWidget_ == nullptr)
        {
            try
            {
                setFocusedKeyboardWidgetProperty(previous.get());
            }
            catch (...)
            {
                if (!pendingException)
                {
                    pendingException = std::current_exception();
                }
            }
        }
        if (pendingException)
        {
            std::rethrow_exception(pendingException);
        }
    }

    void Desktop::ReconcileTooltipOwnership()
    {
        const std::shared_ptr<Widget> tooltip = tooltip_;
        const std::shared_ptr<Widget> owner = tooltipOwner_.lock();
        if (!tooltip || (owner && owner->desktop_ == this && tooltip->parent_ == nullptr && tooltip->desktop_ == this))
        {
            if (!tooltip)
            {
                tooltipOwner_.reset();
            }
            return;
        }

        tooltip_.reset();
        tooltipOwner_.reset();
        tooltip->setVisibleProperty(false);
    }

    void Desktop::RemoveWidgetFromPreviousOwner(const std::shared_ptr<Widget> &widget)
    {
        while (widget->parent_ != nullptr)
        {
            Widget *const oldParent = widget->parent_;
            if (!oldParent->RemoveChild(widget.get()))
            {
                throw std::logic_error("A widget parent did not retain its reported child.");
            }
        }
        while (widget->desktop_ != nullptr && widget->desktop_ != this)
        {
            Desktop *const oldDesktop = widget->desktop_;
            if (!oldDesktop->RemoveWidget(widget.get()))
            {
                widget->SetDesktop(nullptr);
            }
        }
    }

    std::shared_ptr<Widget> Desktop::RetainWidget(const Widget *const widget) const
    {
        if (widget == nullptr)
        {
            return nullptr;
        }

        const auto findTarget = [&widget](auto &&self,
                                          const std::shared_ptr<Widget> &current) -> std::shared_ptr<Widget>
        {
            if (current.get() == widget)
            {
                return current;
            }
            for (const std::shared_ptr<Widget> &child : current->getChildrenProperty())
            {
                if (std::shared_ptr<Widget> result = self(self, child))
                {
                    return result;
                }
            }
            return nullptr;
        };

        for (const std::shared_ptr<Widget> &root : attachedRoots_)
        {
            if (std::shared_ptr<Widget> result = findTarget(findTarget, root))
            {
                return result;
            }
        }
        return nullptr;
    }

    bool Desktop::ContainsWidget(const Widget &root, const Widget *const target) const
    {
        if (&root == target)
        {
            return true;
        }
        for (const std::shared_ptr<Widget> &child : root.children_)
        {
            if (ContainsWidget(*child, target))
            {
                return true;
            }
        }
        return false;
    }

    void Desktop::ClearFocusForDetaching(Widget &root)
    {
        if (menuBar_ != nullptr && ContainsWidget(root, menuBar_))
        {
            menuBar_ = nullptr;
        }
        if (focusedKeyboardWidget_ == nullptr || !ContainsWidget(root, focusedKeyboardWidget_))
        {
            return;
        }
        if (focusChanging_)
        {
            Widget *const oldValue = focusedKeyboardWidget_;
            focusedKeyboardWidget_ = nullptr;
            focusClearedDuringCallback_ = true;
            try
            {
                oldValue->OnLostKeyboardFocus();
            }
            catch (...)
            {
                oldValue->isKeyboardFocused_ = false;
                throw;
            }
            return;
        }
        Widget *const oldValue = focusedKeyboardWidget_;
        try
        {
            ChangeFocus(nullptr, false);
        }
        catch (...)
        {
            focusedKeyboardWidget_ = nullptr;
            focusChanging_ = false;
            oldValue->isKeyboardFocused_ = false;
            throw;
        }
    }

    void Desktop::ForceDetachForDestruction(Widget &root) noexcept
    {
        root.UnsubscribeDragEvents();
        if (root.desktop_ == this)
        {
            root.desktop_ = nullptr;
        }
        root.transformDirty_ = true;
        for (const std::shared_ptr<Widget> &child : root.children_)
        {
            if (child)
            {
                ForceDetachForDestruction(*child);
            }
        }
    }

    void Desktop::ChangeFocus(Widget *const value, const bool allowCancellation)
    {
        if (value == focusedKeyboardWidget_)
        {
            return;
        }
        if (value != nullptr && value->desktop_ != this)
        {
            throw std::invalid_argument("Keyboard focus can only be assigned to a widget on this desktop.");
        }

        Widget *const oldValue = focusedKeyboardWidget_;
        const std::shared_ptr<Widget> retainedOld = RetainWidget(oldValue);
        focusChanging_ = true;
        focusClearedDuringCallback_ = false;
        try
        {
            if (oldValue != nullptr)
            {
                Events::CancellableEventArgsT<Widget *> arguments(oldValue, InputEventType::KeyboardFocusLosing);
                WidgetLosingKeyboardFocus.Invoke(nullptr, arguments);
                if (focusClearedDuringCallback_)
                {
                    focusChanging_ = false;
                    return;
                }
                if (allowCancellation && oldValue->desktop_ == this && arguments.Cancel)
                {
                    focusChanging_ = false;
                    return;
                }
            }

            if (value != nullptr && value->desktop_ != this)
            {
                focusChanging_ = false;
                return;
            }
            focusedKeyboardWidget_ = value;
            if (oldValue != nullptr)
            {
                oldValue->OnLostKeyboardFocus();
            }
            if (focusedKeyboardWidget_ != nullptr)
            {
                const std::shared_ptr<Widget> retainedNew = RetainWidget(focusedKeyboardWidget_);
                focusedKeyboardWidget_->OnGotKeyboardFocus();
                Utility::EventsExtensions::Invoke(WidgetGotKeyboardFocus, focusedKeyboardWidget_,
                                                  InputEventType::KeyboardFocusLosing);
                static_cast<void>(retainedNew);
            }
        }
        catch (...)
        {
            focusChanging_ = false;
            throw;
        }
        focusChanging_ = false;
        static_cast<void>(retainedOld);
    }

    void Desktop::InvalidateTransform()
    {
        transformDirty_ = true;
        const std::vector<std::shared_ptr<Widget>> snapshot = attachedRoots_;
        for (const std::shared_ptr<Widget> &child : snapshot)
        {
            child->InvalidateTransform();
        }
    }

    void Desktop::InvalidateWidgetsOrder() noexcept
    {
        widgetsDirty_ = true;
    }

    void Desktop::UpdateTransform()
    {
        if (!transformDirty_)
        {
            return;
        }
        transform_.emplace(Vector2(static_cast<float>(internalBounds_.X), static_cast<float>(internalBounds_.Y)),
                           Vector2(transformOrigin_.X * static_cast<float>(internalBounds_.Width),
                                   transformOrigin_.Y * static_cast<float>(internalBounds_.Height)),
                           scale_, rotation_ * std::numbers::pi_v<float> / 180.0F);
        transformDirty_ = false;
    }

    void Desktop::UpdateWidgetsCopy()
    {
        if (!widgetsDirty_)
        {
            return;
        }
        widgetsCopy_.clear();
        widgetsCopy_.reserve(static_cast<std::size_t>(widgets_.getCountProperty()));
        for (SharpRuntime::intcs index = 0; index < widgets_.getCountProperty(); ++index)
        {
            widgetsCopy_.push_back(widgets_.getItem(index));
        }
        Utility::UIUtils::SortWidgetsByZIndex(widgetsCopy_);
        widgetsDirty_ = false;
    }

    const Graphics2D::Transform &Desktop::getTransformProperty()
    {
        UpdateTransform();
        return *transform_;
    }

    void Desktop::setInternalBoundsProperty(const Rectangle value)
    {
        if (internalBounds_ == value)
        {
            return;
        }
        internalBounds_ = value;
        InvalidateLayout();
        InvalidateTransform();
    }
} // namespace Myra::Graphics2D::UI
