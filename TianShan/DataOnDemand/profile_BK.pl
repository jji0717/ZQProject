
use vars qw(@DataStream @DODApp @All_Services);
my @struct=(
         ["key","description","name","default","type","display or not, 1 enable, 0 disable"]
        );

@DataStream = (
		["SYSTEM\\CurrentControlSet\\Services\\DataStream","Image Path","ImagePath","%EXEDIR%\\DataStream.exe","S","1"],
               );
              
@DODApp = (
                ["SOFTWARE\\Seachange\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogFile Name for DODApp","LogFileName","%LOGDIR%\\DODAppLogFile.log","S","1"],
                ["SOFTWARE\\Seachange\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogFile Size","LogFileSize","200000000","D","1"],
                ["SOFTWARE\\Seachange\\DataOnDemand\\CurrentVersion\\Services\\DODApp_Shell","Image Path","ImagePath","%EXEDIR%\\DODApp.exe","S","1"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\DODApp","Management Port Number of DODApp","MgmtPortNumber","0x00001F4A","D","0"],
                ["SOFTWARE\\SeaChange\\Management\\CurrentVersion\\Services\\DODApp_Shell","Management Port Number of DODApp_shell","MgmtPortNumber","0x00001F4B","D","0"],
		["SYSTEM\\CurrentControlSet\\Services\\DODApp","Product Name","ProductName","DataOnDemand","S","0"],
		["SYSTEM\\CurrentControlSet\\Services\\DODApp","Image Path","ImagePath","%EXEDIR%\\SrvShell.exe","S","0"],
		["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","The Event Message File of DODApp","EventMessageFile","%EXEDIR%\\ItvMessages.dll","E","0"],
                ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","The type of Supported of DODApp","Typessupported","7","D","0"],
               
               );            
               
@All_Services = (
                 DataStream,
                 DODApp,
                 );