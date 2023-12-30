# Microsoft Developer Studio Project File - Name="TSClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSClient.mak" CFG="TSClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSClient - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSClient - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSClient - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCLIENT_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I "..\..\common" /I "..\..\..\common" /I "$(STLPORT_ROOT)" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/ChannelOnDemand" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCLIENT_EXPORTS" /D "NEWLOGFMT" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x500 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 mgmtapi.lib snmpapi.lib ice.lib iceutil.lib /nologo /dll /pdb:"../../bin/TSClient.pdb" /map:"../../bin/TSClient.map" /debug /machine:I386 /out:"../../bin/TSClient.dll" /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TSClient - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCLIENT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I "..\..\common" /I "..\..\..\common" /I "$(STLPORT_ROOT)" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/ChannelOnDemand" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSCLIENT_EXPORTS" /D "NEWLOGFMT" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x500 /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mgmtapi.lib snmpapi.lib Iced.lib IceUtild.lib /nologo /dll /pdb:"../../bin/TSClient_d.pdb" /map:"../../bin/TSClient.map" /debug /machine:I386 /out:"../../bin/TSClient_d.dll" /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "TSClient - Win32 Release"
# Name "TSClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GridHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SNMPOper.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TSClient.cpp
# End Source File
# Begin Source File

SOURCE=.\TSClient.def
# End Source File
# Begin Source File

SOURCE=.\VarCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shell\ZQSNMPManPkg\ZQSnmpUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GridHandler.h
# End Source File
# Begin Source File

SOURCE=.\SNMPOper.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TSClient.h
# End Source File
# Begin Source File

SOURCE=.\VarCache.h
# End Source File
# End Group
# Begin Group "ICE Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\ChannelOnDemand\ChannelOnDemand.ICE

!IF  "$(CFG)" == "TSClient - Win32 Release"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir $(ZQProjsPath)/TianShan/Sentry/TSClient $(ZQProjsPath)/TianShan/ChannelOnDemand/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TSClient - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir $(ZQProjsPath)/TianShan/Sentry/TSClient $(ZQProjsPath)/TianShan/ChannelOnDemand/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Client Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemand.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandClient.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandClient.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
