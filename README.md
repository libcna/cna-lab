# CNA Ruby Template

> **Status: In progress - ZATÍM NEFUNKČNÍ**


A multi-platform Ruby game template using the CNA framework (XNA 4.0 compatible API).

## Features
- **Desktop**: Windows, Linux, macOS (via standard Ruby)
- **Web**: Browser support (via Opal)
- **Mobile**: Android (via MRuby/Ruboto)
- **Adaptive Graphics**: Automatic 3D Cube (HiDef) or 2D Logo (Reach) rendering.
- **Renderer Banner**: In-game banner showing the active renderer.

## Prerequisites
- [Ruby](https://www.ruby-lang.org/) (3.0+)
- [Bundler](https://bundler.io/)

## Getting Started

### Desktop
1. Install dependencies:
   ```bash
   bundle install
   ```
2. Run the game:
   ```bash
   ruby main.rb
   ```

### Web (Opal)
1. Build the web version:
   ```bash
   bundle exec opal -c game/HelloGame.rb -o web/game.js
   ```
2. Open `web/index.html` in your browser.

### Android
Refer to `android/README.md` for instructions on building with MRuby.

## Project Structure
- `game/`: Shared game logic (XNA-like).
- `main.rb`: Desktop entry point.
- `web/`: Web-specific assets and entry point.
- `android/`: Android-specific configuration.
- `Content/`: Game assets (textures, etc.).

## Smoke Test
Run a quick automated test to verify the engine loop:
```bash
ruby main.rb --smoke-test
```
