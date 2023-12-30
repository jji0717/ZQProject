setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

copy %ZQPROJSPATH%\TianShan\RenContentName.pl
copy %ZQPROJSPATH%\TianShan\Readme_ren.txt
copy %ZQPROJSPATH%\TianShan\bin\ContentClient.exe
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonStlp.dll

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


del /q/f  ..\RenContentName.zip

%PACKCMD% -r ..\RenContentName.zip .


cd ..
rd /s/q buildtemp

endlocal
