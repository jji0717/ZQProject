setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir bin

cd bin
copy ..\..\ModAuthorization\Release\MultiverseAuthorization.dll
copy ..\..\ModAuthorization\MultiverseAuthorization.xml MultiverseAuthorization_Sample.xml
cd ..
copy ..\*.pl
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ZQPROJSPATH%\build\utils\VerCheck.exe package\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% ..\MODPlugIn .

cd ..
rd /s/q buildtemp

endlocal
