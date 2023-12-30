# Microsoft Developer Studio Project File - Name="MainCtrl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MainCtrl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MainCtrl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MainCtrl.mak" CFG="MainCtrl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MainCtrl - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MainCtrl - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/ScheduledTV/MainCtrl", UDCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MainCtrl - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MainCtrl - Win32 Debug"

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
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "MainCtrl - Win32 Release"
# Name "MainCtrl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ScheduleTV.cpp
# End Source File
# Begin Source File

SOURCE=.\STVStreamUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\STVStreamWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\testMC.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ScheduleTV.h
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
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
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

SOURCE=..\PlaylistMod\STVPlaylist.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistMod\STVPlaylist.h
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

SOURCE=..\RtspClient\RtspClientConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspClientConnection.h
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspConnectionManager.h
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

SOURCE=..\RtspClient\RtspRecvSink.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspRecvSink.h
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

SOURCE=..\RtspClient\RtspSock.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspClient\RtspSock.h
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

SOURCE=..\..\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.h
# End Source File
# Begin Source File

SOURCE=..\..\initest\ini.cpp
# End Source File
# Begin Source File

SOURCE=..\..\initest\ini.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\IPreference.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScLog.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\XMLPreference.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ZQ_common_conf.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\source\zqstringconv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\COMEXTRA\include\ZqStringConv.h
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
# Begin Group "AeBuilder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\AeBuilder\AeBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\AeBuilder\AeBuilder.h
# End Source File
# Begin Source File

SOURCE=..\AeBuilder\AeBuilderConf.h
# End Source File
# End Group
# End Target
# End Project
