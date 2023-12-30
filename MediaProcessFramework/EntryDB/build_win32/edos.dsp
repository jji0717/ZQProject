# Microsoft Developer Studio Project File - Name="edos" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=edos - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "edos.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "edos.mak" CFG="edos - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "edos - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "edos - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MediaProcessFramework/EntryDB/build_win32", HPGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "edos - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDOS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /I "." /I ".." /I "../.." /I "../../../Common" /I "../include/expat" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDOS_EXPORTS" /D "XML_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /implib:"../lib/edos.lib" /pdbtype:sept /libpath:"../lib/win32" /libpath:"../lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\edos.dll ..\..\Samples\MNApps\edos.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "edos - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDOS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I ".." /I "../.." /I "../../../Common" /I "../include/expat" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EDOS_EXPORTS" /D "XML_STATIC" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /implib:"../lib/edos.lib" /pdbtype:sept /libpath:"../lib/win32" /libpath:"../lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\edos.dll ..\..\Samples\MNApps\edos.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "edos - Win32 Release"
# Name "edos - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Dll"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\edos.def
# End Source File
# Begin Source File

SOURCE=.\edosdll.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Addins.cpp
# End Source File
# Begin Source File

SOURCE=..\DSO.cpp
# End Source File
# Begin Source File

SOURCE=..\EDB.cpp
# End Source File
# Begin Source File

SOURCE=..\EDBNil.cpp
# End Source File
# Begin Source File

SOURCE=..\EntryDB.cpp
# End Source File
# Begin Source File

SOURCE=..\ExpatDB.cpp
# End Source File
# Begin Source File

SOURCE=..\expatxx.cpp
# End Source File
# Begin Source File

SOURCE=..\Timestamp.cpp
# End Source File
# Begin Source File

SOURCE=..\urlstr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Addins.h
# End Source File
# Begin Source File

SOURCE=..\DSO.h
# End Source File
# Begin Source File

SOURCE=..\EDB.h
# End Source File
# Begin Source File

SOURCE=..\EDBImpl.h
# End Source File
# Begin Source File

SOURCE=..\EDBNil.h
# End Source File
# Begin Source File

SOURCE=..\EntryDB.h
# End Source File
# Begin Source File

SOURCE=..\ExpatDB.h
# End Source File
# Begin Source File

SOURCE=..\expatxx.h
# End Source File
# Begin Source File

SOURCE=..\IEDB.h
# End Source File
# Begin Source File

SOURCE=..\PollingTimer.h
# End Source File
# Begin Source File

SOURCE=..\Timestamp.h
# End Source File
# Begin Source File

SOURCE=..\urlstr.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
