# Microsoft Developer Studio Project File - Name="CPE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CPE - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CPE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CPE.mak" CFG="CPE - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CPE - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CPE - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CPE - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "../PT_FtpServer" /D "NDEBUG" /D _WIN32_WINNT=0x500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "LOGFMTWITHTID" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /out:"../../bin/CPE.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CPE - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "../PT_FtpServer" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "LOGFMTWITHTID" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"../../bin/CPE_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "CPE - Win32 Release"
# Name "CPE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\PT_FtpServer\CECommon.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\CmdLine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\ContentToProvision.cpp
# End Source File
# Begin Source File

SOURCE=..\CPE.cpp
# End Source File
# Begin Source File

SOURCE=..\cpecfg.cpp
# End Source File
# Begin Source File

SOURCE=.\CPECmd.cpp
# End Source File
# Begin Source File

SOURCE=..\CPEEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\CPEFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\CPEImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpPushSess.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpServer.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpSite.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpSock.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\FtpsXfer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\ProvisionCmds.cpp
# End Source File
# Begin Source File

SOURCE=..\ProvisionFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\ProvisionState.cpp
# End Source File
# Begin Source File

SOURCE=..\PushMgrClient.cpp
# End Source File
# Begin Source File

SOURCE=..\PushModule.cpp
# End Source File
# Begin Source File

SOURCE=..\QueueBufMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\RDSBridge.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\TermService.cpp
# End Source File
# Begin Source File

SOURCE=..\PT_FtpServer\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ContentToProvision.h
# End Source File
# Begin Source File

SOURCE=..\CPE.h
# End Source File
# Begin Source File

SOURCE=..\CPEEnv.h
# End Source File
# Begin Source File

SOURCE=..\CPEFactory.h
# End Source File
# Begin Source File

SOURCE=..\CPEImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\ICPHelper.h
# End Source File
# Begin Source File

SOURCE=..\ProvisionCmds.h
# End Source File
# Begin Source File

SOURCE=..\ProvisionFactory.h
# End Source File
# Begin Source File

SOURCE=..\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\CPE.ICE

!IF  "$(CFG)" == "CPE - Win32 Release"

# Begin Custom Build
InputPath=..\CPE.ICE
InputName=CPE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/AccreditedPath --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice  --index "TianShanIce::ContentProvision::ContentToProvision,TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" ContentToProvision --output-dir .. ../$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ContentToProvision.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ContentToProvision.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CPE - Win32 Debug"

# Begin Custom Build
InputDir=\ZQProjs\TianShan\CPE
InputPath=..\CPE.ICE
InputName=CPE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/AccreditedPath --output-dir .. $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice  --index "TianShanIce::ContentProvision::ContentToProvision,TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" ContentToProvision --output-dir .. $(InputDir)\$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ContentToProvision.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ContentToProvision.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPE.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "." /i ".." /i "$(ZQProjsPath)/build"
# End Source File
# Begin Source File

SOURCE=..\..\ContentStore\ICE\PushModule.ice

!IF  "$(CFG)" == "CPE - Win32 Release"

# Begin Custom Build
InputPath=..\..\ContentStore\ICE\PushModule.ice
InputName=PushModule

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice --output-dir .. $(InputPath)

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CPE - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ContentStore\ICE\PushModule.ice
InputName=PushModule

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice --output-dir .. $(InputPath)

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
