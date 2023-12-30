# Microsoft Developer Studio Project File - Name="NSSRtspSession" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=NSSRtspSession - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NSSRtspSession.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NSSRtspSession.mak" CFG="NSSRtspSession - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NSSRtspSession - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NSSRtspSession - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NSSRtspSession - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "NSSRtspSession - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\Common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX
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

# Name "NSSRtspSession - Win32 Release"
# Name "NSSRtspSession - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ngod_chop_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_daemon_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_parse_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_recv_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_action.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_ss.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_send_threadreq.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ngod_chop_thread.h
# End Source File
# Begin Source File

SOURCE=.\ngod_common_structure.h
# End Source File
# Begin Source File

SOURCE=.\ngod_daemon_thread.h
# End Source File
# Begin Source File

SOURCE=.\ngod_parse_thread.h
# End Source File
# Begin Source File

SOURCE=.\ngod_recv_thread.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_action.h
# End Source File
# Begin Source File

SOURCE=.\ngod_send_threadreq.h
# End Source File
# End Group
# Begin Group "RTSPParser"

# PROP Default_Filter ""
# Begin Group "RTSPMessage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessage.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageLine.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageLine.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageParser.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageParser.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageSession.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPMessage\RTSPMessageSession.h
# End Source File
# End Group
# Begin Group "RTSPHeader"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPExtensionHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPInBandMarkerHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPNoticeHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPPolicyHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPR2Header.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPR6Header.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPReasonHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPRequierHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPS4Header.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPSessionGroupHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPStartPointHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPStopPointHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPStreamControlProtoHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPTransportHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\RTSPVolumeHeader.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\RTSPHeader\SDPContent.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ngod_rtsp_parser\ClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\ClientSocket.h
# End Source File
# Begin Source File

SOURCE=.\ngod_rtsp_parser\Common.h
# End Source File
# End Group
# End Target
# End Project
