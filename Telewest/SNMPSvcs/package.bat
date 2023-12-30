setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir bin
cd bin
copy ..\..\SNMPSvcs.exe
copy ..\..\SNMPSvcs.pdb
copy ..\..\snmp.ini
copy %SNMP_PLUS_ROOT%\bin\snmp_pp.dll
copy %RegExppKit%\libs\regex\build\vc6\boost_regex_vc6_mdi.dll


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
copy %ZQPROJSPATH%\Telewest\SNMPSvcs\ZQServerTrap.MIB
cd ..

copy ..\SNMPSvcs.pl
copy ..\profile.pl
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% ..\SNMPSvcs .

cd ..
rd /s/q buildtemp

endlocal
