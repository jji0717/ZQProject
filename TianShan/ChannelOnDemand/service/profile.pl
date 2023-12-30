use vars qw(@ChOdSvc);

($SVCACCOUNT, $SVCPASSWD)
=  (".\\SeaChange", "SeaChange");

@ChOdSvc = (
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Log File Name","LogFileName","%LOGDIR%\\Chodsvc.log","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Log File Size","LogFileSize","0x00989680","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Keep Alive Intervals","KeepAliveIntervals","0x00003a98","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Crash Dump Path","CrashDumpPath","%CRASHDUMPPATH%","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Evictor size for channelpublishpoint","ChannelPublishPointEvictorSize","40","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Evictor size for purchase","PurchaseEvictorSize","400","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Evictor size for purchaseitem","PurchaseItemEvictorSize","2000","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Default max channel bitrate","DefaultChannelMaxBitrate","5000000","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","ChannelOnDemand Safestore Path","SafestorePath","c:\\TianShan\\CODSS","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","real-time stream protect time, in ms","PlayProtectTimeInMs","10000","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","ChannelOnDemand EndPoint, the IP is the local ip,must be set","ChannelOnDemandEndPoint","tcp -h x.x.x.x -p 9832","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Topic Manager EndPoint that StreamSmith registered, must be set","TopicManagerEndPoint","tcp -h x.x.x.x -p 10000","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","JBoss server ip port, must be set","JMSServerIPPort","192.168.80.70:1099","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","JMS message destination name","JMSDestinationName","queue/TodQueue","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","JMS safestore file","JMSMessageStoreFile","%EXEDIR%\\CodJmsStore.txt","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","JMS message propertys","JMSMessagePropertys","string,MESSAGECLASS,NOTIFICATION;int,MESSAGECODE,1101","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Set to 1 to enable todas validate, 0 for disable","TodasEnable","1","REG_DWORD"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","Todas endpoint, must be set, format:TodasForCod:tcp -h IP -p port","TodasEndPoint","TodasForCod:tcp -h 192.168.80.70 -p 6789","REG_SZ"],
           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc","ChannelOnDemand thread pool size","ThreadPoolSize","20","REG_DWORD"],

           ["SOFTWARE\\Seachange\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","Image path","ImagePath","%EXEDIR%\\ChodSvc.exe","REG_SZ"],
           ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\ChOdSvc","Managment Port Number for ChannelOnDemand","MgmtPortNumber","0x00002cfd","REG_DWORD"],
           ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\ChOdSvc_shell","Managment Port Number for ChannelOnDemand service shell","MgmtPortNumber","0x00002cfe","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\ChodSvc","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\ChodSvc","ImagePath of SrvShell","ImagePath","%EXEDIR%\\SrvShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ChOdSvc","log message dll file","EventMessageFile","%EXEDIR%\\itvmessages.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ChOdSvc","log message support type","TypesSupported","7","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","class path for JMS message","classpath","%EXEDIR%\\JMSClient;%EXEDIR%\\JMSClient\\jbossall-client.jar","REG_EXPAND_SZ"],
           );