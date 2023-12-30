# Microsoft Developer Studio Project File - Name="ChPub" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ChPub - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ChPub.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ChPub.mak" CFG="ChPub - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ChPub - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ChPub - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ChannelOnDemand/Tests/ChPub", ZQTAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ChPub - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I ".." /I "../.." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"../../../bin/ChPub.exe" /libpath:"$(EXPATPATH)/lib" /libpath:"$(ICE_ROOT)/lib"

!ELSEIF  "$(CFG)" == "ChPub - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "../.." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D "_DEBUG" /D "WITH_ICESTORM" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"../../bin/ChPub_d.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib"

!ENDIF 

# Begin Target

# Name "ChPub - Win32 Release"
# Name "ChPub - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BtnST.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ChPub.cpp
# End Source File
# Begin Source File

SOURCE=.\ChPub.rc
# End Source File
# Begin Source File

SOURCE=.\ChPubDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TestClient.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\ChannelDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChPub.h
# End Source File
# Begin Source File

SOURCE=.\ChPubDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConnectDlg.h
# End Source File
# Begin Source File

SOURCE=.\ItemDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TestClient.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\add_32.ico
# End Source File
# Begin Source File

SOURCE=.\res\cancel.ico
# End Source File
# Begin Source File

SOURCE=.\res\channel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\channel.ico
# End Source File
# Begin Source File

SOURCE=.\res\ChPub.ico
# End Source File
# Begin Source File

SOURCE=.\res\ChPub.rc2
# End Source File
# Begin Source File

SOURCE=.\res\connect.ico
# End Source File
# Begin Source File

SOURCE=".\res\control-play_16.ico"
# End Source File
# Begin Source File

SOURCE=".\res\control-stop_16.ico"
# End Source File
# Begin Source File

SOURCE=.\res\disconnect.ico
# End Source File
# Begin Source File

SOURCE=.\res\edit.ico
# End Source File
# Begin Source File

SOURCE=.\res\home.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Home.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\info.ico
# End Source File
# Begin Source File

SOURCE=.\res\item.bmp
# End Source File
# Begin Source File

SOURCE=.\res\item.ico
# End Source File
# Begin Source File

SOURCE=.\res\ok.ico
# End Source File
# Begin Source File

SOURCE=.\res\refresh.ico
# End Source File
# Begin Source File

SOURCE=.\res\remove.ico
# End Source File
# Begin Source File

SOURCE=.\res\root.ico
# End Source File
# Begin Source File

SOURCE=.\res\srvrunning.ico
# End Source File
# Begin Source File

SOURCE=.\res\srvstopped.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
