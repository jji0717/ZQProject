setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

mkdir bin
cd bin

copy %ICE_ROOT%\bin\BZIP2.dll
copy %ICE_ROOT%\bin\FREEZE3?.dll
copy %ICE_ROOT%\bin\ICE3?.dll
copy %ICE_ROOT%\bin\ICEUTIL3?.dll
copy %ICE_ROOT%\bin\LIBDB4?.dll
copy %ICE_ROOT%\bin\STLPORT_VC646.dll

copy ..\..\..\TianShan\bin\ZQCommonStlp.dll
copy ..\..\..\TianShan\bin\ZQCfgPkg.dll
copy ..\..\..\TianShan\bin\ZQSnmpManPkg.dll
copy ..\..\..\TianShan\bin\ZQShellMsgs.dll
copy ..\..\..\TianShan\bin\ZQShell.exe

rem copy for TSDump
copy ..\..\..\Generic\TSDump\Release\TSDump.exe
copy ..\..\..\Generic\TSDump\Release\TSDump.pdb
copy ..\..\..\Generic\TSDump\Release\ReadMe.txt

rem copy for DODApp
copy ..\..\Phase2\DODApp\Release\DODApplication.exe
copy ..\..\Phase2\DODApp\Release\DODApplication.pdb
copy ..\..\Phase2\DODApp\JMSDispatch\Release\JMSDISPATCH.dll
copy ..\..\Phase2\DODApp\JMSDispatch\Release\JMSDispatch.pdb
copy ..\..\Phase2\DODApp\JMSC.dll
copy ..\..\Phase2\DODApp\Localconfig.xml Localconfig_Sample.xml

mkdir java
cd java
xcopy %ZQPROJSPATH%\Generic\JMSCppLib\JMSCpp\java /E
cd ..

rem copy for DODContentStore
copy ..\..\Phase2\DODContentStore\Release\DODCS.exe
copy ..\..\Phase2\DODContentStore\Release\DODCS.pdb

rem copy for DataStream
copy ..\..\Phase2\DataStream\Release\DataSS.exe
copy ..\..\Phase2\DataStream\Release\DataSS.pdb

copy ..\..\Phase2\DSClient\Release\DSClient.exe
copy ..\..\Phase2\DSClient\Release\DSClient.pdb

cd ..

mkdir etc
cd etc
copy ..\..\Phase2\etc\DODApp.xml DODApp_Sample.xml
copy ..\..\phase2\etc\DODContentStore.xml DODContentStore_Sample.xml
copy ..\..\phase2\etc\DataStream.xml DataStream_Sample.xml
cd ..

mkdir ConfigDefine
cd ConfigDefine
copy ..\..\phase2\DOD_Initial_for_Conf\def\ConfDefine_DOD.xml
copy ..\..\phase2\DOD_Initial_for_Conf\languages\*.xml
cd ..

mkdir modules
cd modules
copy ..\..\phase2\DodPho\Release\pho_dod.dll
copy ..\..\phase2\DodPho\Release\pho_dod.pdb
cd ..

copy ..\DataOnDemandSetup.pl
copy ..\profile.pl

copy %ZQPROJSPATH%\build\utils\VerCheck.exe

if exist ..\DataOnDemand_Setup.zip del /q/f  ..\DataOnDemand_Setup.zip

%VersionCheck% bin > VersionInfo_DataOnDemand.txt
%VersionCheck% dll >>VersionInfo_DataOnDemand.txt
%VersionCheck% modules >>VersionInfo_DataOnDemand.txt

copy VersionInfo_DataOnDemand.txt bin

%PACKCMD% -r ..\DataOnDemand_setup.zip .

cd ..

rd /s/q buildtemp

endlocal
