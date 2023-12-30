# Microsoft Developer Studio Project File - Name="SNMPService" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SNMPService - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SNMPService.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SNMPService.mak" CFG="SNMPService - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SNMPService - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SNMPService - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SNMPService - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "$(RegExppKit)" /I "$(ZQProjsPATH)\Common" /I "$(ITVSDKPATH)\include" /I "$(SNMP_PLUS_ROOT)\include" /D "WIN32" /D "_NDEBUG" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_NO_SYSMON" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\..\build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../SNMPSvcs.pdb" /debug /machine:I386 /out:"../SNMPSvcs.exe" /pdbtype:sept /libpath:"$(SNMP_PLUS_ROOT)\lib" /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(ITVSDKPATH)\lib\debug" /libpath:"$(RegExppKit)\libs\regex\build\vc6"

!ELSEIF  "$(CFG)" == "SNMPService - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPATH)\Common" /I "$(ITVSDKPATH)\include" /I "$(RegExppKit)" /I "$(SNMP_PLUS_ROOT)\include" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_NO_SYSMON" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../SNMPSvcs.exe" /pdbtype:sept /libpath:"$(SNMP_PLUS_ROOT)\lib" /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(ITVSDKPATH)\lib\debug" /libpath:"$(RegExppKit)\libs\regex\build\vc6"

!ENDIF 

# Begin Target

# Name "SNMPService - Win32 Release"
# Name "SNMPService - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\BaseSchangeServiceApplication.cpp"
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\formatstr.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\ini.cpp
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\Log.cpp"
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LogScanPos.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LogTrigger.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LtCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadLine.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\registryex.cpp
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\SchangeServiceAppMain.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScLog.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScReporter.cpp"
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\SnmpService.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\SnmpWrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\TailTrigger.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\BaseSchangeServiceApplication.h"
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\formatstr.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\ini.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LogScanPos.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LogTrigger.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\LtCommon.h
# End Source File
# Begin Source File

SOURCE=.\ReadLine.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\registryex.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\safemalloc.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\SnmpWrapper.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\LogTrapper\TailTrigger.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\SnmpSvcs.rc
# End Source File
# End Group
# Begin Source File

SOURCE=..\LogTrapper\ReadMe.txt
# End Source File
# End Target
# End Project
