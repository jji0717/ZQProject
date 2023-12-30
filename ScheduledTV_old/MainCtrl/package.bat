setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir package

copy %ITVSDKPATH%\exe\instserv.exe
copy %ZQPROJSPATH%\build\installService\CopyInstallFile.exe
copy %ZQPROJSPATH%\build\installService\install.bat
copy ..\profile.pl

copy ..\ServRelease\ITVPlayback.exe package\ITVPlayback.exe

copy %SYSTEMROOT%\SYSTEM32\mfc42u.dll package\mfc42u.dll
copy %SYSTEMROOT%\SYSTEM32\mfc42ud.dll package\mfc42ud.dll

copy %ITVSDKPATH%\exe\atomic_queues.dll package\atomic_queues.dll
copy %ITVSDKPATH%\exe\cfgpkgU.dll package\cfgpkgU.dll
copy %ITVSDKPATH%\exe\CLog.dll package\CLog.dll
copy %ITVSDKPATH%\exe\ItvMessages.dll package\ItvMessages.dll
copy %ITVSDKPATH%\exe\ManPkgU.dll package\ManPkgU.dll
copy %ITVSDKPATH%\exe\MCastSvc.dll package\MCastSvc.dll
copy %ITVSDKPATH%\exe\MtTcpComm.dll package\MtTcpComm.dll
copy %ITVSDKPATH%\exe\queue.dll package\queue.dll
copy %ITVSDKPATH%\exe\Reporter.dll package\Reporter.dll
copy %ITVSDKPATH%\exe\ReporterMsgs.dll package\ReporterMsgs.dll
copy %ITVSDKPATH%\exe\ScThreadPool.dll package\ScThreadPool.dll
copy %ITVSDKPATH%\exe\ShellMsgs.dll package\ShellMsgs.dll
copy %ITVSDKPATH%\exe\srvshell.exe package\srvshell.exe


%PACKCMD% ..\ITVPlayback .

cd ..
rd /s/q buildtemp

endlocal
