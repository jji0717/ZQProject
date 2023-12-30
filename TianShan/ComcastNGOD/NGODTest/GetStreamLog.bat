rem get all the log files about streaming test

setlocal

call .\envdef.bat

cd logs

timestamp.exe "set TIMESTAMP=" > }{.bat
call }{.bat
set archive=ngodstrm-%TIMESTAMP%
echo Creating log archive: %archive%
echo Creating log archive: %archive% > savelogs.log
mkdir %archive%

cd %archive%

mkdir ax
mkdir rt1log
mkdir n0log
mkdir n1log
mkdir n2log

if .%1. == .stream. goto streamloc1
:streamloc1
rem for %%f in (ICM.Log) do copy /y \\%CM1%\c$\itv\log\%%f .\ax
for %%f in (vmca.log) do copy /y \\%VMCA1%\c$\itv\log\%%f .\ax

xcopy /if \\%TS1%\c$\tianshan\logs\rtsp* .\rt1log
xcopy /if \\%TS1%\c$\tianshan\logs\ssm* .\rt1log
xcopy /if \\%TS1%\c$\tianshan\etc\*.*    .\rt1log


xcopy /if \\%CLUSTER%-n0\c$\tianshan\logs\*.log  .\n0log
xcopy /if \\%CLUSTER%-n1\c$\tianshan\logs\*.log  .\n1log
xcopy /if \\%CLUSTER%-n2\c$\tianshan\logs\*.log  .\n2log

xcopy /if \\%CLUSTER%-n0\c$\tianshan\etc\*.*  .\n0log
xcopy /if \\%CLUSTER%-n1\c$\tianshan\etc\*.*  .\n1log
xcopy /if \\%CLUSTER%-n2\c$\tianshan\etc\*.*  .\n2log

cd..
cd..

call GenRep.bat logs\%archive% %archive%

cd logs\%archive%
..\..\rar a -r -m1 ..\%archive%.rar .
cd..
cd..

endlocal


