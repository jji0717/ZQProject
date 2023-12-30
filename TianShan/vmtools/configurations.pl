# -----------------
# global declares
# -----------------

$TIANSHAN_HOME="D:/TianShan";
$BUILDZIP_HOME="//hawkeye/build";
$TOOLS_HOME="C:/TsVm_tools";

my @now = (localtime(time))[0,1,2,3,4,5];
$UPDATE_TIME = sprintf "%04d-%02d-%02d %02d:%02d", $now[5]+1900, $now[4]+1, $now[3], $now[2], $now[1], $now[0];
$TIMESTAMP = sprintf "%04d%02d%02dT%02d%02d", $now[5] +1900, $now[4]+1, $now[3], $now[2], $now[1], $now[0];

