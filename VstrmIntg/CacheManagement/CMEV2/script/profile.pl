use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD 
            %CMEV2Svc 
            @CMEV2Svc @All_Services);



($INSTALLROOT, $SVCACCOUNT, $SVCPASSWORD)
=  ("C:\\CMEV2", "LocalSystem", "");


%ProjTemplate = (
                   'name'             => 'service name', #it is a hard code, except service ClusterContentStore
                   'desc'             => 'service description', # should not been modified
                   'Registry'         => 'configuration default setting for service',
                   'configfile'	      => 'config file for service'		
                   );


%CMEV2Svc = (
                     'name'             => 'CMEV2Svc',
                     'desc'             => 'XORMedia CMEV2 Service',
                     'Registry'         => \@CMEV2Svc,
                     'ServiceOID'	=> '1111',
                     'configfile'	=> ["CME"],

                   );                                                             

@CMEV2Svc = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","config File directory for CMEV2Svc","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","LogLevel","LogLevel","0x00000006","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","ImagePath","ImagePath","%EXEDIR%\\CMEV2.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","logDir for CMEV2Svc_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\CMEV2Svc_shell","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\CMEV2Svc","Display Name","DisplayName","ZQ CMEV2Svc","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\CMEV2Svc","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CMEV2Svc","The Event Message File of CMEV2Svc","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\CMEV2Svc","Event message support type","TypesSupported","7","REG_DWORD"],
	   );                 	   	      
                                                                                        
@All_Services = (
                 \%CMEV2Svc,
                );
