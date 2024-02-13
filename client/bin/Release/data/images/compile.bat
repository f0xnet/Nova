xcopy /E /I /Y "C:\Nova\assets" "C:\Nova\bin\Release\assets"
windres icon/appicon.rc -O coff -o icon/appicon.res

g++ -o obj/Release/main.o -O0 -DNDEBUG -I sdk/include -c main.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/RenderClass.o -O0 -DNDEBUG -I sdk/include -c api/nova/RenderClass.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/NovaClass.o -O0 -DNDEBUG -I sdk/include -c api/nova/NovaClass.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/DrawClass.o -O0 -DNDEBUG -I sdk/include -c api/draw/DrawClass.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/SystemClass.o -O0 -DNDEBUG -I sdk/include -c api/nova/SystemClass.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/UIManager.o -O0 -DNDEBUG -I sdk/include -c api/ui/UIManager.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/UIClass.o -O0 -DNDEBUG -I sdk/include -c api/ui/UIClass.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/UIGraphic.o -O0 -DNDEBUG -I sdk/include -c api/ui/UIGraphic.cpp -Wall -DSFML_STATIC
g++ -o obj/Release/UIString.o -O0 -DNDEBUG -I sdk/include -c api/ui/UIString.cpp -Wall -DSFML_STATIC

g++ -o bin/Release/Nova obj/Release/*.o -O0 -DNDEBUG -I sdk/include -L sdk/libs -DSFML_STATIC -lsfml-graphics-s-d -lsfml-window-s-d -lsfml-audio-s-d -lsfml-network-s-d -lsfml-system-s-d -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg -lws2_32 -std=c++17 -static-libgcc -static-libstdc++ -static icon/appicon.res


cd C:\Nova\bin\Release\
Nova.exe
cd C:\Nova