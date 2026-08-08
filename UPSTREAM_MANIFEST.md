# Upstream provenance manifest

| Destination | Upstream source | Pinned revision | Licence / lineage | Status | Tests |
| --- | --- | --- | --- | --- | --- |
| `include/Myra/Events/EventHandlingStrategy.hpp` | `src/Myra/Events/EventHandlingStrategy.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventHandlingStrategyTests.ProvidesBothUpstreamPropagationModes` |
| `include/Myra/Graphics2D/Thickness.hpp` | `src/Myra/Graphics2D/Thickness.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `ThicknessTests.*`, `ThicknessRectangleTests.*` |
| `src/Myra/Graphics2D/Thickness.cpp` | `src/Myra/Graphics2D/Thickness.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `ThicknessTests.*`, `ThicknessRectangleTests.*` |
| `include/Myra/Graphics2D/UI/InputEventType.hpp` | `src/Myra/Graphics2D/UI/InputEventsManager.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.*`, `InputEventsManagerTests.*` |
| `include/Myra/Graphics2D/UI/InputEventsManager.hpp` | `src/Myra/Graphics2D/UI/InputEventsManager.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `InputEventsManagerTests.*` |
| `src/Myra/Graphics2D/UI/InputEventsManager.cpp` | `src/Myra/Graphics2D/UI/InputEventsManager.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `InputEventsManagerTests.*` |
| `include/Myra/MyraEnvironment.hpp` | `src/Myra/MyraEnvironment.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Partial: `EventHandlingModel` only; expanded as dependencies arrive | `InputEventsManagerTests.*` |
| `src/Myra/MyraEnvironment.cpp` | `src/Myra/MyraEnvironment.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Partial: `EventHandlingModel` only; expanded as dependencies arrive | `InputEventsManagerTests.*` |
| `include/Myra/Events/MyraEventArgs.hpp` | `src/Myra/Events/MyraEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.*`, `InputEventsManagerTests.StopPropagationRemovesPendingEventsOfTheSameType` |
| `src/Myra/Events/MyraEventArgs.cpp` | `src/Myra/Events/MyraEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.*`, `InputEventsManagerTests.StopPropagationRemovesPendingEventsOfTheSameType` |
| `include/Myra/Events/CancellableEventArgs.hpp` | `src/Myra/Events/CancellableEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` |
| `include/Myra/Events/CancellableEventArgsT.hpp` | `src/Myra/Events/CancellableEventArgs{T}.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` |
| `include/Myra/Events/GenericEventArgs.hpp` | `src/Myra/Events/GenericEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` |
| `include/Myra/Events/MyraEventHandler.hpp` | `src/Myra/Events/MyraEventHandler.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported using sharp-runtime `MulticastAction` | `EventsTests.GenericHandlerSupportsMutationAndTokenRemoval` |
| `include/Myra/Events/TextDeletedEventArgs.hpp` | `src/Myra/Events/TextDeletedEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` |
| `include/Myra/Events/ValueChangedEventArgs.hpp` | `src/Myra/Events/ValueChangedEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` |
| `include/Myra/Events/ValueChangingEventArgs.hpp` | `src/Myra/Events/ValueChangingEventArgs.cs` | `0d79b939310bfe1d00b21803fe15e291caf60aa1` | Myra MIT, copyright 2017-2020 The Myra Team; complete notice: `THIRD_PARTY_NOTICES.md` | Ported | `EventsTests.*` |

## Rules

1. Add one row before committing each source or asset copied or translated from
   an upstream project.
2. The destination must be a repository-relative path and match the source
   header's `Ported from:` value.
3. State the exact upstream path and immutable commit/tag, not only a branch.
4. Include every applicable lineage, such as Myra plus MonoGame.Extended or
   TextCopy, and name a regression test.
5. Sources intentionally not ported are recorded with `Excluded` status and a
   reason; the seven upstream `src/Myra/Platform/**` files are FNA-excluded.
