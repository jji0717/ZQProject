# Microsoft Developer Studio Project File - Name="SentryCmd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SentryCmd - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SentryCmd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SentryCmd.mak" CFG="SentryCmd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SentryCmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SentryCmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/Sentry/service"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SentryCmd - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "EMBED_PATHSVC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/SentryCmd.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SentryCmd - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "../../common" /I "$(ExpatPath)/include" /I "../ssllib" /I "$(OPENSSLPATH)/include" /D "_NO_NAMESPACE" /D _WIN32_WINNT=0x400 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_SPEED_VERSION" /D "_WIN32_SPECIAL_VERSION" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "XML_STATIC" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libeay32.lib ssleay32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/SentryCmd_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib" /libpath:"$(OPENSSLPATH)/lib"
# SUBTRACT LINK32 /profile /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SentryCmd - Win32 Release"
# Name "SentryCmd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\httpd.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpYeoman.cpp
# End Source File
# Begin Source File

SOURCE=..\Neighborhood.cpp
# End Source File
# Begin Source File

SOURCE=.\SentryCmd.cpp
# End Source File
# Begin Source File

SOURCE=..\SentryCommand.cpp
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

SOURCE=..\ServiceFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\ssllib\ssllib.cpp
# End Source File
# Begin Source File

SOURCE=..\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\httpdInterface.h
# End Source File
# Begin Source File

SOURCE=..\HttpDlg.h
# End Source File
# Begin Source File

SOURCE=..\HttpYeoman.h
# End Source File
# Begin Source File

SOURCE=..\Neighborhood.h
# End Source File
# Begin Source File

SOURCE=..\SentryCommand.h
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

SOURCE=..\ServiceFrame.h
# End Source File
# Begin Source File

SOURCE=..\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\common\TianShanDefines.h
# End Source File
# Begin Source File

SOURCE=..\..\common\ZqNodeIce.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
