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
copy %ICE_ROOT%\bin\ice30.dll
copy %ICE_ROOT%\bin\iceutil30.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\libeay32.dll
copy %ICE_ROOT%\bin\SSLEAY32.DLL
copy ..\..\..\..\common\dll\ReleaseStlp\ZQCommonstlp.dll

cd ..
mkdir utils
cd utils
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
cd ..

del /q/f  ..\WMSService.zip

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% -r ..\WMSService.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal