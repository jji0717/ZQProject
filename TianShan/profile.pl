use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD 
            %ChOdSvc %StreamSmith %RtspProxy %RtspProxy2 %TianShanRDS %ClusterContentStore %NodeContentStore %NASContentStore %RTFCPNode %RTICPNode %Weiwoo %MODSvc %GBMODSvc %SiteAdminSvc %TianShanRDA %Sentry %CPESvc %EventGateway %EventChannel %NSS %MediaClusterCS %DODApp %DODContentStore %DataStream %VLCVSS %BcastChannel %CDNCS %CDNSS %HttpCRG %EdgeRM %ContentLib %EventRE %GBVSS %TSPump %DsmccCRG %GBCS %DummySvc %C2SS
            @ChOdSvc @StreamSmith @RtspProxy @RtspProxy2 @TianShanRDS @ClusterContentStore @NodeContentStore @NASContentStore @RTFCPNode @RTICPNode @Weiwoo @MODSvc @GBMODSvc @SiteAdminSvc @TianShanRDA @Sentry @CPESvc @EventGateway @EventChannel @NSS @MediaClusterCS @DODApp @DODContentStore @DataStream @VLCVSS @BcastChannel @CDNCS @CDNSS @HttpCRG @EdgeRM @ContentLib @EventRE @GBVSS @TSPump @DsmccCRG @GBCS @DummySvc @C2SS @All_Services);



($INSTALLROOT, $SVCACCOUNT, $SVCPASSWORD)
=  ("C:\\TianShan", "LocalSystem", "");


%ProjTemplate = (
                   'name'             => 'service name', #it is a hard code, except service ClusterContentStore
                   'desc'             => 'service description', # should not been modified
                   'Registry'         => 'configuration default setting for service',
                   'configfile'	      => 'config file for service'		
                   );


%ChOdSvc = (
                     'name'             => 'ChOdSvc',
                     'desc'             => 'TianShan ChannelOnDemand Service',
                     'Registry'         => \@ChOdSvc,
                     'ServiceOID'	=> '800',
                     'configfile'	=> ["ChannelOnDemand","CODEvent","syntax"],
                   );
%StreamSmith = (
               'name'             => 'StreamSmith',
               'desc'             => 'TianShan StreamSmith Service',
               'Registry'         => \@StreamSmith,
               'ServiceOID'	  => '100',
               'configfile'	=> ["StreamSmith","syntax"],
                 );
%RtspProxy = (
               'name'             => 'RtspProxy',
               'desc'             => 'TianShan RtspProxy Service',
               'Registry'         => \@RtspProxy,
               'ServiceOID'	  => '1000',
               'configfile'	=> ["RtspProxy","ssm","RTSPscript","NGODHist","keyfile","syntax"],
                 );
%RtspProxy2 = (
               'name'             => 'RtspProxy2',
               'desc'             => 'TianShan RtspProxy Service',
               'Registry'         => \@RtspProxy2,
               'ServiceOID'	  => '1020',
               'configfile'	=> ["RtspProxy","ssm","RTSPscript","NGODHist","keyfile","syntax"],
               );                
%Weiwoo = (
               'name'             => 'Weiwoo',
               'desc'             => 'TianShan Weiwoo Service',
               'Registry'         => \@Weiwoo,
               'ServiceOID'	  => '200',
               'configfile'	=> ["Weiwoo","pho","syntax"],
                 );   
                 
%MODSvc = (
               'name'             => 'MODSvc',
               'desc'             => 'TianShan MOD Application Service',
               'Registry'         => \@MODSvc,
               'ServiceOID'	  => '900',
               'configfile'	=> ["MovieOnDemand"],
                 );
                 
%GBMODSvc = (
               'name'             => 'GBMODSvc',
               'desc'             => 'TianShan  GBMOD Application Service',
               'Registry'         => \@GBMODSvc,
               'ServiceOID'	  => '2800',
               'configfile'	=> ["GBMovieOnDemand"],
                 );
                 
%BcastChannel = (
               'name'             => 'BcastChannel',
               'desc'             => 'TianShan BroadcastChannel Serveice',
               'Registry'         => \@BcastChannel,
               'ServiceOID'	=> '1900',
               'configfile'	=> ["BroadcastChannel","syntax"],
                 );                 
                 
%SiteAdminSvc = (
               'name'             => 'SiteAdminSvc',
               'desc'             => 'TianShan SiteAdmin Service',
               'Registry'         => \@SiteAdminSvc,
               'ServiceOID'	  => '300',
               'configfile'	=> ["SiteAdminSvc","EventSenderForSiteAdmin"],
                 );   
                 
%Sentry = (
               'name'             => 'Sentry',
               'desc'             => 'TianShan Sentry Service',
               'Registry'         => \@Sentry,
               'ServiceOID'	=> '1100',
               'configfile'	=> ["Sentry","Weblayout","MsgSender","snmpsender","NGODHist","syntax"],
                 );        
                 
%CPESvc = (
               'name'             => 'CPESvc',
               'desc'             => 'TianShan CPESvc Service',
               'Registry'         => \@CPESvc,
               'ServiceOID'	=> '700',
               'configfile'	=> ["CPE","CPH","syntax"],
                 );                                          

%EventGateway = (
               'name'             => 'EventGateway',
               'desc'             => 'TianShan EventGateway Service',
               'Registry'         => \@EventGateway,
               'ServiceOID'	=> '1200',
               'configfile'	=> ["EventGateway","EGH","syntax"],
                 ); 
                 
%EventChannel = (
               'name'             => 'EventChannel',
               'desc'             => 'TianShan EventChannel Service',
               'Registry'         => \@EventChannel,
               'ServiceOID'	=> '1300',
               'configfile'	=> ["EventChannel","syntax"],
                 );
                 
%NSS = (
               'name'             => 'NSS',
               'desc'             => 'NGOD Streaming Service',
               'Registry'         => \@NSS,
               'ServiceOID'	=> '1400',
               'configfile'	=> ["NSS","syntax"],
                 ); 
                 
%VLCVSS = (
               'name'             => 'VLCVSS',
               'desc'             => 'TianShan VLCVSS Service',
               'Registry'         => \@VLCVSS,
               'ServiceOID'	=> '3000',
               'configfile'	=> ["VLCVSS","syntax"],
                 ); 
                 
%GBVSS = (
               'name'             => 'GBVSS',
               'desc'             => 'TianShan GBVSS Service',
               'Registry'         => \@GBVSS,
               'ServiceOID'	=> '2900',
               'configfile'	=> ["GBVSS","syntax"],
                 );     
                 
%TSPump = (
               'name'             => 'TSPump',
               'desc'             => 'TianShan TSPump Service',
               'Registry'         => \@TSPump,
               'ServiceOID'	=> '2600',
               'configfile'	=> ["TSPump","syntax"],
                 );  
                 
%DsmccCRG = (
               'name'             => 'DsmccCRG',
               'desc'             => 'TianShan DsmccCRG Service',
               'Registry'         => \@DsmccCRG,
               'ServiceOID'	=> '2700',
               'configfile'	=> ["DsmccCRG","CRM_Dsmcc","syntax"],
                 );                                                                                  
                 
%MediaClusterCS = (
               'name'             => 'MediaClusterCS',
               'desc'             => 'TianShan MediaClusterCS Serveice',
               'Registry'         => \@MediaClusterCS,
               'ServiceOID'	=> '1500',
               'configfile'	=> ["MediaClusterCS","syntax"],
                 );
                  
%CDNCS = (
               'name'             => 'CDNCS',
               'desc'             => 'TianShan CDNCS Serveice',
               'Registry'         => \@CDNCS,
               'ServiceOID'	=> '2000',
               'configfile'	=> ["CDNCS","syntax"],
                 ); 
                 
%CDNSS = (
               'name'             => 'CDNSS',
               'desc'             => 'TianShan CDNSS Serveice',
               'Registry'         => \@CDNSS,
               'ServiceOID'	=> '2100',
               'configfile'	=> ["CDNSS","syntax"],
                 );                  
                 
%HttpCRG = (
               'name'             => 'HttpCRG',
               'desc'             => 'TianShan HttpCRG Serveice',
               'Registry'         => \@HttpCRG,
               'ServiceOID'	=> '2200',
               'configfile'	=> ["HttpCRG","CRM_A3Server","syntax","CRM_C2Locator","CRM_A3Message"],
                 );   
                 
%EdgeRM = (
               'name'             => 'EdgeRM',
               'desc'             => 'TianShan EdgeRM Serveice',
               'Registry'         => \@EdgeRM,
               'ServiceOID'	=> '2300',
               'configfile'	=> ["EdgeRM","syntax"],
                 );     
                 
%ContentLib = (
                     'name'             => 'ContentLib',
                     'desc'             => 'TianShan ContentLib Service',
                     'Registry'         => \@ContentLib,
                     'ServiceOID'	=> '2400',
                     'configfile'	=> ["ContentLib","syntax"],
                   );  
                   
%EventRE = (
                     'name'             => 'EventRE',
                     'desc'             => 'TianShan EventRuleEngine Service',
                     'Registry'         => \@EventRE,
                     'ServiceOID'	=> '2500',
                     'configfile'	=> ["EventRuleEngine","EventRules","syntax"],
                   );                                        
                                                                                                    
                                    
%DODApp = (
                     'name'             => 'DODApp',
                     'desc'             => 'TianShan DOD Application Service',
                     'Registry'         => \@DODApp,
                     'ServiceOID'	=> '1600',
                     'configfile'	=> ["DODApp","Localconfig","ConfDefine_DOD","dod_language"],
                   );
                 
%DODContentStore = (
                     'name'             => 'DODContentStore',
                     'desc'             => 'TianShan DODContentStore Service',
                     'Registry'         => \@DODContentStore,
                     'ServiceOID'	=> '1700',
                     'configfile'	=> ["DODContentStore","CODEvent","ConfDefine_DOD","dod_language"],
                   );
                   
%DataStream = (
                     'name'             => 'DataStream',
                     'desc'             => 'TianShan DataStream Service',
                     'Registry'         => \@DataStream,
                     'ServiceOID'	=> '1800',
                     'configfile'	=> ["DataStream","CODEvent","ConfDefine_DOD","dod_language"],
                   ); 
                   
%GBCS = (
               'name'             => 'GBCS',
               'desc'             => 'TianShan GBCS Service',
               'Registry'         => \@GBCS,
               'ServiceOID'	=> '3100',
               'configfile'	=> ["GBCS","syntax"],
                 );  
                 
%DummySvc = (
               'name'             => 'DummySvc',
               'desc'             => 'TianShan DummySvc Service',
               'Registry'         => \@DummySvc,
               'ServiceOID'	=> '3200',
               'configfile'	=> ["DummySvc","syntax"],
                 ); 
                 
%C2SS = (
               'name'             => 'C2SS',
               'desc'             => 'C2 Streaming Service',
               'Registry'         => \@C2SS,
               'ServiceOID'	=> '3300',
               'configfile'	=> ["C2SS","syntax"],
                 );                                                      
                   
                                                                                

@ChOdSvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","config File directory for ChOdSvc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","ImagePath","ImagePath","%EXEDIR%\\ChodSvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","logDir for ChodSvc_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ChOdSvc_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\ChOdSvc","Display Name","DisplayName","ZQ ChOdSvc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\ChOdSvc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ChOdSvc","The Event Message File of ChOdSvc","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ChOdSvc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
           
           
@StreamSmith = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","config File directory for StreamSmith","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","ImagePath","ImagePath","%EXEDIR%\\StreamSmith.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","LogDir for StreamSmith_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\StreamSmith","Display Name","DisplayName","ZQ StreamSmith","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\StreamSmith","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\StreamSmith","The Event Message File of StreamSmith","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\StreamSmith","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@DummySvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","config File directory for DummySvc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","ImagePath","ImagePath","%EXEDIR%\\DummySvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","LogDir for DummySvc_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DummySvc_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DummySvc","Display Name","DisplayName","ZQ DummySvc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DummySvc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DummySvc","The Event Message File of DummySvc","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DummySvc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@RtspProxy = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","config File directory for RtspProxy","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","ImagePath","ImagePath","%EXEDIR%\\RtspProxy.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","logDir for RtspProxy_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy","Display Name","DisplayName","ZQ RtspProxy","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy","The Event Message File of RtspProxy","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy","Event message support type","TypesSupported","7","REG_DWORD"],
	   );                       

@RtspProxy2 = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","config File directory for RtspProxy2","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2_shell","ImagePath","ImagePath","%EXEDIR%\\RtspProxy.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2_shell","logDir for RtspProxy_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy2_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy2","Display Name","DisplayName","ZQ RtspProxy2","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy2","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy2","The Event Message File of RtspProxy","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy2","Event message support type","TypesSupported","7","REG_DWORD"],
	   );

@Weiwoo = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","config File directory for Weiwoo","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           #["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","Weiwoo Service EndPoint","PathEndPoint","tcp -h <ipaddress> -p <port>","REG_SZ"],
           #["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","SiteAdmin Service EndPoint","SiteEndPoint","tcp -h <ipaddress> -p <port>","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","ImagePath","ImagePath","%EXEDIR%\\WeiwooService.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","logDir for Weiwoo_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Weiwoo_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\Weiwoo","Display Name","DisplayName","ZQ Weiwoo","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Weiwoo","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\Weiwoo","The Event Message File of Weiwoo","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\Weiwoo","Event message support type","TypesSupported","7","REG_DWORD"],
	   );          
	   
@MODSvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\MODAppSvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for $servicesname_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );     
	   
@GBMODSvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","config File directory for GBMODSvc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc_shell","ImagePath","ImagePath","%EXEDIR%\\GBMODAppSvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc_shell","logDir for RtspProxy_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\GBMODSvc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\GBMODSvc","Display Name","DisplayName","ZQ GBMODSvc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\GBMODSvc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\GBMODSvc","The Event Message File of RtspProxy","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\GBMODSvc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   
	   
@SiteAdminSvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","config File directory for SiteAdminSvc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","ImagePath","ImagePath","%EXEDIR%\\SiteAdminSvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","logDir for SiteAdminSvc_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\SiteAdminSvc_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\SiteAdminSvc","Display Name","DisplayName","ZQ SiteAdminSvc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\SiteAdminSvc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SiteAdminSvc","The Event Message File of SiteAdminSvc","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\SiteAdminSvc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
           
@Sentry = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","config File directory for Sentry","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","logDir","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","AdapterColletor port","AdapterCollectorPort","11999","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","multicast group port","GroupPort","65001","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","multicast group address","GroupAddress","239.200.200.1","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","multicast group bind address","GroupBind","0.0.0.0","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry","published http bind address","WebBindAddr","<ipaddress>","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","ImagePath","ImagePath","%EXEDIR%\\SentrySvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\ZQSNMPExtension","LoggingMask for ZQSNMPAgent","LoggingMask","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\ZQSNMPExtension","ZQSNMPAgent Path Name","Pathname","%EXEDIR%\\ZQSNMP.dll","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\ZQSNMPExtension","LogDir for ZQSNMPAgent","LogDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\NgodSnmp","NgodSnmp Path Name","Pathname","%EXEDIR%\\NGODSNMPExt.dll","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\NgodSnmp","NgodSnmp RootOid","RootOid",".1.3.6.1.4.1.22839.99","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","logDir for Sentry_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\Sentry","Display Name","DisplayName","ZQ Sentry","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Sentry","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\Sentry","The Event Message File of Sentry","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\Sentry","Event message support type","TypesSupported","7","REG_DWORD"],
	   );  
	   
@CPESvc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","config File directory for CPESvc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","ImagePath","ImagePath","%EXEDIR%\\CPESvc.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","logDir for CPESvc_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CPESvc_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\CPESvc","Display Name","DisplayName","ZQ CPESvc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\CPESvc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CPESvc","The Event Message File of CPESvc","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CPESvc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	
	   
@EventGateway = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","config File directory for EventGateway","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","ImagePath","ImagePath","%EXEDIR%\\EventGateway.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","logDir for EventGateway_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventGateway_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventGateway","Display Name","DisplayName","ZQ EventGateway","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventGateway","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventGateway","The Event Message File of EventGateway","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventGateway","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@EventChannel = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","config File directory for EventChannel","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","ImagePath","ImagePath","%EXEDIR%\\EventChannel.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","logDir for EventChannel_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventChannel_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventChannel","Display Name","DisplayName","ZQ EventChannel","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventChannel","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventChannel","The Event Message File of EventChannel","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventChannel","Event message support type","TypesSupported","7","REG_DWORD"],
	   );   
	   
@NSS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\NSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for $servicesname_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   

@C2SS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\C2SS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for $servicesname_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   
	   
@VLCVSS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","config File directory for VLCVSS","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","ImagePath","ImagePath","%EXEDIR%\\VLCVSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","logDir for VLCVSS_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\VLCVSS_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\VLCVSS","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\VLCVSS","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\VLCVSS","The Event Message File of VLCVSS","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\VLCVSS","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	
	    
@GBVSS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\GBVSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for GBVSS_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	
	   
@GBCS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\GBCS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for GBCS_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	    
	   
@TSPump = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\TSPump.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for TSPump_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ TSPump","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@DsmccCRG = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","config File directory for DsmccCRG","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","ImagePath","ImagePath","%EXEDIR%\\DsmccCRG.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","logDir for DsmccCRG_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DsmccCRG_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DsmccCRG","Display Name","DisplayName","ZQ DsmccCRG","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DsmccCRG","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DsmccCRG","The Event Message File of DsmccCRG","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DsmccCRG","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   	       
	   
@MediaClusterCS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","config File directory for $ins_servicesname","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$ins_servicesname","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","ImagePath","ImagePath","%EXEDIR%\\MediaClusterCS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","logDir for $servicesname_shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\$servicesname_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","Display Name","DisplayName","ZQ $ins_servicesname","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\$ins_servicesname","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","The Event Message File of $ins_servicesname","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\$ins_servicesname","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@BcastChannel = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","config File directory for BcastChannel","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","ImagePath","ImagePath","%EXEDIR%\\BcastChannel.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","logDir for BcastChannel_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\BcastChannel_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\BcastChannel","Display Name","DisplayName","ZQ BcastChannel","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\BcastChannel","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\BcastChannel","The Event Message File of BcastChannel","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\BcastChannel","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@CDNCS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","config File directory for CDNCS","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","ImagePath","ImagePath","%EXEDIR%\\CDNCS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","logDir for CDNCS_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNCS_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\CDNCS","Display Name","DisplayName","ZQ CDNCS","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\CDNCS","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CDNCS","The Event Message File of CDNCS","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CDNCS","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@CDNSS = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","config File directory for CDNSS","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","ImagePath","ImagePath","%EXEDIR%\\CDNSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","logDir for CDNSS_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CDNSS_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\CDNSS","Display Name","DisplayName","ZQ CDNSS","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\CDNSS","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CDNSS","The Event Message File of CDNSS","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CDNSS","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   
	   
@HttpCRG = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","config File directory for HttpCRG","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","ImagePath","ImagePath","%EXEDIR%\\HttpCRG.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","logDir for HttpCRG_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\HttpCRG_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\HttpCRG","Display Name","DisplayName","ZQ HttpCRG","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\HttpCRG","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\HttpCRG","The Event Message File of HttpCRG","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\HttpCRG","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@EdgeRM = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","config File directory for EdgeRM","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","ImagePath","ImagePath","%EXEDIR%\\EdgeRMService.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","logDir for EdgeRM_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EdgeRM_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\EdgeRM","Display Name","DisplayName","ZQ EdgeRM","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\EdgeRM","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EdgeRM","The Event Message File of EdgeRM","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EdgeRM","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	
	   
@ContentLib = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","config File directory for ContentLib","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","ImagePath","ImagePath","%EXEDIR%\\ContentLib.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","logDir for ContentLib_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ContentLib_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\ContentLib","Display Name","DisplayName","ZQ ContentLib","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\ContentLib","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ContentLib","The Event Message File of ContentLib","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ContentLib","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@EventRE = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","config File directory for EventRE","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","ImagePath","ImagePath","%EXEDIR%\\EventRuleEngine.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","logDir for EventRE_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\EventRE_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventRE","Display Name","DisplayName","ZQ EventRE","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\EventRE","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventRE","The Event Message File of EventRE","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\EventRE","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   		      	   	    
	   
@DODApp = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","config File directory for DODApp","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","ImagePath","ImagePath","%EXEDIR%\\DODApplication.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","logDir for DODApp_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODApp_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODApp","Display Name","DisplayName","ZQ DODApp","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODApp","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODApp","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","The Event Message File of DODApp","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   	                  	   	      
                    
@DODContentStore = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","config File directory for DODContentStore","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","ImagePath","ImagePath","%EXEDIR%\\DODCS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","logDir for DODContentStore_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DODContentStore_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODContentStore","Display Name","DisplayName","ZQ DODContentStore","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODContentStore","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODContentStore","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODContentStore","The Event Message File of DODContentStore","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODContentStore","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	   	                  	   	      
               
@DataStream = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","config File directory for DataStream","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","ImagePath","ImagePath","%EXEDIR%\\DataSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","logDir for DataStream_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\DataStream_shell","SnmpLoggingMask","SnmpLoggingMask","6","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DataStream","Display Name","DisplayName","ZQ DataStream","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DataStream","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DataStream","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DataStream","The Event Message File of DataStream","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DataStream","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	    	                  	   	      
                                                                                        
@All_Services = (
                 \%ChOdSvc,
                 \%StreamSmith,
                 \%DummySvc,
                 \%RtspProxy,
                 \%RtspProxy2,
                 \%Weiwoo,
                 \%MODSvc,
                 \%GBMODSvc,
                 \%SiteAdminSvc,
                 \%Sentry,
                 \%CPESvc,
                 \%EventGateway,
                 \%EventChannel,
                 \%NSS,
                 \%C2SS,
                 \%VLCVSS,
                 \%GBVSS,
                 \%GBCS,
                 \%TSPump,
                 \%DsmccCRG,
                 \%MediaClusterCS,
                 \%BcastChannel,
                 \%CDNCS,
                 \%CDNSS,
                 \%HttpCRG,
                 \%EdgeRM,
                 \%ContentLib,
                 \%EventRE,
                 \%DODApp,
                 \%DODContentStore,
                 \%DataStream,
                );
