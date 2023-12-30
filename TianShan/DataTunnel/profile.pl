use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD 
            %DODContentStore %DODApp %DataStream
            @DODContentStore @DODApp @DataStream @All_Services);



($INSTALLROOT, $SVCACCOUNT, $SVCPASSWORD)
=  ("C:\\DataOnDemand", "LocalSystem", "");


%ProjTemplate = (
                   'name'             => 'service name', # should not been modified
                   'desc'             => 'service description',
                   'Registry'         => 'configuration default setting for service',
                   );

%DODContentStore = (
               'name'             => 'DODContentStore',
               'desc'             => 'ZQ DODContentStore Service',
               'Registry'         => \@DODContentStore,
                 );


%DODApp = (
                     'name'             => 'DODApp',
                     'desc'             => 'ZQ DOD Application Serveice',
                     'Registry'         => \@DODApp,
                   );
                   
%DataStream = (
               'name'             => 'DataStream',
               'desc'             => 'ZQ DataStream Service',
               'Registry'         => \@DataStream,
                 );

@DODContentStore = (
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","config File directory for DODContentStore Service","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","log file directory for DODContentStore","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","LogLevel","LogLevel","0x00000007","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","ImagePath","ImagePath","%EXEDIR%\\DODCS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","logDir for DODContentStore_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODContentStore_shell","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODContentStore","Display Name","DisplayName","ZQ DODContentStore","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODContentStore","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODContentStore","The Event Message File of DODContentStore","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODContentStore","Event message support type","TypesSupported","7","REG_DWORD"],
	   );   

@DODApp = (
	   ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","config File directory for DODApp Service","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","log file directory for DODApp","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","LogLevel","LogLevel","0x00000007","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","ImagePath","ImagePath","%EXEDIR%\\DODApplication.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","logDir for DODApp_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DODApp_shell","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODApp","Display Name","DisplayName","ZQ DODApp","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DODApp","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","The Event Message File of DODApp","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DODApp","Event message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@DataStream = (
	   ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","config File directory for DataStream Service","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","log file directory for DataStream","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","LogFileSize","LogFileSize","40000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","LogFileCount","LogFileCount","5","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","LogLevel","LogLevel","0x00000007","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","ImagePath","ImagePath","%EXEDIR%\\DataSS.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","logDir for DataStream_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\DataOnDemand\\CurrentVersion\\Services\\DataStream_shell","SnmpLoggingMask","SnmpLoggingMask","0","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\DataStream","Display Name","DisplayName","ZQ DataStream","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\DataStream","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DataStream","The Event Message File of DataStream","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\DataStream","Event message support type","TypesSupported","7","REG_DWORD"],
	   );	                 
                                                          
@All_Services = (
                 \%DODContentStore,
                 \%DODApp,
                 \%DataStream,
                );