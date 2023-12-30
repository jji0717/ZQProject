setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp
copy ..\exe\AssetGear.exe
copy ..\exe\AssetGear.pdb
copy ..\exe\PMclient.dll
copy ..\exe\PMclient.pdb
copy ..\exe\AGTest.exe
copy ..\exe\AGTest.pdb

copy ..\install.bat
copy ..\readme.txt
copy ..\changes.txt
copy ..\AssetGear.xls
copy %ITVSDKPATH%\EXE\regsetup.exe
copy ..\NpvrRegistries.pl
copy "..\Npvr User Manual.doc"

del /q/f  ..\nPVR.zip

%PACKCMD% -r ..\nPVR.zip .

cd ..
rd /s/q buildtemp

endlocal
