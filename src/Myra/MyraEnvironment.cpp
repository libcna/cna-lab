// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <utility>

namespace Myra
{
    Events::EventHandlingStrategy MyraEnvironment::eventHandlingModel_ = Events::EventHandlingStrategy::EventCapturing;
    bool MyraEnvironment::drawWidgetsFrames_ = false;
    bool MyraEnvironment::drawKeyboardFocusedWidgetFrame_ = false;
    bool MyraEnvironment::drawMouseHoveredWidgetFrame_ = false;
    bool MyraEnvironment::drawTextGlyphsFrames_ = false;
    bool MyraEnvironment::disableClipping_ = false;
    bool MyraEnvironment::setMouseCursorFromWidget_ = true;
    Graphics2D::UI::MouseCursorType MyraEnvironment::mouseCursorType_ = Graphics2D::UI::MouseCursorType::Arrow;
    Graphics2D::UI::MouseCursorType MyraEnvironment::defaultMouseCursorType_ = Graphics2D::UI::MouseCursorType::Arrow;
#ifdef MYRA_CNA_HAS_CNA_TARGET
    MyraEnvironment::MouseInfoGetter MyraEnvironment::mouseInfoGetter_ = MyraEnvironment::DefaultMouseInfoGetter;
    MyraEnvironment::DownKeysGetter MyraEnvironment::downKeysGetter_ = MyraEnvironment::DefaultDownKeysGetter;
#else
    MyraEnvironment::MouseInfoGetter MyraEnvironment::mouseInfoGetter_;
    MyraEnvironment::DownKeysGetter MyraEnvironment::downKeysGetter_;
#endif
    int MyraEnvironment::doubleClickIntervalInMs_ = 500;
    int MyraEnvironment::doubleClickRadius_ = 2;
    int MyraEnvironment::tooltipDelayInMs_ = 500;
    Microsoft::Xna::Framework::Point MyraEnvironment::tooltipOffset_{0, 20};
    MyraEnvironment::TooltipCreator MyraEnvironment::tooltipCreator_;

    Events::EventHandlingStrategy MyraEnvironment::getEventHandlingModelProperty() noexcept
    {
        return eventHandlingModel_;
    }

    void MyraEnvironment::setEventHandlingModelProperty(const Events::EventHandlingStrategy value) noexcept
    {
        eventHandlingModel_ = value;
    }

    bool MyraEnvironment::getDrawWidgetsFramesProperty() noexcept
    {
        return drawWidgetsFrames_;
    }

    void MyraEnvironment::setDrawWidgetsFramesProperty(const bool value) noexcept
    {
        drawWidgetsFrames_ = value;
    }

    bool MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty() noexcept
    {
        return drawKeyboardFocusedWidgetFrame_;
    }

    void MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(const bool value) noexcept
    {
        drawKeyboardFocusedWidgetFrame_ = value;
    }

    bool MyraEnvironment::getDrawMouseHoveredWidgetFrameProperty() noexcept
    {
        return drawMouseHoveredWidgetFrame_;
    }

    void MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(const bool value) noexcept
    {
        drawMouseHoveredWidgetFrame_ = value;
    }

    bool MyraEnvironment::getDrawTextGlyphsFramesProperty() noexcept
    {
        return drawTextGlyphsFrames_;
    }

    void MyraEnvironment::setDrawTextGlyphsFramesProperty(const bool value) noexcept
    {
        drawTextGlyphsFrames_ = value;
    }

    bool MyraEnvironment::getDisableClippingProperty() noexcept
    {
        return disableClipping_;
    }

    void MyraEnvironment::setDisableClippingProperty(const bool value) noexcept
    {
        disableClipping_ = value;
    }

    bool MyraEnvironment::getSetMouseCursorFromWidgetProperty() noexcept
    {
        return setMouseCursorFromWidget_;
    }

    void MyraEnvironment::setSetMouseCursorFromWidgetProperty(const bool value) noexcept
    {
        setMouseCursorFromWidget_ = value;
    }

    Graphics2D::UI::MouseCursorType MyraEnvironment::getMouseCursorTypeProperty() noexcept
    {
        return mouseCursorType_;
    }

    void MyraEnvironment::setMouseCursorTypeProperty(const Graphics2D::UI::MouseCursorType value)
    {
        if (mouseCursorType_ == value)
        {
            return;
        }

        // Preserve upstream assignment ordering: an invalid cast is retained in
        // the backing property before platform lookup reports that it is unmapped.
        mouseCursorType_ = value;
        ApplyMouseCursorType(value);
    }

    Graphics2D::UI::MouseCursorType MyraEnvironment::getDefaultMouseCursorTypeProperty() noexcept
    {
        return defaultMouseCursorType_;
    }

    void MyraEnvironment::setDefaultMouseCursorTypeProperty(const Graphics2D::UI::MouseCursorType value) noexcept
    {
        defaultMouseCursorType_ = value;
    }

    const MyraEnvironment::MouseInfoGetter &MyraEnvironment::getMouseInfoGetterProperty() noexcept
    {
        return mouseInfoGetter_;
    }

    void MyraEnvironment::setMouseInfoGetterProperty(MouseInfoGetter value)
    {
        mouseInfoGetter_ = std::move(value);
    }

    const MyraEnvironment::DownKeysGetter &MyraEnvironment::getDownKeysGetterProperty() noexcept
    {
        return downKeysGetter_;
    }

    void MyraEnvironment::setDownKeysGetterProperty(DownKeysGetter value)
    {
        downKeysGetter_ = std::move(value);
    }

    int MyraEnvironment::getDoubleClickIntervalInMsProperty() noexcept
    {
        return doubleClickIntervalInMs_;
    }

    void MyraEnvironment::setDoubleClickIntervalInMsProperty(const int value) noexcept
    {
        doubleClickIntervalInMs_ = value;
    }

    int MyraEnvironment::getDoubleClickRadiusProperty() noexcept
    {
        return doubleClickRadius_;
    }

    void MyraEnvironment::setDoubleClickRadiusProperty(const int value) noexcept
    {
        doubleClickRadius_ = value;
    }

    int MyraEnvironment::getTooltipDelayInMsProperty() noexcept
    {
        return tooltipDelayInMs_;
    }

    void MyraEnvironment::setTooltipDelayInMsProperty(const int value) noexcept
    {
        tooltipDelayInMs_ = value;
    }

    const Microsoft::Xna::Framework::Point &MyraEnvironment::getTooltipOffsetProperty() noexcept
    {
        return tooltipOffset_;
    }

    void MyraEnvironment::setTooltipOffsetProperty(const Microsoft::Xna::Framework::Point value) noexcept
    {
        tooltipOffset_ = value;
    }

    const MyraEnvironment::TooltipCreator &MyraEnvironment::getTooltipCreatorProperty() noexcept
    {
        return tooltipCreator_;
    }

    void MyraEnvironment::setTooltipCreatorProperty(TooltipCreator value)
    {
        tooltipCreator_ = std::move(value);
    }
} // namespace Myra
