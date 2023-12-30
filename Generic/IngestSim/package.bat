setlocal

set doxygenpath=C:\Program Files\doxygen\bin
set PATH=%PATH%;%ZQPROJSPATH%\build\utils;%doxygenpath%
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir bin
cd bin

copy ..\..\Release\*.exe
copy %ITVSDKPATH%\EXE\CLog.dll

cd ..
copy %WPCAPSDKPATH%\..\WinPcap*.exe
copy ..\doc\ReadMe.txt
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

del /q/f  ..\IngestSim.zip

%PACKCMD% -r ..\IngestSim.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal