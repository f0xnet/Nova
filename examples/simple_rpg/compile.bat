@echo off
setlocal enabledelayedexpansion
cls

:: ============================================
:: SIMPLE RPG EXAMPLE - BUILD SCRIPT
:: ============================================

echo.
echo ========================================
echo   SIMPLE RPG EXAMPLE - BUILD SYSTEM
echo ========================================
echo.

:: ============================================
:: CONFIGURATION
:: ============================================

set "PROJECT_ROOT=C:\Nova"
set "EXAMPLE_DIR=%PROJECT_ROOT%\examples\simple_rpg"
set "SDK_DIR=%PROJECT_ROOT%\sdk\include"
set "LIB_DIR=%PROJECT_ROOT%\sdk\libs"
set "BIN_DIR=%EXAMPLE_DIR%\bin\Release"
set "OBJ_DIR=%EXAMPLE_DIR%\obj\Release"
set "ASSETS_DIR=%EXAMPLE_DIR%\assets"

cd "%EXAMPLE_DIR%"

:: ============================================
:: STEP 1: SETUP DIRECTORIES
:: ============================================

echo [STEP 1/4] Setting up directories...
echo.

if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

echo    [OK] Directories ready

:: ============================================
:: STEP 2: COPY ASSETS
:: ============================================

echo.
echo [STEP 2/4] Copying assets...
echo.

xcopy /E /I /Y "%ASSETS_DIR%\*" "%BIN_DIR%\data\" >nul 2>&1

if exist "%BIN_DIR%\data" (
    echo    [OK] Assets copied to bin/Release/data/
) else (
    echo    [WARNING] Failed to copy assets
)

:: ============================================
:: STEP 3: COMPILE
:: ============================================

echo.
echo [STEP 3/4] Compiling main.cpp...
echo.

:: Liste des fichiers sources du SDK à compiler
set "SDK_SOURCES="
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\BackendManager.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\Core\BackendTypes.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLWindowBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLGraphicsBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLInputBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLResourceBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLAudioBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLFontBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Backend\SFML\SFMLViewportBackend.cpp"
set "SDK_SOURCES=%SDK_SOURCES% %PROJECT_ROOT%\sdk\src\Core\Logger.cpp"

:: Compiler les fichiers SDK si nécessaire
for %%f in (%SDK_SOURCES%) do (
    for %%n in ("%%f") do set "BASE_NAME=%%~nxn"
    set "OBJ_FILE=!BASE_NAME:.cpp=.o!"
    set "OBJ_PATH=%OBJ_DIR%\!OBJ_FILE!"

    if not exist "!OBJ_PATH!" (
        echo    Compiling SDK: %%~nxf...
        g++ -o "!OBJ_PATH!" -O2 -DNDEBUG -I "%SDK_DIR%" -c "%%f" -DSFML_STATIC -std=c++17

        if !ERRORLEVEL! NEQ 0 (
            echo    [ERROR] SDK compilation failed
            goto :build_failed
        )
    )
)

:: Compiler main.cpp
echo    Compiling main.cpp...
g++ -o "%OBJ_DIR%\main.o" -O2 -DNDEBUG -I "%SDK_DIR%" -c "%EXAMPLE_DIR%\main.cpp" -DSFML_STATIC -std=c++17

if %ERRORLEVEL% NEQ 0 (
    echo    [ERROR] Compilation failed
    goto :build_failed
)

echo    [OK] Compilation successful

:: ============================================
:: STEP 4: LINKING
:: ============================================

echo.
echo [STEP 4/4] Linking executable...
echo.

g++ -o "%BIN_DIR%\SimpleRPG.exe" "%OBJ_DIR%\*.o" ^
    -O2 -DNDEBUG ^
    -I "%SDK_DIR%" ^
    -L "%LIB_DIR%" ^
    -DSFML_STATIC ^
    -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-network-s -lsfml-system-s ^
    -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 ^
    -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 ^
    -std=c++17 -static-libgcc -static-libstdc++ -static

if %ERRORLEVEL% NEQ 0 (
    echo    [ERROR] Linking failed
    goto :build_failed
)

if not exist "%BIN_DIR%\SimpleRPG.exe" (
    echo    [ERROR] SimpleRPG.exe not created
    goto :build_failed
)

echo    [OK] Executable created successfully

:: ============================================
:: BUILD SUCCESS
:: ============================================

:build_success
echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: %BIN_DIR%\SimpleRPG.exe
echo Assets:     %BIN_DIR%\data\
echo.
echo IMPORTANT: Before running, make sure you have:
echo   1. Created the sprite files (player.png, npc_merchant.png, npc_innkeeper.png)
echo   2. Copied a font file to data/arial.ttf
echo.
echo See README.md for details on creating sprites.
echo.

choice /C YN /M "Run the game now"
if errorlevel 1 if not errorlevel 2 (
    cd "%BIN_DIR%"
    start SimpleRPG.exe
    cd "%EXAMPLE_DIR%"
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
echo Common issues:
echo   - SDK source files not found (check PROJECT_ROOT path)
echo   - MinGW not in PATH
echo   - SFML libraries not found in sdk/libs/
echo.
pause
exit /b 1
