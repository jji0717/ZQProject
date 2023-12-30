setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir bin
cd bin
copy ..\..\Navigator\ReleaseServ\*.exe

copy %SYSTEMROOT%\SYSTEM32\mfc71u.dll
copy %SYSTEMROOT%\SYSTEM32\msvcp71.dll
copy %SYSTEMROOT%\SYSTEM32\msvcr71.dll

copy %ITVSDKPATH%\exe\cfgpkgU.dll
copy %ITVSDKPATH%\exe\CLog.dll
copy %ITVSDKPATH%\exe\Ctail.exe
copy %ITVSDKPATH%\exe\ItvMessages.dll
copy %ITVSDKPATH%\exe\ManPkgU.dll
copy %ITVSDKPATH%\exe\MCastSvc.dll
copy %ITVSDKPATH%\exe\MtTcpComm.dll
copy %ITVSDKPATH%\exe\queue.dll
copy %ITVSDKPATH%\exe\Reporter.dll
copy %ITVSDKPATH%\exe\ReporterMsgs.dll
copy %ITVSDKPATH%\exe\ScThreadPool.dll
copy %ITVSDKPATH%\exe\SeaErrorMsgs.dll
copy %ITVSDKPATH%\exe\ShellMsgs.dll
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\srvshell.exe
cd ..

copy ..\NSSync.pl
copy ..\profile.pl
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

del NSSync_forVirginMedia.Zip

%PACKCMD% ..\NSSync_forVirginMedia .

cd ..
rd /s/q buildtemp

endlocal
