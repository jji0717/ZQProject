# Microsoft Developer Studio Project File - Name="PlaylistAS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PlaylistAS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PlaylistAS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlaylistAS.mak" CFG="PlaylistAS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlaylistAS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PlaylistAS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/Telewest/PlaylistAS", QRFAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PlaylistAS - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /I "$(MSSOAP_PATH)" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /map /machine:I386 /libpath:"$(ITVSDKPATH)\lib\debug" /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(MSSOAP_PATH)"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "PlaylistAS - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\Common" /I "$(ITVSDKPATH)\include" /I "$(MSSOAP_PATH)" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(ITVSDKPATH)\lib\debug" /libpath:"$(ITVSDKPATH)\lib\release" /libpath:"$(MSSOAP_PATH)"

!ENDIF 

# Begin Target

# Name "PlaylistAS - Win32 Release"
# Name "PlaylistAS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PlaylistAppServ.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistSoapProxy.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\StreamData.cpp

!IF  "$(CFG)" == "PlaylistAS - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "PlaylistAS - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StreamOpCtrl.cpp

!IF  "$(CFG)" == "PlaylistAS - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "PlaylistAS - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StreamSession.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PlaylistAppServ.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistSoapProxy.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StreamData.h
# End Source File
# Begin Source File

SOURCE=.\StreamOpCtrl.h
# End Source File
# Begin Source File

SOURCE=.\StreamSession.h
# End Source File
# Begin Source File

SOURCE=.\stv_ssctrl.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PlaylistAS.rc
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\BaseSchangeServiceApplication.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\BaseSchangeServiceApplication.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\Exception.cpp"

!IF  "$(CFG)" == "PlaylistAS - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "PlaylistAS - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\Exception.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\IPreference.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\Log.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\Log.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\SchangeServiceAppMain.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScLog.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScLog.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScReporter.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ScReporter.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\XMLPreference.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\XMLPreference.h"
# End Source File
# Begin Source File

SOURCE="$(ZQProjsPath)\Common\ZQ_common_conf.h"
# End Source File
# End Group
# Begin Group "ExtHeaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dsmccuser.h
# End Source File
# Begin Source File

SOURCE=.\mod.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
