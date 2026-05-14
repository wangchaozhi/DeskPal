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
- Runtime language switching: system, English, and Simplified Chinese

See `docs/desktop-pet-tech-plan.md` for the desktop pet technical plan.

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
