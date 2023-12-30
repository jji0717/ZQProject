# Microsoft Developer Studio Project File - Name="PT_FtpServer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PT_FtpServer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PT_FtpServer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PT_FtpServer.mak" CFG="PT_FtpServer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PT_FtpServer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PT_FtpServer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PT_FtpServer - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PT_FTPSERVER_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "." /I "$(ZQProjsPath)/common" /I "$(ZQProjsPath)/tianshan/include" /I "$(ZQProjsPath)\TianShan\Shell\ZQCfgPkg" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)/include" /I "$(VSTRMKITPATH)/" /I "$(VSTRMKITPATH)/MCPSDK" /I "$(VSTRMKITPATH)/SDK/inc" /I "$(ExpatPath)/include" /I "$(ZQProjsPath)\TianShan\ContentStore\MPEConsole" /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PT_FTPSERVER_EXPORTS" /D "NEWLOGFMT" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../../bin/PT_FtpServer.pdb" /debug /machine:I386 /out:"../../../bin/PT_FtpServer.dll" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ICE_ROOT)/lib" /libpath:"../exe" /libpath:"$(VSTRMKITPATH)/MCPSDK" /libpath:"$(VSTRMKITPATH)/SDK/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PT_FtpServer - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PT_FTPSERVER_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "$(ZQProjsPath)/common" /I "$(ZQProjsPath)/tianshan/include" /I "$(ZQProjsPath)\TianShan\Shell\ZQCfgPkg" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)/include" /I "$(VSTRMKITPATH)/" /I "$(VSTRMKITPATH)/MCPSDK" /I "$(VSTRMKITPATH)/SDK/inc" /I "$(ExpatPath)/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PT_FTPSERVER_EXPORTS" /D "USER_VERSION" /D "_STLP_NEW_PLATFORM_SDK" /D "_AFXDLL" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../../bin/PT_FtpServer.pdb" /debug /machine:I386 /out:"../../../bin/PT_FtpServer_d.dll" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ICE_ROOT)/lib" /libpath:"../exe" /libpath:"$(VSTRMKITPATH)/MCPSDK" /libpath:"$(VSTRMKITPATH)/SDK/lib" /libpath:"$(ITVSDKPATH)/lib/debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PT_FtpServer - Win32 Release"
# Name "PT_FtpServer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CECfg.h
# End Source File
# Begin Source File

SOURCE=.\CmdLine.h
# End Source File
# Begin Source File

SOURCE=.\Ftp_Svr.h
# End Source File
# Begin Source File

SOURCE=.\FtpConnection.h
# End Source File
# Begin Source File

SOURCE=.\FtpSite.h
# End Source File
# Begin Source File

SOURCE=.\FtpSock.h
# End Source File
# Begin Source File

SOURCE=.\FtpsXfer.h
# End Source File
# Begin Source File

SOURCE=.\FtpsXferExtension.h
# End Source File
# Begin Source File

SOURCE=.\PT_FtpServer.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TermService.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
