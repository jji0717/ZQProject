# Microsoft Developer Studio Project File - Name="InfoCol" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=InfoCol - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "InfoCol.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "InfoCol.mak" CFG="InfoCol - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "InfoCol - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "InfoCol - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/Generic/InfoCol", ZJOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "InfoCol - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "%RegExppKit%" /I "../../Common" /I "$(RegExppKit)" /I "$(ITVSDKPATH)/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"$(RegExppKit)\libs\regex\build\vc6" /libpath:"$(ITVSDKPATH)/lib/release"

!ELSEIF  "$(CFG)" == "InfoCol - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(RegExppKit)" /I "../../Common" /I "$(ITVSDKPATH)/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(RegExppKit)\libs\regex\build\vc6" /libpath:"$(ITVSDKPATH)/lib/debug"

!ENDIF 

# Begin Target

# Name "InfoCol - Win32 Release"
# Name "InfoCol - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BaseInfoCol.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseMessageHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseMessageReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\BoostRegHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelMessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=.\HandlerGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ICClassFac.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoCol.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\InitInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SCLogCol.cpp
# End Source File
# Begin Source File

SOURCE=.\StringFuncImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\StringFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\TextFileWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\UnixTextLog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BaseCTail.h
# End Source File
# Begin Source File

SOURCE=.\BaseInfoCol.h
# End Source File
# Begin Source File

SOURCE=.\BaseMessageHandler.h
# End Source File
# Begin Source File

SOURCE=.\BaseMessageReceiver.h
# End Source File
# Begin Source File

SOURCE=.\BoostRegHandler.h
# End Source File
# Begin Source File

SOURCE=.\ChannelMessageQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=.\HandlerGroup.h
# End Source File
# Begin Source File

SOURCE=.\InfoCol.h
# End Source File
# Begin Source File

SOURCE=.\InfoCollector.h
# End Source File
# Begin Source File

SOURCE=.\ini.h
# End Source File
# Begin Source File

SOURCE=.\InitInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.h
# End Source File
# Begin Source File

SOURCE=.\SCLogCol.h
# End Source File
# Begin Source File

SOURCE=.\TextFileWriter.h
# End Source File
# Begin Source File

SOURCE=.\UnixTextLog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\InfoCol.ini
# End Source File
# End Target
# End Project
