// SPDX-License-Identifier: MIT
// Calls XNA refuses, each expecting the exception XNA 4.0 throws. Every rule here was read from
// the XNA 4.0 assemblies (../xna4-decomp), not from FNA: where the two differ, CNA follows XNA.
#include "ChaosEngine.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <optional>

#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/TimeSpan.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/OcclusionQuery.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

#include "ChaosSupport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Audio;

namespace CnaKiller
{
    namespace
    {
        const float kNaN = std::numeric_limits<float>::quiet_NaN();

        std::vector<SharpRuntime::bytecs> Silence(int bytes)
        {
            return std::vector<SharpRuntime::bytecs>(static_cast<std::size_t>(bytes), 0);
        }

        void ApplyVertexColorEffect(BasicEffect& effect)
        {
            effect.VertexColorEnabled = true;
            effect.getCurrentTechniqueProperty()->getPassesProperty()[0]->Apply();
        }
    }

    // -----------------------------------------------------------------------------------------
    // SpriteBatch Begin/End pairing (SpriteBatch.Begin/End/InternalDraw)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseSpriteBatch()
    {
        GraphicsDevice& device = Device();
        SpriteBatch first(device);
        SpriteBatch second(device);
        Texture2D& texture = WhiteTexture();

        switch (RandomInt(0, 6))
        {
            case 0:
                first.Begin();
                Expect<System::InvalidOperationException>("SpriteBatch.Begin twice without End", [&] { first.Begin(); });
                first.End();
                break;
            case 1:
                Expect<System::InvalidOperationException>("SpriteBatch.End without Begin", [&] { first.End(); });
                break;
            case 2:
                Expect<System::InvalidOperationException>("SpriteBatch.Draw without Begin",
                                                          [&] { first.Draw(texture, Vector2::Zero, Color::White); });
                break;
            case 3:
                first.Begin();
                Expect<System::InvalidOperationException>(
                    "SpriteBatch.Begin(Immediate) while another batch is between Begin and End",
                    [&] { second.Begin(SpriteSortMode::Immediate, BlendState::AlphaBlend); });
                first.End();
                break;
            case 4:
                first.Begin(SpriteSortMode::Immediate, BlendState::AlphaBlend);
                Expect<System::InvalidOperationException>(
                    "SpriteBatch.Begin while another batch is in Immediate mode", [&] { second.Begin(); });
                first.End();
                break;
            default:
            {
                // A texture disposed between Draw and End: End binds it to Textures[0], and
                // XNA's TextureCollection setter refuses a disposed texture.
                Texture2D doomed(device, 4, 4);
                std::array<Color, 16> pixels{};
                pixels.fill(RandomOpaqueColor());
                doomed.SetData(pixels.data(), 16);
                first.Begin();
                first.Draw(doomed, Vector2::Zero, Color::White);
                doomed.Dispose();
                Expect<System::ObjectDisposedException>("SpriteBatch.End with a texture disposed after Draw",
                                                        [&] { first.End(); });
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // Viewport, ScissorRectangle and texture slots (GraphicsDevice setters, TextureCollection)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseDeviceState()
    {
        GraphicsDevice& device = Device();
        const PresentationParameters& pp = device.getPresentationParametersProperty();
        const int width = pp.getBackBufferWidthProperty();
        const int height = pp.getBackBufferHeightProperty();

        switch (RandomInt(0, 10))
        {
            case 0:
                Expect<System::ArgumentException>("a Viewport with a negative X",
                                                  [&] { device.setViewportProperty(Viewport(-1, 0, 1, 1)); });
                break;
            case 1:
                Expect<System::ArgumentException>("a Viewport with zero width",
                                                  [&] { device.setViewportProperty(Viewport(0, 0, 0, 1)); });
                break;
            case 2:
                Expect<System::ArgumentException>("a Viewport reaching past the back buffer", [&] {
                    device.setViewportProperty(Viewport(width / 2, 0, width / 2 + 1, height));
                });
                break;
            case 3:
            {
                Viewport viewport(0, 0, 1, 1);
                viewport.setMinDepthProperty(0.75f);
                viewport.setMaxDepthProperty(0.25f);
                Expect<System::ArgumentException>("a Viewport with MaxDepth below MinDepth",
                                                  [&] { device.setViewportProperty(viewport); });
                break;
            }
            case 4:
            {
                Viewport viewport(0, 0, 1, 1);
                viewport.setMaxDepthProperty(1.5f);
                Expect<System::ArgumentException>("a Viewport with MaxDepth above 1",
                                                  [&] { device.setViewportProperty(viewport); });
                break;
            }
            case 5:
                Expect<System::ArgumentException>("a ScissorRectangle with a negative Y",
                                                  [&] { device.setScissorRectangleProperty(Rectangle(0, -1, 1, 1)); });
                break;
            case 6:
                Expect<System::ArgumentException>("a ScissorRectangle reaching past the back buffer", [&] {
                    device.setScissorRectangleProperty(Rectangle(1, 0, width, 1));
                });
                break;
            case 7:
                Expect<System::ArgumentOutOfRangeException>("setting Textures[16] (HiDef has 16 slots)",
                                                            [&] { device.getTexturesProperty()(16, nullptr); });
                break;
            case 8:
                Expect<System::ArgumentOutOfRangeException>("reading Textures[-1]",
                                                            [&] { (void)device.getTexturesProperty()[-1]; });
                break;
            default:
            {
                RenderTarget2D target(device, 8, 8);
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("setting the active render target into Textures[0]",
                                                          [&] { device.getTexturesProperty()(0, &target); });
                device.SetRenderTarget(nullptr);
                break;
            }
        }
        device.setViewportProperty(Viewport(0, 0, width, height));
    }

    // -----------------------------------------------------------------------------------------
    // Draw calls and buffers (GraphicsDevice.Draw*, VertexBuffer.CopyData, resource constructors)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseDraw()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        BasicEffect effect(device);
        ApplyVertexColorEffect(effect);
        const auto& declaration = VertexPositionColor::getVertexDeclarationStatic();
        std::vector<VertexPositionColor> vertices(6, VertexPositionColor(Vector3(0, 0, 0), Color::White));

        switch (RandomInt(0, 12))
        {
            case 0:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                buffer.SetData(vertices.data(), 6);
                device.SetVertexBuffer(&buffer);
                Expect<System::ArgumentOutOfRangeException>("DrawPrimitives with primitiveCount 0", [&] {
                    device.DrawPrimitives(PrimitiveType::TriangleList, 0, 0);
                });
                device.SetVertexBuffer(nullptr);
                break;
            }
            case 1:
                Expect<System::InvalidOperationException>("DrawPrimitives with no vertex buffer set", [&] {
                    device.DrawPrimitives(PrimitiveType::TriangleList, 0, 1);
                });
                break;
            case 2:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                buffer.SetData(vertices.data(), 6);
                device.SetVertexBuffer(&buffer);
                Expect<System::InvalidOperationException>("DrawIndexedPrimitives with no index buffer set", [&] {
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 6, 0, 2);
                });
                device.SetVertexBuffer(nullptr);
                break;
            }
            case 3:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                IndexBuffer indices(device, IndexElementSize::SixteenBits, 6, BufferUsage::None);
                device.SetVertexBuffer(&buffer);
                device.SetIndexBuffer(&indices);
                Expect<System::ArgumentOutOfRangeException>("DrawIndexedPrimitives with numVertices 0", [&] {
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 0, 0, 2);
                });
                device.SetIndexBuffer(nullptr);
                device.SetVertexBuffer(nullptr);
                break;
            }
            case 4:
                Expect<System::ArgumentOutOfRangeException>("DrawUserPrimitives with primitiveCount 0", [&] {
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices, 0, 0);
                });
                break;
            case 5:
                Expect<System::ArgumentOutOfRangeException>("DrawUserPrimitives needing more vertices than the array holds", [&] {
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices, 0, 3);
                });
                break;
            case 6:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::WriteOnly);
                buffer.SetData(vertices.data(), 6);
                std::vector<VertexPositionColor> readBack(6);
                Expect<System::NotSupportedException>("GetData on a WriteOnly vertex buffer",
                                                      [&] { buffer.GetData(readBack.data(), 6); });
                break;
            }
            case 7:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                device.SetVertexBuffer(&buffer);
                Expect<System::InvalidOperationException>("VertexBuffer.SetData while the buffer is bound",
                                                          [&] { buffer.SetData(vertices.data(), 6); });
                device.SetVertexBuffer(nullptr);
                break;
            }
            case 8:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                Expect<System::InvalidOperationException>("VertexBuffer.SetData past the end of the buffer",
                                                          [&] { buffer.SetData(5 * 16, vertices.data(), 0, 2, 16); });
                break;
            }
            case 9:
            {
                VertexBuffer buffer(device, declaration, 6, BufferUsage::None);
                Expect<System::ArgumentOutOfRangeException>("VertexBuffer.SetData with a stride smaller than the vertex",
                                                            [&] { buffer.SetData(0, vertices.data(), 0, 2, 8); });
                break;
            }
            case 10:
                Expect<System::ArgumentOutOfRangeException>("a VertexBuffer of zero vertices", [&] {
                    VertexBuffer buffer(device, declaration, 0, BufferUsage::None);
                });
                break;
            default:
                Expect<System::ArgumentOutOfRangeException>("a Texture2D of zero width",
                                                            [&] { Texture2D texture(device, 0, 4); });
                break;
        }
    }

    // -----------------------------------------------------------------------------------------
    // Render targets (GraphicsDevice.SetRenderTargets/Present/GetBackBufferData/Clear, Texture2D.CopyData)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseRenderTargets()
    {
        GraphicsDevice& device = Device();
        RenderTarget2D target(device, 8, 8);
        std::array<Color, 64> pixels{};

        switch (RandomInt(0, 8))
        {
            case 0:
                Expect<System::ArgumentException>("SetRenderTargets with the same target twice", [&] {
                    device.SetRenderTargets({RenderTargetBinding(&target), RenderTargetBinding(&target)});
                });
                break;
            case 1:
            {
                RenderTarget2D other(device, 9, 8);
                Expect<System::ArgumentException>("SetRenderTargets with targets of different sizes", [&] {
                    device.SetRenderTargets({RenderTargetBinding(&target), RenderTargetBinding(&other)});
                });
                device.SetRenderTarget(nullptr);
                break;
            }
            case 2:
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("Present while a render target is set", [&] { device.Present(); });
                break;
            case 3:
            {
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("GetBackBufferData while a render target is set", [&] {
                    device.GetBackBufferData(pixels.data(), 1);
                });
                break;
            }
            case 4:
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("GetData on the active render target",
                                                          [&] { target.GetData(pixels.data(), 64); });
                break;
            case 5:
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("SetData on the active render target",
                                                          [&] { target.SetData(pixels.data(), 64); });
                break;
            case 6:
                device.SetRenderTarget(&target);
                Expect<System::InvalidOperationException>("Clear(DepthBuffer) on a target without a depth buffer", [&] {
                    device.Clear(ClearOptions::DepthBuffer, Color::Black, 1.0f, 0);
                });
                break;
            default:
                Expect<System::ArgumentOutOfRangeException>("a RenderTarget2D of zero height",
                                                            [&] { RenderTarget2D empty(device, 4, 0); });
                break;
        }
        device.SetRenderTarget(nullptr);
    }

    // -----------------------------------------------------------------------------------------
    // Texture data (Texture2D.CopyData, Texture.GetAndValidateSizes/CopyData, Helpers.ValidateCopyParameters)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseTextureData()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        const int width = RandomInt(2, 33);
        const int height = RandomInt(2, 33);
        Texture2D texture(device, width, height);
        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) + 1, Color::White);
        const int count = width * height;

        switch (RandomInt(0, 5))
        {
            case 0:
                device.getTexturesProperty()(0, &texture);
                Expect<System::InvalidOperationException>("SetData on a texture still set in Textures[0]",
                                                          [&] { texture.SetData(pixels.data(), count); });
                device.getTexturesProperty()(0, nullptr);
                break;
            case 1:
                Expect<System::ArgumentException>("SetData with one element too few for the level",
                                                  [&] { texture.SetData(pixels.data(), count - 1); });
                break;
            case 2:
                Expect<System::ArgumentException>("SetData with one element too many for the level",
                                                  [&] { texture.SetData(pixels.data(), count + 1); });
                break;
            case 3:
                Expect<System::ArgumentOutOfRangeException>("SetData with elementCount 0",
                                                            [&] { texture.SetData(pixels.data(), 0); });
                break;
            default:
            {
                // XNA checks the element type's size against the format: a 16-byte Vector4 cannot
                // describe a 4-byte Color texel.
                std::vector<Vector4> wide(static_cast<std::size_t>(count));
                Expect<System::ArgumentException>("SetData<Vector4> on a Color texture",
                                                  [&] { texture.SetData(wide.data(), count); });
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // Disposed resources (Helpers.CheckDisposed, TextureCollection setter)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseDisposed()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        std::array<Color, 16> pixels{};
        pixels.fill(Color::White);
        std::vector<VertexPositionColor> vertices(3, VertexPositionColor(Vector3(0, 0, 0), Color::White));

        switch (RandomInt(0, 7))
        {
            case 0:
            {
                Texture2D texture(device, 4, 4);
                texture.Dispose();
                Expect<System::ObjectDisposedException>("SetData on a disposed Texture2D",
                                                        [&] { texture.SetData(pixels.data(), 16); });
                break;
            }
            case 1:
            {
                Texture2D texture(device, 4, 4);
                texture.Dispose();
                Expect<System::ObjectDisposedException>("GetData on a disposed Texture2D",
                                                        [&] { texture.GetData(pixels.data(), 16); });
                break;
            }
            case 2:
            {
                Texture2D texture(device, 4, 4);
                texture.Dispose();
                Expect<System::ObjectDisposedException>("setting a disposed texture into Textures[0]",
                                                        [&] { device.getTexturesProperty()(0, &texture); });
                device.getTexturesProperty()(0, nullptr);
                break;
            }
            case 3:
            {
                VertexBuffer buffer(device, VertexPositionColor::getVertexDeclarationStatic(), 3, BufferUsage::None);
                buffer.Dispose();
                Expect<System::ObjectDisposedException>("SetData on a disposed VertexBuffer",
                                                        [&] { buffer.SetData(vertices.data(), 3); });
                break;
            }
            case 4:
            {
                // Dispose is idempotent in XNA; the second call must be a no-op.
                RenderTarget2D target(device, 4, 4);
                target.Dispose();
                target.Dispose();
                break;
            }
            case 5:
            {
                // Rules not pinned down from the IL: refusing or ignoring is fine, crashing is not.
                VertexBuffer buffer(device, VertexPositionColor::getVertexDeclarationStatic(), 3, BufferUsage::None);
                buffer.SetData(vertices.data(), 3);
                BasicEffect effect(device);
                ApplyVertexColorEffect(effect);
                device.SetVertexBuffer(&buffer);
                buffer.Dispose();
                (void)Tolerate("DrawPrimitives from a vertex buffer disposed while bound",
                               [&] { device.DrawPrimitives(PrimitiveType::TriangleList, 0, 1); });
                device.SetVertexBuffer(nullptr);
                break;
            }
            default:
            {
                RenderTarget2D target(device, 4, 4);
                target.Dispose();
                (void)Tolerate("SetRenderTarget with a disposed render target", [&] {
                    device.SetRenderTarget(&target);
                    device.Clear(Color::Black);
                });
                device.SetRenderTarget(nullptr);
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------------------
    // OcclusionQuery pairing (OcclusionQuery.Begin/End)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseOcclusion()
    {
        OcclusionQuery query(Device());
        switch (RandomInt(0, 3))
        {
            case 0:
                Expect<System::InvalidOperationException>("OcclusionQuery.End without Begin", [&] { query.End(); });
                break;
            case 1:
                query.Begin();
                Expect<System::InvalidOperationException>("OcclusionQuery.Begin twice", [&] { query.Begin(); });
                query.End();
                break;
            default:
                // XNA insists IsComplete is read between one End and the next Begin.
                query.Begin();
                query.End();
                Expect<System::InvalidOperationException>("OcclusionQuery.Begin again before IsComplete was read",
                                                          [&] { query.Begin(); });
                break;
        }
    }

    // -----------------------------------------------------------------------------------------
    // Audio (SoundEffect.FromBuffer, SoundEffectInstance setters, DynamicSoundEffectInstance)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseAudio()
    {
        const int pick = RandomInt(0, 17);
        const float outOfRange = Chance(3) ? kNaN : (Chance(2) ? 1.5f : -1.5f);
        WithAudio([&] {
            switch (pick)
            {
                case 0:
                    Expect<System::ArgumentOutOfRangeException>("a SoundEffect at 7999 Hz", [&] {
                        SoundEffect effect(Silence(64), 7999, AudioChannels::Mono);
                    });
                    break;
                case 1:
                    Expect<System::ArgumentOutOfRangeException>("a SoundEffect at 48001 Hz", [&] {
                        SoundEffect effect(Silence(64), 48001, AudioChannels::Mono);
                    });
                    break;
                case 2:
                    Expect<System::ArgumentOutOfRangeException>("a SoundEffect with three channels", [&] {
                        SoundEffect effect(Silence(60), 22050, static_cast<AudioChannels>(3));
                    });
                    break;
                case 3:
                    Expect<System::ArgumentException>("a SoundEffect with an empty buffer", [&] {
                        SoundEffect effect(Silence(0), 22050, AudioChannels::Mono);
                    });
                    break;
                case 4:
                    Expect<System::ArgumentException>("a stereo SoundEffect whose buffer is not whole frames", [&] {
                        SoundEffect effect(Silence(6), 22050, AudioChannels::Stereo);
                    });
                    break;
                case 5:
                    Expect<System::ArgumentException>("a SoundEffect whose loop region runs past the samples", [&] {
                        SoundEffect effect(Silence(64), 0, 64, 22050, AudioChannels::Mono, 16, 32);
                    });
                    break;
                case 6:
                    Expect<System::ArgumentException>("a SoundEffect whose offset+count runs past the buffer", [&] {
                        SoundEffect effect(Silence(64), 32, 64, 22050, AudioChannels::Mono, 0, 0);
                    });
                    break;
                case 7:
                case 8:
                case 9:
                {
                    SoundEffect effect(Silence(4410), 22050, AudioChannels::Mono);
                    SoundEffectInstance instance = effect.CreateInstance();
                    if (pick == 7)
                        Expect<System::ArgumentOutOfRangeException>("SoundEffectInstance.Volume outside 0..1 or NaN",
                                                                    [&] { instance.setVolumeProperty(outOfRange); });
                    else if (pick == 8)
                        Expect<System::ArgumentOutOfRangeException>("SoundEffectInstance.Pitch outside -1..1 or NaN",
                                                                    [&] { instance.setPitchProperty(outOfRange * 2); });
                    else
                        Expect<System::ArgumentOutOfRangeException>("SoundEffectInstance.Pan outside -1..1 or NaN",
                                                                    [&] { instance.setPanProperty(outOfRange * 2); });
                    break;
                }
                case 10:
                {
                    SoundEffect effect(Silence(4410), 22050, AudioChannels::Mono);
                    SoundEffectInstance instance = effect.CreateInstance();
                    instance.setVolumeProperty(0.0f);
                    instance.Play();
                    Expect<System::InvalidOperationException>("SoundEffectInstance.IsLooped after Play",
                                                              [&] { instance.setIsLoopedProperty(true); });
                    instance.Stop();
                    break;
                }
                case 11:
                {
                    SoundEffect effect(Silence(4410), 22050, AudioChannels::Mono);
                    SoundEffectInstance instance = effect.CreateInstance();
                    instance.setVolumeProperty(0.0f);
                    instance.Apply3D(AudioListener(), AudioEmitter());
                    instance.Play();
                    Expect<System::InvalidOperationException>("SoundEffectInstance.Pan on a playing 3D instance",
                                                              [&] { instance.setPanProperty(0.5f); });
                    instance.Stop();
                    break;
                }
                case 12:
                {
                    SoundEffect effect(Silence(4410), 22050, AudioChannels::Mono);
                    SoundEffectInstance instance = effect.CreateInstance();
                    effect.Dispose();
                    Expect<System::ObjectDisposedException>("Play on an instance whose SoundEffect was disposed",
                                                            [&] { instance.Play(); });
                    Expect<System::ObjectDisposedException>("CreateInstance on a disposed SoundEffect",
                                                            [&] { (void)effect.CreateInstance(); });
                    break;
                }
                case 13:
                    Expect<System::ArgumentOutOfRangeException>("SoundEffect.MasterVolume outside 0..1 or NaN",
                                                                [&] { SoundEffect::setMasterVolumeProperty(outOfRange); });
                    break;
                case 14:
                    Expect<System::ArgumentOutOfRangeException>("SoundEffect.SpeedOfSound of zero",
                                                                [&] { SoundEffect::setSpeedOfSoundProperty(0.0f); });
                    break;
                case 15:
                    Expect<System::ArgumentOutOfRangeException>("SoundEffect.DopplerScale below zero",
                                                                [&] { SoundEffect::setDopplerScaleProperty(-1.0f); });
                    break;
                default:
                {
                    const int kind = RandomInt(0, 3);
                    if (kind == 0)
                    {
                        Expect<System::ArgumentOutOfRangeException>("a DynamicSoundEffectInstance at 100 Hz", [&] {
                            DynamicSoundEffectInstance voice(100, AudioChannels::Mono);
                        });
                        break;
                    }
                    DynamicSoundEffectInstance voice(22050, AudioChannels::Stereo);
                    if (kind == 1)
                        Expect<System::ArgumentException>("SubmitBuffer of a buffer that is not whole stereo frames",
                                                          [&] { voice.SubmitBuffer(Silence(6)); });
                    else
                        Expect<System::InvalidOperationException>("DynamicSoundEffectInstance.IsLooped = true",
                                                                  [&] { voice.setIsLoopedProperty(true); });
                    break;
                }
            }
        });
    }

    // -----------------------------------------------------------------------------------------
    // Game timing (Game.TargetElapsedTime / InactiveSleepTime setters)
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionMisuseGameTiming()
    {
        switch (RandomInt(0, 3))
        {
            case 0:
                Expect<System::ArgumentOutOfRangeException>("Game.TargetElapsedTime of zero", [&] {
                    game_->setTargetElapsedTimeProperty(System::TimeSpan::Zero);
                });
                break;
            case 1:
                Expect<System::ArgumentOutOfRangeException>("a negative Game.TargetElapsedTime", [&] {
                    game_->setTargetElapsedTimeProperty(System::TimeSpan::FromMilliseconds(-5));
                });
                break;
            default:
                Expect<System::ArgumentOutOfRangeException>("a negative Game.InactiveSleepTime", [&] {
                    game_->setInactiveSleepTimeProperty(System::TimeSpan::FromMilliseconds(-1));
                });
                break;
        }
    }
}
