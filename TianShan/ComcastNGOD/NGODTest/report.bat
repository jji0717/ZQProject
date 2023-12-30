rem must run under D:\NGODTest with MS office installed

if .%1. == .. goto quit

mkdir report

copy report\%1.mdb ngod2_test.mdb
copy Template\Ngod2Report.xls report\%1.xls

pause
start /wait report\%1.xls

del /q report\%1.xls

:quit
