# Microsoft Developer Studio Project File - Name="StreamSmithAdmin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=StreamSmithAdmin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StreamSmithAdmin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StreamSmithAdmin.mak" CFG="StreamSmithAdmin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StreamSmithAdmin - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "StreamSmithAdmin - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/StreamSmith/StreamSmithAdmin"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StreamSmithAdmin - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "$(ICE_ROOT)\include\stlport" /I "$(ICE_ROOT)\include" /I "$(ZQPROJSPATH)\common\\" /I "$(ZQPROJSPATH)\tianshan\common" /I "$(ZQPROJSPATH)\tianshan\ice" /I "$(ZQPROJSPATH)\tianshan\streamsmith\\" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0400 /D "NDEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_DLL" /D "_MT" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib /nologo /subsystem:console /pdb:"../../bin/StreamSmithAdmin.pdb" /debug /machine:I386 /out:"../../bin/StreamSmithAdmin.exe" /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "StreamSmithAdmin - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ICE_ROOT)\include\stlport" /I "$(ICE_ROOT)\include" /I "$(ZQPROJSPATH)\common\\" /I "$(ZQPROJSPATH)\tianshan\common" /I "$(ZQPROJSPATH)\tianshan\ice" /I "$(ZQPROJSPATH)\tianshan\streamsmith\\" /D "_CONSOLE" /D _WIN32_WINNT=0x0400 /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "_MT" /D "_DLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Freezed.lib Iced.lib IceUtild.lib /nologo /subsystem:console /pdb:"../../bin/StreamSmithAdmin.pdb" /debug /machine:I386 /out:"../../bin/StreamSmithAdmin.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "StreamSmithAdmin - Win32 Release"
# Name "StreamSmithAdmin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\1StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdParser.cpp
# End Source File
# Begin Source File

SOURCE=.\InitInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\NewTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\SSAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\SSAdminWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskScheduler.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CmdParser.h
# End Source File
# Begin Source File

SOURCE=.\InitInfo.h
# End Source File
# Begin Source File

SOURCE=.\NewTest.h
# End Source File
# Begin Source File

SOURCE=.\ScriptEnv.h
# End Source File
# Begin Source File

SOURCE=.\ScriptRequest.h
# End Source File
# Begin Source File

SOURCE=.\SSAdmin.h
# End Source File
# Begin Source File

SOURCE=.\SSAdminWrapper.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithAdmin.h
# End Source File
# Begin Source File

SOURCE=.\TaskScheduler.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\Service\StreamSmithAdmin.ICE

!IF  "$(CFG)" == "StreamSmithAdmin - Win32 Release"

# Begin Custom Build
InputPath=..\Service\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe   -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\  ../service/streamsmithadmin.ice

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmithAdmin - Win32 Debug"

# Begin Custom Build
InputPath=..\Service\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe   -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\  ../service/streamsmithadmin.ice

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
