@echo off
setlocal enabledelayedexpansion

c:/Nova/client/compile.bat
c:/Nova/server/compile.bat
c:/Nova/client/bin/Release/nova.exe
c:/Nova/server/bin/Release/NovaServer.exe

pause
