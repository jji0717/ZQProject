setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir SRM
cd SRM

copy ..\..\Release\SRM.exe
cd ..
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
del /q/f  ..\SRM_Setup.zip

%VersionCheck% SRM -out VersionInfo.txt


%PACKCMD% -r ..\SRM_setup.zip .

cd ..

rd /s/q buildtemp

endlocal
