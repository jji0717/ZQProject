# Microsoft Developer Studio Project File - Name="DllPort" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DllPort - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DllPort.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DllPort.mak" CFG="DllPort - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DllPort - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DllPort - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MediaProcessFramework", DPGAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DllPort - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLPORT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I ".." /I "../XMLRPC" /I "../entrydb" /I "$(ZQPROJSPATH)/Common" /I "$(ZQPROJSPATH)/Common/COMEXTRA" /I "../requestposter" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLPORT_EXPORTS" /D "_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"srm.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy srm.dll ..\Samples\MNApps\srm.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPort - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DllPort___Win32_Debug"
# PROP BASE Intermediate_Dir "DllPort___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLPORT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".." /I "../XMLRPC" /I "../entrydb" /I "$(ZQPROJSPATH)/Common" /I "$(ZQPROJSPATH)/Common/COMEXTRA" /I "../requestposter" /D "_DEBUG" /D "_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLPORT_EXPORTS" /FR /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"srm_d.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy srm_d.dll ..\Samples\MNApps\srm_d.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DllPort - Win32 Release"
# Name "DllPort - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\book.cpp
# End Source File
# Begin Source File

SOURCE=..\listinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaNode.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaResource.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaSession.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaTask.cpp
# End Source File
# Begin Source File

SOURCE=..\MPFUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMApi.rc

!IF  "$(CFG)" == "DllPort - Win32 Release"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)" /i "$(ZQPROJSPATH)/build"

!ELSEIF  "$(CFG)" == "DllPort - Win32 Debug"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)" /i "$(ZQPROJSPATH)/build"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRMCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMDaemon.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMPoster.cpp
# End Source File
# Begin Source File

SOURCE=.\SRMSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\SystemInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\zqlock.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\book.h
# End Source File
# Begin Source File

SOURCE=..\listinfo.h
# End Source File
# Begin Source File

SOURCE=..\listinfo_def.h
# End Source File
# Begin Source File

SOURCE=.\MetaNode.h
# End Source File
# Begin Source File

SOURCE=.\MetaRecord.h
# End Source File
# Begin Source File

SOURCE=.\MetaResource.h
# End Source File
# Begin Source File

SOURCE=.\MetaSession.h
# End Source File
# Begin Source File

SOURCE=.\MetaTask.h
# End Source File
# Begin Source File

SOURCE=..\MPFCommon.h
# End Source File
# Begin Source File

SOURCE=..\srmapi.h
# End Source File
# Begin Source File

SOURCE=.\SRMCommon.h
# End Source File
# Begin Source File

SOURCE=.\SRMDaemon.h
# End Source File
# Begin Source File

SOURCE=.\SRMInfo.h
# End Source File
# Begin Source File

SOURCE=.\SRMPoster.h
# End Source File
# Begin Source File

SOURCE=.\SRMSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\SystemInfo_def.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
