@echo off
setlocal

:: Spécifiez le chemin du dossier à parcourir
set "SOURCE_DIR=C:\nova\"

:: Spécifiez le fichier de sortie
set "OUTPUT_FILE=C:\nova\output.txt"

:: Spécifiez le chemin du dossier à exclure (dans ce cas, le dossier sdk)
set "EXCLUDE_DIR=C:\nova\sdk"

:: Vérifiez si le fichier de sortie existe déjà et supprimez-le si c'est le cas
if exist "%OUTPUT_FILE%" del "%OUTPUT_FILE%"

:: Parcourez tous les fichiers .cpp et .hpp, excluez les fichiers dans le dossier sdk et ajoutez leur contenu au fichier de sortie
for /r "%SOURCE_DIR%" %%f in (*.cpp *.hpp) do (
    echo %%f | findstr /i /c:"%EXCLUDE_DIR%" > nul
    if errorlevel 1 (
        echo Processing: %%f
        type "%%f" >> "%OUTPUT_FILE%"
        echo. >> "%OUTPUT_FILE%"
    )
)
echo Done.
