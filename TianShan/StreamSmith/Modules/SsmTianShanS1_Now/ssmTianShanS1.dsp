# Microsoft Developer Studio Project File - Name="ssmTianShanS1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ssmTianShanS1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssmTianShanS1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssmTianShanS1.mak" CFG="ssmTianShanS1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssmTianShanS1 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ssmTianShanS1 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssmTianShanS1 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSMTIANSHANS1_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /Zi /I "." /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(ZQPROJSPATH)/Tianshan/Ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(RegExppKit)" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "_MBCS" /D "_USRDLL" /D "SSMTIANSHANS1_EXPORTS" /D "WITH_ICESTORM" /D "_STLP_NEW_PLATFORM_SDK" /D "LOGFMTWITHTID" /D "CHECK_WITH_GLOG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ice.lib freeze.lib icestorm.lib iceutil.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\..\..\bin\ssm_tianshan_s1.pdb" /debug /machine:I386 /out:"..\..\..\bin\ssm_tianshan_s1.dll" /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "ssmTianShanS1 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SSMTIANSHANS1_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /I "." /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(ZQPROJSPATH)/Tianshan/Ice" /I "$(ZQPROJSPATH)/Tianshan/common" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)\tianshan\shell\ZQCfgPkg" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(RegExppKit)" /D "_DEBUG" /D "_STLP_DEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "_MBCS" /D "_USRDLL" /D "SSMTIANSHANS1_EXPORTS" /D "WITH_ICESTORM" /D "_STLP_NEW_PLATFORM_SDK" /D "LOGFMTWITHTID" /D "CHECK_WITH_GLOG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iced.lib freezed.lib icestormd.lib iceutild.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"..\..\..\bin\ssm_tianshan_s1_d.pdb" /debug /machine:I386 /out:"..\..\..\bin\ssm_tianshan_s1_d.dll" /pdbtype:sept /libpath:"$(ICE_ROOT)\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ssmTianShanS1 - Win32 Release"
# Name "ssmTianShanS1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectService.cpp
# End Source File
# Begin Source File

SOURCE=.\DescribeRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\Environment.cpp
# End Source File
# Begin Source File

SOURCE=.\Factory.cpp
# End Source File
# Begin Source File

SOURCE=.\GetParamRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\PauseRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\PingHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\RequestHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionContext_ice.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionContextImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionView.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamIdx.cpp
# End Source File
# Begin Source File

SOURCE=.\stroprt.cpp
# End Source File
# Begin Source File

SOURCE=.\TeardownRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\TsConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\WatchDog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ConnectService.h
# End Source File
# Begin Source File

SOURCE=.\DescribeRequest.h
# End Source File
# Begin Source File

SOURCE=.\Environment.h
# End Source File
# Begin Source File

SOURCE=.\Factory.h
# End Source File
# Begin Source File

SOURCE=.\GetParamRequest.h
# End Source File
# Begin Source File

SOURCE=.\OptionRequest.h
# End Source File
# Begin Source File

SOURCE=.\PauseRequest.h
# End Source File
# Begin Source File

SOURCE=.\PingHandler.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistEvent.h
# End Source File
# Begin Source File

SOURCE=.\PlayRequest.h
# End Source File
# Begin Source File

SOURCE=.\RequestHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\RtspRelevant.h
# End Source File
# Begin Source File

SOURCE=.\SessionContext_ice.h
# End Source File
# Begin Source File

SOURCE=.\SessionContextImpl.h
# End Source File
# Begin Source File

SOURCE=.\SessionView.h
# End Source File
# Begin Source File

SOURCE=.\SetupRequest.h
# End Source File
# Begin Source File

SOURCE=.\StreamEvent.h
# End Source File
# Begin Source File

SOURCE=.\StreamIdx.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# Begin Source File

SOURCE=.\TeardownRequest.h
# End Source File
# Begin Source File

SOURCE=.\TsConfig.h
# End Source File
# Begin Source File

SOURCE=.\WatchDog.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\SessionContext_ice.ICE

!IF  "$(CFG)" == "ssmTianShanS1 - Win32 Release"

# Begin Custom Build
InputPath=.\SessionContext_ice.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)\slice -I$(ZQProjsPath)\TianShan\ice --output-dir . SessionContext_ice.ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)\slice -I$(ZQProjsPath)\TianShan\ice --index "TianShanS1::StreamIdx, TianShanS1::SessionContext, streamID" StreamIdx --output-dir . SessionContext_ice.ICE \
	

"SessionContext_ice.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionContext_ice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StreamIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ssmTianShanS1 - Win32 Debug"

# Begin Custom Build
InputPath=.\SessionContext_ice.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ice_root)\slice --output-dir . SessionContext_ice.ICE

"SessionContext_ice.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionContext_ice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ssm_tianshan_s1.rc
# End Source File
# End Group
# End Target
# End Project
