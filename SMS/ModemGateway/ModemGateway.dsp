# Microsoft Developer Studio Project File - Name="SMSVODService" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SMSVODService - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ModemGateway.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ModemGateway.mak" CFG="SMSVODService - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SMSVODService - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SMSVODService - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SMSVODService - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)\include" /I "..\SMSCommon\\" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)\build" /i "$(ITVSDKPATH)\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"$(ITVSDKPATH)\lib\release"

!ELSEIF  "$(CFG)" == "SMSVODService - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)\include" /I "..\SMSCommon\\" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)\build" /i "$(ITVSDKPATH)\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/ModemGateway_d.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\lib\debug"

!ENDIF 

# Begin Target

# Name "SMSVODService - Win32 Release"
# Name "SMSVODService - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\BaseSchangeServiceApplication.cpp"
# End Source File
# Begin Source File

SOURCE=.\ComCommunication.cpp
# End Source File
# Begin Source File

SOURCE=.\Communication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\ModemGateway.cpp
# End Source File
# Begin Source File

SOURCE=.\ModemGateway.rc
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\NativeThread.cpp"
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\NPVRSocket.cpp
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\SchangeServiceAppMain.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScReporter.cpp"
# End Source File
# Begin Source File

SOURCE=..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\SMSComPortReadThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SMSComPortWriteThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SMSContentProcessReq.cpp
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\SMSDB.cpp
# End Source File
# Begin Source File

SOURCE=.\SMSProcessRawMsgThread.cpp
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\SMSXmlProc.cpp
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\TicpProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\ComCommunication.h
# End Source File
# Begin Source File

SOURCE=.\Communication.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=.\Locks.h
# End Source File
# Begin Source File

SOURCE=.\ModemGateway.h
# End Source File
# Begin Source File

SOURCE=..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThreadPool.h
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\NPVRSocket.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\Common\ScReporter.h
# End Source File
# Begin Source File

SOURCE=.\SMSComPortReadThread.h
# End Source File
# Begin Source File

SOURCE=.\SMSComPortWriteThread.h
# End Source File
# Begin Source File

SOURCE=.\SMSContentProcessReq.h
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\SMSDB.h
# End Source File
# Begin Source File

SOURCE=.\SMSProcessRawMsgThread.h
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\SMSXmlProc.h
# End Source File
# Begin Source File

SOURCE=..\Common\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\SMSCommon\TicpProc.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.h
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
