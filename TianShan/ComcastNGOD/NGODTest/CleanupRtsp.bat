rem clean up all the RTSP server logs and safestore

rem declare machine NICs
set TS1=TS1_SS_NC
set TS2=TS2_SS_NC
set CLUSTER=SEA60012
set MDS1=igrn1mds1
set MDS2=igrn1mds2
set CM1=igrn1cm1
set VMCA1=igrn1vmca1

REM
REM stop services
REM

REM Node0
instserv StreamSmith stop %CLUSTER%-n0
instserv NodeContentStore stop %CLUSTER%-n0
REM instserv RtfCPNode stop %CLUSTER%-n0

REM Node 1
instserv StreamSmith stop %CLUSTER%-n1
instserv NodeContentStore stop %CLUSTER%-n1
REM instserv RtfCPNode stop %CLUSTER%-n1

REM Node 2
instserv StreamSmith stop %CLUSTER%-n2
instserv NodeContentStore stop %CLUSTER%-n2
REM instserv RtfCPNode stop %CLUSTER%-n2



REM RTSP pair
REM  instserv ClusterContentStore stop %TS1%


rem instserv MODSvc stop   %TS1%
instserv RtspProxy stop  %TS1%
rem instserv SiteAdminSvc stop  %TS1%
rem instserv Weiwoo stop  %TS1%



REM
REM remove logs and savestore
REM

for %%s in (172.16.200.40 172.16.200.41 172.16.200.42) do del /q \\%%s\c$\tianshan\logs\*.log
for %%s in (172.16.200.40 172.16.200.41 172.16.200.42) do del /q \\%%s\c$\tianshan\data\streamsmith\*.*



del /q \\%TS1%\c$\tianshan\logs\*.log
del /q \\%TS1%\d$\tianshan\logs\*.log
rem del /q \\%TS2%\d$\Multiverse\logs\*.log
del /q \\%TS1%\d$\tianshan\data\Sessions\*.*
del /q \\%TS1%\d$\tianshan\data\PathTickets\*.*
del /q \\%TS1%\c$\tianshan\data\Sessions\*.*
del /q \\%TS1%\c$\tianshan\data\PathTickets\*.*
del /q \\%TS1%\d$\tianshan\data\Sessions\*.*
del /q \\%TS1%\d$\tianshan\data\PathTickets\*.*
del /q \\%TS1%\g$\tianshan\data\Sessions\*.*
del /q \\%TS1%\g$\tianshan\data\PathTickets\*.*
del /q \\%TS1%\g$\tianshan\data\ssm_NGOD_r2c1\*.*
del /q \\%TS1%\d$\tianshan\data\ssm_NGOD2\*.*
del /q \\%TS1%\c$\tianshan\data\ssm_NGOD2\*.*
del /q \\%TS1%\r$\Sessions\*.*
del /q \\%TS1%\r$\PathTickets\*.*
del /q \\%TS1%\r$\ssm_NGOD_r2c1\*.*
del /q \\%TS1%\r$\ssm_NGOD2\*.*
del /q \\%TS1%\r$\MOD\*.*


del /q \\%TS1%\d$\tianshan\data\ssm_NGOD_r2c1\*.*
del /q \\%TS1%\d$\tianshan\data\MOD\*.*


del /q \\%CLUSTER%-n0\g$\streamsmith\*.*
del /q \\%CLUSTER%-n1\g$\streamsmith\*.*
del /q \\%CLUSTER%-n2\g$\streamsmith\*.*


:startservices
REM
REM start services
REM


REM rtsp pair
REM instserv ClusterContentStore start %TS1%


rem instserv MODSvc start %TS1%
instserv RtspProxy start  %TS1%
rem instserv SiteAdminSvc start  %TS1%
rem instserv Weiwoo start  %TS1%


REM Node0
instserv StreamSmith start %CLUSTER%-n0
instserv NodeContentStore start %CLUSTER%-n0
REM instserv RtfCPNode start %CLUSTER%-n0

REM Node 1
instserv StreamSmith start %CLUSTER%-n1
instserv NodeContentStore start %CLUSTER%-n1
REM instserv RtfCPNode start %CLUSTER%-n1

REM Node 2
instserv StreamSmith start %CLUSTER%-n2
instserv NodeContentStore start %CLUSTER%-n2
REM instserv RtfCPNode start %CLUSTER%-n2


@echo off
