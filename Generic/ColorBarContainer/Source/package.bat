setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

mkdir bin
cd bin
copy ..\..\ReleaseMinDependency\CBarContainer.dll
copy ..\..\Debug\CBarContainer_d.dll
cd ..

del /q/f  ..\ColorBarContainer.zip


%PACKCMD% -r ..\ColorBarContainer.zip .

cd ..

rd /s/q buildtemp

endlocal


