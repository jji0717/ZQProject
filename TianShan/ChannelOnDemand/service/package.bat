setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

copy ..\..\JavaClient\ChannelOnDemandSDK.zip

mkdir bin
cd bin

copy %ICE_ROOT%\bin\bzip2.dll
copy %ICE_ROOT%\bin\freeze31.dll
copy %ICE_ROOT%\bin\ice31.dll
copy %ICE_ROOT%\bin\icebox31.dll
copy %ICE_ROOT%\bin\icegrid31.dll
copy %ICE_ROOT%\bin\icessl31.dll
copy %ICE_ROOT%\bin\icestorm31.dll
copy %ICE_ROOT%\bin\iceutil31.dll
copy %ICE_ROOT%\bin\icexml31.dll
copy %ICE_ROOT%\bin\libdb43.dll
copy %ICE_ROOT%\bin\libexpat.dll

copy %ICE_ROOT%\bin\libexpatw.dll
copy %ICE_ROOT%\bin\msvcp60.dll
copy %ICE_ROOT%\bin\msvcrt.dll
copy %ICE_ROOT%\bin\slice31.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll

copy %ITVSDKPATH%\EXE\reporter.dll
copy %ITVSDKPATH%\EXE\MCASTSVC.DLL
copy %ITVSDKPATH%\EXE\MANPKGU.DLL
copy %ITVSDKPATH%\EXE\queue.dll
copy %ITVSDKPATH%\EXE\clog.dll
copy %ITVSDKPATH%\EXE\CFGPKGU.DLL
copy %ITVSDKPATH%\EXE\itvmessages.dll
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\instserv.exe

copy ..\..\..\bin\ChODSvc.exe
copy ..\..\..\bin\ChODSvc.pdb

copy ..\..\..\..\generic\JMSCppLib\JMSCpp\lib\jmsc.dll

md JMSClient
cd JMSClient
xcopy ..\..\..\..\..\generic\JMSCppLib\JMSCpp\Java\. /s /y
cd..

cd ..
copy ..\*.pl


copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

rem mkdir ssm_PauseTV_sl
rem cd ssm_PauseTV_sl
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_pausetv_s1\Release\ssm_pausetv_s1.dll
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_pausetv_s1\readme.txt
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_pausetv_s1\ssm_PauseTV_s1.xml
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_pausetv_s1\Install.bat
rem copy %ITVSDKPATH%\EXE\regsetup.exe
rem cd ..

rem mkdir ssm_tianshan_sl
rem cd ssm_tianshan_sl
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_tianshan_s1\Release\ssm_tianshan_s1.dll
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_tianshan_s1\readme.txt
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_tianshan_s1\ssm_tianshan_s1.xml
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_tianshan_s1\Install.bat
rem copy %ITVSDKPATH%\EXE\regsetup.exe
rem cd ..

%VersionCheck% bin >VersionInfo.txt
rem %VersionCheck% ssm_PauseTV_sl >>VersionInfo.txt
rem %VersionCheck% ssm_tianshan_sl >>VersionInfo.txt

del /q/f  ..\ChodSvc_setup.zip

%PACKCMD% -r ..\ChodSvc_setup.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal
