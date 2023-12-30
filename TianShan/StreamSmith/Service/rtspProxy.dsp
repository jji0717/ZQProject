# Microsoft Developer Studio Project File - Name="rtspProxy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rtspProxy - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rtspProxy.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rtspProxy.mak" CFG="rtspProxy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rtspProxy - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "rtspProxy - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/StreamSmith/Service"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rtspProxy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "rtspProxy___Win32_Release"
# PROP BASE Intermediate_Dir "rtspProxy___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "rtspProxy___Win32_Release"
# PROP Intermediate_Dir "rtspProxy___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "$(ZQPROJSPATH)\Common" /I "$(ICE_ROOT)/include/stlport" /I "../" /I "$(OPENSSLPATH)\include" /I "$(ZQPROJSPATH)\Common\Rtsp\RtspParser" /I "$(ZQPROJSPATH)\Common\Rtsp\Utils" /I "$(ZQPROJSPATH)\Common\SSLLib" /I "$(ZQPROJSPATH)\Common\ServiceFrame" /I "." /I "$(ICE_ROOT)/include/" /I "$(ExpatPath)\include" /I "$(ZQPROJSPATH)\tianshan\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "./" /I "$(ZQPROJSPATH)\tianshan\streamsmith\lscp\lscplib" /I "$(RegExppKit)" /D "_SUPPORT_LSC_PROTOCOL_" /D "_SPEED_VERSION" /D "_RTSP_PROXY" /D _WIN32_WINNT=0x400 /D "_NO_SYSMON" /D "_CONSOLE" /D "_MT" /D "_MBCS" /D "MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "WIN32" /D "_WIN32_SPECIAL_VERSION" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /d "NDEBUG" /d "_RTSP_PROXY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib ws2_32.lib libeay32.lib ssleay32.lib ole32.lib /nologo /subsystem:console /pdb:"../../bin/rtspProxy.pdb" /debug /machine:I386 /out:"../../bin/rtspProxy.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\Common\Rtsp\RtspParser\Release" /libpath:"$(ZQPROJSPATH)\Common\ServiceFrame\Lib\release" /libpath:"$(OPENSSLPATH)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rtspProxy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "rtspProxy___Win32_Debug"
# PROP BASE Intermediate_Dir "rtspProxy___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "rtspProxy___Win32_Debug"
# PROP Intermediate_Dir "rtspProxy___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "$(ZQPROJSPATH)\Common" /I "$(ICE_ROOT)/include/stlport" /I "../" /I "$(ZQPROJSPATH)\Common\SSLLib" /I "$(OPENSSLPATH)\include" /I "$(ZQPROJSPATH)\Common\ServiceFrame" /I "." /I "$(ZQPROJSPATH)\Common\Rtsp\RtspParser" /I "$(ZQPROJSPATH)\Common\Rtsp\Utils" /I "$(ICE_ROOT)/include/" /I "$(ExpatPath)\include" /I "$(ZQPROJSPATH)\tianshan\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "./" /I "$(ZQPROJSPATH)\tianshan\streamsmith\lscp\lscplib" /I "$(RegExppKit)" /D "_SPEED_VERSION" /D "WIN32" /D _WIN32_WINNT=0x400 /D "_RTSP_PROXY" /D "_NO_SYSMON" /D "_CONSOLE" /D "_MT" /D "_DEBUG" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "_WIN32_SPECIAL_VERSION" /D "_SUPPORT_LSC_PROTOCOL_" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /d "_DEBUG" /d "_RTSP_PROXY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib /nologo /subsystem:console /incremental:no /pdb:"../../bin/rtspProxy.pdb" /debug /machine:I386 /out:"../../bin/rtspProxy.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\Common\Rtsp\RtspParser\Debug" /libpath:"$(ZQPROJSPATH)\Common\ServiceFrame\Lib\Debug" /libpath:"$(OPENSSLPATH)\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "rtspProxy - Win32 Release"
# Name "rtspProxy - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\DialogCreator.cpp
# End Source File
# Begin Source File

SOURCE=..\global.cpp
# End Source File
# Begin Source File

SOURCE=..\LscDialogImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\LscProtolmpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RtspProxy.rc
# End Source File
# Begin Source File

SOURCE=..\RtspSession.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspSessionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSmithService.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamSmithSite.cpp
# End Source File
# Begin Source File

SOURCE=..\URIParser.cpp
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

SOURCE=..\..\..\COMMON\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\DialogCreator.h
# End Source File
# Begin Source File

SOURCE=..\global.h
# End Source File
# Begin Source File

SOURCE=..\LscDialogImpl.h
# End Source File
# Begin Source File

SOURCE=..\LscProtolmpl.h
# End Source File
# Begin Source File

SOURCE=.\proxydefinition.h
# End Source File
# Begin Source File

SOURCE=..\ResourceManager.h
# End Source File
# Begin Source File

SOURCE=..\RtspDialog.h
# End Source File
# Begin Source File

SOURCE=.\rtspProxyConfig.h
# End Source File
# Begin Source File

SOURCE=..\RtspSession.h
# End Source File
# Begin Source File

SOURCE=..\RtspSessionMgr.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmith.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithModule.h
# End Source File
# Begin Source File

SOURCE=.\StreamSmithService.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithSite.h
# End Source File
# Begin Source File

SOURCE=..\URIParser.h
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
