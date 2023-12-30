# Microsoft Developer Studio Project File - Name="appshell" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=appshell - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "appshell.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "appshell.mak" CFG="appshell - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "appshell - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "appshell - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "appshell - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "appshell - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ITV-NEW/Shell/appshell", INCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "appshell - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Release\zqappshell.lib"
# Begin Special Build Tool
IntDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Desc=Copy files
PostBuild_Cmds=echo        Y        |        xcopy        /f/r        $(IntDir)\*.lib        ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appshell - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "_DEBUG" /D "DEBUG_EXCEPTION" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Debug\zqappshell_d.lib"
# Begin Special Build Tool
IntDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Desc=Copy files
PostBuild_Cmds=echo        Y        |        xcopy        /f/r        $(IntDir)\*.lib        ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appshell - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "appshell___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "appshell___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Unicode_Debug"
# PROP Intermediate_Dir ".\Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "_DEBUG" /D "DEBUG_EXCEPTION" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "_DEBUG" /D "DEBUG_EXCEPTION" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Debug\appshell_d.lib"
# ADD LIB32 /nologo /out:".\Unicode_Debug\zqappshell_d.lib"
# Begin Special Build Tool
IntDir=.\Unicode_Debug
SOURCE="$(InputPath)"
PostBuild_Desc=Copy files
PostBuild_Cmds=echo        Y        |        xcopy        /f/r        $(IntDir)\*.lib        ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appshell - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "appshell___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "appshell___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Unicode_Release"
# PROP Intermediate_Dir ".\Unicode_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "..\Include" /I "..\ManPkg" /I "..\CfgPkg" /I "..\\" /I ".\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DEBUG_EXCEPTION" /D "UNICODE" /D "_UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Unicode_Release\zqappshell.lib"
# Begin Special Build Tool
IntDir=.\Unicode_Release
SOURCE="$(InputPath)"
PostBuild_Desc=Copy files
PostBuild_Cmds=echo        Y        |        xcopy        /f/r        $(IntDir)\*.lib        ..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "appshell - Win32 Release"
# Name "appshell - Win32 Debug"
# Name "appshell - Win32 Unicode Debug"
# Name "appshell - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\appshell.cpp
# End Source File
# Begin Source File

SOURCE=.\svcsubr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\appshell.h
# End Source File
# Begin Source File

SOURCE=.\shell.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
