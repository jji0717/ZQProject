setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

mkdir bin64
cd bin64
copy ..\..\bin64\ServerLoad.exe
copy ..\..\bin64\ServerLoad.pdb
copy ..\..\..\..\TianShan\bin64\ZQShell.exe
copy ..\..\..\..\TianShan\bin64\ZQShell.pdb
copy ..\..\..\..\TianShan\bin64\ZQShellMsgs.dll
copy ..\..\..\..\TianShan\bin64\ZQCfgPkg.dll
copy ..\..\..\..\TianShan\bin64\ZQCfgPkg.pdb
copy ..\..\..\..\TianShan\bin64\ZQSnmp.dll
copy ..\..\..\..\TianShan\bin64\ZQSnmp.pdb
copy ..\..\..\..\TianShan\bin64\ZQSnmpManPkg.dll
copy ..\..\..\..\TianShan\bin64\ZQSnmpManPkg.pdb
copy ..\..\..\..\TianShan\bin64\ZQCommonstlp.dll
copy ..\..\..\..\TianShan\bin64\ZQCommonstlp.pdb
rem copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\x64\libexpat.dll
cd ..


mkdir bin
cd bin
copy ..\..\bin\ServerLoad.exe
copy ..\..\bin\ServerLoad.pdb
copy ..\..\..\..\TianShan\bin\ZQShell.exe
copy ..\..\..\..\TianShan\bin\ZQShell.pdb
copy ..\..\..\..\TianShan\bin\ZQShellMsgs.dll
copy ..\..\..\..\TianShan\bin\ZQCfgPkg.dll
copy ..\..\..\..\TianShan\bin\ZQCfgPkg.pdb
copy ..\..\..\..\TianShan\bin\ZQSnmp.dll
copy ..\..\..\..\TianShan\bin\ZQSnmp.pdb
copy ..\..\..\..\TianShan\bin\ZQSnmpManPkg.dll
copy ..\..\..\..\TianShan\bin\ZQSnmpManPkg.pdb
copy ..\..\..\..\TianShan\bin\ZQCommonstlp.dll
copy ..\..\..\..\TianShan\bin\ZQCommonstlp.pdb
rem copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\libexpat.dll
cd ..

mkdir etc
cd etc
copy ..\..\ServerLoad.xml ServerLoad_Sample.xml
cd ..

mkdir doc
cd doc
copy ..\..\doc\TianShanFormat.xml
cd ..


copy ..\*.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

%VersionCheck% bin >VersionInfo_ServerLoad.txt

copy VersionInfo_ServerLoad.txt bin\

del /q/f  ..\ServerLoad_setup.zip

%PACKCMD% -r ..\ServerLoad_setup.zip .

rem pause

cd ..
rd /s/q buildtemp

endlocal