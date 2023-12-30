use vars qw(@SMSGateway @struct);
my @struct=(
         ["key","description","name","default","type"]
        );

@SMSGateway = (
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","path of config file","ConfigFilePath","%EXEDIR%\\config.xml","S"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","clear database time in hour","DBClearHours","2","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","path of database","DBPath","%EXEDIR%\\NPVRSMSDB.mdb","S"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","HeartBeat Log Window","HeartBeatLogWindow","20","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","Log File Name of SMSGateway","LogFileName","%LOGDIR%\\SMSGateway.log","S"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","Log File Size","LogFileSize","100000","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","Log Level","LogLevel","7","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","max count of heartbeat lost","LostHeartBeatMaxCount","10","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","Over Time","OverTime","2","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","wait time of ticp result","SelectTimeOut","5000","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","service id","ServiceID","1500","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","ticp re operation times","Times","5","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","TM re operation times","TMTimes","5","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway","Wait short message response time","WaitTime","10000","D"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway_shell","Image Path","ImagePath","%EXEDIR%\\SMSGateway.exe","S"],
                ["SOFTWARE\\SeaChange\\NPVR\\CurrentVersion\\Services\\SMSGateway_shell","LogFile Path of SMSGateway_shell","LogFilePath","%LOGDIR%\\SMSGateway_shell.log","S"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SMSGateway","Management Port Number for SMSGateway","MgmtPortNumber","0x00001F5a","D"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\SMSGateway_shell","Management Port Number for SMSGateway_shell","MgmtPortNumber","0x00001F5b","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SMSGateway","The Event Message File of SMSGateway","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SMSGateway","The type of Supported of SMSGateway","TypesSupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SMSGateway_shell","The Event Message File of SMSGateway_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SMSGateway_shell","The type of Supported of SMSGateway_shell","TypesSupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\SMSGateway","Product Name","ProductName","NPVR","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\SMSGateway","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","S"],
                );