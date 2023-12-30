setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir package

copy %ITVSDKPATH%\exe\instserv.exe
copy %ZQPROJSPATH%\build\installService\CopyInstallFile.exe
copy %ZQPROJSPATH%\build\installService\install.bat
copy ..\profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

copy ..\PLAYLISTSOAPUDTMAPPER\ReleaseUMinDependency\PlaylistSoapUDTMapper.dll package\PlaylistSoapUDTMapper.dll
copy ..\PLAYLISTAS\Release\PlaylistAs.exe package\PlaylistAs.exe
copy ..\PlaylistSoapInterface.wsdl package\PlaylistSoapInterface.wsdl
copy ..\PlaylistSoapInterface.wsml package\PlaylistSoapInterface.wsml

copy %SYSTEMROOT%\SYSTEM32\mfc42u.dll package\mfc42u.dll

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
copy %ITVSDKPATH%\exe\SeaErrorMsgs.dll package\SeaErrorMsgs.dll
copy %ITVSDKPATH%\exe\ShellMsgs.dll package\ShellMsgs.dll
copy %ITVSDKPATH%\exe\srvshell.exe package\srvshell.exe

%VersionCheck% package -out VersionInfo.txt

%PACKCMD% ..\PlaylistAS .

cd ..
rd /s/q buildtemp

endlocal
