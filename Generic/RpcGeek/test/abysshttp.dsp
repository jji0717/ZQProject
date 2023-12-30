# Microsoft Developer Studio Project File - Name="abysshttp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=abysshttp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "abysshttp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "abysshttp.mak" CFG="abysshttp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "abysshttp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "abysshttp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "abysshttp"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "abysshttp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\abysshttp"
# PROP Intermediate_Dir "Release\abysshttp"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".." /I "../include" /I "../lib/util/include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "ABYSS_WIN32" /D "_THREAD" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\abysshttp.lib"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\abysshttp"
# PROP Intermediate_Dir "Debug\abysshttp"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I "../include" /I "../lib/util/include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "ABYSS_WIN32" /D "_THREAD" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\abysshttpD.lib"

!ENDIF 

# Begin Target

# Name "abysshttp - Win32 Release"
# Name "abysshttp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;cc"
# Begin Source File

SOURCE=..\lib\abyss\src\conf.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\conn.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\data.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\file.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\http.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\server.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\socket.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\thread.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\token.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lib\abyss\src\trace.c

!IF  "$(CFG)" == "abysshttp - Win32 Release"

!ELSEIF  "$(CFG)" == "abysshttp - Win32 Debug"

# ADD CPP /D "_THREAD"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
