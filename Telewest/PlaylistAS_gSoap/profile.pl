use vars qw(@PlaylistAS @struct @Filelist);
my @struct=(
         ["key","description","name","default","type","display or not, 1 enable, 0 disable"]
        );
@Filelist = (
              'PlaylistAS.exe',
              'cfgpkgU.dll',
              'CLog.dll',
              'ctail.exe',
              'instserv.exe',
              'ItvMessages.dll',
              'ManPkgU.dll',
              'MCastSvc.dll',
              'MFC71u.dll',
              'msvcp71.dll',
              'msvcr71.dll',
              'MtTcpComm.dll',
              'queue.dll',
              'Reporter.dll',
              'ReporterMsgs.dll',
              'ScThreadPool.dll',
              'ShellMsgs.dll',
              'srvshell.exe',
              );

@PlaylistAS = (
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","Instance","Instance","1","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","KeepAliveIntervals","KeepAliveIntervals","2000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogBufferSize","LogBufferSize","4096","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileName","LogFileName","%LOGDIR%\\PlaylistAS.log","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileSize","LogFileSize","40960000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogFileTimeOut","LogFileTimeOut","2000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","LogLevel","LogLevel","7","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","ShutdownWaitTime","ShutdownWaitTime","10000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","ISS Application UID","ISSAppUID","80002","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","ISS Instance ID","ISSInst","1","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","Soap endpoint","SoapWSDLFilePath","http://ZAPWFES:9090/services/PlaylistSoapInterface?wsdl","S","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename","Soap call timeout","SoapTimeout","10000","D","1"],
                ["SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\Services\\$servicename_shell","ImagePath","ImagePath","%EXEDIR%\\PlaylistAS.exe","S","1"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\$servicename","Management Port Number for PlaylistAS","MgmtPortNumber","6940","D","1"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\$servicename_shell","Management Port Number for PlaylistAS_shell","MgmtPortNumber","6941","D","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\$servicename","Product Name","ProductName","ITV","S","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\$servicename","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","S","1"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename","The Event Message File of PlaylistAS","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S","1"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename","The type of Supported of PlaylistAS","Typessupported","7","D","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename_shell","The Event Message File Of PlaylistAS_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S","1"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$servicename_shell","The type of Supported Of PlaylistAS_shell","Typessupported","7","D","1"],
                );