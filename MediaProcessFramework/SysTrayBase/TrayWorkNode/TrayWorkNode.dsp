# Microsoft Developer Studio Project File - Name="TrayWorkNode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TrayWorkNode - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TrayWorkNode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TrayWorkNode.mak" CFG="TrayWorkNode - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TrayWorkNode - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TrayWorkNode - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MediaProcessFramework/SysTrayBase", HJJAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TrayWorkNode - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 xmlrpc.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "TrayWorkNode - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../XmlRpc" /I "../../../common" /I "../../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
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

# Name "TrayWorkNode - Win32 Release"
# Name "TrayWorkNode - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\InfoQueryThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\MPFUtils.cpp

!IF  "$(CFG)" == "TrayWorkNode - Win32 Release"

!ELSEIF  "$(CFG)" == "TrayWorkNode - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.cpp

!IF  "$(CFG)" == "TrayWorkNode - Win32 Release"

!ELSEIF  "$(CFG)" == "TrayWorkNode - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PerWorkInsPopup.cpp
# End Source File
# Begin Source File

SOURCE=..\SortList.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\TabSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\TrayWorkNode.cpp
# End Source File
# Begin Source File

SOURCE=.\TrayWorkNode.rc
# End Source File
# Begin Source File

SOURCE=.\TrayWorkNodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WN_GeneralInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\WN_TaskType.cpp
# End Source File
# Begin Source File

SOURCE=.\WN_WorkInstance.cpp
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

SOURCE=..\..\MPFCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=.\PerWorkInsPopup.h
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

SOURCE=..\TabSheet.h
# End Source File
# Begin Source File

SOURCE=.\TrayWorkNode.h
# End Source File
# Begin Source File

SOURCE=.\TrayWorkNodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\WN_GeneralInfo.h
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

SOURCE=.\res\TrayWorkNode.ico
# End Source File
# Begin Source File

SOURCE=.\res\TrayWorkNode.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
