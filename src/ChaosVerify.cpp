// SPDX-License-Identifier: MIT
// Checks that read back what was written or drawn and compare it with what XNA produces.
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <optional>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorWriteChannels.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicIndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicVertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SetDataOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

#include "ChaosSupport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace CnaKiller
{
    namespace
    {
        std::string Size(int width, int height)
        {
            return std::to_string(width) + "x" + std::to_string(height);
        }

        std::string Rect(const Rectangle& r)
        {
            return "(" + std::to_string(r.X) + "," + std::to_string(r.Y) + " " + Size(r.Width, r.Height) + ")";
        }

        /** @brief A clockwise full-screen quad: XNA's default CullCounterClockwise keeps it. */
        std::array<VertexPositionColor, 6> FullScreenQuad(const Color& color, float z)
        {
            return {VertexPositionColor(Vector3(-1, 1, z), color), VertexPositionColor(Vector3(1, 1, z), color),
                    VertexPositionColor(Vector3(-1, -1, z), color), VertexPositionColor(Vector3(1, 1, z), color),
                    VertexPositionColor(Vector3(1, -1, z), color), VertexPositionColor(Vector3(-1, -1, z), color)};
        }

        std::vector<Color> ReadColors(RenderTarget2D& target)
        {
            std::vector<Color> pixels(static_cast<std::size_t>(target.getWidthProperty()) *
                                      static_cast<std::size_t>(target.getHeightProperty()));
            target.GetData(pixels.data(), static_cast<int>(pixels.size()));
            return pixels;
        }
    }

    // -----------------------------------------------------------------------------------------
    // Texture2D SetData/GetData in every HiDef format, by level and rectangle
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifyTextureRoundTrip()
    {
        const std::vector<Support::FormatInfo>& formats = Support::TextureFormats();
        const Support::FormatInfo& info = formats[static_cast<std::size_t>(RandomInt(0, static_cast<int>(formats.size())))];
        if (info.format == SurfaceFormat::HdrBlendable)
            return; // a render-target format; XNA has no Texture2D of it

        int width = RandomInt(1, 97);
        int height = RandomInt(1, 97);
        if (info.compressed)
        {
            width = RandomInt(1, 25) * 4;
            height = RandomInt(1, 25) * 4;
        }
        const bool mipMap = Chance(2);
        Support::UnbindAll(Device());
        Texture2D texture(Device(), width, height, mipMap, info.format);

        const int levels = texture.getLevelCountProperty();
        const int level = RandomInt(0, levels);
        const int levelWidth = Support::LevelSize(width, level);
        const int levelHeight = Support::LevelSize(height, level);

        // Fill the whole level, then overwrite a region: what comes back must be the composite,
        // which catches a region landing at the wrong rows (a vertical flip) or with the wrong pitch.
        std::vector<std::uint8_t> expected = RandomBytes(Support::RegionBytes(info, levelWidth, levelHeight));
        Support::MakeExactlyStorable(info, expected);
        texture.SetData(level, nullptr, expected.data(), 0, static_cast<int>(expected.size()));

        std::optional<Rectangle> region;
        if (Chance(2))
        {
            int x = RandomInt(0, levelWidth);
            int y = RandomInt(0, levelHeight);
            int w = RandomInt(1, levelWidth - x + 1);
            int h = RandomInt(1, levelHeight - y + 1);
            if (info.compressed)
            {
                // Block-aligned, or reaching the level's edge.
                x -= x % 4;
                y -= y % 4;
                w = std::min(levelWidth - x, ((w + 3) / 4) * 4);
                h = std::min(levelHeight - y, ((h + 3) / 4) * 4);
            }
            region = Rectangle(x, y, w, h);
        }

        if (region)
        {
            std::vector<std::uint8_t> patch = RandomBytes(Support::RegionBytes(info, region->Width, region->Height));
            Support::MakeExactlyStorable(info, patch);
            // A leading window of junk: startIndex selects where reading begins.
            const int startIndex = RandomInt(0, 17);
            std::vector<std::uint8_t> source(static_cast<std::size_t>(startIndex), 0xCD);
            source.insert(source.end(), patch.begin(), patch.end());
            texture.SetData(level, &*region, source.data(), startIndex, static_cast<int>(patch.size()));

            if (info.compressed)
            {
                const int blocksWide = (levelWidth + 3) / 4;
                const int regionBlocksWide = (region->Width + 3) / 4;
                const int regionBlocksHigh = (region->Height + 3) / 4;
                for (int row = 0; row < regionBlocksHigh; ++row)
                {
                    std::memcpy(expected.data() + (static_cast<std::size_t>(region->Y / 4 + row) * blocksWide +
                                                   static_cast<std::size_t>(region->X / 4)) * info.bytes,
                                patch.data() + static_cast<std::size_t>(row) * regionBlocksWide * info.bytes,
                                static_cast<std::size_t>(regionBlocksWide) * info.bytes);
                }
            }
            else
            {
                for (int row = 0; row < region->Height; ++row)
                {
                    std::memcpy(expected.data() + (static_cast<std::size_t>(region->Y + row) * levelWidth +
                                                   static_cast<std::size_t>(region->X)) * info.bytes,
                                patch.data() + static_cast<std::size_t>(row) * region->Width * info.bytes,
                                static_cast<std::size_t>(region->Width) * info.bytes);
                }
            }
        }

        std::vector<std::uint8_t> readBack(expected.size() + 8, 0xEE);
        texture.GetData(level, nullptr, readBack.data(), 4, static_cast<int>(expected.size()));

        findings_.CountCheck();
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (readBack[i + 4] != expected[i])
            {
                const std::size_t unit = info.compressed ? 1 : static_cast<std::size_t>(info.bytes);
                const std::size_t texel = i / unit;
                Report(FindingKind::Mismatch,
                       std::string("Texture2D ") + info.name + " GetData does not return what SetData wrote",
                       Size(width, height) + (mipMap ? " mipmapped" : "") + ", level " + std::to_string(level) + " (" +
                           Size(levelWidth, levelHeight) + ")" + (region ? ", region " + Rect(*region) : "") +
                           ", first difference at byte " + std::to_string(i) +
                           (info.compressed ? "" : " (texel " + std::to_string(texel % levelWidth) + "," +
                                                       std::to_string(texel / levelWidth) + ")"));
                return;
            }
        }
        if (readBack[0] != 0xEE || readBack[3] != 0xEE || readBack[expected.size() + 4] != 0xEE)
        {
            Report(FindingKind::Mismatch, std::string("Texture2D ") + info.name + " GetData writes outside its window",
                   "startIndex 4, " + std::to_string(expected.size()) + " elements");
        }
    }

    // -----------------------------------------------------------------------------------------
    // Render targets
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifyRenderTargetClear()
    {
        const std::vector<SurfaceFormat>& formats = Support::RenderTargetFormats();
        const SurfaceFormat requested = formats[static_cast<std::size_t>(RandomInt(0, static_cast<int>(formats.size())))];
        const int width = RandomInt(1, 65);
        const int height = RandomInt(1, 65);
        const int multiSample = requested == SurfaceFormat::Color && Chance(3) ? 4 : 0;
        const bool preserve = Chance(3);
        const Color color = RandomAnyColor();

        RenderTarget2D target(Device(), width, height, false, requested, DepthFormat::None, multiSample,
                              preserve ? RenderTargetUsage::PreserveContents : RenderTargetUsage::DiscardContents);
        // XNA may substitute the closest supported format; the target reports the one it has.
        const SurfaceFormat actual = target.getFormatProperty();
        const Support::FormatInfo& info = Support::Info(actual);
        if (info.bytes == 0)
        {
            Report(FindingKind::Mismatch, "a render target reports a format that is not a HiDef surface format",
                   "requested " + std::string(Support::Info(requested).name));
            return;
        }

        GraphicsDevice& device = Device();
        device.SetRenderTarget(&target);
        device.Clear(color);
        device.SetRenderTarget(nullptr);
        if (preserve)
        {
            // PreserveContents: setting the target again without clearing keeps what it held.
            device.SetRenderTarget(&target);
            device.SetRenderTarget(nullptr);
        }

        std::vector<std::uint8_t> bytes(Support::RegionBytes(info, width, height));
        target.GetData(0, nullptr, bytes.data(), 0, static_cast<int>(bytes.size()));

        const std::array<float, 4> wanted{color.getRProperty() / 255.0f, color.getGProperty() / 255.0f,
                                          color.getBProperty() / 255.0f, color.getAProperty() / 255.0f};
        findings_.CountCheck();
        for (int i = 0; i < width * height; ++i)
        {
            const Support::Decoded texel =
                Support::DecodeRenderTargetTexel(actual, bytes.data() + static_cast<std::size_t>(i) * info.bytes);
            for (std::size_t c = 0; c < 4; ++c)
            {
                if (!texel.present[c])
                    continue;
                // A channel of a few bits holds the nearest representable value.
                if (std::abs(texel.value[c] - wanted[c]) > texel.tolerance[c] + 0.5f / 255.0f)
                {
                    Report(FindingKind::Mismatch,
                           std::string("a ") + info.name + " render target does not hold its Clear colour" +
                               (multiSample ? " after resolving MSAA" : "") + (preserve ? " (PreserveContents)" : ""),
                           "requested " + std::string(Support::Info(requested).name) + ", " + Size(width, height) +
                               ", cleared to " + Support::Describe(color) + ", texel " + std::to_string(i) +
                               " channel " + std::to_string(c) + " reads " + std::to_string(texel.value[c]) +
                               " for " + std::to_string(wanted[c]));
                    return;
                }
            }
        }
    }

    void ChaosEngine::ActionVerifyMultipleRenderTargets()
    {
        const int count = RandomInt(2, 5);
        const int width = RandomInt(1, 65);
        const int height = RandomInt(1, 65);
        const Color color = RandomOpaqueColor();

        std::vector<std::unique_ptr<RenderTarget2D>> targets;
        std::vector<RenderTargetBinding> bindings;
        for (int i = 0; i < count; ++i)
        {
            targets.push_back(std::make_unique<RenderTarget2D>(Device(), width, height));
            bindings.emplace_back(targets.back().get());
        }
        GraphicsDevice& device = Device();
        device.SetRenderTargets(bindings);
        device.Clear(color);
        device.SetRenderTarget(nullptr);

        findings_.CountCheck();
        for (int i = 0; i < count; ++i)
        {
            const std::vector<Color> pixels = ReadColors(*targets[static_cast<std::size_t>(i)]);
            const auto wrong = std::find_if(pixels.begin(), pixels.end(), [&](const Color& p) { return !Support::Near(p, color, 0); });
            if (wrong != pixels.end())
            {
                Report(FindingKind::Mismatch, "Clear with several render targets bound does not clear every one",
                       std::to_string(count) + " targets of " + Size(width, height) + ", target " + std::to_string(i) +
                           " reads " + Support::Describe(*wrong) + " for " + Support::Describe(color));
                return;
            }
        }
    }

    void ChaosEngine::ActionVerifyCubeRenderTarget()
    {
        const int size = RandomInt(1, 33);
        RenderTargetCube cube(Device(), size, false, SurfaceFormat::Color, DepthFormat::None);
        std::array<Color, 6> colors{};
        GraphicsDevice& device = Device();
        for (int face = 0; face < 6; ++face)
        {
            colors[static_cast<std::size_t>(face)] = RandomOpaqueColor();
            device.SetRenderTarget(&cube, static_cast<CubeMapFace>(face));
            device.Clear(colors[static_cast<std::size_t>(face)]);
        }
        device.SetRenderTarget(nullptr);

        findings_.CountCheck();
        std::vector<Color> pixels(static_cast<std::size_t>(size) * static_cast<std::size_t>(size));
        for (int face = 0; face < 6; ++face)
        {
            cube.GetData(static_cast<CubeMapFace>(face), pixels.data(), static_cast<int>(pixels.size()));
            const Color expected = colors[static_cast<std::size_t>(face)];
            const auto wrong = std::find_if(pixels.begin(), pixels.end(), [&](const Color& p) { return !Support::Near(p, expected, 0); });
            if (wrong != pixels.end())
            {
                Report(FindingKind::Mismatch, "a RenderTargetCube face does not hold the colour it was cleared to",
                       "size " + std::to_string(size) + ", face " + std::to_string(face) + " reads " +
                           Support::Describe(*wrong) + " for " + Support::Describe(expected));
                return;
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // A full-screen quad with a known XNA result per pixel
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifySolidQuad()
    {
        enum Variant { Opaque, AlphaBlend, Additive, SubViewport, Scissor, Culling, DepthTest, StencilTest, WriteMask };
        const auto variant = static_cast<Variant>(RandomInt(0, 9));
        static constexpr const char* kNames[] = {"Opaque", "AlphaBlend", "Additive", "SubViewport", "Scissor",
                                                 "Culling", "DepthTest", "StencilTest", "WriteMask"};

        const int width = RandomInt(4, 65);
        const int height = RandomInt(4, 65);
        const int multiSample = Chance(4) ? 4 : 0;
        const bool needsDepth = variant == DepthTest || variant == StencilTest;
        RenderTarget2D target(Device(), width, height, false, SurfaceFormat::Color,
                              needsDepth ? DepthFormat::Depth24Stencil8 : DepthFormat::None, multiSample,
                              RenderTargetUsage::DiscardContents);

        const Color background = RandomAnyColor();
        Color quad = RandomAnyColor();
        if (variant != AlphaBlend && variant != Additive)
            quad.setAProperty(255);

        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        device.SetRenderTarget(&target);

        BlendState blend = BlendState::Opaque;
        DepthStencilState depthStencil = DepthStencilState::None;
        RasterizerState rasterizer = RasterizerState::CullNone;
        Rectangle covered(0, 0, width, height);
        bool drawn = true;
        std::string detail;

        const float clearDepth = Chance(2) ? 1.0f : 0.0f;
        const int clearStencil = RandomInt(0, 4);
        device.Clear(ClearOptions::Target | (needsDepth ? ClearOptions::DepthBuffer | ClearOptions::Stencil : ClearOptions::Target),
                     background, clearDepth, clearStencil);

        switch (variant)
        {
            case AlphaBlend:
                blend = BlendState::AlphaBlend;
                break;
            case Additive:
                blend = BlendState::Additive;
                break;
            case SubViewport:
            {
                const int x = RandomInt(0, width);
                const int y = RandomInt(0, height);
                const int w = RandomInt(1, width - x + 1);
                const int h = RandomInt(1, height - y + 1);
                covered = Rectangle(x, y, w, h);
                device.setViewportProperty(Viewport(covered.X, covered.Y, covered.Width, covered.Height));
                detail = ", viewport " + Rect(covered);
                break;
            }
            case Scissor:
            {
                const int x = RandomInt(0, width);
                const int y = RandomInt(0, height);
                const int w = RandomInt(1, width - x + 1);
                const int h = RandomInt(1, height - y + 1);
                covered = Rectangle(x, y, w, h);
                rasterizer = RasterizerState();
                rasterizer.setCullModeProperty(CullMode::None);
                rasterizer.setScissorTestEnableProperty(true);
                device.setScissorRectangleProperty(covered);
                detail = ", scissor " + Rect(covered);
                break;
            }
            case Culling:
            {
                const int mode = RandomInt(0, 3);
                rasterizer = mode == 0 ? RasterizerState::CullNone
                           : mode == 1 ? RasterizerState::CullCounterClockwise
                                       : RasterizerState::CullClockwise;
                // The quad is wound clockwise, XNA's front face.
                drawn = mode != 2;
                detail = mode == 0 ? ", CullNone" : mode == 1 ? ", CullCounterClockwise" : ", CullClockwise";
                break;
            }
            case DepthTest:
                depthStencil = DepthStencilState::Default; // LessEqual
                drawn = clearDepth == 1.0f;                 // the quad sits between the planes
                detail = ", depth cleared to " + std::to_string(clearDepth);
                break;
            case StencilTest:
            {
                const int reference = RandomInt(0, 4);
                depthStencil = DepthStencilState();
                depthStencil.setDepthBufferEnableProperty(false);
                depthStencil.setStencilEnableProperty(true);
                depthStencil.setStencilFunctionProperty(CompareFunction::Equal);
                depthStencil.setReferenceStencilProperty(reference);
                drawn = reference == clearStencil;
                detail = ", stencil cleared to " + std::to_string(clearStencil) + ", Equal " + std::to_string(reference);
                break;
            }
            case WriteMask:
            {
                blend = BlendState();
                blend.setColorWriteChannelsProperty(ColorWriteChannels::Red | ColorWriteChannels::Alpha);
                detail = ", ColorWriteChannels Red|Alpha";
                break;
            }
            default:
                break;
        }

        device.setBlendStateProperty(blend);
        device.setDepthStencilStateProperty(depthStencil);
        device.setRasterizerStateProperty(rasterizer);

        BasicEffect effect(Device());
        effect.VertexColorEnabled = true;
        effect.World = Matrix::getIdentityProperty();
        effect.View = Matrix::getIdentityProperty();
        effect.Projection = Matrix::getIdentityProperty();
        effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
        const std::array<VertexPositionColor, 6> vertices = FullScreenQuad(quad, 0.5f);
        device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, 2);
        device.SetRenderTarget(nullptr);
        device.setBlendStateProperty(BlendState::Opaque);
        device.setDepthStencilStateProperty(DepthStencilState::Default);
        device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

        // Expected colour where the quad lands.
        const auto channel = [](int value) { return std::clamp(value, 0, 255); };
        Color inside = quad;
        const int alpha = quad.getAProperty();
        switch (variant)
        {
            case AlphaBlend: // premultiplied: src + dst * (1 - srcA)
                inside = Color(channel(quad.getRProperty() + background.getRProperty() * (255 - alpha) / 255),
                               channel(quad.getGProperty() + background.getGProperty() * (255 - alpha) / 255),
                               channel(quad.getBProperty() + background.getBProperty() * (255 - alpha) / 255),
                               channel(alpha + background.getAProperty() * (255 - alpha) / 255));
                break;
            case Additive: // src * srcA + dst
                inside = Color(channel(quad.getRProperty() * alpha / 255 + background.getRProperty()),
                               channel(quad.getGProperty() * alpha / 255 + background.getGProperty()),
                               channel(quad.getBProperty() * alpha / 255 + background.getBProperty()),
                               channel(alpha * alpha / 255 + background.getAProperty()));
                break;
            case WriteMask:
                inside = Color(quad.getRProperty(), background.getGProperty(), background.getBProperty(), quad.getAProperty());
                break;
            default:
                break;
        }
        const int tolerance = (variant == AlphaBlend || variant == Additive) ? 2 : 1;

        findings_.CountCheck();
        const std::vector<Color> pixels = ReadColors(target);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const bool inCover = drawn && covered.Contains(x, y);
                const Color& want = inCover ? inside : background;
                const Color& got = pixels[static_cast<std::size_t>(y) * width + x];
                if (!Support::Near(got, want, tolerance))
                {
                    Report(FindingKind::Mismatch,
                           std::string("a full-screen quad (") + kNames[variant] + ") does not produce XNA's pixels" +
                               (multiSample ? " with MSAA" : ""),
                           Size(width, height) + detail + ", pixel (" + std::to_string(x) + "," + std::to_string(y) +
                               ") reads " + Support::Describe(got) + " for " + Support::Describe(want) + " (quad " +
                               Support::Describe(quad) + " over " + Support::Describe(background) + ")");
                    return;
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // SpriteBatch placement and texture orientation
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifySpriteFill()
    {
        // A 2x2 texture with four distinct colours, drawn scaled into a rectangle with point
        // sampling: each quadrant must come out in the right corner, flipped exactly as the
        // SpriteEffects say. The source is sometimes a render target, the texture kind most
        // often stored upside down by a GL backend.
        const std::array<Color, 4> quadrants{Color(255, 0, 0, 255), Color(0, 255, 0, 255), Color(0, 0, 255, 255),
                                             Color(255, 255, 0, 255)}; // TL, TR, BL, BR
        const bool fromRenderTarget = Chance(2);
        std::unique_ptr<Texture2D> texture;
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        if (fromRenderTarget)
        {
            auto source = std::make_unique<RenderTarget2D>(device, 2, 2);
            // Draw the four quadrants into the target with scissored clears... XNA's Clear ignores
            // the scissor, so fill it through SetData instead, which a render target also accepts.
            source->SetData(quadrants.data(), 4);
            texture = std::move(source);
        }
        else
        {
            texture = std::make_unique<Texture2D>(device, 2, 2);
            texture->SetData(quadrants.data(), 4);
        }

        const int width = RandomInt(4, 65);
        const int height = RandomInt(4, 65);
        const int half = RandomInt(1, std::min(width, height) / 2 + 1);
        const int x = RandomInt(0, width - 2 * half + 1);
        const int y = RandomInt(0, height - 2 * half + 1);
        const Rectangle destination(x, y, 2 * half, 2 * half);
        const int flip = RandomInt(0, 4);
        const auto effects = static_cast<SpriteEffects>(flip);
        const Color background = RandomOpaqueColor();
        const std::array<SpriteSortMode, 4> sortModes{SpriteSortMode::Deferred, SpriteSortMode::Immediate,
                                                      SpriteSortMode::Texture, SpriteSortMode::BackToFront};
        const SpriteSortMode sortMode = sortModes[static_cast<std::size_t>(RandomInt(0, 4))];

        RenderTarget2D target(device, width, height);
        device.SetRenderTarget(&target);
        device.Clear(background);
        SpriteBatch batch(device);
        const SamplerState sampler = SamplerState::PointClamp;
        const DepthStencilState depth = DepthStencilState::None;
        const RasterizerState raster = RasterizerState::CullCounterClockwise;
        batch.Begin(sortMode, BlendState::Opaque, &sampler, &depth, &raster);
        batch.Draw(*texture, destination, std::nullopt, Color::White, 0.0f, Vector2::Zero, effects, 0.0f);
        batch.End();
        device.SetRenderTarget(nullptr);

        findings_.CountCheck();
        const std::vector<Color> pixels = ReadColors(target);
        for (int py = 0; py < height; ++py)
        {
            for (int px = 0; px < width; ++px)
            {
                Color want = background;
                if (destination.Contains(px, py))
                {
                    int column = (px - x) < half ? 0 : 1;
                    int row = (py - y) < half ? 0 : 1;
                    if (flip & 1) // FlipHorizontally
                        column = 1 - column;
                    if (flip & 2) // FlipVertically
                        row = 1 - row;
                    want = quadrants[static_cast<std::size_t>(row * 2 + column)];
                }
                const Color& got = pixels[static_cast<std::size_t>(py) * width + px];
                if (!Support::Near(got, want, 1))
                {
                    Report(FindingKind::Mismatch,
                           std::string("a sprite from a ") + (fromRenderTarget ? "render target" : "Texture2D") +
                               " lands wrongly or with the wrong orientation",
                           Size(width, height) + " target, destination " + Rect(destination) + ", SpriteEffects " +
                               std::to_string(flip) + ", pixel (" + std::to_string(px) + "," + std::to_string(py) +
                               ") reads " + Support::Describe(got) + " for " + Support::Describe(want));
                    return;
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // Vertex and index buffers
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifyBufferRoundTrip()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        const int count = RandomInt(1, 300);

        if (Chance(2))
        {
            // Vertices: fill, then overwrite a window at a byte offset, then read everything.
            const bool dynamic = Chance(2);
            std::unique_ptr<VertexBuffer> buffer;
            if (dynamic)
                buffer = std::make_unique<DynamicVertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                               count, BufferUsage::None);
            else
                buffer = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(), count,
                                                        BufferUsage::None);

            const auto randomVertex = [&] {
                const Vector3 position{RandomFloat(-1000, 1000), RandomFloat(-1000, 1000), RandomFloat(-1, 1)};
                const Color color = RandomAnyColor();
                return VertexPositionColor(position, color);
            };
            std::vector<VertexPositionColor> expected;
            for (int i = 0; i < count; ++i)
                expected.push_back(randomVertex());
            buffer->SetData(expected.data(), count);

            const int first = RandomInt(0, count);
            const int length = RandomInt(1, count - first + 1);
            const int startIndex = RandomInt(0, 5);
            std::vector<VertexPositionColor> patch(static_cast<std::size_t>(startIndex + length));
            for (VertexPositionColor& vertex : patch)
                vertex = randomVertex();
            constexpr int kStride = 16; // VertexPositionColor's declaration stride
            if (dynamic && Chance(2))
                static_cast<DynamicVertexBuffer&>(*buffer).SetData(first * kStride, patch.data(), startIndex, length,
                                                                    kStride, SetDataOptions::NoOverwrite);
            else
                buffer->SetData(first * kStride, patch.data(), startIndex, length, kStride);
            for (int i = 0; i < length; ++i)
                expected[static_cast<std::size_t>(first + i)] = patch[static_cast<std::size_t>(startIndex + i)];

            std::vector<VertexPositionColor> readBack(static_cast<std::size_t>(count));
            buffer->GetData(readBack.data(), count);
            findings_.CountCheck();
            for (int i = 0; i < count; ++i)
            {
                const VertexPositionColor& a = readBack[static_cast<std::size_t>(i)];
                const VertexPositionColor& b = expected[static_cast<std::size_t>(i)];
                if (a.Position.X != b.Position.X || a.Position.Y != b.Position.Y || a.Position.Z != b.Position.Z ||
                    a.Color.getPackedValueProperty() != b.Color.getPackedValueProperty())
                {
                    Report(FindingKind::Mismatch,
                           std::string(dynamic ? "a DynamicVertexBuffer" : "a VertexBuffer") +
                               " does not read back what SetData wrote at a byte offset",
                           std::to_string(count) + " vertices, window " + std::to_string(first) + "+" +
                               std::to_string(length) + " from startIndex " + std::to_string(startIndex) +
                               ", first difference at vertex " + std::to_string(i));
                    return;
                }
            }
            return;
        }

        // Indices, 16 or 32 bits.
        const bool wide = Chance(2);
        const bool dynamic = Chance(2);
        const IndexElementSize size = wide ? IndexElementSize::ThirtyTwoBits : IndexElementSize::SixteenBits;
        std::unique_ptr<IndexBuffer> buffer;
        if (dynamic)
            buffer = std::make_unique<DynamicIndexBuffer>(device, size, count, BufferUsage::None);
        else
            buffer = std::make_unique<IndexBuffer>(device, size, count, BufferUsage::None);

        const int first = RandomInt(0, count);
        const int length = RandomInt(1, count - first + 1);
        const auto check = [&](auto zero) {
            using Index = decltype(zero);
            std::vector<Index> expected(static_cast<std::size_t>(count));
            for (Index& index : expected)
                index = static_cast<Index>(RandomInt(0, 1 << 30));
            buffer->SetData(expected.data(), count);
            std::vector<Index> patch(static_cast<std::size_t>(length));
            for (Index& index : patch)
                index = static_cast<Index>(RandomInt(0, 1 << 30));
            buffer->SetData(first * static_cast<int>(sizeof(Index)), patch.data(), 0, length);
            std::copy(patch.begin(), patch.end(), expected.begin() + first);

            std::vector<Index> readBack(static_cast<std::size_t>(count));
            buffer->GetData(readBack.data(), count);
            findings_.CountCheck();
            const auto mismatch = std::mismatch(expected.begin(), expected.end(), readBack.begin());
            if (mismatch.first != expected.end())
            {
                Report(FindingKind::Mismatch,
                       std::string(dynamic ? "a DynamicIndexBuffer" : "an IndexBuffer") + " of " +
                           (wide ? "32" : "16") + "-bit indices does not read back what SetData wrote at a byte offset",
                       std::to_string(count) + " indices, window " + std::to_string(first) + "+" +
                           std::to_string(length) + ", first difference at index " +
                           std::to_string(mismatch.first - expected.begin()));
            }
        };
        if (wide)
            check(std::uint32_t{});
        else
            check(std::uint16_t{});
    }

    // -----------------------------------------------------------------------------------------
    // The back buffer, inside Draw()
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionVerifyBackBuffer()
    {
        const Color color = RandomOpaqueColor();
        const int pick = RandomInt(0, 1 << 16);
        QueueDrawCheck("VerifyBackBuffer", [this, color, pick] {
            GraphicsDevice& device = Device();
            const PresentationParameters& pp = device.getPresentationParametersProperty();
            if (pp.getBackBufferFormatProperty() != SurfaceFormat::Color)
                return;
            const int width = pp.getBackBufferWidthProperty();
            const int height = pp.getBackBufferHeightProperty();
            device.Clear(color);

            // A small rectangle somewhere in the back buffer, read back as Color.
            const int w = 1 + pick % std::min(width, 16);
            const int h = 1 + (pick / 16) % std::min(height, 16);
            const Rectangle rect((pick * 7) % std::max(1, width - w + 1), (pick * 13) % std::max(1, height - h + 1), w, h);
            std::vector<Color> pixels(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
            device.GetBackBufferData(&rect, pixels.data(), 0, static_cast<int>(pixels.size()));
            findings_.CountCheck();
            const auto wrong = std::find_if(pixels.begin(), pixels.end(), [&](const Color& p) { return !Support::Near(p, color, 0); });
            if (wrong != pixels.end())
            {
                Report(FindingKind::Mismatch, "GetBackBufferData does not return the colour the back buffer was cleared to",
                       Size(width, height) + " back buffer, rectangle " + Rect(rect) + " reads " +
                           Support::Describe(*wrong) + " for " + Support::Describe(color));
            }
        });
    }
}
