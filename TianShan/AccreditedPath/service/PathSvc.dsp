# Microsoft Developer Studio Project File - Name="PathSvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PathSvc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PathSvc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PathSvc.mak" CFG="PathSvc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PathSvc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PathSvc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/AccreditedPath/PathSvc", FCTAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PathSvc - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/TianShan/ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ITVSDKPATH)\include" /I "$(ZQProjsPath)\TianShan\Weiwoo\service" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/PathSvc.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PathSvc - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I ".." /I "$(ZQProjsPath)/TianShan/Common" /I "../../Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ITVSDKPATH)\include" /I "$(ZQProjsPath)\TianShan\Weiwoo\service" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"../../bin/PathSvc_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PathSvc - Win32 Release"
# Name "PathSvc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\common\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\PathCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\PathFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\PathHelperMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\PathManagerImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\PathSvc.cpp
# End Source File
# Begin Source File

SOURCE=..\PathSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ScLog.cpp
# End Source File
# Begin Source File

SOURCE=..\ServiceGroupDict.cpp
# End Source File
# Begin Source File

SOURCE=..\ServiceGroupToStreamLink.cpp
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
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\common\ConfigLoader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\pho\IpEdgePHO.h
# End Source File
# Begin Source File

SOURCE=..\PathCommand.h
# End Source File
# Begin Source File

SOURCE=..\PathFactory.h
# End Source File
# Begin Source File

SOURCE=..\PathHelperMgr.h
# End Source File
# Begin Source File

SOURCE=..\PathManagerImpl.h
# End Source File
# Begin Source File

SOURCE=..\PathSvcEnv.h
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

SOURCE=..\TsPathAdmin.h
# End Source File
# Begin Source File

SOURCE=..\..\Weiwoo\service\WeiwooConfig.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon_path.ico
# End Source File
# Begin Source File

SOURCE=.\PathSvc2.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i ".." /i "../../../build"
# End Source File
# Begin Source File

SOURCE=..\TsPathAdmin.ICE

!IF  "$(CFG)" == "PathSvc - Win32 Release"

# Begin Custom Build
InputPath=..\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/ice --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir .. ../$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "PathSvc - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
