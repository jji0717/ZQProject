@echo off
setlocal
set WORKDIR=C:\temp
set TianShanHome=C:\TianShan
set TianShanLogs=%TianShanHome%\logs
set TianShanEtc=%TianShanHome%\etc
set ZIPCMD=%TianShanHome%\utils\zip -m -r 

rem ========================================================================================
echo TsStat.bat   Utilty to collect TianShan log files
echo   Usage: TsStat [MM-DD-YYYY]
echo       MM-DD-YYYY    to specify the date from when the utility should collect the log files
echo   CurrentSettings:
echo       WORKDIR      = %WORKDIR%
echo       TianShanHome = %TianShanHome%
echo       TianShanLogs = %TianShanLogs%
echo       TianShanEtc  = %TianShanEtc%
echo   Please edit the batch job to correct the settings if the above didn't match               
echo.                                                                                         
                                                                                              
if ./?. == .%1. goto endloc                                                                   
                                                                                              
if not .. == .%1. set LogSince=%1                                                             
for /F "usebackq " %%i in (`hostname`) do set HOSTNAME=%%i                                    
cd /d %WORKDIR%                                                                               
if %ERRORLEVEL% NEQ 0 goto endloc                                                             
                                                                                              
rd /s/q %HOSTNAME%
mkdir %HOSTNAME%
chdir %HOSTNAME%

echo collecting registry settings ...
regedit /e reg_1.tmp "HKEY_LOCAL_MACHINE\SOFTWARE\ZQ Interactive\TianShan"
regedit /e reg_2.tmp "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services"
copy reg_1.tmp + reg_2.tmp reg.txt

echo collecting process information ...
wmic process >proc.txt

echo collecting net stat ...
netstat -ano >netstat.txt

set ZIPNAME=TsStat_%HOSTNAME%
if EXIST %TianShanHome%\utils\timestamp.exe for /F "usebackq " %%i in (`%TianShanHome%\utils\timestamp.exe`) do set ZIPNAME=TsStat_%HOSTNAME%_%%i

set XCOPYOPT=/IEFY

echo collecting configuration from %TianShanEtc% ...
xcopy %XCOPYOPT% %TianShanEtc% etc >xcopy.log

echo collecting log files from %TianShanLogs% ...
if not .. == .%LogSince%. set XCOPYOPT=%XCOPYOPT% /D:%LogSince%
echo.>>xcopy.log
echo ==============logfiles >>%LogSince%===================>>xcopy.log
xcopy %XCOPYOPT% %TianShanLogs% logs >>xcopy.log

echo creating log archive %ZIPNAME%.zip ...
del /Q *.tmp
%ZIPCMD% ..\%ZIPNAME%.zip . > NULL
cd ..
rd /s/q %HOSTNAME%
echo kit %WORKDIR%\%ZIPNAME%.zip has been prepared

if not .Y.==.%2. goto endloc

if exist %TianShanHome%\logs\Neighbors.bat call %TianShanHome%\logs\Neighbors.bat
if exist %TianShanHome%\utils\NeighborStat.bat for %%i in (%TsNeighbors%) do call NeighborStat.bat %%i

:endloc
endlocal
