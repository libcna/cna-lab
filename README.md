# cna-java-template

Modern cross-platform Java starter template for the [CNA](https://github.com/openeggbert/cna) framework, following XNA 4.0 patterns.

## Features

- **Multi-platform support**: Windows, Linux, macOS, Android, iOS, and Web.
- **XNA 4.0 API Style**: Familiar lifecycle (Initialize, LoadContent, Update, Draw).
- **Adaptive Graphics**:
  - **3D**: Renders a rotating textured cube on hardware with 3D capabilities.
  - **2D**: Falling back to a bouncing animated logo on 2D-only renderers.
- **Integrated Renderer Banner**: Displays the active graphics driver name using an internal bitmap font.
- **Gradle Build System**: Modern multi-module setup for easy dependency management and platform targeting.
- **Smoke Test Mode**: Automatic termination for CI/CD validation using the `--smoke-test` flag.

## Project Structure

- `game/`: Shared game logic and assets.
  - `src/main/java/`: Main game class (`HelloGame`) and entry point.
  - `src/main/resources/Content/`: Game assets (textures, etc.).
- `android/`: Android-specific project configuration and Activity.
- `teavm/`: Web support using TeaVM (compiles Java bytecode to JavaScript).
- `gwt/`: Web support using GWT (compiles Java source to JavaScript).

## Getting Started

### Prerequisites

- [Java Development Kit (JDK) 17](https://adoptium.net/) or newer.
- [Gradle](https://gradle.org/install/) (or use the included wrapper if provided).

### Building and Running (Desktop)

To build and run the shared game module on your current desktop platform:

```bash
./gradlew :game:run
```

### Running a Smoke Test

To verify the game starts and renders correctly:

```bash
./gradlew :game:run --args="--smoke-test"
```

### Android

To build the Android APK:

```bash
./gradlew :android:assembleDebug
```

### Web (TeaVM)

To compile the game to JavaScript using TeaVM:

```bash
./gradlew :teavm:teavmCompile
```

### Web (GWT)

To compile the game to JavaScript using GWT:

```bash
./gradlew :gwt:gwtCompile
```

## Supported Engines

The template is designed to work with the **CNA (Java)** binding, but the `HelloGame` logic is kept compatible with standard XNA patterns to allow potential future ports to other Java-based game frameworks.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
