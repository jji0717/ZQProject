setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rem rd /s/q buildtemp
if not exist buildtemp mkdir buildtemp
cd buildtemp

rem =========== Preparing subdir bin ===========
mkdir bin
cd bin

copy %ICE_ROOT%\bin\ssleay32.dll
copy %ICE_ROOT%\bin\libeay32.dll

rem copy ICESDK
copy %ICE_ROOT%\ICE.zip
copy %ICE_ROOT%\bin\bzip2.dll
copy %ICE_ROOT%\bin\freeze3?.dll
copy %ICE_ROOT%\bin\ice3?.dll
copy %ICE_ROOT%\bin\icebox3?.dll
copy %ICE_ROOT%\bin\icegrid3?.dll
copy %ICE_ROOT%\bin\icessl3?.dll
copy %ICE_ROOT%\bin\icestorm3?.dll
copy %ICE_ROOT%\bin\icestormservice3?.dll
copy %ICE_ROOT%\bin\iceutil3?.dll
copy %ICE_ROOT%\bin\icexml3?.dll
copy %ICE_ROOT%\bin\libdb4?.dll
copy %ICE_ROOT%\bin\libexpat.dll

copy %ICE_ROOT%\bin\libexpatw.dll
copy %ICE_ROOT%\bin\slice3?.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\glacier23?.dll
copy %BOOSTPYTHONPATH%\bin\boost_python-vc80-mt-1_51.dll

rem Copy Cdmifuse
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\AquaClient.dll .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\AquaClientCLR\Release\AquaClientCLR.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonStlp.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonStlp.pdb
copy %ZQPROJSPATH%\TianShan\bin\CdmiFuseCmd.exe
copy %ZQPROJSPATH%\TianShan\bin\CdmiFuseCmd.pdb
copy %ZQPROJSPATH%\TianShan\bin\CdmiFuseTray.exe
copy %ZQPROJSPATH%\TianShan\bin\CdmiFuseTray.pdb

copy %LibCURLPath%\dll\Release\x86\libcurl.dll

cd ..

mkdir cdmi-serve
cd cdmi-serve
xcopy /E %ZQPROJSPATH%\Generic\CdmiFuse\cdmi-serve
cd ..


mkdir SDK
cd SDK

mkdir cpp
cd cpp
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\AquaClient.dll .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\Release\AquaClient.lib .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\Release\AquaClient.pdb .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\AquaClient.h .
cd ..

mkdir java
cd java
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\java\bin\AquaClient.jar .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\java\lib\*.jar .
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\java\com\xormedia\aqua\sdk\AquaClientTest.java .
cd ..

mkdir python
cd python
copy %ZQPROJSPATH%\Generic\CdmiFuse\sdk\python\*.py .
cd ..

mkdir doc
cd doc
xcopy /IEF %ZQPROJSPATH%\Generic\CdmiFuse\sdk\java\build\doc html
copy %ZQPROJSPATH%\Generic\CdmiFuse\doc\"CDMI FUSE Extension thru ioctl".doc .
copy %ZQPROJSPATH%\Generic\CdmiFuse\doc\AquaClientSDK.doc .
cd ..

cd ..

%PACKCMD% -r ..\CdmiFuse_setup.zip .

cd ..
rd /s/q buildtemp