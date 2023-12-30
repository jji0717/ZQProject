use vars qw($INSTALLROOT $SVCACCOUNT $SVCPASSWORD @Weiwoo);
use Win32::TieRegistry;
use Win32::Registry;

($SVCACCOUNT, $SVCPASSWORD)
=  ("LocalSystem", "");

@Weiwoo = (
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','config File directory for Weiwoo','configDir','%CONFIGDIR%','REG_SZ'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','logFolder','logFolder','%LOGDIR%','REG_SZ'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','LogFile Name','LogFileName','%LOGDIR%\\WeiwooSvc.log','REG_SZ'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','LogFileSize','LogFileSize','50000','REG_DWORD'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','LogWriteTimeOut','LogWriteTimeOut','2','REG_DWORD'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','LogBufferSize','LogBufferSize','0x00032000','REG_DWORD'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo','KeepAliveIntervals','KeepAliveIntervals','2','REG_DWORD'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo_shell','ImagePath','ImagePath','%EXEDIR%\\WeiwooService.exe','REG_SZ'],
           ['SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo_shell','LogFilePath for weiwoo_shell','LogFilePath','%LOGDIR%\\WeiwooService_shell.log','REG_SZ'],
           ["SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\WeiWoo_shell","LoggingMask for WeiWoo_shell","LoggingMask","1","REG_DWORD"],
           ["SYSTEM\\CurrentControlSet\\Services\\WeiWoo","Product Name","ProductName","TianShan","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\WeiWoo","Display Name","DisplayName","ZQ Weiwoo","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\WeiWoo","ImagePath of ZQShell","ImagePath","%EXEDIR%\\ZQShell.exe","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\WeiWoo","log message dll file","EventMessageFile","%EXEDIR%\\ZQShellMsgs.dll","REG_SZ"],
           ["SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\WeiWoo","log message support type","TypesSupported","7","REG_DWORD"],
	   );