# GamerServicesComponent evidence

**Updated:** 2026-08-23

**Authority:** Microsoft XNA Framework 4.0 Windows runtime metadata and IL

**Runtime evidence:** CNA ABI 0.7.0, Linux x86-64, HEADLESS renderer, NULL audio

## Selected contract and outcome

The selected Windows-runtime profile contains only:

```text
public class Microsoft.Xna.Framework.GamerServices.GamerServicesComponent
    extends Microsoft.Xna.Framework.GameComponent

GamerServicesComponent(Game game)
public void Initialize()
public void Update(GameTime gameTime)
```

No Gamer, SignedInGamer, Guide, Achievement, Avatar, NetworkSession, or leaderboard type belongs to
this profile. None was added.

The outcome is a truthful structural and lifecycle implementation, with the broader service
backend explicitly blocked. The exact XNA IL performs these operations:

```text
constructor: GameComponent(game)
Initialize:  dispatcher.WindowHandle = Game.Window.Handle
             subscribe InstallingTitleUpdate -> Game.Exit
             dispatcher.Initialize(Game.Services)
             base.Initialize()
Update:      dispatcher.Update()
             base.Update(gameTime)
```

CNA ABI 0.7 exposes canonical routes for window assignment, initialization from a game/service
container, and update. CNA-Java calls those three routes in XNA order around the inherited managed
lifecycle. The selected type owns no native object and has no cleanup route.

The private XNA `InstallingTitleUpdate` event is not in the selected public profile, CNA's
canonical component does not subscribe to it, and the current dispatcher never raises an
equivalent automatic event. CNA-Java does not fabricate it. Real sign-in, Guide UI, achievements,
networking, and title-update behavior remain `BACKEND_BLOCKED`; structural strict zero is not a
claim that those separate profiles work.

## CNA qualification and upstream blockers

The qualified ABI 0.7 artifact exports and executes:

```text
cna_gamer_services_dispatcher_set_window_handle(uint64_t)
cna_gamer_services_dispatcher_initialize(CNA_Handle)
cna_gamer_services_dispatcher_update(void)
```

The JNI bridge copies the opaque window token, borrows the live game for initialization, and calls
the process-wide update route. Pointer depth is zero for both input values, `CNA_Handle` and the
window token are 64-bit, there is no boolean representation involved, and return values are
`CNA_Result`. All three calls execute synchronously on the Game lifecycle/owner thread; non-success
results become the existing `CnaNativeException`. There is no allocation, callback context,
ownership transfer, or callback lifetime, and the export verifier confirms every symbol in the
qualified library.

Current read-only CNA HEAD was remeasured at
`1bb2145d99ed572dd4eb15009c34e2e5f410fcf0`:

- networking off plus C API: configuration succeeds; compilation fails because
  `CnaCApiDetail.hpp` includes the unavailable
  `Microsoft/Xna/Framework/GamerServices/GameUpdateRequiredException.hpp`;
- networking on plus C API: configuration succeeds; compilation fails at the renderer identity
  assertion because the measured values are `49 == 50` (false).

Neither upstream checkout nor either blocker was modified. These HEAD build failures do not erase
the usable GamerServices exports already present in the separately qualified ABI 0.7 artifact.

## Verification

Managed tests cover constructor/Game association and inherited Enabled, UpdateOrder, and component
event behavior. Native tests add the component to three successive Games, run one disabled and two
enabled frames in each, verify update-order sorting, observe one initialization and two component
updates, and close every Game normally.

```text
STRICT_STATUS=STRICT_COMPLETE
LIFECYCLE_STATUS=NATIVE_VERIFIED
BROADER_GAMERSERVICES_STATUS=BACKEND_BLOCKED
TITLE_UPDATE_EVENT_STATUS=BACKEND_BLOCKED
NATIVE_OWNERS=0
GAME_RECREATION_CYCLES=3
```

The project-wide result is `265/265` mapped target types, `3206/3206` mapped members, zero
diagnostics, and an empty allowlist. This is strict zero for the selected XNA 4.0 Windows runtime
projection only—not for historical GamerServices/Avatar profiles or universal runtime behavior.
