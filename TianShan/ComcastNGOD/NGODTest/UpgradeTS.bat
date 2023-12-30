@Echo off

set RTSPB1=172.16.200.51
set RTSPB2=172.16.200.52
set CLN0=sea60187-n0
set CLN1=sea60187-n1
set CLN2=sea60187-n2


REM %1 current version
REM %2 remove logs, if it's 1, remove


IF "%1"=="" GOTO :usage


@Echo on
REM
REM stop services
REM

REM Node0
instserv StreamSmith stop %CLN0%
instserv NodeContentStore stop %CLN0%
instserv RtfCPNode stop %CLN0%

REM Node 1
instserv StreamSmith stop %CLN1%
instserv NodeContentStore stop %CLN1%
instserv RtfCPNode stop %CLN1%

REM Node 2
instserv StreamSmith stop %CLN2%
instserv NodeContentStore stop %CLN2%
instserv RtfCPNode stop %CLN2%



REM RTSP pair
instserv ClusterContentStore stop %RTSPB1%
instserv MODSvc stop  %RTSPB1%
instserv RtspProxy stop %RTSPB1%
instserv SiteAdminSvc stop %RTSPB1%
instserv Weiwoo stop %RTSPB1%



REM
REM copy files
REM

xcopy /Y /C D:\TianShanKit\%1\bin \\%CLN0%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\dll \\%CLN0%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\modules \\%CLN0%\c$\tianshan\modules

xcopy /Y /C D:\TianShanKit\%1\bin \\%CLN1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\dll \\%CLN1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\modules \\%CLN1%\c$\tianshan\modules

xcopy /Y /C D:\TianShanKit\%1\bin \\%CLN2%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\dll \\%CLN2%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\modules \\%CLN2%\c$\tianshan\modules

xcopy /Y /C D:\TianShanKit\%1\bin \\%RTSPB1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\dll \\%RTSPB1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\modules \\%RTSPB1%\c$\tianshan\modules


xcopy /Y /C D:\TianShanKit\%1\bin \\%RTSPB1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\dll \\%RTSPB1%\c$\tianshan\bin
xcopy /Y /C D:\TianShanKit\%1\modules \\%RTSPB1%\c$\tianshan\modules


REM
REM remove logs
REM

IF "%2"=="1" GOTO :removelogs


:startservices
REM
REM start services
REM

REM rt1
instserv ClusterContentStore start %RTSPB1%

instserv MODSvc start  %RTSPB1%
instserv RtspProxy start %RTSPB1%
instserv SiteAdminSvc start %RTSPB1%
instserv Weiwoo start %RTSPB1%


REM rtsp pair
instserv ClusterContentStore start %RTSPB1%


instserv MODSvc start %RTSPB1%
instserv RtspProxy start  %RTSPB1%
instserv SiteAdminSvc start  %RTSPB1%
instserv Weiwoo start  %RTSPB1%


REM Node0
instserv StreamSmith start %CLN0%
instserv NodeContentStore start %CLN0%
instserv RtfCPNode start %CLN0%

REM Node 1
instserv StreamSmith start %CLN1%
instserv NodeContentStore start %CLN1%
instserv RtfCPNode start %CLN1%

REM Node 2
instserv StreamSmith start %CLN2%
instserv NodeContentStore start %CLN2%
instserv RtfCPNode start %CLN2%


@echo off
GOTO :end

:usage
echo.
echo Usage:   Upgrade.bat [current version] [remove logs if 1]
echo.
pause
GOTO :end


:removelogs
del /q \\%RTSPB1%\c$\tianshan\logs\*.log
del /q \\%RTSPB1%\d$\Multiverse\logs\*.log
del /q \\%RTSPB1%\c$\tianshan\data\Sessions\*.*
del /q \\%RTSPB1%\c$\tianshan\data\PathTickets\*.*
del /q \\%RTSPB1%\c$\tianshan\data\ssm_NGOD_r2c1\*.*
del /q \\%RTSPB1%\c$\tianshan\data\MOD\*.*


del /q \\%RTSPB1%\c$\tianshan\logs\*.log
del /q \\%RTSPB1%\d$\tianshan\data\Sessions\*.*
del /q \\%RTSPB1%\d$\tianshan\data\PathTickets\*.*
del /q \\%RTSPB1%\d$\tianshan\data\ssm_NGOD_r2c1\*.*
del /q \\%RTSPB1%\d$\tianshan\data\MOD\*.*


for %%s in (%CLN0% %CLN1% %CLN2%) do del /q \\%%s\c$\tianshan\logs\*.log

GOTO :startservices


:end