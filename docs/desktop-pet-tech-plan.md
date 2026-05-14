# Desktop Pet Technical Plan

## Goal

Build a small Windows desktop pet application with Qt 6, QML, and C++20.
The first milestone focuses on a usable MVP: transparent window, draggable pet, context menu, tray icon, saved position, and deployment scripts.

## Technology Stack

- Qt 6.11.1
- Qt Quick / QML for the desktop pet UI
- C++20 for native integration
- Qt Widgets for `QSystemTrayIcon`
- CMake
- MSVC 2022 64-bit
- `windeployqt` for runtime deployment

## Architecture

The app is split into two layers:

- QML presentation layer: transparent pet window, animation, and mouse interaction.
- C++ native layer: system tray, native context menu, app lifetime, settings persistence, and future Windows integrations.

Current main files:

- `main.cpp`: application bootstrap, QML engine, and C++ controller registration.
- `src/AppController.h/.cpp`: QML-facing facade and application command coordinator.
- `src/SettingsStore.h/.cpp`: persisted application settings.
- `src/TrayController.h/.cpp`: system tray icon and native context menu.
- `src/TranslationManager.h/.cpp`: Qt and application translation loading.
- `Main.qml`: root QML entry.
- `qml/PetWindow.qml`: transparent desktop window and interactions.
- `qml/PetBody.qml`: placeholder pet visual and idle animation.

## MVP Features

- Transparent frameless desktop pet window.
- Always-on-top toggle.
- Drag the pet with the left mouse button.
- Clamp pet movement to the current screen's available work area.
- Save and restore the last window position.
- Native right-click menu with topmost, hide, reset position, and quit.
- System tray menu with show, hide, topmost, reset position, and quit.
- Runtime language selection with system, English, and Simplified Chinese options.
- Debug and Release Visual Studio build scripts.
- Debug and Release deployment scripts using `windeployqt`.

## Next Milestones

1. Replace the placeholder QML pet with sprite assets.
2. Add pet states: idle, clicked, dragged, sleeping, and walking.
3. Add random movement and screen-edge constraints.
4. Add speech bubbles and configurable text.
5. Add a settings panel for scale, opacity, topmost, and startup behavior.
6. Add Windows startup registration.
7. Add packaged release output.

## Build

```bat
run-vs-debug.bat
run-vs-release.bat
```

## Deploy

```bat
deploy-vs-debug.bat
deploy-vs-release.bat
```

The deployed application directories are:

- `build-vs\Debug`
- `build-vs\Release`
