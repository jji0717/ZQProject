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

copy ..\..\bin\rtspProxy.exe
copy ..\..\bin\rtspProxy.pdb
copy ..\..\bin\ZQShellMsgs.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.dll
copy %ZQPROJSPATH%\common\dll\ReleaseStlp\ZQCommonStlp.pdb
copy ..\..\bin\ZQCFGPKG.DLL
copy ..\..\bin\ZQShell.exe
copy ..\..\bin\ZQSnmpManPkg.dll
if %1==Cluster copy ..\..\bin\ssm_tianshan_s1.dll
if %1==Cluster copy ..\..\bin\ssm_tianshan_s1.pdb
if %1==Node copy ..\..\bin\ssm_pausetv_s1.dll
if %1==Node copy ..\..\bin\ssm_pausetv_s1.pdb


cd ..
mkdir etc
cd etc
copy ..\..\streamsmith\service\rtspProxyServices.xml
if %1==Cluster copy ..\..\streamsmith\modules\ssm_tianshan_s1\ssm_tianshan_s1.xml
if %1==Node copy ..\..\streamsmith\modules\ssm_pausetv_s1\ssm_pausetv_s1.xml

cd..


copy ..\StreamSmith\setup\RtspProxy.pl
copy ..\StreamSmith\setup\profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe


del /q/f  ..\rtspProxy_setup.zip

%VersionCheck% bin -out VersionInfo.txt

%PACKCMD% -r ..\rtspProxy_setup.zip .


cd ..
rd /s/q buildtemp

endlocal
