setlocal

set doxygenpath=C:\Program Files\doxygen\bin
set PATH=%PATH%;%ZQPROJSPATH%\build\utils;%doxygenpath%
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rem generate documents
doxygen NoiseSim.dox

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir bin
cd bin

copy ..\..\Release\*.exe
copy ..\..\NoiseSim.cfg
copy ..\..\NoiseSim.chm

cd ..
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

del /q/f  ..\NoiseSim.zip

%PACKCMD% -r ..\NoiseSim.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal
