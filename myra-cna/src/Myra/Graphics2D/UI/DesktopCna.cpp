// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;

    Microsoft::Xna::Framework::Rectangle Desktop::DefaultBoundsFetcher()
    {
        const Microsoft::Xna::Framework::Point size = Utility::CrossEngineStuff::getViewSizeProperty();
        return {0, 0, size.X, size.Y};
    }

    void Desktop::InitializeTextInput()
    {
        using Microsoft::Xna::Framework::Input::TextInputEXT;
        TextInputEXT::StartTextInput();
        const std::shared_ptr<InputProcessor> processor = inputProcessor_;
        textInputToken_ =
            TextInputEXT::TextInput.Add([processor](const char16_t character) { processor->ProcessChar(character); });
    }

    void Desktop::DisposeTextInput() noexcept
    {
        if (!textInputToken_)
        {
            return;
        }
        // The selected FNA Desktop removes its handler but does not globally stop text input.
        Microsoft::Xna::Framework::Input::TextInputEXT::TextInput.Remove(*textInputToken_);
        textInputToken_.reset();
    }

    void Desktop::Render()
    {
        if (disposed_)
        {
            throw std::logic_error("A disposed Desktop cannot render another frame.");
        }

        UpdateLayout();
        UpdateInput();
        ProcessWidgetInput();
        InputEventsManager::ProcessEvents();
        UpdateLayout();
        RenderVisual();
    }

    void Desktop::RenderVisual()
    {
        if (disposed_)
        {
            throw std::logic_error("A disposed Desktop cannot render visuals.");
        }
        if (!renderContext_)
        {
            renderContext_ = std::make_shared<Graphics2D::RenderContext>();
        }

        const std::shared_ptr<Graphics2D::RenderContext> retainedContext = renderContext_;
        Graphics2D::RenderContext &context = *retainedContext;
        const Rectangle oldDeviceScissor = context.getDeviceScissorProperty();
        bool rendering = false;
        try
        {
            context.Begin();
            rendering = true;
            context.setTransformProperty(getTransformProperty());
            if (Utility::Mathematics::IsZero(rotation_))
            {
                context.setScissorProperty(getTransformProperty().Apply(getLayoutBoundsProperty()));
            }
            context.setOpacityProperty(opacity_);

            const std::shared_ptr<Graphics2D::IBrush> background = background_;
            if (background)
            {
                background->Draw(context, getLayoutBoundsProperty(), Color::White);
            }

            const std::vector<std::shared_ptr<Widget>> roots = getChildrenCopyProperty();
            for (const std::shared_ptr<Widget> &widget : roots)
            {
                if (!widget->getVisibleProperty())
                {
                    continue;
                }
                if (MyraEnvironment::getEnableModalDarkeningProperty() && widget->getIsModalProperty())
                {
                    context.FillRectangle(getLayoutBoundsProperty(), MyraEnvironment::getDarkeningColorProperty());
                }
                widget->Render(context);
            }

            context.End();
            rendering = false;
            context.setDeviceScissorProperty(oldDeviceScissor);
        }
        catch (...)
        {
            const std::exception_ptr pendingException = std::current_exception();
            if (rendering && context.getIsRenderingProperty())
            {
                try
                {
                    context.End();
                }
                catch (...)
                {
                }
            }
            try
            {
                context.setDeviceScissorProperty(oldDeviceScissor);
            }
            catch (...)
            {
            }
            std::rethrow_exception(pendingException);
        }
    }

    void Desktop::DisposeGraphicsResources()
    {
        if (renderContext_)
        {
            renderContext_->Dispose();
            renderContext_.reset();
        }
    }
} // namespace Myra::Graphics2D::UI
