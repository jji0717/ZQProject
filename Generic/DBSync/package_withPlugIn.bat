setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe
 
rd /s/q buildtemp
mkdir buildtemp

cd buildtemp
mkdir bin
cd bin

copy ..\..\release\*.exe
copy ..\..\release\*.pdb
copy %ITVSDKPATH%\exe\cfgpkgU.dll
copy %ITVSDKPATH%\exe\CLog.dll
copy %ITVSDKPATH%\exe\Ctail.exe
copy %ITVSDKPATH%\exe\idsapi.dll
copy %SystemRoot%\System32\mfc42u.dll
copy %SystemRoot%\System32\msvcr71.dll
copy %SystemRoot%\System32\msvcp71.dll
copy %SystemRoot%\System32\mfc71u.dll
copy %ITVSDKPATH%\exe\ItvMessages.dll
copy %ITVSDKPATH%\exe\ManPkgU.dll
copy %ITVSDKPATH%\exe\MCastSvc.dll
copy %SystemRoot%\System32\MSVCP60.DLL
copy %SystemRoot%\System32\mfc71.dll
copy %ITVSDKPATH%\exe\MtTcpComm.dll
copy %ITVSDKPATH%\exe\ipsapi.dll
copy %ITVSDKPATH%\exe\queue.dll
copy %ITVSDKPATH%\exe\Reporter.dll
copy %ITVSDKPATH%\exe\ScThreadPool.dll
copy %ITVSDKPATH%\exe\ShellMsgs.dll
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\instserv.exe

cd ..

md plugin
cd plugin
copy ..\..\..\DBSADI\IDSSyncEventPlugin\IDSSyncEventPlugin\Release\IDSSyncEventPlugin.dll
copy ..\..\..\DBSADI\IDSSyncEventPlugin\IDSSyncEventPlugin\Release\IDSSyncEventPlugin.pdb
copy ..\..\..\DBSADI\IDSSyncEventPlugin\IDSSyncEventPlugin\IDSSyncEventPlugin.ini
copy ..\..\..\DBSADI\ManualSyncAddin\ManualSyncAddin\Release\ManualSyncPlugin.dll
copy ..\..\..\DBSADI\ManualSyncAddin\ManualSyncAddin\Release\ManualSyncPlugin.pdb
copy ..\..\..\JMSCppLib\JMSCpp\lib\jmsc.dll

md JMSClient
cd JMSClient
md java
cd java
xcopy ..\..\..\..\..\JMSCppLib\JMSCpp\Java\. /s /y
cd ..
cd ..
cd ..

copy ..\DBSync_withPlugIn.pl DBSync.pl
copy ..\profile_withPlugIn.pl profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe


%VersionCheck% bin -out VersionInfo.txt

if exist ..\DBSync.zip del /q/f  ..\DBSync.zip

%PACKCMD% ..\DBSync .

cd ..
rd /s/q buildtemp

endlocal
