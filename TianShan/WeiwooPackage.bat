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

copy ..\..\bin\weiwooservice.exe
copy ..\..\bin\weiwooservice.pdb
copy ..\..\bin\ZQShellMsgs.dll
copy ..\..\bin\FileLogDll.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.pdb
copy ..\..\bin\ZQCFGPKG.DLL
copy ..\..\bin\ZQShell.exe
copy ..\..\bin\ZQSnmpManPkg.dll

cd ..
mkdir modules
cd modules
copy ..\..\bin\pho_SeaChange.dll

cd ..
mkdir etc
cd etc
copy ..\..\weiwoo\service\weiwooServices.xml

cd..


copy ..\Weiwoo\setup\Weiwoo.pl
copy ..\Weiwoo\setup\Profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe


del /q/f  ..\WeiwooService_setup.zip

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% -r ..\WeiwooService_setup.zip .


cd ..
rd /s/q buildtemp

endlocal
