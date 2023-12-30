setlocal
set PATH=%PATH%;%CMEPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%CMEPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%CMEPATH%\build\utils\VerCheck.exe

cd %CMEPATH%

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

rem =========== Preparing subdir bin ===========
mkdir bin64
cd bin64

rem copy TianShanSDK
copy %TianShanSDK%\x64\bin\ZQShell.exe
copy %TianShanSDK%\x64\bin\ZQCfgPkg.dll
copy %TianShanSDK%\x64\bin\ZQCommonStlp.dll
copy %TianShanSDK%\x64\bin\ZQShellMsgs.dll
copy %TianShanSDK%\x64\bin\ZQSNMPAgent.dll
copy %TianShanSDK%\x64\bin\ZQSNMPManPkg.dll

copy %TianShanSDK%\x64\pdb\ZQShell.pdb
copy %TianShanSDK%\x64\pdb\ZQCfgPkg.pdb
copy %TianShanSDK%\x64\pdb\ZQCommonStlp.pdb
copy %TianShanSDK%\x64\pdb\ZQShellMsgs.pdb
copy %TianShanSDK%\x64\pdb\ZQSNMPAgent.pdb
copy %TianShanSDK%\x64\pdb\ZQSNMPManPkg.pdb


rem copy for CMEV2
copy %CMEPATH%\bin64\CMEV2.exe
copy %CMEPATH%\bin64\CMEV2.pdb


cd ..

rem =========== Preparing subdir utils ===========
mkdir utils64
cd utils64
copy %CMEPATH%\build\utils\VerCheck64.exe
copy %CMEPATH%\build\utils\VerCheck.exe
copy %CMEPATH%\build\utils\zip.exe
copy %CMEPATH%\build\utils\tail.exe
copy %CMEPATH%\build\utils\timestamp.exe
copy %CMEPATH%\build\utils\cygiconv-2.dll
copy %CMEPATH%\build\utils\cygintl-8.dll
copy %CMEPATH%\build\utils\cygwin1.dll
copy %CMEPATH%\build\utils\libexpat.dll

cd ..

rem =========== Kit siganitures ===========
%VersionCheck% bin64 >VersionInfo_CMEV2_64.txt
rem %VersionCheck% modules64 >>VersionInfo_CMEV2_64.txt

copy VersionInfo_CMEV2_64.txt bin64\

del /q/f  ..\CMEV2_setup.zip

rem %PACKCMD% -r ..\CMEV2_setup.zip .

rem pause

cd ..
rem rd /s/q buildtemp

endlocal
