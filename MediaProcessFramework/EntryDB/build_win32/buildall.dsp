# Microsoft Developer Studio Project File - Name="buildall" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=buildall - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "buildall.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "buildall.mak" CFG="buildall - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "buildall - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "buildall - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MediaProcessFramework/EntryDB/build_win32", HPGAAAAA"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "buildall - Win32 Release"

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
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=distribute.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "buildall - Win32 Debug"

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
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=distribute.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "buildall - Win32 Release"
# Name "buildall - Win32 Debug"
# End Target
# End Project
