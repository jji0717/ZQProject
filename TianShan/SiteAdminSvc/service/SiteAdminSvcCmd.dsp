# Microsoft Developer Studio Project File - Name="SiteAdminSvcCmd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SiteAdminSvcCmd - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SiteAdminSvcCmd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SiteAdminSvcCmd.mak" CFG="SiteAdminSvcCmd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SiteAdminSvcCmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SiteAdminSvcCmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/SiteAdminSvc/service"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SiteAdminSvcCmd - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)\include" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /I "$(RegExppKit)" /D "_WITH_EVENTSENDER_" /D "WITH_ICESTORM" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "EMBED_PATHSVC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(ITVSDKPATH)\include" /i "$(ZQPROJSPATH)\build" /i ".\\" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /pdb:"../../bin/SiteAdminSvc.pdb" /debug /machine:I386 /out:"../../bin/SiteAdminSvc.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)\lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SiteAdminSvcCmd - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "./" /I "$(ZQProjsPath)/Common" /I "$(ICE_ROOT)/include/stlport" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "../../Ice" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /I "../" /D "_WITH_EVENTSENDER_" /D "WITH_ICESTORM" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ZQPROJSPATH)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /pdb:"../../bin/SiteAdminSvc.pdb" /debug /debugtype:both /machine:I386 /out:"../../bin/SiteAdminSvc.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib"
# SUBTRACT LINK32 /profile /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SiteAdminSvcCmd - Win32 Release"
# Name "SiteAdminSvcCmd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AppDict.cpp
# End Source File
# Begin Source File

SOURCE=..\AppToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\eventSenderManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MdbLog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\MountToTxn.cpp
# End Source File
# Begin Source File

SOURCE=..\SACommand.cpp
# End Source File
# Begin Source File

SOURCE=..\SASFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\siteadmin.rc
# End Source File
# Begin Source File

SOURCE=..\SiteAdminImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteAdminSvc.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteAdminSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteDict.cpp
# End Source File
# Begin Source File

SOURCE=.\SiteService.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToTxn.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamEventReceiver.cpp
# End Source File
# Begin Source File

SOURCE=..\TxnToEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ZQServiceAppMain.cpp
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

SOURCE=..\..\..\COMMON\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\eventSenderManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\MountToTxn.h
# End Source File
# Begin Source File

SOURCE=..\SACommand.h
# End Source File
# Begin Source File

SOURCE=..\SASFactory.h
# End Source File
# Begin Source File

SOURCE=..\SessToTxn.h
# End Source File
# Begin Source File

SOURCE=..\SiteAdminImpl.h
# End Source File
# Begin Source File

SOURCE=..\SiteAdminSvc.h
# End Source File
# Begin Source File

SOURCE=..\SiteAdminSvcEnv.h
# End Source File
# Begin Source File

SOURCE=..\SiteDict.h
# End Source File
# Begin Source File

SOURCE=.\SiteService.h
# End Source File
# Begin Source File

SOURCE=.\SiteServiceConfig.h
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.h
# End Source File
# Begin Source File

SOURCE=..\SiteToTxn.h
# End Source File
# Begin Source File

SOURCE=..\StreamEventReceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\common\TianShanDefines.h
# End Source File
# Begin Source File

SOURCE=..\TxnToEvent.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\SiteAdminSvc.ICE

!IF  "$(CFG)" == "SiteAdminSvcCmd - Win32 Release"

# Begin Custom Build
InputDir=\SOS\ZQProjs\TianShan\SiteAdminSvc
InputPath=..\SiteAdminSvc.ICE
InputName=SiteAdminSvc

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::SiteDict,string,TianShanIce::Site::VirtualSite" --dict-index "TianShanIce::Site::SiteDict,name" SiteDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::AppDict,string,TianShanIce::Site::AppInfo" --dict-index "TianShanIce::Site::AppDict,name" AppDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::MountToTxn ,TianShanIce::Site::LiveTxn,mountedPath,case-insensitive" MountToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::TxnToEvent,TianShanIce::Site::TxnEvent,identTxn" TxnToEvent $(InputDir)\$(InputName).ice \
	

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

"..\MountToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\MountToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\TxnToEvent.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\TxnToEvent.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "SiteAdminSvcCmd - Win32 Debug"

# Begin Custom Build
InputDir=\SOS\ZQProjs\TianShan\SiteAdminSvc
InputPath=..\SiteAdminSvc.ICE
InputName=SiteAdminSvc

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::SiteDict,string,TianShanIce::Site::VirtualSite" --dict-index "TianShanIce::Site::SiteDict,name" SiteDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::AppDict,string,TianShanIce::Site::AppInfo" --dict-index "TianShanIce::Site::AppDict,name" AppDict $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::MountToTxn ,TianShanIce::Site::LiveTxn,mountedPath,case-insensitive" MountToTxn $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::TxnToEvent,TianShanIce::Site::TxnEvent,identTxn" TxnToEvent $(InputDir)\$(InputName).ice \
	

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

"..\MountToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\MountToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\SiteToTxn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\TxnToEvent.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\TxnToEvent.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
