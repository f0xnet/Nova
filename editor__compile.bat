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

:: Editor source files (Phase 1: Minimal JSON-based UI)
set "EDITOR_FILES=main.cpp"
set "EDITOR_FILES=%EDITOR_FILES% src\EditorApplication.cpp"
set "EDITOR_FILES=%EDITOR_FILES% src\EditorCamera.cpp"
set "EDITOR_FILES=%EDITOR_FILES% src\Core\EditorState.cpp"
set "EDITOR_FILES=%EDITOR_FILES% src\Core\EditorConfig.cpp"
set "EDITOR_FILES=%EDITOR_FILES% src\UI\EditorUIManager.cpp"

:: Engine source files (from client)
set "ENGINE_DIR=%PROJECT_DIR%\client"
set "ENGINE_FILES=src\Backend\BackendManager.cpp src\Backend\Core\BackendTypes.cpp src\Backend\SFML\SFMLAudioBackend.cpp src\Backend\SFML\SFMLFontBackend.cpp src\Backend\SFML\SFMLGraphicsBackend.cpp src\Backend\SFML\SFMLInputBackend.cpp src\Backend\SFML\SFMLResourceBackend.cpp src\Backend\SFML\SFMLViewportBackend.cpp src\Backend\SFML\SFMLWindowBackend.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\UI\UIManager.cpp src\UI\UIComponent.cpp src\UI\UILoader.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\UI\Components\Button.cpp src\UI\Components\Image.cpp src\UI\Components\Text.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\UI\Components\Panel.cpp src\UI\Components\Animation.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\UI\Components\TextInput.cpp src\UI\Components\Slider.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Core\NovaEngine.cpp src\Core\ConfigManager.cpp src\Core\Logger.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\ECS\Entity.cpp src\ECS\EntityRegistry.cpp src\ECS\Scene.cpp src\ECS\SceneManager.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\ECS\Components\TransformComponent.cpp src\ECS\Components\SpriteComponent.cpp src\ECS\Components\LightComponent.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Resources\ResourceTypes.cpp src\Resources\ResourceManager.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Audio\SoundPlayer.cpp src\Audio\MusicPlayer.cpp src\Audio\AudioManager.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Events\EventDispatcher.cpp src\Events\Event.cpp src\Events\EventHandler.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Rendering\PostProcessManager.cpp src\Rendering\PostProcessPipeline.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Rendering\Effects\CRTEffect.cpp src\Rendering\Effects\SSAOEffect.cpp src\Rendering\Effects\BloomEffect.cpp src\Rendering\Effects\ColorGradingEffect.cpp src\Rendering\Effects\DynamicLightingEffect.cpp"
set "ENGINE_FILES=%ENGINE_FILES% src\Systems\LightingSystem.cpp"

:: Compile editor source files with incremental compilation
set COMPILED=0
set SKIPPED=0

echo    Compiling editor files...
for %%f in (%EDITOR_FILES%) do (
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
        echo       [Editor] %%f
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

echo    Compiling engine files...
for %%f in (%ENGINE_FILES%) do (
    set "SOURCE_PATH=%ENGINE_DIR%\%%f"
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
        echo       [Engine] %%f
        g++ -o "!OBJ_PATH!" -O2 -DNDEBUG -I "%SDK_DIR%" -c "!SOURCE_PATH!" -Wall -DSFML_STATIC -std=c++17

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
