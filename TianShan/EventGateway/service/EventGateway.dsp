# Microsoft Developer Studio Project File - Name="EventGateway" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=EventGateway - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EventGateway.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EventGateway.mak" CFG="EventGateway - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EventGateway - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "EventGateway - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EventGateway - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I ".." /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(EXPATPATH)/include" /I "$(ZQProjsPath)/common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(RegExppKit)" /D "WITH_ICESTORM" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ice.lib iceutil.lib icestorm.lib /nologo /subsystem:console /pdb:"../../bin/EventGateway.pdb" /debug /machine:I386 /out:"../../bin/EventGateway.exe" /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "EventGateway - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(EXPATPATH)/include" /I "$(ZQProjsPath)/common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(RegExppKit)" /D "CHECK_WITH_GLOG" /D "WITH_ICESTORM" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iced.lib iceutild.lib icestormd.lib /nologo /subsystem:console /pdb:"../../bin/EventGateway_d.pdb" /debug /machine:I386 /out:"../../bin/EventGateway_d.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "EventGateway - Win32 Release"
# Name "EventGateway - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\common\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\EGConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\EventGateway.cpp
# End Source File
# Begin Source File

SOURCE=.\EventGwService.cpp
# End Source File
# Begin Source File

SOURCE=..\GenEventSinkI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\PluginHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\common\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\EGConfig.h
# End Source File
# Begin Source File

SOURCE=..\EventGateway.h
# End Source File
# Begin Source File

SOURCE=..\EventGwHelper.h
# End Source File
# Begin Source File

SOURCE=.\EventGwService.h
# End Source File
# Begin Source File

SOURCE=..\GenEventSinkI.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\PluginHelper.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\EventGw.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)\build"
# End Source File
# End Group
# End Target
# End Project
