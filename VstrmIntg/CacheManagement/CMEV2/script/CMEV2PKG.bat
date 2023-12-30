setlocal
set PATH=%PATH%;%CMEPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%CMEPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%CMEPATH%\build\utils\VerCheck.exe

cd %CMEPATH%

if not exist buildtemp mkdir buildtemp
cd buildtemp

rem =========== Preparing subdir bin ===========
mkdir bin
cd bin

rem copy TianShanSDK
copy %TianShanSDK%\x86\bin\ZQShell.exe
copy %TianShanSDK%\x86\bin\ZQCfgPkg.dll
copy %TianShanSDK%\x86\bin\ZQCommonStlp.dll
copy %TianShanSDK%\x86\bin\ZQShellMsgs.dll
copy %TianShanSDK%\x86\bin\ZQSNMPAgent.dll
copy %TianShanSDK%\x86\bin\ZQSNMPManPkg.dll

copy %TianShanSDK%\x86\pdb\ZQShell.pdb
copy %TianShanSDK%\x86\pdb\ZQCfgPkg.pdb
copy %TianShanSDK%\x86\pdb\ZQCommonStlp.pdb
copy %TianShanSDK%\x86\pdb\ZQShellMsgs.pdb
copy %TianShanSDK%\x86\pdb\ZQSNMPAgent.pdb
copy %TianShanSDK%\x86\pdb\ZQSNMPManPkg.pdb

rem copy for CMEV2
copy %CMEPATH%\bin\CMEV2.exe
copy %CMEPATH%\bin\CMEV2.pdb

cd ..

rem =========== Preparing subdir utils ===========
mkdir utils
cd utils
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
rem =========== Preparing subdir etc ===========
mkdir etc
cd etc
copy %CMEPATH%\etc\CME.xml CME_sample.xml
copy %CMEPATH%\etc\TianShanDef.xml TianShanDef_sample.xml
copy %CMEPATH%\etc\ProactiveCache.xml ProactiveCache_sample.xml

cd ..

rem =========== Preparing installation script ===========

copy %CMEPATH%\CMEV2\script\CMEV2Setup.bat
copy %CMEPATH%\CMEV2\script\CMEV2Setup.bat CMEV2Setup.pl
copy %CMEPATH%\CMEV2\script\profile.pl
copy %CMEPATH%\build\utils\VerCheck.exe

rem =========== Kit siganitures ===========
%VersionCheck% bin >VersionInfo_CMEV2_32.txt
%VersionCheck% modules >>VersionInfo_CMEV2_32.txt
%VersionCheck% sdk >>VersionInfo_CMEV2_32.txt

copy VersionInfo_CMEV2_32.txt bin\

xcopy /fsi *.pdb ..\CMEV2Symbols

del /s *.pdb 

del /q/f  ..\CMEV2_setup.zip

%PACKCMD% -r ..\CMEV2_setup.zip .

rem pause

cd ..
rd /s/q buildtemp

cd CMEV2Symbols

del /q/f ..\CMEV2Symbols.zip

%PACKCMD% -r ..\CMEV2Symbols.zip .

cd ..
rd /s/q CMEV2Symbols

endlocal
