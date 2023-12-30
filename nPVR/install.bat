rem @echo off
setlocal

set TARGETDIR=c:\ITV
set EXEDIR=%TARGETDIR%\EXE
set LOGDIR=%TARGETDIR%\log
set ITVEXEROOT=c:\ITV\EXE

set SVCACCOUNT=.\SeaChange
set SVCPASSWD=SeaChange

md %TARGETDIR%
md %LOGDIR%
md %EXEDIR%

bin\instserv AssetGear stop
;@echo on

copy ".\AssetGear.exe" "%EXEDIR%" /Y
copy ".\AssetGear.pdb" "%EXEDIR%" /Y
copy ".\PMclient.dll" "%EXEDIR%" /Y
copy ".\PMclient.pdb" "%EXEDIR%" /Y

instserv AssetGear "%ITVEXEROOT%"\srvshell.exe local %SVCACCOUNT% %SVCPASSWD% auto own


regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "LogFile"  S  "%LOGDIR%\AssetGear.log"
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "CrashDumpPath"  S  "c:\itv\crashdump"
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "LogFileSize"  D 	0xA00000
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "LogLevel" 	 D	900
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "AssetGearUrl"  S  "http://0.0.0.0:1200/services/AssetGearService"
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "SoapThreadPool" 	 D	10
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "MetadataGatewayUrl"  S  "http://MetadataGateway:8000/services/PMService"

regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "ORBEndpoint"  S  ""
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "NameService"  S  "NameServer:30000"
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "ContentStoreNsEntry"  S  "Factories.Context/SeaChangeContentStore.Factory"
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "ItvImportPath"  S  ""
regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear"  "TaoThreadPool" 	 D	10


regsetup "SYSTEM\CurrentControlSet\Services\AssetGear"  "ProductName" S "ITV"

regsetup "SYSTEM\CurrentControlSet\Services\Eventlog\Application\AssetGear"  "EventMessageFile" E "c:\itv\exe\ItvMessages.dll"
regsetup "SYSTEM\CurrentControlSet\Services\Eventlog\Application\AssetGear"  "Typessupported"  D 7

regsetup "SYSTEM\CurrentControlSet\Services\Eventlog\Application\AssetGear_shell"  "EventMessageFile" E "c:\itv\exe\ShellMsgs.dll"
regsetup "SYSTEM\CurrentControlSet\Services\Eventlog\Application\AssetGear_shell"  "Typessupported"  D 7

regsetup "SOFTWARE\SeaChange\Management\CurrentVersion\Services\AssetGear"  "MgmtPortNumber" D 6861

regsetup "SOFTWARE\SeaChange\Management\CurrentVersion\Services\AssetGear_shell"  "MgmtPortNumber" D 6862

regsetup "SOFTWARE\SeaChange\ITV\CurrentVersion\Services\AssetGear_shell"  "ImagePath"  E "%EXEDIR%\AssetGear.exe"

cd ..

endlocal

