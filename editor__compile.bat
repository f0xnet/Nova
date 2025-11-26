@echo off
setlocal enabledelayedexpansion
cls

:: ============================================
:: NOVAENGINE EDITOR - BUILD SCRIPT
:: ============================================

echo.
echo ========================================
echo   NOVAENGINE EDITOR BUILD SYSTEM
echo ========================================
echo.

:: ============================================
:: CONFIGURATION
:: ============================================

set "PROJECT_DIR=C:\Nova"
set "EDITOR_DIR=%PROJECT_DIR%\editor"
set "BIN_DIR=%EDITOR_DIR%\bin\Release"
set "OBJ_DIR=%EDITOR_DIR%\obj\Release"
set "SDK_DIR=%PROJECT_DIR%\sdk\include"

cd "%EDITOR_DIR%"

:: ============================================
:: STEP 1: COMPILE
:: ============================================

echo [STEP 1/2] Compiling editor source files...
echo.

:: Create directories
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

:: Editor source files
set "SOURCE_FILES=main.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EditorApplication.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EditorState.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EditorCamera.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EditorUI.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EntityPalette.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Gizmos.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\EditorHistory.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\SceneSerializer.cpp"

:: Compile each source file with incremental compilation
set COMPILED=0
set SKIPPED=0

for %%f in (%SOURCE_FILES%) do (
    set "SOURCE_PATH=%EDITOR_DIR%\%%f"
    for %%n in ("%%f") do set "BASE_NAME=%%~nxn"
    set "OBJ_FILE=!BASE_NAME:.cpp=.o!"
    set "OBJ_PATH=%OBJ_DIR%\!OBJ_FILE!"

    set "NEEDS_COMPILE=0"

    if not exist "!OBJ_PATH!" (
        set "NEEDS_COMPILE=1"
    ) else (
        for %%s in ("!SOURCE_PATH!") do set "SOURCE_TIME=%%~ts"
        for %%o in ("!OBJ_PATH!") do set "OBJ_TIME=%%~to"
        if "!SOURCE_TIME!" gtr "!OBJ_TIME!" set "NEEDS_COMPILE=1"
    )

    if "!NEEDS_COMPILE!"=="1" (
        echo    Compiling %%f...
        g++ -o "!OBJ_PATH!" -O2 -DNDEBUG -I "%SDK_DIR%" -I "%EDITOR_DIR%\include" -c "!SOURCE_PATH!" -Wall -DSFML_STATIC -std=c++17

        if !ERRORLEVEL! NEQ 0 (
            echo    [ERROR] Compilation failed for %%f
            goto :build_failed
        )
        set /a COMPILED+=1
    ) else (
        set /a SKIPPED+=1
    )
)

echo.
echo    Compiled: %COMPILED% file(s)
echo    Skipped:  %SKIPPED% file(s) (up-to-date)

:: ============================================
:: STEP 2: LINKING
:: ============================================

echo.
echo [STEP 2/2] Linking editor executable...
echo.

g++ -o "%BIN_DIR%\NovaEditor.exe" "%OBJ_DIR%\*.o" ^
    -O2 -DNDEBUG ^
    -I "%SDK_DIR%" ^
    -I "%EDITOR_DIR%\include" ^
    -L "%PROJECT_DIR%\sdk\libs" ^
    -DSFML_STATIC ^
    -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-network-s -lsfml-system-s ^
    -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 ^
    -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 ^
    -std=c++17 -static-libgcc -static-libstdc++ -static

if %ERRORLEVEL% NEQ 0 (
    echo    [ERROR] Linking failed
    goto :build_failed
)

if not exist "%BIN_DIR%\NovaEditor.exe" (
    echo    [ERROR] NovaEditor.exe not created
    goto :build_failed
)

echo    [OK] Editor executable created successfully

:: ============================================
:: BUILD SUCCESS
:: ============================================

:build_success
echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: %BIN_DIR%\NovaEditor.exe
echo.
echo USAGE:
echo   1. Make sure your game data is in C:\Nova\data\
echo   2. Run the editor from project root: cd C:\Nova
echo   3. Execute: editor\bin\Release\NovaEditor.exe
echo.
echo CONTROLS:
echo   WASD / Arrows  - Move camera
echo   Mouse Wheel    - Zoom
echo   Left Click     - Select entity
echo   G              - Toggle grid
echo   Ctrl+S         - Save scene
echo   Delete         - Delete selected
echo   ESC            - Quit
echo.

choice /C YN /M "Run the editor now"
if errorlevel 1 if not errorlevel 2 (
    cd "%PROJECT_DIR%"
    start editor\bin\Release\NovaEditor.exe
    cd "%EDITOR_DIR%"
)

exit /b 0

:: ============================================
:: BUILD FAILED
:: ============================================

:build_failed
echo.
echo ========================================
echo   BUILD FAILED!
echo ========================================
echo.
echo Check the error messages above.
echo.
pause
exit /b 1
