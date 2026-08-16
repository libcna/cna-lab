# cna-go-template

> **Status: In progress - ZATÍM NEFUNKČNÍ**


Modern template for CNA applications using the Go programming language.

## Features

- **Adaptive Rendering**: Automatically switches between a 3D rotating cube (3D capable) and a bouncing 2D logo (2D only).
- **Renderer Banner**: Displays the name of the active graphics renderer during the first 5 seconds.
- **Cross-Platform**: Designed to run on Windows, Linux, macOS, Android, iOS, and Web (WebAssembly).
- **XNA 4.0 API Style**: Uses PascalCase for methods and follows familiar XNA 4.0 patterns.

## Getting Started

### Prerequisites

- [Go 1.21+](https://golang.org/dl/)
- For Android: [gomobile](https://pkg.go.dev/golang.org/x/mobile/cmd/gomobile)

### Building and Running

#### Desktop (Windows, Linux, macOS)
```bash
go run ./cmd/desktop
```

#### Web (WebAssembly)
```bash
GOOS=js GOARCH=wasm go build -o game.wasm ./cmd/wasm
```

#### Android
```bash
gomobile build -target=android ./android
```

### Automation

The template supports a "smoke test" mode which runs for a few frames and then exits. This is useful for CI/CD pipelines.

```bash
go run ./cmd/desktop --smoke-test
```

## Project Structure

- `game/`: Shared game logic (`HelloGame.go`).
- `cmd/desktop/`: Entry point for desktop platforms.
- `cmd/wasm/`: Entry point for WebAssembly.
- `android/`: Entry point for Android (gomobile).
- `Content/`: Asset files (textures, etc.).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
