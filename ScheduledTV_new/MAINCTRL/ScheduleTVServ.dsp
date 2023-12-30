# Microsoft Developer Studio Project File - Name="ScheduleTVServ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ScheduleTVServ - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ScheduleTVServ.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ScheduleTVServ.mak" CFG="ScheduleTVServ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ScheduleTVServ - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ScheduleTVServ - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ScheduledTV_new/MainCtrl", UDCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ScheduleTVServ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ServRelease"
# PROP Intermediate_Dir "ServRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /ZI /I "$(ZQProjsPath)\Common" /I "$(ZQProjsPath)\Common\COMEXTRA" /I "$(ITVSDKPATH)\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FR /FD /c
# SUBTRACT CPP /O<none> /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /map /debug /machine:I386 /out:"ServRelease/ITVPlayback.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\Lib\Release" /libpath:"$(ITVSDKPATH)\Lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "ServRelease\ITVPlayback.exe" "..\installation\Data Files\Exe\ITVPlayback.exe"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ScheduleTVServ - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQProjsPath)\Common" /I "$(ZQProjsPath)\Common\COMEXTRA" /I "$(ITVSDKPATH)\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/ITVPlayback_d.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)\Lib\Release" /libpath:"$(ITVSDKPATH)\Lib\Debug"

!ENDIF 

# Begin Target

# Name "ScheduleTVServ - Win32 Release"
# Name "ScheduleTVServ - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\BaseSchangeServiceApplication.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\SchangeServiceAppMain.cpp"
# End Source File
# Begin Source File

SOURCE=.\ScheduleTV.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduleTVServ.cpp
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScReporter.cpp"
# End Source File
# Begin Source File

SOURCE=.\STVStreamUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\STVStreamWorker.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\BaseSchangeServiceApplication.h"
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleTV.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleTVServ.h
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScReporter.h"
# End Source File
# Begin Source File

SOURCE=..\STVMainHeaders.h
# End Source File
# Begin Source File

SOURCE=.\STVStreamUnit.h
# End Source File
# Begin Source File

SOURCE=.\STVStreamWorker.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ScheduleTVServ.rc
# End Source File
# End Group
# Begin Group "SM Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SMInterface\SMConnector.cpp
# End Source File
# Begin Source File

SOURCE=..\SMInterface\SMConnector.h
# End Source File
# Begin Source File

SOURCE=..\SMInterface\SMXmlProc.cpp
# End Source File
# Begin Source File

SOURCE=..\SMInterface\SMXmlProc.h
# End Source File
# Begin Source File

SOURCE=..\SMInterface\XmlWriteStream.cpp
# End Source File
# Begin Source File

SOURCE=..\SMInterface\XmlWriteStream.h
# End Source File
# End Group
# Begin Group "Playlist Man"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PlaylistMod\STVChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVChannel.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVList.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVList.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVList_def.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVPlaylistManager.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVPlaylistManager.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVPMTimerMan.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVPMTimerMan.h
# End Source File
# End Group
# Begin Group "Rtsp Man"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\RtspClient\RtspClient.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspClient.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspConnectionManager.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspDaemon.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspDaemon.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspHeaders.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspMessage.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspMsgHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspMsgHeader.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspRequest.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspRequest.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspResponse.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspResponse.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspSocket.h
# End Source File
# End Group
# Begin Group "ISS Man"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamData.cpp
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamData.h
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamOpCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamOpCtrl.h
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamSession.cpp
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\StreamSession.h
# End Source File
# Begin Source File

SOURCE=..\IssStreamCtrl\stv_ssctrl.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Exception.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Exception.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\InetAddr.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\InetAddr.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\IPreference.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Log.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\Log.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\NativeThread.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\NativeThread.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScLog.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ScLog.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\ZQProjs\Common\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ZQProjs\Common\Socket.h
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\XMLPreference.cpp"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\XMLPreference.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\ZQ_common_conf.h"
# End Source File
# Begin Source File

SOURCE="$(ZQPROJSPATH)\Common\COMEXTRA\ZqStringConv.h"
# End Source File
# End Group
# Begin Group "Filler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Filler\AutoFiller.cpp
# End Source File
# Begin Source File

SOURCE=..\Filler\AutoFiller.h
# End Source File
# Begin Source File

SOURCE=..\Filler\BarkerFiller.cpp
# End Source File
# Begin Source File

SOURCE=..\Filler\BarkerFiller.h
# End Source File
# Begin Source File

SOURCE=..\Filler\filler.cpp
# End Source File
# Begin Source File

SOURCE=..\Filler\Filler.h
# End Source File
# Begin Source File

SOURCE=..\Filler\FillerConf.h
# End Source File
# End Group
# End Target
# End Project
