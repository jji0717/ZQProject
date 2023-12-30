setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

mkdir bin
cd bin
copy %ICE_ROOT%\bin\freeze31.dll
copy %ICE_ROOT%\bin\ice31.dll
copy %ICE_ROOT%\bin\iceutil31.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\libeay32.dll
copy %ICE_ROOT%\bin\libdb43.dll
copy %ICE_ROOT%\bin\bzip2.dll
copy %ICE_ROOT%\bin\icestorm31.dll
copy %ICE_ROOT%\bin\ssleay32.dll

copy ..\..\bin\StreamSmith.exe
copy ..\..\bin\StreamSmith.pdb
copy ..\..\bin\ZQShellMsgs.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.pdb
copy ..\..\bin\ZQCFGPKG.DLL
copy ..\..\bin\ZQShell.exe
copy ..\..\bin\ZQSnmpManPkg.dll
copy ..\..\bin\streamsmithadmin.exe

cd ..
mkdir modules
cd modules
copy ..\..\bin\ssm_tianshan.dll
copy ..\..\bin\ssm_tianshan.pdb
copy ..\..\bin\ssm_zq_lam.dll
copy ..\..\bin\ssm_zq_lam.pdb


cd ..
mkdir etc
cd etc
copy ..\..\streamsmith\service\StreamSmithServices.xml

cd..


copy ..\StreamSmith\setup\StreamSmith.pl
copy ..\StreamSmith\setup\profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe


del /q/f  ..\StreamSmith_Setup.zip

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% -r ..\StreamSmith_Setup.zip .


cd ..
rd /s/q buildtemp

endlocal
