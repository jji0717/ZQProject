# Microsoft Developer Studio Project File - Name="RpcGeek_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=RpcGeek_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RpcGeek_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RpcGeek_static.mak" CFG="RpcGeek_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RpcGeek_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "RpcGeek_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/Generic/RpcGeek/RpcGeek", ANOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RpcGeek_static - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".." /I "../../../Common" /I "$(XMLRPCKit)" /I "$(XMLRPCKit)/include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "ABYSS_WIN32" /D "SKEL_ALONE" /D "RPCGEEK_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "RpcGeek_static - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I "../../../Common" /I "$(XMLRPCKit)" /I "$(XMLRPCKit)/include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "ABYSS_WIN32" /D "SKEL_ALONE" /D "RPCGEEK_STATIC" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "RpcGeek_static - Win32 Release"
# Name "RpcGeek_static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=..\RpcGeekUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=..\RpcGeekServer.cpp
# End Source File
# Begin Source File

SOURCE=..\RpcGeekClient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\urlstr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Variant.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThreadPool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\PollingTimer.h
# End Source File
# Begin Source File

SOURCE=..\RpcGeekCommon.h
# End Source File
# Begin Source File

SOURCE=..\RpcGeekUtils.h
# End Source File
# Begin Source File

SOURCE=..\RpcGeekServer.h
# End Source File
# Begin Source File

SOURCE=..\RpcGeekClient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\urlstr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Variant.h
# End Source File
# End Group
# End Target
# End Project
