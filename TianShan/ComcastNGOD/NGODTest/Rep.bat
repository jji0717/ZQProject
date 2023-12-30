REM %1 rar file
REM %2 name of db file

mkdir Report

set EXPATHDIR=%2

if "%2"=="" set EXPATHDIR=%1

md Rpt\%EXPATHDIR%
rar e Rpt\%1.rar Rpt\%EXPATHDIR%

copy Template\ngod2_template.mdb  "Report\%EXPATHDIR%.mdb" /y

import "Rpt\%EXPATHDIR%" "Report\%EXPATHDIR%.mdb" >importlog.txt
rem notepad importlog.txt