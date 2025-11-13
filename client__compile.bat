@echo off
setlocal enabledelayedexpansion
cls

:: ============================================
:: NOVAENGINE - BUILD SCRIPT WITH DPI SUPPORT
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

cd "%SOURCE_DIR%"

:: ============================================
:: STEP 1: CREATE DPI-AWARE MANIFEST
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
echo.    
echo   ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^>
echo     ^<security^>
echo       ^<requestedPrivileges^>
echo         ^<requestedExecutionLevel level="asInvoker" uiAccess="false"/^>
echo       ^</requestedPrivileges^>
echo     ^</security^>
echo   ^</trustInfo^>
echo.  
echo   ^<asmv3:application^>
echo     ^<asmv3:windowsSettings^>
echo       ^<dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings"^>true/pm^</dpiAware^>
echo       ^<dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings"^>PerMonitorV2^</dpiAwareness^>
echo     ^</asmv3:windowsSettings^>
echo   ^</asmv3:application^>
echo.  
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
    echo    [ERROR] Failed to create manifest
    goto :build_failed
)

:: ============================================
:: STEP 2: SETUP RESOURCEHACKER
:: ============================================

echo.
echo [STEP 2/5] Setting up ResourceHacker...
echo.

if exist "!RH_EXE!" (
    echo    [OK] ResourceHacker already installed
    goto :compile
)

echo    ResourceHacker not found, installing...
echo.

if not exist "%RH_DIR%" mkdir "%RH_DIR%"

echo    Downloading ResourceHacker...
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; try { Invoke-WebRequest -Uri 'http://www.angusj.com/resourcehacker/resource_hacker.zip' -OutFile '%RH_DIR%\rh.zip' -UseBasicParsing -TimeoutSec 30 } catch { exit 1 }}" 2>nul

if not exist "%RH_DIR%\rh.zip" (
    echo    [ERROR] Download failed
    echo.
    echo    Please download manually from:
    echo    http://www.angusj.com/resourcehacker/
    echo    Extract to: %RH_DIR%\
    echo.
    choice /C YN /M "Continue without manifest embedding"
    if errorlevel 2 goto :build_failed
    set "RH_EXE="
    goto :compile
)

echo    Extracting...
powershell -Command "Expand-Archive -Path '%RH_DIR%\rh.zip' -DestinationPath '%RH_DIR%' -Force" 2>nul

if exist "!RH_EXE!" (
    echo    [OK] ResourceHacker installed successfully
    del "%RH_DIR%\rh.zip" 2>nul
) else (
    echo    [WARNING] Extraction failed, continuing without manifest embedding
    set "RH_EXE="
)

:: ============================================
:: STEP 3: COMPILE
:: ============================================

:compile
echo.
echo [STEP 3/5] Compiling source files...
echo.

:: Create directories
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

:: Copy assets
xcopy /E /I /Y "%SOURCE_DIR%\assets\" "%BIN_DIR%\data\" >nul 2>&1

:: Compile resources
echo    Compiling resources...
windres "%SOURCE_DIR%\icon\appicon.rc" -O coff -o "%SOURCE_DIR%\icon\appicon.res"

if not exist "%SOURCE_DIR%\icon\appicon.res" (
    echo    [WARNING] Icon resource compilation failed
)

:: Source files list
set "SOURCE_FILES=main.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\game.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Backend\BackendManager.cpp src\Backend\Core\BackendTypes.cpp src\Backend\SFML\SFMLAudioBackend.cpp src\Backend\SFML\SFMLFontBackend.cpp src\Backend\SFML\SFMLGraphicsBackend.cpp src\Backend\SFML\SFMLInputBackend.cpp src\Backend\SFML\SFMLResourceBackend.cpp src\Backend\SFML\SFMLViewportBackend.cpp src\Backend\SFML\SFMLWindowBackend.cpp
set "SOURCE_FILES=%SOURCE_FILES% src\UI\UIManager.cpp src\UI\UIComponent.cpp src\UI\UILoader.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Button.cpp src\UI\Components\Image.cpp src\UI\Components\Text.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Panel.cpp src\UI\Components\Animation.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\TextInput.cpp src\UI\Components\Slider.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Core\NovaEngine.cpp src\Core\ConfigManager.cpp src\Core\Logger.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Resources\ResourceTypes.cpp src\Resources\ResourceManager.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Audio\SoundPlayer.cpp src\Audio\MusicPlayer.cpp src\Audio\AudioManager.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Events\EventDispatcher.cpp src\Events\Event.cpp src\Events\EventHandler.cpp"

:: Compile each source file
set COMPILED=0
set SKIPPED=0
set FAILED=0

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
        echo    Compiling %%f...
        g++ -o "!OBJ_PATH!" -O0 -DNDEBUG -I "%SDK_DIR%" -c "!SOURCE_PATH!" -Wall -DSFML_STATIC
        
        if !ERRORLEVEL! NEQ 0 (
            echo    [ERROR] Compilation failed for %%f
            set /a FAILED+=1
        ) else (
            set /a COMPILED+=1
        )
    ) else (
        set /a SKIPPED+=1
    )
)

echo.
echo    Compiled: %COMPILED% file(s)
echo    Skipped:  %SKIPPED% file(s) (up-to-date)
if %FAILED% GTR 0 (
    echo    Failed:   %FAILED% file(s)
    goto :build_failed
)

:: ============================================
:: STEP 4: LINKING
:: ============================================

echo.
echo [STEP 4/5] Linking executable...
echo.

g++ -o "%BIN_DIR%\Nova.exe" "%OBJ_DIR%\*.o" ^
    -O0 -DNDEBUG ^
    -I "%SDK_DIR%" ^
    -L "%PROJECT_DIR%\sdk\libs" ^
    -DSFML_STATIC ^
    -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-network-s -lsfml-system-s ^
    -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 ^
    -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 ^
    -std=c++17 -static-libgcc -static-libstdc++ -static ^
    "%SOURCE_DIR%\icon\appicon.res"

if %ERRORLEVEL% NEQ 0 (
    echo    [ERROR] Linking failed
    goto :build_failed
)

if not exist "%BIN_DIR%\Nova.exe" (
    echo    [ERROR] Nova.exe not created
    goto :build_failed
)

echo    [OK] Executable created successfully

:: ============================================
:: STEP 5: EMBED MANIFEST
:: ============================================

echo.
echo [STEP 5/5] Embedding DPI-aware manifest...
echo.

if not defined RH_EXE (
    echo    [WARNING] ResourceHacker not available
    echo    [WARNING] Manifest not embedded - app may be blurry on HiDPI displays
    goto :build_success_no_manifest
)

if not exist "!RH_EXE!" (
    echo    [WARNING] ResourceHacker not found
    echo    [WARNING] Manifest not embedded - app may be blurry on HiDPI displays
    goto :build_success_no_manifest
)

if not exist "%SOURCE_DIR%\app.manifest" (
    echo    [ERROR] Manifest file not found
    goto :build_success_no_manifest
)

"!RH_EXE!" -open "%BIN_DIR%\Nova.exe" -save "%BIN_DIR%\Nova.exe" -action addoverwrite -res "%SOURCE_DIR%\app.manifest" -mask MANIFEST,1,0 >nul 2>&1

set RH_ERROR=!ERRORLEVEL!

if !RH_ERROR! EQU 0 (
    echo    [SUCCESS] DPI-aware manifest embedded!
    echo.
    echo    Your application will now display correctly on:
    echo    - High DPI displays (4K, Retina, etc.)
    echo    - Windows scaling 125%%, 150%%, 200%%
    echo    - Multi-monitor setups with different DPI
    goto :build_success
) else (
    echo    [WARNING] Manifest embedding failed (error: !RH_ERROR!)
    goto :build_success_no_manifest
)

:: ============================================
:: BUILD SUCCESS
:: ============================================

:build_success
echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: %BIN_DIR%\Nova.exe
echo Manifest:   Embedded (DPI-aware)
echo.

choice /C YN /M "Run the game now"
if errorlevel 1 if not errorlevel 2 (
    cd "%BIN_DIR%"
    start Nova.exe
    cd "%SOURCE_DIR%"
)

exit /b 0

:build_success_no_manifest
echo.
echo ========================================
echo   BUILD SUCCESSFUL (without manifest)
echo ========================================
echo.
echo Executable: %BIN_DIR%\Nova.exe
echo Manifest:   NOT embedded
echo.
echo NOTE: Install ResourceHacker for DPI support:
echo       http://www.angusj.com/resourcehacker/
echo.

choice /C YN /M "Run the game now"
if errorlevel 1 if not errorlevel 2 (
    cd "%BIN_DIR%"
    start Nova.exe
    cd "%SOURCE_DIR%"
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