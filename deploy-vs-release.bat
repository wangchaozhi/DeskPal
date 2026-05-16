@echo off
setlocal

set "ROOT_DIR=%~dp0"
set "ROOT_DIR=%ROOT_DIR:~0,-1%"
set "BUILD_DIR=%ROOT_DIR%\build-vs"
set "DEPLOY_DIR=%BUILD_DIR%\bin\Release"
set "APP_EXE=%BUILD_DIR%\bin\Release\DeskPal.exe"

if not defined QT_PREFIX if exist "E:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" set "QT_PREFIX=E:\Qt\6.11.1\msvc2022_64"
if not defined QT_PREFIX if exist "E:\qt\6.11.1\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" set "QT_PREFIX=E:\qt\6.11.1\msvc2022_64"

if not defined QT_PREFIX (
    for /d %%Q in ("E:\Qt\6.*\msvc*_64" "E:\qt\6.*\msvc*_64" "C:\Qt\6.*\msvc*_64") do (
        if exist "%%~Q\bin\windeployqt.exe" if not defined QT_PREFIX set "QT_PREFIX=%%~Q"
    )
)

if not defined QT_PREFIX (
    echo Qt for MSVC was not found.
    echo Example: set "QT_PREFIX=E:\Qt\6.11.1\msvc2022_64"
    exit /b 1
)

if not exist "%APP_EXE%" (
    echo Release executable was not found. Build it first:
    echo run-vs-release.bat
    exit /b 1
)

if exist "%DEPLOY_DIR%\appQt_Template.exe" del /q "%DEPLOY_DIR%\appQt_Template.exe"
if exist "%DEPLOY_DIR%\qml" rmdir /s /q "%DEPLOY_DIR%\qml"
if exist "%DEPLOY_DIR%\Qt6QuickControls2.dll" del /q "%DEPLOY_DIR%\Qt6QuickControls2*.dll"

"%QT_PREFIX%\bin\windeployqt.exe" --release --qmldir "%ROOT_DIR%\qml" ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    --no-ffmpeg ^
    "%APP_EXE%"
if errorlevel 1 exit /b %errorlevel%

xcopy /e /i /y "%ROOT_DIR%\assets" "%DEPLOY_DIR%\assets" >nul
if errorlevel 1 exit /b %errorlevel%

echo Release deployment finished: %DEPLOY_DIR%
