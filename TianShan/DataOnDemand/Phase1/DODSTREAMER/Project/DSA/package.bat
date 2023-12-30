setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir DSA
cd DSA

copy ..\..\project\DSA\Release\DSA.exe
copy ..\..\project\PortControllerdll\Release\DODServerController.dll
copy ..\..\filter\DATAWATCHERSOURCE\Release\DATAWATCHERSOURCE.ax
copy ..\..\filter\DATAWRAPPERFILTER\Release\DATAWRAPPER.ax
copy ..\..\filter\ZQBroadcastFilter\Release\ZQBroadcastFilter.ax

cd ..
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

del /q/f  ..\DSA_Setup.zip

%VersionCheck% DSA -out VersionInfo.txt


%PACKCMD% -r ..\DSA_setup.zip .

cd ..

rd /s/q buildtemp

endlocal