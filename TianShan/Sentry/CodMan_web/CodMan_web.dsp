# Microsoft Developer Studio Project File - Name="CodMan_web" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CodMan_web - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CodMan_web.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CodMan_web.mak" CFG="CodMan_web - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CodMan_web - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CodMan_web - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CodMan_web - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CODMAN_WEB_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I "$(zqprojspath)/common" /I "$(zqprojspath)/tianshan/ice" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CODMAN_WEB_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
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
# ADD LINK32 ice.lib iceutil.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"../../bin/CodWeb.pdb" /debug /machine:I386 /out:"../../bin/CodWeb.dll" /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "CodMan_web - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CODMAN_WEB_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(zqprojspath)/common" /I "$(zqprojspath)/tianshan/ice" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CODMAN_WEB_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /FD /GZ /c
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
# ADD LINK32 iced.lib iceutild.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"../../bin/CodWeb_d.pdb" /debug /machine:I386 /out:"../../bin/CodWeb_d.dll" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "CodMan_web - Win32 Release"
# Name "CodMan_web - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\BasePage.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=.\CodMain.cpp
# End Source File
# Begin Source File

SOURCE=.\CodMan_web.cpp
# End Source File
# Begin Source File

SOURCE=.\EditChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\EditItem.cpp
# End Source File
# Begin Source File

SOURCE=.\InsertItem.cpp
# End Source File
# Begin Source File

SOURCE=.\PushItem.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoveChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoveItem.cpp
# End Source File
# Begin Source File

SOURCE=.\ShowChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\ShowItem.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\stroprt.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddChannel.h
# End Source File
# Begin Source File

SOURCE=.\BasePage.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemand.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.h
# End Source File
# Begin Source File

SOURCE=.\CodMain.h
# End Source File
# Begin Source File

SOURCE=.\DataTypes.h
# End Source File
# Begin Source File

SOURCE=.\EditChannel.h
# End Source File
# Begin Source File

SOURCE=.\EditItem.h
# End Source File
# Begin Source File

SOURCE=.\InsertItem.h
# End Source File
# Begin Source File

SOURCE=.\PushItem.h
# End Source File
# Begin Source File

SOURCE=.\RemoveChannel.h
# End Source File
# Begin Source File

SOURCE=.\RemoveItem.h
# End Source File
# Begin Source File

SOURCE=.\ShowChannel.h
# End Source File
# Begin Source File

SOURCE=.\ShowItem.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\ChannelOnDemand\ChannelOnDemand.ICE

!IF  "$(CFG)" == "CodMan_web - Win32 Release"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir . ..\..\ChannelOnDemand\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CodMan_web - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir . ..\..\ChannelOnDemand\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ChannelOnDemand\ChannelOnDemandEx.ICE

!IF  "$(CFG)" == "CodMan_web - Win32 Release"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I..\..\ChannelOnDemand --output-dir . ..\..\ChannelOnDemand\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CodMan_web - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ChannelOnDemand\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I..\..\ChannelOnDemand --output-dir . ..\..\ChannelOnDemand\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CodMan_web.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
