setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
mkdir AssetGear
mkdir EventCollector
cd AssetGear
copy ..\..\exe\AssetGear.exe
copy ..\..\exe\AssetGear.pdb
copy ..\..\exe\AGTest.exe
copy ..\..\exe\AGTest.pdb
copy ..\..\exe\PMclient.dll
copy ..\..\exe\PMclient.pdb
cd ..
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy ..\NpvrSetup.pl
copy ..\profile.pl
copy ..\changes.txt
copy ..\NpvrRegistries.xls
copy "..\Npvr User Manual.doc"
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

cd EventCollector
copy ..\..\exe\EventCollector.exe
copy ..\..\exe\EventCollector.pdb
copy ..\..\isaalarm.ini
copy ..\..\event_ChodSvc.ini
copy ..\..\..\generic\JMSCppLib\JMSCpp\lib\jmsc.dll

rem regular text dll
copy %RegExppKit%\libs\regex\build\vc6\boost_regex_vc6_mdi.dll

md JMSClient
cd JMSClient
xcopy ..\..\..\..\generic\JMSCppLib\JMSCpp\Java\. /s /y
cd..

cd..
md dll
cd dll
rem tao dll
copy %ACE_ROOT%\TAO\BIN\tao_cosnotification.dll  
copy %ACE_ROOT%\TAO\BIN\tao_etcl.dll
copy %ACE_ROOT%\TAO\BIN\tao.dll
copy %ACE_ROOT%\TAO\BIN\ace.dll
copy %ACE_ROOT%\TAO\BIN\tao_cosevent.dll
copy %ACE_ROOT%\TAO\BIN\tao_dynamicany.dll
copy %ACE_ROOT%\TAO\BIN\tao_portableserver.dll
copy %ACE_ROOT%\TAO\BIN\tao_strategies.dll
copy %ACE_ROOT%\TAO\BIN\tao_cosnaming.dll
copy %ACE_ROOT%\TAO\BIN\tao_svc_utils.dll
copy %ACE_ROOT%\TAO\BIN\tao_iortable.dll
copy %ACE_ROOT%\TAO\BIN\tao_messaging.dll
copy %ACE_ROOT%\TAO\BIN\ACEXML.dll
copy %ACE_ROOT%\TAO\BIN\ACEXML_Parser.dll

rem ITV DLL
copy %ITVSDKPATH%\EXE\cfgpkgU.dll
copy %ITVSDKPATH%\EXE\CLog.dll
copy %ITVSDKPATH%\EXE\ItvMessages.dll
copy %ITVSDKPATH%\EXE\ManPkgU.dll
copy %ITVSDKPATH%\EXE\MCastSvc.dll
copy %ITVSDKPATH%\EXE\MtTcpComm.dll
copy %ITVSDKPATH%\EXE\queue.dll
copy %ITVSDKPATH%\EXE\Reporter.dll
copy %ITVSDKPATH%\EXE\ScThreadPool.dll
copy %ITVSDKPATH%\EXE\ShellMsgs.dll
copy %ITVSDKPATH%\EXE\ATOMIC_QUEUES.DLL
copy %ITVSDKPATH%\EXE\IDSAPI.DLL
copy %ITVSDKPATH%\EXE\MCAApiDll.dll
copy %VSTRMKITPATH%\libexpat.dll




cd..


del /q/f  ..\nPVR.zip


%VersionCheck% AssetGear >VersionInfo.txt
%VersionCheck% EventCollector >>VersionInfo.txt
%VersionCheck% dll >>VersionInfo.txt


%PACKCMD% -r ..\nPVR.zip .

cd ..
rd /s/q buildtemp

endlocal
