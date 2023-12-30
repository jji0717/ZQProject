# Microsoft Developer Studio Project File - Name="SysTray" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SysTray - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SysTray.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SysTray.mak" CFG="SysTray - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SysTray - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SysTray - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SysTray - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "../../xmlRpc" /I "../../" /I "../../../common" /I "$(ZQPROJSPATH)/common/COMEXTRA" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "SysTray - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../xmlRpc" /I "../../" /I "../../../common" /I "$(ZQPROJSPATH)/common/COMEXTRA" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SysTray - Win32 Release"
# Name "SysTray - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\InfoQueryThread.cpp
# End Source File
# Begin Source File

SOURCE=.\mn_managedworkins.cpp
# End Source File
# Begin Source File

SOURCE=.\MN_Sessions.cpp
# End Source File
# Begin Source File

SOURCE=.\MN_WorkNodes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\MPFUtils.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\NodeConfigure.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeGeneralInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\PerWorkInsPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\PopSessionDetail.cpp
# End Source File
# Begin Source File

SOURCE=..\SortList.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SysTray.cpp
# End Source File
# Begin Source File

SOURCE=.\SysTray.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /i "../../"
# End Source File
# Begin Source File

SOURCE=.\SysTrayDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\TabSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\WN_TaskType.cpp
# End Source File
# Begin Source File

SOURCE=.\WN_WorkInstance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\COMEXTRA\zqsafemem.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\InfoDlg.h
# End Source File
# Begin Source File

SOURCE=..\InfoQueryThread.h
# End Source File
# Begin Source File

SOURCE=..\..\listinfo_def.h
# End Source File
# Begin Source File

SOURCE=.\mn_managedworkins.h
# End Source File
# Begin Source File

SOURCE=.\mn_sessions.h
# End Source File
# Begin Source File

SOURCE=.\MN_WorkNodes.h
# End Source File
# Begin Source File

SOURCE=..\..\MPFCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=.\NodeConfigure.h
# End Source File
# Begin Source File

SOURCE=.\NodeGeneralInfo.h
# End Source File
# Begin Source File

SOURCE=.\PerWorkInsPopup.h
# End Source File
# Begin Source File

SOURCE=.\PopSessionDetail.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\SortList.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SysTray.h
# End Source File
# Begin Source File

SOURCE=.\SysTrayDlg.h
# End Source File
# Begin Source File

SOURCE=..\TabSheet.h
# End Source File
# Begin Source File

SOURCE=.\WN_TaskType.h
# End Source File
# Begin Source File

SOURCE=.\WN_WorkInstance.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap6.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap7.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap8.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SysTray.ico
# End Source File
# Begin Source File

SOURCE=.\res\SysTray.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
