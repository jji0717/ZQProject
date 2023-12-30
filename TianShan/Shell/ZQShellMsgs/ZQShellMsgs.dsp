# Microsoft Developer Studio Project File - Name="ZQShellMsgs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ZQShellMsgs - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZQShellMsgs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZQShellMsgs.mak" CFG="ZQShellMsgs - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZQShellMsgs - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZQShellMsgs - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZQShellMsgs - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZQShellMsgs - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/Shell/ZQShellMsgs"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ZQShellMsgs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /Zi /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"ShellMsgs.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 MSVCRT.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"../../bin/ZQShellMsgs.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
IntDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                     $(IntDir)\*.lib                                                  ..\..\lib\   	copy                    ShellMsgs.h                                                   ..\..\include\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"ShellMsgs.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 MSVCRTD.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"../../bin/ZQShellMsgs_d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
IntDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Desc=copy files
PostBuild_Cmds=copy                      $(IntDir)\*.lib                                                   ..\..\lib\            	copy                    .\ShellMsgs.h                                                   ..\..\include\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ZQShellMsgs___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "ZQShellMsgs___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Unicode_Debug"
# PROP Intermediate_Dir ".\Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MBCS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /fo"ShellMsgs.res" /d "_DEBUG"
# ADD RSC /l 0x409 /fo"ShellMsgs.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 MSVCRTD.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"../../bin/ZQShellMsgs_d.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 MSVCRTD.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"../../bin/ZQShellMsgsU_d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
IntDir=.\Unicode_Debug
SOURCE="$(InputPath)"
PostBuild_Desc=copy files
PostBuild_Cmds=copy                      $(IntDir)\*.lib                                                   ..\..\lib\            	copy                    .\ShellMsgs.h                                                   ..\..\include\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ZQShellMsgs___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "ZQShellMsgs___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Unicode_Release"
# PROP Intermediate_Dir ".\Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /Zi /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MBCS" /D "_MBCS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /Zi /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /fo"ShellMsgs2.res" /d "NDEBUG"
# ADD RSC /l 0x409 /fo"ShellMsgs2.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"../../bin/ZQShellMsgs.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 MSVCRT.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"../../bin/ZQShellMsgsU.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
IntDir=.\Unicode_Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                      $(IntDir)\*.lib                                                   ..\..\lib\            	copy                    .\ShellMsgs.h                                                   ..\..\include\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ZQShellMsgs - Win32 Release"
# Name "ZQShellMsgs - Win32 Debug"
# Name "ZQShellMsgs - Win32 Unicode Debug"
# Name "ZQShellMsgs - Win32 Unicode Release"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\ShellMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellMsgs.def
# End Source File
# Begin Source File

SOURCE=.\ShellMsgs.mc

!IF  "$(CFG)" == "ZQShellMsgs - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Message File Build
InputPath=.\ShellMsgs.mc

"ShellMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc ShellMsgs.mc

# End Custom Build

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Message File Build
InputPath=.\ShellMsgs.mc

"ShellMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc ShellMsgs.mc

# End Custom Build

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Message File Build
InputPath=.\ShellMsgs.mc

"ShellMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc ShellMsgs.mc

# End Custom Build

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Release"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Message File Build
InputPath=.\ShellMsgs.mc

"ShellMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc ShellMsgs.mc

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ShellMsgs.rc

!IF  "$(CFG)" == "ZQShellMsgs - Win32 Release"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ZQShellMsgs - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Target
# End Project
