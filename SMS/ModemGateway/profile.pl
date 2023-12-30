use vars qw(@ModemGateway @struct);
my @struct=(
         ["key","description","name","default","type"]
        );

@ModemGateway = (
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","path of config file","ConfigFile","%EXEDIR%\\config.xml","S"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Number of ComPort","ComPort","1","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","path of database","DBPath","%EXEDIR%\\NPVRSMSDB.mdb","S"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Modem echo","Echo","1","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Interval of TICP operation","Interval","5000","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","log file name","LogFileName","%LOGDIR%\\ModemGateway.log","S"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Log File Size","LogFileSize","100000","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Log Level","LogLevel","7","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","short message over time","OverTime","2","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Ip address of TICP server, you must set","TicpIpAddress","<IPAddress>","S"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","Ip port of TICP server","TicpPort","61232","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway","operating times with TICP","Times","5","D"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway_shell","ImagePath","ImagePath","%EXEDIR%\\ModemGateway.exe","S"],
                ["SOFTWARE\\SeaChange\\TelVod\\CurrentVersion\\Services\\ModemGateway_shell","Log File Path for ModemGateway_shell","LogFilePath","%LOGDIR%\\ModemGateway_shell.log","S"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\ModemGateway","Management Port Number for ModemGateway","MgmtPortNumber","0x00001F48","D"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\ModemGateway_shell","Management Port Number for ModemGateway_shell","MgmtPortNumber","0x00001F49","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ModemGateway","The Event Message File of ModemGateway","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ModemGateway","The type of Supported of ModemGateway","TypesSupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ModemGateway_shell","The Event Message File of ModemGateway_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ModemGateway_shell","The type of Supported of ModemGateway_shell","TypesSupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\ModemGateway","Product Name","ProductName","TelVod","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\ModemGateway","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","S"],
                );