# cna-ts-template

> **Status: In progress - NOT YET FUNCTIONAL**


Multi-platform template for CNA (JavaScript/TypeScript) applications.

## Overview

This template provides a starting point for building games and applications using the [CNA](https://github.com/openeggbert/cna) framework with the TypeScript binding. It is designed to run on:

- **Web** (Browsers)
- **Desktop** (Windows, Linux, macOS via Electron)
- **Mobile** (Android, iOS via Capacitor)

## Features

- **Adaptive Rendering**: Automatically switches between a 3D rotating cube (HiDef/3D capable) and a bouncing 2D logo (Reach/2D only).
- **Renderer Banner**: Displays the current graphics renderer name using an internal bitmap font.
- **TypeScript**: Full type safety and modern JS features.
- **Cross-Platform**: Unified codebase for Web, Desktop, and Mobile.
- **Smoke Test Support**: Includes a `--smoke-test` mode for automated verification.

## Project Structure

- `src/`: TypeScript source code.
  - `HelloGame.ts`: Core game logic (XNA-compatible API).
  - `RendererBanner.ts`: Internal bitmap font renderer.
  - `main.ts`: Entry point.
- `public/Content/`: Game assets (textures, etc.).
- `index.html`: Web entry point.
- `vite.config.ts`: Vite configuration for Web and bundling.
- `capacitor.config.json`: Capacitor configuration for Mobile.

## Getting Started

### Prerequisites

- [Node.js](https://nodejs.org/) (v16 or later)
- [npm](https://www.npmjs.com/) (usually bundled with Node.js)

### Installation

```bash
npm install
```

### Development (Web)

Run a local development server with hot-reload:

```bash
npm run dev
```

### Build (Web)

Compile and bundle for production:

```bash
npm run build
```

### Desktop (Electron)

To run as a desktop application:

```bash
npm run electron:dev
```

### Mobile (Capacitor)

1. Build the web version: `npm run build`
2. Sync with native projects: `npm run cap:sync`
3. Open in Android Studio / Xcode:
   ```bash
   npm run cap:open:android
   npm run cap:open:ios
   ```

## Smoke Test

You can run a quick automated test to verify the template:

```bash
# In Node.js environment
node dist/main.js --smoke-test

# Or in browser by adding ?smoke-test to URL
http://localhost:3000/?smoke-test
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
