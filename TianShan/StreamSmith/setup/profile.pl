use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD @StreamSmith @RtspProxy);
use Win32::TieRegistry;
use Win32::Registry;

($SVCACCOUNT, $SVCPASSWORD)
=  ("LocalSystem", "");

@StreamSmith = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","config File directory for StreamSmith","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","logFolder","logFolder","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogFile Name","LogFileName","%LOGDIR%\\StreamSmithSvc.log","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogFileSize","LogFileSize","50000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith","KeepAliveIntervals","KeepAliveIntervals","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","ImagePath","ImagePath","%EXEDIR%\\StreamSmith.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","LogFilePath for StreamSmith_shell","LogFilePath","%LOGDIR%\\StreamSmithService_shell.log","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\StreamSmith_shell","LoggingMask for StreamSmith_shell","LoggingMask","1","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\StreamSmith","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\StreamSmith","Display Name","DisplayName","ZQ StreamSmith","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\StreamSmith","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\StreamSmith","log message dll file","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\StreamSmith","log message support type","TypesSupported","7","REG_DWORD"],
	   );
	   
@RtspProxy = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","config File directory for RtspProxy","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","logFolder","logFolder","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogFile Name","LogFileName","%LOGDIR%\\RtspProxySvc.log","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogFileSize","LogFileSize","50000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy","KeepAliveIntervals","KeepAliveIntervals","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","ImagePath","ImagePath","%EXEDIR%\\RtspProxy.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","LogFilePath for RtspProxy_shell","LogFilePath","%LOGDIR%\\RtspProxyService_shell.log","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\RtspProxy_shell","LoggingMask for RtspProxy_shell","LoggingMask","1","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy","Display Name","DisplayName","ZQ RtspProxy","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\RtspProxy","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy","log message dll file","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\RtspProxy","log message support type","TypesSupported","7","REG_DWORD"],
	   );