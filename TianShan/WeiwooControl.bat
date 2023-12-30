setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

copy ..\bin\TestAdminCtl.exe
copy ..\bin\AdminControl.dll
copy %ICE_ROOT%\bin\bzip2.dll
copy %ICE_ROOT%\bin\ice31.dll
copy %ICE_ROOT%\bin\icestorm31.dll
copy %ICE_ROOT%\bin\iceutil31.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy ..\AccreditedPath\AdminControl\RegisterControl.bat


del /q/f  ..\WeiwooControl_setup.zip


%PACKCMD% -r ..\WeiwooControl_setup.zip .


cd ..
rd /s/q buildtemp