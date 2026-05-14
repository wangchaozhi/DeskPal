@echo off
setlocal

set "ROOT_DIR=%~dp0"
set "ROOT_DIR=%ROOT_DIR:~0,-1%"
set "BUILD_DIR=%ROOT_DIR%\build-vs"
set "APP_EXE=%BUILD_DIR%\Debug\DeskPal.exe"

if not defined CMAKE_EXE (
    for /f "delims=" %%C in ('where cmake.exe 2^>nul') do if not defined CMAKE_EXE set "CMAKE_EXE=%%C"
)

if not defined CMAKE_EXE if exist "E:\Qt\Tools\CMake_64\bin\cmake.exe" set "CMAKE_EXE=E:\Qt\Tools\CMake_64\bin\cmake.exe"
if not defined CMAKE_EXE if exist "E:\qt\Tools\CMake_64\bin\cmake.exe" set "CMAKE_EXE=E:\qt\Tools\CMake_64\bin\cmake.exe"
if not defined CMAKE_EXE if exist "C:\Qt\Tools\CMake_64\bin\cmake.exe" set "CMAKE_EXE=C:\Qt\Tools\CMake_64\bin\cmake.exe"
if not defined CMAKE_EXE if exist "C:\Program Files\CMake\bin\cmake.exe" set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not defined CMAKE_EXE (
    echo CMake was not found.
    echo Install CMake, add it to PATH, or run this script after setting CMAKE_EXE.
    echo Example: set "CMAKE_EXE=C:\Qt\Tools\CMake_64\bin\cmake.exe"
    exit /b 1
)

if not defined QT_PREFIX if exist "E:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" set "QT_PREFIX=E:\Qt\6.11.1\msvc2022_64"
if not defined QT_PREFIX if exist "E:\qt\6.11.1\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" set "QT_PREFIX=E:\qt\6.11.1\msvc2022_64"

if not defined QT_PREFIX (
    for /d %%Q in ("E:\Qt\6.*\msvc*_64" "E:\qt\6.*\msvc*_64" "C:\Qt\6.*\msvc*_64") do (
        if exist "%%~Q\lib\cmake\Qt6\Qt6Config.cmake" if not defined QT_PREFIX set "QT_PREFIX=%%~Q"
    )
)

if not defined QT_PREFIX (
    echo Qt for MSVC was not found.
    echo Install a Qt MSVC kit, for example msvc2022_64, or set QT_PREFIX manually.
    echo Example: set "QT_PREFIX=E:\Qt\6.11.1\msvc2022_64"
    exit /b 1
)

set "PATH=%QT_PREFIX%\bin;%PATH%"

"%CMAKE_EXE%" -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_PREFIX%"
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" --build "%BUILD_DIR%" --config Debug
if errorlevel 1 exit /b %errorlevel%

start "" "%APP_EXE%"
