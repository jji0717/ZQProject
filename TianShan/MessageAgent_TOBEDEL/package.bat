setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe


rd /s/q buildtemp
mkdir buildtemp

cd buildtemp

mkdir bin

cd bin

copy    ..\..\release\MessageAgent.exe
copy    ..\..\..\..\Common\dll\ReleaseStlp\ZQCommonStlp.dll
copy    %ITVSDKPATH%\exe\MCastSvc.dll
copy    %ITVSDKPATH%\exe\Reporter.dll
copy    %ITVSDKPATH%\exe\queue.dll
copy    %ITVSDKPATH%\exe\CLog.dll
copy    %ITVSDKPATH%\exe\ManPkgU.dll
copy    %ITVSDKPATH%\exe\cfgpkgU.dll
copy    %ITVSDKPATH%\exe\MtTcpComm.dll
copy    %ITVSDKPATH%\exe\ScThreadPool.dll
copy    %ITVSDKPATH%\exe\ShellMsgs.dll
copy    %ZQProjsPath%\build\bin\ItvMessages.dll
copy    %ITVSDKPATH%\exe\ctail.exe
copy    %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\lib\jmsc.dll

mkdir java
cd java
copy    %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\java\jbossall-client.jar
xcopy   %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\java\weblogic /E
cd ..

cd ..

mkdir config
cd config
copy    ..\..\JmsTopicConfig.xml
copy ..\..\config.icebox
copy ..\..\config.service
cd ..

copy ..\MessageAgent.pl
copy ..\Profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe

%VersionCheck% bin -out VersionInfo.txt
del  /q/f  ..\MessageAgent.zip

%PACKCMD% -r  ..\MessageAgent.zip .

cd..
rd /s/q buildtemp

endlocal


