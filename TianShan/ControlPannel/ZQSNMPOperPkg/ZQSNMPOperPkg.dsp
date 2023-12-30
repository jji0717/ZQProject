# Microsoft Developer Studio Project File - Name="ZQSNMPOperPkg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ZQSNMPOperPkg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZQSNMPOperPkg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZQSNMPOperPkg.mak" CFG="ZQSNMPOperPkg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZQSNMPOperPkg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZQSNMPOperPkg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ZQSNMPOperPkg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZQSNMPOPERPKG_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/TianShan/Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /D "_USRDLL" /D "ZQSNMPOPERPKG_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mgmtapi.lib snmpapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/ZQSnmpOperPkg_d.dll" /pdbtype:sept /libpath:"$(ZQProjsPath)/TianShan/Lib"
# Begin Special Build Tool
IntDir=.\Debug
SOURCE="$(InputPath)"
PreLink_Desc=copy files
PreLink_Cmds=echo                                Y                                |                                xcopy                                /f/r                                $(IntDir)\*.lib                                ..\lib\ 
PostBuild_Desc=copy files
PostBuild_Cmds=echo                                Y                                |                                xcopy                                /f/r                                $(IntDir)\*.lib                               ..\..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ZQSNMPOperPkg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ZQSNMPOperPkg___Win32_Release"
# PROP BASE Intermediate_Dir "ZQSNMPOperPkg___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/TianShan/Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /D "_USRDLL" /D "ZQSNMPMANPKG_EXPORTS" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/TianShan/Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /D "_USRDLL" /D "ZQSNMPOPERPKG_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 mgmtapi.lib snmpapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/ZQSnmpOperPkg_d.dll" /pdbtype:sept /libpath:"$(ZQProjsPath)/TianShan/Lib"
# ADD LINK32 mgmtapi.lib snmpapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../bin/ZQSnmpOperPkg.dll" /pdbtype:sept /libpath:"$(ZQProjsPath)/TianShan/Lib"
# Begin Special Build Tool
IntDir=.\Release
SOURCE="$(InputPath)"
PreLink_Desc=copy files
PreLink_Cmds=echo                                Y                                |                                xcopy                                /f/r                                $(IntDir)\*.lib                                ..\lib\ 
PostBuild_Desc=copy files
PostBuild_Cmds=echo                                Y                                |                                xcopy                                /f/r                                $(IntDir)\*.lib                               ..\..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ZQSNMPOperPkg - Win32 Debug"
# Name "ZQSNMPOperPkg - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQSNMPOperPkg.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQSNMPOperPkg.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# Begin Source File

SOURCE=.\ZQSNMPOperPkg.h
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
