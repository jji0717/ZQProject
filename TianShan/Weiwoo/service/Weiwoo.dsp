# Microsoft Developer Studio Project File - Name="Weiwoo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Weiwoo - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Weiwoo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Weiwoo.mak" CFG="Weiwoo - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Weiwoo - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Weiwoo - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/Weiwoo", GXSAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Weiwoo - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "EMBED_PATHSVC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/Weiwoo.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Weiwoo - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "EMBED_PATHSVC" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"../../bin/Weiwoo_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Weiwoo - Win32 Release"
# Name "Weiwoo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AppDict.cpp
# End Source File
# Begin Source File

SOURCE=..\AppToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\AppToTxn.cpp
# End Source File
# Begin Source File

SOURCE=..\AppToTxn.h
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\BusinessRouterImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\IdToSess.cpp
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

SOURCE=..\ServiceGroupDict.cpp
# End Source File
# Begin Source File

SOURCE=..\ServiceGroupToStreamLink.cpp
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

SOURCE=..\SessToTxn.cpp
# End Source File
# Begin Source File

SOURCE=..\SessToTxn.h
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\SiteAdminSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteDict.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToTxn.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToTxn.h
# End Source File
# Begin Source File

SOURCE=..\StorageDict.cpp
# End Source File
# Begin Source File

SOURCE=..\StorageLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\StorageToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamerDict.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamerToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamerToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\TsPathAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\WeiwooCmd.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooSvcEnv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\AppDict.h
# End Source File
# Begin Source File

SOURCE=..\AppToMount.h
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\BusinessRouterImpl.h
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

SOURCE=..\ServiceGroupDict.h
# End Source File
# Begin Source File

SOURCE=..\ServiceGroupToStreamLink.h
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

SOURCE=..\..\SiteAdminSvc\SiteAdminSvcEnv.h
# End Source File
# Begin Source File

SOURCE=..\SiteDict.h
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.h
# End Source File
# Begin Source File

SOURCE=..\StorageDict.h
# End Source File
# Begin Source File

SOURCE=..\StorageLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\StorageToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\StreamerDict.h
# End Source File
# Begin Source File

SOURCE=..\StreamerToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\StreamerToStreamLink.h
# End Source File
# Begin Source File

SOURCE=..\StreamLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TianShanDefines.h
# End Source File
# Begin Source File

SOURCE=..\TsPathAdmin.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.h
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

SOURCE=..\..\SiteAdminSvc\SiteAdminSvc.ICE

!IF  "$(CFG)" == "Weiwoo - Win32 Release"

!ELSEIF  "$(CFG)" == "Weiwoo - Win32 Debug"

# Begin Custom Build
InputDir=\projects\ZQProjs\TianShan\SiteAdminSvc
InputPath=..\..\SiteAdminSvc\SiteAdminSvc.ICE
InputName=SiteAdminSvc

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --dict "TianShanIce::Site::SiteDict,string,TianShanIce::Site::VirtualSite" --dict-index "TianShanIce::Site::SiteDict,name" SiteDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --dict "TianShanIce::Site::AppDict,string,TianShanIce::Site::AppInfo" --dict-index "TianShanIce::Site::AppDict,name" AppDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --index "TianShanIce::Site::SessToTxn,TianShanIce::Site::LiveTxn,sessId,case-insensitive" SessToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/SiteAdminSvc --output-dir .. --index "TianShanIce::Site::AppToTxn,TianShanIce::Site::LiveTxn,appName,case-insensitive" AppToTxn $(InputDir)\$(InputName).ice \
	

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\AppToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SessToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SessToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.ICE

!IF  "$(CFG)" == "Weiwoo - Win32 Release"

# Begin Custom Build
InputPath=..\..\AccreditedPath\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/AccreditedPath --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice  -I$(ZQProjsPath)/tianshan/AccreditedPath  --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir .. ..\..\AccreditedPath\$(InputName).ice \
	

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Weiwoo - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Weiwoo2.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.ICE

!IF  "$(CFG)" == "Weiwoo - Win32 Release"

!ELSEIF  "$(CFG)" == "Weiwoo - Win32 Debug"

# Begin Custom Build
InputDir=\projects\ZQProjs\TianShan\Weiwoo
InputPath=..\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::SRM::SiteDict,string,TianShanIce::SRM::Site" --dict-index "TianShanIce::SRM::SiteDict,name" SiteDict --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::SRM::AppDict,string,TianShanIce::SRM::AppInfo" --dict-index "TianShanIce::SRM::AppDict,name" AppDict --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::SiteToMountIndex,TianShanIce::SRM::AppMount,siteName,case-insensitive" SiteToMountIndex --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::AppToMountIndex,TianShanIce::SRM::AppMount,appName,case-insensitive" AppToMountIndex --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::SessionIndex,TianShanIce::SRM::SessionEx,sessId,case-sensitive" SessionIndex --output-dir .. $(InputDir)/$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::IdToSess,TianShanIce::SRM::SessionEx,sessId,case-sensitive" IdToSess --output-dir .. $(InputDir)/$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
