setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%PACKEXT%.==.. set PACKEXT=zip 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir bin

copy ..\McastFwd\release\*.exe bin
copy ..\McastTest\release\*.exe bin

rem ITV DLL/ util
mkdir dll
for %%f in (cfgpkgU, CLog, ItvMessages, ManPkgU, MCastSvc, cpComm, Reporter, ScThreadPool, ShellMsgs) do copy %ITVSDKPATH%\EXE\%%f.dll dll

for %%f in (regsetup.exe,  srvshell.exe, instserv.exe) do copy %ITVSDKPATH%\EXE\%%f bin

copy ..\doc\"Installation and User Guide.pdf"
copy %ZQPROJSPATH%\build\utils\VerCheck.exe 

del /q/f ..\McastFwd.%PACKEXT%

%VersionCheck% bin >VersionInfo.txt
%VersionCheck% dll >>VersionInfo.txt

%PACKCMD% -r ..\McastFwd .
cd ..
rd /s/q buildtemp

endlocal