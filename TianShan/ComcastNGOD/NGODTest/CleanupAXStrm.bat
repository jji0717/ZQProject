rem clean up all stream related log files on Axiom

rem declare machine NICs
set TS1=TS1_SS_NC
set TS2=TS2_SS_NC
set CLUSTER=SEA60012
set MDS1=igrn1mds1
set MDS2=igrn1mds2
set CM1=igrn1cm1
set VMCA1=igrn1vmca1

instserv vmca stop %VMCA1%
instserv iss stop %VMCA1%
instserv icm stop %CM1%

del \\%VMCA1%\c$\itv\log\*.log /q
del \\%CM1%\c$\itv\log\icm.log /q
del \\%VMCA1%\d$\itv\safestore\vmca* /q
instserv vmca start %VMCA1%
instserv iss start %VMCA1%
instserv icm start %CM1%