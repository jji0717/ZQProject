setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp


copy ..\..\bin\MD4File.exe
copy ..\..\bin\cp.exe
copy ..\..\bin\md5sum.exe

copy %ZQPROJSPATH%\TianShan\CovAxiomToNGOD.pl
copy %ZQPROJSPATH%\TianShan\Calmd5.pl
copy %ZQPROJSPATH%\TianShan\Readme.txt
copy %ZQPROJSPATH%\TianShan\bin\ContentClient.exe
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonStlp.dll

rem copy for ITVSDKPATH
copy %ITVSDKPATH%\exe\idsapi.dll
copy %ITVSDKPATH%\exe\cfgpkgU.dll
copy %ITVSDKPATH%\exe\CLog.dll
copy %ITVSDKPATH%\exe\ManPkgU.dll
copy %ITVSDKPATH%\exe\MtTcpComm.dll
copy %ITVSDKPATH%\exe\queue.dll
copy %ITVSDKPATH%\exe\Reporter.dll
copy %ITVSDKPATH%\exe\ScThreadPool.dll

rem copy for ICE SDK
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


del /q/f  ..\CovAxiomToNGOD.zip

%PACKCMD% -r ..\CovAxiomToNGOD.zip .


cd ..
rd /s/q buildtemp

endlocal
