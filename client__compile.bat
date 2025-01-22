@echo off
setlocal enabledelayedexpansion
cd C:\Nova\client

:: Répertoires
set "project_dir=C:\Nova"
set "source_dir=C:\Nova\client"
set "bin_dir=%source_dir%\bin\Release"
set "obj_dir=%source_dir%\obj\Release"
set "sdk_dir=%project_dir%\sdk\include"

:: Créer les répertoires nécessaires
if not exist "%bin_dir%" mkdir "%bin_dir%"
if not exist "%obj_dir%" mkdir "%obj_dir%"

:: Copier les assets
xcopy /E /I /Y "%source_dir%\assets\" "%bin_dir%\data\"

:: Compiler les ressources
windres "%source_dir%\icon\appicon.rc" -O coff -o "%source_dir%\icon\appicon.res"

:: Liste des fichiers source
set "files=main.cpp api\nova\RenderClass.cpp api\nova\NovaClass.cpp api\draw\DrawClass.cpp api\nova\SystemClass.cpp api\ui\UIManager.cpp api\ui\UIClass.cpp api\ui\UIGraphic.cpp api\ui\UIString.cpp api\ui\UIbutton.cpp api\network\NetworkManager.cpp api/event/EventHandler.cpp api/event/ButtonEvent.cpp api/nova/SceneManager.cpp api/nova/Scene.cpp api\animation\AnimationManager.cpp api\animation\Animation.cpp"

:: Compilation
for %%f in (%files%) do (
    set "source_file=%%f"
    set "source_path=%source_dir%\!source_file!"

    :: Extraire uniquement le nom de base du fichier
    for %%n in ("!source_file!") do set "base_name=%%~nxn"
    set "object_file=!base_name:.cpp=.o!"
    set "object_path=%obj_dir%\!object_file!"

    :: Vérifier si la source a été modifiée depuis la dernière compilation
    if not exist "!object_path!" (
        echo Compiling !source_file! because !object_path! does not exist.
        call :compile
    ) else (
        for %%s in ("!source_path!") do set "source_time=%%~ts"
        for %%o in ("!object_path!") do set "object_time=%%~to"
        if "!source_time!" gtr "!object_time!" (
            echo Compiling !source_file! because source has been modified.
            call :compile
        ) else (
            echo Skipping !source_file! because it is up-to-date.
        )
    )
)

:: Lien des fichiers objets
echo Lien des fichiers objets...
g++ -o "%bin_dir%\Nova.exe" "%obj_dir%\*.o" -O0 -DNDEBUG -I "%sdk_dir%" -L %project_dir%\sdk\libs -DSFML_STATIC -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-network-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 -std=c++17 -static-libgcc -static-libstdc++ -static "%source_dir%\icon\appicon.res"

:: Exécuter le programme
cd "%bin_dir%"
Nova.exe
cd "%source_dir%"

:: Fin du script
exit /b

:compile
g++ -o "!object_path!" -O0 -DNDEBUG -I "%sdk_dir%" -c "!source_path!" -Wall -DSFML_STATIC
echo Compiled !source_file! at !source_time!
exit /b
pause
