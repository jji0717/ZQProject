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

mkdir DSA
cd DSA
mkdir bin
cd bin
copy ..\..\..\project\DSA\Release\DSA.exe
copy ..\..\..\project\DSA\Release\DSA.pdb
copy ..\..\..\project\PortControllerdll\Release\DODServerController.dll
copy ..\..\..\project\PortControllerdll\Release\DODServerController.pdb
copy ..\..\..\filter\DATAWATCHERSOURCE\Release\DATAWATCHERSOURCE.ax
copy ..\..\..\filter\DATAWATCHERSOURCE\Release\DATAWATCHERSOURCE.pdb
copy ..\..\..\filter\DATAWRAPPERFILTER\Release\DATAWRAPPER.ax
copy ..\..\..\filter\DATAWRAPPERFILTER\Release\DATAWRAPPER.pdb
copy ..\..\..\filter\ZQBroadcastFilter\Release\ZQBroadcastFilter.ax
copy ..\..\..\filter\ZQBroadcastFilter\Release\ZQBroadcastFilter.pdb
cd ..
mkdir etc
cd etc
copy ..\..\..\project\DSA\DSA.ini
cd ..
cd ..

mkdir DCA
cd DCA
mkdir bin
cd bin
copy ..\..\..\project\DCA\Release\DCA.exe
copy ..\..\..\project\DCA\Release\DCA.pdb
copy ..\..\..\project\DCA\jmsc.dll
copy ..\..\..\project\DevKit\Release\DODDevKit.dll
copy ..\..\..\project\DevKit\Release\DODDevKit.pdb
mkdir java
cd java
xcopy %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\java /E
cd ..
cd ..
mkdir etc
cd etc
copy ..\..\..\project\DCA\DCA.xml
copy ..\..\..\project\DCA\LocalConfig.xml
cd ..
cd ..

mkdir SRM
cd SRM
mkdir bin
cd bin
copy ..\..\..\project\SRM\Release\SRM.exe
copy ..\..\..\project\SRM\Release\SRM.pdb
cd ..
mkdir etc
cd etc
copy ..\..\..\project\SRM\SRMConfiguration.xml
cd ..
cd ..

mkdir ConfigDefine
cd ConfigDefine
copy ..\..\DOD_Initial_for_Conf\def\ConfDefine_DOD.xml
copy ..\..\DOD_Initial_for_Conf\languages\*.xml
cd ..

copy ..\*.pl

copy %ITVSDKPATH%\EXE\regsetup.exe
copy %ITVSDKPATH%\EXE\srvshell.exe
copy %ITVSDKPATH%\EXE\instserv.exe
copy %ITVSDKPATH%\EXE\REGDMP.exe

copy %ZQPROJSPATH%\build\utils\VerCheck.exe
del /q/f  ..\DODServer_Setup.zip

%VersionCheck% DSA\bin > VersionInfo.txt
%VersionCheck% DCA\bin >>VersionInfo.txt
%VersionCheck% SRM\bin >>VersionInfo.txt


%PACKCMD% -r ..\DODServer_setup.zip .

cd ..

rd /s/q buildtemp

endlocal


