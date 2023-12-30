use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD 
            %ServerLoad @ServerLoad);



($INSTALLROOT, $SVCACCOUNT, $SVCPASSWORD)
=  ("C:\\ServerLoad", ".\\SeaChange", "SeaChange");

@Filelist = (
		'ServerLoad.exe','ServerLoad.pdb','ZQShell.exe','ZQShell.pdb','ZQShellMsgs.dll','ZQCfgPkg.dll',
		'ZQSnmpManPkg.dll','ZQCommonstlp.dll','stlport_vc646.dll','VersionInfo_ServerLoad.txt',
	    );


%ServerLoad = (

               'name'             => 'ServerLoad',
               'desc'             => 'ZQ ServerLoad Serveice',
               'Registry'         => \@ServerLoad,
                 );                 
                 
@ServerLoad = (
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","config File directory for ServerLoad","configDir","%CONFIGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","logFolder","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","LogFileSize","LogFileSize","10000000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","LogWriteTimeOut","LogWriteTimeOut","2","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","LogBufferSize","LogBufferSize","0x00032000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","LogLevel","LogLevel","0x00000007","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","KeepAliveIntervals","KeepAliveIntervals","15000","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad","SnmpLoggingMask","SnmpLoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","ImagePath","ImagePath","%EXEDIR%\\ServerLoad.exe","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","logDir for ChonSvc_Shell","logDir","%LOGDIR%","REG_SZ"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","LoggingMask for ZQShell","LoggingMask","1","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","RestartInterval","RestartInterval","120","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","RestartTries","RestartTries","3","REG_DWORD"],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\ServerLoad_shell","SnmpLoggingMask","SnmpLoggingMask","1","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\ServerLoad","Display Name","DisplayName","ZQ ServerLoad","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\ServerLoad","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ServerLoad","The Event Message File of ServerLoad","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\ServerLoad","Event message support type","TypesSupported","7","REG_DWORD"],
	   );                  