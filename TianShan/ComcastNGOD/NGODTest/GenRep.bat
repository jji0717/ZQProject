setlocal

REM %1 the path for logs collected by getN or getlogs
REM %2 the name of rar file

set COLPATH=%1
set REPORTZIP=%2
if .%2.==.. set REPORTZIP=report

del scan\*.txt /q

RegExCol -c .\col.ini -l .\col.log

cd scan
..\rar a -m5 ..\Rpt\%REPORTZIP%.rar *.txt

endlocal