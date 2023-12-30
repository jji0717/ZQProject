setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

mkdir common
cd common
copy %systemroot%\\system32\\MFC71.dll
copy %systemroot%\\system32\\msvcp71.dll
copy %systemroot%\\system32\\msvcr71.dll
cd ..

mkdir DataStream
cd DataStream
mkdir bin
cd bin
copy ..\..\..\Phase1\DODSTREAMER\Project\DSA\Release\DataStream.exe
copy ..\..\..\Phase1\DODSTREAMER\Project\DSA\Release\DataStream.pdb
copy ..\..\..\Phase1\DODSTREAMER\Project\PortControllerdll\Release\DODServerController.dll
copy ..\..\..\Phase1\DODSTREAMER\Project\PortControllerdll\Release\DODServerController.pdb

rem copy ..\..\..\project\PortControllerdll\Release\DODServerController.dll
rem copy ..\..\..\project\PortControllerdll\Release\DODServerController.pdb
copy ..\..\..\Phase1\DODSTREAMER\filter\DATAWATCHERSOURCE\Release\DataWatcherSource.ax
copy ..\..\..\Phase1\DODSTREAMER\filter\DATAWATCHERSOURCE\Release\DataWatcherSource.pdb
copy ..\..\..\Phase1\DODSTREAMER\filter\DATAWRAPPERFILTER\Release\DATAWRAPPER.ax
copy ..\..\..\Phase1\DODSTREAMER\filter\DATAWRAPPERFILTER\Release\DATAWRAPPER.pdb
copy ..\..\..\Phase1\DODSTREAMER\filter\ZQBroadcastFilter\Release\ZQBroadcastFilter.ax
copy ..\..\..\Phase1\DODSTREAMER\filter\ZQBroadcastFilter\Release\ZQBroadcastFilter.pdb

rem copy ICE DLL
copy %ICE_VC7_ROOT%\bin\BZIP2.DLL
copy %ICE_VC7_ROOT%\bin\FREEZE31.DLL
copy %ICE_VC7_ROOT%\bin\ICE31.DLL
copy %ICE_VC7_ROOT%\bin\ICEUTIL31.DLL
copy %ICE_VC7_ROOT%\bin\LIBDB43.DLL

cd ..
mkdir etc
cd etc
copy ..\..\..\Phase1\DODSTREAMER\Project\DSA\config
copy ..\..\..\Phase1\DODSTREAMER\Project\DSA\datastream.ini
cd ..
cd ..

mkdir DODApp
cd DODApp
mkdir bin
cd bin
copy ..\..\..\Phase1\DODApp\Release\DODApp.exe
copy ..\..\..\Phase1\DODApp\Release\DODApp.pdb
copy ..\..\..\Phase1\DODApp\jmsc.dll
copy ..\..\..\Phase1\DODApp\JMSDispatch\Release\JMSDispatch.dll

copy ..\..\..\..\Common\dll\ReleaseStlp\ZQCommonStlp.dll

rem copy ICE DLL
copy %ICE_ROOT%\bin\BZIP2.dll
copy %ICE_ROOT%\bin\FREEZE31.dll
copy %ICE_ROOT%\bin\ICE31.dll
copy %ICE_ROOT%\bin\ICEUTIL31.dll
copy %ICE_ROOT%\bin\LIBDB43.dll
copy %ICE_ROOT%\bin\STLPORT_VC646.dll

rem copy ITV DLL
copy %ITVSDKPATH%\EXE\cfgpkgU.dll
copy %ITVSDKPATH%\EXE\CLog.dll
copy %ITVSDKPATH%\EXE\ManPkgU.dll
copy %ITVSDKPATH%\EXE\MCastSvc.dll
copy %ITVSDKPATH%\EXE\MtTcpComm.dll
copy %ITVSDKPATH%\EXE\queue.dll
copy %ITVSDKPATH%\EXE\Reporter.dll
copy %ITVSDKPATH%\EXE\ScThreadPool.dll
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\ItvMessages.dll

rem copy ..\..\..\project\DevKit\Release\DODDevKit.dll
rem copy ..\..\..\project\DevKit\Release\DODDevKit.pdb
mkdir java
cd java
xcopy %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\java /E
cd ..
cd ..
mkdir etc
cd etc
copy ..\..\..\Phase1\DODApp\DODAppConfig
copy ..\..\..\Phase1\DODApp\Localconfig.xml
cd ..
cd ..

mkdir ConfigDefine
cd ConfigDefine
copy ..\..\Phase1\DOD_Initial_for_Conf\def\ConfDefine_DOD.xml
copy ..\..\Phase1\DOD_Initial_for_Conf\languages\*.xml
cd ..

copy ..\*.pl

copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe

copy %ZQPROJSPATH%\build\utils\VerCheck.exe
del /q/f  ..\DataOnDemand_Setup.zip

%VersionCheck% DataStream\bin > VersionInfo.txt
%VersionCheck% DODApp\bin >>VersionInfo.txt


%PACKCMD% -r ..\DataOnDemand_setup.zip .

cd ..

rd /s/q buildtemp

endlocal


