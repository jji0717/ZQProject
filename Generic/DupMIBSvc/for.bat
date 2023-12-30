DupMIBSvc -s 1400 -n 0 -o nss.MIB TianShan.MIB
for /L %%i in (1 1 9) do (
 echo "%%i number is "%%i
 DupMIBSvc -s 1400 -n %%i -o nss%%i.MIB TianShan.MIB
) 