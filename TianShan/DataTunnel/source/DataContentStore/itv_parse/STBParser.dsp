# Microsoft Developer Studio Project File - Name="STBParser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=STBParser - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "STBParser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "STBParser.mak" CFG="STBParser - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "STBParser - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "STBParser - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "STBParser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I ".\Parser\inc" /I ".\z_lib\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "STBParser - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".\Parser\inc" /I ".\z_lib\inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "STBParser - Win32 Release"
# Name "STBParser - Win32 Debug"
# Begin Group "z_lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\z_lib\src\adler32.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\compress.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\crc32.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\crc32.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\deflate.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\deflate.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\gzio.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\infback.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\inffast.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\inffast.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\inflate.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\inflate.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\main.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\minigzip.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\trees.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\trees.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\zconf.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\zconf.in.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\zlib.h
# End Source File
# Begin Source File

SOURCE=.\z_lib\src\zutil.c
# End Source File
# Begin Source File

SOURCE=.\z_lib\inc\zutil.h
# End Source File
# End Group
# Begin Group "Parser"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\Parser\inc\itv_errors.h
# End Source File
# Begin Source File

SOURCE=.\Parser\inc\itv_pal.h
# End Source File
# Begin Source File

SOURCE=.\Parser\inc\itv_types.h
# End Source File
# Begin Source File

SOURCE=.\Parser\inc\object_debug.h
# End Source File
# Begin Source File

SOURCE=.\Parser\src\parse.c
# End Source File
# Begin Source File

SOURCE=.\Parser\inc\parse.h
# End Source File
# End Group
# End Target
# End Project
