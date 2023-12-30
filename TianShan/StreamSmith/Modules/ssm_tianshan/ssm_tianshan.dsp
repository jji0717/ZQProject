# Microsoft Developer Studio Project File - Name="ssm_tianshan" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ssm_tianshan - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssm_tianshan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssm_tianshan.mak" CFG="ssm_tianshan - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssm_tianshan - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ssm_tianshan - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/StreamNow/StreamSmith/Modules/ssm_tianshan", WJTAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssm_tianshan - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_TIANSHAN_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "$(ICE_ROOT)/include/stlport/" /I "$(ICE_ROOT)/include/" /I "../../../tianshan/ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(ZQPROJSPATH)\Common" /I "..\..\\" /I "..\..\service" /I "..\..\..\..\tianshan\ice" /I ".\\" /D _WIN32_WINNT=0x0400 /D "_ICE_INTERFACE_SUPPORT" /D "WITH_ICESTORM" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_TIANSHAN_EXPORTS" /D "NDEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_DLL" /D "_MT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib icestorm.lib Freeze.lib Ice.lib IceUtil.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../../bin/ssm_tianshan.pdb" /debug /machine:I386 /out:"../../../bin/ssm_tianshan.dll" /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ssm_tianshan - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_TIANSHAN_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ICE_ROOT)/include/stlport/" /I "$(ICE_ROOT)/include/" /I "../../../tianshan/ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(ZQPROJSPATH)\Common" /I "..\..\\" /I "..\..\service" /I "..\..\..\..\tianshan\ice" /I ".\\" /D "_ICE_INTERFACE_SUPPORT" /D "WITH_ICESTORM" /D _WIN32_WINNT=0x0400 /D "_WINDOWS" /D "_USRDLL" /D "SSM_TIANSHAN_EXPORTS" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "_MT" /D "_DLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 StreamSmith.lib icestormd.lib Freezed.lib Iced.lib IceUtild.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../../bin/ssm_tianshan.pdb" /debug /machine:I386 /out:"../../../bin/ssm_tianshan.dll" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib" /libpath:"..\..\service\debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ssm_tianshan - Win32 Release"
# Name "ssm_tianshan - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\PlaylistInternalUse.cpp
# End Source File
# Begin Source File

SOURCE=.\SSEventSinkI.cpp
# End Source File
# Begin Source File

SOURCE=.\ssm_tianshan.cpp
# End Source File
# Begin Source File

SOURCE=.\ssm_tianshan.def
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdminI.cpp
# End Source File
# Begin Source File

SOURCE=.\version.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\public.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SSEventSinkI.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\StreamSmithAdmin.h
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdminI.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
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
