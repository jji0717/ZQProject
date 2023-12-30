# Microsoft Developer Studio Project File - Name="SentryService" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SentryService - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SentryService.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SentryService.mak" CFG="SentryService - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SentryService - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SentryService - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SentryService - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SentryService___Win32_Release"
# PROP BASE Intermediate_Dir "SentryService___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SentryService___Win32_Release"
# PROP Intermediate_Dir "SentryService___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "../../Ice" /I "../../common" /I "$(ExpatPath)/include" /I "../ssllib" /I "$(OPENSSLPATH)/include" /I "$(ZQPROJSPATH)\tianshan\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(ZQProjsPath)/Common" /I "$(RegExppKit)" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_NO_NAMESPACE" /D _WIN32_WINNT=0x400 /D "_SPEED_VERSION" /D "_WIN32_SPECIAL_VERSION" /D "_STLP_NEW_PLATFORM_SDK" /D "XML_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 libeay32.lib ssleay32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../../bin/SentrySvc.pdb" /map /debug /machine:I386 /out:"../../bin/SentrySvc.exe" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(EXPATPATH)\lib" /libpath:"$(OPENSSLPATH)\lib" /libpath:"$(RegExppKit)\bin"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "SentryService - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SentryService___Win32_Debug"
# PROP BASE Intermediate_Dir "SentryService___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SentryService___Win32_Debug"
# PROP Intermediate_Dir "SentryService___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "../../Ice" /I "../../common" /I "$(ExpatPath)/include" /I "../ssllib" /I "$(OPENSSLPATH)/include" /I "$(ZQPROJSPATH)\tianshan\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(ZQProjsPath)/Common" /I "$(RegExppKit)" /D "CHECK_WITH_GLOG" /D "_NO_NAMESPACE" /D _WIN32_WINNT=0x400 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_SPEED_VERSION" /D "_WIN32_SPECIAL_VERSION" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "XML_STATIC" /YX /FD /GZ /Zm200 /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libeay32.lib ssleay32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../../bin/SentrySvc_d.pdb" /debug /machine:I386 /out:"../../bin/SentrySvc_d.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib" /libpath:"$(EXPATPATH)\lib" /libpath:"$(OPENSSLPATH)\lib" /libpath:"$(RegExppKit)\bin"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "SentryService - Win32 Release"
# Name "SentryService - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpYeoman.cpp
# End Source File
# Begin Source File

SOURCE=.\LayoutConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\LogPaserManagement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\Neighborhood.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessPerfMonitor.cpp
# End Source File
# Begin Source File

SOURCE=..\SentryCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\SentryConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\SentryEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\SentryImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\SentryPages.cpp
# End Source File
# Begin Source File

SOURCE=.\SentryService.cpp
# End Source File
# Begin Source File

SOURCE=..\ServiceFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\ssllib\ssllib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\debug.h
# End Source File
# Begin Source File

SOURCE=..\HttpDlg.h
# End Source File
# Begin Source File

SOURCE=..\HttpYeoman.h
# End Source File
# Begin Source File

SOURCE=.\LayoutConfig.h
# End Source File
# Begin Source File

SOURCE=..\LogPaserManagement.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\Neighborhood.h
# End Source File
# Begin Source File

SOURCE=.\ProcessPerfMonitor.h
# End Source File
# Begin Source File

SOURCE=..\SentryCommand.h
# End Source File
# Begin Source File

SOURCE=.\SentryConfig.h
# End Source File
# Begin Source File

SOURCE=..\SentryEnv.h
# End Source File
# Begin Source File

SOURCE=..\SentryImpl.h
# End Source File
# Begin Source File

SOURCE=..\SentryPages.h
# End Source File
# Begin Source File

SOURCE=.\SentryService.h
# End Source File
# Begin Source File

SOURCE=..\ServiceFrame.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Sentry.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)\build"
# End Source File
# End Group
# Begin Group "eventsink"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=..\eventsink\BaseMessageHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\BoostRegHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\ChannelMessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\HandlerGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\InfoSyntax.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\MagBase.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\MagMultiLog.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\MsgSenderPump.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\MultiLogThread.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\MutiSCLogThread.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\StringFuncImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\eventsink\StringFunction.cpp
# End Source File
# End Group
# Begin Group "eventSink headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\eventsink\BaseMessageHandler.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\BoostRegHandler.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\ChannelMessageQueue.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\EventSinkCfg.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\HandlerGroup.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\InfoSyntax.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MagBase.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MagMultiLog.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MsgSenderInterface.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MsgSenderPump.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MultiLogThread.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\MutiSCLogThread.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\StringFuncImpl.h
# End Source File
# Begin Source File

SOURCE=..\eventsink\StringFuncton.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
