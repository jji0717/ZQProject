# Microsoft Developer Studio Project File - Name="EventCollector" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=EventCollector - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EventCollector.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EventCollector.mak" CFG="EventCollector - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EventCollector - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "EventCollector - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ISA/EventCollector", EYNAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EventCollector - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "$(RegExppKit)/" /I "$(ZQPROJSPATH)/Common" /I "$(ITVSDKPATH)/include" /I "$(ACE_ROOT)/TAO/include" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)/include" /i "$(ZQPROJSPATH)/ISA/ISA_15" /i "$(ZQProjsPath)/build" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /pdb:"../exe/EventCollector.pdb" /map:"../exe/EventCollector.map" /debug /machine:I386 /out:"../exe/EventCollector.exe" /pdbtype:sept /libpath:"$(RegExppKit)\libs\regex\build\vc6" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ACE_ROOT)/TAO/LIB" /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib\\"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "EventCollector - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /Zi /Od /I "$(RegExppKit)/" /I "$(ZQPROJSPATH)/Common" /I "$(ITVSDKPATH)/include" /I "$(ACE_ROOT)/TAO/include" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)/" /i "$(ZQProjsPath)/ISA/ISA_15" /i "$(ZQProjsPath)/build" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /incremental:no /map /debug /machine:I386 /out:"debug/EventCollector_d.exe" /pdbtype:sept /libpath:"$(RegExppKit)\libs\regex\build\vc6" /libpath:"$(ITVSDKPATH)/lib/debug" /libpath:"$(ACE_ROOT)/TAO/LIB" /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib\\"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "EventCollector - Win32 Release"
# Name "EventCollector - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AlarmService.cpp
# End Source File
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

SOURCE=.\BaseSchangeServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\BoostRegHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelMessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\CORBAEventReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\EventCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\FileLogCol.cpp
# End Source File
# Begin Source File

SOURCE=.\HandlerGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ICClassFac.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\InitInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\JmsMsgSender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\COMMON\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SchangeServiceAppMain.cpp
# End Source File
# Begin Source File

SOURCE=.\SCLogCol.cpp
# End Source File
# Begin Source File

SOURCE=.\ScReporter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\StringFuncImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\StringFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\TaoRoutine.cpp
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

SOURCE=.\AlarmService.h
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

SOURCE=.\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\BoostRegHandler.h
# End Source File
# Begin Source File

SOURCE=.\ChannelMessageQueue.h
# End Source File
# Begin Source File

SOURCE=.\CORBAEventReceiver.h
# End Source File
# Begin Source File

SOURCE=.\FileLogCol.h
# End Source File
# Begin Source File

SOURCE=.\HandlerGroup.h
# End Source File
# Begin Source File

SOURCE=.\InfoCollector.h
# End Source File
# Begin Source File

SOURCE=.\InitInfo.h
# End Source File
# Begin Source File

SOURCE=.\JmsMsgSender.h
# End Source File
# Begin Source File

SOURCE=.\KeyDefine.h
# End Source File
# Begin Source File

SOURCE=.\MiniDump.h
# End Source File
# Begin Source File

SOURCE=.\NativeThread.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SCLogCol.h
# End Source File
# Begin Source File

SOURCE=.\ScReporter.h
# End Source File
# Begin Source File

SOURCE=.\StringFuncImpl.h
# End Source File
# Begin Source File

SOURCE=.\StringFuncton.h
# End Source File
# Begin Source File

SOURCE=.\TextFileWriter.h
# End Source File
# Begin Source File

SOURCE=.\UnixTextLog.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\EventCollector.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
