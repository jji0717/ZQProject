use vars qw(@struct @MODPlugIn);

my @struct=(
         ["key","description","name","default","type","prompt or not"]
        );
@MODPlugIn = (
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Path of authorization plug-in DLL for MOD1","AuthorizationDllPath","%EXEDIR%","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Dll name of authorization plug-in DLL for MOD1","AuthorizationDllName","MultiverseAuthorization.dll","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Flag. if TRUE, then authorize when check errors for MOD1","AuthorizeOnError","0","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Minimun number of threads for MOD1","AuthorizationDllThreadPoolMin","10","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Maximum number of threads for MOD1","AuthorizationDllThreadPoolMax","20","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Number of threads to grow by when pool exhausted for MOD1","AuthorizationDllThreadPoolGrowBy","5","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Time to wait before releasing idle threads for MOD1","AuthorizationDllThreadPoolIdleTime","10","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Time to wait for outstanding threads to complete for MOD1","AuthorizationDllThreadPoolReleaseTime","12000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand","Time to wait for new job to start for MOD1","AuthorizationDllThreadPoolStartTime","12000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","LogFile Path for MOD1 AuthorizationDLL","LogFilePath","%LOGDIR%\\MultiverseAuthorization.log","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","LogFile Size for MOD1AuthorizationDLL","LogFileSize","10000000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","LogFile TraceLevel for MOD1AuthorizationDLL","LogFileTraceLevel","1000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","The WFES wsdl URL for MOD1AuthorizationDLL","SOAPWsdlPath","http://ZAPWFES:9090/services/MoDSoapInterface?wsdl","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","Timeout in milliseconds to connect to WFES for MOD1AuthorizationDLL","SOAPConnectTimeout","5000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","Re-try count once connect to WFES failed for MOD1AuthorizationDLL","SOAPRetryCount","5","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand\\AuthorizationDLL","Configuration file for multiple WFES setting for MOD1AuthorizationDLL","AppSitesConfigurationFile","%EXEDIR%\\MultiverseAuthorization.xml","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Path of authorization plug-in DLL for MOD2","AuthorizationDllPath","%EXEDIR%","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Dll name of authorization plug-in DLL for MOD2","AuthorizationDllName","MultiverseAuthorization.dll","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Flag. if TRUE, then authorize when check errors for MOD2","AuthorizeOnError","0","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Number of threads to grow by when pool exhausted for MOD2","AuthorizationDllThreadPoolMin","10","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Maximum number of threads for MOD2","AuthorizationDllThreadPoolMax","20","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Number of threads to grow by when pool exhausted for for MOD2","AuthorizationDllThreadPoolGrowBy","5","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Time to wait before releasing idle threads for MOD2","AuthorizationDllThreadPoolIdleTime","10","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Time to wait for outstanding threads to complete for MOD2","AuthorizationDllThreadPoolReleaseTime","12000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2","Time to wait for new job to start for MOD2","AuthorizationDllThreadPoolStartTime","12000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","LogFile Path for MOD2 AuthorizationDLL","LogFilePath","%LOGDIR%\\MultiverseAuthorization2.log","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","LogFile Size for MOD2 AuthorizationDLL","LogFileSize","10000000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","LogFile TraceLevel for MOD2 AuthorizationDLL","LogFileTraceLevel","1000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","The WFES wsdl URL for MOD2 AuthorizationDLL","SOAPWsdlPath","http://ZAPWFES:9090/services/MoDSoapInterface?wsdl","S","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","Timeout in milliseconds to connect to WFES for MOD2 AuthorizationDLL","SOAPConnectTimeout","5000","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","Re-try count once connect to WFES failed for MOD2 AuthorizationDLL","SOAPRetryCount","5","D","0"],
	["SOFTWARE\\SeaChange\\ITV Applications\\CurrentVersion\\services\\Movies On Demand2\\AuthorizationDLL","Configuration file for multiple WFES setting for MOD2 AuthorizationDLL","AppSitesConfigurationFile","%EXEDIR%\\MultiverseAuthorization.xml","S","0"],
);