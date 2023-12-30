setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rem rd /s/q buildtemp
if not exist Components4RTF.V2.0 mkdir Components4RTF.V2.0
cd Components4RTF.V2.0

rem =========== Preparing subdir bin ===========
mkdir bin
cd bin

rem copy for CPE
copy %ZQPROJSPATH%\TianShan\bin\CPESvc.exe
copy %ZQPROJSPATH%\TianShan\bin\CPESvc.pdb
copy %RTFLIBSDKPATH%\exe\Win32\RTFLib.dll
copy %RTFLIBSDKPATH%\exe\Win32\RTFLib.pdb
copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.dll
copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.pdb


copy %VSTRMKITPATH%\PacedIndex.dll
copy %VSTRMKITPATH%\PacedVV2.dll
copy %VSTRMKITPATH%\PacedVVX.dll

copy %VSTRMKITPATH%\i386\PacedIndex.dll /y
copy %VSTRMKITPATH%\i386\PacedVV2.dll   /y
copy %VSTRMKITPATH%\i386\PacedVVX.dll   /y

cd..

rem =========== Preparing subdir modules ===========
mkdir modules
cd modules
rem for CPE service
copy %ZQPROJSPATH%\TianShan\bin\CPH_RDS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RDS.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTFRDS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTFRDS.pdb
rem copy %ZQPROJSPATH%\TianShan\bin\CPH_NasCopy.dll
rem copy %ZQPROJSPATH%\TianShan\bin\CPH_NasCopy.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTI.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTI.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTINAS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTINAS.pdb

cd ..

xcopy /fsi *.pdb ..\TianShanSymbols\Components4RTF.V2.0

del /s *.pdb 

del /q/f  ..\Components4RTF.V2.0.zip

%PACKCMD% -r ..\Components4RTF.V2.0.zip .

rem pause

cd ..
rd /s/q Components4RTF.V2.0

endlocal
