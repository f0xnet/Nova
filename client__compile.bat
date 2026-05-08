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
set "LIB_DIR=%PROJECT_DIR%\sdk\libs"
set "LUA_SRC=%PROJECT_DIR%\deps\lua-5.4.7\src"
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
if not exist "%OBJ_DIR%\nlohmann" mkdir "%OBJ_DIR%\nlohmann"
if not exist "%OBJ_DIR%\sol"      mkdir "%OBJ_DIR%\sol"

:: Copy assets
xcopy /E /I /Y "%SOURCE_DIR%\assets\" "%BIN_DIR%\data\" >nul 2>&1
if exist "%SOURCE_DIR%\assets\data\" (
    xcopy /E /I /Y "%SOURCE_DIR%\assets\data\" "%BIN_DIR%\data\" >nul 2>&1
)

:: Compile resources
echo    Compiling resources...
windres "%SOURCE_DIR%\icon\appicon.rc" -O coff -o "%SOURCE_DIR%\icon\appicon.res"

if not exist "%SOURCE_DIR%\icon\appicon.res" (
    echo    [WARNING] Icon resource compilation failed
)

:: Lua 5.4 — compile si la lib est absente
if not exist "%LIB_DIR%\lua54.lib" (
    echo    Compiling Lua 5.4.7...
    if not exist "%OBJ_DIR%\lua" mkdir "%OBJ_DIR%\lua"
    gcc -O2 -c ^
        "%LUA_SRC%\lapi.c"    "%LUA_SRC%\lauxlib.c" "%LUA_SRC%\lbaselib.c" ^
        "%LUA_SRC%\lcode.c"   "%LUA_SRC%\lcorolib.c" "%LUA_SRC%\lctype.c" ^
        "%LUA_SRC%\ldblib.c"  "%LUA_SRC%\ldebug.c"  "%LUA_SRC%\ldo.c" ^
        "%LUA_SRC%\ldump.c"   "%LUA_SRC%\lfunc.c"   "%LUA_SRC%\lgc.c" ^
        "%LUA_SRC%\linit.c"   "%LUA_SRC%\liolib.c"  "%LUA_SRC%\llex.c" ^
        "%LUA_SRC%\lmathlib.c" "%LUA_SRC%\lmem.c"   "%LUA_SRC%\loadlib.c" ^
        "%LUA_SRC%\lobject.c" "%LUA_SRC%\lopcodes.c" "%LUA_SRC%\loslib.c" ^
        "%LUA_SRC%\lparser.c" "%LUA_SRC%\lstate.c"  "%LUA_SRC%\lstring.c" ^
        "%LUA_SRC%\lstrlib.c" "%LUA_SRC%\ltable.c"  "%LUA_SRC%\ltablib.c" ^
        "%LUA_SRC%\ltm.c"     "%LUA_SRC%\lundump.c" "%LUA_SRC%\lutf8lib.c" ^
        "%LUA_SRC%\lvm.c"     "%LUA_SRC%\lzio.c"
    if !ERRORLEVEL! NEQ 0 ( echo    [ERROR] Lua compilation failed & goto :build_failed )
    move *.o "%OBJ_DIR%\lua\" >nul
    ar rcs "%LIB_DIR%\lua54.lib" "%OBJ_DIR%\lua\*.o"
    echo    [OK] lua54.lib ready
) else (
    echo    [OK] lua54.lib already built, skipping.
)
echo.

:: PCH — nlohmann/json.hpp
set "JSON_PCH_SRC=%SDK_DIR%\nlohmann\json.hpp"
set "JSON_PCH_OUT=%OBJ_DIR%\nlohmann\json.hpp.gch"
powershell -NoProfile -Command "exit [int](-not (Test-Path '%JSON_PCH_OUT%') -or (Get-Item '%JSON_PCH_SRC%').LastWriteTime -gt (Get-Item '%JSON_PCH_OUT%').LastWriteTime)"
if %ERRORLEVEL% EQU 1 (
    echo    Compiling precompiled header ^(json.hpp^)...
    g++ -x c++-header "%JSON_PCH_SRC%" -o "%JSON_PCH_OUT%" ^
        -I "%SDK_DIR%" -DSFML_STATIC -std=c++17 -O2
    if !ERRORLEVEL! NEQ 0 ( echo    [ERROR] json.hpp PCH failed & goto :build_failed )
    echo    [OK] json.hpp PCH ready.
) else (
    echo    [OK] json.hpp PCH up-to-date, skipping.
)

:: PCH — sol/sol.hpp  ^(sol2 — header-only, très lourd en templates^)
set "SOL_PCH_SRC=%SDK_DIR%\sol\sol.hpp"
set "SOL_PCH_OUT=%OBJ_DIR%\sol\sol.hpp.gch"
powershell -NoProfile -Command "exit [int](-not (Test-Path '%SOL_PCH_OUT%') -or (Get-Item '%SOL_PCH_SRC%').LastWriteTime -gt (Get-Item '%SOL_PCH_OUT%').LastWriteTime)"
if %ERRORLEVEL% EQU 1 (
    echo    Compiling precompiled header ^(sol/sol.hpp^)...
    g++ -x c++-header "%SOL_PCH_SRC%" -o "%SOL_PCH_OUT%" ^
        -I "%SDK_DIR%" -DSFML_STATIC -std=c++17 -O2
    if !ERRORLEVEL! NEQ 0 ( echo    [ERROR] sol/sol.hpp PCH failed & goto :build_failed )
    echo    [OK] sol/sol.hpp PCH ready.
) else (
    echo    [OK] sol/sol.hpp PCH up-to-date, skipping.
)
echo.

:: -------------------------------------------------------
:: Script PowerShell pour le check de dépendances
:: Fichier commité dans tools/ — pas de génération dynamique.
:: Passe les chemins via variables d'environnement pour
:: éviter les problèmes d'échappement de guillemets.
::
:: Exit 1 = recompilation nécessaire
:: Exit 0 = à jour
:: -------------------------------------------------------
set "PS_DEP_CHECK=%PROJECT_DIR%\tools\nova_dep_check.ps1"

:: Source files list
set "SOURCE_FILES=main.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Game.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Dialogue\DialogueSystem.cpp src\Player\PlayerController.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Backend\BackendManager.cpp src\Backend\Core\BackendTypes.cpp src\Backend\SFML\SFMLAudioBackend.cpp src\Backend\SFML\SFMLFontBackend.cpp src\Backend\SFML\SFMLGraphicsBackend.cpp src\Backend\SFML\SFMLInputBackend.cpp src\Backend\SFML\SFMLResourceBackend.cpp src\Backend\SFML\SFMLViewportBackend.cpp src\Backend\SFML\SFMLWindowBackend.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\UIManager.cpp src\UI\UIComponent.cpp src\UI\UILoader.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Button.cpp src\UI\Components\Image.cpp src\UI\Components\Text.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\Panel.cpp src\UI\Components\Animation.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\UI\Components\TextInput.cpp src\UI\Components\Slider.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Core\ConfigManager.cpp src\Core\Logger.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Resources\ResourceTypes.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Audio\AudioManager.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Events\EventDispatcher.cpp src\Events\Event.cpp src\Events\EventHandler.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Rendering\PostProcessManager.cpp src\Rendering\PostProcessPipeline.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Rendering\Effects\CRTEffect.cpp src\Rendering\Effects\SSAOEffect.cpp src\Rendering\Effects\BloomEffect.cpp src\Rendering\Effects\ColorGradingEffect.cpp src\Rendering\Effects\DynamicLightingEffect.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Systems\LightingSystem.cpp"
set "SOURCE_FILES=%SOURCE_FILES% src\Scripting\LuaBindings.cpp"

set "COMPILED=0"
set "SKIPPED=0"

for %%f in (%SOURCE_FILES%) do (
    set "SOURCE_PATH=%SOURCE_DIR%\%%f"
    for %%n in ("%%f") do set "BASE_NAME=%%~nxn"
    set "OBJ_FILE=!BASE_NAME:.cpp=.o!"
    set "OBJ_PATH=%OBJ_DIR%\!OBJ_FILE!"
    set "DEP_FILE=%OBJ_DIR%\!BASE_NAME:.cpp=.d!"

    :: Vérification des dépendances
    set "NEEDS_COMPILE=0"
    if not exist "!OBJ_PATH!" (
        set "NEEDS_COMPILE=1"
    ) else (
        set "NOVA_OBJ=!OBJ_PATH!"
        set "NOVA_DEP=!DEP_FILE!"
        powershell -NoProfile -ExecutionPolicy Bypass -File "!PS_DEP_CHECK!" > nul 2>&1
        if !ERRORLEVEL! NEQ 0 set "NEEDS_COMPILE=1"
    )

    if "!NEEDS_COMPILE!"=="1" (
        echo    [COMPILE] %%f
        g++ -o "!OBJ_PATH!" -O0 -DNDEBUG ^
            -I "%SDK_DIR%" -I "%OBJ_DIR%" ^
            -include nlohmann/json.hpp ^
            -include sol/sol.hpp ^
            -MMD -MF "!DEP_FILE!" ^
            -c "!SOURCE_PATH!" ^
            -Wall -DSFML_STATIC -std=c++17 -Wa,-mbig-obj

        if !ERRORLEVEL! NEQ 0 (
            echo    [ERROR] Compilation failed for %%f
            goto :build_failed
        )
        set /a COMPILED+=1
    ) else (
        echo    [UP-TO-DATE] %%f
        set /a SKIPPED+=1
    )
)


echo.
echo    Compiled: !COMPILED! file(s) -- Skipped: !SKIPPED! file(s) ^(up-to-date^)

:: Aucun fichier recompilé et exécutable déjà présent → pas besoin de relinker
if "!COMPILED!"=="0" if exist "%BIN_DIR%\Nova.exe" (
    echo.
    echo    Nothing to do -- executable is up-to-date.
    goto :build_success_no_manifest
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
    -llua54 ^
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
