# CNA-Swift native stress

`run.py` launches each lifetime family as a separate XCTest process. It never
accepts a relative native-library path. The selected tests cover 20 Game
recreations, 20 Texture2D and SpriteBatch lifetimes, 20 callback failures,
double dispose, parent-before-child cleanup, child-before-parent cleanup,
wrong-thread refusal with owner-thread retry, decode-create rollback, and a
retained callback-borrowed GraphicsDevice.

The GamePad modes additionally cover 50 calls per dead-zone route, 20
capability calls, generation replacement, and safe wrong-thread preflight.
Repeated vibration stress is deliberately omitted without qualified controller
hardware.

Ordinary successful runs establish crash/UAF/double-free observations only;
they are not allocator leak proof. Native sanitizer claims require a separately
instrumented CNA library.
