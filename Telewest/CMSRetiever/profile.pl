use vars qw(@SrvLoad @struct @Filelist);
my @struct=(
         ["key","description","name","default","type","display or not, 1 enable, 0 disable"]
        );
        
@Filelist = (
             'SrvLoad.exe',
             'cfgpkgU.dll',
             'CLog.dll',
             'ctail.exe',
             'Ipsapi.dll',
             'ItvMessages.dll',
             'ManPkgU.dll',
             'MCastSvc.dll',
             'mfc42u.dll',
             'msvcp60.dll',
             'MtTcpComm.dll',
             'queue.dll',
             'Reporter.dll',
             'ScThreadPool.dll',
             'ShellMsgs.dll',
             );      
  
@SrvLoad = (      
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Instance","Instance","1","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileName","LogFileName","%LOGDIR%\\SrvLoad.log","S","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileSize","LogFileSize","20000000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogFileTimeOut","LogFileTimeOut","2000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogLevel","LogLevel","7","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","LogBufferSize","LogBufferSize","2000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","ShutdownWaitTime","ShutdownWaitTime","3000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","KeepAliveIntervals","KeepAliveIntervals","3000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Configuration file for multiple WFES setting","AppSitesConfigurationFile","%EXEDIR%\\SrvLoadAppsitesCfg.xml","S","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","DSN name for SrvLoad to connect local database","LAMDBDSN","sqlserver","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Username for SrvLoad to access local database","LAMDBNAME","multiverse","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","Password for SrvLoad to access local database","LAMDBPASSWORD","multiverse","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","The ITV srvload xml file name","LOADFILENAME","You must set it manual","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad","The interval time in millisecond to re-load the srvload xml file","READINTERVAL","2000","D","0"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\SrvLoad_shell","ImagePath","ImagePath","%EXEDIR%\\SrvLoad.exe","S","0"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SrvLoad","Managenent Port Number for SrvLoad","MgmtPortNumber","0x000018B1","D","0"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SrvLoad_shell","Managenent Port Number for SrvLoad_shell","MgmtPortNumber","0x000018B2","D","0"],
                ["SYSTEM\\CurrentControlSet\\Services\\SrvLoad","Product Name","ProductName","ITV","S","0"],
                ["SYSTEM\\CurrentControlSet\\Services\\SrvLoad","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","S","0"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad","The Event Message File of SrvLoad","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S","0"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad","The type of Supported of SrvLoad","Typessupported","7","D","0"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad_shell","The Event Message File Of SrvLoad_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S","0"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SrvLoad_shell","The type of Supported Of SrvLoad_shell","Typessupported","7","D","0"],
                );