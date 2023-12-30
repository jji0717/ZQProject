# Microsoft Developer Studio Project File - Name="TsCtrlClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TsCtrlClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TsCtrlClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TsCtrlClient.mak" CFG="TsCtrlClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TsCtrlClient - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TsCtrlClient - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TsCtrlClient - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCTRLCLIENT_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/StreamSmith" /I "$(ZQProjsPath)/TianShan/ChannelOnDemand" /I "$(ZQProjsPath)/TianShan/Weiwoo" /I "$(ZQProjsPath)/TianShan/SiteAdminSvc" /I "$(ZQProjsPath)/TianShan/AccreditedPath" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/TsCtrlClient.dll" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib"

!ELSEIF  "$(CFG)" == "TsCtrlClient - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCTRLCLIENT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/StreamSmith" /I "$(ZQProjsPath)/TianShan/ChannelOnDemand" /I "$(ZQProjsPath)/TianShan/Weiwoo" /I "$(ZQProjsPath)/TianShan/SiteAdminSvc" /I "$(ZQProjsPath)/TianShan/AccreditedPath" /D "WIN32" /D "_DEBUG" /D _WIN32_WINNT=0x500 /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "_STLP_DEBUG" /FR /FD /GZ /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/TsCtrlClient_d.dll" /pdbtype:sept /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib"

!ENDIF 

# Begin Target

# Name "TsCtrlClient - Win32 Release"
# Name "TsCtrlClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\ChannelOnDemand\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ChannelOnDemand\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistEventSinkImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\ProvisionProgressImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\ProvisionStateChangeImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionEventSinkImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\SiteAdminSvc.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamProgressSinkImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\StreamSmith\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\TianShanEventImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsSite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsStorage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsStreamer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Weiwoo\WeiwooAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQEventsCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQEventsCtrl.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PlaylistEventSinkImpl.h
# End Source File
# Begin Source File

SOURCE=.\ProvisionProgressImpl.h
# End Source File
# Begin Source File

SOURCE=.\ProvisionStateChangeImpl.h
# End Source File
# Begin Source File

SOURCE=.\SessionEventSinkImpl.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkImpl.h
# End Source File
# Begin Source File

SOURCE=.\StreamProgressSinkImpl.h
# End Source File
# Begin Source File

SOURCE=.\TianShanEventImpl.h
# End Source File
# Begin Source File

SOURCE=.\TsCtrlClient.h
# End Source File
# Begin Source File

SOURCE=.\ZQEventsCtrl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\SiteAdminSvc\SiteAdminSvc.ICE

!IF  "$(CFG)" == "TsCtrlClient - Win32 Release"

# Begin Custom Build
InputPath=..\..\SiteAdminSvc\SiteAdminSvc.ICE
InputName=SiteAdminSvc

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/  ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --dict "TianShanIce::Site::SiteDict,string,TianShanIce::Site::VirtualSite" --dict-index "TianShanIce::Site::SiteDict,name" SiteDict ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --dict "TianShanIce::Site::AppDict,string,TianShanIce::Site::AppInfo" --dict-index "TianShanIce::Site::AppDict,name" AppDict ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --index "TianShanIce::Site::SessToTxn,TianShanIce::Site::LiveTxn,sessId,case-insensitive" SessToTxn ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn ../../SiteAdminSvc\SiteAdminSvc.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/ --index "TianShanIce::Site::AppToTxn,TianShanIce::Site::LiveTxn,appName,case-insensitive" AppToTxn ../../SiteAdminSvc\SiteAdminSvc.ICE \
	

"../../SiteAdminSvc/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/AppToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SiteToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SessToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/SessToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TsCtrlClient - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\AccreditedPath\TsPathAdmin.ICE

!IF  "$(CFG)" == "TsCtrlClient - Win32 Release"

# Begin Custom Build - dddd
InputPath=..\..\AccreditedPath\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../AccreditedPath  ..\..\AccreditedPath\TsPathAdmin.ICE

"../../AccreditedPath/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../AccreditedPath/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TsCtrlClient - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Weiwoo\WeiwooAdmin.ICE

!IF  "$(CFG)" == "TsCtrlClient - Win32 Release"

# Begin Custom Build
InputPath=..\..\Weiwoo\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../Weiwoo  ..\..\Weiwoo\WeiwooAdmin.ICE

"../../Weiwoo/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../Weiwoo/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TsCtrlClient - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
