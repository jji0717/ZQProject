***************************************************************************************************************
* Readme
	PRODUCTION NAME: TianShanIceStorm

*Installation
1.	Install 3rd software
	Install Ice3.1.0 package on the machine.
        After installation, add system environment variable ICEROOT to specify the destination path.Then add <ICEROOT>\bin to the system path environment, the ICEROOT must be absolute path.

2.      Set installation path
        Edit "IceStorm_install.bat"

        For Example:
        set InatllPath=xxxx, you can modify it manunal, otherwise use the default value.Default is "C:\TianShan"

3.      Run IceStorm_install.bat


*Configuration

1). Edit config.service file under <InatllPath>\bin to set the IP and Port on it
IceStorm.TopicManager.Proxy
IceStorm.TopicManager.Endpoints
IceStorm.Publish.Endpoints

Freeze.DbEnv.IceStorm.DbHome ¨C set path to <InatllPath>\bin\data. This path must absolute path without using any system environment.

2). Edit config.icebox file under <InatllPath>\bin
IceBox.Service.IceStorm=IceStormService,31:createIceStorm --Ice.Config=<InatllPath>\bin\config.service

Notes: 
<InatllPath> is the absolute path which you specified at IceStorm_install.bat, default is C:\TianShan

*Start

      way 1.
            
          On a command,run:
                       net start TianShanIceStorm


      way 2.
        
          From Service management start TianShanIceStorm service

