# DeskPal

DeskPal is a Qt Quick/QML desktop pet application built with CMake.

## Requirements

- Qt 6.10 or later
- CMake 3.16 or later
- A C++ compiler with C++20 support

## Project Type

This project uses Qt 6, Qt Quick, and Qt Widgets:

- C++ entry point: `main.cpp`
- QML UI: `Main.qml`
- Build system: CMake
- C++ standard: C++20
- Native integration: system tray and settings via C++
- Runtime language switching: system, English, Simplified Chinese, and Japanese

## Pet System

- Settings panel: tray menu → `Pet Settings`. Visual editing of `Profile` / `2D Settings` / `3D Stage`,
  with live preview that mirrors `view3d` changes onto the desktop pet.
- Pet packs live under `assets/pets/<id>/pet.json`. `view3d` (3D camera/light/transform) and
  `pet2d.fps` (PNG sequence frame rate) sit in nested blocks; the loader still accepts legacy
  top-level `fps` for backward compatibility.
- GLB/GLTF assets are loaded at runtime via `QtQuick3D.AssetUtils.RuntimeLoader` and
  the Timeline animation whose `objectName` matches the `animations` map is enabled.
- The `Generate Sample` button installs ready-to-edit copies of `sample_svg_2d` and
  `sample_quick3d` into the writable pets folder.
- `New Pet` initializes from a sample template so new pets ship with working resources
  instead of empty fields.
- Each source/action field has a `…` button that copies a selected file or folder into
  the pet directory and fills in the relative path.
- Validation distinguishes errors (missing files, out-of-range sizes, mismatched
  extensions) from warnings (fields not configured yet, unknown action references).

See `docs/pet-replacement-system.md` for the full pet system reference, and
`docs/desktop-pet-tech-plan.md` for the broader technical plan.

## Build

Open the project folder in Qt Creator and configure it with a Qt 6.10+ kit.

You can also build it from the command line:

```powershell
cmake -S . -B build
cmake --build build
```

## Run

After building, run the generated application executable:

```powershell
.\build\DeskPal.exe
```

The application loads the `Main` QML component from the `DeskPal` QML module.

## Visual Studio Scripts

Use these scripts to configure, build, and launch the app with the Visual Studio 2022 CMake generator:

```powershell
.\run-vs-debug.bat
.\run-vs-release.bat
```

The scripts generate the Visual Studio build tree in `build-vs`.

If `cmake` is not available in PATH, install CMake or set `CMAKE_EXE` before running a script:

```bat
set "CMAKE_EXE=E:\Qt\Tools\CMake_64\bin\cmake.exe"
run-vs-debug.bat
```

The Visual Studio scripts need a Qt MSVC kit, such as `msvc2022_64`.
If it is not installed in a standard Qt folder, set `QT_PREFIX` manually:

```bat
set "QT_PREFIX=E:\Qt\6.11.1\msvc2022_64"
run-vs-debug.bat
```

To copy the Qt runtime DLLs and QML dependencies beside the executable, run:

```bat
deploy-vs-debug.bat
deploy-vs-release.bat
```

The deployment scripts scan only the source `qml` directory and remove stale deployed QML output before running `windeployqt`.

Visual Studio builds place executables under `build-vs\bin\Debug` and `build-vs\bin\Release`.

## CI Release

GitHub Actions can build and publish cross-platform release assets manually.
See `docs/ci-release.md` for the release workflow.
