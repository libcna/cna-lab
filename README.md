# CNA-Ruby desktop canary

A real CNA-Ruby consumer: it creates a native CNA Game, receives the native lifecycle and timing
callbacks, borrows the game-owned graphics device, decodes the project's own PNG into a Texture2D,
clears, submits a rotating/scaling/moving SpriteBatch draw, reads the real keyboard state, and exits
cleanly.

## Qualified evidence (re-measured 2026-09-30)

MRI Ruby 3.3.8 on Linux x86-64, CNA C ABI **0.35.0** built from CNA `next`, consuming the exact
built `cna-ruby-0.1.0.dev0.gem` installed into an isolated `GEM_HOME` -- not the `path:` Gemfile
dependency and not a source load path:

| Artifact | `--frames 60` | `--frames 600` |
| --- | --- | --- |
| HEADLESS (`~/deps/cna-c-abi-0.35.0`) | 60 draws, 60 updates, exit 0 | 600 draws, 600 updates, exit 0 |
| OPENGLES3 (`~/deps/cna-c-abi-0.35.0-opengles3-fx`, private Xwayland on the real GPU) | 60 draws, 85 updates, exit 0 | 600 draws, 911 updates, exit 0 |

`--frames` counts Draw calls. XNA's fixed time step runs as many Updates as it needs before each
Draw, so on a renderer whose Present waits for the display there are more Updates than Draws; the
canary requires exactly the requested Draws and at least as many Updates. There is no pixel
readback in this canary. Windows, macOS, browser, Android and a physical display are not qualified.

## Running it

From the exact gem (release qualification):

```sh
gem build ../cna-ruby/cna-ruby.gemspec
GEM_HOME=/path/to/isolated GEM_PATH=/path/to/isolated gem install --local cna-ruby-0.1.0.dev0.gem
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so GEM_HOME=/path/to/isolated GEM_PATH=/path/to/isolated \
  ruby -e 'gem "cna-ruby"; load "main.rb"' -- --frames 60
```

For development the Gemfile's `path:` dependency on `../cna-ruby` works with
`bundle exec ruby main.rb --frames 60`. A windowed artifact must run on a private display
(on the CNA host: `cna/tools/platform/run_gpu_tests_private.sh --exec ...` with
`SDL_VIDEODRIVER=x11` and `WAYLAND_DISPLAY` unset), never on a live desktop.
