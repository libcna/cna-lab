// SPDX-License-Identifier: MIT
// Drawing chaos: every primitive type and draw path, every stock effect with hostile parameters,
// SpriteBatch in every sort mode, instancing, occlusion queries, cube and volume textures.
// These count on a crash or an unexpected exception to speak for themselves.
#include "ChaosEngine.hpp"

#include <array>
#include <cmath>
#include <limits>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/AlphaTestEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DualTextureEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EnvironmentMapEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/OcclusionQuery.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBufferBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"

#include "ChaosSupport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace CnaKiller
{
    namespace
    {
        const float kNaN = std::numeric_limits<float>::quiet_NaN();

        int VerticesFor(PrimitiveType type, int primitives)
        {
            switch (type)
            {
                case PrimitiveType::TriangleList: return primitives * 3;
                case PrimitiveType::TriangleStrip: return primitives + 2;
                case PrimitiveType::LineList: return primitives * 2;
                case PrimitiveType::LineStrip: return primitives + 1;
            }
            return primitives * 3;
        }

        /** Plain vertex structures with explicit declarations, for layouts no stock type has. */
        struct DualTextureVertex
        {
            float position[3];
            float uv0[2];
            float uv1[2];
        };

        struct SkinnedVertex
        {
            float position[3];
            float normal[3];
            float uv[2];
            std::uint8_t indices[4];
            float weights[4];
        };

        const VertexDeclaration& DualTextureDeclaration()
        {
            static const VertexDeclaration declaration{
                VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
                VertexElement(12, VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
                VertexElement(20, VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 1)};
            return declaration;
        }

        const VertexDeclaration& SkinnedDeclaration()
        {
            static const VertexDeclaration declaration{
                VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
                VertexElement(12, VertexElementFormat::Vector3, VertexElementUsage::Normal, 0),
                VertexElement(24, VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
                VertexElement(32, VertexElementFormat::Byte4, VertexElementUsage::BlendIndices, 0),
                VertexElement(36, VertexElementFormat::Vector4, VertexElementUsage::BlendWeight, 0)};
            return declaration;
        }
    }

    // -----------------------------------------------------------------------------------------
    // Every primitive type through every draw path
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionDrawPrimitiveTypes()
    {
        static constexpr std::array<PrimitiveType, 4> kTypes{PrimitiveType::TriangleList, PrimitiveType::TriangleStrip,
                                                             PrimitiveType::LineList, PrimitiveType::LineStrip};
        const PrimitiveType type = kTypes[static_cast<std::size_t>(RandomInt(0, 4))];
        const int primitives = Chance(10) ? RandomInt(1, 200000) : RandomInt(1, 2000);
        const int path = RandomInt(0, 5);
        const bool poison = Chance(8); // NaN and infinite positions

        GraphicsDevice& device = Device();
        const int needed = VerticesFor(type, primitives);
        // 16-bit indices address at most 65536 vertices.
        const bool wideIndices = path != 2 || needed > 65535;
        const int offset = RandomInt(0, 8);
        std::vector<VertexPositionColor> vertices(static_cast<std::size_t>(needed + offset));
        const std::vector<std::uint8_t> noise = RandomBytes(vertices.size() * 8);
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            const auto coordinate = [&](std::size_t byte) { return noise[i * 8 + byte] / 64.0f - 2.0f; };
            vertices[i] = VertexPositionColor(Vector3(coordinate(0), coordinate(1), noise[i * 8 + 2] / 255.0f),
                                              Color(noise[i * 8 + 3], noise[i * 8 + 4], noise[i * 8 + 5], noise[i * 8 + 6]));
            if (poison && noise[i * 8 + 7] < 16)
                vertices[i].Position.X = noise[i * 8 + 7] < 8 ? kNaN : std::numeric_limits<float>::infinity();
        }

        std::unique_ptr<RenderTarget2D> target;
        if (Chance(2))
        {
            const int width = RandomInt(1, 257);
            const int height = RandomInt(1, 257);
            target = std::make_unique<RenderTarget2D>(device, width, height);
            device.SetRenderTarget(target.get());
            device.Clear(RandomOpaqueColor());
        }
        BasicEffect effect(device);
        effect.VertexColorEnabled = true;
        effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();

        switch (path)
        {
            case 0:
                device.DrawUserPrimitives(type, vertices.data(), offset, primitives);
                break;
            case 1:
            case 2:
            {
                std::vector<std::uint32_t> wide(static_cast<std::size_t>(needed + offset));
                for (std::size_t i = 0; i < wide.size(); ++i)
                    wide[i] = static_cast<std::uint32_t>((i * 7919u) % static_cast<std::size_t>(needed));
                if (wideIndices)
                {
                    device.DrawUserIndexedPrimitives(type, vertices.data(), offset, needed, wide.data(), offset, primitives);
                }
                else
                {
                    std::vector<std::uint16_t> narrow(wide.begin(), wide.end());
                    device.DrawUserIndexedPrimitives(type, vertices.data(), offset, needed, narrow.data(), offset, primitives);
                }
                break;
            }
            case 3:
            {
                VertexBuffer buffer(device, VertexPositionColor::getVertexDeclarationStatic(),
                                    static_cast<int>(vertices.size()), BufferUsage::WriteOnly);
                Support::UnbindAll(device);
                buffer.SetData(vertices.data(), static_cast<int>(vertices.size()));
                device.SetVertexBuffer(&buffer, offset);
                device.DrawPrimitives(type, 0, primitives);
                device.SetVertexBuffer(nullptr);
                break;
            }
            default:
            {
                VertexBuffer buffer(device, VertexPositionColor::getVertexDeclarationStatic(),
                                    static_cast<int>(vertices.size()), BufferUsage::None);
                IndexBuffer indices(device, IndexElementSize::ThirtyTwoBits, needed, BufferUsage::WriteOnly);
                std::vector<std::uint32_t> data(static_cast<std::size_t>(needed));
                for (std::size_t i = 0; i < data.size(); ++i)
                    data[i] = static_cast<std::uint32_t>(i);
                Support::UnbindAll(device);
                buffer.SetData(vertices.data(), static_cast<int>(vertices.size()));
                indices.SetData(data.data(), needed);
                device.SetVertexBuffer(&buffer);
                device.SetIndexBuffer(&indices);
                device.DrawIndexedPrimitives(type, offset, 0, needed, 0, primitives);
                device.SetIndexBuffer(nullptr);
                device.SetVertexBuffer(nullptr);
                break;
            }
        }
        if (target)
            device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // Every stock effect, with hostile parameters
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionDrawStockEffects()
    {
        GraphicsDevice& device = Device();
        const int which = RandomInt(0, 5);
        const int quads = RandomInt(1, 64);
        const bool hostile = Chance(6);

        // World/View/Projection: a real camera, or NaN, or a degenerate projection.
        const float angle = RandomFloat(0, MathHelper::TwoPi);
        const float scale = RandomFloat(0.1f, 3.0f);
        Matrix world = Matrix::CreateRotationY(angle) * Matrix::CreateScale(scale);
        Matrix view = Matrix::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::Up);
        Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 1.5f, 0.1f, 100.0f);
        if (hostile)
        {
            switch (RandomInt(0, 3))
            {
                case 0: world.M11 = kNaN; break;
                case 1: projection = Matrix(); break; // all zero
                default: view.M43 = std::numeric_limits<float>::infinity(); break;
            }
        }
        const Texture2D& texture = textures_.Empty() ? WhiteTexture() : textures_.RandomItem(random_);
        Texture2D* texturePointer = const_cast<Texture2D*>(&texture);

        std::unique_ptr<RenderTarget2D> target;
        if (Chance(2))
        {
            const int width = RandomInt(1, 200);
            const int height = RandomInt(1, 200);
            target = std::make_unique<RenderTarget2D>(device, width, height, false, SurfaceFormat::Color,
                                                      DepthFormat::Depth24, 0, RenderTargetUsage::DiscardContents);
            device.SetRenderTarget(target.get());
            device.Clear(RandomOpaqueColor());
        }
        device.setDepthStencilStateProperty(Chance(2) ? DepthStencilState::Default : DepthStencilState::None);
        device.setRasterizerStateProperty(RasterizerState::CullNone);
        device.getSamplerStatesProperty()[0] = Chance(2) ? SamplerState::LinearWrap : SamplerState::PointClamp;

        const auto grid = [&](auto&& makeVertex) {
            using Vertex = decltype(makeVertex(0.0f, 0.0f));
            std::vector<Vertex> vertices;
            for (int q = 0; q < quads; ++q)
            {
                const float x = RandomFloat(-2, 2);
                const float y = RandomFloat(-2, 2);
                const float s = RandomFloat(0.05f, 1.0f);
                for (const auto& [dx, dy] : std::array<std::pair<float, float>, 6>{
                         {{0, 0}, {s, 0}, {0, s}, {s, 0}, {s, s}, {0, s}}})
                    vertices.push_back(makeVertex(x + dx, y + dy));
            }
            return vertices;
        };

        switch (which)
        {
            case 0:
            {
                BasicEffect effect(device);
                effect.setWorldProperty(world);
                effect.setViewProperty(view);
                effect.setProjectionProperty(projection);
                effect.setTextureEnabledProperty(Chance(2));
                effect.setTextureProperty(texturePointer);
                if (Chance(2))
                {
                    effect.EnableDefaultLighting();
                    effect.setPreferPerPixelLightingProperty(Chance(2));
                    effect.setSpecularPowerProperty(hostile ? -4.0f : RandomFloat(1, 64));
                }
                effect.setFogEnabledProperty(Chance(3));
                effect.setFogStartProperty(RandomFloat(-5, 5));
                effect.setFogEndProperty(RandomFloat(-5, 10)); // may equal or precede the start
                effect.setAlphaProperty(RandomFloat(0, 1));
                const auto vertices = grid([](float x, float y) {
                    return VertexPositionNormalTexture(Vector3(x, y, 0), Vector3(0, 0, 1), Vector2(x, y));
                });
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, quads * 2);
                break;
            }
            case 1:
            {
                AlphaTestEffect effect(device);
                effect.setWorldProperty(world);
                effect.setViewProperty(view);
                effect.setProjectionProperty(projection);
                effect.setTextureProperty(texturePointer);
                effect.setAlphaFunctionProperty(static_cast<CompareFunction>(RandomInt(0, 8)));
                effect.setReferenceAlphaProperty(RandomInt(0, 256));
                effect.setVertexColorEnabledProperty(true);
                const auto vertices = grid([&](float x, float y) {
                    return VertexPositionColorTexture(Vector3(x, y, 0), Color::White, Vector2(x, y));
                });
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, quads * 2);
                break;
            }
            case 2:
            {
                DualTextureEffect effect(device);
                effect.setWorldProperty(world);
                effect.setViewProperty(view);
                effect.setProjectionProperty(projection);
                effect.setTextureProperty(texturePointer);
                effect.setTexture2Property(&WhiteTexture());
                const auto vertices = grid([](float x, float y) {
                    return DualTextureVertex{{x, y, 0}, {x, y}, {y, x}};
                });
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, quads * 2,
                                          DualTextureDeclaration());
                break;
            }
            case 3:
            {
                const int size = RandomInt(1, 17);
                TextureCube cube(device, size, Chance(2), SurfaceFormat::Color);
                std::vector<Color> face(static_cast<std::size_t>(size) * size, RandomOpaqueColor());
                for (int f = 0; f < 6; ++f)
                    cube.SetData(static_cast<CubeMapFace>(f), face.data(), static_cast<int>(face.size()));
                EnvironmentMapEffect effect(device);
                effect.setWorldProperty(world);
                effect.setViewProperty(view);
                effect.setProjectionProperty(projection);
                effect.setTextureProperty(texturePointer);
                effect.setEnvironmentMapProperty(&cube);
                effect.setEnvironmentMapAmountProperty(RandomFloat(-1, 2));
                effect.setFresnelFactorProperty(hostile ? kNaN : RandomFloat(0, 4));
                effect.EnableDefaultLighting();
                const auto vertices = grid([](float x, float y) {
                    return VertexPositionNormalTexture(Vector3(x, y, 0), Vector3(0, 0, 1), Vector2(x, y));
                });
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, quads * 2);
                break;
            }
            default:
            {
                SkinnedEffect effect(device);
                effect.setWorldProperty(world);
                effect.setViewProperty(view);
                effect.setProjectionProperty(projection);
                effect.setTextureProperty(texturePointer);
                static constexpr std::array<int, 3> kWeights{1, 2, 4};
                effect.setWeightsPerVertexProperty(kWeights[static_cast<std::size_t>(RandomInt(0, 3))]);
                const int boneCount = RandomInt(1, 73); // SkinnedEffect.MaxBones is 72
                std::vector<Matrix> bones(static_cast<std::size_t>(boneCount));
                for (Matrix& bone : bones)
                {
                    const float x = RandomFloat(-1, 1);
                    const float y = RandomFloat(-1, 1);
                    bone = Matrix::CreateTranslation(x, y, 0);
                }
                effect.SetBoneTransforms(bones);
                if (Chance(2))
                    effect.EnableDefaultLighting();
                const auto vertices = grid([&](float x, float y) {
                    SkinnedVertex vertex{{x, y, 0}, {0, 0, 1}, {x, y}, {}, {0.25f, 0.25f, 0.25f, 0.25f}};
                    for (std::uint8_t& index : vertex.indices)
                        index = static_cast<std::uint8_t>(RandomInt(0, boneCount));
                    return vertex;
                });
                effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, quads * 2,
                                          SkinnedDeclaration());
                break;
            }
        }
        if (target)
            device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // SpriteBatch in every sort mode, with hostile sprites
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionSpriteBatchChaos()
    {
        GraphicsDevice& device = Device();
        static constexpr std::array<SpriteSortMode, 5> kModes{SpriteSortMode::Deferred, SpriteSortMode::Immediate,
                                                              SpriteSortMode::Texture, SpriteSortMode::BackToFront,
                                                              SpriteSortMode::FrontToBack};
        const std::array<BlendState, 4> blends{BlendState::Opaque, BlendState::AlphaBlend, BlendState::Additive,
                                               BlendState::NonPremultiplied};
        const std::array<SamplerState, 6> samplers{SamplerState::PointClamp,  SamplerState::PointWrap,
                                                   SamplerState::LinearClamp, SamplerState::LinearWrap,
                                                   SamplerState::AnisotropicClamp, SamplerState::AnisotropicWrap};
        const SpriteSortMode mode = kModes[static_cast<std::size_t>(RandomInt(0, 5))];
        const BlendState blend = blends[static_cast<std::size_t>(RandomInt(0, 4))];
        const SamplerState sampler = samplers[static_cast<std::size_t>(RandomInt(0, 6))];
        const DepthStencilState depth = Chance(2) ? DepthStencilState::None : DepthStencilState::DepthRead;
        RasterizerState raster = RasterizerState::CullCounterClockwise;
        if (Chance(4))
        {
            raster = RasterizerState();
            raster.setScissorTestEnableProperty(true);
        }

        Matrix transform = Matrix::getIdentityProperty();
        switch (RandomInt(0, 6))
        {
            case 0: transform = Matrix::CreateScale(RandomFloat(-3, 3)); break;
            case 1: transform = Matrix::CreateRotationZ(RandomFloat(0, MathHelper::TwoPi)); break;
            case 2: transform.M11 = kNaN; break;
            default: break;
        }

        std::unique_ptr<RenderTarget2D> target;
        if (Chance(2))
        {
            const int width = RandomInt(1, 300);
            const int height = RandomInt(1, 300);
            target = std::make_unique<RenderTarget2D>(device, width, height);
            device.SetRenderTarget(target.get());
            device.Clear(RandomOpaqueColor());
        }

        std::unique_ptr<BasicEffect> effect;
        if (Chance(4))
        {
            effect = std::make_unique<BasicEffect>(device);
            effect->setTextureEnabledProperty(true);
            effect->VertexColorEnabled = true;
            effect->Projection = Matrix::CreateOrthographicOffCenter(0, 800, 600, 0, 0, 1);
        }

        // Sprite sources: pooled textures, render targets not currently set, and the white texel.
        std::vector<const Texture2D*> sources{&WhiteTexture()};
        for (std::size_t i = 0; i < textures_.Size() && sources.size() < 16; ++i)
            sources.push_back(&textures_.At(i));
        for (std::size_t i = 0; i < renderTargets_.Size() && sources.size() < 24; ++i)
            sources.push_back(&renderTargets_.At(i));

        SpriteBatch batch(device);
        batch.Begin(mode, blend, &sampler, &depth, &raster, effect.get(), transform);
        const int sprites = Chance(20) ? RandomInt(1, 20000) : RandomInt(1, 500);
        for (int i = 0; i < sprites; ++i)
        {
            const Texture2D& texture = *sources[static_cast<std::size_t>(RandomInt(0, static_cast<int>(sources.size())))];
            const float x = RandomFloat(-100, 1000);
            const float y = RandomFloat(-100, 800);
            const Color tint = RandomAnyColor();
            std::optional<Rectangle> source;
            if (Chance(3))
            {
                // Sometimes outside the texture, sometimes with a negative size.
                const int sx = RandomInt(-8, texture.getWidthProperty() + 8);
                const int sy = RandomInt(-8, texture.getHeightProperty() + 8);
                const int sw = RandomInt(-4, texture.getWidthProperty() + 8);
                const int sh = RandomInt(-4, texture.getHeightProperty() + 8);
                source = Rectangle(sx, sy, sw, sh);
            }
            const float rotation = Chance(50) ? kNaN : RandomFloat(-10, 10);
            const float originX = RandomFloat(-50, 50);
            const float originY = RandomFloat(-50, 50);
            const float layer = RandomFloat(-1, 2);
            const auto effects = static_cast<SpriteEffects>(RandomInt(0, 4));
            switch (RandomInt(0, 5))
            {
                case 0:
                    batch.Draw(texture, Vector2(x, y), tint);
                    break;
                case 1:
                {
                    const int w = RandomInt(-10, 300);
                    const int h = RandomInt(-10, 300);
                    batch.Draw(texture, Rectangle(static_cast<int>(x), static_cast<int>(y), w, h), tint);
                    break;
                }
                case 2:
                    batch.Draw(texture, Vector2(x, y), source, tint, rotation, Vector2(originX, originY),
                               RandomFloat(-2, 4), effects, layer);
                    break;
                case 3:
                {
                    const float sx = RandomFloat(-3, 3);
                    const float sy = RandomFloat(-3, 3);
                    batch.Draw(texture, Vector2(x, y), source, tint, rotation, Vector2(originX, originY),
                               Vector2(sx, sy), effects, layer);
                    break;
                }
                default:
                {
                    const int w = RandomInt(0, 300);
                    const int h = RandomInt(0, 300);
                    batch.Draw(texture, Rectangle(static_cast<int>(x), static_cast<int>(y), w, h), source, tint,
                               rotation, Vector2(originX, originY), effects, layer);
                    break;
                }
            }
        }
        batch.End();
        if (target)
            device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // Occlusion queries
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionOcclusionQueries()
    {
        GraphicsDevice& device = Device();
        const int size = RandomInt(1, 65);
        RenderTarget2D target(device, size, size, false, SurfaceFormat::Color, DepthFormat::Depth24, 0,
                              RenderTargetUsage::DiscardContents);
        device.SetRenderTarget(&target);
        device.Clear(Color::Black);
        device.setDepthStencilStateProperty(DepthStencilState::Default);
        device.setRasterizerStateProperty(RasterizerState::CullNone);

        auto query = std::make_unique<OcclusionQuery>(device);
        BasicEffect effect(device);
        effect.VertexColorEnabled = true;
        effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
        const std::array<VertexPositionColor, 6> quad{
            VertexPositionColor(Vector3(-1, 1, 0.5f), Color::White), VertexPositionColor(Vector3(1, 1, 0.5f), Color::White),
            VertexPositionColor(Vector3(-1, -1, 0.5f), Color::White), VertexPositionColor(Vector3(1, 1, 0.5f), Color::White),
            VertexPositionColor(Vector3(1, -1, 0.5f), Color::White), VertexPositionColor(Vector3(-1, -1, 0.5f), Color::White)};

        query->Begin();
        device.DrawUserPrimitives(PrimitiveType::TriangleList, quad.data(), 0, 2);
        if (Chance(5))
        {
            // Destroyed while still between Begin and End.
            query.reset();
        }
        else
        {
            query->End();
            bool complete = false;
            for (int i = 0; i < 2000 && !complete; ++i)
                complete = query->getIsCompleteProperty();
            if (complete)
            {
                const int pixels = query->getPixelCountProperty();
                findings_.CountCheck();
                if (pixels < 0 || (query->isPixelCountPreciseEXT() && pixels != size * size))
                {
                    Report(FindingKind::Mismatch, "an OcclusionQuery around a full-target quad reports the wrong pixel count",
                           std::to_string(size) + "x" + std::to_string(size) + " target, PixelCount " +
                               std::to_string(pixels));
                }
            }
        }
        device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // Instancing
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionDrawInstanced()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        const int instances = RandomInt(1, 2000);
        VertexBuffer geometry(device, VertexPositionColor::getVertexDeclarationStatic(), 3, BufferUsage::None);
        const std::array<VertexPositionColor, 3> triangle{
            VertexPositionColor{Vector3(-0.1f, 0.1f, 0), RandomOpaqueColor()},
            VertexPositionColor{Vector3(0.1f, 0.1f, 0), RandomOpaqueColor()},
            VertexPositionColor{Vector3(-0.1f, -0.1f, 0), RandomOpaqueColor()}};
        geometry.SetData(triangle.data(), 3);

        static const VertexDeclaration instanceDeclaration{
            VertexElement(0, VertexElementFormat::Vector4, VertexElementUsage::TextureCoordinate, 1)};
        VertexBuffer perInstance(device, instanceDeclaration, instances, BufferUsage::None);
        std::vector<Vector4> offsets(static_cast<std::size_t>(instances));
        for (Vector4& offset : offsets)
        {
            const float x = RandomFloat(-1, 1);
            const float y = RandomFloat(-1, 1);
            offset = Vector4(x, y, 0, 0);
        }
        perInstance.SetData(0, offsets.data(), 0, instances, 16);

        IndexBuffer indices(device, IndexElementSize::SixteenBits, 3, BufferUsage::None);
        const std::array<std::uint16_t, 3> order{0, 1, 2};
        indices.SetData(order.data(), 3);

        device.SetVertexBuffers({VertexBufferBinding(&geometry, 0, 0), VertexBufferBinding(&perInstance, 0, 1)});
        device.SetIndexBuffer(&indices);
        BasicEffect effect(device);
        effect.VertexColorEnabled = true;
        effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
        device.DrawInstancedPrimitives(PrimitiveType::TriangleList, 0, 0, 3, 0, 1, instances);
        Support::UnbindAll(device);
    }

    // -----------------------------------------------------------------------------------------
    // Cube and volume textures
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionCubeAndVolumeTextures()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        if (Chance(2))
        {
            const int size = RandomInt(1, 65);
            TextureCube cube(device, size, Chance(2), SurfaceFormat::Color);
            const auto face = static_cast<CubeMapFace>(RandomInt(0, 6));
            std::vector<Color> pixels(static_cast<std::size_t>(size) * size);
            const std::vector<std::uint8_t> noise = RandomBytes(pixels.size() * 4);
            for (std::size_t i = 0; i < pixels.size(); ++i)
                pixels[i] = Color(noise[i * 4], noise[i * 4 + 1], noise[i * 4 + 2], noise[i * 4 + 3]);
            cube.SetData(face, pixels.data(), static_cast<int>(pixels.size()));
            std::vector<Color> readBack(pixels.size());
            cube.GetData(face, readBack.data(), static_cast<int>(readBack.size()));
            findings_.CountCheck();
            for (std::size_t i = 0; i < pixels.size(); ++i)
            {
                if (readBack[i].getPackedValueProperty() != pixels[i].getPackedValueProperty())
                {
                    Report(FindingKind::Mismatch, "a TextureCube face does not read back what SetData wrote",
                           "size " + std::to_string(size) + ", face " + std::to_string(static_cast<int>(face)) +
                               ", first difference at texel " + std::to_string(i));
                    break;
                }
            }
            return;
        }

        const int width = RandomInt(1, 33);
        const int height = RandomInt(1, 33);
        const int depth = RandomInt(1, 17);
        Texture3D volume(device, width, height, depth, Chance(2), SurfaceFormat::Color);
        std::vector<Color> voxels(static_cast<std::size_t>(width) * height * depth);
        const std::vector<std::uint8_t> noise = RandomBytes(voxels.size() * 4);
        for (std::size_t i = 0; i < voxels.size(); ++i)
            voxels[i] = Color(noise[i * 4], noise[i * 4 + 1], noise[i * 4 + 2], noise[i * 4 + 3]);
        volume.SetData(voxels.data(), static_cast<int>(voxels.size()));

        // Read back one box.
        const int left = RandomInt(0, width);
        const int top = RandomInt(0, height);
        const int front = RandomInt(0, depth);
        const int right = RandomInt(left + 1, width + 1);
        const int bottom = RandomInt(top + 1, height + 1);
        const int back = RandomInt(front + 1, depth + 1);
        std::vector<Color> box(static_cast<std::size_t>(right - left) * (bottom - top) * (back - front));
        volume.GetData(0, left, top, right, bottom, front, back, box.data(), 0, static_cast<int>(box.size()));
        findings_.CountCheck();
        std::size_t k = 0;
        for (int z = front; z < back; ++z)
        {
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x, ++k)
                {
                    const Color& want = voxels[(static_cast<std::size_t>(z) * height + y) * width + x];
                    if (box[k].getPackedValueProperty() != want.getPackedValueProperty())
                    {
                        Report(FindingKind::Mismatch, "a Texture3D box does not read back what SetData wrote",
                               std::to_string(width) + "x" + std::to_string(height) + "x" + std::to_string(depth) +
                                   ", box " + std::to_string(left) + ".." + std::to_string(right) + "," +
                                   std::to_string(top) + ".." + std::to_string(bottom) + "," + std::to_string(front) +
                                   ".." + std::to_string(back) + ", first difference at voxel (" + std::to_string(x) +
                                   "," + std::to_string(y) + "," + std::to_string(z) + ")");
                        return;
                    }
                }
            }
        }
    }
}
