setlocal

set HHC="%ProgramFiles%\HTML Help Workshop\hhc.exe"
set PATH=%PATH%;%ZQPROJSPATH%\build\utils;%ProgramFiles%\doxygen\bin
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
set PUBLISHLOC=\\10.50.12.4\d$\wpub\engineering

set CWD=%CD%

cd ChannelOnDemand\service

cd ..\doc
mkdir html
copy images\*.png html\
doxygen.exe ChannelOnDemand.dox
if exist %PUBLISHLOC%\PauseTV\html rd/s/q %PUBLISHLOC%\PauseTV\html & xcopy /IEF html %PUBLISHLOC%\PauseTV\html

cd ..\JavaClient

del /q ChannelOnDemandSDK.zip

mkdir lib
mkdir doc

copy ..\doc\ChannelOnDemand.chm doc\
copy *.jar lib\
copy %ZQPROJSPATH%\TianShan\bin\*.jar lib\

%PACKCMD% -r ChannelOnDemandSDK.zip lib doc

cd ..
cd ..

endlocal
