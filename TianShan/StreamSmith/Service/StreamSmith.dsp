# Microsoft Developer Studio Project File - Name="StreamSmith" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=StreamSmith - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StreamSmith.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StreamSmith.mak" CFG="StreamSmith - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StreamSmith - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "StreamSmith - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/StreamSmith/Service"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "$(RegExppKit)" /I "$(ZQPROJSPATH)\Common" /I "$(ICE_ROOT)/include/stlport/" /I "$(OPENSSLPATH)\include" /I "$(ZQPROJSPATH)\Common\Rtsp\RtspParser" /I "$(ZQPROJSPATH)\Common\Rtsp\Utils" /I "$(ZQPROJSPATH)\Common\SSLLib" /I "$(ZQPROJSPATH)\Common\ServiceFrame" /I "$(VSTRMKITPATH)\sdk\inc" /I "." /I "$(ITVSDKPATH)\include" /I "$(ICE_ROOT)/include/" /I "../../../tianshan/ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "..\\" /I "$(ExpatPath)\include" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\include" /I "..\nodecontentstore\\" /I "..\nodecontentstore\ice" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ZQPROJSPATH)\Generic\ContentProcess" /D "WITH_ICESTORM" /D "_ICE_INTERFACE_SUPPORT" /D _WIN32_WINNT=0x0400 /D "_CONSOLE" /D "_MBCS" /D "MBCS" /D "_MT" /D "_WIN32_SPECIAL_VERSION" /D "NDEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_DLL" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQPROJSPATH)/build" /i "$(VSTRMKITPATH)/SDK/inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 shlwapi.lib msvcrt.lib SpigotDll.lib icestorm.lib Freeze.lib Ice.lib IceUtil.lib ws2_32.lib VstrmDLL.lib libeay32.lib ssleay32.lib advapi32.lib /nologo /subsystem:console /pdb:"../../bin/StreamSmith.pdb" /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"../../bin/StreamSmith.exe" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(ZQPROJSPATH)\Common\Rtsp\RtspParser\Release" /libpath:"$(ZQPROJSPATH)\Common\ServiceFrame\Lib\release" /libpath:"$(VSTRMKITPATH)\sdk\LIB" /libpath:"$(OPENSSLPATH)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "$(ZQPROJSPATH)\Common" /I "$(ICE_ROOT)/include/stlport/" /I "$(OPENSSLPATH)\include" /I "$(ZQPROJSPATH)\Common\Rtsp\RtspParser" /I "$(ZQPROJSPATH)\Common\Rtsp\Utils" /I "$(ZQPROJSPATH)\Common\SSLLib" /I "$(ZQPROJSPATH)\Common\ServiceFrame" /I "$(VSTRMKITPATH)\sdk\inc" /I "." /I "$(ITVSDKPATH)\include" /I "$(ICE_ROOT)/include/" /I "../../../tianshan/ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "..\\" /I "$(ExpatPath)\include" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\include" /I "..\nodecontentstore\\" /I "..\nodecontentstore\ice" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ZQPROJSPATH)\Generic\ContentProcess" /I "$(RegExppKit)" /D "WITH_ICESTORM" /D "_ICE_INTERFACE_SUPPORT" /D "_DEBUG_TEST" /D _WIN32_WINNT=0x0400 /D "_CONSOLE" /D "MBCS" /D "_MT" /D "_WIN32_SPECIAL_VERSION" /D "_DLL" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQPROJSPATH)/build" /i "$(VSTRMPATH)/inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shlwapi.lib msvcrtd.lib SpigotDll.lib icestormd.lib Freezed.lib Iced.lib IceUtild.lib ws2_32.lib VstrmDLL.lib libeay32.lib ssleay32.lib ole32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"../../bin/StreamSmith.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"../../bin/StreamSmith.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib" /libpath:"$(ZQPROJSPATH)\Common\Rtsp\RtspParser\debug" /libpath:"$(ZQPROJSPATH)\Common\ServiceFrame\Lib\release" /libpath:"$(VSTRMKITPATH)\sdk\LIB" /libpath:"$(OPENSSLPATH)\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "StreamSmith - Win32 Release"
# Name "StreamSmith - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ADebugMem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\checkContent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\ConfigParser.cpp
# End Source File
# Begin Source File

SOURCE=..\DialogCreator.cpp
# End Source File
# Begin Source File

SOURCE=..\FailStorePlInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\global.cpp
# End Source File
# Begin Source File

SOURCE=..\ItemToPlaylist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\Playlist.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistDict.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistExI.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistInternalUse.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistItemI.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspSession.cpp
# End Source File
# Begin Source File

SOURCE=..\RtspSessionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ssService.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSmith.rc
# End Source File
# Begin Source File

SOURCE=..\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamSmithSite.cpp
# End Source File
# Begin Source File

SOURCE=..\URIParser.cpp
# End Source File
# Begin Source File

SOURCE=..\VstrmClass.cpp
# End Source File
# Begin Source File

SOURCE=..\VstrmSessMon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ADebugMem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\checkContent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ConfigLoader.h
# End Source File
# Begin Source File

SOURCE=..\ConfigParser.h
# End Source File
# Begin Source File

SOURCE=..\descCode.h
# End Source File
# Begin Source File

SOURCE=..\DialogCreator.h
# End Source File
# Begin Source File

SOURCE=..\FailStorePlInfo.h
# End Source File
# Begin Source File

SOURCE=..\global.h
# End Source File
# Begin Source File

SOURCE=..\ItemToPlaylist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\Playlist.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistDict.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistExI.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistInternalUse.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistItemI.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\ResourceManager.h
# End Source File
# Begin Source File

SOURCE=..\RtspDialog.h
# End Source File
# Begin Source File

SOURCE=..\RtspSession.h
# End Source File
# Begin Source File

SOURCE=..\RtspSessionMgr.h
# End Source File
# Begin Source File

SOURCE=.\ssService.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithAdmin.h
# End Source File
# Begin Source File

SOURCE=.\StreamSmithConfig.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithModule.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithModuleEx.h
# End Source File
# Begin Source File

SOURCE=..\StreamSmithSite.h
# End Source File
# Begin Source File

SOURCE=..\URIParser.h
# End Source File
# Begin Source File

SOURCE=..\VstrmClass.h
# End Source File
# Begin Source File

SOURCE=..\VVXParser\VstrmProc.h
# End Source File
# Begin Source File

SOURCE=..\VstrmSessMon.h
# End Source File
# Begin Source File

SOURCE=..\VVXParser\VvxParser.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ContentDict.ice

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\ContentDict.ice
InputName=ContentDict

BuildCmds= \
	$(ICE_ROOT)/bin/slice2cpp.exe  -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath)  --output-dir ../nodecontentstore/ice/  $(InputPath) \
	$(ICE_ROOT)/bin/slice2freeze  -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict Contents,string,ContentDictData::ContentAttrs Data $(InputPath)  --output-dir ../nodecontentstore/ice/ $(InputPath) \
	

"../NodeContentStore/ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/Data.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/Data.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\ContentDict.ice
InputName=ContentDict

BuildCmds= \
	$(ICE_ROOT)/bin/slice2cpp.exe  -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath)  --output-dir ../nodecontentstore/ice/  $(InputPath) \
	$(ICE_ROOT)/bin/slice2freeze  -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict Contents,string,ContentDictData::ContentAttrs Data $(InputPath)  --output-dir ../nodecontentstore/ice/ $(InputPath) \
	

"../NodeContentStore/ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/Data.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/Data.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\icon_smith.ico
# End Source File
# Begin Source File

SOURCE=.\PlaylistInternalUse.ice

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

# Begin Custom Build
InputPath=.\PlaylistInternalUse.ice
InputName=PlaylistInternalUse

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\ $(InputName).ice

"..\$(inputname).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

# Begin Custom Build
InputPath=.\PlaylistInternalUse.ice
InputName=PlaylistInternalUse

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\ $(InputName).ice

"..\$(inputname).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ProvSchdSafeStore.ice

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\ProvSchdSafeStore.ice
InputName=ProvSchdSafeStore

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath)  --output-dir ../nodecontentstore/ice/  $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict ProvStatusMsgs,string,TianShanIce::Storage::StatusMsg StatusMsgData $(InputPath) --output-dir ../nodecontentstore/ice/  $(InputPath) \
	

"../NodeContentStore/ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StatusMsgData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StatusMsgData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\ProvSchdSafeStore.ice
InputName=ProvSchdSafeStore

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath)  --output-dir ../nodecontentstore/ice/  $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict ProvStatusMsgs,string,TianShanIce::Storage::StatusMsg StatusMsgData $(InputPath) --output-dir ../nodecontentstore/ice/  $(InputPath) \
	

"../NodeContentStore/ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StatusMsgData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StatusMsgData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdmin.ICE

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

# Begin Custom Build
InputPath=.\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\ $(InputName).ice \
	$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --dict "TianShanIce::Streamer::PlaylistDict,string,TianShanIce::Streamer::PlaylistEx" --dict-index "TianShanIce::Streamer::PlaylistDict,guid" PlaylistDict --output-dir ..\ $(InputName).ICE \
	$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "TianShanIce::Streamer::IndexItemToPlaylist,TianShanIce::Streamer::PlaylistItem,guid,case-insensitive" ItemToPlaylist --output-dir ..\ $(InputName).ICE \
	

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\PlaylistDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\PlaylistDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ItemToPlaylist.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ItemToPlaylist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

# Begin Custom Build
InputPath=.\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ..\ $(InputName).ice \
	$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --dict "TianShanIce::Streamer::PlaylistDict,string,TianShanIce::Streamer::PlaylistEx" --dict-index "TianShanIce::Streamer::PlaylistDict,guid" PlaylistDict --output-dir ..\ $(InputName).ICE \
	$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "TianShanIce::Streamer::IndexItemToPlaylist,TianShanIce::Streamer::PlaylistItem,guid,case-insensitive" ItemToPlaylist --output-dir ..\ $(InputName).ICE \
	

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\PlaylistDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\PlaylistDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ItemToPlaylist.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ItemToPlaylist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\TsStorageMediaCluster.ice

!IF  "$(CFG)" == "StreamSmith - Win32 Release"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\TsStorageMediaCluster.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ../nodecontentstore/ice/ $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --dict IdleContents,string,string  IdleContents  --output-dir ../nodecontentstore/ice/ $(InputPath) \
	

"../NodeContentStore/ice/StorageMediacluster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StorageMediacluster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/IdleContents.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/IdleContents.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "StreamSmith - Win32 Debug"

# Begin Custom Build
InputPath=..\NodeContentStore\ICE\TsStorageMediaCluster.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir ../nodecontentstore/ice/ $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --dict IdleContents,string,string  IdleContents  --output-dir ../nodecontentstore/ice/ $(InputPath) \
	

"../NodeContentStore/ice/StorageMediacluster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/StorageMediacluster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/IdleContents.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../NodeContentStore/ice/IdleContents.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "NCS source files"

# PROP Default_Filter "*.cpp;*.c"
# Begin Source File

SOURCE=..\NodeContentStore\ContentCleaner.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ContentDict.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStorage.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreI.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreServer.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\Data.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IceLog.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\IdleContents.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\MCNodeStoreServ.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\NCSBridge.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ProvSchdSafeStore.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ProvSchdSafeStoreI.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\StatusMsgData.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\StatusUpdater.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\TsStorageMediaCluster.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\vstrmFileEventMonitor.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VstrmProc.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VV2Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VvxParser.cpp
# End Source File
# End Group
# Begin Group "NCS header files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\NodeContentStore\ContentAttrCheck.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentCleaner.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ContentDict.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStorage.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreConfig.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreI.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreServer.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ContentStoreUtils.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\Data.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\EventDispatcher.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\EventPublisher.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\IdleContents.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\MCNodeStoreServ.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\NCSBridge.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\NodeCS.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\NodeCSTask.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\NodeGraph.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ProvisionEventSink.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\ProvSchdSafeStore.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ProvSchdSafeStoreI.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\StatusMsgData.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\StatusUpdater.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\ICE\TsStorageMediaCluster.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\vstrmFileEventMonitor.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VstrmProc.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VV2Parser.h
# End Source File
# Begin Source File

SOURCE=..\NodeContentStore\VvxParser.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
