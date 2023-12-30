setlocal

set doxygenpath=C:\Program Files\doxygen\bin
set PATH=%PATH%;%ZQPROJSPATH%\build\utils;%doxygenpath%
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe -r 

rem generate documents
doxygen JMSCpp.dox

rd /s/q buildtemp
mkdir buildtemp

cd buildtemp

xcopy /ISE ..\lib        lib
xcopy /ISE ..\java       java
xcopy /ISE ..\..\samples samples

mkdir doc
copy ..\*.chm doc\

mkdir include
copy ..\*.h include\*

%PACKCMD% ..\JMSCpp .

cd ..
rem rd /s/q buildtemp

endlocal
