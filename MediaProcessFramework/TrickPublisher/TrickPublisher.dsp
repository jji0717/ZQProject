# Microsoft Developer Studio Project File - Name="TrickPublisher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TrickPublisher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TrickPublisher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TrickPublisher.mak" CFG="TrickPublisher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TrickPublisher - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TrickPublisher - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TrickPublisher - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TRICKPUBLISHER_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /Gf /Gy /I "$(ITVSDKPATH)/include" /I "../../common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TRICKPUBLISHER_EXPORTS" /D "USER_VERSION" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "_WIN32" /D "_MT" /D "_DLL" /FR /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 mcpapi.lib strmbase.lib winmm.lib msvcrt.lib quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Release/TrickPub.dll" /libpath:"VSTRMSDK/lib" /libpath:"$(ITVSDKPATH)/Lib/Release" /libpath:"mcpsdk" /libpath:"../../common"
# SUBTRACT LINK32 /profile /incremental:yes

!ELSEIF  "$(CFG)" == "TrickPublisher - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TRICKPUBLISHER_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(ITVSDKPATH)/include" /D "WIN32" /D "_MBCS" /D "_WINDOWS" /D "_USRDLL" /D "TRICKPUBLISHER_EXPORTS" /D "USER_VERSION" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmbasd.lib winmm.lib quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Debug/TrickPub.dll" /pdbtype:sept /libpath:"VSTRMSDK/lib" /libpath:"$(ITVSDKPATH)/Lib/Debug"

!ENDIF 

# Begin Target

# Name "TrickPublisher - Win32 Release"
# Name "TrickPublisher - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Publish"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActiveSession.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveSession.h
# End Source File
# Begin Source File

SOURCE=.\McpCommon.h
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpEventsSink.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpEventsSink.h
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpObject.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpObject.h
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpPublisher.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpPublisher.h
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpSession.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpSession.h
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPSession\McpSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\TThread.cpp
# End Source File
# Begin Source File

SOURCE=.\TThread.h
# End Source File
# End Group
# Begin Group "Filter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\trickpub.def
# End Source File
# Begin Source File

SOURCE=.\trickpub.h
# End Source File
# Begin Source File

SOURCE=.\TrickPublisher.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickPubRender.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickPubRender.h
# End Source File
# End Group
# Begin Group "Trick"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TrickModule\LibBuffer.h
# End Source File
# Begin Source File

SOURCE=.\TrickModule\LibQueue.h
# End Source File
# Begin Source File

SOURCE=.\TrickModule\NtFileIo.h
# End Source File
# Begin Source File

SOURCE=.\TrickModule\Subfile.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickModule\Subfile.h
# End Source File
# Begin Source File

SOURCE=.\TrickModule\TrickImportUser.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickModule\TrickImportUser.h
# End Source File
# End Group
# Begin Group "Job"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TrickModule\BufferPool.h
# End Source File
# Begin Source File

SOURCE=.\BufPoolLock.cpp
# End Source File
# Begin Source File

SOURCE=.\BufPoolLock.h
# End Source File
# Begin Source File

SOURCE=.\FNString.cpp
# End Source File
# Begin Source File

SOURCE=.\FNString.h
# End Source File
# Begin Source File

SOURCE=.\TrickModule\LibBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickModule\LibQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\ShareBufferMan.cpp
# End Source File
# Begin Source File

SOURCE=.\ShareBufferMan.h
# End Source File
# Begin Source File

SOURCE=.\TrickJob.cpp
# End Source File
# Begin Source File

SOURCE=.\TrickJob.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=.\MCPBaseObect.cpp
# End Source File
# Begin Source File

SOURCE=.\MCPBaseObject.h
# End Source File
# Begin Source File

SOURCE=.\MCPEventCode.h
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
