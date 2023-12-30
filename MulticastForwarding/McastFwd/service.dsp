# Microsoft Developer Studio Project File - Name="McastFwd Service" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=McastFwd Service - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "service.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "service.mak" CFG="McastFwd Service - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "McastFwd Service - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "McastFwd Service - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MulticastForwarding", YBBAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "McastFwd Service - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb".\Release\McastFwd.tlb" /win32
# ADD MTL /nologo /tlb".\Release\McastFwd.tlb" /win32
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "C:\IPv6Kit\inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "UNICODE" /D "_AFXDLL" /D "_UNICODE" /D "_MBCS" /Fp".\Release/McastFwd.pch" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "../inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "UNICODE" /D "_AFXDLL" /D "_UNICODE" /D "_MBCS" /Fp".\Release/McastFwd.pch" /YX /GF /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib manpkgu.lib appshell.lib cfgpkgu.lib reporter.lib sysmonlib.lib /nologo /subsystem:console /machine:IX86 /out:"c:\mcastfwd\bin\McastFwd.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib manpkgu.lib appshell.lib cfgpkgu.lib reporter.lib sysmonlib.lib /nologo /subsystem:console /machine:IX86 /out:"c:\mcastfwd\bin\McastFwd.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "McastFwd Service - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb".\Debug\McastFwd_d.tlb" /win32
# ADD MTL /nologo /tlb".\Debug\McastFwd_d.tlb" /win32
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "c:\IPv6Kit\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_MBCS" /GZ
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "../inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /D "__FUNCTION__" /GZ
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "E:\Project\IPv6Kit\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:IX86 /out:".\debug\McastFwd_d.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:IX86 /out:".\debug\McastFwd_d.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "McastFwd Service - Win32 Release"
# Name "McastFwd Service - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.cpp
DEP_CPP_BASES=\
	"..\..\Common\BaseSchangeServiceApplication.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\ScLog.h"\
	"..\..\Common\ScReporter.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\appshell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\cfgpkg.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\itv.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ItvError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\LogFile.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ManPkg.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\NtEventLog.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\Reporter.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\RptRegistration.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\shell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\sysmonlib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\Conversation.cpp
DEP_CPP_CONVE=\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\Guid.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\Conversation.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwd.h"\
	"..\McastFwdConf.h"\
	"..\McastSniffer.h"\
	"..\Tunnel.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
# End Source File
# Begin Source File

SOURCE=..\DenyList.cpp
DEP_CPP_DENYL=\
	"..\..\Common\Exception.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwdConf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\Exception.cpp
DEP_CPP_EXCEP=\
	"..\..\Common\Exception.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	

!IF  "$(CFG)" == "McastFwd Service - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "McastFwd Service - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Common\FtTcpComm.cpp
DEP_CPP_FTTCP=\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\getopt.cpp
DEP_CPP_GETOP=\
	"..\..\Common\getopt.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.cpp
DEP_CPP_INETA=\
	"..\..\Common\Exception.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	

!IF  "$(CFG)" == "McastFwd Service - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "McastFwd Service - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
DEP_CPP_LOG_C=\
	"..\..\Common\CombString.h"\
	"..\..\Common\Exception.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	
# End Source File
# Begin Source File

SOURCE=..\McastFwd.cpp
DEP_CPP_MCAST=\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\Guid.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\Conversation.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwd.h"\
	"..\McastFwdConf.h"\
	"..\McastSniffer.h"\
	"..\Tunnel.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MCastFwDServ.cpp
DEP_CPP_MCASTF=\
	"..\..\Common\BaseSchangeServiceApplication.h"\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\Guid.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\ScLog.h"\
	"..\..\Common\ScReporter.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\XMLPreference.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\Conversation.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwd.h"\
	"..\McastFwdConf.h"\
	"..\McastSniffer.h"\
	"..\Tunnel.h"\
	".\MCastFwDServ.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\appshell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\itv.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ItvError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\LogFile.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ManPkg.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\NtEventLog.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\Reporter.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\RptRegistration.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\shell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\sysmonlib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\McastListener.cpp
DEP_CPP_MCASTL=\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\getopt.h"\
	"..\..\Common\Guid.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\Conversation.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwdConf.h"\
	"..\McastListener.h"\
	"..\McastSniffer.h"\
	"..\Tunnel.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\SchangeServiceAppMain.cpp
DEP_CPP_SCHAN=\
	"..\..\Common\BaseSchangeServiceApplication.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\ScLog.h"\
	"..\..\Common\ScReporter.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\appshell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\itv.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ItvError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\LogFile.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ManPkg.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\NtEventLog.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\Reporter.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\RptRegistration.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\shell.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\sysmonlib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.cpp
DEP_CPP_SCLOG=\
	"..\..\Common\CombString.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\ScLog.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\CLog.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.cpp
DEP_CPP_SCREP=\
	"..\..\Common\CombString.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\ScReporter.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\LogFile.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\NtEventLog.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\Reporter.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ReporterTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\RptRegistration.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
NODEP_CPP_SCREP=\
	"..\..\Common\itvmessages.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.cpp
DEP_CPP_SOCKE=\
	"..\..\Common\Exception.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.cpp
DEP_CPP_THREA=\
	"..\..\Common\Log.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\scthreadpool.h"\
	
# End Source File
# Begin Source File

SOURCE=..\Tunnel.cpp
DEP_CPP_TUNNE=\
	"..\..\Common\Exception.h"\
	"..\..\Common\FtTcpComm.h"\
	"..\..\Common\Guid.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\Locks.h"\
	"..\..\Common\Log.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\Thread.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\Conversation.h"\
	"..\DenyList.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	"..\McastFwdConf.h"\
	"..\McastSniffer.h"\
	"..\Tunnel.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\DebugCriticalSection.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\FtWinsock.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\ITVServiceTypes.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpcomm.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\MtTcpCommError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaChange.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaCommon.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaError.h"\
	"D:\Data\SeaChangeKits\ITVSDK-v2.0.2.INTERNAL\include\SeaVersion.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\UDPSocket.cpp
DEP_CPP_UDPSO=\
	"..\..\Common\Exception.h"\
	"..\..\Common\InetAddr.h"\
	"..\..\Common\Socket.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\UDPSocket.h"\
	"..\..\Common\ZQ_common_conf.h"\
	"..\inc\tpipv6.h"\
	"..\inc\wspiapi.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.cpp
DEP_CPP_XMLPR=\
	"..\..\Common\CombString.h"\
	"..\..\Common\Exception.h"\
	"..\..\Common\IPreference.h"\
	"..\..\Common\StdAfx.h"\
	"..\..\Common\XMLPreference.h"\
	"..\..\Common\ZQ_common_conf.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\CombString.h
# End Source File
# Begin Source File

SOURCE=..\Conversation.h
# End Source File
# Begin Source File

SOURCE=..\DenyList.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\FtTcpComm.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Guid.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\IPreference.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Locks.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\McastFwd.h
# End Source File
# Begin Source File

SOURCE=..\McastFwdConf.h
# End Source File
# Begin Source File

SOURCE=.\MCastFwDServ.h
# End Source File
# Begin Source File

SOURCE=..\McastListener.h
# End Source File
# Begin Source File

SOURCE="..\..\..\ITV-V02.0Sdk\include\MtTcpComm.h"
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.h
# End Source File
# Begin Source File

SOURCE=..\Tunnel.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\UDPSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ZQ_common_conf.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\McastFwd.xml
# End Source File
# End Group
# End Target
# End Project
