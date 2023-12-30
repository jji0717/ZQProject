# Microsoft Developer Studio Project File - Name="WeiwooService" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=WeiwooService - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WeiwooService.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WeiwooService.mak" CFG="WeiwooService - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WeiwooService - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "WeiwooService - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/Weiwoo/service", WWXAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WeiwooService - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "$(RegExppKit)" /I "$(ICE_ROOT)/include/stlport" /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /I "$(ZQProjsPath)/TianShan/include" /I "../../AccreditedPath/" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "EMBED_PATHSVC" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)\include" /i "$(ZQPROJSPATH)\build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /pdb:"../../bin/WeiwooService.pdb" /debug /machine:I386 /out:"../../bin/WeiwooService.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                            $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll                           ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "WeiwooService - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(RegExppKit)" /I "$(ICE_ROOT)/include/stlport" /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /I "$(ZQProjsPath)/TianShan/include" /I "../../AccreditedPath/" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "EMBED_PATHSVC" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)\include" /i "$(ZQPROJSPATH)\build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /pdb:"../../bin/WeiwooService.pdb" /map /debug /machine:I386 /out:"../../bin/WeiwooService.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib"
# SUBTRACT LINK32 /profile /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                            $(ZQProjsPath)\Common\dll\DebugStlp\*.dll                                            ..\..\bin\                                           	copy                                            $(ZQPRojsPath)\common\FileLogDll\debug\filelogdll_d.dll                                            ..\..\bin\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "WeiwooService - Win32 Release"
# Name "WeiwooService - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\IdToSess.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathHelperMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathManagerImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\ServiceGroupDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\ServiceGroupToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionState.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionWatchDog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\WeiwooService.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\IdToSess.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\pho\IpEdgePHO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathCommand.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathHelperMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathManagerImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\PathSvcEnv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ScLog.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\ServiceGroupDict.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\ServiceGroupToStreamLink.h
# End Source File
# Begin Source File

SOURCE=..\SessionCommand.h
# End Source File
# Begin Source File

SOURCE=..\SessionImpl.h
# End Source File
# Begin Source File

SOURCE=..\SessionState.h
# End Source File
# Begin Source File

SOURCE=..\SessionWatchDog.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageDict.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StorageToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerDict.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamerToStreamLink.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\StreamLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\strHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.h
# End Source File
# Begin Source File

SOURCE=.\WeiwooConfig.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.h
# End Source File
# Begin Source File

SOURCE=.\WeiwooService.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooSvcEnv.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon_weiwoo.ico
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.ICE

!IF  "$(CFG)" == "WeiwooService - Win32 Release"

# Begin Custom Build
InputPath=..\..\AccreditedPath\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\  $(ZQPROJSPATH)/tianshan/AccreditedPath/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	

"$(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "WeiwooService - Win32 Debug"

# Begin Custom Build
InputPath=..\..\AccreditedPath\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\  $(ZQPROJSPATH)/tianshan/AccreditedPath/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/ice  --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir $(ZQPROJSPATH)\tianshan\AccreditedPath\ $(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).ice \
	

"$(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(ZQPROJSPATH)\tianshan\AccreditedPath\ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Weiwoo.rc
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.ICE

!IF  "$(CFG)" == "WeiwooService - Win32 Release"

# Begin Custom Build
InputPath=..\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::IdToSess,TianShanIce::SRM::SessionEx,sessId,case-sensitive" IdToSess --output-dir .. ../$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "WeiwooService - Win32 Debug"

# Begin Custom Build
InputPath=..\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::SRM::SiteDict,string,TianShanIce::SRM::Site" --dict-index "TianShanIce::SRM::SiteDict,name" SiteDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::SRM::AppDict,string,TianShanIce::SRM::AppInfo" --dict-index "TianShanIce::SRM::AppDict,name" AppDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::SiteToMount,TianShanIce::SRM::AppMount,siteName,case-insensitive" SiteToMount --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::AppToMount,TianShanIce::SRM::AppMount,appName,case-insensitive" AppToMount --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::IdToSess,TianShanIce::SRM::SessionEx,sessId,case-sensitive" IdToSess --output-dir .. ../$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
