# Microsoft Developer Studio Project File - Name="DBSyncServ_Eng" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DBSyncServ_Eng - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DBSyncServ_Eng.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DBSyncServ_Eng.mak" CFG="DBSyncServ_Eng - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DBSyncServ_Eng - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DBSyncServ_Eng - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ScheduledTV/DBSync/DBSyncServ", RQEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DBSyncServ_Eng - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DBSyncServ_Eng___Win32_Release"
# PROP BASE Intermediate_Dir "DBSyncServ_Eng___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Eng"
# PROP Intermediate_Dir "Release_Eng"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(ITVSDKPATH)" /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_ENG_VERSION" /FR /YX"stdafx.h" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Release_Eng/DBSync.exe" /libpath:"$(ITVSDKPATH)\lib\debug" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release_Eng\DBSync.exe" ".\installation\Data Files\MiniData"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DBSyncServ_Eng - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DBSyncServ_Eng___Win32_Debug"
# PROP BASE Intermediate_Dir "DBSyncServ_Eng___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Eng"
# PROP Intermediate_Dir "Debug_Eng"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)" /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_ENG_VERSION" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug_Eng/DBSync.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\lib\Debug" /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(ITVSDKPATH)\lib\debug"

!ENDIF 

# Begin Target

# Name "DBSyncServ_Eng - Win32 Release"
# Name "DBSyncServ_Eng - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\DBSync.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /d "_ENG_VERSION"
# End Source File
# Begin Source File

SOURCE=.\DBSyncServ.cpp
# End Source File
# Begin Source File

SOURCE=.\DSInterface_Eng.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalDB.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\SchangeServiceAppMain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TriggerWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\TriggerWorkerMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\zqlock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\zqthread.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\ConnChecker.h
# End Source File
# Begin Source File

SOURCE=.\DBSyncConf.h
# End Source File
# Begin Source File

SOURCE=.\DBSyncServ.h
# End Source File
# Begin Source File

SOURCE=.\DSInterface.h
# End Source File
# Begin Source File

SOURCE=.\LocalDB.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TriggerWorker.h
# End Source File
# Begin Source File

SOURCE=.\TriggerWorkerMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\ZqLock.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\ZqThread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project