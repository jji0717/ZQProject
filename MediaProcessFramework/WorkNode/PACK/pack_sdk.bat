rd /s /q "package"
md package

cd package
md bin
md include
md lib
md help
md samples

cd include
md worknode
md xmlrpc

cd..
cd..
cd..

doxygen

cd PACK

copy "..\Debug\worknode_d.dll" "package\bin\worknode_d.dll"
copy "..\Debug\worknode_d.lib" "package\lib\worknode_d.lib"
copy "..\Release\worknode.dll" "package\bin\worknode.dll"
copy "..\Release\worknode.lib" "package\lib\worknode.lib"

copy "..\..\XmlRpc\XMLRPC_d.dll" "package\bin\XMLRPC_d.dll"
copy "..\..\XmlRpc\XMLRPC_d.lib" "package\lib\XMLRPC_d.lib"
copy "..\..\XmlRpc\XMLRPC.dll" "package\bin\XMLRPC.dll"
copy "..\..\XmlRpc\XMLRPC.lib" "package\lib\XMLRPC.lib"

copy "..\*.h" "package\include\worknode\*.h"

copy "..\..\MPFCommon.h" "package\include\worknode\MPFCommon.h"
copy "..\..\MPFLogHandler.h" "package\include\worknode\MPFLogHandler.h"
copy "..\..\listinfo.h" "package\include\worknode\listinfo.h"
copy "..\..\listinfo_def.h" "package\include\worknode\listinfo_def.h"
copy "..\..\MPFVersion.h" "package\include\worknode\MPFVersion.h"
copy "..\..\SystemInfo.h" "package\include\worknode\SystemInfo.h"
copy "..\..\SystemInfo_def.h" "package\include\worknode\SystemInfo_def.h"

copy "..\..\MPFUtils.cpp" "package\samples\MPFUtils.cpp"


copy "..\..\..\common\getopt.h" "package\samples\getopt.h"
copy "..\..\..\common\getopt.cpp" "package\samples\getopt.cpp"
copy "..\..\..\common\nativethread.cpp" "package\samples\nativethread.cpp"

copy "..\..\..\common\exception.h" "package\include\worknode\exception.h"
copy "..\..\..\common\locks.h" "package\include\worknode\locks.h"
copy "..\..\..\common\zq_common_conf.h" "package\include\worknode\zq_common_conf.h"
copy "..\..\..\common\nativethread.h" "package\include\worknode\nativethread.h"
copy "..\..\..\common\log.h" "package\include\worknode\log.h"
copy "..\..\..\common\comextra\zqsafemem.h" "package\include\worknode\zqsafemem.h"

copy "..\..\XmlRpc\*.h" "package\include\xmlrpc\*.h"

copy "..\worknode.chm" "package\help\worknode.chm"
copy "..\..\XmlRpc\xmlrpc.chm" "package\help\xmlrpc.chm"

copy "..\..\Samples\WNApps\*.h" "package\samples\*.h"
copy "..\..\Samples\WNApps\*.cpp" "package\samples\*.cpp"
copy "*.dsp" "package\samples\*.dsp"
copy "*.dsw" "package\samples\*.dsw"

copy "..\ReleaseNotes.txt" "package\ReleaseNotes.txt"

attrib -R "package" /S /D

cd package\include\xmlrpc
del stdafx.h
cd ..\worknode
del stdafx.h

cd..
cd..
cd..

