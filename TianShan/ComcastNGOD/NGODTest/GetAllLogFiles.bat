setlocal

call .\envdef.bat

rem get all the log files

cd logs

timestamp.exe "set TIMESTAMP=" > }{.bat
call }{.bat
set archive=ngodfull-%TIMESTAMP%
echo Creating log archive: %archive%
echo Creating log archive: %archive% > savelogs.log
mkdir %archive%

cd %archive%

mkdir ax
mkdir rt1log
mkdir rt1lmullog
mkdir rt1lmul2log
mkdir n0log
mkdir n1log
mkdir n2log


if .%1. == .stream. goto streamloc1
for %%f in (IPS.Log) do copy /y \\%MDS2%\c$\itv\log\%%f .\ax
:streamloc1
for %%f in (ICM.Log) do copy /y \\%CM1%\c$\itv\log\%%f .\ax
for %%f in (vmca.log) do copy /y \\%VMCA1%\c$\itv\log\%%f .\ax

xcopy /if \\%TS1%\c$\tianshan\logs\*.log .\rt1log
xcopy /if \\%TS1%\d$\tianshan\logs\*.log .\rt1log
xcopy /if \\%TS1%\c$\tianshan\crashdump\*.dmp .\rt1log



if .%1. == .stream. goto streamloc2
for %%f in (stdout_.log stdout_.log.1 stdout_.log.2 stdout_.log.3 stdout_.log.5 stderr_.log jmsMessage_.log) do copy /y \\%TS2%\c$\Multiverse\logs\%%f .\rt1lmullog
:streamloc2

xcopy /if \\%CLUSTER%-N0\c$\tianshan\logs\*.log  .\n0log
xcopy /if \\%CLUSTER%-N1\c$\tianshan\logs\*.log  .\n1log
xcopy /if \\%CLUSTER%-N2\c$\tianshan\logs\*.log  .\n2log

xcopy /if \\%TS1%\c$\tianshan\etc\*.*    .\rt1log


xcopy /if \\%CLUSTER%-N0\c$\tianshan\crashdump\*.dmp .\n0log
xcopy /if \\%CLUSTER%-N1\c$\tianshan\crashdump\*.dmp .\n1log
xcopy /if \\%CLUSTER%-N2\c$\tianshan\crashdump\*.dmp .\n2log

xcopy /if \\%CLUSTER%-N0\c$\tianshan\etc\*.*  .\n0log
xcopy /if \\%CLUSTER%-N1\c$\tianshan\etc\*.*  .\n1log
xcopy /if \\%CLUSTER%-N2\c$\tianshan\etc\*.*  .\n2log


..\..\rar a -m1 -r ..\%archive%.rar .
rem if "%1"=="" rmdir /S /Q %archive%

cd..
cd..

endlocal


