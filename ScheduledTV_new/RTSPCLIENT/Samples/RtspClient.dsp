# Microsoft Developer Studio Project File - Name="RtspClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=RtspClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RtspClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RtspClient.mak" CFG="RtspClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RtspClient - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RtspClient - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ScheduledTV/RtspClient", MBCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RtspClient - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "RtspClient - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(ZQPROJSPATH)/Common" /I "$(ITVSDKPATH)/include" /I "../" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/RtspClient_d.exe" /pdbtype:sept /libpath:"$(itvsdkpath)/lib/debug"

!ENDIF 

# Begin Target

# Name "RtspClient - Win32 Release"
# Name "RtspClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Exception.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Log.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\NativeThread.cpp"
# End Source File
# Begin Source File

SOURCE=..\RtspClient.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspDaemon.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspMsgHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspRequest.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspResponse.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspSocket.cpp
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScLog.cpp"
# End Source File
# Begin Source File

SOURCE=.\simpleRtsp.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Exception.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Locks.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Log.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\NativeThread.h"
# End Source File
# Begin Source File

SOURCE=..\RtspClient.h
# End Source File
# Begin Source File

SOURCE=..\RtspConnectionManager.h
# End Source File
# Begin Source File

SOURCE=..\RtspDaemon.h
# End Source File
# Begin Source File

SOURCE=..\RtspHeaders.h
# End Source File
# Begin Source File

SOURCE=..\RtspMessage.h
# End Source File
# Begin Source File

SOURCE=..\RtspMsgHeader.h
# End Source File
# Begin Source File

SOURCE=..\RtspRequest.h
# End Source File
# Begin Source File

SOURCE=..\RtspResponse.h
# End Source File
# Begin Source File

SOURCE=..\RtspSocket.h
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScLog.h"
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ZQ_common_conf.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
