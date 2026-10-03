# XNA to Swift missing-type inventory

Generated from the compiler Symbol Graph and the pinned XNA 4.0 Windows runtime contract.
Normal strict status is green: the selected profile is complete.

```text
REFERENCE_TYPES=257
REFERENCE_MEMBERS=2964
EXPECTED_SWIFT_TYPES=257
EXPECTED_SWIFT_MEMBERS=2887
TARGET_TYPES=257
TARGET_MEMBERS=2890
TOTAL_DIAGNOSTICS=0
COMPLETE_TYPES=257
PARTIAL_TYPES=0
MISSING_TYPES=0
MISSING_TYPE=0
MISSING_MEMBER=0
UNEXPECTED_TYPE=0
UNEXPECTED_MEMBER=0
TYPE_KIND_MISMATCH=0
BASE_MAPPING_MISMATCH=0
INTERFACE_MAPPING_MISMATCH=0
FIELD_MAPPING_MISMATCH=0
PROPERTY_MAPPING_MISMATCH=0
METHOD_SIGNATURE_MAPPING_MISMATCH=0
PARAMETER_MAPPING_MISMATCH=0
RETURN_MAPPING_MISMATCH=0
OVERLOAD_MAPPING_MISMATCH=0
GENERIC_MAPPING_MISMATCH=0
ENUM_VALUE_MISMATCH=0
FLAGS_MAPPING_MISMATCH=0
EVENT_MAPPING_MISMATCH=0
OPERATOR_MAPPING_MISMATCH=0
REF_OUT_MAPPING_MISMATCH=0
LANGUAGE_MAPPING_MISMATCH=0
INTERNAL_TYPE_LEAK=0
RAW_HANDLE_LEAK=0
PUBLIC_NATIVE_FFI_LEAK=0
INHERITANCE_MAPPING_MISMATCH=0
UNMEASURED_STRUCTURAL_CATEGORY=0
ALLOWLIST_ENTRIES=0
APPLIED_ALLOWLIST_ENTRIES=0
LANGUAGE_PROJECTION_EXCLUSIONS=132
ENUM_STORAGE_FIELD_EXCLUSIONS=49
FINALIZER_LANGUAGE_MAPPINGS=28
NAMESPACE_MARKERS=13
INHERITED_MEMBER_PROJECTIONS=4
PROTOCOL_WITNESS_MEMBER_PROJECTIONS=38
ARRAY_MUTATION_MAPPINGS=20
COMPARABLE_INTERFACE_PROJECTIONS=1
COLLECTION_INTERFACE_PROJECTIONS=1
ENUMERATOR_SUPPORT_PROJECTIONS=13
ACCESSOR_PROJECTIONS=840
INDEXED_ACCESSOR_PROJECTIONS=24
THROWING_GETTER_PROJECTIONS=114
PROPERTY_SETTER_PROJECTIONS=165
WRITER_METHOD_PROJECTIONS=114
THROWING_WRITER_METHOD_PROJECTIONS=114
INFALLIBLE_WRITER_METHOD_PROJECTIONS=0
GETTER_ONLY_PROJECTIONS=561
WRITE_ONLY_PROJECTIONS=0
MEASURED_ACCESSOR_PROJECTIONS=114
PENDING_ACCESSOR_PROJECTIONS=0
GLOBAL_OPTIONAL_OPERATOR_PROJECTIONS=2
NONPUBLIC_CONSTRUCTION_PROJECTIONS=59
EVENT_PROJECTIONS=49
EVENT_SUPPORT_TYPE_MEASUREMENTS=4
BCL_SUPPORT_TYPE_MEASUREMENTS=45
XNA_SEALED_CLASS_PROJECTIONS=69
NONDERIVABLE_UNSEALED_CLASSES=0
BCL_RESOURCE_STRING_PROJECTIONS=40
BCL_ABSTRACT_BASE_WIDENINGS=4
BCL_STATIC_TABLE_PROJECTIONS=1
XNA_RESOURCE_STRING_PROJECTIONS=103
MEASURED_SUPPORT_BASE_PROJECTIONS=25
BCL_BASE_PROJECTIONS=21
PROJECTED_BCL_BASE_TYPES=21
PENDING_BCL_BASE_TYPES=0
BCL_INHERITED_MEMBER_PROJECTIONS=144
REFERENCE_RETURN_PROJECTIONS=369
OPTIONAL_RETURN_PROJECTIONS=151
NONOPTIONAL_RETURN_PROJECTIONS=218
PROVEN_NULLABLE_RETURN_PROJECTIONS=113
PROVEN_NONNULL_RETURN_PROJECTIONS=134
UNKNOWN_RETURN_NULLABILITY_PROJECTIONS=122
NULLABLE_INFALLIBLE_RETURN_PROJECTIONS=68
NULLABLE_FALLIBLE_RETURN_PROJECTIONS=45
NONNULL_INFALLIBLE_RETURN_PROJECTIONS=67
NONNULL_FALLIBLE_RETURN_PROJECTIONS=67
MEASURED_RETURN_NULLABILITY_PROJECTIONS=369
PENDING_RETURN_NULLABILITY_PROJECTIONS=0
OPTIONAL_RETURN_PROJECTIONS_OBSERVED=151
```

## Complete types

- `Microsoft.Xna.Framework.Audio.AudioCategory`
- `Microsoft.Xna.Framework.Audio.AudioChannels`
- `Microsoft.Xna.Framework.Audio.AudioEmitter`
- `Microsoft.Xna.Framework.Audio.AudioEngine`
- `Microsoft.Xna.Framework.Audio.AudioListener`
- `Microsoft.Xna.Framework.Audio.AudioStopOptions`
- `Microsoft.Xna.Framework.Audio.Cue`
- `Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance`
- `Microsoft.Xna.Framework.Audio.InstancePlayLimitException`
- `Microsoft.Xna.Framework.Audio.Microphone`
- `Microsoft.Xna.Framework.Audio.MicrophoneState`
- `Microsoft.Xna.Framework.Audio.NoAudioHardwareException`
- `Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException`
- `Microsoft.Xna.Framework.Audio.RendererDetail`
- `Microsoft.Xna.Framework.Audio.SoundBank`
- `Microsoft.Xna.Framework.Audio.SoundEffect`
- `Microsoft.Xna.Framework.Audio.SoundEffectInstance`
- `Microsoft.Xna.Framework.Audio.SoundState`
- `Microsoft.Xna.Framework.Audio.WaveBank`
- `Microsoft.Xna.Framework.BoundingBox`
- `Microsoft.Xna.Framework.BoundingFrustum`
- `Microsoft.Xna.Framework.BoundingSphere`
- `Microsoft.Xna.Framework.Color`
- `Microsoft.Xna.Framework.ContainmentType`
- `Microsoft.Xna.Framework.Content.ContentLoadException`
- `Microsoft.Xna.Framework.Content.ContentManager`
- `Microsoft.Xna.Framework.Content.ContentReader`
- `Microsoft.Xna.Framework.Content.ContentSerializerAttribute`
- `Microsoft.Xna.Framework.Content.ContentSerializerCollectionItemNameAttribute`
- `Microsoft.Xna.Framework.Content.ContentSerializerIgnoreAttribute`
- `Microsoft.Xna.Framework.Content.ContentSerializerRuntimeTypeAttribute`
- `Microsoft.Xna.Framework.Content.ContentSerializerTypeVersionAttribute`
- `Microsoft.Xna.Framework.Content.ContentTypeReader`
- `Microsoft.Xna.Framework.Content.ContentTypeReaderManager`
- `Microsoft.Xna.Framework.Content.ContentTypeReaderOfT`
- `Microsoft.Xna.Framework.Content.ResourceContentManager`
- `Microsoft.Xna.Framework.Curve`
- `Microsoft.Xna.Framework.CurveContinuity`
- `Microsoft.Xna.Framework.CurveKey`
- `Microsoft.Xna.Framework.CurveKeyCollection`
- `Microsoft.Xna.Framework.CurveLoopType`
- `Microsoft.Xna.Framework.CurveTangent`
- `Microsoft.Xna.Framework.Design.BoundingBoxConverter`
- `Microsoft.Xna.Framework.Design.BoundingSphereConverter`
- `Microsoft.Xna.Framework.Design.ColorConverter`
- `Microsoft.Xna.Framework.Design.MathTypeConverter`
- `Microsoft.Xna.Framework.Design.MatrixConverter`
- `Microsoft.Xna.Framework.Design.PlaneConverter`
- `Microsoft.Xna.Framework.Design.PointConverter`
- `Microsoft.Xna.Framework.Design.QuaternionConverter`
- `Microsoft.Xna.Framework.Design.RayConverter`
- `Microsoft.Xna.Framework.Design.RectangleConverter`
- `Microsoft.Xna.Framework.Design.Vector2Converter`
- `Microsoft.Xna.Framework.Design.Vector3Converter`
- `Microsoft.Xna.Framework.Design.Vector4Converter`
- `Microsoft.Xna.Framework.DisplayOrientation`
- `Microsoft.Xna.Framework.DrawableGameComponent`
- `Microsoft.Xna.Framework.FrameworkDispatcher`
- `Microsoft.Xna.Framework.Game`
- `Microsoft.Xna.Framework.GameComponent`
- `Microsoft.Xna.Framework.GameComponentCollection`
- `Microsoft.Xna.Framework.GameComponentCollectionEventArgs`
- `Microsoft.Xna.Framework.GameServiceContainer`
- `Microsoft.Xna.Framework.GameTime`
- `Microsoft.Xna.Framework.GameWindow`
- `Microsoft.Xna.Framework.GamerServices.GamerServicesComponent`
- `Microsoft.Xna.Framework.Graphics.AlphaTestEffect`
- `Microsoft.Xna.Framework.Graphics.BasicEffect`
- `Microsoft.Xna.Framework.Graphics.Blend`
- `Microsoft.Xna.Framework.Graphics.BlendFunction`
- `Microsoft.Xna.Framework.Graphics.BlendState`
- `Microsoft.Xna.Framework.Graphics.BufferUsage`
- `Microsoft.Xna.Framework.Graphics.ClearOptions`
- `Microsoft.Xna.Framework.Graphics.ColorWriteChannels`
- `Microsoft.Xna.Framework.Graphics.CompareFunction`
- `Microsoft.Xna.Framework.Graphics.CubeMapFace`
- `Microsoft.Xna.Framework.Graphics.CullMode`
- `Microsoft.Xna.Framework.Graphics.DepthFormat`
- `Microsoft.Xna.Framework.Graphics.DepthStencilState`
- `Microsoft.Xna.Framework.Graphics.DeviceLostException`
- `Microsoft.Xna.Framework.Graphics.DeviceNotResetException`
- `Microsoft.Xna.Framework.Graphics.DirectionalLight`
- `Microsoft.Xna.Framework.Graphics.DisplayMode`
- `Microsoft.Xna.Framework.Graphics.DisplayModeCollection`
- `Microsoft.Xna.Framework.Graphics.DualTextureEffect`
- `Microsoft.Xna.Framework.Graphics.DynamicIndexBuffer`
- `Microsoft.Xna.Framework.Graphics.DynamicVertexBuffer`
- `Microsoft.Xna.Framework.Graphics.Effect`
- `Microsoft.Xna.Framework.Graphics.EffectAnnotation`
- `Microsoft.Xna.Framework.Graphics.EffectAnnotationCollection`
- `Microsoft.Xna.Framework.Graphics.EffectMaterial`
- `Microsoft.Xna.Framework.Graphics.EffectParameter`
- `Microsoft.Xna.Framework.Graphics.EffectParameterClass`
- `Microsoft.Xna.Framework.Graphics.EffectParameterCollection`
- `Microsoft.Xna.Framework.Graphics.EffectParameterType`
- `Microsoft.Xna.Framework.Graphics.EffectPass`
- `Microsoft.Xna.Framework.Graphics.EffectPassCollection`
- `Microsoft.Xna.Framework.Graphics.EffectTechnique`
- `Microsoft.Xna.Framework.Graphics.EffectTechniqueCollection`
- `Microsoft.Xna.Framework.Graphics.EnvironmentMapEffect`
- `Microsoft.Xna.Framework.Graphics.FillMode`
- `Microsoft.Xna.Framework.Graphics.GraphicsAdapter`
- `Microsoft.Xna.Framework.Graphics.GraphicsDevice`
- `Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus`
- `Microsoft.Xna.Framework.Graphics.GraphicsProfile`
- `Microsoft.Xna.Framework.Graphics.GraphicsResource`
- `Microsoft.Xna.Framework.Graphics.IEffectFog`
- `Microsoft.Xna.Framework.Graphics.IEffectLights`
- `Microsoft.Xna.Framework.Graphics.IEffectMatrices`
- `Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService`
- `Microsoft.Xna.Framework.Graphics.IVertexType`
- `Microsoft.Xna.Framework.Graphics.IndexBuffer`
- `Microsoft.Xna.Framework.Graphics.IndexElementSize`
- `Microsoft.Xna.Framework.Graphics.Model`
- `Microsoft.Xna.Framework.Graphics.ModelBone`
- `Microsoft.Xna.Framework.Graphics.ModelBoneCollection`
- `Microsoft.Xna.Framework.Graphics.ModelBoneCollection.Enumerator`
- `Microsoft.Xna.Framework.Graphics.ModelEffectCollection`
- `Microsoft.Xna.Framework.Graphics.ModelEffectCollection.Enumerator`
- `Microsoft.Xna.Framework.Graphics.ModelMesh`
- `Microsoft.Xna.Framework.Graphics.ModelMeshCollection`
- `Microsoft.Xna.Framework.Graphics.ModelMeshCollection.Enumerator`
- `Microsoft.Xna.Framework.Graphics.ModelMeshPart`
- `Microsoft.Xna.Framework.Graphics.ModelMeshPartCollection`
- `Microsoft.Xna.Framework.Graphics.ModelMeshPartCollection.Enumerator`
- `Microsoft.Xna.Framework.Graphics.NoSuitableGraphicsDeviceException`
- `Microsoft.Xna.Framework.Graphics.OcclusionQuery`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Alpha8`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Bgr565`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Bgra4444`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Bgra5551`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Byte4`
- `Microsoft.Xna.Framework.Graphics.PackedVector.HalfSingle`
- `Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector2`
- `Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector4`
- `Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVector`
- `Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVectorOfT`
- `Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte2`
- `Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte4`
- `Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedShort2`
- `Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedShort4`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Rg32`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Rgba1010102`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Rgba64`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Short2`
- `Microsoft.Xna.Framework.Graphics.PackedVector.Short4`
- `Microsoft.Xna.Framework.Graphics.PresentInterval`
- `Microsoft.Xna.Framework.Graphics.PresentationParameters`
- `Microsoft.Xna.Framework.Graphics.PrimitiveType`
- `Microsoft.Xna.Framework.Graphics.RasterizerState`
- `Microsoft.Xna.Framework.Graphics.RenderTarget2D`
- `Microsoft.Xna.Framework.Graphics.RenderTargetBinding`
- `Microsoft.Xna.Framework.Graphics.RenderTargetCube`
- `Microsoft.Xna.Framework.Graphics.RenderTargetUsage`
- `Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs`
- `Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs`
- `Microsoft.Xna.Framework.Graphics.SamplerState`
- `Microsoft.Xna.Framework.Graphics.SamplerStateCollection`
- `Microsoft.Xna.Framework.Graphics.SetDataOptions`
- `Microsoft.Xna.Framework.Graphics.SkinnedEffect`
- `Microsoft.Xna.Framework.Graphics.SpriteBatch`
- `Microsoft.Xna.Framework.Graphics.SpriteEffects`
- `Microsoft.Xna.Framework.Graphics.SpriteFont`
- `Microsoft.Xna.Framework.Graphics.SpriteSortMode`
- `Microsoft.Xna.Framework.Graphics.StencilOperation`
- `Microsoft.Xna.Framework.Graphics.SurfaceFormat`
- `Microsoft.Xna.Framework.Graphics.Texture`
- `Microsoft.Xna.Framework.Graphics.Texture2D`
- `Microsoft.Xna.Framework.Graphics.Texture3D`
- `Microsoft.Xna.Framework.Graphics.TextureAddressMode`
- `Microsoft.Xna.Framework.Graphics.TextureCollection`
- `Microsoft.Xna.Framework.Graphics.TextureCube`
- `Microsoft.Xna.Framework.Graphics.TextureFilter`
- `Microsoft.Xna.Framework.Graphics.VertexBuffer`
- `Microsoft.Xna.Framework.Graphics.VertexBufferBinding`
- `Microsoft.Xna.Framework.Graphics.VertexDeclaration`
- `Microsoft.Xna.Framework.Graphics.VertexElement`
- `Microsoft.Xna.Framework.Graphics.VertexElementFormat`
- `Microsoft.Xna.Framework.Graphics.VertexElementUsage`
- `Microsoft.Xna.Framework.Graphics.VertexPositionColor`
- `Microsoft.Xna.Framework.Graphics.VertexPositionColorTexture`
- `Microsoft.Xna.Framework.Graphics.VertexPositionNormalTexture`
- `Microsoft.Xna.Framework.Graphics.VertexPositionTexture`
- `Microsoft.Xna.Framework.Graphics.Viewport`
- `Microsoft.Xna.Framework.GraphicsDeviceInformation`
- `Microsoft.Xna.Framework.GraphicsDeviceManager`
- `Microsoft.Xna.Framework.IDrawable`
- `Microsoft.Xna.Framework.IGameComponent`
- `Microsoft.Xna.Framework.IGraphicsDeviceManager`
- `Microsoft.Xna.Framework.IUpdateable`
- `Microsoft.Xna.Framework.Input.ButtonState`
- `Microsoft.Xna.Framework.Input.Buttons`
- `Microsoft.Xna.Framework.Input.GamePad`
- `Microsoft.Xna.Framework.Input.GamePadButtons`
- `Microsoft.Xna.Framework.Input.GamePadCapabilities`
- `Microsoft.Xna.Framework.Input.GamePadDPad`
- `Microsoft.Xna.Framework.Input.GamePadDeadZone`
- `Microsoft.Xna.Framework.Input.GamePadState`
- `Microsoft.Xna.Framework.Input.GamePadThumbSticks`
- `Microsoft.Xna.Framework.Input.GamePadTriggers`
- `Microsoft.Xna.Framework.Input.GamePadType`
- `Microsoft.Xna.Framework.Input.KeyState`
- `Microsoft.Xna.Framework.Input.Keyboard`
- `Microsoft.Xna.Framework.Input.KeyboardState`
- `Microsoft.Xna.Framework.Input.Keys`
- `Microsoft.Xna.Framework.Input.Mouse`
- `Microsoft.Xna.Framework.Input.MouseState`
- `Microsoft.Xna.Framework.Input.Touch.GestureSample`
- `Microsoft.Xna.Framework.Input.Touch.GestureType`
- `Microsoft.Xna.Framework.Input.Touch.TouchCollection`
- `Microsoft.Xna.Framework.Input.Touch.TouchCollection.Enumerator`
- `Microsoft.Xna.Framework.Input.Touch.TouchLocation`
- `Microsoft.Xna.Framework.Input.Touch.TouchLocationState`
- `Microsoft.Xna.Framework.Input.Touch.TouchPanel`
- `Microsoft.Xna.Framework.Input.Touch.TouchPanelCapabilities`
- `Microsoft.Xna.Framework.LaunchParameters`
- `Microsoft.Xna.Framework.MathHelper`
- `Microsoft.Xna.Framework.Matrix`
- `Microsoft.Xna.Framework.Media.Album`
- `Microsoft.Xna.Framework.Media.AlbumCollection`
- `Microsoft.Xna.Framework.Media.Artist`
- `Microsoft.Xna.Framework.Media.ArtistCollection`
- `Microsoft.Xna.Framework.Media.Genre`
- `Microsoft.Xna.Framework.Media.GenreCollection`
- `Microsoft.Xna.Framework.Media.MediaLibrary`
- `Microsoft.Xna.Framework.Media.MediaPlayer`
- `Microsoft.Xna.Framework.Media.MediaQueue`
- `Microsoft.Xna.Framework.Media.MediaSource`
- `Microsoft.Xna.Framework.Media.MediaSourceType`
- `Microsoft.Xna.Framework.Media.MediaState`
- `Microsoft.Xna.Framework.Media.Picture`
- `Microsoft.Xna.Framework.Media.PictureAlbum`
- `Microsoft.Xna.Framework.Media.PictureAlbumCollection`
- `Microsoft.Xna.Framework.Media.PictureCollection`
- `Microsoft.Xna.Framework.Media.Playlist`
- `Microsoft.Xna.Framework.Media.PlaylistCollection`
- `Microsoft.Xna.Framework.Media.Song`
- `Microsoft.Xna.Framework.Media.SongCollection`
- `Microsoft.Xna.Framework.Media.Video`
- `Microsoft.Xna.Framework.Media.VideoPlayer`
- `Microsoft.Xna.Framework.Media.VideoSoundtrackType`
- `Microsoft.Xna.Framework.Media.VisualizationData`
- `Microsoft.Xna.Framework.Plane`
- `Microsoft.Xna.Framework.PlaneIntersectionType`
- `Microsoft.Xna.Framework.PlayerIndex`
- `Microsoft.Xna.Framework.Point`
- `Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs`
- `Microsoft.Xna.Framework.Quaternion`
- `Microsoft.Xna.Framework.Ray`
- `Microsoft.Xna.Framework.Rectangle`
- `Microsoft.Xna.Framework.Storage.StorageContainer`
- `Microsoft.Xna.Framework.Storage.StorageDevice`
- `Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException`
- `Microsoft.Xna.Framework.TitleContainer`
- `Microsoft.Xna.Framework.Vector2`
- `Microsoft.Xna.Framework.Vector3`
- `Microsoft.Xna.Framework.Vector4`

## Partial types and exact diagnostics

## Missing types
