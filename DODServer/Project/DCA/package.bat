setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir DCA
cd DCA

if exist ..\..\Release\DCA.exe copy ..\..\Release\DCA.exe
if exist ..\..\Debug\DCA.exe  copy ..\..\Debug\DCA.exe
copy ..\..\DCA.xml
copy ..\..\DODDevKit.dll
copy ..\..\jmsc.dll
copy ..\..\LocalConfig.xml

cd ..
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

del /q/f  ..\DCA_Setup.zip

%VersionCheck% DCA -out VersionInfo.txt


%PACKCMD% -r ..\DCA_setup.zip .

cd ..

rd /s/q buildtemp

endlocal