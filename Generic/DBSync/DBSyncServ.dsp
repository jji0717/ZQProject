# Microsoft Developer Studio Project File - Name="DBSyncServ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DBSyncServ - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DBSyncServ.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DBSyncServ.mak" CFG="DBSyncServ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DBSyncServ - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DBSyncServ - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ScheduledTV/DBSync/DBSyncServ", RQEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DBSyncServ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DBSyncServ___Win32_Release"
# PROP BASE Intermediate_Dir "DBSyncServ___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(ITVSDKPATH)" /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /D "_NO_SYSMON" /FR /YX"stdafx.h" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 odbc32.lib odbccp32.lib /nologo /subsystem:console /verbose /incremental:yes /debug /machine:I386 /out:"Release/DBSync.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(ITVSDKPATH)\lib\debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DBSyncServ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DBSyncServ___Win32_Debug"
# PROP BASE Intermediate_Dir "DBSyncServ___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)" /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_NO_SYSMON" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SysMonLib_d.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"MSVCRT.lib" /out:"Debug/DBSync_d.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\lib\Debug"

!ENDIF 

# Begin Target

# Name "DBSyncServ - Win32 Release"
# Name "DBSyncServ - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\CallBackProcessThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\DBSAdiMan.cpp
# End Source File
# Begin Source File

SOURCE=.\DBSync.rc
# End Source File
# Begin Source File

SOURCE=.\DBSyncServ.cpp
# End Source File
# Begin Source File

SOURCE=.\DSCallBack.cpp
# End Source File
# Begin Source File

SOURCE=.\DSInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalDB.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualSyncAdi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\SchangeServiceAppMain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\CallBackProcessThread.h
# End Source File
# Begin Source File

SOURCE=.\ConnChecker.h
# End Source File
# Begin Source File

SOURCE=.\DBSAdi_def.h
# End Source File
# Begin Source File

SOURCE=.\DBSAdiMan.h
# End Source File
# Begin Source File

SOURCE=.\DBSyncConf.h
# End Source File
# Begin Source File

SOURCE=.\DBSyncServ.h
# End Source File
# Begin Source File

SOURCE=.\DSCallBack.h
# End Source File
# Begin Source File

SOURCE=.\DSInterface.h
# End Source File
# Begin Source File

SOURCE=.\LocalDB.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Locks.h
# End Source File
# Begin Source File

SOURCE=.\ManualSyncAdi.h
# End Source File
# Begin Source File

SOURCE=.\ManualSyncDef.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
