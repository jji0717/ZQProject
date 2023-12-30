setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir ZQSpot
cd ZQSpot

copy ..\..\..\..\common\dll\Releasestlp\zqcommonstlp.dll
copy ..\..\Release\zqspot.dll
copy ..\..\SpotExplorer\Release\SpotExplorer_R.dll

rem copy ICE dll
copy %ICE_ROOT%\bin\ice30.dll
copy %ICE_ROOT%\bin\iceutil30.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\bzip2.dll

cd ..

copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% ZQSpot -out VersionInfo.txt

del /q/f  ..\ZQSpot.zip

%PACKCMD% -r ..\ZQSpot.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal

