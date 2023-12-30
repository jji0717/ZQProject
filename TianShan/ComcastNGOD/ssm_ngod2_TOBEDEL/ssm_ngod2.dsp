# Microsoft Developer Studio Project File - Name="ssm_ngod2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ssm_ngod2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssm_ngod2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssm_ngod2.mak" CFG="ssm_ngod2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssm_ngod2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ssm_ngod2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssm_ngod2 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_NGOD2_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ZQPROJSPATH)/common" /I "$(ZQPROJSPATH)/Tianshan/Ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(ZQPROJSPATH)\tianshan\StreamSmith" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)\common" /I "$(ZQPROJSPATH)\TianShan\Shell\ZQSNMPManPkg" /I "$(RegExppKit)" /D "WITH_ICESTORM" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_NGOD2_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /D "CHECK_WITH_GLOG" /D "LOGFMTWITHTID" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)\build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../bin/ssm_ngod2.pdb" /debug /machine:I386 /out:"../../bin/ssm_ngod2.dll" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(RegExppKit)\bin"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ssm_ngod2 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_NGOD2_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ZQPROJSPATH)/common" /I "$(ZQPROJSPATH)/Tianshan/Ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(ZQPROJSPATH)\tianshan\StreamSmith" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)\common" /I "$(ZQPROJSPATH)\TianShan\Shell\ZQSNMPManPkg" /I "$(RegExppKit)" /D "WITH_ICESTORM" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_NGOD2_EXPORTS" /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /D "LOGFMTWITHTID" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)\build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../bin/ssm_ngod2.pdb" /debug /machine:I386 /out:"../../bin/ssm_ngod2.dll" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ssm_ngod2 - Win32 Release"
# Name "ssm_ngod2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\ContextImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\GetParamHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupIdx.cpp
# End Source File
# Begin Source File

SOURCE=.\LAMFacade.cpp
# End Source File
# Begin Source File

SOURCE=.\Ngod2Config.cpp
# End Source File
# Begin Source File

SOURCE=.\NGODEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\NGODFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\NGODr2c1.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\PauseHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\PenaltyManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PingHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\RequestHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionTimerCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionWatchDog.cpp
# End Source File
# Begin Source File

SOURCE=.\SetParamHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupHandlerCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\ssm_ngod2.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkI.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamIdx.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\stroprt.cpp
# End Source File
# Begin Source File

SOURCE=.\TeardownHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\thrdConnService.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=.\ContextImpl.h
# End Source File
# Begin Source File

SOURCE=.\GetParamHandler.h
# End Source File
# Begin Source File

SOURCE=.\GroupIdx.h
# End Source File
# Begin Source File

SOURCE=.\LAMFacade.h
# End Source File
# Begin Source File

SOURCE=.\Ngod2Config.h
# End Source File
# Begin Source File

SOURCE=.\ngodDefs.h
# End Source File
# Begin Source File

SOURCE=.\NGODEnv.h
# End Source File
# Begin Source File

SOURCE=.\NGODFactory.h
# End Source File
# Begin Source File

SOURCE=.\NGODr2c1.h
# End Source File
# Begin Source File

SOURCE=.\OptionHandler.h
# End Source File
# Begin Source File

SOURCE=.\PauseHandler.h
# End Source File
# Begin Source File

SOURCE=.\PenaltyManager.h
# End Source File
# Begin Source File

SOURCE=.\PingHandler.h
# End Source File
# Begin Source File

SOURCE=.\PlayHandler.h
# End Source File
# Begin Source File

SOURCE=.\RequestHandler.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SessionTimerCommand.h
# End Source File
# Begin Source File

SOURCE=.\SessionWatchDog.h
# End Source File
# Begin Source File

SOURCE=.\SetParamHandler.h
# End Source File
# Begin Source File

SOURCE=.\SetupHandler.h
# End Source File
# Begin Source File

SOURCE=.\SetupHandlerCommand.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkI.h
# End Source File
# Begin Source File

SOURCE=.\StreamIdx.h
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdmin.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# Begin Source File

SOURCE=.\TeardownHandler.h
# End Source File
# Begin Source File

SOURCE=.\thrdConnService.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\LAMFacade.ice

!IF  "$(CFG)" == "ssm_ngod2 - Win32 Release"

# Begin Custom Build
InputPath=.\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_ngod2 - Win32 Debug"

# Begin Custom Build
InputPath=.\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NGODr2c1.ICE

!IF  "$(CFG)" == "ssm_ngod2 - Win32 Release"

# Begin Custom Build
InputPath=.\NGODr2c1.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice  -I$(ZQPROJSPATH)/tianshan/ice  --output-dir . .\NGODr2c1.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "NGODr2c1::StreamIdx,NGODr2c1::Context,streamShortID" StreamIdx --output-dir . .\NGODr2c1.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "NGODr2c1::GroupIdx,NGODr2c1::Context,groupID" GroupIdx --output-dir . .\NGODr2c1.ICE \
	

"NGODr2c1.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"NGODr2c1.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"GroupIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"GroupIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_ngod2 - Win32 Debug"

# Begin Custom Build
InputPath=.\NGODr2c1.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice  -I$(ZQPROJSPATH)/tianshan/ice  --output-dir . .\NGODr2c1.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "NGODr2c1::StreamIdx,NGODr2c1::Context,streamShortID" StreamIdx --output-dir . .\NGODr2c1.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --index "NGODr2c1::GroupIdx,NGODr2c1::Context,groupID" GroupIdx --output-dir . .\NGODr2c1.ICE \
	

"NGODr2c1.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"NGODr2c1.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"GroupIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"GroupIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ssm_NGOD2.rc
# End Source File
# Begin Source File

SOURCE=..\..\StreamSmith\Service\StreamSmithAdmin.ICE

!IF  "$(CFG)" == "ssm_ngod2 - Win32 Release"

# Begin Custom Build
InputPath=..\..\StreamSmith\Service\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir .\  ..\..\StreamSmith\Service\$(InputName).ice

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_ngod2 - Win32 Debug"

# Begin Custom Build
InputPath=..\..\StreamSmith\Service\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice --output-dir .\  ..\..\StreamSmith\Service\$(InputName).ice

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
