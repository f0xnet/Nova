@echo off
setlocal enabledelayedexpansion
cls

:: ============================================
:: NOVAENGINE - COMPLETE BUILD SCRIPT
:: ============================================

echo.
echo ========================================
echo   NOVAENGINE BUILD SYSTEM
echo ========================================
echo.

:: ============================================
:: CONFIGURATION
:: ============================================

set "PROJECT_DIR=C:\Nova"
set "SOURCE_DIR=C:\Nova\client"
set "BIN_DIR=%SOURCE_DIR%\bin\Release"
set "OBJ_DIR=%SOURCE_DIR%\obj\Release"
set "SDK_DIR=%PROJECT_DIR%\sdk\include"
set "TOOLS_DIR=%PROJECT_DIR%\tools"
set "RH_DIR=%TOOLS_DIR%\ResourceHacker"
set "RH_EXE=%RH_DIR%\ResourceHacker.exe"

<<<<<<< HEAD
cd "%SOURCE_DIR%"
=======
:: Liste des fichiers source
set "files=main.cpp api\nova\RenderClass.cpp api\nova\NovaClass.cpp api\draw\DrawClass.cpp api\nova\SystemClass.cpp api\ui\UIManager.cpp api\ui\UIClass.cpp api\ui\UIGraphic.cpp api\ui\UIString.cpp api\ui\UIbutton.cpp api\network\NetworkManager.cpp api/event/EventHandler.cpp api/event/ButtonEvent.cpp api/nova/SceneManager.cpp api/nova/Scene.cpp api\animation\AnimationManager.cpp api\animation\Animation.cpp"
>>>>>>> 0947907034e9057bbd1b26a647dea7620b0457a2

:: ============================================
:: STEP 1: CREATE DPI MANIFEST
:: ============================================

echo [STEP 1/5] Creating DPI-aware manifest...
echo.

(
echo ^<?xml version="1.0" encoding="UTF-8" standalone="yes"?^>
echo ^<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3"^>
echo   ^<assemblyIdentity
echo     type="win32"
echo     name="Nova.Game"
echo     version="1.0.0.0"
echo     processorArchitecture="*"/^>
echo   ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^>
echo     ^<security^>
echo       ^<requestedPrivileges^>
echo         ^<requestedExecutionLevel level="asInvoker" uiAccess="false"/^>
echo       ^</requestedPrivileges^>
echo     ^</security^>
echo   ^</trustInfo^>
echo   ^<asmv3:application^>
echo     ^<asmv3:windowsSettings^>
echo       ^<dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings"^>true/pm^</dpiAware^>
echo       ^<dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings"^>PerMonitorV2^</dpiAwareness^>
echo     ^</asmv3:windowsSettings^>
echo   ^</asmv3:application^>
echo   ^<compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1"^>
echo     ^<application^>
echo       ^<supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/^>
echo       ^<supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/^>
echo       ^<supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/^>
echo       ^<supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/^>
echo     ^</application^>
echo   ^</compatibility^>
echo ^</assembly^>
) > "%SOURCE_DIR%\app.manifest"

if exist "%SOURCE_DIR%\app.manifest" (
    echo    [OK] Manifest created successfully
) else (
    echo    [WARNING] Failed to create manifest
)

:: ============================================
:: STEP 2: CHECK RESOURCEHACKER
:: ============================================

echo.
echo [STEP 2/5] Checking ResourceHacker...
echo.

if exist "%RH_EXE%" (
    echo    [OK] ResourceHacker found
    set "HAS_RH=1"
) else (
    echo    [WARNING] ResourceHacker not found
    echo    [INFO] Download from: http://www.angusj.com/resourcehacker/
    echo    [INFO] Extract to: %RH_DIR%\
    echo    [INFO] Continuing without DPI manifest embedding...
    set "HAS_RH=0"
)

:: ============================================
:: STEP 3: COMPILE
:: ============================================

echo.
echo [STEP 3/5] Compiling source files...
echo.

:: Create directories
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

:: Copy assets
echo    Copying assets...
xcopy /E /I /Y "%SOURCE_DIR%\assets\" "%BIN_DIR%\data\" >nul 2>&1

:: Compile icon resources
echo    Compiling icon resources...
windres "%SOURCE_DIR%\icon\appicon.rc" -O coff -o "%SOURCE_DIR%\icon\appicon.res" 2>nul
if exist "%SOURCE_DIR%\icon\appicon.res" (
    echo       [OK] Icon compiled
) else (
    echo       [SKIP] Icon not available - continuing without icon
    echo. > "%SOURCE_DIR%\icon\appicon.res"
)

:: Source files list
set "SOURCE_FILES=main.cpp"

set "SOURCE_FILES=%SOURCE_FILES% src\Backend\BackendManager.cpp src\Backend\Core\BackendTypes.cpp src\Backend\SFML\SFMLAudioBackend.cpp src\Backend\SFML\SFMLFontBackend.cpp src\Backend\SFML\SFMLGraphicsBackend.cpp src\Backend\SFML\SFMLInputBackend.cpp src\Backend\SFML\SFMLResourceBackend.cpp src\Backend\SFML\SFMLViewportBackend.cpp src\Backend\SFML\SFMLWindowBackend.cpp"

set "SOURCE_FILES=%SOURCE_FILES% src\game.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\UIManager.cpp src\UI\UIComponent.cpp src\UI\UILoader.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Button.cpp src\UI\Components\Image.cpp src\UI\Components\Text.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Panel.cpp src\UI\Components\Animation.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\TextInput.cpp src\UI\Components\Slider.cpp"
set "SOURCE_FILES=%SOURCE_FILES%  src\Core\ConfigManager.cpp src\Core\Logger.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Resources\ResourceTypes.cpp src\Resources\ResourceManager.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Audio\SoundPlayer.cpp src\Audio\MusicPlayer.cpp src\Audio\AudioManager.cpp"

set "SOURCE_FILES=%SOURCE_FILES% src\Events\EventDispatcher.cpp src\Events\Event.cpp src\Events\EventHandler.cpp"

:: Compile each source file
set COMPILED=0
set SKIPPED=0

echo.
echo    Compiling C++ files...

for %%f in (%SOURCE_FILES%) do (
    set "SOURCE_PATH=%SOURCE_DIR%\%%f"
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
        echo       [COMPILE] %%f
        
        g++ -o "!OBJ_PATH!" -O0 -DNDEBUG -I "%SDK_DIR%" -c "!SOURCE_PATH!" -Wall -DSFML_STATIC 2>&1
        
        if !ERRORLEVEL! NEQ 0 (
            echo.
            echo    ========================================
            echo    [ERROR] Compilation failed for %%f
            echo    ========================================
            echo.
            echo    Check the errors above.
            echo.
            pause
            exit /b 1
        )
        set /a COMPILED+=1
    ) else (
        set /a SKIPPED+=1
    )
)

echo.
echo    Summary:
echo       Compiled: %COMPILED% file(s)
echo       Skipped:  %SKIPPED% file(s) (up-to-date)

:: ============================================
:: STEP 4: LINKING
:: ============================================

echo.
echo [STEP 4/5] Linking executable...
echo.

<<<<<<< HEAD
::  -mwindows pour retirer terminal 

g++ -o "%BIN_DIR%\Nova.exe" "%OBJ_DIR%\*.o" ^
    -O0 -DNDEBUG ^
    -I "%SDK_DIR%" ^
    -L "%PROJECT_DIR%\sdk\libs" ^
    -DSFML_STATIC ^
    -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-network-s -lsfml-system-s ^
    -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 ^
    -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 ^
    -std=c++17 -static-libgcc -static-libstdc++ -static ^
    "%SOURCE_DIR%\icon\appicon.res" 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo    ========================================
    echo    [ERROR] Linking failed
    echo    ========================================
    echo.
    echo    Check the errors above.
    echo.
    pause
    exit /b 1
)

if not exist "%BIN_DIR%\Nova.exe" (
    echo    [ERROR] Nova.exe not created
    pause
    exit /b 1
)

echo    [OK] Executable created successfully

:: ============================================
:: STEP 5: EMBED MANIFEST
:: ============================================

echo.
echo [STEP 5/5] Embedding DPI-aware manifest...
echo.

if "%HAS_RH%"=="0" (
    echo    [SKIP] ResourceHacker not available
    echo    [WARNING] Manifest not embedded - app may be blurry on HiDPI displays
    goto :build_success_no_manifest
)

if not exist "%SOURCE_DIR%\app.manifest" (
    echo    [WARNING] Manifest file not found
    goto :build_success_no_manifest
)

"%RH_EXE%" -open "%BIN_DIR%\Nova.exe" -save "%BIN_DIR%\Nova.exe" -action addoverwrite -res "%SOURCE_DIR%\app.manifest" -mask MANIFEST,1,0 >nul 2>&1

if %ERRORLEVEL% EQU 0 (
    echo    [SUCCESS] DPI-aware manifest embedded!
    echo.
    echo    Your application will display correctly on:
    echo      - High DPI displays (4K, Retina, etc.)
    echo      - Windows scaling 125%%, 150%%, 200%%
    echo      - Multi-monitor setups with different DPI
    goto :build_success
) else (
    echo    [WARNING] Manifest embedding failed
    goto :build_success_no_manifest
)

:: ============================================
:: BUILD SUCCESS WITH MANIFEST
:: ============================================

:build_success
echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Location: %BIN_DIR%\Nova.exe
echo Manifest: DPI-aware (embedded)
echo.
for %%A in ("%BIN_DIR%\Nova.exe") do set SIZE=%%~zA
set /a SIZE_MB=SIZE/1024/1024
echo Size: %SIZE_MB% MB
echo.

choice /C YN /M "Run the game now"
if errorlevel 1 if not errorlevel 2 (
    echo.
    echo Starting Nova...
    cd "%BIN_DIR%"
    start Nova.exe
    cd "%SOURCE_DIR%"
)

exit /b 0

:: ============================================
:: BUILD SUCCESS WITHOUT MANIFEST
:: ============================================

:build_success_no_manifest
echo.
echo ========================================
echo   BUILD SUCCESSFUL (without manifest)
echo ========================================
echo.
echo Location: %BIN_DIR%\Nova.exe
echo Manifest: NOT embedded
echo.
for %%A in ("%BIN_DIR%\Nova.exe") do set SIZE=%%~zA
set /a SIZE_MB=SIZE/1024/1024
echo Size: %SIZE_MB% MB
echo.
echo NOTE: Install ResourceHacker for DPI support:
echo       http://www.angusj.com/resourcehacker/
echo       Extract to: %RH_DIR%\
echo.

choice /C YN /M "Run the game now"
if errorlevel 1 if not errorlevel 2 (
    echo.
    echo Starting Nova...
    cd "%BIN_DIR%"
    start Nova.exe
    cd "%SOURCE_DIR%"
)

exit /b 0
=======
:compile
g++ -o "!object_path!" -O0 -DNDEBUG -I "%sdk_dir%" -c "!source_path!" -Wall -DSFML_STATIC
echo Compiled !source_file! at !source_time!
exit /b
pause
>>>>>>> 0947907034e9057bbd1b26a647dea7620b0457a2
