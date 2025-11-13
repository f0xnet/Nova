@echo off
setlocal enabledelayedexpansion

echo ========================================
echo RESOURCEHACKER SETUP
echo ========================================
echo.
echo ResourceHacker est un outil gratuit pour modifier les ressources Windows.
echo Il peut embedder des manifests sans probleme.
echo.

set "RH_DIR=C:\Nova\tools\ResourceHacker"
set "RH_EXE=%RH_DIR%\ResourceHacker.exe"

if exist "!RH_EXE!" (
    echo [OK] ResourceHacker deja installe: !RH_EXE!
    goto :use_rh
)

echo ResourceHacker n'est pas installe.
echo.
echo TELECHARGEMENT AUTOMATIQUE...
echo.

if not exist "%RH_DIR%" mkdir "%RH_DIR%"

:: Télécharger ResourceHacker depuis le site officiel
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'http://www.angusj.com/resourcehacker/resource_hacker.zip' -OutFile '%RH_DIR%\rh.zip' -UseBasicParsing}" 2>nul

if exist "%RH_DIR%\rh.zip" (
    echo [OK] Telechargement reussi
    echo Extraction...
    
    powershell -Command "Expand-Archive -Path '%RH_DIR%\rh.zip' -DestinationPath '%RH_DIR%' -Force" 2>nul
    
    if exist "!RH_EXE!" (
        echo [SUCCESS] ResourceHacker installe avec succes!
        del "%RH_DIR%\rh.zip"
    ) else (
        echo [ERROR] Extraction echouee
        goto :manual
    )
) else (
    echo [ERROR] Telechargement echoue
    goto :manual
)

:use_rh
echo.
echo ResourceHacker est pret a l'emploi.
echo Il sera automatiquement utilise par votre script de build.
echo.
pause
exit /b 0

:manual
echo.
echo ========================================
echo INSTALLATION MANUELLE REQUISE
echo ========================================
echo.
echo 1. Telecharge ResourceHacker depuis:
echo    http://www.angusj.com/resourcehacker/
echo.
echo 2. Installe-le ou extrait-le vers:
echo    %RH_DIR%\
echo.
echo 3. Relance ce script
echo.
pause
exit /b 1