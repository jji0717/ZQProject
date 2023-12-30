rd /s /q "package"
md package

cd package
md bin
md include
md lib
md help
md samples
cd include
md srmapi
md xmlrpc
md requestposter
md entrydb
md common
cd common
md comextra
cd..
cd..
cd..

copy "..\*.h" "package\include\srmapi\*.h"
copy "..\..\*.h" "package\include\*.h"

copy "..\srm.dll" "package\bin\srm.dll"
copy "..\srm_d.dll" "package\bin\srm_d.dll"
copy "..\..\EntryDB\build_win32\release\edbb4.edm" "package\bin\edbb4.edm"
copy "..\..\EntryDB\build_win32\release\edos.dll" "package\bin\edos.dll"
copy "..\..\EntryDB\build_win32\release\edbsh.exe" "package\bin\edbsh.exe"
copy "..\..\xmlrpc\xmlrpc.dll" "package\bin\xmlrpc.dll"
copy "..\..\xmlrpc\xmlrpc_d.dll" "package\bin\xmlrpc_d.dll"
copy "..\..\RequestPoster\requestposter.dll" "package\bin\requestposter.dll"
copy "..\..\RequestPoster\requestposter_d.dll" "package\bin\requestposter_d.dll"

copy "..\..\RequestPoster\requestposter.lib" "package\lib\requestposter.lib"
copy "..\..\requestposter\requestposter_d.lib" "package\lib\requestposter_d.lib"
copy "..\..\xmlrpc\xmlrpc.lib" "package\lib\xmlrpc.lib"
copy "..\..\xmlrpc\xmlrpc_d.lib" "package\lib\xmlrpc_d.lib"
copy "..\..\entrydb\lib\edos.lib" "package\lib\edos.lib"
copy "..\srm.lib" "package\lib\srm.lib"
copy "..\srm_d.lib" "package\lib\srm_d.lib"

copy "..\..\samples\mnapps\udreceiver.cpp" "package\samples\udreceiver.cpp"
copy "updatereceiver.dsp" "package\samples\updatereceiver.dsp"
copy "samples.dsw" "package\samples\samples.dsw"
copy "readme.txt" "package\readme.txt"

copy "..\..\xmlrpc\xmlrpc.chm" "package\help\xmlrpc.chm"
copy "..\srmapi.chm" "package\help\srmapi.chm"
copy "..\..\requestposter\requestposter.chm" "package\help\requestposter.chm"

copy "..\..\xmlrpc\*.h" "package\include\xmlrpc\*.h"

copy "..\..\requestposter\*.h" "package\include\requestposter\*.h"

copy "..\..\..\common\ZQ_common_conf.h" "package\include\common\zq_common_conf.h"
copy "..\..\..\common\locks.h" "package\include\common\locks.h"
copy "..\..\..\common\comextra\zqcommon.h" "package\include\common\comextra\zqcommon.h"
copy "..\..\..\common\comextra\zqsafemem.h" "package\include\common\comextra\zqsafemem.h"
copy "..\..\..\common\comextra\zqbuffer.h" "package\include\common\comextra\zqbuffer.h"
copy "..\..\..\common\comextra\zqsafemem.cpp" "package\samples\zqsafemem.cpp"
copy "..\..\..\common\nativethread.h" "package\include\common\nativethread.h"
copy "..\..\..\common\getopt.h" "package\samples\getopt.h"
copy "..\..\..\common\getopt.cpp" "package\samples\getopt.cpp"

copy "..\..\entrydb\*.h" "package\include\entrydb\*.h"
