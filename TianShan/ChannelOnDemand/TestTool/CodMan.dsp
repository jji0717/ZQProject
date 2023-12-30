# Microsoft Developer Studio Project File - Name="CodMan" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CodMan - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CodMan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CodMan.mak" CFG="CodMan - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CodMan - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CodMan - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CodMan - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "." /I "$(ICE_ROOT)\include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)\tianshan\ice" /D _WIN32_WINNT=0x501 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ice.lib iceutil.lib /nologo /subsystem:windows /pdb:"../../bin/CodMan.pdb" /debug /machine:I386 /out:"../../bin/CodMan.exe" /libpath:"$(ICE_ROOT)\lib"

!ELSEIF  "$(CFG)" == "CodMan - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ICE_ROOT)\include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)\tianshan\ice" /D _WIN32_WINNT=0x501 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iced.lib iceutild.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"$(ICE_ROOT)\lib"

!ENDIF 

# Begin Target

# Name "CodMan - Win32 Release"
# Name "CodMan - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChannelEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelTV.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ChnlItemEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CodMan.cpp
# End Source File
# Begin Source File

SOURCE=.\CodMan.rc
# End Source File
# Begin Source File

SOURCE=.\CodManDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CodManView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenCodDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyView.cpp
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

SOURCE=.\ChannelEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemand.h
# End Source File
# Begin Source File

SOURCE=.\ChannelOnDemandEx.h
# End Source File
# Begin Source File

SOURCE=.\ChannelTV.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ChnlItemEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\CodMan.h
# End Source File
# Begin Source File

SOURCE=.\CodManDoc.h
# End Source File
# Begin Source File

SOURCE=.\CodManView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\OpenCodDlg.h
# End Source File
# Begin Source File

SOURCE=.\PropertyView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\ChannelOnDemand.ICE

!IF  "$(CFG)" == "CodMan - Win32 Release"

# Begin Custom Build
InputPath=..\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir . ..\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CodMan - Win32 Debug"

# Begin Custom Build
InputPath=..\ChannelOnDemand.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)\slice -I$(ZQPROJSPATH)\tianshan\ice --output-dir . ..\ChannelOnDemand.ICE

".\ChannelOnDemand.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ChannelOnDemand.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemandEx.ICE

!IF  "$(CFG)" == "CodMan - Win32 Release"

# Begin Custom Build
InputPath=..\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir . ..\$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "CodMan - Win32 Debug"

# Begin Custom Build
InputPath=..\ChannelOnDemandEx.ICE

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)\slice -I$(ZQPROJSPATH)\tianshan\ice --output-dir . ..\ChannelOnDemandEx.ICE

".\ChannelOnDemandEx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ChannelOnDemandEx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\CodMan.ico
# End Source File
# Begin Source File

SOURCE=.\res\CodMan.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CodManDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
