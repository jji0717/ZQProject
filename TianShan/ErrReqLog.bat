set MACHINE=%1
set POPULAR_ERRS=404 454 455 500 503
set ERRS_TO_DETAIL=500 503

findstr /c:"..Method-Code: " RtspProxy* > responses_%MACHINE%.txt
findstr /c:"RTSP/1.0 200 OK" /V  responses_%MACHINE%.txt >failedresp_%MACHINE%_t.txt

sed -i 's/RtspProxy.*log://g' failedresp_%MACHINE%_t.txt
sort failedresp_%MACHINE%_t.txt > failedresp_%MACHINE%.txt
del /Q failedresp_%MACHINE%_t.txt

for %%i in (%POPULAR_ERRS%) do findstr /c:"RTSP/1.0 %%i" failedresp_%MACHINE%.txt > %%i_%MACHINE%.txt

copy failedresp_%MACHINE%.txt 999_%MACHINE%-0.txt
for %%i in (%POPULAR_ERRS%) do findstr /c:"RTSP/1.0 %%i" /V 999_%MACHINE%-0.txt > 999_%MACHINE%.txt && copy /Y 999_%MACHINE%.txt 999_%MACHINE%-0.txt
del /Q 999_%MACHINE%-0.txt

for %%j in (%ERRS_TO_DETAIL%) do for %%i in (SETUP PLAY PAUSE TEARDOWN GET_PARAMETER) do findstr /c:"Method-Code: %%i" %%j_%MACHINE%.txt > %%jz_%%i_%MACHINE%.txt

wc -l [0-9]*.txt failedresp_%MACHINE%.txt responses_%MACHINE%.txt > counts_%MACHINE%.txt
sed -i 's/\.txt//g' counts_%MACHINE%.txt
del /Q responses_%MACHINE%.txt

rem findstr /c:"doAction(PLAY) OnDemandSess" ssm_tia*%.log > actPLAY_%MACHINE%.txt

findstr /c:"TianShanIce::ServerError" ssm_tianshan_s1* > SvrErr_%MACHINE%_t.txt
sed -i 's/ssm_tianshan_s1.*log://g' SvrErr_%MACHINE%_t.txt
sort SvrErr_%MACHINE%_t.txt > SvrErr_%MACHINE%.txt
del /Q SvrErr_%MACHINE%_t.txt

for %%i in (SETUP PLAY GET_PARAMETER TEARDOWN PAUSE) do findstr /c:"ContentHandler:%%i" SvrErr_%MACHINE%.txt > SvrErr_%%i_%MACHINE%.txt
findstr /c:".getInfo" SvrErr_%MACHINE%.txt >> SvrErr_GET_PARAMETER_%MACHINE%.txt

ren SvrErr_GET_PARAMETER_%MACHINE%.txt SvrErr_GET_PARAMETER_%MACHINE%-0.txt
sort SvrErr_GET_PARAMETER_%MACHINE%-0.txt > SvrErr_GET_PARAMETER_%MACHINE%.txt
del /Q SvrErr_GET_PARAMETER_%MACHINE%-0.txt
