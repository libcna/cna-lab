# CNA-Ruby desktop canary

This is a real Foundation 1 desktop canary for CNA-Ruby. It creates a native CNA Game, receives native lifecycle/timing callbacks, borrows the game-owned graphics device, decodes the project-owned PNG as a real Texture2D, clears, submits a rotating/scaling/moving SpriteBatch draw, captures actual CNA keyboard state, and exits cleanly.

Qualified evidence currently covers MRI Ruby 3.3.8 on Linux x86-64 with an external CNA C ABI 0.7.0 HEADLESS/NULL artifact. HEADLESS proves lifecycle and graphics command execution; it does not qualify visible renderer output.

Set the native library explicitly:

```sh
export CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so
bundle install
bundle exec ruby main.rb --frames 60
bundle exec ruby main.rb --frames 600
```

`--frames` counts successful native Update and Draw callbacks. Interactive execution without `--frames` exits with the real Escape key state.

The Gemfile path dependency is development-only. Release qualification installs the exact built `.gem` into a clean GEM_HOME and runs this consumer without `RUBYLIB` or a source load path.

Not supported or claimed here: Windows/macOS qualification, browser/Opal, Android/MRuby/Ruboto, Content/XNB, Effects/BasicEffect, 3D/Model, audio, media, or a visible renderer backend.
