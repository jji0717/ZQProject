use vars qw(@MessageAgent);
my @struct=(
         ["key","description","name","default","type"]
        );

@MessageAgent = (
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Agent Config File","AgentConfigFile","%CONFIGDIR%\\JmsTopicConfig.xml","S"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Jms Server IP","JmsServerIP","You must set it manually","S"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Jms Server Port","JmsServerPort","You must set it manually","D"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Keep Alive Intervals","KeepAliveIntervals","0x000007d0","D"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Log Buffer Size","LogBufferSize","0x00001000","D"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Log File Name","LogFileName","%LOGDIR%\\MessageAgent.log","S"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Log File Size","LogFileSize","0x000f4240","D"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Log File TimeOut","LogFileTimeOut","0x000007d0","D"],
                ["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Log Level","LogLevel","7","D"],
		["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","Safestore Path","SafestorePath","%SAFESTOREDIR%","S"],
		["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent","TopicManager Endpoints","TopicManagerEndpoints","TianShanEvents/TopicManager:default -h <IPAddress> -p <Port>","S"],
		["SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\MessageAgent_shell","Image Path","ImagePath","%EXEDIR%\\MessageAgent.exe","S"],
		["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\MessageAgent","Management Port Number of MessageAgent","MgmtPortNumber","0x000013c2","D"],
		["SYSTEM\\CurrentControlSet\\Services\\MessageAgent","Product Name","ProductName","TianShan","S"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\MessageAgent","The Event Message File of MessageAgent","EventMessageFile","%EXEDIR%\\ItvMessages.dll","S"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\MessageAgent","The type of Supported of MessageAgent","Typessupported","7","D"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\MessageAgent_shell","The Event Message File Of MessageAgent_shell","EventMessageFile","%EXEDIR%\\ShellMsgs.dll","S"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\MessageAgent_shell","The type of Supported Of MessageAgent_shell","Typessupported","7","D"]
                );