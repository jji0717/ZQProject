# Microsoft Developer Studio Project File - Name="ssm_PauseTV_s1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ssm_PauseTV_s1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssm_PauseTV_s1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssm_PauseTV_s1.mak" CFG="ssm_PauseTV_s1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssm_PauseTV_s1 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ssm_PauseTV_s1 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/StreamNow/StreamSmith/Modules/ssm_PauseTV_s1/ssm_PauseTV_s1", YTXAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_PAUSETV_S1_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "./" /I "..\..\\" /I "..\\" /I "$(ZQPROJSPATH)/common" /I "$(ZQPROJSPATH)\Tianshan\Ice" /I "$(ZQPROJSPATH)\Tianshan\common" /I "$(ZQPROJSPATH)\ChannelOnDemand" /I "$(ITVSDKPATH)\Include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_PAUSETV_S1_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x400 /D "WITH_ICESTORM" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../../bin/ssm_PauseTV_s1.pdb" /debug /machine:I386 /out:"../../../bin/ssm_PauseTV_s1.dll" /libpath:"$(ITVSDKPATH)\Lib\Release" /libpath:"$(ICE_ROOT)\lib"

!ELSEIF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_PAUSETV_S1_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I ".\\" /I "..\..\\" /I "..\\" /I "$(ZQPROJSPATH)\channelondemand" /I "$(ZQPROJSPATH)\StreamNow\StreamSmith\Service" /I "$(ZQPROJSPATH)\common" /I "$(ZQPROJSPATH)\tianshan\common" /I "$(ZQPROJSPATH)\Tianshan\Ice" /I "$(ZQPROJSPATH)\build" /I "$(ITVSDKPATH)\Include" /I "$(ICE_ROOT)\include" /I "$(ICE_ROOT)\include\stlport" /D "_DEBUG" /D "_STLP_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSM_PAUSETV_S1_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x400 /D "WITH_ICESTORM" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iced.lib iceutild.lib icestormd.lib freezed.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"$(ITVSDKPATH)\Lib\Debug" /libpath:"$(ICE_ROOT)\lib"

!ENDIF 

# Begin Target

# Name "ssm_PauseTV_s1 - Win32 Release"
# Name "ssm_PauseTV_s1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\FileLog.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistEventSinkI.cpp
# End Source File
# Begin Source File

SOURCE=.\ssm_PauseTV_s1.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkI.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\thrdCleanupSession.cpp
# End Source File
# Begin Source File

SOURCE=.\thrdConnService.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\tianshan\ice\TsStreamer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChannelOnDemand.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.h
# End Source File
# Begin Source File

SOURCE=.\exptHandle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\FileLog.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistEventSinkI.h
# End Source File
# Begin Source File

SOURCE=.\purchaseProperty.h
# End Source File
# Begin Source File

SOURCE=.\ssm_PauseTV_s1.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StreamEventSinkI.h
# End Source File
# Begin Source File

SOURCE=.\StreamSmithAdmin.h
# End Source File
# Begin Source File

SOURCE=..\..\StreamSmithModule.h
# End Source File
# Begin Source File

SOURCE=.\thrdCleanupSession.h
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

SOURCE=..\..\..\ChannelOnDemand\ChannelOnDemand.ICE

!IF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Release"

# Begin Custom Build
InputPath=..\..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir $(ZQProjspath)\tianshan\streamsmith\modules\ssm_pausetv_s1\ $(ZQProjsPath)\TianShan\ChannelOnDemand\$(InputName).ice

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Debug"

# Begin Custom Build
InputPath=..\..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir $(ZQProjspath)\tianshan\streamsmith\modules\ssm_pausetv_s1\ $(ZQProjsPath)\TianShan\ChannelOnDemand\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\ChannelOnDemand\ChannelOnDemandEx.ICE

!IF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Release"

# Begin Custom Build
InputPath=..\..\..\ChannelOnDemand\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)\TianShan\ChannelOnDemand --output-dir $(ZQProjspath)\tianshan\streamsmith\modules\ssm_Pausetv_s1 $(ZQProjsPath)\TianShan\ChannelOnDemand\$(InputName).ice

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Debug"

# Begin Custom Build
InputPath=..\..\..\ChannelOnDemand\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)\ChannelOnDemand --output-dir $(ZQProjspath)\tianshan\streamsmith\modules\ssm_Pausetv_s1 $(ZQProjsPath)\TianShan\ChannelOnDemand\$(InputName).ice

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ssm_PauseTV_s1.def
# End Source File
# Begin Source File

SOURCE=.\ssm_PauseTV_s1.rc
# End Source File
# Begin Source File

SOURCE=..\..\Service\StreamSmithAdmin.ICE

!IF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Release"

# Begin Custom Build
InputPath=..\..\Service\StreamSmithAdmin.ICE
InputName=StreamSmithAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ice --output-dir . ..\..\service\$(InputName).ice

"..\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssm_PauseTV_s1 - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
