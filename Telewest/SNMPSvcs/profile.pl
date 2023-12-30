use vars qw(@SNMPSvc @struct @Filelist);
my @struct=(
         ["key","description","name","default","type","display or not, 1 enable, 0 disable"]
        );
@Filelist = (
              'SNMPSvcs.exe',
              'snmp_pp.dll',
	      'boost_regex_vc6_mdi.dll',
              'snmp.ini',
              'cfgpkgU.dll',
              'CLog.dll',
              'ctail.exe',
              'instserv.exe',
              'ItvMessages.dll',
              'ManPkgU.dll',
              'MCastSvc.dll',
              'MtTcpComm.dll',
              'queue.dll',
              'Reporter.dll',
              'ReporterMsgs.dll',
              'ScThreadPool.dll',
              'ShellMsgs.dll',
              'srvshell.exe',
              );

@SNMPSvcs = (
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","Instance","Instance","1","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","Community","Community","public","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","KeepAliveIntervals","KeepAliveIntervals","2000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogBufferSize","LogBufferSize","4096","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileName","LogFileName","%LOGDIR%\\SnmpSvcs.log","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileSize","LogFileSize","10240000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileTimeOut","LogFileTimeOut","2000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogLevel","LogLevel","7","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","ShutdownWaitTime","ShutdownWaitTime","200","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","SuperviseInterval","SuperviseInterval","60","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","ConfigFileName","ConfigFileName","%EXEDIR%\\snmp.ini","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","SenderAddress","SenderAddress","127.0.0.1","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename_shell","ImagePath","ImagePath","%EXEDIR%\\SNMPSvcs.exe","S","1"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\$servicename","Management Port Number for SNMP service","MgmtPortNumber","0x00001B22","D","1"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\$servicename_shell","Management Port Number for SNMP service shell","MgmtPortNumber","0x00001B23","D","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\$servicename","Product Name","ProductName","ITV","S","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\$servicename","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","S","1"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename","The Event Message File of NSSync","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S","1"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename","The type of Supported of SNMP service","Typessupported","7","D","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename_shell","The Event Message File Of SNMP service shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename_shell","The type of Supported Of SNMP service shell","Typessupported","7","D","1"],
                );