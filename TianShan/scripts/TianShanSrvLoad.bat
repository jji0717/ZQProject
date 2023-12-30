 @Echo off

REM %1 sever ip
REM %2 database name, it used in SQL script in job 
REM %3 rtspproxy1 IP
REM %4 rtspproxy2 IP
REM %5 user name
REM %6 password

IF "%1"=="" GOTO :usage
IF "%2"=="" GOTO :usage
IF "%3"=="" GOTO :usage
IF "%4"=="" GOTO :usage
IF "%5"=="" GOTO :usage

:remote

IF exist temp_script_for_create_job_tianshan_srvload.sql DEL temp_script_for_create_job_tianshan_srvload.sql
type create_job_tianshan_srvload.sql.1 > temp_script_for_create_job_tianshan_srvload.sql
ECHO Set @db_name = '%2' >> temp_script_for_create_job_tianshan_srvload.sql
ECHO Set @rtspPrxyIP1 = '%3' >> temp_script_for_create_job_tianshan_srvload.sql
ECHO Set @rtspPrxyIP2 = '%4' >> temp_script_for_create_job_tianshan_srvload.sql
type create_job_tianshan_srvload.sql.2 >> temp_script_for_create_job_tianshan_srvload.sql

echo executing TianShanSrvLoad.sql ...
isql -d %2 -U %5 -P %6 -S %1 < TianShanSrvLoad.sql

echo creating TianShanSrvLoad job ...
isql -d %2 -U %5 -P %6 -S %1 < temp_script_for_create_job_tianshan_srvload.sql

DEL temp_script_for_create_job_tianshan_srvload.sql

GOTO :end

:usage
echo.
echo Usage:   TianShanSrvLoad.bat [ServerIP] [DatabaseName] [RtspProxy1IP] [RtspProxy2IP] [UserName] [Password] 
echo.
echo [ServerIP]:     The Server IP that database running on
echo [RtspProxy1IP]: The IP address for RTSPProxy instance 1 running on
echo [RtspProxy2IP]: The IP address for RTSPProxy instance 2 running on
echo [DatabaseName]: The database name the task operating on.
echo [UserName]:     The username to execute the schedule
echo [Password]:     The password of the UserName, can be empty if no password specified
echo.
echo * Example:
echo     TianShanSrvLoad.bat 10.9.0.22 multiverse 10.9.0.32 10.9.0.34 sa sapassword 
echo.
echo After run the command, SQL Server will periodically update TianShan RTSPProxy srvload information
echo.
pause

:end
