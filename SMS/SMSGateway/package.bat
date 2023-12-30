setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir bin
cd bin

copy  ..\..\release\*.exe
copy  ..\..\NPVRSMSDB.mdb
copy  %ITVSDKPATH%\exe\MCastSvc.dll
copy  %ITVSDKPATH%\exe\Reporter.dll
copy  %ITVSDKPATH%\exe\queue.dll
copy  %ITVSDKPATH%\exe\CLog.dll
copy  %ITVSDKPATH%\exe\ManPkgU.dll
copy  %ITVSDKPATH%\exe\cfgpkgU.dll
copy  %ITVSDKPATH%\exe\MtTcpComm.dll
copy  %ITVSDKPATH%\exe\ScThreadPool.dll
copy  %ITVSDKPATH%\exe\ShellMsgs.dll
copy  %ITVSDKPATH%\exe\ItvMessages.dll
copy  %ITVSDKPATH%\EXE\srvshell.exe
copy  %ITVSDKPATH%\EXE\instserv.exe

copy  %SystemRoot%\System32\mfc42u.dll
copy  %SystemRoot%\System32\MSVCP60.DLL
cd ..

mkdir config
cd config
copy  ..\..\config.xml
cd ..

copy ..\*.pl
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

del  /q/f  ..\SMSGateway.zip

%PACKCMD% -r  ..\SMSGateway.zip .

cd..
rd /s/q buildtemp

endlocal