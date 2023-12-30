use vars qw(@SrvLoad @struct);
my @struct=(
         ["key","description","name","default","type"]
        );
        
  
@SrvLoad = (      
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Instance","Instance","1","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileName","LogFileName","%LOGDIR%\\SrvLoad.log","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileSize","LogFileSize","20000000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileTimeOut","LogFileTimeOut","2000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogLevel","LogLevel","7","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogBufferSize","LogBufferSize","2000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","ShutdownWaitTime","ShutdownWaitTime","3000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","KeepAliveIntervals","KeepAliveIntervals","3000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Configuration file for multiple WFES setting","AppSitesConfigurationFile","%EXEDIR%\\SrvLoadAppsitesCfg.xml","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","DSN name for SrvLoad to connect local database","LAMDBDSN","sqlserver","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Username for SrvLoad to access local database","LAMDBNAME","multiverse","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Password for SrvLoad to access local database","LAMDBPASSWORD","multiverse","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","The ITV srvload xml file name","LOADFILENAME","You must set it manual","S"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","The interval time in millisecond to re-load the srvload xml file","READINTERVAL","2000","D"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad_shell","ImagePath","ImagePath","%EXEDIR%\\SrvLoad.exe","S"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SrvLoad","Managenent Port Number for SrvLoad","MgmtPortNumber","0x000018B1","D"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SrvLoad_shell","Managenent Port Number for SrvLoad_shell","MgmtPortNumber","0x000018B2","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\SrvLoad","Product Name","ProductName","ITV","S"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad","The Event Message File of SrvLoad","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad","The type of Supported of SrvLoad","Typessupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad_shell","The Event Message File Of SrvLoad_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad_shell","The type of Supported Of SrvLoad_shell","Typessupported","7","D"],
                );