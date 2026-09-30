# Upstream findings: the online profile

Re-measured 2026-09-30 against CNA C ABI `0.35.0` (CNA `next` `4228ff913`), on the
HEADLESS and OPENGLES3 artifacts named in `docs/runtime-capabilities.json`. The
2026-09-02 write-ups against `0.21.0` are in Git history before commit `3e098d3`.
C reproducers live in the shared, ignored CNA probe directory
`cna/build-probe/qual-probes/`.

| # | Finding (0.21) | Now | Evidence |
|---|---|---|---|
| 1 | `cna_packet_reader_read_color` consumed 16 bytes where the writer packs 4 | **Fixed upstream** (BINDFIX-022): a colour round-trips in 4 bytes, as XNA's IL does. `net.h` still documents the old four-float asymmetry -- a documentation defect. | `PacketTests.test_a_colour_round_trips_through_a_packet`; `py-packet-color.c` (reads back 10,20,30,40 consuming 4) |
| 2 | `cna_avatar_description_create_random_for_body_type` ignored its argument | **Fixed upstream** (ABI 0.33 real avatars) | `AvatarTests.test_the_body_type_argument_is_honoured` |
| 3 | `cna_gamer_*` reject network-gamer handles; `net_gamers.h` has no gamertag route | **Still reproduces.** A remote `NetworkGamer`'s inherited `Gamertag`/`DisplayName`/`GetProfile` raise `NotImplementedError` naming the gap; a local one resolves through its signed-in gamer. | `NetworkSessionTests.test_a_remote_gamers_inherited_members_name_the_missing_route`; `py-network-gamer-gamertag.c` (`cna_gamer_get_gamertag_size` -> 2 "The handle does not name a gamer this call can use.") |
| 4 | `cna_network_session_destroy` refused a disposed session, so XNA's `Dispose` leaked it | **Fixed upstream.** `Dispose` now calls `dispose` then `destroy`, after releasing every gamer view and machine copy taken from the session (CNA refuses `destroy` while one is open). | `SessionLifetimeTests.test_dispose_ends_the_session_and_releases_its_handle` |
| 5 | `cna_network_session_replace_session_properties` declared but not exported | **Fixed upstream**: exported; the `PENDING_ROUTES` stub is removed. | `tools/audit_cna_abi.py` `MISSING_SYMBOLS=0` |

Re-measured behaviour that tests used to pin as XNA's and that XNA's IL
contradicts, now asserted the XNA way: `ReceiveData(PacketReader)` returns the
packet size (CNA GS-007m); `NetworkSessionProperties` has eight fixed slots and
refuses an index past 7; `Guide.IsTrialMode` is latched from `SimulateTrialMode`
at each gamer-services update, and `Guide.IsVisible` is true exactly while a
Guide screen is up (the setters were retired at ABI 0.34).
