setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rem rd /s/q buildtemp
if not exist Components4RTF.V2.0 mkdir Components4RTF.V2.0
cd Components4RTF.V2.0

rem =========== Preparing subdir bin64 ===========
mkdir bin64
cd bin64

rem copy for CPE
copy %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\CPESvc.pdb
if exist %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe copy %RTFLIBSDKPATH%\exe\x64\RTFLib.dll
if exist %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe copy %RTFLIBSDKPATH%\exe\x64\RTFLib.pdb
if exist %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.dll
if exist %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.pdb

copy %VSTRMKITPATH%\x64\PacedIndex.dll
copy %VSTRMKITPATH%\x64\PacedVV2.dll
copy %VSTRMKITPATH%\x64\PacedVVX.dll

cd..

rem =========== Preparing subdir modules ===========
mkdir modules64
cd modules64
rem for CPE service
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTFRDS.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTFRDS.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_NasCopy.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_NasCopy.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTI.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTI.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTINAS.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTINAS.pdb

cd ..

rem xcopy /fsi *.pdb ..\TianShanSymbols\Components4RTF.V2.0
rem del /s *.pdb
cd ..

endlocal
